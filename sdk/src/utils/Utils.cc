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

#include "Utils.h"
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#ifdef OPENSSL_IS_BORINGSSL
#include <openssl/base64.h>
#endif
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <map>
#include <regex>
#include <iomanip>
#include <alibabacloud/pds/Const.h>
#include <alibabacloud/pds/http/HttpType.h>
#include <alibabacloud/pds/http/Url.h>
#include "../external/json/json.h"

using namespace AlibabaCloud::PDS;

#if defined(__GNUG__) && __GNUC__ < 5

#else
static const std::regex ipPattern("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\\.){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
#endif

std::string AlibabaCloud::PDS::GenerateUuid()
{
    return "";
}

std::string AlibabaCloud::PDS::UrlEncode(const std::string & src)
{
    std::stringstream dest;
    static const char *hex = "0123456789ABCDEF";
    unsigned char c;

    for (size_t i = 0; i < src.size(); i++) {
        c = src[i];
        if (isalnum(c) || (c == '-') || (c == '_') || (c == '.') || (c == '~')) {
            dest << c;
        } else if (c == ' ') {
            dest << "%20";
        } else {
            dest << '%' << hex[c >> 4] << hex[c & 15];
        }
    }

    return dest.str();
}

std::string AlibabaCloud::PDS::UrlDecode(const std::string & src)
{
    std::stringstream unescaped;
    unescaped.fill('0');
    unescaped << std::hex;

    size_t safeLength = src.size();
    const char *safe = src.c_str();
    for (auto i = safe, n = safe + safeLength; i != n; ++i)
    {
        char c = *i;
        if(c == '%')
        {
            char hex[3];
            hex[0] = *(i + 1);
            hex[1] = *(i + 2);
            hex[2] = 0;
            i += 2;
            auto hexAsInteger = strtol(hex, nullptr, 16);
            unescaped << (char)hexAsInteger;
        }
        else
        {
            unescaped << *i;
        }
    }

    return unescaped.str();
}

std::string AlibabaCloud::PDS::Base64Encode(const std::string &src)
{
    return AlibabaCloud::PDS::Base64Encode(src.c_str(), static_cast<int>(src.size()));
}

std::string AlibabaCloud::PDS::Base64Encode(const ByteBuffer& buffer)
{
    return AlibabaCloud::PDS::Base64Encode(reinterpret_cast<const char*>(buffer.data()), static_cast<int>(buffer.size()));
}

std::string AlibabaCloud::PDS::Base64Encode(const char *src, int len)
{
    if (!src || len == 0) {
        return "";
    }

    static const char *ENC = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    auto in = reinterpret_cast<const unsigned char *>(src);
    auto inLen = len;
    std::stringstream ss;
    while (inLen) {
        // first 6 bits of char 1
        ss << ENC[*in >> 2];
        if (!--inLen) {
            // last 2 bits of char 1, 4 bits of 0
            ss << ENC[(*in & 0x3) << 4];
            ss << '=';
            ss << '=';
            break;
        }
        // last 2 bits of char 1, first 4 bits of char 2
        ss << ENC[((*in & 0x3) << 4) | (*(in + 1) >> 4)];
        in++;
        if (!--inLen) {
            // last 4 bits of char 2, 2 bits of 0
            ss << ENC[(*in & 0xF) << 2];
            ss << '=';
            break;
        }
        // last 4 bits of char 2, first 2 bits of char 3
        ss << ENC[((*in & 0xF) << 2) | (*(in + 1) >> 6)];
        in++;
        // last 6 bits of char 3
        ss << ENC[*in & 0x3F];
        in++, inLen--;
    }
    return ss.str();
}

std::string AlibabaCloud::PDS::Base64EncodeUrlSafe(const std::string &src)
{
    return AlibabaCloud::PDS::Base64EncodeUrlSafe(src.c_str(), static_cast<int>(src.size()));
}

std::string AlibabaCloud::PDS::Base64EncodeUrlSafe(const char *src, int len)
{
    std::string out = AlibabaCloud::PDS::Base64Encode(src, len);

    while (out.size() > 0 && *out.rbegin() == '=')
        out.pop_back();

    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        if (c == '+') return '-';
        if (c == '/') return '_';
        return (char)c;
    });
    return out;
}

std::string AlibabaCloud::PDS::XmlEscape(const std::string& value)
{
    struct Entity {
        const char* pattern;
        char value;
    };

    static const Entity entities[] = {
        { "&quot;", '\"' },
        { "&amp;",  '&'  },
        { "&apos;", '\'' },
        { "&lt;",	'<'  },
        { "&gt;",	'>'  },
        { "&#13;",	'\r' }
    };

    if (value.empty()) {
        return value;
    }

    std::stringstream ss;
    for (size_t i = 0; i < value.size(); i++) {
        bool flag = false;
        for (size_t j = 0; j < (sizeof(entities)/sizeof(entities[0])); j++) {
            if (value[i] == entities[j].value) {
                flag = true;
                ss << entities[j].pattern;
                break;
            }
        }

        if (!flag) {
            ss << value[i];
        }
    }

    return ss.str();
}
ByteBuffer AlibabaCloud::PDS::Base64Decode(const char *data, int len)
{
    int in_len = len;
    int i = 0;
    int in_ = 0;
    unsigned char part4[4];

    const int max_len = (len * 3 / 4);
    ByteBuffer ret(max_len);
    int idx = 0;

    while (in_len-- && (data[in_] != '=')) {
        unsigned char ch = data[in_++];
        if ('A' <= ch && ch <= 'Z')  ch = ch - 'A';           // A - Z
        else if ('a' <= ch && ch <= 'z') ch = ch - 'a' + 26;  // a - z
        else if ('0' <= ch && ch <= '9') ch = ch - '0' + 52;  // 0 - 9
        else if ('+' == ch) ch = 62;                          // +
        else if ('/' == ch) ch = 63;                          // /
        else if ('=' == ch) ch = 64;                          // =
        else ch = 0xff;                                       // something wrong
        part4[i++] = ch;
        if (i == 4) {
            ret[idx++] = (part4[0] << 2) + ((part4[1] & 0x30) >> 4);
            ret[idx++] = ((part4[1] & 0xf) << 4) + ((part4[2] & 0x3c) >> 2);
            ret[idx++] = ((part4[2] & 0x3) << 6) + part4[3];
            i = 0;
        }
    }

    if (i) {
        for (int j = i; j < 4; j++)
            part4[j] = 0xFF;
        ret[idx++] = (part4[0] << 2) + ((part4[1] & 0x30) >> 4);
        if (part4[2] != 0xFF) {
            ret[idx++] = ((part4[1] & 0xf) << 4) + ((part4[2] & 0x3c) >> 2);
            if (part4[3] != 0xFF) {
                ret[idx++] = ((part4[2] & 0x3) << 6) + part4[3];
            }
        }
    }

    ret.resize(idx);
    return ret;
}

ByteBuffer AlibabaCloud::PDS::Base64Decode(const std::string &src)
{
    return Base64Decode(src.c_str(), src.size());
}


std::string AlibabaCloud::PDS::ComputeContentMD5(const std::string& data)
{
    return ComputeContentMD5(data.c_str(), data.size());
}

std::string AlibabaCloud::PDS::ComputeContentMD5(const char * data, size_t size)
{
    if (!data) {
        return "";
    }

    unsigned char md[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(data), size, (unsigned char*)&md);

    char encodedData[100];
    EVP_EncodeBlock(reinterpret_cast<unsigned char*>(encodedData), md, MD5_DIGEST_LENGTH);
    return encodedData;
}

std::string AlibabaCloud::PDS::ComputeContentMD5(std::istream& stream)
{
    auto ctx = EVP_MD_CTX_create();

    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len = 0;

    EVP_MD_CTX_init(ctx);
#ifndef OPENSSL_IS_BORINGSSL
    EVP_MD_CTX_set_flags(ctx, EVP_MD_CTX_FLAG_NON_FIPS_ALLOW);
#endif
    EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);

    auto currentPos = stream.tellg();
    if (currentPos == static_cast<std::streampos>(-1)) {
        currentPos = 0;
        stream.clear();
    }
    stream.seekg(0, stream.beg);

    char streamBuffer[2048];
    while (stream.good())
    {
        stream.read(streamBuffer, 2048);
        auto bytesRead = stream.gcount();

        if (bytesRead > 0)
        {
            EVP_DigestUpdate(ctx, streamBuffer, static_cast<size_t>(bytesRead));
        }
    }

    EVP_DigestFinal_ex(ctx, md_value, &md_len);
    EVP_MD_CTX_destroy(ctx);
    stream.clear();
    stream.seekg(currentPos, stream.beg);

    //Based64
    char encodedData[100];
    EVP_EncodeBlock(reinterpret_cast<unsigned char*>(encodedData), md_value, md_len);
    return encodedData;
}

std::string AlibabaCloud::PDS::ComputeContentSha1(const std::string& data)
{
    return ComputeContentSha1(data.c_str(), data.size());
}

std::string AlibabaCloud::PDS::ComputeContentSha1(const char * data, size_t size)
{
    if (!data) {
        return "";
    }

    unsigned char md[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(data), size, (unsigned char*)&md);

    std::ostringstream sout;
    sout<<std::hex<<std::setfill('0');
    for(auto c: md)
        sout<<std::setw(2)<<(int)c;

    return sout.str();
}

std::string AlibabaCloud::PDS::ComputeContentSha1(std::istream& stream)
{
    auto ctx = EVP_MD_CTX_create();

    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len = 0;

    EVP_MD_CTX_init(ctx);
#ifndef OPENSSL_IS_BORINGSSL
    EVP_MD_CTX_set_flags(ctx, EVP_MD_CTX_FLAG_NON_FIPS_ALLOW);
#endif
    EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr);

    auto currentPos = stream.tellg();
    if (currentPos == static_cast<std::streampos>(-1)) {
        currentPos = 0;
        stream.clear();
    }
    stream.seekg(0, stream.beg);

    char streamBuffer[2048];
    while (stream.good())
    {
        stream.read(streamBuffer, 2048);
        auto bytesRead = stream.gcount();

        if (bytesRead > 0)
        {
            EVP_DigestUpdate(ctx, streamBuffer, static_cast<size_t>(bytesRead));
        }
    }

    EVP_DigestFinal_ex(ctx, md_value, &md_len);
    EVP_MD_CTX_destroy(ctx);
    stream.clear();
    stream.seekg(currentPos, stream.beg);

    std::ostringstream sout;
    sout<<std::hex<<std::setfill('0');
    for(unsigned int idx = 0; idx < md_len; idx++)
        sout<<std::setw(2)<<(int)md_value[idx];

    return sout.str();
}

std::string AlibabaCloud::PDS::ComputeContentSha1(std::istream& stream, ProgressControl progressControl)
{
    auto ctx = EVP_MD_CTX_create();

    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len = 0;

    EVP_MD_CTX_init(ctx);
#ifndef OPENSSL_IS_BORINGSSL
    EVP_MD_CTX_set_flags(ctx, EVP_MD_CTX_FLAG_NON_FIPS_ALLOW);
#endif
    EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr);

    auto currentPos = stream.tellg();
    if (currentPos == static_cast<std::streampos>(-1)) {
        currentPos = 0;
        stream.clear();
    }
    stream.seekg(0, stream.beg);

    char streamBuffer[2048];
    uint64_t count = 0;
    while (stream.good())
    {
        count++;
        // 10MB check once
        if (count == 5120) {
            if (progressControl.Handler) {
                if (progressControl.Handler(progressControl.UserData) != 0) {
                    EVP_MD_CTX_destroy(ctx);
                    stream.clear();
                    stream.seekg(currentPos, stream.beg);
                    return "";
                }
            }
            count = 0;
        }

        stream.read(streamBuffer, 2048);
        auto bytesRead = stream.gcount();

        if (bytesRead > 0)
        {
            EVP_DigestUpdate(ctx, streamBuffer, static_cast<size_t>(bytesRead));
        }
    }

    EVP_DigestFinal_ex(ctx, md_value, &md_len);
    EVP_MD_CTX_destroy(ctx);
    stream.clear();
    stream.seekg(currentPos, stream.beg);

    std::ostringstream sout;
    sout<<std::hex<<std::setfill('0');
    for(unsigned int idx = 0; idx < md_len; idx++)
        sout<<std::setw(2)<<(int)md_value[idx];

    return sout.str();
}

static std::string HexToString(const unsigned char *data, size_t size)
{
    static char hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    std::stringstream ss;
    for (size_t i = 0; i < size; i++)
        ss << hex[(data[i] >> 4)] << hex[(data[i] & 0x0F)];
    return ss.str();
}

std::string AlibabaCloud::PDS::ComputeContentETag(const std::string& data)
{
    return ComputeContentETag(data.c_str(), data.size());
}

std::string AlibabaCloud::PDS::ComputeContentETag(const char * data, size_t size)
{
    if (!data) {
        return "";
    }
    unsigned char md[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(data), size, (unsigned char*)&md);

    return HexToString(md, MD5_DIGEST_LENGTH);
}

std::string AlibabaCloud::PDS::ComputeContentETag(std::istream& stream)
{
    auto ctx = EVP_MD_CTX_create();

    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len = 0;

    EVP_MD_CTX_init(ctx);
#ifndef OPENSSL_IS_BORINGSSL
    EVP_MD_CTX_set_flags(ctx, EVP_MD_CTX_FLAG_NON_FIPS_ALLOW);
#endif

    EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);

    auto currentPos = stream.tellg();
    if (currentPos == static_cast<std::streampos>(-1)) {
        currentPos = 0;
        stream.clear();
    }
    stream.seekg(0, stream.beg);

    char streamBuffer[2048];
    while (stream.good())
    {
        stream.read(streamBuffer, 2048);
        auto bytesRead = stream.gcount();

        if (bytesRead > 0)
        {
            EVP_DigestUpdate(ctx, streamBuffer, static_cast<size_t>(bytesRead));
        }
    }

    EVP_DigestFinal_ex(ctx, md_value, &md_len);
    EVP_MD_CTX_destroy(ctx);
    stream.clear();
    stream.seekg(currentPos, stream.beg);

    return HexToString(md_value, md_len);
}


void AlibabaCloud::PDS::StringReplace(std::string & src, const std::string & s1, const std::string & s2)
{
    std::string::size_type pos =0;
    while ((pos = src.find(s1, pos)) != std::string::npos)
    {
        src.replace(pos, s1.length(), s2);
        pos += s2.length();
    }
}

std::string AlibabaCloud::PDS::LeftTrim(const char* source)
{
    std::string copy(source);
    copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](unsigned char ch) { return !::isspace(ch); }));
    return copy;
}

std::string AlibabaCloud::PDS::RightTrim(const char* source)
{
    std::string copy(source);
    copy.erase(std::find_if(copy.rbegin(), copy.rend(), [](unsigned char ch) { return !::isspace(ch); }).base(), copy.end());
    return copy;
}

std::string AlibabaCloud::PDS::Trim(const char* source)
{
    return LeftTrim(RightTrim(source).c_str());
}

std::string AlibabaCloud::PDS::LeftTrimQuotes(const char* source)
{
    std::string copy(source);
    copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](int ch) { return !(ch == '"'); }));
    return copy;
}

std::string AlibabaCloud::PDS::RightTrimQuotes(const char* source)
{
    std::string copy(source);
    copy.erase(std::find_if(copy.rbegin(), copy.rend(), [](int ch) { return !(ch == '"'); }).base(), copy.end());
    return copy;
}

std::string AlibabaCloud::PDS::TrimQuotes(const char* source)
{
    return LeftTrimQuotes(RightTrimQuotes(source).c_str());
}

std::string AlibabaCloud::PDS::ToLower(const char* source)
{
    std::string copy;
    if (source) {
        size_t srcLength = strlen(source);
        copy.resize(srcLength);
        std::transform(source, source + srcLength, copy.begin(), [](unsigned char c) { return (char)::tolower(c); });
    }
    return copy;
}

std::string AlibabaCloud::PDS::ToUpper(const char* source)
{
    std::string copy;
    if (source) {
        size_t srcLength = strlen(source);
        copy.resize(srcLength);
        std::transform(source, source + srcLength, copy.begin(), [](unsigned char c) { return (char)::toupper(c); });
    }
    return copy;
}

std::string AlibabaCloud::PDS::ToGmtTime(std::time_t &t)
{
    std::stringstream date;
    std::tm tm;
#ifdef _WIN32
    ::gmtime_s(&tm, &t);
#else
    ::gmtime_r(&t, &tm);
#endif

#if defined(__GNUG__) && __GNUC__ < 5
    static const char wday_name[][4] = {
      "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    static const char mon_name[][4] = {
      "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    char tmbuff[26];
    snprintf(tmbuff, sizeof(tmbuff), "%.3s, %.2d %.3s %d %.2d:%.2d:%.2d",
        wday_name[tm.tm_wday], tm.tm_mday, mon_name[tm.tm_mon],
        1900 + tm.tm_year,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
    date << tmbuff << " GMT";
#else
    date.imbue(std::locale::classic());
    date << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
#endif
    return date.str();
}

std::string AlibabaCloud::PDS::ToUtcTime(std::time_t &t)
{
    std::stringstream date;
    std::tm tm;
#ifdef _WIN32
    ::gmtime_s(&tm, &t);
#else
    ::gmtime_r(&t, &tm);
#endif
#if defined(__GNUG__) && __GNUC__ < 5
    char tmbuff[26];
    strftime(tmbuff, 26, "%Y-%m-%dT%H:%M:%S.000Z", &tm);
    date << tmbuff;
#else
    date.imbue(std::locale::classic());
    date << std::put_time(&tm, "%Y-%m-%dT%X.000Z");
#endif
    return date.str();
}

std::time_t AlibabaCloud::PDS::UtcToUnixTime(const std::string &t)
{
    const char* date = t.c_str();
    std::tm tm;
    std::time_t tt = -1;
    int ms;
    auto result = sscanf(date, "%4d-%2d-%2dT%2d:%2d:%2d.%dZ",
        &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &ms);

    if (result == 7) {
        tm.tm_year = tm.tm_year - 1900;
        tm.tm_mon = tm.tm_mon - 1;
#ifdef _WIN32
        tt = _mkgmtime64(&tm);
#else
        tt = timegm(&tm);
#endif // _WIN32
    }
    return tt < 0 ? -1 : tt;
}

bool AlibabaCloud::PDS::IsValidEndpoint(const std::string &value)
{
    auto host = Url(value).host();

    if (host.empty())
        return false;

    for (const auto c : host) {
        if (!((c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c == '_') ||
            (c == '-') ||
            (c == '.'))) {
            return false;
        }
    }

    return true;
}

std::string AlibabaCloud::PDS::GetHostString(
        const std::string &endpoint)
{
    Url url(endpoint);
    if (url.scheme().empty()) {
        url.setScheme(Http::SchemeToString(Http::HTTP));
    }

    std::ostringstream out;
    out << url.scheme() << "://" << url.authority();
    return out.str();
}

const std::string& AlibabaCloud::PDS::LookupMimeType(const std::string &name)
{
    const static std::map<std::string, std::string> mimeType = {
        {"html", "text/html"},
        {"htm", "text/html"},
        {"shtml", "text/html"},
        {"css", "text/css"},
        {"xml", "text/xml"},
        {"gif", "image/gif"},
        {"jpeg", "image/jpeg"},
        {"jpg", "image/jpeg"},
        {"js", "application/x-javascript"},
        {"atom", "application/atom+xml"},
        {"rss", "application/rss+xml"},
        {"mml", "text/mathml"},
        {"txt", "text/plain"},
        {"jad", "text/vnd.sun.j2me.app-descriptor"},
        {"wml", "text/vnd.wap.wml"},
        {"htc", "text/x-component"},
        {"png", "image/png"},
        {"tif", "image/tiff"},
        {"tiff", "image/tiff"},
        {"wbmp", "image/vnd.wap.wbmp"},
        {"ico", "image/x-icon"},
        {"jng", "image/x-jng"},
        {"bmp", "image/x-ms-bmp"},
        {"svg", "image/svg+xml"},
        {"svgz", "image/svg+xml"},
        {"webp", "image/webp"},
        {"jar", "application/java-archive"},
        {"war", "application/java-archive"},
        {"ear", "application/java-archive"},
        {"hqx", "application/mac-binhex40"},
        {"doc ", "application/msword"},
        {"pdf", "application/pdf"},
        {"ps", "application/postscript"},
        {"eps", "application/postscript"},
        {"ai", "application/postscript"},
        {"rtf", "application/rtf"},
        {"xls", "application/vnd.ms-excel"},
        {"ppt", "application/vnd.ms-powerpoint"},
        {"wmlc", "application/vnd.wap.wmlc"},
        {"kml", "application/vnd.google-earth.kml+xml"},
        {"kmz", "application/vnd.google-earth.kmz"},
        {"7z", "application/x-7z-compressed"},
        {"cco", "application/x-cocoa"},
        {"jardiff", "application/x-java-archive-diff"},
        {"jnlp", "application/x-java-jnlp-file"},
        {"run", "application/x-makeself"},
        {"pl", "application/x-perl"},
        {"pm", "application/x-perl"},
        {"prc", "application/x-pilot"},
        {"pdb", "application/x-pilot"},
        {"rar", "application/x-rar-compressed"},
        {"rpm", "application/x-redhat-package-manager"},
        {"sea", "application/x-sea"},
        {"swf", "application/x-shockwave-flash"},
        {"sit", "application/x-stuffit"},
        {"tcl", "application/x-tcl"},
        {"tk", "application/x-tcl"},
        {"der", "application/x-x509-ca-cert"},
        {"pem", "application/x-x509-ca-cert"},
        {"crt", "application/x-x509-ca-cert"},
        {"xpi", "application/x-xpinstall"},
        {"xhtml", "application/xhtml+xml"},
        {"zip", "application/zip"},
        {"wgz", "application/x-nokia-widget"},
        {"bin", "application/octet-stream"},
        {"exe", "application/octet-stream"},
        {"dll", "application/octet-stream"},
        {"deb", "application/octet-stream"},
        {"dmg", "application/octet-stream"},
        {"eot", "application/octet-stream"},
        {"iso", "application/octet-stream"},
        {"img", "application/octet-stream"},
        {"msi", "application/octet-stream"},
        {"msp", "application/octet-stream"},
        {"msm", "application/octet-stream"},
        {"mid", "audio/midi"},
        {"midi", "audio/midi"},
        {"kar", "audio/midi"},
        {"mp3", "audio/mpeg"},
        {"ogg", "audio/ogg"},
        {"m4a", "audio/x-m4a"},
        {"ra", "audio/x-realaudio"},
        {"3gpp", "video/3gpp"},
        {"3gp", "video/3gpp"},
        {"mp4", "video/mp4"},
        {"mpeg", "video/mpeg"},
        {"mpg", "video/mpeg"},
        {"mov", "video/quicktime"},
        {"webm", "video/webm"},
        {"flv", "video/x-flv"},
        {"m4v", "video/x-m4v"},
        {"mng", "video/x-mng"},
        {"asx", "video/x-ms-asf"},
        {"asf", "video/x-ms-asf"},
        {"wmv", "video/x-ms-wmv"},
        {"avi", "video/x-msvideo"},
        {"ts", "video/MP2T"},
        {"m3u8", "application/x-mpegURL"},
        {"apk", "application/vnd.android.package-archive"}
        };

    const static std::string defaultType("application/octet-stream");
    std::string::size_type last_pos  = name.find_last_of('.');
    std::string::size_type first_pos = name.find_first_of('.');
    std::string prefix, ext, ext2;

    if (last_pos == std::string::npos) {
        return defaultType;
    }

    // extract the last extension
    if (last_pos != std::string::npos) {
        ext = name.substr(1 + last_pos, std::string::npos);
    }
    if (last_pos != std::string::npos) {
        if (first_pos != std::string::npos && first_pos < last_pos) {
            prefix = name.substr(0, last_pos);
            // Now get the second to last file extension
            std::string::size_type next_pos = prefix.find_last_of('.');
            if (next_pos != std::string::npos) {
                ext2 = prefix.substr(1 + next_pos, std::string::npos);
            }
        }
    }

    ext = ToLower(ext.c_str());
    auto iter = mimeType.find(ext);
    if (iter != mimeType.end()) {
        return (*iter).second;
    }

    if (first_pos == last_pos) {
        return defaultType;
    }

    ext2 = ToLower(ext2.c_str());
    iter = mimeType.find(ext2);
    if (iter != mimeType.end()) {
        return (*iter).second;
    }

    return defaultType;
}

std::streampos AlibabaCloud::PDS::GetIOStreamLength(std::iostream &stream)
{
    auto currentPos = stream.tellg();
    if (currentPos == static_cast<std::streampos>(-1)) {
        currentPos = 0;
        stream.clear();
    }
    stream.seekg(0, stream.end);
    auto streamSize = stream.tellg();
    stream.seekg(currentPos, stream.beg);
    return streamSize;
}
