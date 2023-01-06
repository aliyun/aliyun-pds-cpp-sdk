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

#include <memory>
#include <iostream>
#include <alibabacloud/pds/Global.h>
#include <alibabacloud/pds/Types.h>
#include <alibabacloud/pds/PdsError.h>
#include <alibabacloud/pds/ServiceResult.h>
#include <alibabacloud/pds/utils/Outcome.h>

#include <alibabacloud/pds/model/VoidResult.h>

#include <alibabacloud/pds/model/AsyncTaskGetRequest.h>
#include <alibabacloud/pds/model/AsyncTaskGetResult.h>

#include <alibabacloud/pds/model/DirCreateRequest.h>
#include <alibabacloud/pds/model/DirCreateResult.h>
#include <alibabacloud/pds/model/DirDeleteRequest.h>
#include <alibabacloud/pds/model/DirDeleteResult.h>
#include <alibabacloud/pds/model/DirListRequest.h>
#include <alibabacloud/pds/model/DirListResult.h>
#include <alibabacloud/pds/model/DirSearchRequest.h>
#include <alibabacloud/pds/model/DirSearchResult.h>
#include <alibabacloud/pds/model/DirRenameRequest.h>
#include <alibabacloud/pds/model/DirRenameResult.h>
#include <alibabacloud/pds/model/DirTrashRequest.h>
#include <alibabacloud/pds/model/DirTrashResult.h>
#include <alibabacloud/pds/model/DirCopyRequest.h>
#include <alibabacloud/pds/model/DirCopyResult.h>
#include <alibabacloud/pds/model/DirMoveRequest.h>
#include <alibabacloud/pds/model/DirMoveResult.h>
#include <alibabacloud/pds/model/DirHiddenRequest.h>
#include <alibabacloud/pds/model/DirHiddenResult.h>

#include <alibabacloud/pds/model/FileCreateRequest.h>
#include <alibabacloud/pds/model/FileCreateResult.h>
#include <alibabacloud/pds/model/FileGetUploadUrlRequest.h>
#include <alibabacloud/pds/model/FileGetUploadUrlResult.h>
#include <alibabacloud/pds/model/FileListUploadedPartsRequest.h>
#include <alibabacloud/pds/model/FileListUploadedPartsResult.h>
#include <alibabacloud/pds/model/FileCompleteRequest.h>
#include <alibabacloud/pds/model/FileCompleteResult.h>
#include <alibabacloud/pds/model/FileGetDownloadUrlRequest.h>
#include <alibabacloud/pds/model/FileGetDownloadUrlResult.h>
#include <alibabacloud/pds/model/FileGetRequest.h>
#include <alibabacloud/pds/model/FileGetResult.h>
#include <alibabacloud/pds/model/FileDeleteRequest.h>
#include <alibabacloud/pds/model/FileDeleteResult.h>
#include <alibabacloud/pds/model/FileRenameRequest.h>
#include <alibabacloud/pds/model/FileRenameResult.h>
#include <alibabacloud/pds/model/FileTrashRequest.h>
#include <alibabacloud/pds/model/FileTrashResult.h>
#include <alibabacloud/pds/model/FileUploadRequest.h>
#include <alibabacloud/pds/model/FileDownloadRequest.h>
#include <alibabacloud/pds/model/FileGetVideoPreviewPlayInfoRequest.h>
#include <alibabacloud/pds/model/FileGetVideoPreviewPlayInfoResult.h>
#include <alibabacloud/pds/model/FileCopyRequest.h>
#include <alibabacloud/pds/model/FileCopyResult.h>
#include <alibabacloud/pds/model/FileMoveRequest.h>
#include <alibabacloud/pds/model/FileMoveResult.h>
#include <alibabacloud/pds/model/FileHiddenRequest.h>
#include <alibabacloud/pds/model/FileHiddenResult.h>

#include <alibabacloud/pds/model/DataGetByUrlRequest.h>
#include <alibabacloud/pds/model/DataGetResult.h>
#include <alibabacloud/pds/model/DataPutByUrlRequest.h>
#include <alibabacloud/pds/model/DataPutResult.h>

#include <alibabacloud/pds/model/MetaUserTagsDeleteRequest.h>
#include <alibabacloud/pds/model/MetaUserTagsPutRequest.h>
#include <alibabacloud/pds/model/MetaUserTagsPutResult.h>

#include <alibabacloud/pds/model/RequestMetaData.h>


namespace AlibabaCloud
{
namespace PDS
{
    using PdsOutcome = Outcome<PdsError, ServiceResult>;
    using VoidOutcome = Outcome<PdsError, VoidResult>;
    using StringOutcome = Outcome<PdsError, std::string>;

    using AsyncTaskGetOutcome = Outcome<PdsError, AsyncTaskGetResult>;


    using DirCreateOutcome = Outcome<PdsError, DirCreateResult>;
    using DirListOutcome = Outcome<PdsError, DirListResult>;
    using DirSearchOutcome = Outcome<PdsError, DirSearchResult>;
    using DirRenameOutcome = Outcome<PdsError, DirRenameResult>;
    using DirTrashOutcome = Outcome<PdsError, DirTrashResult>;
    using DirDeleteOutcome = Outcome<PdsError, DirDeleteResult>;
    using DirCopyOutcome = Outcome<PdsError, DirCopyResult>;
    using DirMoveOutcome = Outcome<PdsError, DirMoveResult>;
    using DirHiddenOutcome = Outcome<PdsError, DirHiddenResult>;

    using FileCreateOutcome = Outcome<PdsError, FileCreateResult>;
    using FileGetUploadUrlOutcome = Outcome<PdsError, FileGetUploadUrlResult>;
    using FileListUploadedPartsOutcome = Outcome<PdsError, FileListUploadedPartsResult>;
    using FileCompleteOutcome = Outcome<PdsError, FileCompleteResult>;
    using FileGetDownloadUrlOutcome = Outcome<PdsError, FileGetDownloadUrlResult>;
    using FileGetOutcome = Outcome<PdsError, FileGetResult>;
    using FileRenameOutcome = Outcome<PdsError, FileRenameResult>;
    using FileTrashOutcome = Outcome<PdsError, FileTrashResult>;
    using FileDeleteOutcome = Outcome<PdsError, FileDeleteResult>;
    using FileGetVideoPreviewPlayInfoOutcome = Outcome<PdsError, FileGetVideoPreviewPlayInfoResult>;
    using FileCopyOutcome = Outcome<PdsError, FileCopyResult>;
    using FileMoveOutcome = Outcome<PdsError, FileMoveResult>;
    using FileHiddenOutcome = Outcome<PdsError, FileHiddenResult>;

    using DataGetOutcome = Outcome<PdsError, DataGetResult>;
    using DataPutOutcome = Outcome<PdsError, DataPutResult>;

    using MetaUserTagsPutOutcome = Outcome<PdsError, MetaUserTagsPutResult>;

    using RequestMetaDataOutcome = Outcome<PdsError, RequestMetaData>;
}
}
