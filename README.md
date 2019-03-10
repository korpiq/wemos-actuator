# wemos-actuator

Software for a remote controllable actuator.

[![Sponsored](https://img.shields.io/badge/chilicorn-sponsored-brightgreen.svg)](http://spiceprogram.org/oss-sponsorship/)

## Development Setup

Setup to compile software to run on a Wemos D1 Mini board to control any electric device with a relay.

### On MacOS

Script `setup/macos.sh` contains commands to install everything necessary to build and install this software from a MacOS machine.

## Device Setup

1. Install software on the device as usual with Arduino/Wemos.
2. Device led should blink 10 times in 5 seconds to notify that configuration is starting.
3. Wait 6 minutes while the device creates a filesystem on first install.
4. Join WLAN named `DEVICE_SETUP`.
5. If a dialog doesn't open up, open browser and go to any address, eg. [http://set.up/](http://set.up/)
6. Fill in requested information
    - name and password of the WLAN to use for access to Internet
    - address of MQTT server for uploading the metrics.
7. Device led blinks 10 times in 1 second to notify that the device has been configured.
8. Device led blinks once per second when it works normally.

## Usage

### Via USB

You can listen in on the serial port for debug messages.

- Send a one-letter message to activate the device for duration of `default_run_milliseconds`.
- Send a JSON object to update configuration. Each of the following fields is optional:
  - "wifi_name": name (SSID) of wifi network to connect to
  - "wifi_password": password for the above
  - "mqtt_server_url": IotHub connection string
  - "run_duration_ms": how many milliseconds to run the motor when started.

### Via Wifi

Via browser you can
- run the actuator for a specified number of seconds
- configure the same settings as above.

The only difficulty is to get the IP address of the device to connect to.
