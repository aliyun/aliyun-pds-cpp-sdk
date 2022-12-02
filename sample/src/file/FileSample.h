#include <alibabacloud/pds/PdsClient.h>

class FileSample
{
public:
    FileSample();
    ~FileSample();

    std::string FileCreate();
    void FileDownload(const std::string& fileID);
    void FileGet(const std::string& fileID);
    void FileGetByPath(const std::string& fileID);
    void FileRename(const std::string& fileID);
    void FileTrash(const std::string& fileID);
    void FileDelete(const std::string& fileID);
    void FileGetVideoPreviewPlayInfo(const std::string& fileID);
    void UserTagsPut(const std::string& fileID);
    void UserTagsDelete(const std::string& fileID);

private:
    void PrintError(const std::string &funcName, const AlibabaCloud::PDS::PdsError &error);
    AlibabaCloud::PDS::PdsClient *client;
    std::string bucket_;
};
