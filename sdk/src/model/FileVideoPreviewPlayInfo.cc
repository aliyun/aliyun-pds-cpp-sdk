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


#include <alibabacloud/pds/model/FileVideoPreviewPlayInfo.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"

using namespace AlibabaCloud::PDS;

void FileVideoPreviewPlayInfoMeta::ParseFromJson(const Json::Value& jValue)
{
    duration_ = jValue["duration"].asFloat();
    width_ = jValue["width"].asInt64();
    height_ = jValue["height"].asInt64();
}


void FileLiveTranscodingTask::ParseFromJson(const Json::Value& jValue)
{
    templateID_ = jValue["template_id"].asString();
    status_ = jValue["status"].asString();
    url_ = jValue["url"].asString();
    keepOriginalResolution_ = jValue["keep_original_resolution"].asBool();
}

void FileVideoPreviewPlayInfo::ParseFromJson(const Json::Value& jValue)
{
    category_ = jValue["category"].asString();

    if (jValue.isMember("meta")) {
        Json::Value meta = jValue["meta"];
        meta_.ParseFromJson(meta);
    }

    for (uint32_t i = 0; i < jValue["live_transcoding_task_list"].size(); i++) {
        Json::Value task = jValue["live_transcoding_task_list"][i];

        FileLiveTranscodingTask item;
        item.ParseFromJson(task);
        fileLiveTranscodingTaskList_.push_back(item);
    }
}