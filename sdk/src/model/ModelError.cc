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

#include "ModelError.h"

using namespace AlibabaCloud::PDS;

static const char * GetArgErrorMsg(const int code)
{
    static const char * msg[] =
    {
        "Argument is invalid, please check.",
        /*iostream request body -1*/
        "Request body is null.",
        "Request body is in fail state. Logical error on i/o operation.",
        "Request body is in bad state. Read/writing error on i/o operation.",
        /*MultipartUpload -4*/
        "PartList is empty.",
        "PartSize should not be less than 100*1024 or greater than 5*1024*1024*1024.",
        "PartNumber should not be less than 1 or greater than 10000.",
        /*ResumableUpload -7*/
        "Open upload file failed.",
        "The part size is less than 100KB.",
        "The thread num is less than 1.",
        "Checkpoint directory is not exist.",
        "Parse resumable upload record failed.",
        "Upload file has been modified since last upload.",
        "Upload record is invalid, it has been modified.",
        /*ResumableDownload -14*/
        "The path of file download to is empty",
        "Source file has been modified since last download.",
        "Parse resumable download record failed.",
        "Open temp file for download failed",
        /*Resumable for wstring path -18*/
        "Only support wstring path in windows os.",
        "The type of filePath and checkpointDir should be the same, either string or wstring."
        /*Get Data -20*/
        "The range is invalid. The start should not be less than 0 or less then the end. The end could be -1 to get the rest of the data.",
    };

    int index = code - ARG_ERROR_START;
    int msg_size = sizeof(msg)/sizeof(msg[0]);
    if (code < ARG_ERROR_START || index > msg_size) {
        index = 0;
    }

    return msg[index];
}


const char * AlibabaCloud::PDS::GetModelErrorMsg(const int code)
{
    if (code >= ARG_ERROR_START && code <= ARG_ERROR_END) {
        return GetArgErrorMsg(code);
    }

    return "Model error, but undefined.";
}
