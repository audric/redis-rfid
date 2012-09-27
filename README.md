# redis-rfid

## Overview
This is a set of software tools used in a Raspberry Pi RFID enabled Kiosk.

### The complete project involves
- running chromium in kiosk mode as a single .xinitrc program browsing localhost (see example.xinitrc)
- getting RFID id and store it locally (see redis-rfid-serial.c)
- use the locally stored rfid, add some other values on a web form (see example-ajax.html)
- send data via web-service to a remote (ADempiere) server (see sinatra app redis-rfid.rb)

## redis-rfid-serial
Endlessly reads RFID card tokens (serially from /dev/ttyUSB0) and pushes the value into redis with name rfidcard.
Tested on a [Raspberry Pi](http://www.raspberrypi.org/) with a 
[Sparkfun RFID USB Reader SEN-09963](https://www.sparkfun.com/products/9963)

### To compile
- first get and compile the hiredis c library https://github.com/antirez/hiredis
- then compile this program and link it with libhiredis.a

```sh
$ cc -o redis-rfid-serial redis-rfid-serial.c libhiredis.a
```
### To test
```sh
$ ./redis-rfid-serial &
```
then swipe some rfid card or tag on the reader and check
```sh
$ redis-cli get rfidcard
```

### Todo
- a makefile
- a redis connection settings and serial reader settings as parameter
- a debug flag to show on stdio what's happening
- wrap up an init.d script to daemonized the thing

## redis-rfid.rb
This is a very minimal and simple sinatra application that demonstrate how to wrap it all up.
It contains:
- the ajax call to refresh the input text with the last rfid card read

### Todo
- a lookup function to retrive operator's picture
- add ajax code to show picture on web page

## example.xinitrc
Simple way to start x windows in kiosk mode with chromium.

## example-ajax.html
Simple file to show rfidcard refresh with ajax and [webdis](https://github.com/nicolasff/webdis)

## example-unicorn.init
The unicorn init.d file i'm using.

## example-rc.local
The rc.local i'm using

## example-inittab
The inittab i'm using
