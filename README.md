# wundervoice

WunderVoice is a Pebble smartwatch app that allows you to quickly add items to a Wunderlist list using speech.

This repository contains the code for the Pebble app, which communicates with Wunderlist to add tasks to a specified list. Please note, this application is dependent on a [wundervoice-config](https://github.com/alirawashdeh/wundervoice-config) configuration server - see [here](https://github.com/alirawashdeh/wundervoice-config) for the source code.

## Configuration

Ensure that [wundervoice-config](https://github.com/alirawashdeh/wundervoice-config) is configured and deployed somewhere.

Modify the following part of the pebble-js-app.js file to include your Wunderlist Developer Client ID and the URL for the wundervoice-config application:

```
var client_id = "77e7a497276456d896a7";
var config_url = 'https://wunderdictate.herokuapp.com';
```

## Build and run

The easiest way to deploy your Pebble application is to sign up for a [CloudPebble](http://cloudpebble.com/) account. Make sure that the developer connection is running on your phone before deploying through CloudPebble.

```
cd wundervoice
pebble build
pebble install --cloudpebble
```

# Credits

Thanks to [WunderPebble](https://github.com/jahdaic/WunderPebble), [WunderPebbleJS](https://github.com/jahdaic/WunderPebbleJS) and [pebble/simple_voice_demo](https://github.com/pebble-examples/simple-voice-demo) for the vast majority of this code.
