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
#include <alibabacloud/pds/model/ObjectMetaData.h>

namespace AlibabaCloud
{
namespace PDS
{
    class ALIBABACLOUD_PDS_EXPORT FileDownloadRequest : public PdsResumableBaseRequest
    {
    public:
        FileDownloadRequest(const std::string& driveID, const std::string& fileID,
            const std::string& filePath);
        FileDownloadRequest(const std::string& driveID, const std::string& fileID,
            const std::string& filePath, const std::string& checkpointDir,
            const uint64_t partSize, const uint32_t threadNum);
        FileDownloadRequest(const std::string& driveID, const std::string& fileID,
            const std::string& filePath, const std::string& checkpointDir);

        const std::string& FilePath() const { return filePath_; }
        const std::string& TempFilePath() const { return tempFilePath_; }

        FileDownloadRequest(const std::string& driveID, const std::string& fileID,
            const std::wstring& filePath);
        FileDownloadRequest(const std::string& driveID, const std::string& fileID,
            const std::wstring& filePath, const std::wstring& checkpointDir,
            const uint64_t partSize, const uint32_t threadNum);
        FileDownloadRequest(const std::string& driveID, const std::string& fileID,
            const std::wstring& filePath, const std::wstring& checkpointDir);

        const std::wstring& FilePathW() const { return filePathW_; }
        const std::wstring& TempFilePathW() const { return tempFilePathW_; }

        std::shared_ptr<std::iostream> Content() { return content_; }
        bool RangeIsSet() const{ return rangeIsSet_; }
        int64_t RangeStart() const { return range_[0]; }
        int64_t RangeEnd() const { return range_[1]; }
        const std::string& ModifiedSinceConstraint() const { return modifiedSince_; }
        const std::string& UnmodifiedSinceConstraint() const { return unmodifiedSince_; }
        const std::vector<std::string>& MatchingETagsConstraint() const { return matchingETags_; }
        const std::vector<std::string>& NonmatchingETagsConstraint() const { return nonmatchingETags_;}

        void setModifiedSinceConstraint(const std::string& gmt);
        void setUnmodifiedSinceConstraint(const std::string& gmt);
        void setMatchingETagConstraints(const std::vector<std::string>& match);
        void setNonmatchingETagConstraints(const std::vector<std::string>& match);

        const std::string& DriveID() const { return driveID_; }
        const std::string& FileID() const { return fileID_; }

    protected:
        virtual int validate() const;

    private:
        std::string driveID_;
        std::string fileID_;
        bool rangeIsSet_;
        int64_t range_[2];
        std::string modifiedSince_;
        std::string unmodifiedSince_;
        std::vector<std::string> matchingETags_;
        std::vector<std::string> nonmatchingETags_;

        std::string filePath_;
        std::string tempFilePath_;
        std::shared_ptr<std::iostream> content_;

        std::wstring filePathW_;
        std::wstring tempFilePathW_;
    };
}
}