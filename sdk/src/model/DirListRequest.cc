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


#include <alibabacloud/pds/model/DirListRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"
#include "ModelError.h"
#include <sstream>
using namespace AlibabaCloud::PDS;


DirListRequest::DirListRequest(const std::string& driveID,  const std::string& parentFileID,
    const std::string& orderBy, const std::string& orderDirection,
    const std::string& fields, const std::string& marker, int64_t limit):
    DirListRequest(driveID, "", parentFileID, orderBy, orderDirection, fields, marker, limit)
{}

DirListRequest::DirListRequest(const std::string& driveID, const std::string& shareID, const std::string& parentFileID,
    const std::string& orderBy, const std::string& orderDirection,
    const std::string& fields, const std::string& marker, int64_t limit):
        driveID_(driveID),
        shareID_(shareID),
        parentFileID_(parentFileID),
        orderBy_(orderBy),
        orderDirection_(orderDirection),
        marker_(marker),
        limit_(limit),
        fields_(fields),
        type_(),
        category_(),
        urlExpireSec_(0),
        starred_(nullptr),
        all_(nullptr)
{
    setPath("/v2/file/list");
}

std::shared_ptr<std::iostream> DirListRequest::Body() const
{
    Json::Value root;
    if (!driveID_.empty()) {
        root["drive_id"] = driveID_;
    }
    if (!shareID_.empty()) {
        root["share_id"] = shareID_;
    }
    root["parent_file_id"] = parentFileID_;
    root["order_by"] = orderBy_;
    root["order_direction"] = orderDirection_;
    root["marker"] = marker_;
    root["limit"] = limit_;
    root["fields"] = fields_;

    if (!type_.empty()) {
        root["type"] = type_;
    }

    if (!category_.empty()) {
        root["category"] = category_;
    }

    if (0 != urlExpireSec_) {
        root["url_expire_sec"] = urlExpireSec_;
    }

    if (nullptr != starred_) {
        root["starred"] = *starred_;
    }

    if (nullptr != all_) {
        root["all"] = *all_;
    }

    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    std::shared_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    auto content = std::make_shared<std::stringstream>();
    writer->write(root, content.get());
    return content;
}

void DirListRequest::setType(const std::string& type)
{
    type_ = type;
}

void DirListRequest::setCategory(const std::string& category)
{
    category_ = category;
}

void DirListRequest::setUrlExpireSec(int64_t urlExpireSec)
{
    urlExpireSec_ = urlExpireSec;
}

void DirListRequest::setStarred(bool* starred)
{
    if (nullptr != starred) {
        starred_ = starred;
    }
}

void DirListRequest::setAll(bool* all)
{
    if (nullptr != all) {
        all_ = all;
    }
}

void DirListRequest::setShareToken(const std::string& shareToken)
{
    shareToken_ = shareToken;
}

HeaderCollection DirListRequest::specialHeaders() const
{
    auto headers = PdsRequest::specialHeaders();
    if (!shareToken_.empty()) {
        headers["x-share-token"] = shareToken_;
    }
    return headers;
}

int DirListRequest::validate() const
{
    return 0;
}
