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


#include <alibabacloud/pds/model/DirListResult.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"

using namespace AlibabaCloud::PDS;

DirListResult::DirListResult():
        PdsResult()
{
}

DirListResult::DirListResult(
        const std::shared_ptr<std::iostream>& content)
{
    Json::Value root;
    Json::CharReaderBuilder rbuilder;
    std::string errMsg;
    if (!Json::parseFromStream(rbuilder, *content, &root, &errMsg))
    {
        return;
    }

    nextMarker_ = root["next_marker"].asString();
    for (uint32_t i = 0; i < root["items"].size(); i++) {
        Json::Value jValue = root["items"][i];

        FileItem item;
        item.ParseFromJson(jValue);
        /*
        item.Category = jValue["category"].asString();
        item.ContentHash = jValue["content_hash"].asString();
        item.ContentHashName = jValue["content_hash_name"].asString();
        item.ContentType = jValue["content_type"].asString();
        item.Crc64Hash = jValue["crc64_hash"].asString();
        item.CreatedAt = jValue["created_at"].asString();
        item.Description = jValue["description"].asString();
        item.DomainID = jValue["domain_id"].asString();
        item.DownloadUrl = jValue["download_url"].asString();
        item.DriveID = jValue["drive_id"].asString();
        item.EncryptMode = jValue["encrypt_mode"].asString();
        item.FileExtension = jValue["file_extension"].asString();
        item.FileID = jValue["file_id"].asString();
        item.Hidden = jValue["hidden"].asBool();
        item.Name = jValue["name"].asString();
        item.ParentFileID = jValue["parent_file_id"].asString();
        item.PunishFlag = jValue["punish_flag"].asInt64();
        item.Size = jValue["size"].asInt64();
        item.Starred = jValue["starred"].asBool();
        item.Status = jValue["status"].asString();
        item.Type = jValue["type"].asString();
        item.UpdatedAT = jValue["updated_at"].asString();
        item.Url = jValue["url"].asString();
        */

        fileItemList_.push_back(item);
    }
}

void DirListResult::PrintString()
{
    std::cout << "next_marker: " << nextMarker_ <<
        ", item number:" << fileItemList_.size() << std::endl;
}
