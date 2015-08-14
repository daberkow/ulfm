#!/bin/bash

pkill -f pebble
pebble build
if [ $? -eq 0 ];then
  pebble install --emulator aplite & pebble install --logs --emulator basalt
fi
