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

var st = require('smartthings');
var http = require('http');

var SWITCH = '/capability/switch/main/0';
var AUDIOVOLUME = '/capability/audioVolume/main/0';
var MEDIAINPUTSOURCE = '/capability/mediaInputSource/main/0';
var MEDIAPLAYBACK = '/capability/mediaPlayback/main/0';
var MEDIATRACKCONTROL = '/capability/mediaTrackControl/main/0';
var MEDIAPLAYBACKREPEAT = '/capability/mediaPlaybackRepeat/main/0';
var MEDIAPLAYBACKSHUFFLE = '/capability/mediaPlaybackShuffle/main/0';
var AUDIOTRACKDATA = '/capability/audioTrackData/main/0';

/*
  * Server
**/
var logCount = 0;
var serverLog = {
  logs: [],
};
function pushLog(data) {
  serverLog.logs.push({
    S: logCount++,
    C: data,
  });
}

function getLogString() {
  return JSON.stringify(serverLog);
}

function clearLog() {
  serverLog.logs = [];
}

http.createServer(function(req, res) {
  req.on('end', function() {
    var msg;

    msg = getLogString();
    clearLog();

    res.writeHead(200, {
      'Connection': 'close',
      'Content-Length': msg.length,
      'Content-Type': 'text/plain',
      'Access-Control-Allow-Origin': '*',
      'Access-Control-Allow-Methods': 'GET',
      'Access-Control-Allow-Headers:':
        'Origin, X-Requested-With, Content-Type, Accept',
    });

    res.end(msg);
  });
}).listen(5696).on('error', function(e) {
  console.log(e);
});


/*
  * Smart Things
**/
var gSwitch = false;
var gVolume = 50;

st.start({deviceDefinition: "device_def.json"});

st.on('getRequest', getRequest);
st.on('setRequest', setRequest);

function getRequest(msg, resp) {
  var uri = msg.resourceUri;
  console.log('getRequest', uri);

  if (uri === SWITCH) {
    if (msg.hasPropertyKey('power')) {
      resp.set('power', gSwitch ? 'on' : 'off');
    }
  } else if (uri === AUDIOVOLUME) {
    if (msg.hasPropertyKey('volume')) {
      resp.set('volume', gVolume);
    }
  }
}


function setRequest(msg, resp) {
  var uri = msg.resourceUri;
  console.log('setRequest', uri);

  if (uri === SWITCH) {
    var power = msg.rep.get('power');
    if (power) {
      if (power === 'off') {
        gSwitch = false;
      } else {
        gSwitch = true;
      }
    }
    resp.set('power', gSwitch ? 'on' : 'off');

    pushLog(gSwitch ? 'play': 'stop');

  } else if (uri === AUDIOVOLUME) {
    gVolume = msg.rep.get('volume');
    resp.set('volume', gVolume);

    pushLog('volume=' + gVolume);
  } else if (uri === MEDIATRACKCONTROL) {
    var modes = msg.rep.get('modes');
    pushLog(modes[0] === 'next' ? 'next': 'prev');
  }

  st.notifyObservers(uri);
}
