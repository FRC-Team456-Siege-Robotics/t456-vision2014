#!/usr/bin/env python
'''
=======================
=======================
'''

# Import/use system module
import sys

# Import/use OpenCV module
import cv2

# Print OpenCV version
print "OpenCV version:", cv2.__version__

# Capture numbers for video camera
#  0 = /dev/video0
#  1 = /dev/video1
#  etc...

#
#  Parse the input command line
#
if len(sys.argv) == 2:
    vidnum = sys.argv[1]
else:
    vidnum = 0

print "Using /dev/video%d" % int(vidnum)
print " press 'q' to quit"

#
#  Setup video capture
#
cap = cv2.VideoCapture(int(vidnum))

#
#  Loop until 'q' is pressed
#
while(True):

    # capture frame from camera
    ret, frame = cap.read()

    # display on screen camera frame
    cv2.imshow('frame',frame)

    if cv2.waitKey(1) & 0xff == ord('q'):
        break


#  
#  Release video capture object
#
cap.release()

#
#  Close and destroy display window
#
cv2.destroyAllWindows()

#
