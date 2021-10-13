#include <iostream>
#include "../Config.h"
#include "../utils/FileOperationUtils.h"
#include "../utils/MyCredentialsProvider.h"
#include "FileSample.h"
#include <alibabacloud/pds/Const.h>
#include <memory>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace AlibabaCloud::PDS;

FileSample::FileSample()
{
    ClientConfiguration conf;
    std::shared_ptr<MyCredentialsProvider> cProvider = std::make_shared<MyCredentialsProvider>(Config::AccessToken);
    client = new PdsClient(Config::Endpoint, cProvider, conf);
}

FileSample::~FileSample() {
    delete client;
}

void FileSample::PrintError(const std::string &funcName, const PdsError &error)
{
    std::cout << funcName << " fail" <<
        ",code:" << error.Code() <<
        ",message:" << error.Message() <<
        ",request_id:" << error.RequestId() << std::endl;
}

std::string FileSample::FileCreate()
{
    time_t lastMtime;
    std::streamsize fSize;
    if (!GetPathInfo(Config::FileToUpload, lastMtime, fSize)) {
        PrintError(__FUNCTION__, PdsError("FileNotFound", "upload file is not found"));
    }
    uint64_t fileSize = static_cast<uint64_t>(fSize);

    // PDS create
    std::string fileID;
    FileCreateRequest createRequest(Config::DriveID, Config::RootParentID, "test_file", "", "auto_rename", fileSize);

    // set user tags when create
    UserTagList userTagList;
    UserTag userTag("key", "");
    userTagList.push_back(userTag);
    createRequest.setUserTags(userTagList);

    auto createOutcome = client->FileCreate(createRequest);
    if (!createOutcome.isSuccess()) {
        PrintError(__FUNCTION__, createOutcome.error());
        return fileID;
    }
    fileID = createOutcome.result().FileID();
    std::string uploadID = createOutcome.result().UploadID();
    std::cout << __FUNCTION__ << " call FileCreate success, file id: " << fileID << ", upload id: " << uploadID << std::endl;

    // PDS get upload url
    PartInfoReqList partInfoReqList;
    PartInfoReq info(1, fileSize, 0, fileSize-1);
    partInfoReqList.push_back(info);

    FileGetUploadUrlRequest getUploadUrlRequest(Config::DriveID, fileID, uploadID, partInfoReqList);
    auto getUploadUrlOutcome = client->FileGetUploadUrl(getUploadUrlRequest);
    if (!getUploadUrlOutcome.isSuccess()) {
        PrintError(__FUNCTION__, getUploadUrlOutcome.error());
        return fileID;
    }
    std::cout << __FUNCTION__ << " call FileGetUploadUrl success" << std::endl;

    PartInfoRespList partInfoRespList = getUploadUrlOutcome.result().PartInfoRespList();
    if (partInfoRespList.size() == 0) {
        std::cout << __FUNCTION__ << " partInfoRespList empty" << std::endl;
        return fileID;
    }
    std::string uploadURl = partInfoRespList[0].UploadUrl();
    std::cout << __FUNCTION__ << " upload url: " << uploadURl << std::endl;

    // upload to OSS
    std::shared_ptr<std::iostream> content = std::make_shared<std::fstream>(Config::FileToUpload, std::ios::in | std::ios::binary);
    DataPutByUrlRequest uploadDataRequest(uploadURl, content);
    auto uploadDataOutcome = client->DataPutByUrl(uploadDataRequest);
    if (!uploadDataOutcome.isSuccess()) {
        PrintError(__FUNCTION__, uploadDataOutcome.error());
        return fileID;
    }
    std::cout << __FUNCTION__ << " upload data success" << std::endl;

    // PDS complete
    FileCompleteRequest completeRequest(Config::DriveID, fileID, uploadID);
    auto completeOutcome = client->FileComplete(completeRequest);
    if (!completeOutcome.isSuccess()) {
        PrintError(__FUNCTION__, completeOutcome.error());
        return fileID;
    }
    std::cout << __FUNCTION__ << " call FileComplete success, file id: " << fileID << ", name: " << completeOutcome.result().Name() << std::endl;
    return fileID;
}

void FileSample::FileDownload(const std::string& fileID)
{
    FileGetDownloadUrlRequest request(Config::DriveID, "", fileID);
    auto getOutcome = client->FileGetDownloadUrl(request);
    if (!getOutcome.isSuccess()) {
        PrintError(__FUNCTION__, getOutcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call FileGetDownloadUrl success, file id: " << fileID << std::endl;

    std::string downloadURl = getOutcome.result().Url();
    int64_t size = getOutcome.result().Size();
    std::cout << __FUNCTION__ << " download url:" << downloadURl << ", size:" << size << std::endl;

    // download from OSS to file
    DataGetByUrlRequest downloadDataRequest(downloadURl);
    downloadDataRequest.setResponseStreamFactory([=]() {return std::make_shared<std::fstream>(Config::FileDownloadTo,
        std::ios_base::out | std::ios_base::in | std::ios_base::trunc| std::ios_base::binary); });
    auto downloadDataOutcome = client->DataGetByUrl(downloadDataRequest);
    if (!downloadDataOutcome.isSuccess()) {
        PrintError(__FUNCTION__, downloadDataOutcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " download data success" << std::endl;
}

void FileSample::FileGet(const std::string& fileID)
{
    FileGetRequest request(Config::DriveID, "", fileID);
    auto outcome = client->FileGet(request);
    if (!outcome.isSuccess()) {
        PrintError(__FUNCTION__, outcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call FileRename success, file id: " << fileID << ", name: " << outcome.result().Name() << std::endl;
}

void FileSample::FileRename(const std::string& fileID)
{
    FileRenameRequest request(Config::DriveID, fileID, "test_file", "auto_rename");
    auto outcome = client->FileRename(request);
    if (!outcome.isSuccess()) {
        PrintError(__FUNCTION__, outcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call FileRename success, file id: " << fileID << ", name: " << outcome.result().Name() << std::endl;
}

void FileSample::FileTrash(const std::string& fileID)
{
    FileTrashRequest request(Config::DriveID, fileID);
    auto outcome = client->FileTrash(request);
    if (!outcome.isSuccess()) {
        PrintError(__FUNCTION__, outcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call FileTrash success, file id: " << fileID << std::endl;
}

void FileSample::FileDelete(const std::string& fileID)
{
    FileDeleteRequest request(Config::DriveID, fileID);
    auto outcome = client->FileDelete(request);
    if (!outcome.isSuccess()) {
        PrintError(__FUNCTION__, outcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call FileDelete success, file id: " << fileID << std::endl;
}

void FileSample::UserTagsPut(const std::string& fileID)
{

    UserTagList tags;
    tags.push_back(UserTag("key1", "value1"));
    tags.push_back(UserTag("key2", "value2"));
    MetaUserTagsPutRequest request(Config::DriveID, fileID, tags);
    auto outcome = client->MetaUserTagsPut(request);
    if (!outcome.isSuccess()) {
        PrintError(__FUNCTION__, outcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call MetaUserTagsPut success, fileID:" << fileID << std::endl;
}

void FileSample::UserTagsDelete(const std::string& fileID)
{

    std::vector<std::string> keys;
    keys.push_back("key1");
    keys.push_back("key2");
    MetaUserTagsDeleteRequest request(Config::DriveID, fileID, keys);
    auto outcome = client->MetaUserTagsDelete(request);
    if (!outcome.isSuccess()) {
        PrintError(__FUNCTION__, outcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call MetaUserTagsDelete success, fileID:" << fileID << std::endl;
}