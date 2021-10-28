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


#include <alibabacloud/pds/model/DataPutResult.h>
#include "../utils/Utils.h"
#include <alibabacloud/pds/http/HttpType.h>
using namespace AlibabaCloud::PDS;

DataPutResult::DataPutResult():
    PdsDataResult(),
    content_(nullptr)
{
}

DataPutResult::DataPutResult(const HeaderCollection& header, const std::shared_ptr<std::iostream>& content):
    PdsDataResult(header)
{
    if (header.find(Http::ETAG) != header.end())
    {
        eTag_ = TrimQuotes(header.at(Http::ETAG).c_str());
    }

    if (header.find("x-oss-hash-crc64ecma") != header.end()) {
        crc64_ = std::strtoull(header.at("x-oss-hash-crc64ecma").c_str(), nullptr, 10);
    }

    if (content != nullptr && content->peek() != EOF) {
        content_ = content;
    }
}

DataPutResult::DataPutResult(const HeaderCollection & header):
    DataPutResult(header, nullptr)
{
}

const std::string& DataPutResult::ETag() const
{
    return eTag_;
}

uint64_t DataPutResult::CRC64()
{
    return crc64_;
}

const std::shared_ptr<std::iostream>& DataPutResult::Content() const
{
    return content_;
}
