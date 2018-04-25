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

#include "iotjs_module_st_setmessage.h"

IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(sthings_set_message);


static iotjs_sthings_set_message_t* iotjs_sthings_set_message_create(
    jerry_value_t jobj) {
  iotjs_sthings_set_message_t* obj = IOTJS_ALLOC(iotjs_sthings_set_message_t);
  obj->jobject = jobj;

  jerry_set_object_native_pointer(jobj, obj, &this_module_native_info);
  return obj;
}

static void iotjs_sthings_set_message_destroy(
    iotjs_sthings_set_message_t* obj) {
  jerry_release_value(obj->jrep);
  IOTJS_RELEASE(obj);
}

static void init_set_message(jerry_value_t jthis_obj,
                             iotjs_sthings_set_message_t* set_msg_obj) {
  iotjs_jval_set_property_string_raw(jthis_obj, IOTJS_ST_MAGIC_RESOURCEURI,
                                     set_msg_obj->msg->resource_uri);

  iotjs_jval_set_property_jval(jthis_obj, IOTJS_ST_MAGIC_REP,
                               set_msg_obj->jrep);
}

jerry_value_t iotjs_sthings_jset_message_create(
    st_things_set_request_message_s* msg, jerry_value_t resource_type) {
  jerry_value_t jobj = jerry_create_object();

  iotjs_sthings_set_message_t* obj = iotjs_sthings_set_message_create(jobj);
  obj->msg = msg;
  obj->jrep = iotjs_sthings_jrep_create(msg->rep, resource_type);

  init_set_message(jobj, obj);
  return jobj;
}

JS_FUNCTION(setMessage) {
  DJS_CHECK_THIS();

  jerry_value_t jthis_obj = JS_GET_THIS();
  iotjs_sthings_set_message_t* set_msg_obj =
      iotjs_sthings_set_message_create(jthis_obj);

  init_set_message(jthis_obj, set_msg_obj);

  return jerry_create_undefined();
}
