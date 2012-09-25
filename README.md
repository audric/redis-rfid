# redis-rfid

## Overview
This is a set of software tools used in a Raspberry Pi RFID enabled Kiosk.

### The complete project involves
- running chromium in kiosk mode as a single .xinitrc program browsing localhost (see example.xinitrc)
- getting RFID id and store it locally (see redis-rfid-serial.c)
- use thr locally stored rfid, add some other values on a web form (see example-ajax.html)
- send data via web-service to a remote (ADempiere) server


## Files
### redis-rfid-serial
Endlessly reads RFID card tokens (serially from /dev/ttyUSB0) and pushes the value into redis with name rfidcard.
Tested on a Raspberry Pi with a Sparkfun RFID USB Reader SEN-09963

## To compile
- first compile hiredis https://github.com/antirez/hiredis
- then compile this program and link it with libhiredis.a
```sh
$cc -o redis-rfid-serial redis-rfid-serial.c libhiredis.a
```

## To test
```sh
$ ./redis-rfid-serial &
```
then swipe come rfid card or tag on the reader and check
```sh
$redis-cli get rfidcard
```

## Todo
- publish to git
- makefile
- make redis connection settings and serial reader settings as parameter
- implement a debug flag to show on stdio what's happening
- wrap up a init.d script to daemonized the thing
