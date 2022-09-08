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

#ifndef ALIBABACLOUD_PDS_PDSCLIENTIMPL_H_
#define ALIBABACLOUD_PDS_PDSCLIENTIMPL_H_

#include <alibabacloud/pds/client/ClientConfiguration.h>
#include <alibabacloud/pds/auth/CredentialsProvider.h>
#include <alibabacloud/pds/PdsRequest.h>
#include <alibabacloud/pds/PdsResponse.h>
#include <alibabacloud/pds/utils/Executor.h>
#include <alibabacloud/pds/PdsFwd.h>
#include "client/Client.h"
#ifdef GetObject
#undef GetObject
#endif

namespace AlibabaCloud
{
namespace PDS
{
    class PdsClientImpl : public Client
    {
    public:
        typedef Client BASE;

        PdsClientImpl(const std::string &endpoint, const std::shared_ptr<CredentialsProvider>& credentialsProvider, const ClientConfiguration & configuration);
        virtual ~PdsClientImpl();
        int asyncExecute(Runnable * r) const;

        /*AsyncTask*/
        AsyncTaskGetOutcome AsyncTaskGet(const AsyncTaskGetRequest& request) const;

        /*Dir*/
        DirCreateOutcome DirCreate(const DirCreateRequest& request) const;
        DirListOutcome DirList(const DirListRequest& request) const;
        DirSearchOutcome DirSearch(const DirSearchRequest& request) const;
        DirRenameOutcome DirRename(const DirRenameRequest& request) const;
        DirTrashOutcome DirTrash(const DirTrashRequest& request) const;
        DirDeleteOutcome DirDelete(const DirDeleteRequest& request) const;

        /*File*/
        FileCreateOutcome FileCreate(const FileCreateRequest& request) const;
        FileGetUploadUrlOutcome FileGetUploadUrl(const FileGetUploadUrlRequest& request) const;
        FileListUploadedPartsOutcome FileListUploadedParts(const FileListUploadedPartsRequest& request) const;
        FileCompleteOutcome FileComplete(const FileCompleteRequest& request) const;
        FileGetDownloadUrlOutcome FileGetDownloadUrl(const FileGetDownloadUrlRequest& request) const;
        FileGetOutcome FileGet(const FileGetRequest &request) const;
        FileRenameOutcome FileRename(const FileRenameRequest& request) const;
        FileTrashOutcome FileTrash(const FileTrashRequest& request) const;
        FileDeleteOutcome FileDelete(const FileDeleteRequest& request) const;

        /*Data*/
        DataGetOutcome DataGetByUrl(const DataGetByUrlRequest &request) const;
        DataPutOutcome DataPutByUrl(const DataPutByUrlRequest &request) const;

        /*Resumable Operation*/
        FileCompleteOutcome ResumableFileUpload(const FileUploadRequest& request) const;
        DataGetOutcome ResumableFileDownload(const FileDownloadRequest& request) const;

        /*Meta*/
        MetaUserTagsPutOutcome MetaUserTagsPut(const MetaUserTagsPutRequest& request) const;
        PdsOutcome MetaUserTagsDelete(const MetaUserTagsDeleteRequest& request) const;

        /*Requests control*/
        void DisableRequest();
        void EnableRequest();

    protected:
        virtual std::shared_ptr<HttpRequest> buildHttpRequest(const std::string & endpoint, const ServiceRequest &msg, Http::Method method) const;
        virtual bool hasResponseError(const std::shared_ptr<HttpResponse>&response)  const;
        PdsOutcome MakeRequest(const PdsRequest &request, Http::Method method) const;

    private:
        void addHeaders(const std::shared_ptr<HttpRequest> &httpRequest, const HeaderCollection &headers) const;
        void addPdsHeaders(const std::shared_ptr<HttpRequest> &httpRequest) const;
        void addBody(const std::shared_ptr<HttpRequest> &httpRequest, const std::shared_ptr<std::iostream>& body, bool contentMd5 = false) const;
        void addUrl(const std::shared_ptr<HttpRequest> &httpRequest, const std::string &endpoint, const ServiceRequest &request) const;
        void addOther(const std::shared_ptr<HttpRequest> &httpRequest, const ServiceRequest &request) const;

        PdsError buildError(const Error &error) const;
        ServiceResult buildResult(const PdsRequest &request, const std::shared_ptr<HttpResponse> &httpResponse) const;

    private:
        std::string endpoint_;
        std::shared_ptr<CredentialsProvider> credentialsProvider_;
        std::shared_ptr<Executor> executor_;
        bool isValidEndpoint_;
    };
}
}
#endif // !ALIBABACLOUD_PDS_PDSCLIENTIMPL_H_
