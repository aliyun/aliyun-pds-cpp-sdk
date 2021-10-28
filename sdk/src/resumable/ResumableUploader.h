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
#include "ResumableBaseWorker.h"
#include "../external/json/json.h"

namespace AlibabaCloud
{
namespace PDS
{

    struct UploadPartRecord {
        int64_t partNumber;
        int64_t offset;
        int64_t size;
        uint64_t crc64;
    };
    typedef std::vector<UploadPartRecord> UploadPartRecordList;
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
        UploadPartRecordList parts;
        std::string md5Sum;
    };

    class ResumableUploader : public ResumableBaseWorker
    {
    public:
        ResumableUploader(const FileUploadRequest& request, const PdsClientImpl *client);

        FileCompleteOutcome Upload();

    protected:
        virtual FileCreateOutcome FileCreateWrap(const FileCreateRequest &request) const;
        virtual FileGetUploadUrlOutcome FileGetUploadUrlWrap(const FileGetUploadUrlRequest &request) const;
        virtual DataPutOutcome UploadPartWrap(const DataPutByUrlRequest &request) const;
        virtual FileListUploadedPartsOutcome ListUploadedPartsWrap(const FileListUploadedPartsRequest &request) const;
        virtual FileCompleteOutcome FileCompleteWrap(const FileCompleteRequest &request) const;
        int validateRecord();
        void initRecordInfo();

    private:
        int validate(FileCompleteOutcome& outcome);
        int prepare(FileCompleteOutcome& outcome);
        int getPartsToUpload(PdsError &err, UploadPartRecordList &partsUploaded, UploadPartRecordList &partsToUpload);
        void genRecordPath();
        int loadRecord();
        static void UploadPartProcessCallback(size_t increment, int64_t transfered, int64_t total, void *userData);
        static int32_t UploadPartProcessControlCallback(void *userData);

        const FileUploadRequest& request_;
        UploadRecord record_;
        const PdsClientImpl *client_;
    };
}
}
