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


#include <alibabacloud/pds/model/FileGetDownloadUrlResult.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"

using namespace AlibabaCloud::PDS;

FileGetDownloadUrlResult::FileGetDownloadUrlResult():
        PdsResult()
{
}

FileGetDownloadUrlResult::FileGetDownloadUrlResult(
        const std::shared_ptr<std::iostream>& content)
{
    Json::Value root;
    Json::CharReaderBuilder rbuilder;
    std::string errMsg;
    if (!Json::parseFromStream(rbuilder, *content, &root, &errMsg))
    {
        return;
    }

    cdnUrl_ = root["cdn_url"].asString();
    contentHash_ = root["content_hash"].asString();
    contentHashName_ = root["content_hash_name"].asString();
    crc64Hash_ = root["crc64_hash"].asString();
    expiration_ = root["expiration"].asString();
    internalUrl_ = root["internal_url"].asString();
    method_ = root["method"].asString();
    size_ = root["size"].asInt64();
    url_ = root["url"].asString();
    punishFlag_ = root["punish_flag"].asInt64();
}
