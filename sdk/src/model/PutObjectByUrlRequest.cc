/*
 * Copyright 2009-2017 Alibaba Cloud All rights reserved.
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

#include <alibabacloud/pds/model/PutObjectByUrlRequest.h>
#include <sstream>
#include <iostream>

using namespace AlibabaCloud::PDS;

PutObjectByUrlRequest::PutObjectByUrlRequest(
    const std::string &url,
    const std::shared_ptr<std::iostream> &content) :
    PutObjectByUrlRequest(url, content, ObjectMetaData())
{
}

PutObjectByUrlRequest::PutObjectByUrlRequest(
    const std::string &url,
    const std::shared_ptr<std::iostream> &content,
    const ObjectMetaData &metaData) :
    PdsObjectRequest(),
    content_(content),
    metaData_(metaData)
{
    setPath(url);
    setFlags(Flags()|REQUEST_FLAG_PARAM_IN_PATH|REQUEST_FLAG_CHECK_CRC64);
}

void PutObjectByUrlRequest::setContentLength(uint64_t length)
{
    contentLength_ = length;
    contentLengthIsSet_ = true;
}

void PutObjectByUrlRequest::setTrafficLimit(uint64_t value)
{
    trafficLimit_ = value;
}

void PutObjectByUrlRequest::setUserAgent(const std::string& ua)
{
    userAgent_ = ua;
}

HeaderCollection PutObjectByUrlRequest::specialHeaders() const
{
    auto headers = PdsObjectRequest::specialHeaders();
    headers[Http::CONTENT_TYPE] = "";
    if (contentLengthIsSet_) {
        headers[Http::CONTENT_LENGTH] = std::to_string(contentLength_);
    }
    if (trafficLimit_ != 0) {
        headers["x-oss-traffic-limit"] = std::to_string(trafficLimit_);
    }
    if (!userAgent_.empty()) {
        headers[Http::USER_AGENT] = userAgent_;
    }
    return headers;
}

HeaderCollection PutObjectByUrlRequest::Headers() const
{
    auto headers = metaData_.toHeaderCollection();
    if (headers.find(Http::DATE) == headers.end()) {
        headers[Http::DATE] = "";
    }
    return headers;
}

ParameterCollection PutObjectByUrlRequest::Parameters() const
{
    return ParameterCollection();
}

std::shared_ptr<std::iostream> PutObjectByUrlRequest::Body() const
{
    return content_;
}
