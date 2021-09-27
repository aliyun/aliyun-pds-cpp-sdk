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

#include <alibabacloud/pds/model/DataPutByUrlRequest.h>
#include <sstream>
#include <iostream>
#include "../utils/Utils.h"
#include "ModelError.h"
#include <alibabacloud/pds/Const.h>

using namespace AlibabaCloud::PDS;

DataPutByUrlRequest::DataPutByUrlRequest(
    const std::string &url,
    const std::shared_ptr<std::iostream> &content) :
    DataPutByUrlRequest(url, content, RequestMetaData())
{
}

DataPutByUrlRequest::DataPutByUrlRequest(
    const std::string &url,
    const std::shared_ptr<std::iostream> &content,
    const RequestMetaData &metaData) :
    content_(content),
    metaData_(metaData),
    contentLengthIsSet_(false),
    trafficLimit_(0),
    userAgent_()
{
    setPath(url);
    setFlags(Flags()|REQUEST_FLAG_PARAM_IN_PATH|REQUEST_FLAG_CHECK_CRC64|REQUEST_FLAG_PDS_DATA_REQUEST);
}

void DataPutByUrlRequest::setContentLength(uint64_t length)
{
    contentLength_ = length;
    contentLengthIsSet_ = true;
}

void DataPutByUrlRequest::setTrafficLimit(uint64_t value)
{
    trafficLimit_ = value;
}

void DataPutByUrlRequest::setUserAgent(const std::string& ua)
{
    userAgent_ = ua;
}

void DataPutByUrlRequest::setContent(const std::shared_ptr<std::iostream> &content)
{
    content_ = content;
}

std::shared_ptr<std::iostream> DataPutByUrlRequest::Body() const
{
    return content_;
}

HeaderCollection DataPutByUrlRequest::specialHeaders() const
{
    auto headers = PdsRequest::specialHeaders();
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

int DataPutByUrlRequest::validate() const
{
    int ret = PdsRequest::validate();
    if (ret)
    {
        return ret;
    }

    if (content_ == nullptr) {
        return ARG_ERROR_REQUEST_BODY_NULLPTR;
    }

    if (content_->bad()) {
        return ARG_ERROR_REQUEST_BODY_BAD_STATE;
    }

    if (content_->fail()) {
        return ARG_ERROR_REQUEST_BODY_FAIL_STATE;
    }

    uint64_t partSize;

    if (contentLengthIsSet_) {
        partSize = contentLength_;
    }
    else {
        partSize = GetIOStreamLength(*content_);
    }

    if (partSize > MaxFileSize) {
        return ARG_ERROR_MULTIPARTUPLOAD_PARTSIZE_RANGE;
    }

    return 0;
}
