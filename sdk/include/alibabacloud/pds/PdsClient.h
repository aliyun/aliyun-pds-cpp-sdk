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
#include <alibabacloud/pds/client/ClientConfiguration.h>
#include <alibabacloud/pds/auth/CredentialsProvider.h>
#include <alibabacloud/pds/PdsFwd.h>
#include <alibabacloud/pds/client/AsyncCallerContext.h>
#include <future>
#include <ctime>

namespace AlibabaCloud
{
namespace PDS
{
    /*Global Init/Deinit*/
    void ALIBABACLOUD_PDS_EXPORT InitializeSdk();
    bool ALIBABACLOUD_PDS_EXPORT IsSdkInitialized();
    void ALIBABACLOUD_PDS_EXPORT ShutdownSdk();

    /*Log*/
    void ALIBABACLOUD_PDS_EXPORT SetLogLevel(LogLevel level);
    void ALIBABACLOUD_PDS_EXPORT SetLogCallback(LogCallback callback);

    class PdsClientImpl;
    class ALIBABACLOUD_PDS_EXPORT PdsClient
    {
    public:

        PdsClient(const std::string& endpoint, const ClientConfiguration& configuration);
        PdsClient(const std::string& endpoint, const std::string& accessToken, const ClientConfiguration& configuration);
        PdsClient(const std::string& endpoint, const Credentials& credentials, const ClientConfiguration& configuration);
        PdsClient(const std::string& endpoint, const std::shared_ptr<CredentialsProvider>& credentialsProvider, const ClientConfiguration& configuration);
        virtual ~PdsClient();

        /*AsyncTask*/
        AsyncTaskGetOutcome AsyncTaskGet(const AsyncTaskGetRequest& request) const;

        /*Dir*/
        DirCreateOutcome DirCreate(const DirCreateRequest& request) const;
        DirListOutcome DirList(const DirListRequest& request) const;
        DirRenameOutcome DirRename(const DirRenameRequest& request) const;
        DirTrashOutcome DirTrash(const DirTrashRequest& request) const;
        DirDeleteOutcome DirDelete(const DirDeleteRequest& request) const;

        /*File*/
        FileCreateOutcome FileCreate(const FileCreateRequest& request) const;
        FileGetUploadUrlOutcome FileGetUploadUrl(const FileGetUploadUrlRequest& request) const;
        FileListUploadedPartsOutcome FileListUploadedParts(const FileListUploadedPartsRequest& request) const;
        FileCompleteOutcome FileComplete(const FileCompleteRequest& request) const;
        FileGetOutcome FileGet(const FileGetRequest& request) const;
        FileRenameOutcome FileRename(const FileRenameRequest& request) const;
        FileTrashOutcome FileTrash(const FileTrashRequest& request) const;
        FileDeleteOutcome FileDelete(const FileDeleteRequest& request) const;

        /*Data*/
        DataGetOutcome DataGetByUrl(const DataGetByUrlRequest& request) const;
        DataGetOutcome DataGetByUrl(const std::string& url) const;
        DataGetOutcome DataGetByUrl(const std::string& url, const std::string& file) const;
        DataPutOutcome DataPutByUrl(const DataPutByUrlRequest& request) const;
        DataPutOutcome DataPutByUrl(const std::string& url, const std::string& file) const;
        DataPutOutcome DataPutByUrl(const std::string& url, const std::shared_ptr<std::iostream>& content) const;

        /*Meta*/
        MetaUserTagsPutOutcome MetaUserTagsPut(const MetaUserTagsPutRequest& request) const;
        PdsOutcome MetaUserTagsDelete(const MetaUserTagsDeleteRequest& request) const;

        /*Resumable Operation*/
        FileCompleteOutcome ResumableFileUpload(const FileUploadRequest& request) const;
        DataGetOutcome ResumableFileDownload(const FileDownloadRequest& request) const;

        /*Requests control*/
        void DisableRequest();
        void EnableRequest();
    protected:
        std::shared_ptr<PdsClientImpl> client_;
    };
}
}
