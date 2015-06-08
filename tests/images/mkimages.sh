#!/bin/bash

rm -f winchester0.e4i
../../build/src/emitool --preset win20 --spt 16 --image winchester0.e4i
./fillimage.py winchester0.e4i

