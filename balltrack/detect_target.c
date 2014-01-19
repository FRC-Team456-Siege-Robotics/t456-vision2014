/*
**  FRC Team 456 Siege Robotics
**  Vicksburg, MS
**  2014 Competition Code
**
*/
#include "opencv2/legacy/legacy.hpp"

#include "t456-vision.h"

extern IplImage      *image;
extern int framenum;

void draw_target_center( CvPoint , IplImage *, CvScalar );

/*
**  T456_detect_target
**
**   From a threshold black/white image, detect
**    target and orientation
**
*/
void T456_detect_ball_target( CvMat * image_thresh )
{
   int i;
   CvSeq *detected_circles = 0;
   CvSeq *seq = 0;
   CvMemStorage *circle_storage = NULL;

   /*
   ** Initialize circle storage variables
   */
   circle_storage = cvCreateMemStorage(0);

//   cvSmooth(image_thresh, image_thresh, CV_GAUSSIAN, 15, 0, 0, 0);

   /*
   **  Find the circles in the input image and store in the 
   **   circle list structure.
   */
   detected_circles = cvHoughCircles( image_thresh, circle_storage,
              CV_HOUGH_GRADIENT,
              2,
              35,     /* minimum distance between centers */
              600,    /* upper threshold for detector */
              8,     /* threshold for center detection */
              20,      /* min radius */
              500);     /* max radius */


   /*
   ** Loop through contours and extract the interesting ones
   */
       if ( detected_circles->total != 0 )
       {
          printf("num circles: %d\n",detected_circles->total);

          for ( i = 0; i < detected_circles->total; i++ )
          {
             float *p = (float *) cvGetSeqElem( detected_circles, i);
             cvCircle( image, cvPoint(cvRound(p[0])-20, cvRound(p[1])),cvRound(p[2])+1,
                     CV_RGB(0,255,0), -1, 8, 0);
             draw_target_center( cvPoint(cvRound(p[0])-20, cvRound(p[1])),
                                 image,
                                 CV_RGB(255,0,0) );
          }
       }

   cvReleaseMemStorage( &circle_storage );

}

/*
**   Draw target crosshair on image
*/
void draw_target_center( CvPoint target, IplImage *draw_image,
                         CvScalar cross_color )
{
   int      cross_thickness;
   CvPoint pt1, pt2;

   cross_thickness = 2;

      pt1.x = target.x+8;
      pt1.y = target.y;
      pt2.x = target.x-8;
      pt2.y = target.y;

      cvLine( draw_image, pt1, pt2, cross_color,
              cross_thickness, 8, 0);

      pt1.x = target.x;
      pt1.y = target.y+8;
      pt2.x = target.x;
      pt2.y = target.y-8;
      cvLine( draw_image, pt1, pt2, cross_color,
              cross_thickness, 8, 0);

}

