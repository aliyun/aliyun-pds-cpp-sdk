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

#include <ctime>
#include <algorithm>
#include <sstream>
#include <set>
#include <tinyxml2/tinyxml2.h>
#include <alibabacloud/pds/http/HttpType.h>
#include <alibabacloud/pds/Const.h>
#include <fstream>
#include "utils/Utils.h"
#include "utils/ThreadExecutor.h"
#include "PdsClientImpl.h"
#include "utils/LogUtils.h"
#include "utils/FileSystemUtils.h"
#include "resumable/ResumableUploader.h"
#include "resumable/ResumableDownloader.h"
using namespace AlibabaCloud::PDS;
using namespace tinyxml2;

namespace
{
const std::string SERVICE_NAME = "PDS";
const char *TAG = "PdsClientImpl";
}

PdsClientImpl::PdsClientImpl(const std::string &endpoint, const std::shared_ptr<CredentialsProvider>& credentialsProvider, const ClientConfiguration & configuration) :
    Client(SERVICE_NAME, configuration),
    endpoint_(endpoint),
    credentialsProvider_(credentialsProvider),
    executor_(configuration.executor ? configuration.executor :std::make_shared<ThreadExecutor>()),
    isValidEndpoint_(IsValidEndpoint(endpoint))
{
}

PdsClientImpl::~PdsClientImpl()
{
}

int PdsClientImpl::asyncExecute(Runnable * r) const
{
    if (executor_ == nullptr)
        return 1;

    executor_->execute(r);
    return 0;
}

std::shared_ptr<HttpRequest> PdsClientImpl::buildHttpRequest(const std::string & endpoint, const ServiceRequest & msg, Http::Method method) const
{
    auto httpRequest = std::make_shared<HttpRequest>(method);
    auto calcContentMD5 = !!(msg.Flags()&REQUEST_FLAG_CONTENTMD5);
    auto paramInPath = !!(msg.Flags()&REQUEST_FLAG_PARAM_IN_PATH);
    auto ossDataRequest = !!(msg.Flags()&REQUEST_FLAG_OSS_DATA_REQUEST);
    httpRequest->setResponseStreamFactory(msg.ResponseStreamFactory());
    addHeaders(httpRequest, msg.Headers());
    addBody(httpRequest, msg.Body(), calcContentMD5);
    if (paramInPath) {
        httpRequest->setUrl(Url(msg.Path()));
    }
    else {
        addUrl(httpRequest, endpoint, msg);
    }
    if (!ossDataRequest) {
        addAuthorization(httpRequest);
    }
    addOther(httpRequest, msg);
    return httpRequest;
}

bool PdsClientImpl::hasResponseError(const std::shared_ptr<HttpResponse>&response) const
{
    if (BASE::hasResponseError(response)) {
        return true;
    }

    //check crc64
    if (response->request().hasCheckCrc64() &&
        !response->request().hasHeader(Http::RANGE) &&
        response->hasHeader("x-oss-hash-crc64ecma")) {
        uint64_t clientCrc64 = response->request().Crc64Result();
        uint64_t serverCrc64 = std::strtoull(response->Header("x-oss-hash-crc64ecma").c_str(), nullptr, 10);
        if (clientCrc64 != serverCrc64) {
            response->setStatusCode(ERROR_CRC_INCONSISTENT);
            std::stringstream ss;
            ss << "Crc64 validation failed. Expected hash:" << serverCrc64
                << " not equal to calculated hash:" << clientCrc64
                << ". Transferd bytes:" << response->request().TransferedBytes()
                << ". RequestId:" << response->Header("x-oss-request-id").c_str();
            response->setStatusMsg(ss.str().c_str());
            return true;
        }
    }

    //check Calback
    if (response->statusCode() == 203 &&
        (response->request().hasHeader("x-oss-callback") ||
        (response->request().url().query().find("callback=") != std::string::npos))) {
        return true;
    }

    return false;
}

void PdsClientImpl::addHeaders(const std::shared_ptr<HttpRequest> &httpRequest, const HeaderCollection &headers) const
{
    for (auto const& header : headers) {
        httpRequest->addHeader(header.first, header.second);
    }

    //common headers
    httpRequest->addHeader(Http::USER_AGENT, configuration().userAgent);

    //Date
    if (httpRequest->hasHeader("x-oss-date")) {
        httpRequest->addHeader(Http::DATE, httpRequest->Header("x-oss-date"));
    }
    if (!httpRequest->hasHeader(Http::DATE)) {
        std::time_t t = std::time(nullptr);
        t += getRequestDateOffset();
        httpRequest->addHeader(Http::DATE, ToGmtTime(t));
    }
}

void PdsClientImpl::addBody(const std::shared_ptr<HttpRequest> &httpRequest, const std::shared_ptr<std::iostream>& body, bool contentMd5) const
{
    if (body == nullptr) {
        Http::Method methold = httpRequest->method();
        if (methold == Http::Method::Get || methold == Http::Method::Post) {
            httpRequest->setHeader(Http::CONTENT_LENGTH, "0");
        } else {
            httpRequest->removeHeader(Http::CONTENT_LENGTH);
        }
    }

    if ((body != nullptr) && !httpRequest->hasHeader(Http::CONTENT_LENGTH)) {
        auto streamSize = GetIOStreamLength(*body);
        httpRequest->setHeader(Http::CONTENT_LENGTH, std::to_string(streamSize));
    }

    if (contentMd5 && body && !httpRequest->hasHeader(Http::CONTENT_MD5)) {
        auto md5 = ComputeContentMD5(*body);
        httpRequest->setHeader(Http::CONTENT_MD5, md5);
    }

    httpRequest->addBody(body);
}

void PdsClientImpl::addAuthorization(const std::shared_ptr<HttpRequest> &httpRequest) const
{
    const Credentials credentials = credentialsProvider_->getCredentials();
    if (!credentials.AccessToken().empty()) {
        httpRequest->addHeader(Http::AUTHORIZATION, credentials.AccessToken());
    }

    PDS_LOG(LogLevel::LogDebug, TAG, "client(%p) request(%p) Authorization:%s", this, httpRequest.get(), credentials.AccessToken().c_str());
}

void PdsClientImpl::addUrl(const std::shared_ptr<HttpRequest> &httpRequest, const std::string &endpoint, const ServiceRequest &request) const
{
    auto host = GetHostString(endpoint);
    auto path = request.Path();

    Url url(host);
    url.setPath(path);

    PDS_LOG(LogLevel::LogDebug, TAG, "client(%p) request(%p) host:%s, path:%s", this, httpRequest.get(), host.c_str(), path.c_str());

    auto parameters = request.Parameters();
    if (!parameters.empty()) {
        std::stringstream queryString;
        for (const auto &p : parameters)
        {
            if (p.second.empty())
                queryString << "&" << UrlEncode(p.first);
            else
                queryString << "&" << UrlEncode(p.first) << "=" << UrlEncode(p.second);
        }
        url.setQuery(queryString.str().substr(1));
    }
    httpRequest->setUrl(url);
}

void PdsClientImpl::addOther(const std::shared_ptr<HttpRequest> &httpRequest, const ServiceRequest &request) const
{
    //progress
    httpRequest->setTransferProgress(request.TransferProgress());

    //crc64 check
    auto checkCRC64 = !!(request.Flags()&REQUEST_FLAG_CHECK_CRC64);
    if (configuration().enableCrc64 && checkCRC64 ) {
        httpRequest->setCheckCrc64(true);
#ifdef ENABLE_PDS_TEST
        if (!!(request.Flags()&0x80000000)) {
            httpRequest->addHeader("oss-test-crc64", "1");
        }
#endif
    }
}

PdsError PdsClientImpl::buildError(const Error &error) const
{
    PdsError err;
    if (((error.Status() == 203) || (error.Status() > 299 && error.Status() < 600)) &&
        !error.Message().empty()) {
        std::string contentType;
        auto it = error.Headers().find("Content-Type");
        if (it != error.Headers().end()) {
            contentType = it->second;
        }
        if (contentType.find("application/json") != contentType.npos) {
            Json::Value root;
            Json::CharReaderBuilder rbuilder;
            std::string errMsg;
            std::stringstream msg;
            msg << error.Message();
            if (Json::parseFromStream(rbuilder, msg, &root, &errMsg))
            {
                err.setCode(root["code"].asString());
                err.setMessage(root["message"].asString());
            } else {
                std::stringstream ss;
                ss << "ParseJSONError:" << errMsg;
                err.setCode("ParseJSONError");
                err.setMessage(ss.str());
            }
        } else if (contentType.find("application/json") != contentType.npos) {
            XMLDocument doc;
            XMLError xml_err;
            if ((xml_err = doc.Parse(error.Message().c_str(), error.Message().size())) == XML_SUCCESS) {
                XMLElement* root =doc.RootElement();
                if (root && !std::strncmp("Error", root->Name(), 5)) {
                    XMLElement *node;
                    node = root->FirstChildElement("Code");
                    err.setCode(node ? node->GetText(): "");
                    node = root->FirstChildElement("Message");
                    err.setMessage(node ? node->GetText(): "");
                    node = root->FirstChildElement("RequestId");
                    err.setRequestId(node ? node->GetText(): "");
                    node = root->FirstChildElement("HostId");
                    err.setHost(node ? node->GetText(): "");
                } else {
                    err.setCode("ParseXMLError");
                    err.setMessage("Xml format invalid, root node name is not Error. the content is:\n" + error.Message());
                }
            } else {
                std::stringstream ss;
                ss << "ParseXMLError:" << xml_err;
                err.setCode(ss.str());
                err.setMessage(XMLDocument::ErrorIDToName(xml_err));
            }
        } else {
            err.setCode(error.Code());
            err.setMessage(error.Message());
        }
    }
    else {
        err.setCode(error.Code());
        err.setMessage(error.Message());
    }

    //get from header if body has nothing
    if (err.RequestId().empty()) {
        auto it = error.Headers().find("x-ca-request-id");
        if (it != error.Headers().end()) {
            err.setRequestId(it->second);
        }
    }

    return err;
}

ServiceResult PdsClientImpl::buildResult(const PdsRequest &request, const std::shared_ptr<HttpResponse> &httpResponse) const
{
    ServiceResult result;
    auto flag = request.Flags();
    if ((flag & REQUEST_FLAG_CHECK_CRC64) &&
        (flag & REQUEST_FLAG_SAVE_CLIENT_CRC64)) {
        httpResponse->addHeader("x-oss-hash-crc64ecma-by-client", std::to_string(httpResponse->request().Crc64Result()));
    }
    result.setRequestId(httpResponse->Header("x-ca-request-id"));
    result.setPayload(httpResponse->Body());
    result.setResponseCode(httpResponse->statusCode());
    result.setHeaderCollection(httpResponse->Headers());
    return result;
}

PdsOutcome PdsClientImpl::MakeRequest(const PdsRequest &request, Http::Method method) const
{
    int ret = request.validate();
    if (ret != 0) {
        return PdsOutcome(PdsError("ValidateError", request.validateMessage(ret)));
    }

    if (!isValidEndpoint_) {
        return PdsOutcome(PdsError("ValidateError", "The endpoint is invalid."));
    }

    auto outcome = BASE::AttemptRequest(endpoint_, request, method);
    if (outcome.isSuccess()) {
        return PdsOutcome(buildResult(request, outcome.result()));
    } else {
        return PdsOutcome(buildError(outcome.error()));
    }
}


////////////////////
// AsyncTask
////////////////////
AsyncTaskGetOutcome PdsClientImpl::AsyncTaskGet(const AsyncTaskGetRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return AsyncTaskGetOutcome(AsyncTaskGetResult(outcome.result().payload()));
    }
    else {
        return AsyncTaskGetOutcome(outcome.error());
    }
}

////////////////////
// Dir
////////////////////
DirCreateOutcome PdsClientImpl::DirCreate(const DirCreateRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return DirCreateOutcome(DirCreateResult(outcome.result().payload()));
    }
    else {
        return DirCreateOutcome(outcome.error());
    }
}

DirListOutcome PdsClientImpl::DirList(const DirListRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return DirListOutcome(DirListResult(outcome.result().payload()));
    }
    else {
        return DirListOutcome(outcome.error());
    }
}

DirRenameOutcome PdsClientImpl::DirRename(const DirRenameRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return DirRenameOutcome(DirRenameResult(outcome.result().payload()));
    }
    else {
        return DirRenameOutcome(outcome.error());
    }
}

DirTrashOutcome PdsClientImpl::DirTrash(const DirTrashRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return DirTrashOutcome(DirTrashResult(outcome.result().payload()));
    }
    else {
        return DirTrashOutcome(outcome.error());
    }
}

DirDeleteOutcome PdsClientImpl::DirDelete(const DirDeleteRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return DirDeleteOutcome(DirDeleteResult(outcome.result().payload()));
    }
    else {
        return DirDeleteOutcome(outcome.error());
    }
}

////////////////////
// File
////////////////////
FileCreateOutcome PdsClientImpl::FileCreate(const FileCreateRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return FileCreateOutcome(FileCreateResult(outcome.result().payload()));
    }
    else {
        return FileCreateOutcome(outcome.error());
    }
}

FileGetUploadUrlOutcome PdsClientImpl::FileGetUploadUrl(const FileGetUploadUrlRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return FileGetUploadUrlOutcome(FileGetUploadUrlResult(outcome.result().payload()));
    }
    else {
        return FileGetUploadUrlOutcome(outcome.error());
    }
}

FileListUploadedPartsOutcome PdsClientImpl::FileListUploadedParts(const FileListUploadedPartsRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return FileListUploadedPartsOutcome(FileListUploadedPartsResult(outcome.result().payload()));
    }
    else {
        return FileListUploadedPartsOutcome(outcome.error());
    }
}

FileCompleteOutcome PdsClientImpl::FileComplete(const FileCompleteRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return FileCompleteOutcome(FileCompleteResult(outcome.result().payload()));
    }
    else {
        return FileCompleteOutcome(outcome.error());
    }
}

FileGetOutcome PdsClientImpl::FileGet(const FileGetRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return FileGetOutcome(FileGetResult(outcome.result().payload()));
    }
    else {
        return FileGetOutcome(outcome.error());
    }
}

FileRenameOutcome PdsClientImpl::FileRename(const FileRenameRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return FileRenameOutcome(FileRenameResult(outcome.result().payload()));
    }
    else {
        return FileRenameOutcome(outcome.error());
    }
}

FileTrashOutcome PdsClientImpl::FileTrash(const FileTrashRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return FileTrashOutcome(FileTrashResult());
    }
    else {
        return FileTrashOutcome(outcome.error());
    }
}

FileDeleteOutcome PdsClientImpl::FileDelete(const FileDeleteRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return FileDeleteOutcome(FileDeleteResult());
    }
    else {
        return FileDeleteOutcome(outcome.error());
    }
}


////////////////////
// Object
////////////////////
GetObjectOutcome PdsClientImpl::GetObjectByUrl(const GetObjectByUrlRequest &request) const
{
    auto outcome = BASE::AttemptRequest(endpoint_, request, Http::Method::Get);
    if (outcome.isSuccess()) {
        return GetObjectOutcome(GetObjectResult(outcome.result()->Body(), outcome.result()->Headers()));
    }
    else {
        return GetObjectOutcome(buildError(outcome.error()));
    }
}

PutObjectOutcome PdsClientImpl::PutObjectByUrl(const PutObjectByUrlRequest &request) const
{
    auto outcome = BASE::AttemptRequest(endpoint_, request, Http::Method::Put);
    if (outcome.isSuccess()) {
        return PutObjectOutcome(PutObjectResult(outcome.result()->Headers(),
            outcome.result()->Body()));
    }
    else {
        return PutObjectOutcome(buildError(outcome.error()));
    }
}

////////////////////
// Resumable Operation
////////////////////
FileCompleteOutcome PdsClientImpl::ResumableFileUpload(const FileUploadRequest &request) const
{
    const auto& reqeustBase = static_cast<const PdsResumableBaseRequest &>(request);
    int code = reqeustBase.validate();
    if (code != 0) {
        return FileCompleteOutcome(PdsError("ValidateError", reqeustBase.validateMessage(code)));
    }
    return FileCompleteOutcome();
    // TODO ResumableUploader
}


GetObjectOutcome PdsClientImpl::ResumableFileDownload(const FileDownloadRequest &request) const
{
    const auto& reqeustBase = static_cast<const PdsResumableBaseRequest &>(request);
    int code = reqeustBase.validate();
    if (code != 0) {
        return GetObjectOutcome(PdsError("ValidateError", reqeustBase.validateMessage(code)));
    }
    return GetObjectOutcome();
    // TODO ResumableDownloader
}


/*Requests control*/
void PdsClientImpl::DisableRequest()
{
    BASE::disableRequest();
    PDS_LOG(LogLevel::LogDebug, TAG, "client(%p) DisableRequest", this);
}

void PdsClientImpl::EnableRequest()
{
    BASE::enableRequest();
    PDS_LOG(LogLevel::LogDebug, TAG, "client(%p) EnableRequest", this);
}
