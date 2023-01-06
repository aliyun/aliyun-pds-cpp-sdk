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


#include <alibabacloud/pds/model/FileCopyRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"
#include "ModelError.h"
#include <sstream>
using namespace AlibabaCloud::PDS;

FileCopyRequest::FileCopyRequest(const std::string& driveID, const std::string& fileID, const std::string& toParentFileID,
    bool autoRename):
        driveID_(driveID),
        fileID_(fileID),
        toParentFileID_(toParentFileID),
        autoRename_(autoRename)
{
    setPath("/v2/file/copy");
}

std::shared_ptr<std::iostream> FileCopyRequest::Body() const
{
    Json::Value root;
    root["drive_id"] = driveID_;
    root["file_id"] = fileID_;
    root["to_parent_file_id"] = toParentFileID_;
    root["auto_rename"] = autoRename_;

    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    std::shared_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    auto content = std::make_shared<std::stringstream>();
    writer->write(root, content.get());
    return content;
}

int FileCopyRequest::validate() const
{
    return 0;
}
