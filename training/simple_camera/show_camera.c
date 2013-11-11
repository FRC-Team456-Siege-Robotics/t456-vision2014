
#include "opencv2/highgui/highgui.hpp"
#include "stdio.h"


int main( int argc, char **argv)
{
   int  waitkey_delay = 2;
   CvCapture*    camera = 0;
   IplImage      *image = 0;

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
    //  Get camera information (image height and width)
    camera_img_width = cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH);
    camera_img_height = cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT);

    printf("camera image width: %d\n", camera_img_width);
    printf("camera image height: %d\n", camera_img_height);

    waitkey_delay = 1.0;

    while (cvWaitKey(waitkey_delay) < 0)
    {
       /*
       **  Grab initial frame from image or movie file
       */
       image = cvQueryFrame(camera);
       if ( !image ) {
          exit(-1);
       }

        cvShowImage("contours",image);

    }
}
