#include "MyCredentialsProvider.h"

MyCredentialsProvider::MyCredentialsProvider(const std::string &accessToken) :
    CredentialsProvider(),
    credentials_(accessToken)
{
}

MyCredentialsProvider::~MyCredentialsProvider()
{
}

Credentials MyCredentialsProvider::getCredentials()
{
    std::lock_guard<std::mutex> lck(lock_);
    return credentials_;
}

void MyCredentialsProvider::setAccessToken(const std::string& accessToken)
{
    std::lock_guard<std::mutex> lck(lock_);
    return credentials_.setAccessToken(accessToken);
}