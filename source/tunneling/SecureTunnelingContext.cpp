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
                constexpr char SecureTunnelingContext::NETCAT_COMMAND_FORMAT[];

                SecureTunnelingContext::SecureTunnelingContext(
                    shared_ptr<SharedCrtResourceManager> manager,
                    const string &rootCa,
                    const string &accessToken,
                    const string &endpoint,
                    const string &address,
                    uint16_t port,
                    bool isTcp,
                    OnConnectionShutdownFn onConnectionShutdown)
                {
                    mSharedCrtResourceManager = manager;
                    mRootCa = rootCa;
                    mAccessToken = accessToken;
                    mEndpoint = endpoint;
                    mAddress = address;
                    mPort = port;
                    mIsTcp = isTcp;
                    mOnConnectionShutdown = onConnectionShutdown;
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
                    if (mAccessToken.empty() || mEndpoint.empty())
                    {
                        LOG_ERROR(TAG, "Cannot connect to secure tunnel. Either access token or endpoint is empty");
                        return false;
                    }

                    mSecureTunnel = unique_ptr<SecureTunnel>(new SecureTunnel(
                        mSharedCrtResourceManager->getAllocator(),
                        mSharedCrtResourceManager->getClientBootstrap(),
                        Aws::Crt::Io::SocketOptions(),

                        mAccessToken,
                        AWS_SECURE_TUNNELING_DESTINATION_MODE,
                        mEndpoint,
                        mRootCa,

                        bind(&SecureTunnelingContext::OnConnectionComplete, this),
                        bind(&SecureTunnelingContext::OnConnectionShutdown, this),
                        bind(&SecureTunnelingContext::OnSendDataComplete, this, placeholders::_1),
                        bind(&SecureTunnelingContext::OnDataReceive, this, placeholders::_1),
                        bind(&SecureTunnelingContext::OnStreamStart, this),
                        bind(&SecureTunnelingContext::OnStreamReset, this),
                        bind(&SecureTunnelingContext::OnSessionReset, this)));

                    bool ok = mSecureTunnel->Connect() == AWS_OP_SUCCESS;
                    if (!ok)
                    {
                        LOG_ERROR(TAG, "Cannot connect to secure tunnel. Please see the SDK log for detail.");
                    }
                    return ok;
                }

                void SecureTunnelingContext::ConnectToTcpForward()
                {
                    if (!SecureTunnelingFeature::IsValidPort(mPort))
                    {
                        LOGM_ERROR(TAG, "Cannot connect to invalid local port. port=%u", mPort);
                        return;
                    }

                    if (!SecureTunnelingFeature::IsValidAddress(mAddress))
                    {
                        LOGM_ERROR(TAG, "Cannot connect to invalid IP. address=%s", mAddress.c_str());
                        return;
                    }

                    if (!mIsTcp)
                    {
                        // bind local port to RS485 serial interface
                        string command = Aws::Iot::DeviceClient::Util::FormatMessage(NETCAT_COMMAND_FORMAT, mAddress.c_str(), mPort);
                        int ret = system(command.c_str());
                        LOGM_INFO(TAG, "Running netcat for RS485 connection, return %d", ret);
                    }

                    mTcpForward = unique_ptr<TcpForward>(new TcpForward(
                        mSharedCrtResourceManager,
                        mAddress,
                        mPort,
                        bind(&SecureTunnelingContext::OnTcpForwardDataReceive, this, placeholders::_1)));
                    mTcpForward->Connect();
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

                void SecureTunnelingContext::OnTcpForwardDataReceive(const Crt::ByteBuf &data)
                {
                    LOGM_DEBUG(TAG, "SecureTunnelingContext::OnTcpForwardDataReceive data.len=%zu", data.len);
                    mSecureTunnel->SendData(aws_byte_cursor_from_buf(&data));
                }

            } // namespace SecureTunneling
        }     // namespace DeviceClient
    }         // namespace Iot
} // namespace Aws
