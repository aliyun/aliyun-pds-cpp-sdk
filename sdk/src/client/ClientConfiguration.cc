/*
 * Copyright 2009-2021 Alibaba Cloud All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <alibabacloud/pds/Config.h>
#include <alibabacloud/pds/client/ClientConfiguration.h>
#include <alibabacloud/pds/client/RetryStrategy.h>
#include <sstream>
#include "../utils/Utils.h"

using namespace AlibabaCloud::PDS;

//default for configuration begin
#if defined(PLATFORM_WINDOWS)
static const char* PLATFORM_NAME = "Windows";
#elif defined(PLATFORM_LINUX)
static const char* PLATFORM_NAME = "Linux";
#elif defined(PLATFORM_APPLE)
static const char* PLATFORM_NAME = "MacOS";
#elif defined(PLATFORM_ANDROID)
static const char* PLATFORM_NAME = "Android";
#else
static const char* PLATFORM_NAME = "Unknown";
#endif
static std::string DefaultUserAgent()
{
  std::stringstream ss;
  ss << "aliyun-sdk-cpp/" << ALIBABACLOUD_PDS_VERSION_STR << " (" << PLATFORM_NAME << ")";
  return ss.str();
}

class DefaultRetryStrategy : public RetryStrategy
{
public:

    DefaultRetryStrategy(long maxRetries = 3, long scaleFactor = 300) :
        m_scaleFactor(scaleFactor), m_maxRetries(maxRetries)
    {}

    bool shouldRetry(const Error & error, long attemptedRetries) const;

    long calcDelayTimeMs(const Error & error, long attemptedRetries) const;

private:
    long m_scaleFactor;
    long m_maxRetries;
};

bool DefaultRetryStrategy::shouldRetry(const Error & error, long attemptedRetries) const
{
    if (attemptedRetries >= m_maxRetries)
        return false;

    long responseCode = error.Status();

    // http code
    // accessToken expired, 429 too many requests
    if ((responseCode == 401 && error.Code().compare("AccessTokenInvalid") == 0) ||
        (responseCode == 429) ||
        (responseCode > 499 && responseCode < 599)) {
        return true;
    }
    else {
        switch (responseCode)
        {
        //curl error code
        case (ERROR_CURL_BASE + 7):  //CURLE_COULDNT_CONNECT
        case (ERROR_CURL_BASE + 18): //CURLE_PARTIAL_FILE
        case (ERROR_CURL_BASE + 23): //CURLE_WRITE_ERROR
        case (ERROR_CURL_BASE + 28): //CURLE_OPERATION_TIMEDOUT
        case (ERROR_CURL_BASE + 52): //CURLE_GOT_NOTHING
        case (ERROR_CURL_BASE + 55): //CURLE_SEND_ERROR
        case (ERROR_CURL_BASE + 56): //CURLE_RECV_ERROR
            return true;
        default:
            break;
        };
    }

    return false;
}

long DefaultRetryStrategy::calcDelayTimeMs(const Error & error, long attemptedRetries) const
{
    UNUSED_PARAM(error);
    return (1 << attemptedRetries) * m_scaleFactor;
}

//default for configuration end

ClientConfiguration::ClientConfiguration() :
    userAgent(DefaultUserAgent()),
    scheme(Http::Scheme::HTTP),
    maxConnections(16),
    requestTimeoutMs(10000),
    connectTimeoutMs(5000),
    retryStrategy(std::make_shared<DefaultRetryStrategy>()),
    proxyScheme(Http::Scheme::HTTP),
    proxyPort(0),
    verifySSL(true),
    isCname(false),
    enableCrc64(true),
    enableRapidUpload(false),
    enableCheckDownloadFileSize(true),
    enableDownloadPunishedFile(true),
    sendRateLimiter(nullptr),
    recvRateLimiter(nullptr),
    executor(nullptr),
    httpClient(nullptr)
{

}

