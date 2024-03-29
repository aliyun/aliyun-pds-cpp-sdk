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
#include <alibabacloud/pds/model/FileItem.h>

namespace AlibabaCloud
{
namespace PDS
{
    using FileItemList = std::vector<FileItem>;
    class ALIBABACLOUD_PDS_EXPORT DirSearchResult :public PdsResult
    {
    public:
        DirSearchResult();
        DirSearchResult(const std::shared_ptr<std::iostream>& content);
        const AlibabaCloud::PDS::FileItemList& FileItemList() const { return fileItemList_; }
        const std::string& NextMarker() const { return nextMarker_; }
        int64_t TotalCount() const { return totalCount_; }
    private:
        AlibabaCloud::PDS::FileItemList fileItemList_;
        std::string nextMarker_;
        int64_t totalCount_;
    };
}
}