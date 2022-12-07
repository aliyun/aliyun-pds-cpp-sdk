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

#include <alibabacloud/pds/PdsClient.h>
#include "auth/SimpleCredentialsProvider.h"
#include "http/CurlHttpClient.h"
#include "PdsClientImpl.h"
#include <fstream>
#include "utils/LogUtils.h"
#include "utils/Crc64.h"

using namespace AlibabaCloud::PDS;

static bool SdkInitDone = false;

bool AlibabaCloud::PDS::IsSdkInitialized()
{
    return SdkInitDone;
}

void AlibabaCloud::PDS::InitializeSdk()
{
    if (IsSdkInitialized())
        return;
    InitLogInner();
    CurlHttpClient::initGlobalState();
    SdkInitDone = true;
}

void AlibabaCloud::PDS::ShutdownSdk()
{
    if (!IsSdkInitialized())
        return;

    CurlHttpClient::cleanupGlobalState();
    DeinitLogInner();
    SdkInitDone = false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////

void AlibabaCloud::PDS::SetLogLevel(LogLevel level)
{
    SetLogLevelInner(level);
}

void AlibabaCloud::PDS::SetLogCallback(LogCallback callback)
{
    SetLogCallbackInner(callback);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

PdsClient::PdsClient(const std::string &endpoint, const ClientConfiguration & configuration) :
    PdsClient(endpoint, "", configuration)
{
}

PdsClient::PdsClient(const std::string &endpoint, const std::string & accessToken, const ClientConfiguration & configuration) :
    PdsClient(endpoint, std::make_shared<SimpleCredentialsProvider>(accessToken), configuration)
{
}

PdsClient::PdsClient(const std::string &endpoint, const Credentials &credentials, const ClientConfiguration &configuration) :
    PdsClient(endpoint, std::make_shared<SimpleCredentialsProvider>(credentials), configuration)
{
}

PdsClient::PdsClient(const std::string &endpoint, const std::shared_ptr<CredentialsProvider>& credentialsProvider, const ClientConfiguration & configuration) :
    client_(std::make_shared<PdsClientImpl>(endpoint, credentialsProvider, configuration))
{
}

PdsClient::~PdsClient()
{
}


////////////////////
// AsyncTask
////////////////////
AsyncTaskGetOutcome PdsClient::AsyncTaskGet(const AsyncTaskGetRequest& request) const
{
    return client_->AsyncTaskGet(request);
}


////////////////////
// Dir
////////////////////
DirCreateOutcome PdsClient::DirCreate(const DirCreateRequest& request) const
{
    return client_->DirCreate(request);
}

DirListOutcome PdsClient::DirList(const DirListRequest& request) const
{
    return client_->DirList(request);
}

DirSearchOutcome PdsClient::DirSearch(const DirSearchRequest& request) const
{
    return client_->DirSearch(request);
}

DirRenameOutcome PdsClient::DirRename(const DirRenameRequest& request) const
{
    return client_->DirRename(request);
}

DirTrashOutcome PdsClient::DirTrash(const DirTrashRequest& request) const
{
    return client_->DirTrash(request);
}

DirDeleteOutcome PdsClient::DirDelete(const DirDeleteRequest& request) const
{
    return client_->DirDelete(request);
}


////////////////////
// File
////////////////////
FileCreateOutcome PdsClient::FileCreate(const FileCreateRequest& request) const
{
    return client_->FileCreate(request);
}

FileGetUploadUrlOutcome PdsClient::FileGetUploadUrl(const FileGetUploadUrlRequest& request) const
{
    return client_->FileGetUploadUrl(request);
}

FileListUploadedPartsOutcome PdsClient::FileListUploadedParts(const FileListUploadedPartsRequest& request) const
{
    return client_->FileListUploadedParts(request);
}

FileCompleteOutcome PdsClient::FileComplete(const FileCompleteRequest& request) const
{
    return client_->FileComplete(request);
}

FileGetDownloadUrlOutcome PdsClient::FileGetDownloadUrl(const FileGetDownloadUrlRequest& request) const
{
    return client_->FileGetDownloadUrl(request);
}

FileGetOutcome PdsClient::FileGet(const FileGetRequest& request) const
{
    return client_->FileGet(request);
}

FileRenameOutcome PdsClient::FileRename(const FileRenameRequest& request) const
{
    return client_->FileRename(request);
}

FileTrashOutcome PdsClient::FileTrash(const FileTrashRequest& request) const
{
    return client_->FileTrash(request);
}

FileDeleteOutcome PdsClient::FileDelete(const FileDeleteRequest& request) const
{
    return client_->FileDelete(request);
}

FileGetVideoPreviewPlayInfoOutcome PdsClient::FileGetVideoPreviewPlayInfo(const FileGetVideoPreviewPlayInfoRequest& request) const
{
    return client_->FileGetVideoPreviewPlayInfo(request);
}

////////////////////
// Data
////////////////////
DataGetOutcome PdsClient::DataGetByUrl(const DataGetByUrlRequest &request) const
{
    return client_->DataGetByUrl(request);
}

DataGetOutcome PdsClient::DataGetByUrl(const std::string &url) const
{
    return client_->DataGetByUrl(DataGetByUrlRequest(url));
}

DataGetOutcome PdsClient::DataGetByUrl(const std::string &url, const std::string &file) const
{
    DataGetByUrlRequest request(url);
    request.setResponseStreamFactory([=]() {return std::make_shared<std::fstream>(file, std::ios_base::out | std::ios_base::in | std::ios_base::trunc); });
    return client_->DataGetByUrl(request);
}

DataPutOutcome PdsClient::DataPutByUrl(const DataPutByUrlRequest &request) const
{
    return client_->DataPutByUrl(request);
}

DataPutOutcome PdsClient::DataPutByUrl(const std::string &url, const std::string &file) const
{
    std::shared_ptr<std::iostream> content = std::make_shared<std::fstream>(file, std::ios::in|std::ios::binary);
    return client_->DataPutByUrl(DataPutByUrlRequest(url, content));
}

DataPutOutcome PdsClient::DataPutByUrl(const std::string &url, const std::shared_ptr<std::iostream> &content) const
{
    return client_->DataPutByUrl(DataPutByUrlRequest(url, content));
}

////////////////////
// Meta
////////////////////
MetaUserTagsPutOutcome PdsClient::MetaUserTagsPut(const MetaUserTagsPutRequest &request) const
{
    return client_->MetaUserTagsPut(request);
}

PdsOutcome PdsClient::MetaUserTagsDelete(const MetaUserTagsDeleteRequest &request) const
{
    return client_->MetaUserTagsDelete(request);
}


////////////////////
// Resumable Operation
////////////////////
FileCompleteOutcome PdsClient::ResumableFileUpload(const FileUploadRequest &request) const
{
    return client_->ResumableFileUpload(request);
}


DataGetOutcome PdsClient::ResumableFileDownload(const FileDownloadRequest &request) const
{
    return client_->ResumableFileDownload(request);
}


void PdsClient::DisableRequest()
{
    client_->DisableRequest();
}

void PdsClient::EnableRequest()
{
    client_->EnableRequest();
}
