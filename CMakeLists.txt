# Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# This is a standalone shared libray which
# only requires the iotjs and jerry header file(s).
#
cmake_minimum_required(VERSION 2.8)

set(MODULE_NAME smartthings_native)
set(CMAKE_C_FLAGS "-fPIE -fno-builtin -Wno-unused-parameter")
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv7l)
set(CMAKE_C_COMPILER arm-linux-gnueabi-gcc)

add_definitions("-D__TIZEN__ -DENABLE_DEBUG_LOG")

file(GLOB SRC_DIR src/*.c)
set(INC_DIR)
set(GBS_ROOT_USR "~/GBS-ROOT/local/BUILD-ROOTS/scratch.armv7l.0/usr")
set(GBS_ROOT_IOTJS "~/GBS-ROOT/local/BUILD-ROOTS/scratch.armv7l.0/home/abuild/rpmbuild/BUILD/iotjs-1.0.0")


list(APPEND INC_DIR ${PROJECT_SOURCE_DIR}/src)
list(APPEND INC_DIR ${PROJECT_SOURCE_DIR}/inc)
list(APPEND INC_DIR ${GBS_ROOT_USR}/include)
list(APPEND INC_DIR ${GBS_ROOT_USR}/include/appfw)
list(APPEND INC_DIR ${GBS_ROOT_USR}/include/dlog)
list(APPEND INC_DIR ${GBS_ROOT_USR}/include/glib-2.0)
list(APPEND INC_DIR ${GBS_ROOT_USR}/lib/glib-2.0/include)

list(APPEND INC_DIR ${GBS_ROOT_IOTJS}/deps/jerry/jerry-core/include)
list(APPEND INC_DIR  ${GBS_ROOT_IOTJS}/src)

add_library(${MODULE_NAME} SHARED ${SRC_DIR})

find_library(LIBAPPFW NAMES capi-appfw-app-common PATHS ${GBS_ROOT_USR}/lib)
find_library(LIBOICDASTACK NAMES oicdastack PATH ${PROJECT_SOURCE_DIR}/external)
find_library(LIBSDKAPI NAMES sdkapi PATH ${PROJECT_SOURCE_DIR}/external)

target_link_libraries(${MODULE_NAME} pthread m rt ${LIBAPPFW} ${LIBOICDASTACK} ${LIBSDKAPI})
target_include_directories(${MODULE_NAME}
    PRIVATE ${INC_DIR})
set_target_properties(${MODULE_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".iotjs")