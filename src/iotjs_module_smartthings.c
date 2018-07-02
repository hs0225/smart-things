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

#include "iotjs_module_smartthings.h"
#include "iotjs_module_st_getmessage.h"
#include "iotjs_module_st_magic_strings.h"
#include "iotjs_module_st_setmessage.h"

#include <things_types.h>
#include <unistd.h>
#if defined(__TIZEN__)
#include <app_common.h>
#include <glib.h>
#endif

static iotjs_sthings_t sthings_data = {.is_init = false };
#if defined(__TIZEN__)
GMainLoop* easysetup_loop;
#endif

static iotjs_sthings_async_t* request_async_call(void* data,
                                                 uv_async_cb async_cb) {
  iotjs_sthings_async_t* async = IOTJS_ALLOC(iotjs_sthings_async_t);
  async->data = data;
  async->async.data = async;

  uv_async_init(sthings_data.loop, &async->async, async_cb);
  uv_async_send(&async->async);

  return async;
}

static void close_async_call(iotjs_sthings_async_t* async) {
  uv_close((uv_handle_t*)&async->async, NULL);
  IOTJS_RELEASE(async);
}

static void wait_async_call(iotjs_sthings_async_t* async) {
  uv_sem_init(&async->semp, 0);
  uv_sem_wait(&async->semp);
}

static void wake_async_call(iotjs_sthings_async_t* async) {
  uv_sem_post(&async->semp);
  uv_sem_destroy(&async->semp);
}

iotjs_sthings_t* get_sthings_data() {
  return &sthings_data;
}

/*
 * Handle set/get Request
 */
static bool handle_st_request(STAsync op, void* req_msg,
                              st_things_representation_s* resp_rep) {
  DDDLOG("[ST] %s(%d) IN", __func__, op);

  if (!sthings_data.is_init) {
    DLOG("[ST] not initialize");
    return false;
  }

  iotjs_sthings_reqwrap_t* req_wrap = IOTJS_ALLOC(iotjs_sthings_reqwrap_t);
  req_wrap->message = req_msg;
  req_wrap->resp_rep = resp_rep;

  iotjs_sthings_async_t* async = &sthings_data.async[op];
  async->async.data = req_wrap;

  uv_async_send(&async->async);
  uv_sem_wait(&async->semp);

  IOTJS_RELEASE(req_wrap);

  DDDLOG("[ST] %s OUT", __func__);
  return true;
}

static bool handle_st_get_request(st_things_get_request_message_s* req_msg,
                                  st_things_representation_s* resp_rep) {
  return handle_st_request(kSTAsyncGetRequest, req_msg, resp_rep);
}

static bool handle_st_set_request(st_things_set_request_message_s* req_msg,
                                  st_things_representation_s* resp_rep) {
  return handle_st_request(kSTAsyncSetRequest, req_msg, resp_rep);
}

static void emit_request(const char* event_name, jerry_value_t jmsg,
                         jerry_value_t jrep) {
  jerry_value_t jemit =
      iotjs_jval_get_property(sthings_data.jsthings, IOTJS_MAGIC_STRING_EMIT);

  jerry_value_t jstr = jerry_create_string((const jerry_char_t*)event_name);

  jerry_value_t jargs[] = { jstr, jmsg, jrep };

  DDDLOG("[ST] %s, Emit request event(%s)", __func__, event_name);
  jerry_value_t jres =
      jerry_call_function(jemit, sthings_data.jsthings, jargs, 3);

  jerry_release_value(jres);
  jerry_release_value(jstr);
  jerry_release_value(jemit);
}

#define REQUEST_CALLBACK(op, Op, OP)                                           \
  do {                                                                         \
    iotjs_sthings_reqwrap_t* req_wrap = (iotjs_sthings_reqwrap_t*)async->data; \
    jerry_value_t jresources =                                                 \
        iotjs_jval_get_property(sthings_data.jsthings,                         \
                                IOTJS_ST_MAGIC__RESOURCES);                    \
    jerry_value_t jresourceType =                                              \
        iotjs_jval_get_property(sthings_data.jsthings,                         \
                                IOTJS_ST_MAGIC__RESOURCETYPE);                 \
    st_things_##op##_request_message_s* message =                              \
        (st_things_##op##_request_message_s*)req_wrap->message;                \
    DDDLOG("[ST] %s - %s", __func__, message->resource_uri);                   \
                                                                               \
    jerry_value_t jresource_type =                                             \
        iotjs_jval_get_property(jresources, message->resource_uri);            \
    IOTJS_ASSERT(!jerry_value_is_undefined(jresource_type));                   \
    iotjs_string_t resource_str = iotjs_jval_as_string(jresource_type);        \
    jerry_value_t jresource_type_obj =                                         \
        iotjs_jval_get_property(jresourceType,                                 \
                                iotjs_string_data(&resource_str));             \
    IOTJS_ASSERT(!jerry_value_is_undefined(jresource_type_obj));               \
                                                                               \
    jerry_value_t jmessage =                                                   \
        iotjs_sthings_j##op##_message_create(message, jresource_type_obj);     \
    jerry_value_t jrep =                                                       \
        iotjs_sthings_jrep_create(req_wrap->resp_rep, jresource_type_obj);     \
                                                                               \
    emit_request(IOTJS_ST_MAGIC_##OP##REQUEST, jmessage, jrep);                \
                                                                               \
    jerry_release_value(jmessage);                                             \
    jerry_release_value(jrep);                                                 \
    jerry_release_value(jresource_type_obj);                                   \
    iotjs_string_destroy(&resource_str);                                       \
                                                                               \
    jerry_release_value(jresources);                                           \
    jerry_release_value(jresourceType);                                        \
    uv_sem_post(&sthings_data.async[kSTAsync##Op##Request].semp);              \
  } while (0);

static void async_get_request_callback(uv_async_t* async) {
  REQUEST_CALLBACK(get, Get, GET)
}

static void async_set_request_callback(uv_async_t* async) {
  REQUEST_CALLBACK(set, Set, SET)
}


/*
 * Handle reset request
 */
static void async_reset_request(uv_async_t* async);
static void async_reset_result(uv_async_t* _async);

bool handle_reset_request(void) {
  DDDLOG("[ST] %s, ", __func__);
  bool data = true;
  iotjs_sthings_async_t* async =
      request_async_call((void*)&data, async_reset_request);
  wait_async_call(async);

  DLOG("[ST] Received a reset request: %s", (data ? "true" : "false"));

  return data;
  return false;
}

static void async_reset_request(uv_async_t* _async) {
  DDDLOG("[ST] %s", __func__);
  iotjs_sthings_async_t* async = _async->data;

  jerry_value_t jreset_request =
      iotjs_jval_get_property(sthings_data.jsthings,
                              IOTJS_ST_MAGIC__RESETREQUEST);
  IOTJS_ASSERT(jerry_value_is_function(jreset_request));

  jerry_value_t jres =
      jerry_call_function(jreset_request, sthings_data.jsthings, NULL, 0);
  IOTJS_ASSERT(!jerry_value_is_error(jres));
  bool* data = (bool*)async->data;

  *data = iotjs_jval_as_boolean(jres);

  jerry_release_value(jres);

  wake_async_call(async);
  close_async_call(async);
}

void handle_reset_result(bool result) {
  DDDLOG("[ST] %s", __func__);
  bool* data = IOTJS_ALLOC(bool);
  *data = result;
  request_async_call((void*)data, async_reset_result);
}

static void async_reset_result(uv_async_t* _async) {
  DDDLOG("[ST] %s", __func__);

  iotjs_sthings_async_t* async = _async->data;
  bool* data = async->data;
  jerry_value_t jemit =
      iotjs_jval_get_property(sthings_data.jsthings, IOTJS_MAGIC_STRING_EMIT);
  IOTJS_ASSERT(jerry_value_is_function(jemit));
  jerry_value_t jstr =
      jerry_create_string((const jerry_char_t*)IOTJS_ST_MAGIC_RESETRESULT);
  jerry_value_t jdata = jerry_create_boolean(*data);

  jerry_value_t jargs[] = { jstr, jdata };

  jerry_value_t jres =
      jerry_call_function(jemit, sthings_data.jsthings, jargs, 2);

  jerry_release_value(jres);
  jerry_release_value(jdata);
  jerry_release_value(jstr);
  jerry_release_value(jemit);

  IOTJS_RELEASE(async->data);
  close_async_call(async);
}

/*
 * Handle user confirm request
 */
static void async_user_confirm_callback(uv_async_t* async);

bool handle_user_confirm_request(void) {
  DDDLOG("[ST] %s, ", __func__);
  bool data = true;
  iotjs_sthings_async_t* async =
      request_async_call((void*)&data, async_user_confirm_callback);

  DDDLOG("[ST] Wait a user confirm request");
  wait_async_call(async);
  DDDLOG("[ST] Received a user confirm request: %s", (data ? "true" : "false"));

  return data;
  return true;
}

static void async_user_confirm_callback(uv_async_t* _async) {
  DDDLOG("[ST] %s", __func__);
  iotjs_sthings_async_t* async = _async->data;
  jerry_value_t juser_confirm =
      iotjs_jval_get_property(sthings_data.jsthings,
                              IOTJS_ST_MAGIC__USERCONFIRMREQUEST);
  IOTJS_ASSERT(jerry_value_is_function(juser_confirm));

  jerry_value_t jres =
      jerry_call_function(juser_confirm, sthings_data.jsthings, NULL, 0);
  IOTJS_ASSERT(!jerry_value_is_error(jres));
  bool* data = (bool*)async->data;

  *data = iotjs_jval_as_boolean(jres);

  jerry_release_value(jres);

  wake_async_call(async);
}

/*
 * Handle things status change
 */
static void async_status_change(uv_async_t* _async);

void handle_things_status_change(st_things_status_e things_status) {
  DDDLOG("[ST] Things status is changed: %d", things_status);

#if defined(__TIZEN__)
  if (things_status == ES_STATE_REGISTERED_TO_CLOUD) {
    g_main_loop_quit(easysetup_loop);
    g_main_loop_unref(easysetup_loop);
    DDDLOG("[ST] End easy setup loop.");
  }
#endif
  st_things_status_e* status = IOTJS_ALLOC(st_things_status_e);
  *status = things_status;
  request_async_call((void*)status, async_status_change);
}

static void async_status_change(uv_async_t* _async) {
  iotjs_sthings_async_t* async = _async->data;
  st_things_status_e* status = async->data;

  jerry_value_t jemit =
      iotjs_jval_get_property(sthings_data.jsthings, IOTJS_MAGIC_STRING_EMIT);
  IOTJS_ASSERT(jerry_value_is_function(jemit));
  jerry_value_t str =
      jerry_create_string((const jerry_char_t*)IOTJS_ST_MAGIC_STATUSCHANGE);
  jerry_value_t jstatus = jerry_create_number(*status);
  jerry_value_t jargs[] = { str, jstatus };

  jerry_value_t jres =
      jerry_call_function(jemit, sthings_data.jsthings, jargs, 2);

  jerry_release_value(str);
  jerry_release_value(jres);
  jerry_release_value(jstatus);
  jerry_release_value(jemit);

#if defined(__TIZEN__)
  if (*status == ES_STATE_REGISTERING_TO_CLOUD) {
    DDDLOG("[ST] %s, gmain loop run", __func__);
    g_main_loop_run(easysetup_loop); // Blocks until loop is quit.
    DDDLOG("[ST] %s, gmain loop quit", __func__);
  }
#endif

  IOTJS_RELEASE(status);
  close_async_call(async);
}

/*
 * Handle pin generation
 */
static void async_pin_generation(uv_async_t* _async);
static void async_pin_display_close(uv_async_t* _async);

static void handle_pin_generation(const char* pin_data, const size_t pin_size) {
  DDDLOG("[ST] %s, pin_data(%s), pin_size(%d)", __func__, pin_data, pin_size);

  iotjs_sthings_pin_data_t* data = IOTJS_ALLOC(iotjs_sthings_pin_data_t);
  data->pin_data =
      iotjs_string_create_with_size(pin_data, strlen(pin_data) + 1);
  data->pin_size = pin_size;

  request_async_call((void*)data, async_pin_generation);
}

static void async_pin_generation(uv_async_t* _async) {
  iotjs_sthings_async_t* async = _async->data;
  iotjs_sthings_pin_data_t* data = async->data;
  jerry_value_t jemit =
      iotjs_jval_get_property(sthings_data.jsthings, IOTJS_MAGIC_STRING_EMIT);
  IOTJS_ASSERT(jerry_value_is_function(jemit));

  jerry_value_t jstr =
      jerry_create_string((const jerry_char_t*)IOTJS_ST_MAGIC_PINGENERATION);
  jerry_value_t jpin_data = jerry_create_string(
      (const jerry_char_t*)iotjs_string_data(&data->pin_data));
  jerry_value_t jpin_size = jerry_create_number(data->pin_size);

  jerry_value_t jargs[] = { jstr, jpin_data, jpin_size };

  jerry_value_t jres =
      jerry_call_function(jemit, sthings_data.jsthings, jargs, 3);

  jerry_release_value(jstr);
  jerry_release_value(jpin_data);
  jerry_release_value(jpin_size);
  jerry_release_value(jres);
  jerry_release_value(jemit);

  iotjs_string_destroy(&data->pin_data);

  IOTJS_RELEASE(async->data);
  close_async_call(async);
}

void handle_things_pin_display_close() {
  DDDLOG("[ST] %s", __func__);

  // request_async_call(NULL, async_pin_display_close);
}

static void async_pin_display_close(uv_async_t* _async) {
  iotjs_sthings_async_t* async = _async->data;
  jerry_value_t jemit =
      iotjs_jval_get_property(sthings_data.jsthings, IOTJS_MAGIC_STRING_EMIT);

  jerry_value_t jstr =
      jerry_create_string((const jerry_char_t*)IOTJS_ST_MAGIC_PINDISPLAYCLOSE);
  jerry_value_t jargs[] = { jstr };

  jerry_value_t jres =
      jerry_call_function(jemit, sthings_data.jsthings, jargs, 1);

  jerry_release_value(jres);
  jerry_release_value(jstr);
  jerry_release_value(jemit);

  IOTJS_RELEASE(async->data);
  close_async_call(async);
}

/*
 * Worker
 */
static void st_worker(uv_work_t* work_req) {
  iotjs_sthings_queue_t* req_wrap = (iotjs_sthings_queue_t*)work_req->data;
  STOp op = req_wrap->op;
  DDDLOG("[ST] %s, operation(%d)", __func__, op);

  if (op == kSTNotifyObserver) {
    if (st_things_notify_observers(req_wrap->data) != ST_THINGS_ERROR_NONE) {
      DLOG("[ST] Failed notifyObservers");
      req_wrap->result = false;
    } else {
      req_wrap->result = true;
    }
  }
}

static void st_after_worker(uv_work_t* work_req, int status) {
  iotjs_sthings_queue_t* req_wrap = (iotjs_sthings_queue_t*)work_req->data;
  STOp op = req_wrap->op;

  if (op == kSTNotifyObserver) {
    IOTJS_RELEASE(req_wrap->data);
  }

  IOTJS_RELEASE(req_wrap);
}


static bool init_things(const char* json_path) {
  static bool binitialized = false;
  if (binitialized) {
    DDLOG("[ST] Already initialized!!");
    return false;
  }
  bool easysetup_complete = false;
#if defined(__TIZEN__)
  char app_json_path[128] = {
    0,
  };
  char* app_res_path = NULL;
  char* app_data_path = NULL;


  app_res_path = app_get_resource_path();
  if (!app_res_path) {
    DLOG("[ST] app_res_path is NULL!!");
    return false;
  }

  app_data_path = app_get_data_path();
  if (!app_data_path) {
    DLOG("[ST] app_data_path is NULL!!");
    free(app_res_path);
    return false;
  }

  snprintf(app_json_path, sizeof(app_json_path), "%s%s", app_res_path,
           json_path);

  if (st_things_set_configuration_prefix_path((const char*)app_res_path,
                                              (const char*)app_data_path) !=
      0) {
    DLOG("[ST] st_things_set_configuration_prefix_path() failed!!");
    free(app_res_path);
    free(app_data_path);
    return false;
  }

  free(app_res_path);
  free(app_data_path);


  int res = st_things_initialize(app_json_path, &easysetup_complete);
#elif defined(__TIZENRT__)
  int res = st_things_initialize(json_path, &easysetup_complete);
#endif
  if (res != 0) {
    DLOG("[ST] st_things_initialize failed(%d)!!", res);
    return false;
  }

  // Set smart things callback
  // Both callback function execute in the same thread.
  st_things_register_request_cb(handle_st_get_request, handle_st_set_request);
  st_things_register_reset_cb(handle_reset_request, handle_reset_result);
  st_things_register_user_confirm_cb(handle_user_confirm_request);
  st_things_register_things_status_change_cb(handle_things_status_change);
  st_things_register_pin_handling_cb(handle_pin_generation,
                                     handle_things_pin_display_close);

  if (st_things_start() == ST_THINGS_ERROR_NONE) {
    DDDLOG("[ST] %s, === Start Smart Things Stack ===", __func__);
  } else {
    DLOG("[ST] %s, !!! Cannot start Smart Things Stack !!!");
    return false;
  }

  return true;
}

JS_FUNCTION(SmartThings) {
  DJS_CHECK_THIS();
  sthings_data.jsthings = JS_GET_THIS();
  sthings_data.loop = iotjs_environment_loop(iotjs_environment_get());

  st_things_register_request_cb(handle_st_get_request, handle_st_set_request);
  st_things_register_reset_cb(handle_reset_request, handle_reset_result);
  st_things_register_user_confirm_cb(handle_user_confirm_request);
  st_things_register_things_status_change_cb(handle_things_status_change);
  st_things_register_pin_handling_cb(handle_pin_generation,
                                     handle_things_pin_display_close);

  return jerry_create_undefined();
}

JS_FUNCTION(Start) {
  DJS_CHECK_ARGS(1, object);
  DDDLOG("[ST] %s, Start SmartThings", __func__);
  sthings_data.is_init = true;

  // Initialize uv-async
  uv_loop_t* iotjs_loop = iotjs_environment_loop(iotjs_environment_get());

  uv_async_cb async_cb[kSTAsyncEnd] = { async_get_request_callback,
                                        async_set_request_callback };

  for (int i = 0; i < kSTAsyncEnd; i++) {
    iotjs_sthings_async_t* async = &sthings_data.async[i];
    uv_sem_init(&async->semp, 0);
    uv_async_init(iotjs_loop, &async->async, async_cb[i]);
  }

  // Create gmain loop for easy setup.
  jerry_value_t jconfig = JS_GET_ARG(0, object);
  jerry_value_t jdeviceDefinition =
      iotjs_jval_get_property(jconfig, IOTJS_ST_MAGIC_DEVICEDEFINITION);

  iotjs_string_t path = iotjs_jval_as_string(jdeviceDefinition);
#if defined(__TIZEN__)
  easysetup_loop = g_main_loop_new(g_main_context_default(), FALSE);
#endif

  init_things(iotjs_string_data(&path));

  jerry_release_value(jdeviceDefinition);
  iotjs_string_destroy(&path);

  return jerry_create_undefined();
}

JS_FUNCTION(GetResPath) {
#if defined(__TIZEN__)
  return jerry_create_string((const jerry_char_t*)app_get_resource_path());
#else
  return jerry_create_undefined();
#endif
}

JS_FUNCTION(Stop) {
  DLOG("[ST] %s, Stop Smart Things\n", __func__);
  st_things_deinitialize();
#if defined(__TIZEN__)
  st_things_stop();
#endif

  jerry_release_value(sthings_data.jsthings);

  sthings_data.is_init = false;

  for (int i = 0; i < kSTAsyncEnd; i++) {
    iotjs_sthings_async_t* async = &sthings_data.async[i];
    uv_close((uv_handle_t*)&async->async, NULL);
    uv_sem_destroy(&async->semp);
  }

  return jerry_create_undefined();
}

JS_FUNCTION(NotifyObservers) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, string);
  iotjs_string_t res = JS_GET_ARG(0, string);
  const char *data = iotjs_string_data(&res);

  iotjs_sthings_queue_t* queue_wrap = IOTJS_ALLOC(iotjs_sthings_queue_t);
  queue_wrap->op = kSTNotifyObserver;
  size_t data_length = strlen(data);
  queue_wrap->data = IOTJS_CALLOC(data_length + 1, char);
  strncpy(queue_wrap->data, data, data_length + 1);

  queue_wrap->request = (uv_req_t*)&queue_wrap->req;
  queue_wrap->request->data = queue_wrap;

  uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());
  uv_queue_work(loop, &queue_wrap->req, st_worker, st_after_worker);

  iotjs_string_destroy(&res);
  return jerry_create_undefined();
}

JS_FUNCTION(Reset) {
  DJS_CHECK_THIS();

  st_things_reset();

  return jerry_create_undefined();
}

jerry_value_t init_smartthings_native() {
  jerry_value_t smart_things = jerry_create_external_function(SmartThings);

  jerry_value_t prototype = jerry_create_object();
  iotjs_jval_set_property_jval(smart_things, IOTJS_MAGIC_STRING_PROTOTYPE,
                               prototype);
  iotjs_jval_set_method(prototype, IOTJS_ST_MAGIC__START, Start);
  iotjs_jval_set_method(prototype, IOTJS_ST_MAGIC__GETRESPATH, GetResPath);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_STOP, Stop);
  iotjs_jval_set_method(prototype, IOTJS_ST_MAGIC_NOTIFYOBSERVERS,
                        NotifyObservers);
  iotjs_jval_set_method(prototype, IOTJS_ST_MAGIC_RESET, Reset);

  jerry_release_value(prototype);

  return smart_things;
}

#if defined(__TIZEN__)
IOTJS_MODULE(IOTJS_CURRENT_MODULE_VERSION, 1, smartthings_native);
#endif