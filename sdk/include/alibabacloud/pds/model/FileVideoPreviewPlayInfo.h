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

#pragma once
#include <alibabacloud/pds/Export.h>
#include <alibabacloud/pds/PdsResult.h>
#include <alibabacloud/pds/Types.h>
#include "../external/json/json.h"

namespace AlibabaCloud
{
namespace PDS
{
    class FileVideoPreviewPlayInfoMeta
    {
    public:
        FileVideoPreviewPlayInfoMeta() = default;
        void ParseFromJson(const Json::Value& jValue);
        float Duration() const { return duration_; }
        int64_t Width() const { return width_; }
        int64_t Height() const { return height_; }
    private:
        float duration_;
        int64_t width_;
        int64_t height_;
    };

    class FileLiveTranscodingTask
    {
    public:
        FileLiveTranscodingTask() = default;
        void ParseFromJson(const Json::Value& jValue);
        const std::string& TemplateID() const { return templateID_; }
        const std::string& Status() const { return status_; }
        const std::string& Url() const { return url_; }
        bool KeepOriginalResolution() const { return keepOriginalResolution_; }
    private:
        std::string templateID_;
        std::string status_;
        std::string url_;
        bool keepOriginalResolution_;
    };

    using FileLiveTranscodingTaskList = std::vector<FileLiveTranscodingTask>;

    class FileVideoPreviewPlayInfo
    {
    public:
        FileVideoPreviewPlayInfo() = default;
        void ParseFromJson(const Json::Value& jValue);
        const std::string& Category() const { return category_; }
        const AlibabaCloud::PDS::FileVideoPreviewPlayInfoMeta& FileVideoPreviewPlayInfoMeta() const { return meta_; }
        const AlibabaCloud::PDS::FileLiveTranscodingTaskList& FileLiveTranscodingTaskList() const { return fileLiveTranscodingTaskList_; }
    private:
        std::string category_;
        AlibabaCloud::PDS::FileVideoPreviewPlayInfoMeta meta_;
        AlibabaCloud::PDS::FileLiveTranscodingTaskList fileLiveTranscodingTaskList_;
    };
}
}