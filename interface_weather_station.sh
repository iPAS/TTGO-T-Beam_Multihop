#!/bin/bash

# TODO: array of commands

echo 'Commands'
pyserial-miniterm /dev/ttyUSB0 38400 --parity E --eol CRLF

