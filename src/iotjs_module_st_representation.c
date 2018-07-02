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

#include "iotjs_module_st_representation.h"
#include "iotjs_module_st_magic_strings.h"

IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(sthings_rep);
static jerry_value_t create_js_prototype(jerry_value_t obj);
static bool req_set_data(int type, const char* key, jerry_value_t jdata,
                         iotjs_sthings_rep_t* st);


static iotjs_sthings_rep_t* iotjs_sthings_rep_create(jerry_value_t jobj) {
  iotjs_sthings_rep_t* obj = IOTJS_ALLOC(iotjs_sthings_rep_t);
  obj->jobject = jobj;
  jerry_set_object_native_pointer(jobj, obj, &this_module_native_info);

  return obj;
}

static void iotjs_sthings_rep_destroy(iotjs_sthings_rep_t* obj) {
  IOTJS_RELEASE(obj);
}

jerry_value_t iotjs_sthings_jrep_create(st_things_representation_s* rep,
                                        jerry_value_t resource_type) {
  jerry_value_t jobj = jerry_create_object();
  jerry_value_t jprototype = create_js_prototype(jobj);
  iotjs_jval_set_prototype(jobj, jprototype);
  jerry_release_value(jprototype);

  iotjs_sthings_rep_t* obj = iotjs_sthings_rep_create(jobj);
  obj->rep = rep;
  obj->jresource_type = resource_type;

  return jobj;
}

static jerry_value_t req_get_string(const char* key, iotjs_sthings_rep_t* st) {
  char* data;
  st->rep->get_str_value(st->rep, key, &data);

  return jerry_create_string((const jerry_char_t*)data);
}

static bool req_set_string(const char* key, jerry_value_t jdata,
                           iotjs_sthings_rep_t* st) {
  iotjs_string_t jvalue_raw = iotjs_jval_as_string(jdata);
  const char* jvalue_string_raw = iotjs_string_data(&jvalue_raw);

  DDDLOG("%s, key(%s), value(%s)", __func__, key, jvalue_string_raw);
  bool ret = st->rep->set_str_value(st->rep, key, jvalue_string_raw);
  iotjs_string_destroy(&jvalue_raw);

  return ret;
}

static jerry_value_t req_get_bool(const char* key, iotjs_sthings_rep_t* st) {
  bool data;
  st->rep->get_bool_value(st->rep, key, &data);

  return jerry_create_boolean(data);
}

static bool req_set_bool(const char* key, jerry_value_t jdata,
                         iotjs_sthings_rep_t* st) {
  bool data = jerry_get_boolean_value(jdata);
  return st->rep->set_bool_value(st->rep, key, data);
}

static jerry_value_t req_get_int(const char* key, iotjs_sthings_rep_t* st) {
  int64_t data;
  st->rep->get_int_value(st->rep, key, &data);

  return jerry_create_number((double)data);
}

static bool req_set_int(const char* key, jerry_value_t jdata,
                        iotjs_sthings_rep_t* st) {
  double number_data = iotjs_jval_as_number(jdata);
  return st->rep->set_int_value(st->rep, key, (int64_t)number_data);
}

static jerry_value_t req_get_double(const char* key, iotjs_sthings_rep_t* st) {
  double data;
  st->rep->get_double_value(st->rep, key, &data);

  return jerry_create_number(data);
}

static bool req_set_double(const char* key, jerry_value_t jdata,
                           iotjs_sthings_rep_t* st) {
  double number_data = iotjs_jval_as_number(jdata);
  return st->rep->set_double_value(st->rep, key, number_data);
}

static jerry_value_t req_get_byte(const char* key, iotjs_sthings_rep_t* st) {
  uint8_t* data;
  size_t size;
  st->rep->get_byte_value(st->rep, key, &data, &size);

  return iotjs_jval_create_byte_array(size, (const char*)data);
}

static bool req_set_byte(const char* key, jerry_value_t jdata,
                         iotjs_sthings_rep_t* st) {
  jerry_value_t arr_data = iotjs_jval_as_array(jdata);
  uint32_t length = jerry_get_array_length(arr_data);
  uint8_t* data =
      (uint8_t*)iotjs_buffer_allocate_from_number_array(length, arr_data);

  bool ret = st->rep->set_byte_value(st->rep, key, data, (size_t)length);
  IOTJS_RELEASE(data);
  return ret;
}

static jerry_value_t req_get_object(const char* key, iotjs_sthings_rep_t* st) {
  st_things_representation_s* data;
  st->rep->get_object_value(st->rep, key, &data);

  return iotjs_sthings_jrep_create(data, st->jresource_type);
}

static bool req_set_object_value(jerry_value_t jdata,
                                 st_things_representation_s* rep,
                                 iotjs_sthings_rep_t* st) {
  jerry_value_t jdata_key = jerry_get_object_keys(jdata);
  uint32_t length = jerry_get_array_length(jdata);

  for (uint32_t i = 0; i < length; i++) {
    jerry_value_t jtemp_index = iotjs_jval_get_property_by_index(jdata_key, i);
    iotjs_string_t jtemp_key_string = iotjs_jval_as_string(jtemp_index);
    const char* jtemp_key_stirng_raw = iotjs_string_data(&jtemp_key_string);
    jerry_value_t jtemp_value =
        iotjs_jval_get_property(jdata, jtemp_key_stirng_raw);

    jerry_value_t jtype =
        iotjs_jval_get_property(st->jresource_type, jtemp_key_stirng_raw);
    IOTJS_ASSERT(!jerry_value_is_undefined(jtype));

    int type = iotjs_jval_as_number(jtype);
    iotjs_sthings_rep_t st_tmp;
    st_tmp.rep = rep;
    st_tmp.jresource_type = st->jresource_type;

    req_set_data(type, jtemp_key_stirng_raw, jtemp_value, &st_tmp);

    jerry_release_value(jtemp_index);
    iotjs_string_destroy(&jtemp_key_string);
    jerry_release_value(jtemp_value);
  }

  jerry_release_value(jdata_key);
  return true;
}

static bool req_set_object(const char* key, jerry_value_t jdata,
                           iotjs_sthings_rep_t* st) {
  IOTJS_ASSERT(jerry_value_is_array(jdata));

  st_things_representation_s* temp_rep = st_things_create_representation_inst();

  if (!req_set_object_value(jdata, temp_rep, st)) {
    st_things_destroy_representation_inst(temp_rep);
    return false;
  }

  bool ret = st->rep->set_object_value(st->rep, key, temp_rep);
  st_things_destroy_representation_inst(temp_rep);

  return ret;
}

static jerry_value_t req_get_string_array(const char* key,
                                          iotjs_sthings_rep_t* st) {
  size_t length;
  char** data;
  st->rep->get_str_array_value(st->rep, key, &data, &length);

  jerry_value_t jarray = jerry_create_array(length);
  for (size_t i = 0; i < length; i++) {
    jerry_value_t val = jerry_create_string((const jerry_char_t*)data[i]);
    jerry_set_property_by_index(jarray, i, val);
    jerry_release_value(val);
  }

  return jarray;
}

static bool req_set_string_array(const char* key, jerry_value_t jdata,
                                 iotjs_sthings_rep_t* st) {
  IOTJS_ASSERT(jerry_value_is_array(jdata));

  uint32_t length = jerry_get_array_length(jdata);
  const char* data[length];
  iotjs_string_t str[length];

  for (uint32_t i = 0; i < length; i++) {
    jerry_value_t val = iotjs_jval_get_property_by_index(jdata, i);
    str[i] = iotjs_jval_as_string(val);
    data[i] = iotjs_string_data(&str[i]);
    jerry_release_value(val);
  }

  bool ret = st->rep->set_str_array_value(st->rep, key, data, length);

  for (uint32_t i = 0; i < length; i++) {
    iotjs_string_destroy(&str[i]);
  }

  return ret;
}

static jerry_value_t req_get_int_array(const char* key,
                                       iotjs_sthings_rep_t* st) {
  size_t length;
  int64_t* data;
  st->rep->get_int_array_value(st->rep, key, &data, &length);

  jerry_value_t jarray = jerry_create_array(length);
  for (size_t i = 0; i < length; i++) {
    jerry_value_t val = jerry_create_number((double)data[i]);
    jerry_set_property_by_index(jarray, i, val);
    jerry_release_value(val);
  }

  return jarray;
}

static bool req_set_int_array(const char* key, jerry_value_t jdata,
                              iotjs_sthings_rep_t* st) {
  IOTJS_ASSERT(jerry_value_is_array(jdata));

  uint32_t length = jerry_get_array_length(jdata);
  int64_t data[length];

  for (uint32_t i = 0; i < length; i++) {
    jerry_value_t val = iotjs_jval_get_property_by_index(jdata, i);
    data[i] = iotjs_jval_as_number(val);
    jerry_release_value(val);
  }

  return st->rep->set_int_array_value(st->rep, key, data, length);
}

static jerry_value_t req_get_double_array(const char* key,
                                          iotjs_sthings_rep_t* st) {
  size_t length;
  double* data;
  st->rep->get_double_array_value(st->rep, key, &data, &length);

  jerry_value_t jarray = jerry_create_array(length);
  for (size_t i = 0; i < length; i++) {
    jerry_value_t val = jerry_create_number(data[i]);
    jerry_set_property_by_index(jarray, i, val);
    jerry_release_value(val);
  }

  return jarray;
}

static bool req_set_double_array(const char* key, jerry_value_t jdata,
                                 iotjs_sthings_rep_t* st) {
  IOTJS_ASSERT(jerry_value_is_array(jdata));

  uint32_t length = jerry_get_array_length(jdata);
  double data[length];

  for (uint32_t i = 0; i < length; i++) {
    jerry_value_t val = iotjs_jval_get_property_by_index(jdata, i);
    data[i] = iotjs_jval_as_number(val);
    jerry_release_value(val);
  }

  return st->rep->set_double_array_value(st->rep, key, data, length);
}

static jerry_value_t req_get_object_array(const char* key,
                                          iotjs_sthings_rep_t* st) {
  size_t length;
  st_things_representation_s** data;
  st->rep->get_object_array_value(st->rep, key, &data, &length);

  jerry_value_t jarray = jerry_create_array(length);
  for (size_t i = 0; i < length; i++) {
    jerry_value_t val = iotjs_sthings_jrep_create(data[i], st->jresource_type);
    jerry_set_property_by_index(jarray, i, val);
    jerry_release_value(val);
  }

  return jarray;
}

static bool req_set_object_array(const char* key, jerry_value_t jdata,
                                 iotjs_sthings_rep_t* st) {
  IOTJS_ASSERT(jerry_value_is_array(jdata));

  uint32_t length = jerry_get_array_length(jdata);
  st_things_representation_s* data[length];

  for (uint32_t i = 0; i < length; i++) {
    jerry_value_t val = iotjs_jval_get_property_by_index(jdata, i);
    IOTJS_ASSERT(jerry_value_is_array(val));
    data[i] = st_things_create_representation_inst();

    if (!req_set_object_value(val, data[i], st)) {
      jerry_release_value(val);
      return false;
    }

    jerry_release_value(val);
  }

  bool ret =
      st->rep->set_object_array_value(st->rep, key,
                                      (const struct _st_things_representation**)
                                          data,
                                      length);

  for (uint32_t i = 0; i < length; i++) {
    st_things_destroy_representation_inst(data[i]);
  }

  return ret;
}

static bool req_set_data(int type, const char* key, jerry_value_t jdata,
                         iotjs_sthings_rep_t* st) {
  DDDLOG("[ST] %s, set %s(%d)", __func__, key, type);
  switch (type) {
    case 0:
      return req_set_bool(key, jdata, st);
    case 1:
      return req_set_int(key, jdata, st);
    case 2:
      return req_set_double(key, jdata, st);
    case 3:
      return req_set_string(key, jdata, st);
    case 4:
      return req_set_object(key, jdata, st);
    case 5:
      return req_set_byte(key, jdata, st);
    case 6:
      return req_set_int_array(key, jdata, st);
    case 7:
      return req_set_double_array(key, jdata, st);
    case 8:
      return req_set_string_array(key, jdata, st);
    case 9:
      return req_set_object_array(key, jdata, st);
    default:
      DDLOG("[ST] %s, Cannot support resource type", __func__);
      return false;
  }
}

static jerry_value_t req_get_data(int type, const char* key,
                                  iotjs_sthings_rep_t* st) {
  DDDLOG("[ST] %s, get %s(%d)", __func__, key, type);
  switch (type) {
    case 0:
      return req_get_bool(key, st);
    case 1:
      return req_get_int(key, st);
    case 2:
      return req_get_double(key, st);
    case 3:
      return req_get_string(key, st);
    case 4:
      return req_get_object(key, st);
    case 5:
      return req_get_byte(key, st);
    case 6:
      return req_get_int_array(key, st);
    case 7:
      return req_get_double_array(key, st);
    case 8:
      return req_get_string_array(key, st);
    case 9:
      return req_get_object_array(key, st);
    default:
      DDLOG("[ST] %s, Cannot support resource type", __func__);
      return jerry_create_undefined();
  }
}

JS_FUNCTION(Set) {
  DJS_CHECK_THIS();
  JS_DECLARE_THIS_PTR(sthings_rep, st);

  iotjs_string_t jkey_string = JS_GET_ARG(0, string);
  const char* key = iotjs_string_data(&jkey_string);
  jerry_value_t jtype = iotjs_jval_get_property(st->jresource_type, key);
  IOTJS_ASSERT(!jerry_value_is_undefined(jtype));
  int type = iotjs_jval_as_number(jtype);

  if (!req_set_data(type, key, jargv[1], st)) {
    DDLOG("[ST] %s, cannot set %s(%d)", __func__, key, type);
  }

  iotjs_string_destroy(&jkey_string);
  jerry_release_value(jtype);

  return jerry_create_undefined();
}

JS_FUNCTION(Get) {
  DJS_CHECK_THIS();
  JS_DECLARE_THIS_PTR(sthings_rep, st);

  iotjs_string_t jkey_string = JS_GET_ARG(0, string);
  const char* key = iotjs_string_data(&jkey_string);
  jerry_value_t jtype = iotjs_jval_get_property(st->jresource_type, key);
  IOTJS_ASSERT(!jerry_value_is_undefined(jtype));

  int type = iotjs_jval_as_number(jtype);
  jerry_value_t data = req_get_data(type, key, st);

  iotjs_string_destroy(&jkey_string);
  jerry_release_value(jtype);

  return data;
}

static jerry_value_t create_js_prototype(jerry_value_t obj) {
  jerry_value_t prototype = jerry_create_object();
  iotjs_jval_set_property_jval(obj, IOTJS_MAGIC_STRING_PROTOTYPE, prototype);
  iotjs_jval_set_method(prototype, IOTJS_ST_MAGIC_SET, Set);
  iotjs_jval_set_method(prototype, IOTJS_ST_MAGIC_GET, Get);

  return prototype;
}
