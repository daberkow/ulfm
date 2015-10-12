#!/bin/bash

pkill -f pebble
pebble build
if [ $? -eq 0 ];then
  case $1 in
  -color)
    pebble install --logs --emulator basalt
    ;;
  -bw)
    pebble install --logs --emulator aplite 
    ;;
  *)
    pebble install --emulator aplite & pebble install --logs --emulator basalt
    ;;
  esac  
fi
