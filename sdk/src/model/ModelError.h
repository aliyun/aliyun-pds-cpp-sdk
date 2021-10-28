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
#include <alibabacloud/pds/client/Error.h>

namespace AlibabaCloud
{
namespace PDS
{
	const char * GetModelErrorMsg(const int code);

    /*Error For Argument Check*/
    const int ARG_ERROR_BASE   = ERROR_CLIENT_BASE + 1000;
    const int ARG_ERROR_START  = ARG_ERROR_BASE;
    const int ARG_ERROR_END    = ARG_ERROR_START +  999;

    /*Default*/
    const int ARG_ERROR_DEFAULT     = ARG_ERROR_BASE + 0;

    /*iostream request body*/
    const int ARG_ERROR_REQUEST_BODY_NULLPTR    = ARG_ERROR_BASE + 1;
    const int ARG_ERROR_REQUEST_BODY_FAIL_STATE = ARG_ERROR_BASE + 2;
    const int ARG_ERROR_REQUEST_BODY_BAD_STATE  = ARG_ERROR_BASE + 3;

    /*MultipartUpload*/
    const int ARG_ERROR_MULTIPARTUPLOAD_PARTLIST_EMPTY   = ARG_ERROR_BASE + 4;
    const int ARG_ERROR_MULTIPARTUPLOAD_PARTSIZE_RANGE   = ARG_ERROR_BASE + 5;
    const int ARG_ERROR_MULTIPARTUPLOAD_PARTNUMBER_RANGE = ARG_ERROR_BASE + 6;

    /*Resumable Upload*/
    const int ARG_ERROR_OPEN_UPLOAD_FILE = ARG_ERROR_BASE + 7;
    const int ARG_ERROR_CHECK_PART_SIZE_LOWER = ARG_ERROR_BASE + 8;
    const int ARG_ERROR_CHECK_THREAD_NUM_LOWER = ARG_ERROR_BASE + 9;
    const int ARG_ERROR_CHECK_POINT_DIR_NONEXIST = ARG_ERROR_BASE + 10;
    const int ARG_ERROR_PARSE_UPLOAD_RECORD_FILE = ARG_ERROR_BASE + 11;
    const int ARG_ERROR_UPLOAD_FILE_MODIFIED = ARG_ERROR_BASE + 12;
    const int ARG_ERROR_UPLOAD_RECORD_INVALID = ARG_ERROR_BASE + 13;

    /*Resumable Download*/
    const int ARG_ERROR_DOWNLOAD_FILE_PATH_EMPTY = ARG_ERROR_BASE + 14;
    const int ARG_ERROR_DOWNLOAD_SOURCE_FILE_MODIFIED = ARG_ERROR_BASE + 15;
    const int ARG_ERROR_PARSE_DOWNLOAD_RECORD_FILE = ARG_ERROR_BASE + 16;
    const int ARG_ERROR_OPEN_DOWNLOAD_TEMP_FILE = ARG_ERROR_BASE + 17;

    /*Resumable for wstring path*/
    const int ARG_ERROR_PATH_NOT_SUPPORT_WSTRING_TYPE = ARG_ERROR_BASE + 18;
    const int ARG_ERROR_PATH_NOT_SAME_TYPE = ARG_ERROR_BASE + 19;

    /*Get Data*/
    const int ARG_ERROR_DATA_RANGE_INVALID = ARG_ERROR_BASE + 20;
}
}

