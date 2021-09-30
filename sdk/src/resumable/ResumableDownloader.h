#pragma once
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

namespace AlibabaCloud
{
namespace PDS
{
    struct PartRecord {
        int64_t partNumber;
        int64_t offset;
        int64_t size;
        uint64_t crc64;
    };
    typedef std::vector<PartRecord> PartRecordList;
    struct DownloadRecord {
        std::string opType;
        std::string driveID;
        std::string shareID;
        std::string fileID;
        std::string contentHash;
        std::string crc64Hash;
        std::string filePath;
        std::string mtime;
        uint64_t size;
        uint64_t partSize;
        PartRecordList parts;
        std::string md5Sum;
    };


    class ResumableDownloader : public ResumableBaseWorker
    {
    public:
        ResumableDownloader(const FileDownloadRequest& request, const PdsClientImpl *client, uint64_t fileSize,
            std::string contentHash, std::string crc64Hash, std::string url):
            ResumableBaseWorker(fileSize, request.PartSize()),
            request_(request),
            client_(client),
            contentHash_(contentHash),
            crc64Hash_(crc64Hash),
            url_(url)
        {
        }

        DataGetOutcome Download();

    protected:
        void genRecordPath();
        int loadRecord();
        int validateRecord();
        int prepare(PdsError& err);
        void initRecord();
        int getPartsToDownload(PdsError &err, PartRecordList &partsToDownload);
        bool renameTempFile();
        void removeTempFile();
        static void DownloadPartProcessCallback(size_t increment, int64_t transfered, int64_t total, void *userData);
        static int32_t DownloadPartProcessControlCallback(void *userData);

        virtual FileGetOutcome FileGetWrap(const FileGetRequest &request) const;
        virtual DataGetOutcome DataGetByUrlWrap(const DataGetByUrlRequest &request) const;

        const FileDownloadRequest request_;
        DownloadRecord record_;
        const PdsClientImpl *client_;
        std::string contentHash_;
        std::string crc64Hash_;
        std::string url_;
    };
}
}
