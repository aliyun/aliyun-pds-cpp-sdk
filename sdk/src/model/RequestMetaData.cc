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

#include <alibabacloud/pds/model/RequestMetaData.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"

using namespace AlibabaCloud::PDS;

static const std::string gEmpty = "";

RequestMetaData::RequestMetaData(const HeaderCollection& data)
{
    *this = data;
}

RequestMetaData& RequestMetaData::operator=(const HeaderCollection& data)
{
    for (auto const &header : data) {
        // filter x-oss-meta
        if (header.first.compare(0, 11, "x-oss-meta-", 11))
            metaData_[header.first] = header.second;
    }

    if (metaData_.find(Http::ETAG) != metaData_.end()) {
        metaData_[Http::ETAG] = TrimQuotes(metaData_.at(Http::ETAG).c_str());
    }

    return *this;
}

int64_t RequestMetaData::ContentLength() const
{
    if (metaData_.find(Http::CONTENT_LENGTH) != metaData_.end()) {
        return atoll(metaData_.at(Http::CONTENT_LENGTH).c_str());
    }
    return -1;
}

const std::string &RequestMetaData::ContentType() const
{
    if (metaData_.find(Http::CONTENT_TYPE) != metaData_.end()) {
        return metaData_.at(Http::CONTENT_TYPE);
    }
    return gEmpty;
}

const std::string &RequestMetaData::ContentMd5() const
{
    if (metaData_.find(Http::CONTENT_MD5) != metaData_.end()) {
        return metaData_.at(Http::CONTENT_MD5);
    }
    return gEmpty;
}

uint64_t RequestMetaData::CRC64() const
{
    if (metaData_.find("x-oss-hash-crc64ecma") != metaData_.end()) {
        return std::strtoull(metaData_.at("x-oss-hash-crc64ecma").c_str(), nullptr, 10);
    }
    return 0ULL;
}

const std::string &RequestMetaData::ETag() const
{
    if (metaData_.find(Http::ETAG) != metaData_.end()) {
        return metaData_.at(Http::ETAG);
    }
    return gEmpty;
}

const std::string& RequestMetaData::VersionId() const
{
    if (metaData_.find("x-oss-version-id") != metaData_.end()) {
        return metaData_.at("x-oss-version-id");
    }
    return gEmpty;
}

void RequestMetaData::setContentLength(int64_t value)
{
    metaData_[Http::CONTENT_LENGTH] = std::to_string(value);
}

void RequestMetaData::setContentType(const std::string &value)
{
    metaData_[Http::CONTENT_TYPE] = value;
}

void RequestMetaData::setContentMd5(const std::string &value)
{
    metaData_[Http::CONTENT_MD5] = value;
}

void RequestMetaData::setCrc64(uint64_t value)
{
    metaData_["x-oss-hash-crc64ecma"] = std::to_string(value);
}

void RequestMetaData::setETag(const std::string &value)
{
    metaData_[Http::ETAG] = value;
}

void RequestMetaData::addHeader(const std::string &key, const std::string &value)
{
    metaData_[key] = value;
}

bool RequestMetaData::hasHeader(const std::string& key) const
{
    return (metaData_.find(key) != metaData_.end());
}

void RequestMetaData::removeHeader(const std::string& key)
{
    if (metaData_.find(key) != metaData_.end()) {
        metaData_.erase(key);
    }
}

MetaData &RequestMetaData::HttpMetaData()
{
    return metaData_;
}

const MetaData &RequestMetaData::HttpMetaData() const
{
    return metaData_;
}

HeaderCollection RequestMetaData::toHeaderCollection() const
{
    HeaderCollection headers;
    for (auto const&header : metaData_) {
        headers[header.first] = header.second;
    }
    return headers;
}
