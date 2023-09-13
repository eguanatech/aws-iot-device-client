// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "SecureTunnelingFeature.h"
#include "../logging/LoggerFactory.h"
#include "SecureTunnelingContext.h"
#include "TcpForward.h"
#include <aws/crt/mqtt/MqttClient.h>
#include <aws/iotsecuretunneling/IotSecureTunnelingClient.h>
#include <aws/iotsecuretunneling/SubscribeToTunnelsNotifyRequest.h>
#include <map>
#include <memory>
#include <thread>

using namespace std;
using namespace Aws::Iotsecuretunneling;
using namespace Aws::Iot::DeviceClient::Logging;
using namespace Aws::Iot::DeviceClient::Util;

namespace Aws
{
    namespace Iot
    {
        namespace DeviceClient
        {
            namespace SecureTunneling
            {
                constexpr char SecureTunnelingFeature::TAG[];
                constexpr char SecureTunnelingFeature::DEFAULT_PROXY_ENDPOINT_HOST_FORMAT[];
                std::map<std::string, uint16_t> SecureTunnelingFeature::mServiceToPortMap;

                SecureTunnelingFeature::SecureTunnelingFeature() = default;

                SecureTunnelingFeature::~SecureTunnelingFeature() { aws_http_library_clean_up(); }

                int SecureTunnelingFeature::init(
                    shared_ptr<SharedCrtResourceManager> sharedCrtResourceManager,
                    shared_ptr<ClientBaseNotifier> notifier,
                    const PlainConfig &config)
                {
                    sharedCrtResourceManager->initializeAWSHttpLib();

                    this->mSharedCrtResourceManager = sharedCrtResourceManager;
                    mDeviceApiHandle = unique_ptr<Aws::Iotdevicecommon::DeviceApiHandle>(
                        new Aws::Iotdevicecommon::DeviceApiHandle(sharedCrtResourceManager->getAllocator()));
                    mClientBaseNotifier = notifier;

                    LoadFromConfig(config);

                    return 0;
                }

                string SecureTunnelingFeature::getName() { return "Secure Tunneling"; }

                int SecureTunnelingFeature::start()
                {
                    RunSecureTunneling();
                    mClientBaseNotifier->onEvent((Feature *)this, ClientBaseEventNotification::FEATURE_STARTED);
                    return 0;
                }

                int SecureTunnelingFeature::stop()
                {
                    for (auto &c : mContexts)
                    {
                        c.reset();
                    }

                    mClientBaseNotifier->onEvent((Feature *)this, ClientBaseEventNotification::FEATURE_STOPPED);
                    return 0;
                }

                uint16_t SecureTunnelingFeature::GetPortFromService(const std::string &service)
                {
                    if (mServiceToPortMap.empty())
                    {
                        mServiceToPortMap["SSH"] = 22;
                        mServiceToPortMap["VNC"] = 5900;
                        mServiceToPortMap["GW"] = 8080;
                        mServiceToPortMap["TIVA"] = 502;
                    }

                    auto result = mServiceToPortMap.find(service);
                    if (result == mServiceToPortMap.end())
                    {
                        LOGM_ERROR(TAG, "Requested unsupported service. service=%s", service.c_str());
                        return 0; // TODO: Consider throw
                    }

                    return result->second;
                }

                bool SecureTunnelingFeature::IsValidPort(int port) { return 1 <= port && port <= 65535; }

                void SecureTunnelingFeature::LoadFromConfig(const PlainConfig &config)
                {
                    mThingName = *config.thingName;
                    mRootCa = *config.rootCa;
                    mSubscribeNotification = config.tunneling.subscribeNotification;
                    mEndpoint = config.tunneling.endpoint;

                    if (!config.tunneling.subscribeNotification)
                    {
                        std::unique_ptr<SecureTunnelingContext> context =
                            unique_ptr<SecureTunnelingContext>(new SecureTunnelingContext(
                                mSharedCrtResourceManager,
                                *config.rootCa,
                                *config.tunneling.destinationAccessToken,
                                GetEndpoint(*config.tunneling.region),
                                static_cast<uint16_t>(config.tunneling.port.value()),
                                bind(&SecureTunnelingFeature::OnConnectionShutdown, this, placeholders::_1)));
                        mContexts.push_back(std::move(context));
                    }
                }

                void SecureTunnelingFeature::RunSecureTunneling()
                {
                    LOGM_INFO(TAG, "Running %s!", getName().c_str());

                    if (mSubscribeNotification)
                    {
                        SubscribeToTunnelsNotifyRequest request;
                        request.ThingName = mThingName.c_str();

                        IotSecureTunnelingClient client(mSharedCrtResourceManager->getConnection());
                        client.SubscribeToTunnelsNotify(
                            request,
                            AWS_MQTT_QOS_AT_LEAST_ONCE,
                            bind(
                                &SecureTunnelingFeature::OnSubscribeToTunnelsNotifyResponse,
                                this,
                                placeholders::_1,
                                placeholders::_2),
                            bind(&SecureTunnelingFeature::OnSubscribeComplete, this, placeholders::_1));
                    }
                    else
                    {
                        // Access token and region were loaded from config and have already been validated
                        for (auto &c : mContexts)
                        {
                            c->ConnectToSecureTunnel();
                        }
                    }
                }

                void SecureTunnelingFeature::OnSubscribeToTunnelsNotifyResponse(
                    SecureTunnelingNotifyResponse *response,
                    int ioErr)
                {
                    LOG_DEBUG(TAG, "Received MQTT Tunnel Notification");

                    if (ioErr || !response)
                    {
                        LOGM_ERROR(TAG, "OnSubscribeToTunnelsNotifyResponse received error. ioErr=%d", ioErr);
                        return;
                    }

                    for (auto &c : mContexts)
                    {
                        if (c->IsDuplicateNotification(*response))
                        {
                            LOG_INFO(TAG, "Received duplicate MQTT Tunnel Notification. Ignoring...");
                            return;
                        }
                    }

                    string clientMode = response->ClientMode->c_str();
                    if (clientMode != "destination")
                    {
                        LOGM_ERROR(TAG, "Unexpected client mode: %s", clientMode.c_str());
                        return;
                    }

                    size_t nServices = response->Services->size();
                    if (nServices == 0)
                    {
                        LOG_ERROR(TAG, "no service requested");
                        return;
                    }

                    string accessToken = response->ClientAccessToken->c_str();
                    if (accessToken.empty())
                    {
                        LOG_ERROR(TAG, "access token cannot be empty");
                        return;
                    }

                    string region = response->Region->c_str();
                    if (region.empty())
                    {
                        LOG_ERROR(TAG, "region cannot be empty");
                        return;
                    }

                    string command = "localproxy -r " + region + " -t " + accessToken + " -d ";
                    bool isTcp = true;
                    for (int x = 0; x < (int) nServices; x++) {
                        string service = response->Services->at(x).c_str();
                        uint16_t port = GetPortFromService(service);
                        if (!IsValidPort(port))
                        {
                            LOGM_WARN(TAG, "Requested service is not supported: %s", service.c_str());
                            continue;
                        }

                        LOGM_DEBUG(TAG, "Region=%s, Service=%s", region.c_str(), service.c_str());

                        if (x > 0) {
                            command += ",";
                        }

                        if (service == "SSH") {
                            command += "SSH=10.3.2.1:22";
                        } else if (service == "GW") {
                            command += "GW=10.3.2.1:8080";
                        } else if (service == "TIVA") {
                            int ret = system("ping -c1 -s1 169.254.0.5");
                            if (ret == 0) {
                                LOG_INFO(TAG, "Trying to tunnel to the inverter by TCP.");
                                command += "TIVA=169.254.0.5:502";
                            } else {
                                LOG_INFO(TAG, "Trying to tunnel to the inverter by RS485.");
                                command += "TIVA=10.3.2.1:503";
                                isTcp = false;
                            }
                        }
                    }

                    // store logs to file
                    command += " 2>&1 | tee /var/log/localproxy.log &";
                    // redirect packets received by device client to RS485
                    LOGM_INFO(TAG, "command = %s", command.c_str());
                    int ret = system(command.c_str());
                    LOGM_INFO(TAG, "Running localproxy return code = %d", ret);

                    if (!isTcp) {
                        command = "nc -l 10.3.2.1:503 > /dev/ttymxc2 < /dev/ttymxc2 &";
                        LOGM_INFO(TAG, "command = %s", command.c_str());
                        int ret = system(command.c_str());
                        LOGM_INFO(TAG, "Running netcat command return code = %d", ret);
                    }

                    // if (nServices > 1)
                    // {
                    //     LOGM_INFO(
                    //         TAG,
                    //         "Received a multi-port tunnel request, but multi-port tunneling is not currently supported "
                    //         "by Device Client. region = %s, token = %s", response->Region->c_str(), response->ClientAccessToken->c_str());

                    //     string command = "localproxy -d SSH=10.3.2.1:22,GW=10.3.2.1:8080,TIVA=169.254.0.5:502 -r " + region + " -t " + ClientAccessToken + " 2>&1 | tee /var/log/localproxy.log";
                    //     int ret = system(command.c_str());
                    //     LOGM_INFO(TAG, "Running localproxy instead return code = %d", ret);
                    //     return;
                    // }

                    // string service = response->Services->at(0).c_str();
                    // uint16_t port = GetPortFromService(service);
                    // if (!IsValidPort(port))
                    // {
                    //     LOGM_ERROR(TAG, "Requested service is not supported: %s", service.c_str());
                    //     return;
                    // }

                    // std::unique_ptr<SecureTunnelingContext> context =
                    //     unique_ptr<SecureTunnelingContext>(new SecureTunnelingContext(
                    //         mSharedCrtResourceManager,
                    //         mRootCa,
                    //         accessToken,
                    //         GetEndpoint(region),
                    //         port,
                    //         bind(&SecureTunnelingFeature::OnConnectionShutdown, this, placeholders::_1)));
                    // if (context->ConnectToSecureTunnel())
                    // {
                    //     mContexts.push_back(std::move(context));
                    // }
                }

                void SecureTunnelingFeature::OnSubscribeComplete(int ioErr)
                {
                    LOG_DEBUG(TAG, "Subscribed to tunnel notification topic");

                    if (ioErr)
                    {
                        LOGM_ERROR(TAG, "Couldn't subscribe to tunnel notification topic. ioErr=%d", ioErr);
                        // TODO: Handle subscription error

                        // TODO: UA-5775 - Incorporate the baseClientNotifier onError event
                    }
                }

                string SecureTunnelingFeature::GetEndpoint(const string &region)
                {
                    if (mEndpoint.has_value())
                    {
                        return mEndpoint.value();
                    }

                    string endpoint = FormatMessage(DEFAULT_PROXY_ENDPOINT_HOST_FORMAT, region.c_str());

                    if (region.substr(0, 3) == "cn-")
                    {
                        // Chinese regions have ".cn" at the end:
                        // data.tunneling.iot.<region>.amazonaws.com.cn
                        // Examples of Chinese region name: "cn-north-1", "cn-northwest-1"
                        endpoint = endpoint + ".cn";
                    }

                    return endpoint;
                }

                void SecureTunnelingFeature::OnConnectionShutdown(SecureTunnelingContext *contextToRemove)
                {
                    LOG_DEBUG(TAG, "SecureTunnelingFeature::OnConnectionShutdown");

                    auto it = find_if(mContexts.begin(), mContexts.end(), [&](unique_ptr<SecureTunnelingContext> &c) {
                        return c.get() == contextToRemove;
                    });
                    mContexts.erase(std::remove(mContexts.begin(), mContexts.end(), *it));

#if defined(DISABLE_MQTT)
                    if (mContexts.empty())
                    {
                        stop();
                    }
#endif
                }

            } // namespace SecureTunneling
        }     // namespace DeviceClient
    }         // namespace Iot
} // namespace Aws
