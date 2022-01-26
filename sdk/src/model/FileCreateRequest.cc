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


#include <alibabacloud/pds/model/FileCreateRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"
#include "ModelError.h"
#include <sstream>
#include <alibabacloud/pds/Const.h>

using namespace AlibabaCloud::PDS;

FileCreateRequest::FileCreateRequest(const std::string& driveID, const std::string& parentFileID, const std::string& name,
    const std::string& fileID, const std::string& checkNameMode, const int64_t size):
        driveID_(driveID),
        parentFileID_(parentFileID),
        name_(name),
        fileID_(fileID),
        checkNameMode_(checkNameMode),
        size_(size),
        type_("file"),
        hidden_(false)
{
    setPath("/v2/file/create");
}

std::shared_ptr<std::iostream> FileCreateRequest::Body() const
{
    Json::Value root;
    root["drive_id"] = driveID_;
    root["parent_file_id"] = parentFileID_;
    root["name"] = name_;
    root["check_name_mode"] = checkNameMode_;
    root["size"] = size_;
    root["type"] = type_;
    root["hidden"] = hidden_;
    if (!fileID_.empty()) {
        root["file_id"] = fileID_;
    }
    if (!preHash_.empty()) {
        root["pre_hash"] = preHash_;
    }
    if (!contentHash_.empty()) {
        root["content_hash_name"] = "sha1";
        root["content_hash"] = contentHash_;
    }

    int index = 0;
    for (const PartInfoReq& part : partInfoReqList_) {
        root["part_info_list"][index]["part_number"] = part.PartNumber();
        root["part_info_list"][index]["part_size"] = part.PartSize();
        root["part_info_list"][index]["from"] = part.From();
        root["part_info_list"][index]["to"] = part.To();
        index++;
    }

    index = 0;
    for (const UserTag& userTag : userTags_) {
        root["user_tags"][index]["key"] = userTag.Key();
        root["user_tags"][index]["value"] = userTag.Value();
        index++;
    }

    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    std::shared_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    auto content = std::make_shared<std::stringstream>();
    writer->write(root, content.get());
    return content;
}

void FileCreateRequest::setPartInfoList(const AlibabaCloud::PDS::PartInfoReqList& partInfoReqList)
{
    partInfoReqList_ = partInfoReqList;
}

void FileCreateRequest::setUserTags(const AlibabaCloud::PDS::UserTagList& userTags)
{
    userTags_ = userTags;
}

void FileCreateRequest::setPreHash(const std::string& preHash)
{
    preHash_ = preHash;
}

void FileCreateRequest::setContentHash(const std::string& contentHash)
{
    contentHash_ = contentHash;
}

void FileCreateRequest::setHidden(bool hidden)
{
    hidden_ = hidden;
}

int FileCreateRequest::validate() const
{
    for (const PartInfoReq& part : partInfoReqList_) {
        if(!(part.PartNumber() > 0 && part.PartNumber() < PartNumberUpperLimit)){
            return ARG_ERROR_MULTIPARTUPLOAD_PARTNUMBER_RANGE;
        }
    }
    return 0;
}
