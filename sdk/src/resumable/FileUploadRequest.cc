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

#include <alibabacloud/pds/model/FileUploadRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include <alibabacloud/pds/Const.h>
#include <fstream>
#include "../utils/Utils.h"
#include "../utils/FileSystemUtils.h"
#include "../model/ModelError.h"

using namespace AlibabaCloud::PDS;

FileUploadRequest::FileUploadRequest(const std::string& driveID, const std::string& parentFileID,
    const std::string& name, const std::string& fileID, const std::string& checkNameMode,
    const std::string &filePath, const std::string &checkpointDir,
    const uint64_t partSize, const uint32_t threadNum):
    PdsResumableBaseRequest(checkpointDir, partSize, threadNum),
    driveID_(driveID),
    parentFileID_(parentFileID),
    name_(name),
    fileID_(fileID),
    checkNameMode_(checkNameMode),
    filePath_(filePath)
{
    time_t lastMtime;
    std::streamsize fileSize;
    isFileExist_ = true;
    if (!GetPathInfo(filePath_, lastMtime, fileSize)) {
        //if fail, ignore the lastmodified time.
        lastMtime = 0;
        fileSize = 0;
        isFileExist_ = false;
    }
    mtime_ = ToGmtTime(lastMtime);
    objectSize_ = static_cast<uint64_t>(fileSize);
}

FileUploadRequest::FileUploadRequest(const std::string& driveID, const std::string& parentFileID,
    const std::string& name, const std::string& fileID, const std::string& checkNameMode,
    const std::string &filePath, const std::string &checkpointDir) :
    FileUploadRequest(driveID, parentFileID, name, fileID, checkNameMode, filePath, checkpointDir, DefaultPartSize, DefaultResumableThreadNum)
{}

FileUploadRequest::FileUploadRequest(const std::string& driveID, const std::string& parentFileID,
    const std::string& name, const std::string& fileID, const std::string& checkNameMode,
    const std::string &filePath):
    FileUploadRequest(driveID, parentFileID, name, fileID, checkNameMode, filePath, "", DefaultPartSize, DefaultResumableThreadNum)
{}

//wstring
FileUploadRequest::FileUploadRequest(const std::string& driveID, const std::string& parentFileID,
    const std::string& name, const std::string& fileID, const std::string& checkNameMode,
    const std::wstring &filePath, const std::wstring &checkpointDir,
    const uint64_t partSize, const uint32_t threadNum) :
    PdsResumableBaseRequest(checkpointDir, partSize, threadNum),
    driveID_(driveID),
    parentFileID_(parentFileID),
    name_(name),
    fileID_(fileID),
    checkNameMode_(checkNameMode),
    filePathW_(filePath)
{
#ifdef _WIN32
    time_t lastMtime;
    std::streamsize fileSize;
    isFileExist_ = true;
    if (!GetPathInfo(filePathW_, lastMtime, fileSize)) {
        //if fail, ignore the lastmodified time.
        lastMtime = 0;
        fileSize = 0;
        isFileExist_ = false;
    }
    mtime_ = ToGmtTime(lastMtime);
    objectSize_ = static_cast<uint64_t>(fileSize);
#else
    objectSize_ = 0;
    time_t lastMtime = 0;
    mtime_ = ToGmtTime(lastMtime);
    isFileExist_ = false;
#endif
}

FileUploadRequest::FileUploadRequest(const std::string& driveID, const std::string& parentFileID,
            const std::string& name, const std::string& fileID, const std::string& checkNameMode,
    const std::wstring &filePath, const std::wstring &checkpointDir) :
    FileUploadRequest(driveID, parentFileID, name, fileID, checkNameMode, filePath, checkpointDir, DefaultPartSize, DefaultResumableThreadNum)
{}

FileUploadRequest::FileUploadRequest(const std::string& driveID, const std::string& parentFileID,
            const std::string& name, const std::string& fileID, const std::string& checkNameMode,
    const std::wstring &filePath) :
    FileUploadRequest(driveID, parentFileID, name, fileID, checkNameMode, filePath, L"", DefaultPartSize, DefaultResumableThreadNum)
{}

int FileUploadRequest::validate() const
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

    //path and checkpoint must be same type.
    if ((!filePath_.empty() && !checkpointDirW_.empty()) ||
        (!filePathW_.empty() && !checkpointDir_.empty())) {
        return ARG_ERROR_PATH_NOT_SAME_TYPE;
    }

    if (!isFileExist_) {
        return ARG_ERROR_OPEN_UPLOAD_FILE;
    }

    return 0;
}
