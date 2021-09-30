#include <alibabacloud/pds/auth/Credentials.h>
#include <alibabacloud/pds/auth/CredentialsProvider.h>

using namespace AlibabaCloud::PDS;

class MyRefreshCredentialsProvider : public CredentialsProvider
{
public:
    MyRefreshCredentialsProvider(const std::string& accessToken,
        const std::string& refreshToken, uint64_t tokenExpiresTimestamp);
    ~MyRefreshCredentialsProvider();

    virtual Credentials getCredentials() override;
private:
    Credentials credentials_;
    std::string refreshToken_;
    uint64_t tokenExpiresTimestamp_; // seconds
};
