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
#include <vector>
#include <memory>
#include <iostream>
#include <alibabacloud/pds/Export.h>
#include <alibabacloud/pds/PdsResult.h>
#include <alibabacloud/pds/model/Part.h>

namespace AlibabaCloud
{
namespace PDS
{
    class ALIBABACLOUD_PDS_EXPORT FileListUploadedPartsResult :public PdsResult
    {
    public:
        FileListUploadedPartsResult();
        FileListUploadedPartsResult(const std::shared_ptr<std::iostream>& content);
        const std::string& FileID() const { return fileID_; }
        const std::string& UploadID() const { return uploadID_; }
        const std::string& NextMarker() const { return nextMarker_; }
        const AlibabaCloud::PDS::PartList& PartList() const { return partList_; }
    private:
        std::string fileID_;
        std::string uploadID_;
        std::string nextMarker_;
        AlibabaCloud::PDS::PartList partList_;
    };
}
}
