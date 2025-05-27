// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "SecureTunnelingContext.h"
#include "../logging/LoggerFactory.h"
#include "SecureTunnelingFeature.h"
#include "EguanaTunneling.h"
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
                    const Aws::Crt::Optional<std::string> &rootCa,
                    const string &accessToken,
                    const string &endpoint,
                    const string &address,
                    const int port,
                    const OnConnectionShutdownFn &onConnectionShutdown)
                    : mSharedCrtResourceManager(manager), mRootCa(rootCa.has_value() ? rootCa.value() : ""),
                      mAccessToken(accessToken), mEndpoint(endpoint), mAddress(address), mPort(port),
                      mOnConnectionShutdown(onConnectionShutdown)
                {
                }

                SecureTunnelingContext::~SecureTunnelingContext()
                {
                    if (mSecureTunnel && mSecureTunnel->IsValid())
                    {
                        mSecureTunnel->Close();
                    }
                }

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
                    if (mAccessToken.empty())
                    {
                        LOG_ERROR(TAG, "Cannot connect to secure tunnel. Access token is missing.");
                        return false;
                    }

                    if (mEndpoint.empty())
                    {
                        LOG_ERROR(TAG, "Cannot connect to secure tunnel. Endpoint is missing.");
                        return false;
                    }

                    mSecureTunnel = CreateSecureTunnel(
                        bind(&SecureTunnelingContext::OnConnectionComplete, this),
                        bind(&SecureTunnelingContext::OnConnectionShutdown, this),
                        bind(&SecureTunnelingContext::OnSendDataComplete, this, placeholders::_1),
                        bind(&SecureTunnelingContext::OnDataReceive, this, placeholders::_1),
                        bind(&SecureTunnelingContext::OnStreamStart, this),
                        bind(&SecureTunnelingContext::OnStreamReset, this),
                        bind(&SecureTunnelingContext::OnSessionReset, this));

                    bool connectionSuccess = mSecureTunnel->Connect() == AWS_OP_SUCCESS;

                    if (!connectionSuccess)
                    {
                        LOG_ERROR(TAG, "Cannot connect to secure tunnel. Please see the SDK log for detail.");
                    }

                    return connectionSuccess;
                }

                void SecureTunnelingContext::ConnectToTcpForward()
                {
                    if (!SecureTunnelingFeature::IsValidAddress(mAddress))
                    {
                        LOGM_ERROR(TAG, "Cannot connect to invalid IP address. address=%s", mAddress.c_str());
                        return;
                    }

                    if (!SecureTunnelingFeature::IsValidPort(mPort))
                    {
                        LOGM_ERROR(TAG, "Cannot connect to invalid port. port=%u", mPort);
                        return;
                    }

                    if (mAddress == EMC_LOCAL_INTERFACES_IP_ADDRESS && mPort == SSH_TCP_PORT)
                    {
                        SecureTunnelingFeature::StartDropbearServer();
                    }

                    if (mAddress == TIVA_RS485_IP_ADDRESS && mPort == TIVA_TCP_PORT)
                    {
                        SecureTunnelingFeature::ConfigureRs485Interface();
                        SecureTunnelingFeature::StartSocatListener();
                    }

                    mTcpForward = CreateTcpForward();

                    mTcpForward->Connect();
                }

                void SecureTunnelingContext::DisconnectFromTcpForward() { mTcpForward.reset(); }

                void SecureTunnelingContext::OnConnectionComplete() const
                {
                    LOG_DEBUG(TAG, "SecureTunnelingContext::OnConnectionComplete");
                }

                void SecureTunnelingContext::OnConnectionShutdown()
                {
                    LOG_DEBUG(TAG, "SecureTunnelingContext::OnConnectionShutdown");
                    mOnConnectionShutdown(this);
                }

                void SecureTunnelingContext::OnSendDataComplete(int errorCode) const
                {
                    LOG_DEBUG(TAG, "SecureTunnelingContext::OnSendDataComplete");
                    if (errorCode)
                    {
                        LOGM_ERROR(TAG, "SecureTunnelingContext::OnSendDataComplete errorCode=%d", errorCode);
                    }
                }

                void SecureTunnelingContext::OnDataReceive(const Crt::ByteBuf &data) const
                {
                    LOGM_DEBUG(TAG, "SecureTunnelingContext::OnDataReceive data.len=%zu", data.len);
                    mTcpForward->SendData(aws_byte_cursor_from_buf(&data));
                }

                void SecureTunnelingContext::OnStreamStart()
                {
                    LOG_DEBUG(TAG, "SecureTunnelingContext::OnStreamStart");
                    ConnectToTcpForward();
                }

                void SecureTunnelingContext::OnStreamReset()
                {
                    LOG_DEBUG(TAG, "SecureTunnelingContext::OnStreamReset");
                    DisconnectFromTcpForward();
                }

                void SecureTunnelingContext::OnSessionReset()
                {
                    LOG_DEBUG(TAG, "SecureTunnelingContext::OnSessionReset");
                    DisconnectFromTcpForward();
                }

                void SecureTunnelingContext::OnTcpForwardDataReceive(const Crt::ByteBuf &data) const
                {
                    LOGM_DEBUG(TAG, "SecureTunnelingContext::OnTcpForwardDataReceive data.len=%zu", data.len);
                    mSecureTunnel->SendData(aws_byte_cursor_from_buf(&data));
                }

                void SecureTunnelingContext::StopSecureTunnel()
                {
                    LOG_DEBUG(TAG, "SecureTunnelingContext::StopSecureTunnel");
                    mSecureTunnel->Shutdown();
                }

                std::shared_ptr<SecureTunnelWrapper> SecureTunnelingContext::CreateSecureTunnel(
                    const Aws::Iotsecuretunneling::OnConnectionComplete &onConnectionComplete,
                    const Aws::Iotsecuretunneling::OnConnectionShutdown &onConnectionShutdown,
                    const Aws::Iotsecuretunneling::OnSendDataComplete &onSendDataComplete,
                    const Aws::Iotsecuretunneling::OnDataReceive &onDataReceive,
                    const Aws::Iotsecuretunneling::OnStreamStart &onStreamStart,
                    const Aws::Iotsecuretunneling::OnStreamReset &onStreamReset,
                    const Aws::Iotsecuretunneling::OnSessionReset &onSessionReset)
                {
                    return std::make_shared<SecureTunnelWrapper>(
                        mSharedCrtResourceManager->getAllocator(),
                        mSharedCrtResourceManager->getClientBootstrap(),
                        Crt::Io::SocketOptions(),
                        mAccessToken,
                        AWS_SECURE_TUNNELING_DESTINATION_MODE,
                        mEndpoint,
                        mRootCa,
                        onConnectionComplete,
                        onConnectionShutdown,
                        onSendDataComplete,
                        onDataReceive,
                        onStreamStart,
                        onStreamReset,
                        onSessionReset);
                }

                std::shared_ptr<TcpForward> SecureTunnelingContext::CreateTcpForward()
                {
                    return std::make_shared<TcpForward>(
                        mSharedCrtResourceManager,
                        mAddress,
                        mPort,
                        bind(&SecureTunnelingContext::OnTcpForwardDataReceive, this, placeholders::_1));
                }
            } // namespace SecureTunneling
        }     // namespace DeviceClient
    }         // namespace Iot
} // namespace Aws
