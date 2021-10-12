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


#include <alibabacloud/pds/model/FileListUploadedPartsResult.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"

using namespace AlibabaCloud::PDS;

FileListUploadedPartsResult::FileListUploadedPartsResult():
        PdsResult()
{
}

FileListUploadedPartsResult::FileListUploadedPartsResult(
        const std::shared_ptr<std::iostream>& content)
{
    Json::Value root;
    Json::CharReaderBuilder rbuilder;
    std::string errMsg;
    if (!Json::parseFromStream(rbuilder, *content, &root, &errMsg))
    {
        return;
    }

    fileID_ = root["file_id"].asString();
    uploadID_ = root["upload_id"].asString();
    nextMarker_ = root["next_part_number_marker"].asString();

    for (uint32_t i = 0; i < root["uploaded_parts"].size(); i++) {
        Json::Value partValue = root["uploaded_parts"][i];

        int64_t partNumber = partValue["part_number"].asInt();
        int64_t partSize = partValue["part_size"].asInt64();
        std::string eTag = partValue["etag"].asString();

        Part p(partNumber, partSize, eTag);
        partList_.push_back(p);
    }
}
