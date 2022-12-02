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


#include <alibabacloud/pds/model/FileGetByPathRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"
#include "ModelError.h"
#include <sstream>
using namespace AlibabaCloud::PDS;

FileGetByPathRequest::FileGetByPathRequest(const std::string& driveID, const std::string& filePath):
        driveID_(driveID),
        filePath_(filePath),
        urlExpireSec_(0)
{
    setPath("/v2/file/get_by_path");
}

std::shared_ptr<std::iostream> FileGetByPathRequest::Body() const
{
    Json::Value root;
    root["drive_id"] = driveID_;
    root["file_path"] = filePath_;

    if (0 != urlExpireSec_) {
        root["url_expire_sec"] = urlExpireSec_;
    }

    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    std::shared_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    auto content = std::make_shared<std::stringstream>();
    writer->write(root, content.get());
    return content;
}

int FileGetByPathRequest::validate() const
{
    return 0;
}

void FileGetByPathRequest::setUrlExpireSec(int64_t urlExpireSec)
{
    urlExpireSec_ = urlExpireSec;
}
