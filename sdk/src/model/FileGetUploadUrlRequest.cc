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


#include <alibabacloud/pds/model/FileGetUploadUrlRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"
#include "ModelError.h"
#include <sstream>
#include <alibabacloud/pds/Const.h>

using namespace AlibabaCloud::PDS;

FileGetUploadUrlRequest::FileGetUploadUrlRequest(const std::string& driveID, const std::string& fileID, const std::string& uploadID,
    const AlibabaCloud::PDS::PartInfoReqList& partInfoReqList):
        driveID_(driveID),
        fileID_(fileID),
        uploadID_(uploadID),
        partInfoReqList_(partInfoReqList)
{
    setPath("/v2/file/get_upload_url");
}

std::shared_ptr<std::iostream> FileGetUploadUrlRequest::Body() const
{
    Json::Value root;
    root["drive_id"] = driveID_;
    root["file_id"] = fileID_;
    root["upload_id"] = uploadID_;

    int index = 0;
    for (const PartInfoReq& part : partInfoReqList_) {
        root["part_info_list"][index]["part_number"] = part.PartNumber();
        root["part_info_list"][index]["part_size"] = part.PartSize();
        root["part_info_list"][index]["from"] = part.From();
        root["part_info_list"][index]["to"] = part.To();
        index++;
    }

    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    std::shared_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    auto content = std::make_shared<std::stringstream>();
    writer->write(root, content.get());
    return content;
}

void FileGetUploadUrlRequest::setTrafficLimit(uint64_t value)
{
    trafficLimit_ = value;
}

void FileGetUploadUrlRequest::setUserAgent(const std::string& ua)
{
    userAgent_ = ua;
}

int FileGetUploadUrlRequest::validate() const
{
    if(partInfoReqList_.empty())
        return ARG_ERROR_MULTIPARTUPLOAD_PARTLIST_EMPTY;

    for (const PartInfoReq& part : partInfoReqList_) {
        if(!(part.PartNumber() > 0 && part.PartNumber() < PartNumberUpperLimit)){
            return ARG_ERROR_MULTIPARTUPLOAD_PARTNUMBER_RANGE;
        }
    }
    return 0;
}
