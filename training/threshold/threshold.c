
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
   CvMat *image_thresh = 0;


   int camera_img_width, camera_img_height;

   /*
   **  Setup camera capture
   **   0 = /dev/video0
   **   1 = /dev/video1
   */
   // camera=cvCaptureFromCAM( 1 );
   camera=cvCaptureFromFile( "./full_light_8ms_exposure.mjpg" );

    /*
    **   Check and see if camera/file capture is valid
    */
    if (!camera) {
        printf("camera or image is null\n");
        return;
    }

    //  Get camera information (image height and width)
    camera_img_width = cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH);
    camera_img_height = cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT);


    /*
    **  Allocate memory for camera image operations
    */
    image_hsv = cvCreateMat(camera_img_height, camera_img_width, CV_8UC3);

    image_hue = cvCreateMat(camera_img_height, camera_img_width, CV_8UC1);
    image_sat = cvCreateMat(camera_img_height, camera_img_width, CV_8UC1);
    image_val = cvCreateMat(camera_img_height, camera_img_width, CV_8UC1);
    image_thresh = cvCreateMat(camera_img_height, camera_img_width, CV_8UC1);

    /*
    **  Specify how long to wait for a key press (in msec)
    */
    waitkey_delay = 200.0;

    /*
    **  Loop, process camera image until any key is pressed
    */
    while (cvWaitKey(waitkey_delay) < 0)
    {
       /*
       **  Grab initial frame from image or movie file
       */
       image = cvQueryFrame(camera);
       if ( !image ) {
          exit(-1);
       }

       /*
       **  Convert camera image into HSV color space
       */
       cvCvtColor( image, image_hsv, CV_BGR2HSV );

       /*
       **  Split into three seperate image files
       */
       cvSplit( image_hsv, image_hue, image_sat, image_val, NULL);

       /*
       **  Threshold (select) pixels that are the same brightness (value)
       **    as the target 
       **
       **   select all pixels greater than threshold_value
       **
       **  cvThreshold( input, output, threshold_value, replacement_num, type)
       **  
       **   type = 0 replace all threshold pixels
       **   type = 1 replace all non threshold pixels
       */

       cvThreshold( image_val, image_thresh, 230, 255, 0);

       /*
       **  Display results
       */
       cvShowImage("Original Image",image);
       cvShowImage("Value",image_val);
       cvShowImage("Threshold",image_thresh);
    }

   /*
   **  Release control of camera
   */
   cvReleaseCapture(&camera);

}
