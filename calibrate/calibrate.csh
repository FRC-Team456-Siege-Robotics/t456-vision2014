#!/bin/csh
#
clear
echo "*************************************************************** ";
echo "*                 FRC Team 456 Siege Robotics                 * ";
echo "*                     Vision calibration                      * ";
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
cheese -d $vdev
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
echo " ";
echo /usr/local/bin/webcam_settings.csh
echo " ";
echo -n " Press <ENTER> to continue: ";
set ans = $<

