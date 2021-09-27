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


#include <alibabacloud/pds/model/DataGetByUrlRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include <sstream>
#include "ModelError.h"

using namespace AlibabaCloud::PDS;

DataGetByUrlRequest::DataGetByUrlRequest(const std::string &url):
    DataGetByUrlRequest(url, RequestMetaData())
{
}

DataGetByUrlRequest::DataGetByUrlRequest(const std::string &url, const RequestMetaData &metaData) :
    rangeIsSet_(false),
    metaData_(metaData),
    trafficLimit_(0),
    userAgent_()
{
    setPath(url);
    setFlags(Flags()|REQUEST_FLAG_PARAM_IN_PATH|REQUEST_FLAG_CHECK_CRC64|REQUEST_FLAG_PDS_DATA_REQUEST);
}

void DataGetByUrlRequest::setUrl(const std::string& url)
{
    setPath(url);
}

void DataGetByUrlRequest::setRange(int64_t start, int64_t end)
{
    range_[0] = start;
    range_[1] = end;
    rangeIsSet_ = true;
}

void DataGetByUrlRequest::setTrafficLimit(uint64_t value)
{
    trafficLimit_ = value;
}

void DataGetByUrlRequest::setUserAgent(const std::string& ua)
{
    userAgent_ = ua;
}

int DataGetByUrlRequest::validate() const
{
    int ret = PdsRequest::validate();
    if (ret != 0)
        return ret;

    if (rangeIsSet_ && (range_[0] < 0 || range_[1] < -1 || (range_[1] > -1 && range_[1] < range_[0]) ))
        return ARG_ERROR_DATA_RANGE_INVALID;

    return 0;
}

HeaderCollection DataGetByUrlRequest::specialHeaders() const
{
    auto headers = PdsRequest::specialHeaders();
    if (rangeIsSet_) {
        std::stringstream ss;
        ss << "bytes=" << range_[0] << "-";
        if (range_[1] != -1) ss << range_[1];
        headers[Http::RANGE] = ss.str();
    }

    if (trafficLimit_ != 0) {
        headers["x-oss-traffic-limit"] = std::to_string(trafficLimit_);
    }

    if (!userAgent_.empty()) {
        headers[Http::USER_AGENT] = userAgent_;
    }
    return headers;
}
