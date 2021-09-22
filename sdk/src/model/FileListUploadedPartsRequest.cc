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


#include <alibabacloud/pds/model/FileListUploadedPartsRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"
#include "ModelError.h"
#include <sstream>
using namespace AlibabaCloud::PDS;

FileListUploadedPartsRequest::FileListUploadedPartsRequest(const std::string& driveID, const std::string& fileID, const std::string& uploadID,
   int64_t marker, int64_t limit):
        driveID_(driveID),
        fileID_(fileID),
        uploadID_(uploadID),
        marker_(marker),
        limit_(limit)
{
    setPath("/v2/file/list_uploaded_parts");
}

std::string FileListUploadedPartsRequest::Path() const
{
    return path_;
}

void FileListUploadedPartsRequest::setMarker(int64_t marker)
{
    marker_ = marker;
}

std::shared_ptr<std::iostream> FileListUploadedPartsRequest::Body() const
{
    Json::Value root;
    root["drive_id"] = driveID_;
    root["file_id"] = fileID_;
    root["upload_id"] = uploadID_;
    root["part_number_marker"] = marker_;
    root["limit"] = limit_;

    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    std::shared_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    auto content = std::make_shared<std::stringstream>();
    writer->write(root, content.get());
    return content;
}

int FileListUploadedPartsRequest::validate() const
{
    return 0;
}