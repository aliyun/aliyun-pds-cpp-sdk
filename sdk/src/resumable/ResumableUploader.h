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
#include "ResumableBaseWorker.h"
#include "../external/json/json.h"

namespace AlibabaCloud
{
namespace PDS
{
    struct UploadRecord{
        std::string opType;
        std::string driveID;
        std::string fileID;
        std::string uploadID;
        std::string name;
        std::string filePath;
        std::string mtime;
        uint64_t size;
        uint64_t partSize;
        std::string md5Sum;
        std::string sha1Sum;
    };

    class ResumableUploader : public ResumableBaseWorker
    {
    public:
        ResumableUploader(const FileUploadRequest& request, const PdsClientImpl *client);

        FileCompleteOutcome Upload();

    protected:
        virtual FileCreateOutcome FileCreateWrap(const FileCreateRequest &request) const;
        virtual FileGetUploadUrlOutcome FileGetUploadUrlWrap(const FileGetUploadUrlRequest &request) const;
        virtual PutObjectOutcome UploadPartWrap(const PutObjectByUrlRequest &request) const;
        virtual FileListUploadedPartsOutcome ListUploadedPartsWrap(const FileListUploadedPartsRequest &request) const;
        virtual FileCompleteOutcome FileCompleteWrap(const FileCompleteRequest &request) const;

        virtual void initRecordInfo();
        virtual void buildRecordInfo(const AlibabaCloud::PDS::Json::Value& value);
        virtual void dumpRecordInfo(AlibabaCloud::PDS::Json::Value& value);
        virtual int validateRecord();

    private:
        int getPartsToUpload(PdsError &err, PartList &partsUploaded, PartList &partsToUpload);
        virtual void genRecordPath();
        virtual int loadRecord();
        virtual int prepare(PdsError& err);

        const FileUploadRequest& request_;
        UploadRecord record_;
        const PdsClientImpl *client_;
        std::string driveID_;
        std::string fileID_;
        std::string uploadID_;
        static void UploadPartProcessCallback(size_t increment, int64_t transfered, int64_t total, void *userData);
    };
}
}
