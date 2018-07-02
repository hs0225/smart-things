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

var EventEmitter = require('events').EventEmitter;
var util = require('util');
var fs = require('fs');
var SmartThings;

if (process.platform === 'tizen') {
  SmartThings = require('smartthings_native');
} else if (process.platform === 'tizenrt') {
  SmartThings = native;
} else {
}

function getResources(json) {
  var resources = {};

  json.device.forEach(function(item) {
    // Todo: multi type (Support only single type.)
    if (item.resources && item.resources.single) {
      var singleLength = item.resources.single.length;
      for (var i = 0; i < singleLength; i++) {
        var resource = item.resources.single[i];
        resources[resource.uri] = resource.types[0];
      }
    }
  });
  console.log(resources);
  return resources;
}

function getResourceType(json) {
  var resourceTypes = {};
  json.resourceTypes.forEach(function(resItem, resIndex, resArray) {
    resourceTypes[resItem.type] = {};
    resItem.properties.forEach(function(propItem, propIndex, propArray) {
      resourceTypes[resItem.type][propItem.key] = Number(propItem.type);
    });
  });
  console.log(resourceTypes);
  return resourceTypes;
}

util.mixin(SmartThings.prototype, EventEmitter.prototype);

SmartThings.prototype.start = function(config) {
  if (!util.isObject(config)) {
    throw TypeError('invalid config: config must be a Object.');
  }
  
  if (!util.isString(config.deviceDefinition)) {
    throw TypeError('invalid config: config.deviceDef must be a string and json file.');
  }

  try {
    var jsonPath;
    if (process.platform === 'tizen') {
      // jsonPath = this._getResPath() + config.deviceDefinition;
      jsonPath = '/root/st/' + config.deviceDefinition;
    } else if (process.platform === 'tizenrt') {
      jsonPath = '/rom/' + config.deviceDefinition;
    }
    var jsonData = JSON.parse(fs.readFileSync(jsonPath, 'utf8'));
    this._resources = getResources(jsonData);
    this._resourceType = getResourceType(jsonData);
  } catch (e) {
    throw Error('Failed to analyze device definition.');
  }
  
  // check config
  this._userConfirmRequest = config.userConfirmRequest || function() {
    return true;
  };
  
  this._resetRequest = config.resetRequest || function() {
    return false;
  };
  
  this._start(config);
}

SmartThings.prototype.loopStart = function(time) {
  time = time | 50000;

  this._loop = setInterval(function() {
  }, time);
}

SmartThings.prototype.loopStop = function() {
  if (!util.isNullOrUndefined(this._loop)) {
    clearInterval(this._loop);
    this._loop = null;
  }
}

var st = new SmartThings();

module.exports = st;
