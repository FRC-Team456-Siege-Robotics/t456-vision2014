
/*
**   Webcam configuration
**   Team 456, Vicksburg, MS
**   www.seigerobotics.org
** 
*/

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/legacy/legacy.hpp"
#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

#include "camera_info.h"
#include "target_info.h"


/*
** ================ BEGIN MAIN SECTION =======================
*/
void main( int argc, char** argv )
{

    double t1, t2, fps;
    int  targ_selected = 0;

    CvCapture* camera = 0;
    CvMat         *mat_image = 0;
    IplImage      *image = 0;

    /*  
    **  Capture images from webcam /dev/video0 
    **   /dev/video0 = 0
    **   /dev/video1 = 1
    */
    if ( argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
    {
       camera=cvCaptureFromCAM( argc == 2 ? argv[1][0] - '0' : 0 );
       cvSetCaptureProperty(camera, CV_CAP_PROP_EXPOSURE, 1.0);
    } 
    else 
    {
       /* 
       **  Capture image from file at command line
       **   useful for playback video and testing
       */
       camera = cvCaptureFromFile( argv[1] );
    }

printf("height %f\n", cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT));
printf("width %f\n", cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH));
printf("fps %f\n", cvGetCaptureProperty(camera, CV_CAP_PROP_FPS));

    /*
    **   Check and see if camera/file capture is valid
    */
    if (!camera) {
        printf("camera or image is null\n");
        return;
    }

    cvNamedWindow("original", CV_WINDOW_AUTOSIZE);


    /*
    **  Time estimation variable
    */
    t1 = (double)cvGetTickCount();

    /*
    **   Process images until key press
    */
    while (cvWaitKey(2) < 0)
    {

       /*
       **  Grab initial frame from image or movie file
       */
       image = cvQueryFrame(camera);
       if ( !image ) {
          return;
       }
 
        /*
        **  Display images
        */
        cvShowImage("original",image);

        /*  
        ** keep time of processing 
        */
        t2 = (double)cvGetTickCount();
        fps = 1000.0 / ((t2-t1)/(cvGetTickFrequency()*1000.));

        printf("time: %gms  fps: %.2g\n",(t2-t1)/(cvGetTickFrequency()*1000.),fps);

        t1 = t2;
    }

    /*
    **  Release camera resources
    */
    cvReleaseCapture(&camera);

    exit;
}


