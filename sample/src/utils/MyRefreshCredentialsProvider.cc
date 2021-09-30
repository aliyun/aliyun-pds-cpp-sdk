#include <iostream>
#include "MyRefreshCredentialsProvider.h"
#include "time.h"

MyRefreshCredentialsProvider::MyRefreshCredentialsProvider(const std::string& accessToken,
    const std::string& refreshToken, uint64_t tokenExpiresTimestamp) :
    CredentialsProvider(),
    credentials_(accessToken),
    refreshToken_(refreshToken),
    tokenExpiresTimestamp_(tokenExpiresTimestamp)
{
}

MyRefreshCredentialsProvider::~MyRefreshCredentialsProvider()
{
}

Credentials MyRefreshCredentialsProvider::getCredentials()
{
    // refresh sync
    time_t now = time(NULL);
    if ((now - tokenExpiresTimestamp_) < 120) {
        // call RefreshToken
        // update refreshToken_, tokenExpiresTimestamp_
        // credentials_.setAccessToken(newAccessToken);
    }
    return credentials_;
}

// refresh async in thread, update by credentials_.setAccessToken(newAccessToken);