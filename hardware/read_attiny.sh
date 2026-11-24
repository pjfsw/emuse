#!/bin/sh

avrdude -v -c arduino -P /dev/ttyACM0 -p t2313 -b 19200

