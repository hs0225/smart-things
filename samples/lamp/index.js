var st = require('smartthings');

var SWITCH_URL = '/capability/switch/main/0';
var DIMMING_URL = '/capability/switchLevel/main/0';
var TEMPERATURE_URL = '/capability/colorTemperature/main/0';

var powerStatus = ["on", "off"];
var switchValue = false;

var dimmingSetting = 50;
var dimmingStep = 5;
var dimmingRange = [0, 100];

var colorTemp = 50;
var colorRange = [0, 100];

st.start({deviceDefinition: "device_def.json"});
st.on('getRequest', getRequest);
st.on('setRequest', setRequest);

function getRequest(msg, req) {
  
  var uri = msg.resourceUri;
  console.log('get: ', uri);
  if (uri === SWITCH_URL) {
    if (msg.hasPropertyKey('power')) {
      console.log('power get', switchValue);
      req.set('power', switchValue ? powerStatus[0] : powerStatus[1]);
    }
  } else if (uri === DIMMING_URL) {
    if (msg.hasPropertyKey('dimmingSetting')) {
      req.set('dimmingSetting', dimmingSetting);
    } else if (msg.hasPropertyKey('range')) {
      req.set('range', dimmingRange);
    } else if (msg.hasPropertyKey('step')) {
      req.set('step', dimmingStep);
    }
  } else if (uri === TEMPERATURE_URL) {
    if (msg.hasPropertyKey('ct')) {
      req.set('ct', colorTemp);
    } else if(msg.hasPropertyKey('range')) {
      req.set('range', colorRange);
    }
  }
}

function setRequest(msg, req) {
  var uri = msg.resourceUri;
  if (uri === SWITCH_URL) {
    var power = msg.rep.get('power');
    if (power) {
      if (power === 'off') {
        switchValue = false;
      } else {
        switchValue = true;
      }
      req.set('power', switchValue ? powerStatus[0] : powerStatus[1]);
    }
  } else if (uri === DIMMING_URL) {
    dimmingSetting = msg.rep.get('dimmingSetting');
    req.set('dimmingSetting', dimmingSetting);
  } else if (uri === TEMPERATURE_URL) {
    colorTemp = msg.rep.get('ct');
    req.set('ct', colorTemp);
  }
  
  st.notifyObservers(uri);
}


setInterval(function() {
  console.log('loop');
}, 10000);
