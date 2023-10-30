// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "SecureTunnelingContext.h"
#include "../logging/LoggerFactory.h"
#include "SecureTunnelingFeature.h"
#include <aws/iotsecuretunneling/SecureTunnel.h>

using namespace std;
using namespace Aws::Iotsecuretunneling;
using namespace Aws::Iot::DeviceClient::Logging;

namespace Aws
{
    namespace Iot
    {
        namespace DeviceClient
        {
            namespace SecureTunneling
            {
                constexpr char SecureTunnelingContext::TAG[];

                SecureTunnelingContext::SecureTunnelingContext(
                    shared_ptr<SharedCrtResourceManager> manager,
                    const string &rootCa,
                    const string &accessToken,
                    const string &region,
                    const string &destination,
                    OnConnectionShutdownFn onConnectionShutdown,
                    const bool isRS485)
                {
                    mSharedCrtResourceManager = manager;
                    mRootCa = rootCa;
                    mAccessToken = accessToken;
                    mRegion = region;
                    mDestination = destination;
                    mOnConnectionShutdown = onConnectionShutdown;
                    mIsRS485 = isRS485;
                }

                SecureTunnelingContext::~SecureTunnelingContext() { mSecureTunnel->Close(); }

                template <typename T>
                static bool operator==(const Aws::Crt::Optional<T> &lhs, const Aws::Crt::Optional<T> &rhs)
                {
                    if (!lhs.has_value() && !rhs.has_value())
                    {
                        return true;
                    }
                    else if (lhs.has_value() && rhs.has_value())
                    {
                        return lhs.value() == rhs.value();
                    }
                    else
                    {
                        return false;
                    }
                }

                static bool operator==(
                    const SecureTunnelingNotifyResponse &lhs,
                    const SecureTunnelingNotifyResponse &rhs)
                {
                    return lhs.Region == rhs.Region && lhs.ClientMode == rhs.ClientMode &&
                           lhs.Services == rhs.Services && lhs.ClientAccessToken == rhs.ClientAccessToken;
                }

                bool SecureTunnelingContext::IsDuplicateNotification(
                    const Aws::Iotsecuretunneling::SecureTunnelingNotifyResponse &response)
                {
                    if (mLastSeenNotifyResponse.has_value() && mLastSeenNotifyResponse.value() == response)
                    {
                        return true;
                    }

                    mLastSeenNotifyResponse = response;
                    return false;
                }

                bool SecureTunnelingContext::ConnectToSecureTunnel()
                {
                    if (mAccessToken.empty() || mRegion.empty() || mDestination.empty())
                    {
                        LOG_ERROR(TAG, "Cannot connect to secure tunnel. Either access token or endpoint is empty");
                        return false;
                    }

                    string command = "localproxy -r " + mRegion + " -d " + mDestination + " -t " + mAccessToken + " 2>&1 | tee /var/log/localproxy.log &";
                    LOGM_INFO(TAG, "command=%s", command.c_str());
                    int ret = system(command.c_str());
                    LOGM_INFO(TAG, "Running localproxy, return %d", ret);

                    return ret == 0;

                    // mSecureTunnel = unique_ptr<SecureTunnel>(new SecureTunnel(
                    //     mSharedCrtResourceManager->getAllocator(),
                    //     mSharedCrtResourceManager->getClientBootstrap(),
                    //     Aws::Crt::Io::SocketOptions(),

                    //     mAccessToken,
                    //     AWS_SECURE_TUNNELING_DESTINATION_MODE,
                    //     mEndpoint,
                    //     mRootCa,

                    //     bind(&SecureTunnelingContext::OnConnectionComplete, this),
                    //     bind(&SecureTunnelingContext::OnConnectionShutdown, this),
                    //     bind(&SecureTunnelingContext::OnSendDataComplete, this, placeholders::_1),
                    //     bind(&SecureTunnelingContext::OnDataReceive, this, placeholders::_1),
                    //     bind(&SecureTunnelingContext::OnStreamStart, this),
                    //     bind(&SecureTunnelingContext::OnStreamReset, this),
                    //     bind(&SecureTunnelingContext::OnSessionReset, this)));

                    // bool ok = mSecureTunnel->Connect() == AWS_OP_SUCCESS;
                    // if (!ok)
                    // {
                    //     LOG_ERROR(TAG, "Cannot connect to secure tunnel. Please see the SDK log for detail.");
                    // }
                    // return ok;
                }

                void SecureTunnelingContext::ConnectToTcpForward()
                {
                    // string command = "localproxy -r " + mEndpoint + " -t " + mAccessToken + " -d " + mDestination;
                    // // storing logs to file
                    // command += " 2>&1 | tee /var/log/localproxy.log &";
                    // LOGM_INFO(TAG, "command=%s", command.c_str());
                    // int ret = system(command.c_str());
                    // LOGM_INFO(TAG, "Running localproxy, return %d", ret);

                    if (mIsRS485) {
                        // bind local port to RS485 serial interface
                        string command = "nc -l 10.3.2.1:503 > /dev/ttymxc2 < /dev/ttymxc2 &";
                        LOGM_INFO(TAG, "command=%s", command.c_str());
                        int ret = system(command.c_str());
                        LOGM_INFO(TAG, "Running netcat, return %d", ret);
                    }
                }

                void SecureTunnelingContext::DisconnectFromTcpForward() { mTcpForward.reset(); }

                void SecureTunnelingContext::OnConnectionComplete()
                {
                    LOG_DEBUG(TAG, "SecureTunnelingContext::OnConnectionComplete");
                }

                void SecureTunnelingContext::OnConnectionShutdown()
                {
                    LOG_DEBUG(TAG, "SecureTunnelingContext::OnConnectionShutdown");
                    mOnConnectionShutdown(this);
                }

                void SecureTunnelingContext::OnSendDataComplete(int errorCode)
                {
                    LOG_DEBUG(TAG, "SecureTunnelingContext::OnSendDataComplete");
                    if (errorCode)
                    {
                        LOGM_ERROR(TAG, "SecureTunnelingContext::OnSendDataComplete errorCode=%d", errorCode);
                    }
                }

                void SecureTunnelingContext::OnDataReceive(const Crt::ByteBuf &data)
                {
                    LOGM_DEBUG(TAG, "SecureTunnelingContext::OnDataReceive data.len=%zu", data.len);
                    // mTcpForward->SendData(aws_byte_cursor_from_buf(&data));
                }

                void SecureTunnelingContext::OnStreamStart()
                {
                    LOG_DEBUG(TAG, "SecureTunnelingContext::OnStreamStart");
                    ConnectToTcpForward();
                }

                void SecureTunnelingContext::OnStreamReset()
                {
                    LOG_DEBUG(TAG, "SecureTunnelingContext::OnStreamReset");
                    // DisconnectFromTcpForward();
                }

                void SecureTunnelingContext::OnSessionReset()
                {
                    LOG_DEBUG(TAG, "SecureTunnelingContext::OnSessionReset");
                    // DisconnectFromTcpForward();
                }

                void SecureTunnelingContext::OnTcpForwardDataReceive(const Crt::ByteBuf &data)
                {
                    LOGM_DEBUG(TAG, "SecureTunnelingContext::OnTcpForwardDataReceive data.len=%zu", data.len);
                    // mSecureTunnel->SendData(aws_byte_cursor_from_buf(&data));
                }

            } // namespace SecureTunneling
        }     // namespace DeviceClient
    }         // namespace Iot
} // namespace Aws
