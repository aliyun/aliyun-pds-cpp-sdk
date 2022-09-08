#include <iostream>
#include "../Config.h"
#include "../utils/MyCredentialsProvider.h"
#include "DirSample.h"
#include <alibabacloud/pds/Const.h>
#include <memory>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace AlibabaCloud::PDS;

DirSample::DirSample()
{
    ClientConfiguration conf;
    std::shared_ptr<MyCredentialsProvider> cProvider = std::make_shared<MyCredentialsProvider>("");
    client = new PdsClient(Config::Endpoint, cProvider, conf);
    // update access token
    cProvider->setAccessToken(Config::AccessToken);
}

DirSample::~DirSample() {
    delete client;
}

void DirSample::PrintError(const std::string &funcName, const PdsError &error)
{
    std::cout << funcName << " fail" <<
        ",code:" << error.Code() <<
        ",message:" << error.Message() <<
        ",request_id:" << error.RequestId() << std::endl;
}

std::string DirSample::DirCreate()
{
    DirCreateRequest request(Config::DriveID, Config::RootParentID, "test_dir", "auto_rename");

    // set user tags when create
    UserTagList userTagList;
    UserTag userTag("key", "value");
    userTagList.push_back(userTag);
    request.setUserTags(userTagList);

    auto outcome = client->DirCreate(request);
    if (!outcome.isSuccess()) {
        PrintError(__FUNCTION__, outcome.error());
        return "";
    }
    std::cout << __FUNCTION__ << " call DirCreate success, file id: " << outcome.result().FileID() << ", name: " << outcome.result().FileName() << std::endl;
    return outcome.result().FileID();
}

void DirSample::DirRename(const std::string& fileID)
{
    DirRenameRequest request(Config::DriveID, fileID, "test_dir2", "auto_rename");
    auto outcome = client->DirRename(request);
    if (!outcome.isSuccess()) {
        PrintError(__FUNCTION__, outcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call DirRename success, file id: " << fileID << ", name: " << outcome.result().Name() << std::endl;
}

void DirSample::DirList(const std::string& parentFileID)
{
    DirListRequest request(Config::DriveID, parentFileID, "updated_at", "DESC", "*", "", 10);
    auto outcome = client->DirList(request);
    if (!outcome.isSuccess()) {
        PrintError(__FUNCTION__, outcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call DirList success, parent file id: " <<  parentFileID << std::endl;
    auto fileList = outcome.result().FileItemList();
    for (uint32_t i = 0; i < fileList.size(); i++) {
        std::cout << "Type: " << fileList[i].Type() << ", FileID: " << fileList[i].FileID() <<  ", Name: " << fileList[i].Name() << std::endl;
    }
}

void DirSample::DirSearch()
{
    DirSearchRequest request(Config::DriveID, "name match \"123\"", "updated_at DESC", "", 20);
    auto outcome = client->DirSearch(request);
    if (!outcome.isSuccess()) {
        PrintError(__FUNCTION__, outcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call DirSearch success: " << std::endl;
    auto fileList = outcome.result().FileItemList();
    for (uint32_t i = 0; i < fileList.size(); i++) {
        std::cout << "Type: " << fileList[i].Type() << ", FileID: " << fileList[i].FileID() <<  ", Name: " << fileList[i].Name() << std::endl;
    }
}

void DirSample::DirTrash(const std::string& fileID)
{
    DirTrashRequest request(Config::DriveID, fileID);
    auto outcome = client->DirTrash(request);
    if (!outcome.isSuccess()) {
        PrintError(__FUNCTION__, outcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call DirTrash success, task id: " << outcome.result().AsyncTaskID() << std::endl;
}

std::string DirSample::DirDelete(const std::string& fileID)
{
    DirDeleteRequest request(Config::DriveID, fileID);
    auto outcome = client->DirDelete(request);
    if (!outcome.isSuccess()) {
        PrintError(__FUNCTION__, outcome.error());
        return "";
    }
    std::cout << __FUNCTION__ << " call DirDelete success, task id: " << outcome.result().AsyncTaskID() << std::endl;
    return outcome.result().AsyncTaskID();
}

void DirSample::AsyncTaskGet(const std::string& asyncTaskID)
{
    AsyncTaskGetRequest request(asyncTaskID);
    auto outcome = client->AsyncTaskGet(request);
    if (!outcome.isSuccess()) {
        PrintError(__FUNCTION__, outcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call AsyncTaskGet success, task id: " << outcome.result().AsyncTaskID() << ", task state: " << outcome.result().State() << std::endl;
}
