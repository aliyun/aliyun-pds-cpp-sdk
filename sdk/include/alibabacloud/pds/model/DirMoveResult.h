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

namespace AlibabaCloud
{
namespace PDS
{
    class ALIBABACLOUD_PDS_EXPORT DirMoveResult :public PdsResult
    {
    public:
        DirMoveResult();
        DirMoveResult(const std::shared_ptr<std::iostream>& content);
        const std::string& DriveID() const { return driveID_; }
        const std::string& FileID() const { return fileID_; }
        const std::string& AsyncTaskID() const { return asyncTaskID_; }
        bool Exist() const { return exist_; }
    private:
        std::string driveID_;
        std::string fileID_;
        std::string asyncTaskID_;
        bool exist_;
    };
}
}