#
# Copyright 2009-2017 Alibaba Cloud All rights reserved.
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

cmake_minimum_required(VERSION 3.1)

cmake_policy(SET CMP0048 NEW)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(cpp-sdk-sample VERSION ${version})

file(GLOB sample_src "src/*")
file(GLOB sample_dir_src "src/dir/*")
file(GLOB sample_file_src "src/file/*")
file(GLOB sample_resumable_src "src/resumable/*")

add_executable(${PROJECT_NAME}
	${sample_src}
	${sample_dir_src}
	${sample_file_src}
	${sample_resumable_src})

target_include_directories(${PROJECT_NAME}
	PRIVATE ${CMAKE_SOURCE_DIR}/sdk/include)

target_link_libraries(${PROJECT_NAME} cpp-sdk)
target_link_libraries(${PROJECT_NAME} ${CRYPTO_LIBS})
target_link_libraries(${PROJECT_NAME} ${CLIENT_LIBS})
if (${TARGET_OS} STREQUAL "LINUX")
target_link_libraries(${PROJECT_NAME} pthread)
endif()

target_compile_options(${PROJECT_NAME}
	PRIVATE "${SDK_COMPILER_FLAGS}")

install(TARGETS  ${PROJECT_NAME}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)