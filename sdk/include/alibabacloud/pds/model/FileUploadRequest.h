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

#pragma once
#include <alibabacloud/pds/Export.h>
#include <alibabacloud/pds/PdsRequest.h>
#include <alibabacloud/pds/Types.h>
#include <alibabacloud/pds/http/HttpType.h>

namespace AlibabaCloud
{
namespace PDS
{
    class ALIBABACLOUD_PDS_EXPORT FileUploadRequest : public PdsResumableBaseRequest
    {
    public:
        FileUploadRequest(const std::string& driveID, const std::string& parentFileID,
            const std::string& name, const std::string& fileID, const std::string& checkNameMode,
            const std::string& filePath, const std::string& checkpointDir,
            const uint64_t partSize, const uint32_t threadNum);
        FileUploadRequest(const std::string& driveID, const std::string& parentFileID,
            const std::string& name, const std::string& fileID, const std::string& checkNameMode,
            const std::string& filePath, const std::string& checkpointDir);
        FileUploadRequest(const std::string& driveID, const std::string& parentFileID,
            const std::string& name, const std::string& fileID, const std::string& checkNameMode,
            const std::string& filePath);

        std::shared_ptr<std::iostream> Content(){ return content_; }
        const std::string& FilePath() const{return filePath_;}

        FileUploadRequest(const std::string& driveID, const std::string& parentFileID,
            const std::string& name, const std::string& fileID, const std::string& checkNameMode,
            const std::wstring& filePath, const std::wstring& checkpointDir,
            const uint64_t partSize, const uint32_t threadNum);
        FileUploadRequest(const std::string& driveID, const std::string& parentFileID,
            const std::string& name, const std::string& fileID, const std::string& checkNameMode,
            const std::wstring& filePath, const std::wstring& checkpointDir);
        FileUploadRequest(const std::string& driveID, const std::string& parentFileID,
            const std::string& name, const std::string& fileID, const std::string& checkNameMode,
            const std::wstring& filePath);
        const std::wstring& FilePathW() const { return filePathW_; }

        const std::string& DriveID() const { return driveID_; }
        const std::string& ParentFileID() const { return parentFileID_; }
        const std::string& Name() const { return name_; }
        const std::string& FileID() const { return fileID_; }
        const std::string& CheckNameMode() const { return checkNameMode_; }

    protected:
        virtual int validate() const;
    private:
        std::string driveID_;
        std::string parentFileID_;
        std::string name_;
        std::string fileID_;
        std::string checkNameMode_;
        std::string filePath_;
        std::shared_ptr<std::iostream> content_;
        std::wstring filePathW_;
        bool isFileExist_;
    };
}
}