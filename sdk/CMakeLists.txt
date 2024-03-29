#
# Copyright 2009-2021 Alibaba Cloud All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

project(cpp-sdk VERSION ${version})

configure_file(src/Config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/alibabacloud/pds/Config.h @ONLY)


#common header
file(GLOB sdk_auth_header "include/alibabacloud/pds/auth/*.h")
file(GLOB sdk_client_header "include/alibabacloud/pds/client/*.h")
file(GLOB sdk_http_header "include/alibabacloud/pds/http/*.h")
file(GLOB sdk_utils_header "include/alibabacloud/pds/utils/*.h")
file(GLOB sdk_model_header "include/alibabacloud/pds/model/*.h")
file(GLOB sdk_json_header "include/alibabacloud/pds/external/json/*.h")
file(GLOB sdk_public_header "include/alibabacloud/pds/*.h")

#all header
file(GLOB sdk_header
    ${sdk_auth_header}
    ${sdk_client_header}
    ${sdk_http_header}
    ${sdk_utils_header}
    ${sdk_model_header}
    ${sdk_encryption_header}
    ${sdk_public_header}
)

#common source
file(GLOB sdk_auth_src "src/auth/*.cc")
file(GLOB sdk_client_src "src/client/*.cc")
file(GLOB sdk_http_src "src/http/*.cc")
file(GLOB sdk_utils_src "src/utils/*.cc")
file(GLOB sdk_public_src "src/*.cc")

#add source by DISABLE_XX option
file(GLOB sdk_model_common_src
	"src/model/ModelError.cc"
)

file(GLOB sdk_model_object_src
    "src/model/*AsyncTask*.cc"
	"src/model/*Object*.cc"
    "src/model/*Data*.cc"
    "src/model/*Dir*.cc"
    "src/model/*File*.cc"
    "src/model/*Meta*.cc"
	"src/model/InputFormat.cc"
	"src/model/OutputFormat.cc"
)

file(GLOB sdk_model_src
	${sdk_model_common_src}
	${sdk_model_object_src}
)

file(GLOB sdk_resumable_src "src/resumable/*.cc")

#external source
file(GLOB sdk_external_tinnyxml2_src "src/external/tinyxml2/*.cpp")
file(GLOB sdk_external_json_src "src/external/json/*.cpp")


#all source
file(GLOB sdk_src
    ${sdk_auth_src}
    ${sdk_client_src}
    ${sdk_http_src}
    ${sdk_utils_src}
    ${sdk_model_src}
    ${sdk_public_src}
    ${sdk_resumable_src}
    ${sdk_external_json_src}
    ${sdk_external_tinnyxml2_src}
)

#extra define pass to source code
if (BUILD_TESTS)
    set(EXTRA_DEFINE "-DENABLE_PDS_TEST")
else()
    set(EXTRA_DEFINE "")
endif()

#static lib
add_library(${PROJECT_NAME}${STATIC_LIB_SUFFIX} STATIC
    ${sdk_header}
    ${sdk_src})

set_target_properties(${PROJECT_NAME}${STATIC_LIB_SUFFIX}
    PROPERTIES
    LINKER_LANGUAGE CXX
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    OUTPUT_NAME ${TARGET_OUTPUT_NAME_PREFIX}${PROJECT_NAME}${STATIC_LIB_SUFFIX}
    )

target_include_directories(${PROJECT_NAME}${STATIC_LIB_SUFFIX}
    PRIVATE include
    PRIVATE include/alibabacloud/pds
    PRIVATE src/external/)

target_include_directories(${PROJECT_NAME}${STATIC_LIB_SUFFIX}
    PRIVATE ${CRYPTO_INCLUDE_DIRS}
    PRIVATE ${CLIENT_INCLUDE_DIRS})

target_compile_options(${PROJECT_NAME}${STATIC_LIB_SUFFIX}
    PRIVATE "${SDK_COMPILER_FLAGS}" "${EXTRA_DEFINE}")

#shared lib
if (BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED
        ${sdk_header}
        ${sdk_src})

    set_target_properties(${PROJECT_NAME}
        PROPERTIES
        LINKER_LANGUAGE CXX
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        OUTPUT_NAME ${TARGET_OUTPUT_NAME_PREFIX}${PROJECT_NAME}
        )

    target_include_directories(${PROJECT_NAME}
        PRIVATE include
        PRIVATE include/alibabacloud/pds
        PRIVATE src/external/)

    target_include_directories(${PROJECT_NAME}
        PRIVATE ${CRYPTO_INCLUDE_DIRS}
        PRIVATE ${CLIENT_INCLUDE_DIRS})

    target_compile_options(${PROJECT_NAME}
        PRIVATE "${SDK_COMPILER_FLAGS}" -DALIBABACLOUD_SHARED -DALIBABACLOUD_PDS_LIBRARY)

    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${PROJECT_NAME}
        PRIVATE "-fvisibility=hidden")
    endif()

    target_link_libraries(${PROJECT_NAME} ${CRYPTO_LIBS})
    target_link_libraries(${PROJECT_NAME} ${CLIENT_LIBS})
endif()

#install
install(FILES ${sdk_auth_header}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alibabacloud/pds/auth)
install(FILES ${sdk_client_header}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alibabacloud/pds/client)
install(FILES ${sdk_http_header}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alibabacloud/pds/http)
install(FILES ${sdk_utils_header}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alibabacloud/pds/utils)
install(FILES ${sdk_model_header}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alibabacloud/pds/model)
install(FILES ${sdk_json_header}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alibabacloud/pds/external/json)
install(FILES ${sdk_public_header}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/alibabacloud/pds)

install(TARGETS  ${PROJECT_NAME}${STATIC_LIB_SUFFIX}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

if (BUILD_SHARED_LIBS)
install(TARGETS  ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()