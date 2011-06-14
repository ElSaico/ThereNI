#!/bin/sh
LD_PRELOAD=/usr/local/lib/fakenect/libfreenect.so FAKENECT_PATH=$1 ./thereni.py
