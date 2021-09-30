#include <alibabacloud/pds/PdsClient.h>

class ShareLinkSample
{
public:
    ShareLinkSample();
    ~ShareLinkSample();

    void FileDownload(const std::string& fileID);
    void ResumableFileDownload(const std::string& fileID);
    void ResumableFileDownloadStopOnce(const std::string& fileID);
    void ResumableFileDownloadCancel(const std::string& fileID);

private:
    void PrintError(const std::string &funcName, const AlibabaCloud::PDS::PdsError &error);
    AlibabaCloud::PDS::PdsClient *client;
    std::string bucket_;
};
