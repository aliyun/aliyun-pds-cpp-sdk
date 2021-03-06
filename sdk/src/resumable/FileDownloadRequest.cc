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


#include <alibabacloud/pds/model/FileDownloadRequest.h>
#include <alibabacloud/pds/Const.h>
#include <sstream>
#include <fstream>
#include "../model/ModelError.h"
#include "../utils/FileSystemUtils.h"

using namespace AlibabaCloud::PDS;

FileDownloadRequest::FileDownloadRequest(const std::string& driveID, const std::string& shareID, const std::string& fileID,
    const std::string &filePath, const std::string &checkpointDir,
    const uint64_t partSize, const uint32_t threadNum):
    PdsResumableBaseRequest(checkpointDir, partSize, threadNum),
    driveID_(driveID),
    shareID_(shareID),
    fileID_(fileID),
    filePath_(filePath)
{
    tempFilePath_ = filePath + ".temp";
}

FileDownloadRequest::FileDownloadRequest(const std::string& driveID, const std::string& shareID, const std::string& fileID,
    const std::string &filePath, const std::string &checkpointDir) :
    FileDownloadRequest(driveID, shareID, fileID, filePath, checkpointDir, DefaultPartSize, DefaultResumableThreadNum)
{}

FileDownloadRequest::FileDownloadRequest(const std::string& driveID, const std::string& shareID, const std::string& fileID,
    const std::string &filePath) :
    FileDownloadRequest(driveID, shareID, fileID, filePath, "", DefaultPartSize, DefaultResumableThreadNum)
{}

//wstring
FileDownloadRequest::FileDownloadRequest(const std::string& driveID, const std::string& shareID, const std::string& fileID,
    const std::wstring &filePath, const std::wstring &checkpointDir,
    const uint64_t partSize, const uint32_t threadNum) :
    PdsResumableBaseRequest(checkpointDir, partSize, threadNum),
    driveID_(driveID),
    shareID_(shareID),
    fileID_(fileID),
    filePathW_(filePath)
{
    tempFilePathW_ = filePath + L".temp";
}

FileDownloadRequest::FileDownloadRequest(const std::string &driveID, const std::string& shareID, const std::string &fileID,
    const std::wstring &filePath, const std::wstring &checkpointDir) :
    FileDownloadRequest(driveID, shareID, fileID, filePath, checkpointDir, DefaultPartSize, DefaultResumableThreadNum)
{}

FileDownloadRequest::FileDownloadRequest(const std::string &driveID, const std::string& shareID, const std::string &fileID,
    const std::wstring &filePath) :
    FileDownloadRequest(driveID, shareID, fileID, filePath, L"", DefaultPartSize, DefaultResumableThreadNum)
{}

void FileDownloadRequest::setShareToken(const std::string& shareToken)
{
    shareToken_ = shareToken;
}

int FileDownloadRequest::validate() const
{
    auto ret = PdsResumableBaseRequest::validate();
    if (ret != 0) {
        return ret;
    }

#if !defined(_WIN32)
    if (!filePathW_.empty()) {
        return ARG_ERROR_PATH_NOT_SUPPORT_WSTRING_TYPE;
    }
#endif

    if (filePath_.empty() && filePathW_.empty()) {
        return ARG_ERROR_DOWNLOAD_FILE_PATH_EMPTY;
    }

    //path and checkpoint must be same type.
    if ((!filePath_.empty() && !checkpointDirW_.empty()) ||
        (!filePathW_.empty() && !checkpointDir_.empty())) {
        return ARG_ERROR_PATH_NOT_SAME_TYPE;
    }

    //check tmpfilePath is available
    auto stream = GetFstreamByPath(tempFilePath_, tempFilePathW_, std::ios::out | std::ios::app);
    if (!stream->is_open()) {
        return ARG_ERROR_OPEN_DOWNLOAD_TEMP_FILE;
    }
    stream->close();

    return 0;
}
