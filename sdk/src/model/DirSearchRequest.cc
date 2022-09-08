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


#include <alibabacloud/pds/model/DirSearchRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"
#include "ModelError.h"
#include <sstream>
using namespace AlibabaCloud::PDS;


DirSearchRequest::DirSearchRequest(const std::string& driveID,  const std::string& query,
    const std::string& orderBy, const std::string& marker, int64_t limit):
        driveID_(driveID),
        query_(query),
        orderBy_(orderBy),
        marker_(marker),
        limit_(limit),
        urlExpireSec_(0),
        returnTotalCount_(nullptr)
{
    setPath("/v2/file/search");
}

std::shared_ptr<std::iostream> DirSearchRequest::Body() const
{
    Json::Value root;
    root["drive_id"] = driveID_;
    root["query"] = query_;
    root["order_by"] = orderBy_;
    root["marker"] = marker_;
    root["limit"] = limit_;

    if (0 != urlExpireSec_) {
        root["url_expire_sec"] = urlExpireSec_;
    }

    if (nullptr != returnTotalCount_) {
        root["return_total_count"] = *returnTotalCount_;
    }

    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    std::shared_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    auto content = std::make_shared<std::stringstream>();
    writer->write(root, content.get());
    return content;
}

void DirSearchRequest::setUrlExpireSec(int64_t urlExpireSec)
{
    urlExpireSec_ = urlExpireSec;
}

void DirSearchRequest::setReturnTotalCount(bool* returnTotalCount)
{
    if (nullptr != returnTotalCount) {
        returnTotalCount_ = returnTotalCount;
    }
}

int DirSearchRequest::validate() const
{
    return 0;
}
