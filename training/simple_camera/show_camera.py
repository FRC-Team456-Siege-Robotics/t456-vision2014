#!/usr/bin/env python
'''
=======================
=======================
'''
import numpy as np
import cv2
import sys

print "OpenCV version:", cv2.__version__

# Capture numbers
#  0 = /dev/video0
#  1 = /dev/video1
#  etc...

if len(sys.argv) == 2:
    vidnum = sys.argv[1]
else:
    vidnum = 0

print "Using /dev/video%d" % int(vidnum)
print " press 'q' to quit"

cap = cv2.VideoCapture(int(vidnum))

while(True):
    # capture frame by frame
    ret, frame = cap.read()

    cv2.imshow('frame',frame)
    if cv2.waitKey(1) & 0xff == ord('q'):
        break


cap.release()
cv2.destroyAllWindows()
