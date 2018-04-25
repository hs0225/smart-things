# How to build

## Prerequisites

* build IoT.js for Tizen
```bash
cd <iot.js project home>
./config/tizen/gbsbuild.sh
```

## Building
```bash
cmake -Bbuild -H.
make -C build
```

# API
## Platform Support

The following table shows SmartThings module APIs available for each platform.

|  | Tizen<br/>(Raspberry Pi) |
| :---: | :---: |
| smartthings.start | O |
| smartthings.stop | O |
| smartthings.notifyObserver | O |
| smartthings.reset | O |
| representaion.set | O |
| representaion.get | O |


### SmartThings

This module help you connect to SmartThings Cloud.


#### smartthings.start(configuration)
* `configuration` {Object}
  * `deviceDefinition` {string} device definition json file path. Mandatory configuration.
  * `userConfirmRequest` {Function} user confirm callback.
  * `resetRequest` {Function} user reset request callback.

Start easy-setup and things stack.
Note: Until easy-setup is completed, you not be able to interaction with application and recevie any event.


#### smartthings.stop()

Stop things stack


#### smartthings.reset()

Reset things stack

#### smartthings.notifyObserver()

Notify the observers of a specific resource.


#### Event: 'getRequest'
* `callback` {Function}
  * `getmessage` {Object} GetMessage Object.
  * `representation` {Object} Representation Object.

Emit when representing the Get Request Message.


#### Event: 'setRequest'
* `callback` {Function}
  * `setmessage` {Object} GetMessage Object.
  * `representation` {Object} Representation Object.

Emit when representing the Set Request Message.

#### Event: 'resetResult'
* `callback` {Function}

Emit after the reset process is done.

#### Event: 'statusChange'
* `callback` {Function}
  * `status` {number}

Emit getting notified when SmartThings state change.

#### Event: 'pinGeneration'
* `callback` {Function}
  * `pin_data` {string}
  * `pin_size` {number}

Emit when device receives a Ownership Transfer request from client.

#### Event: 'pinDisplayClose'
* `callback` {Function}

Emit when Ownership Transfer is done so device can stop showing PIN on display.

### Representation

#### representation.set(key, value)
* `key` {string}
* `value` {any}

Set an value of specified key from representation.


#### representation.get(key)
* `key` {string}
* Returns: {any} value

Get an value of specified key from representation.


### SetMessage

#### setmessage.resourceUri
* `{string}`


#### setmessage.rep
* `{Object}` Representation Object


### GetMessage

#### getmessage.hasPropertyKey(key)
* `key` {string}
* Returns: {Boolean}

Check whether the request has a specific property key or not.
