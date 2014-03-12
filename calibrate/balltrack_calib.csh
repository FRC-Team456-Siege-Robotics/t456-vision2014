#!/bin/csh
#
clear
echo "*************************************************************** ";
echo "*                 FRC Team 456 Siege Robotics                 * ";
echo "*                  Ball Tracking Vision Calibration           * ";
echo "*************************************************************** ";
echo " ";
echo -n " Enter video device name (/dev/video0 or /dev/video1): ";
set vdev = $<
echo "       calibration will be using: " $vdev
echo " ";
echo "*************************************************************** ";
echo " STEP 1:         CAMERA POSITION ADJUSTMENT                      ";
echo "                                                                ";
echo "      Purpose: Position camera and verify field of view         ";
echo "                                                                ";
echo "*************************************************************** ";
echo "        Position and Aim camera at down the field ";
echo -n " Press <ENTER> to continue: ";
set ans = $<
#cheese -d $vdev
echo " ";
echo "*************************************************************** ";
echo " STEP 2:   CAMERA EXPOSURE AND SETTINGS ADJUSTMENT              ";
echo "                                                                ";
echo "      Purpose: Adjust white balance and turn off auto exposure  ";
echo "                                                                ";
echo "*************************************************************** ";
echo " ";
echo -n " Press <ENTER> to continue: ";
set ans = $<
#
# Set White Balance Temperature, Auto to fixed (not auto)
#
# control 0x0098090c `White Balance Temperature, Auto' min 0 max 1 step 1 default 1
#
./yavta --set-control '0x0098090c 1' $vdev
#
# Set White Balance Temperature to 4800
# control 0x0098091a `White Balance Temperature' min 2800 max 6500 step 1 
#                        default 3000 
./yavta --set-control '0x0098091a 2800' $vdev
#
#  Set Exposure to Manual Mode
# control 0x009a0901 `Exposure, Auto' min 0 max 3 step 1 default 3 current 3.
#  0: Auto Mode
#  1: Manual Mode
#  2: Shutter Priority Mode
#  3: Aperture Priority Mode
#
./yavta --set-control '0x009a0901 1' $vdev
#  
#  Set Absolute Exposure to 8
# control 0x009a0902 `Exposure (Absolute)' min 2 max 5000 step 1 default 312 
#
./yavta --set-control '0x009a0902 8' $vdev

echo " Setting and capturing 8 ms exposure for 15 secs"
echo -n " Press <ENTER> to continue: ";
set ans = $<
./yavta --set-control '0x009a0902 8' $vdev
avconv -f video4linux2 -i $vdev -t 00:00:15 -an -qscale 1 ball_8ms_exposure.mjpg

echo " Setting and capturing 10 ms exposure for 15 secs"
echo -n " Press <ENTER> to continue: ";
set ans = $<
./yavta --set-control '0x009a0902 10' $vdev
avconv -f video4linux2 -i $vdev -t 00:00:15 -an -qscale 1 ball_10ms_exposure.mjpg

echo " Setting and capturing 20 ms exposure for 15 secs"
echo -n " Press <ENTER> to continue: ";
set ans = $<
./yavta --set-control '0x009a0902 20' $vdev
avconv -f video4linux2 -i $vdev -t 00:00:15 -an -qscale 1 ball_20ms_exposure.mjpg

echo " Setting and capturing 30 ms exposure for 15 secs"
echo -n " Press <ENTER> to continue: ";
set ans = $<
./yavta --set-control '0x009a0902 30' $vdev
avconv -f video4linux2 -i $vdev -t 00:00:15 -an -qscale 1 ball_30ms_exposure.mjpg

echo " Setting and capturing 40 ms exposure for 15 secs"
echo -n " Press <ENTER> to continue: ";
set ans = $<
./yavta --set-control '0x009a0902 40' $vdev
avconv -f video4linux2 -i $vdev -t 00:00:15 -an -qscale 1 ball_40ms_exposure.mjpg

echo " Setting and capturing 60 ms exposure for 15 secs"
echo -n " Press <ENTER> to continue: ";
set ans = $<
./yavta --set-control '0x009a0902 60' $vdev
avconv -f video4linux2 -i $vdev -t 00:00:15 -an -qscale 1 ball_60ms_exposure.mjpg

echo " Setting and capturing 80 ms exposure for 15 secs"
echo -n " Press <ENTER> to continue: ";
set ans = $<
./yavta --set-control '0x009a0902 80' $vdev
avconv -f video4linux2 -i $vdev -t 00:00:15 -an -qscale 1 ball_80ms_exposure.mjpg
