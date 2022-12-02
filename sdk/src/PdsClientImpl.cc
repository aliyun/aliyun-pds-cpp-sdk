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
    auto pdsDataRequest = !!(msg.Flags()&REQUEST_FLAG_PDS_DATA_REQUEST);
    httpRequest->setResponseStreamFactory(msg.ResponseStreamFactory());
    addHeaders(httpRequest, msg.Headers());
    if (!pdsDataRequest) {
        addPdsHeaders(httpRequest);
    }
    addBody(httpRequest, msg.Body(), calcContentMD5);
    if (paramInPath) {
        httpRequest->setUrl(Url(msg.Path()));
    }
    else {
        addUrl(httpRequest, endpoint, msg);
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
    if (!httpRequest->hasHeader(Http::DATE)) {
        std::time_t t = std::time(nullptr);
        httpRequest->addHeader(Http::DATE, ToGmtTime(t));
    }
}

void PdsClientImpl::addPdsHeaders(const std::shared_ptr<HttpRequest> &httpRequest) const
{
    const Credentials credentials = credentialsProvider_->getCredentials();
    if (!credentials.AccessToken().empty()) {
        httpRequest->addHeader(Http::AUTHORIZATION, credentials.AccessToken());
    }

    if (!httpRequest->hasHeader(Http::CONTENT_TYPE)) {
        httpRequest->addHeader(Http::CONTENT_TYPE, "application/json");
    }

    PDS_LOG(LogLevel::LogDebug, TAG, "client(%p) request(%p) Authorization:%s", this, httpRequest.get(), credentials.AccessToken().c_str());
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

    // progress control
    httpRequest->setProgressControl(request.ProgressControl());

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
        } else if (contentType.find("application/xml") != contentType.npos) {
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

DirSearchOutcome PdsClientImpl::DirSearch(const DirSearchRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return DirSearchOutcome(DirSearchResult(outcome.result().payload()));
    }
    else {
        return DirSearchOutcome(outcome.error());
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

FileGetDownloadUrlOutcome PdsClientImpl::FileGetDownloadUrl(const FileGetDownloadUrlRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return FileGetDownloadUrlOutcome(FileGetDownloadUrlResult(outcome.result().payload()));
    }
    else {
        return FileGetDownloadUrlOutcome(outcome.error());
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

FileGetOutcome PdsClientImpl::FileGetByPath(const FileGetByPathRequest& request) const
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

FileGetVideoPreviewPlayInfoOutcome PdsClientImpl::FileGetVideoPreviewPlayInfo(const FileGetVideoPreviewPlayInfoRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return FileGetVideoPreviewPlayInfoOutcome(outcome.result().payload());
    }
    else {
        return FileGetVideoPreviewPlayInfoOutcome(outcome.error());
    }
}

////////////////////
// Data
////////////////////
DataGetOutcome PdsClientImpl::DataGetByUrl(const DataGetByUrlRequest &request) const
{
    auto outcome = MakeRequest(request, Http::Method::Get);
    if (outcome.isSuccess()) {
        return DataGetOutcome(DataGetResult(outcome.result().payload(), outcome.result().headerCollection()));
    }
    else {
        return DataGetOutcome(outcome.error());
    }
}

DataPutOutcome PdsClientImpl::DataPutByUrl(const DataPutByUrlRequest &request) const
{
    auto outcome = AttemptRequest(endpoint_, request, Http::Method::Put);
    if (outcome.isSuccess()) {
        return DataPutOutcome(DataPutResult(outcome.result()->Headers(),
            outcome.result()->Body()));
    }
    else {
        return DataPutOutcome(buildError(outcome.error()));
    }
}

////////////////////
// Meta
////////////////////
MetaUserTagsPutOutcome PdsClientImpl::MetaUserTagsPut(const MetaUserTagsPutRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    if (outcome.isSuccess()) {
        return MetaUserTagsPutOutcome(MetaUserTagsPutResult(outcome.result().payload()));
    }
    else {
        return MetaUserTagsPutOutcome(outcome.error());
    }
}

PdsOutcome PdsClientImpl::MetaUserTagsDelete(const MetaUserTagsDeleteRequest& request) const
{
    auto outcome = MakeRequest(request, Http::Method::Post);
    return outcome;
}

///////////////////////////
// Resumable Operation
///////////////////////////
FileCompleteOutcome PdsClientImpl::ResumableFileUpload(const FileUploadRequest &request) const
{
    const auto& reqeustBase = static_cast<const PdsResumableBaseRequest &>(request);
    int code = reqeustBase.validate();
    if (code != 0) {
        return FileCompleteOutcome(PdsError("ValidateError", reqeustBase.validateMessage(code)));
    }

    if (request.FileSize() <= request.PartSize())
    {
        PartInfoReqList partInfoReqList;
        PartInfoReq info(1, request.FileSize(), 0, request.FileSize()-1);
        partInfoReqList.push_back(info);

        FileCreateResult fileCreateResult;
        // rapid upload
        if (configuration().enableRapidUpload) {
            // caculate pre hash
            uint64_t preHashBlockSize = 1024;
            if (preHashBlockSize > request.FileSize()) {
                preHashBlockSize = request.FileSize();
            }

            auto content = GetFstreamByPath(request.FilePath(), request.FilePathW(),
            std::ios::in | std::ios::binary);
            char streamBuffer[1024];
            uint64_t readSize = 0;
            if (!content->good()) {
                return FileCompleteOutcome(PdsError("ReadFileError", "Failed to read input file"));
            }
            content->read(streamBuffer, preHashBlockSize);
            readSize = static_cast<uint64_t>(content->gcount());
            content->close();

            if (readSize != preHashBlockSize) {
                return FileCompleteOutcome(PdsError("ReadFileError", "Failed to read enough size for caculate pre hash"));
            }
            std::string preHashSHA1 = ComputeContentSha1(streamBuffer, preHashBlockSize);

            // check pre hash request
            auto fileCreatePreCheckReq = FileCreateRequest(request.DriveID(), request.ParentFileID(), request.Name(),
                request.FileID(), request.CheckNameMode(), request.FileSize());
            fileCreatePreCheckReq.setPreHash(preHashSHA1);
            fileCreatePreCheckReq.setPartInfoList(partInfoReqList);
            fileCreatePreCheckReq.setUserTags(request.UserTags());
            fileCreatePreCheckReq.setHidden(request.Hidden());
            auto fileCreatePreCheckOutcome = FileCreate(fileCreatePreCheckReq);

            if (!fileCreatePreCheckOutcome.isSuccess()) {
                // check pre hash error
                if (fileCreatePreCheckOutcome.error().Code() != "PreHashMatched") {
                    return FileCompleteOutcome(fileCreatePreCheckOutcome.error());
                }

                // check pre hash matched, rapid upload
                auto content = GetFstreamByPath(request.FilePath(), request.FilePathW(),
                std::ios::in | std::ios::binary);
                std::string hashSHA1 = ComputeContentSha1(*content);
                content->close();

                // rapid upload request
                auto fileCreateRapidUploadReq = FileCreateRequest(request.DriveID(), request.ParentFileID(), request.Name(),
                    request.FileID(), request.CheckNameMode(), request.FileSize());
                fileCreateRapidUploadReq.setContentHash(hashSHA1);
                fileCreateRapidUploadReq.setPartInfoList(partInfoReqList);
                fileCreateRapidUploadReq.setUserTags(request.UserTags());
                fileCreateRapidUploadReq.setHidden(request.Hidden());
                auto fileCreateRapidUploadOutcome = FileCreate(fileCreateRapidUploadReq);

                if (!fileCreateRapidUploadOutcome.isSuccess()) {
                    return FileCompleteOutcome(fileCreateRapidUploadOutcome.error());
                }

                // rapid upload success
                if (fileCreateRapidUploadOutcome.result().RapidUpload()) {
                    FileCompleteResult result(fileCreateRapidUploadOutcome.result());
                    return FileCompleteOutcome(result);
                }

                // failed to rapid upload, upload data
                fileCreateResult = fileCreateRapidUploadOutcome.result();
            } else {
                // pre hash check not matched, upload data
                fileCreateResult = fileCreatePreCheckOutcome.result();
            }
        } else {
            // create by upload data
            auto fileCreateReq = FileCreateRequest(request.DriveID(), request.ParentFileID(), request.Name(),
                request.FileID(), request.CheckNameMode(), request.FileSize());
            fileCreateReq.setPartInfoList(partInfoReqList);
            fileCreateReq.setUserTags(request.UserTags());
            fileCreateReq.setHidden(request.Hidden());
            auto fileCreateOutcome = FileCreate(fileCreateReq);
            if (!fileCreateOutcome.isSuccess()) {
                return FileCompleteOutcome(fileCreateOutcome.error());
            }
            fileCreateResult = fileCreateOutcome.result();
        }

        if (fileCreateResult.Exist()) {
            return FileCompleteOutcome(PdsError("SameNameFileExist", "Same name file exist."));
        }

        std::string fileID = fileCreateResult.FileID();
        std::string uploadID = fileCreateResult.UploadID();
        PartInfoRespList partInfoRespList = fileCreateResult.PartInfoRespList();
        if (partInfoRespList.size() == 0) {
            return FileCompleteOutcome(PdsError("GetUploadUrlError", "Get Upload url empty."));
        }
        std::string uploadURl = partInfoRespList[0].UploadUrl();

        // upload data
        auto content = GetFstreamByPath(request.FilePath(), request.FilePathW(),
            std::ios::in | std::ios::binary);
        DataPutByUrlRequest putPartRequest(uploadURl, content);
        if (request.TransferProgress().Handler) {
            putPartRequest.setTransferProgress(request.TransferProgress());
        }
        if (request.ProgressControl().Handler) {
            putPartRequest.setProgressControl(request.ProgressControl());
        }
        auto putOutCome = DataPutByUrl(putPartRequest);

        auto controller = request.ProgressControl();
        if (controller.Handler) {
            int32_t controlFlag = controller.Handler(controller.UserData);
            if (controlFlag == ProgressControlStop) {
                return FileCompleteOutcome(PdsError("ClientError:100003", "Upload stop by upper."));
            }
            if (controlFlag == ProgressControlCancel) {
                return FileCompleteOutcome(PdsError("ClientError:100004", "Upload cancel by upper."));
            }
        }

        if (!putOutCome.isSuccess()) {
           return FileCompleteOutcome(putOutCome.error());
        }

        // pds complete file
        FileCompleteRequest completeRequest(request.DriveID(), fileID, uploadID);
        auto completeOutcome = FileComplete(completeRequest);
        if (!completeOutcome.isSuccess()) {
            return FileCompleteOutcome(completeOutcome.error());
        }

        // check size
        uint64_t uploadedfileSize = completeOutcome.result().Size();
        if (request.FileSize() != uploadedfileSize) {
            return FileCompleteOutcome(PdsError("FileSizeCheckError", "Upload data check size fail."));
        }

        return completeOutcome;
    }
    else {
        ResumableUploader uploader(request, this);
        return uploader.Upload();
    }
}

DataGetOutcome PdsClientImpl::ResumableFileDownload(const FileDownloadRequest &request) const
{
    const auto& reqeustBase = static_cast<const PdsResumableBaseRequest &>(request);
    int code = reqeustBase.validate();
    if (code != 0) {
        return DataGetOutcome(PdsError("ValidateError", reqeustBase.validateMessage(code)));
    }

    auto getDownloadUrlReq = FileGetDownloadUrlRequest(request.DriveID(), request.ShareID(), request.FileID());
    getDownloadUrlReq.setShareToken(request.ShareToken());

    auto getDownloadUrlOutcome = FileGetDownloadUrl(getDownloadUrlReq);
    if (!getDownloadUrlOutcome.isSuccess()) {
        return DataGetOutcome(getDownloadUrlOutcome.error());
    }

    auto url = getDownloadUrlOutcome.result().Url();
    if (url.empty()) {
        return DataGetOutcome(PdsError("DownloadUrlEmptyError", "Download data url is empty."));
    }
    auto punishFlag = getDownloadUrlOutcome.result().PunishFlag();
    if (!configuration().enableDownloadPunishedFile && punishFlag == PunishFlagFileFreeze) {
        return DataGetOutcome(PdsError("FileIsPunished", "Punished File cannot be downloaded."));
    }
    auto fileSize = getDownloadUrlOutcome.result().Size();
    auto contentHash = getDownloadUrlOutcome.result().ContentHash();
    auto crc64Hash = getDownloadUrlOutcome.result().Crc64Hash();

    // small file and punished file are downloaded by single OSS GET request
    if (fileSize < (int64_t)request.PartSize() || punishFlag == PunishFlagFileFreeze) {
        auto getDataReq = DataGetByUrlRequest(url);
        if (request.TransferProgress().Handler) {
            getDataReq.setTransferProgress(request.TransferProgress());
        }
        if (request.ProgressControl().Handler) {
            getDataReq.setProgressControl(request.ProgressControl());
        }
        if (request.TrafficLimit() != 0) {
            getDataReq.setTrafficLimit(request.TrafficLimit());
        }
        getDataReq.setResponseStreamFactory([=]() {
            return GetFstreamByPath(request.TempFilePath(), request.TempFilePathW(),
                std::ios_base::out | std::ios_base::in | std::ios_base::trunc | std::ios_base::binary);
        });
        if (configuration().enableCrc64 && !crc64Hash.empty()) {
            getDataReq.setFlags(getDataReq.Flags() | REQUEST_FLAG_CHECK_CRC64 | REQUEST_FLAG_SAVE_CLIENT_CRC64);
        }
        auto outcome = DataGetByUrl(getDataReq);

        auto controller = request.ProgressControl();
        if (controller.Handler) {
            int32_t controlFlag = controller.Handler(controller.UserData);
            if (controlFlag == ProgressControlStop || controlFlag == ProgressControlCancel) {
                if (IsFileExist(request.TempFilePath())) {
                    RemoveFile(request.TempFilePath());
                }
#ifdef _WIN32
                else if (IsFileExist(request.TempFilePathW())) {
                    RemoveFile(request.TempFilePathW());
                }
#endif
            }
            if (controlFlag == ProgressControlStop) {
                return DataGetOutcome(PdsError("ClientError:100003", "Download stop by upper."));
            }
            if (controlFlag == ProgressControlCancel) {
                return DataGetOutcome(PdsError("ClientError:100004", "Download cancel by upper."));
            }
        }
        std::shared_ptr<std::iostream> content = nullptr;
        outcome.result().setContent(content);

        // check size
        if (configuration().enableCheckDownloadFileSize) {
            uint64_t localFileSize = GetFileSize(request.TempFilePath(), request.TempFilePathW());
            // for punished file, downloaded file size may not be equal to origin file size
            if (uint64_t(fileSize) != localFileSize && punishFlag != PunishFlagFileFreeze) {
                return DataGetOutcome(PdsError("FileSizeCheckError", "Download data check size fail."));
            }
        }

        if (configuration().enableCrc64 && !crc64Hash.empty()) {
            auto localCRC64 = std::strtoull(outcome.result().Metadata().HttpMetaData().at("x-oss-hash-crc64ecma-by-client").c_str(), nullptr, 10);
            uint64_t ossServerCrc64 = std::strtoull(outcome.result().Metadata().HttpMetaData().at("x-oss-hash-crc64ecma").c_str(), nullptr, 10);
            if (localCRC64 != ossServerCrc64) {
                if (IsFileExist(request.TempFilePath())) {
                    RemoveFile(request.TempFilePath());
                }
#ifdef _WIN32
                else if (IsFileExist(request.TempFilePathW())) {
                    RemoveFile(request.TempFilePathW());
                }
#endif
                return DataGetOutcome(PdsError("CrcCheckError", "Download data CRC checksum fail."));
            }
        }

        bool renameSuccess = false;
        if (!request.TempFilePath().empty()) {
            renameSuccess = RenameFile(request.TempFilePath(), request.FilePath());
        }
#ifdef _WIN32
        else if (!request.TempFilePathW().empty()) {
           renameSuccess = RenameFile(request.TempFilePathW(), request.FilePathW());
        }
#endif
        if (!renameSuccess) {
            std::stringstream ss;
            ss << "rename temp file failed";
            return DataGetOutcome(PdsError("RenameError", ss.str()));
        }

        if (IsFileExist(request.TempFilePath())) {
            RemoveFile(request.TempFilePath());
        }
#ifdef _WIN32
        else if (IsFileExist(request.TempFilePathW())) {
            RemoveFile(request.TempFilePathW());
        }
#endif

        return outcome;
    }

    ResumableDownloader downloader(request, this, fileSize, contentHash, crc64Hash, url);
    return downloader.Download();
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
