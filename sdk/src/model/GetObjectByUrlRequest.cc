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


#include <alibabacloud/pds/model/GetObjectByUrlRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include <sstream>

using namespace AlibabaCloud::PDS;

GetObjectByUrlRequest::GetObjectByUrlRequest(const std::string &url):
    GetObjectByUrlRequest(url, ObjectMetaData())
{
}

GetObjectByUrlRequest::GetObjectByUrlRequest(const std::string &url, const ObjectMetaData &metaData) :
    ServiceRequest(),
    metaData_(metaData)
{
    setPath(url);
    setFlags(Flags()|REQUEST_FLAG_PARAM_IN_PATH|REQUEST_FLAG_CHECK_CRC64);
}

void GetObjectByUrlRequest::setRange(int64_t start, int64_t end)
{
    range_[0] = start;
    range_[1] = end;
    rangeIsSet_ = true;
}

void GetObjectByUrlRequest::setProcess(const std::string &process)
{
    process_ = process;
}

void GetObjectByUrlRequest::setTrafficLimit(uint64_t value)
{
    trafficLimit_ = value;
}

void GetObjectByUrlRequest::setUserAgent(const std::string& ua)
{
    userAgent_ = ua;
}

HeaderCollection GetObjectByUrlRequest::Headers() const
{
    auto headers = metaData_.toHeaderCollection();
    if (!metaData_.hasHeader(Http::DATE)) {
        headers[Http::DATE] = "";
    }
    return headers;
}

ParameterCollection GetObjectByUrlRequest::Parameters() const
{
    return ParameterCollection();
}

std::shared_ptr<std::iostream> GetObjectByUrlRequest::Body() const
{
    return nullptr;
}
