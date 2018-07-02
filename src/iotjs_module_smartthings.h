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

#ifndef IOTJS_MODULE_SMART_THINGS_H
#define IOTJS_MODULE_SMART_THINGS_H

#include "iotjs_def.h"
#include "iotjs_module_st_representation.h"
#include "iotjs_reqwrap.h"

#include <semaphore.h>

// typedef enum {  };
typedef enum { kSTAsyncGetRequest, kSTAsyncSetRequest, kSTAsyncEnd } STAsync;
typedef enum { kSTNotifyObserver} STOp;

typedef struct {
  uv_async_t async;
  uv_sem_t semp;
  void *data;
} iotjs_sthings_async_t;

typedef struct {
  bool is_init;
  bool is_loop_lock;
  jerry_value_t jsthings;
  iotjs_sthings_async_t async[kSTAsyncEnd];
  uv_loop_t *loop;
} iotjs_sthings_t;

typedef struct {
  bool result;
  st_things_representation_s *resp_rep;
  void *message;
} iotjs_sthings_reqwrap_t;

typedef struct {
  iotjs_string_t pin_data;
  size_t pin_size;
} iotjs_sthings_pin_data_t;

typedef struct {
  uv_req_t *request;
  uv_work_t req;
  STOp op;
  bool result;
  void *data;
} iotjs_sthings_queue_t;

iotjs_sthings_t *get_sthings_data();

#endif
