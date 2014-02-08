#!/bin/csh

#
# Set White Balance Temperature, Auto to fixed (not auto)
#
# control 0x0098090c `White Balance Temperature, Auto' min 0 max 1 step 1 default 1
#
./yavta --set-control '0x0098090c 1' /dev/video0

#
# Set White Balance Temperature to 4800
# control 0x0098091a `White Balance Temperature' min 2800 max 6500 step 1 
#                        default 3000 
./yavta --set-control '0x0098091a 2800' /dev/video0

#
#  Set Exposure to Manual Mode
# control 0x009a0901 `Exposure, Auto' min 0 max 3 step 1 default 3 current 3.
#  0: Auto Mode
#  1: Manual Mode
#  2: Shutter Priority Mode
#  3: Aperture Priority Mode
#
./yavta --set-control '0x009a0901 1' /dev/video0

#  
#  Set Absolute Exposure to 2
# control 0x009a0902 `Exposure (Absolute)' min 2 max 5000 step 1 default 312 
#
./yavta --set-control '0x009a0902 60' /dev/video0

