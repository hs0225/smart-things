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

#include "iotjs_module_st_getmessage.h"
#include "iotjs_module_st_magic_strings.h"

IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(sthings_get_message);
static jerry_value_t create_jprototype(jerry_value_t obj);


static iotjs_sthings_get_message_t* iotjs_sthings_get_message_create(
    jerry_value_t jobj) {
  iotjs_sthings_get_message_t* obj = IOTJS_ALLOC(iotjs_sthings_get_message_t);

  obj->jobject = jobj;
  jerry_set_object_native_pointer(jobj, obj, &this_module_native_info);

  return obj;
}

static void iotjs_sthings_get_message_destroy(
    iotjs_sthings_get_message_t* obj) {
  IOTJS_RELEASE(obj);
}

static void init_get_message(jerry_value_t jthis_obj,
                             iotjs_sthings_get_message_t* get_msg_obj) {
  iotjs_jval_set_property_string_raw(jthis_obj, IOTJS_ST_MAGIC_RESOURCEURI,
                                     get_msg_obj->msg->resource_uri);
}

jerry_value_t iotjs_sthings_jget_message_create(
    st_things_get_request_message_s* msg, jerry_value_t resource_type) {
  jerry_value_t jobj = jerry_create_object();
  jerry_value_t jprototype = create_jprototype(jobj);
  iotjs_jval_set_prototype(jobj, jprototype);
  jerry_release_value(jprototype);

  iotjs_sthings_get_message_t* obj = iotjs_sthings_get_message_create(jobj);
  obj->msg = msg;

  init_get_message(jobj, obj);

  return jobj;
}

JS_FUNCTION(GetMessage) {
  DJS_CHECK_THIS();

  jerry_value_t jthis_obj = JS_GET_THIS();
  iotjs_sthings_get_message_t* get_msg_obj =
      iotjs_sthings_get_message_create(jthis_obj);

  init_get_message(jthis_obj, get_msg_obj);

  return jerry_create_undefined();
}

JS_FUNCTION(HasPropertyKey) {
  DJS_CHECK_THIS();

  iotjs_string_t jkey_string;
  JS_GET_REQUIRED_ARG_VALUE(0, jkey_string, IOTJS_MAGIC_STRING_KEY, string);

  JS_DECLARE_THIS_PTR(sthings_get_message, get_msg);

  bool ret = get_msg->msg->has_property_key(get_msg->msg,
                                            iotjs_string_data(&jkey_string));

  iotjs_string_destroy(&jkey_string);

  return jerry_create_boolean(ret);
}

static jerry_value_t create_jprototype(jerry_value_t obj) {
  jerry_value_t prototype = jerry_create_object();
  iotjs_jval_set_property_jval(obj, IOTJS_MAGIC_STRING_PROTOTYPE, prototype);
  iotjs_jval_set_method(prototype, IOTJS_ST_MAGIC_HASPROPERTYKEY,
                        HasPropertyKey);

  return prototype;
}
