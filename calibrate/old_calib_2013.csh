#!/bin/csh
#
echo "******************************************** ";
echo "*  Vision calibration                      * ";
echo "******************************************** ";
echo " ";
echo "   Aim camera at target ";
echo -n " Press <ENTER> to continue: ";
set ans = $<
cheese
echo "******************************************** ";
echo "*  Adjusting Camera settings               * ";
echo "******************************************** ";
echo " ";
echo -n " Press <ENTER> to continue: ";
set ans = $<
echo " ";
/usr/local/bin/webcam_settings.csh
echo " ";
echo -n " Press <ENTER> to continue: ";
set ans = $<
echo "******************************************** ";
echo "*  Capture different camera exposures      * "
echo "******************************************** ";
echo "   START with NO light covers                ";
echo -n " Press <ENTER> to continue: ";
set ans = $<
echo " ";

echo " Setting and capturing 2 ms exposure for 15 secs"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 2' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/full_light_2ms_exposure.mjpg
echo " Done."
echo " "

echo " Setting and capturing 4 ms exposure for 15 secs"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 4' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/full_light_4ms_exposure.mjpg
echo " Done."

echo " Setting and capturing 6 ms exposure for 15 secs"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 6' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/full_light_6ms_exposure.mjpg
echo " Done."

echo " Setting and capturing 8 ms exposure for 15 secs"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 8' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/full_light_8ms_exposure.mjpg
echo " Done."

echo " Setting and capturing 16 ms exposure for 15 secs"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 16' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/full_light_16ms_exposure.mjpg
echo " Done."

echo " Setting and capturing 40 ms exposure for 15 secs"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 40' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/full_light_40ms_exposure.mjpg
echo " Done."

echo " ****************************************************** ";
echo "   Attach largest hole cover (should cover 1 ring)      ";
echo -n " Press <ENTER> to continue: ";
set ans = $<
echo " Setting and capturing 2 ms exposure for 15 secs for largest hole"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 2' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/cover1_light_2ms_exposure.mjpg
echo " Done."
echo " "

echo " Setting and capturing 4 ms exposure for 15 secs for largest hole"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 4' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/cover1_light_4ms_exposure.mjpg
echo " Done."

echo " Setting and capturing 6 ms exposure for 15 secs for largest hole"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 6' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/cover1_light_6ms_exposure.mjpg
echo " Done."

echo " Setting and capturing 8 ms exposure for 15 secs for largest hole"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 8' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/cover1_light_8ms_exposure.mjpg
echo " Done."

echo " Setting and capturing 16 ms exposure for 15 secs for largest hole"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 16' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/cover1_light_16ms_exposure.mjpg
echo " Done."

echo " Setting and capturing 40 ms exposure for 15 secs for largest hole"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 40' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/cover1_light_40ms_exposure.mjpg
echo " Done."

echo " ****************************************************** ";
echo "   Attach second largest hole cover (should cover 2 rings) "; 
echo -n " Press <ENTER> to continue: ";
set ans = $<

echo " Setting and capturing 2 ms exposure for 15 secs for cover2 hole (next largest)"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 2' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/cover2_light_2ms_exposure.mjpg
echo " Done."

echo " Setting and capturing 6 ms exposure for 15 secs for cover2 hole (next largest)"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 6' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/cover2_light_6ms_exposure.mjpg
echo " Done."

echo " Setting and capturing 8 ms exposure for 15 secs for cover2 hole (next largest)"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 8' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/cover2_light_8ms_exposure.mjpg
echo " Done."

echo " Setting and capturing 16 ms exposure for 15 secs for cover2 hole (next largest)"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 16' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/cover2_light_16ms_exposure.mjpg
echo " Done."

echo " Setting and capturing 40 ms exposure for 15 secs for cover2 hole (next largest)"
echo -n " Press <ENTER> to continue: ";
set ans = $<
/usr/local/bin/yavta --set-control '0x009a0902 40' /dev/video0
ffmpeg -f video4linux2 -i /dev/video0 -t 00:00:15 -an -q 1 auto_calib/cover2_light_40ms_exposure.mjpg
echo " Done."


echo "******************************************** ";
echo "*  Resetting camera                        * ";
echo "******************************************** ";
echo " ";
echo -n " Press <ENTER> to continue: ";
set ans = $<
echo " ";
/usr/local/bin/webcam_settings.csh
