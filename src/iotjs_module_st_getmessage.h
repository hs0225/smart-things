/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IOTJS_MODULE_SMART_THINGS_GET_MESSAGE_H
#define IOTJS_MODULE_SMART_THINGS_GET_MESSAGE_H

#include <st_things_types.h>

#include "iotjs_def.h"
#include "iotjs_reqwrap.h"


typedef struct {
  jerry_value_t jobject;
  st_things_get_request_message_s* msg;
} iotjs_sthings_get_message_t;

jerry_value_t iotjs_sthings_jget_message_create(
    st_things_get_request_message_s* msg, jerry_value_t resource_type);

#endif
