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
#include <alibabacloud/pds/PdsResult.h>
#include <alibabacloud/pds/Types.h>
#include <alibabacloud/pds/model/FileItem.h>

namespace AlibabaCloud
{
namespace PDS
{
    class ALIBABACLOUD_PDS_EXPORT FileGetResult :public PdsResult
    {
    public:
        FileGetResult();
        FileGetResult(const std::shared_ptr<std::iostream>& content);
        const std::string& Category() const { return category_; }
        const std::string& ContentHash() const { return contentHash_; }
        const std::string& ContentHashName() const { return contentHashName_; }
        const std::string& ContentType() const { return contentType_; }
        const std::string& Crc64Hash() const { return crc64Hash_; }
        const std::string& CreatedAt() const { return createdAt_; }
        const std::string& Description() const { return description_; }
        const std::string& DomainID() const { return domainID_; }
        const std::string& DownloadUrl() const { return downloadUrl_; }
        const std::string& DriveID() const { return driveID_; }
        const std::string& EncryptMode() const { return encryptMode_; }
        const std::string& FileExtension() const { return fileExtension_; }
        const std::string& FileID() const { return fileID_; }
        bool Hidden() const{ return hidden_; }
        const std::string& Name() const { return name_; }
        const std::string& ParentFileID() const { return parentFileID_; }
        int64_t PunishFlag() const { return punishFlag_; }
        int64_t Size() const { return size_; }
        bool Starred() const { return starred_; }
        const std::string& Status() const { return status_; }
        const std::string& Thumbnail() const { return thumbnail_; }
        bool Trashed() const { return trashed_; }
        const std::string& Type() const { return type_; }
        const std::string& UpdatedAT() const { return updatedAT_; }
        const std::string& Url() const { return url_; }
        const std::string& UploadID() const { return uploadID_; }
        void PrintString();
    private:
        std::string category_;
        std::string contentHash_;
        std::string contentHashName_;
        std::string contentType_;
        std::string crc64Hash_;
        std::string createdAt_;
        std::string description_;
        std::string domainID_;
        std::string downloadUrl_;
        std::string driveID_;
        std::string encryptMode_;
        std::string fileExtension_;
        std::string fileID_;
        bool hidden_;
        std::string name_;
        std::string parentFileID_;
        int64_t punishFlag_;
        int64_t size_;
        bool starred_;
        std::string status_;
        std::string thumbnail_;
        bool trashed_;
        std::string type_;
        std::string updatedAT_;
        std::string url_;
        std::string uploadID_;
    };
}
}