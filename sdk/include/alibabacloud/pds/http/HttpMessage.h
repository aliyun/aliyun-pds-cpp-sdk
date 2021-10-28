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

#pragma once

#include <iostream>
#include <memory>
#include <alibabacloud/pds/Types.h>
#include <alibabacloud/pds/http/HttpType.h>

namespace AlibabaCloud
{
namespace PDS
{

    class ALIBABACLOUD_PDS_EXPORT HttpMessage
    {
    public:

        HttpMessage(const HttpMessage &other);
        HttpMessage(HttpMessage &&other);
        HttpMessage& operator=(const HttpMessage &other);
        HttpMessage& operator=(HttpMessage &&other);
        virtual ~HttpMessage();

        void addHeader(const std::string &name, const std::string &value);
        void setHeader(const std::string &name, const std::string &value);
        void removeHeader(const std::string &name);
        bool hasHeader(const std::string &name) const ;
        std::string Header(const std::string &name)const;
        const HeaderCollection &Headers()const;

        void addBody(const std::shared_ptr<std::iostream>& body) { body_ = body;}
        std::shared_ptr<std::iostream>& Body() { return body_;}
    protected:
        HttpMessage();
    private:
        HeaderCollection headers_;
        std::shared_ptr<std::iostream> body_;
    };
}
}
