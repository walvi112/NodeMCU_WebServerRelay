# NodeMCU WebServer Relay
Web server on NodeMCU ESP8266 dev board for controlling IO pins.

## Features
* Relay control (IO pin control) via webserver interface with the wifi module working in mode client.
* Support setting of up to schedules for on/off IO pin by day/hour/minute via webserver interface.
* Schedule can run without internet connection (RTC drift time to be tested).
* Automatically synchronize with NTP server for accurate RTC.
* Previous states and schedules are stored in eprom.

## Setting up
* Allow to control an IO pin via webserver (I connected the D1 pin to control relay).
* Button with pull-up resistor on D2 pin.
* After starting up, the board must connect to internet at least once to synchronize RTC and enable schedule setting.

## Demo
To be updated.
