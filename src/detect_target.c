/*
**  FRC Team 456 Siege Robotics
**  Vicksburg, MS
**  2014 Competition Code
**
*/
#include "opencv2/legacy/legacy.hpp"

#include "t456-vision.h"

/*
**  T456_detect_target
**
**   From a threshold black/white image, detect
**    target and orientation
**
*/
void T456_detect_target( CvMat * image_thresh )
{
   CvSeq *contours = 0;
   CvMemStorage *storage = NULL;
   CvMemStorage *hull_storage = NULL;

   /*
   ** Initialize contour storage variables
   */
   storage = cvCreateMemStorage(0);
   hull_storage = cvCreateMemStorage(0);

   /*
   **  Find the contours in the input image and store in the 
   **   contours list structure.
   */
   cvFindContours( image_thresh, storage, &contours , sizeof(CvContour),
                      CV_RETR_LIST, CV_CHAIN_APPROX_TC89_KCOS, cvPoint(0,0) );

   /*
   ** Loop through contours and extract the interesting ones
   */

}
