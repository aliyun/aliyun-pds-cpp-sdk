#include <alibabacloud/pds/auth/Credentials.h>
#include <alibabacloud/pds/auth/CredentialsProvider.h>
#include <mutex>

using namespace AlibabaCloud::PDS;

class MyCredentialsProvider : public CredentialsProvider
{
public:
    MyCredentialsProvider(const std::string& accessToken);
    ~MyCredentialsProvider();

    virtual Credentials getCredentials() override;
    void setAccessToken(const std::string& accessToken);
private:
    Credentials credentials_;
    std::mutex lock_;
};
