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


#include <alibabacloud/pds/model/FileGetResult.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"

using namespace AlibabaCloud::PDS;

FileGetResult::FileGetResult():
        PdsResult()
{
}

FileGetResult::FileGetResult(
        const std::shared_ptr<std::iostream>& content)
{
    Json::Value root;
    Json::CharReaderBuilder rbuilder;
    std::string errMsg;
    if (!Json::parseFromStream(rbuilder, *content, &root, &errMsg))
    {
        return;
    }

    category_ = root["category"].asString();
    contentHash_ = root["content_hash"].asString();
    contentHashName_ = root["content_hash_name"].asString();
    contentType_ = root["content_type"].asString();
    crc64Hash_ = root["crc64_hash"].asString();
    createdAt_ = root["created_at"].asString();
    description_ = root["description"].asString();
    downloadUrl_ = root["download_url"].asString();
    driveID_ = root["drive_id"].asString();
    encryptMode_ = root["encrypt_mode"].asString();
    fileExtension_ = root["file_extension"].asString();
    fileID_ = root["file_id"].asString();
    hidden_ = root["hidden"].asBool();
    name_ = root["name"].asString();
    parentFileID_ = root["parent_file_id"].asString();
    punishFlag_ = root["punish_flag"].asInt64();
    size_ = root["size"].asInt64();
    starred_ = root["starred"].asBool();
    status_ = root["status"].asString();
    thumbnail_ = root["thumbnail"].asString();
    trashed_ = root["trashed"].asBool();
    type_ = root["type"].asString();
    updatedAT_ = root["updated_at"].asString();
    url_ = root["url"].asString();
    uploadID_ = root["upload_id"].asString();

    if (root.isMember("user_tags")) {
        Json::Value tags = root["user_tags"];
        Json::Value::Members members;
        members = tags.getMemberNames();
        for (Json::Value::Members::iterator iter = members.begin(); iter != members.end(); iter++) {
            std::string key = *iter;
            if (tags[key.c_str()].isString()) {
                std::string value = tags[key.c_str()].asString();
                userTags_[key] = value;
            }
        }
    }
}
