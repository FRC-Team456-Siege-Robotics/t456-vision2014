
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/legacy/legacy.hpp"
#include "stdio.h"


int main( int argc, char **argv)
{
   int  waitkey_delay = 2;
   CvCapture*    camera = 0;
   IplImage      *image = 0;
   CvMat *image_hsv = 0;
   CvMat *image_hue = 0;
   CvMat *image_sat = 0;
   CvMat *image_val = 0;


   int camera_img_width, camera_img_height;

   /*
   **  Setup camera capture
   **   0 = /dev/video0
   **   1 = /dev/video1
   */
   camera=cvCaptureFromCAM( 1 );

    /*
    **   Check and see if camera/file capture is valid
    */
    if (!camera) {
        printf("camera or image is null\n");
        return;
    }
    camera_img_width = cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH);
    camera_img_height = cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT);

    image_hsv = cvCreateMat(camera_img_height, camera_img_width, CV_8UC3);

    image_hue = cvCreateMat(camera_img_height, camera_img_width, CV_8UC1);
    image_sat = cvCreateMat(camera_img_height, camera_img_width, CV_8UC1);
    image_val = cvCreateMat(camera_img_height, camera_img_width, CV_8UC1);



    waitkey_delay = 2.0;

    while (cvWaitKey(waitkey_delay) < 0)
    {
       /*
       **  Grab initial frame from image or movie file
       */
       image = cvQueryFrame(camera);
       if ( !image ) {
          exit(-1);
       }

       cvCvtColor( image, image_hsv, CV_BGR2HSV );

       cvSplit( image_hsv, image_hue, image_sat, image_val, NULL);

       cvShowImage("Original Image",image);
       cvShowImage("Hue",image_hue);
       cvShowImage("Saturation",image_sat);
       cvShowImage("Value",image_val);

    }

   /*
   **  Release control of camera
   */
   cvReleaseCapture(&camera);

}
