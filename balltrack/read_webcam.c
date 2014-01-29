
/*
   FRC Team 456 Siege Robotics
   2014 Competition Code

   Function to read web camera (or video) 
*/

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/legacy/legacy.hpp"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "t456-vision.h"

/*
**  External global variables 
*/
extern camera_struct camera_info;         /* information about camera */
extern proc_struct proc_info;             /* information about processes */

extern pthread_mutex_t  targ_msg_mutex;   /* locking variable */



/*
**  Local Global variables
*/
IplImage      *image[MAXTHREADS];   /* image from webcam */
int framenum = -1;
int camera_img_width, camera_img_height;
int STOP = FALSE;

int ball_detects = 0;

/*
**  Error trapping
*/
void sig_handler( int signo )
{
   /* signal detected, try to stop the program */
   STOP = TRUE;
}

/*
**  Main camera function
*/
void *T456_read_webcam( void *arguments)
{
   int i;
   int frame_sum = 0; 
   float fps_sum = 0.0; 
   double t1, t2; 
   float fps; 
   float minfps = 90000.0;
   float maxfps = 0.0;
   int   thread_index = 0;

   arg_struct *args = (arg_struct *) arguments;

   int  waitkey_delay = 2;
   CvCapture*    camera = 0;

   int return_val[4];
   pthread_t threads[4];
   pthread_attr_t attr;           /* attribute thread */

   printf("Number of arguments: %d\n", args->argc);
   if ( args->argc == 2 ) {
     printf("Processing video from file: %s\n", args->argv[1]);
   }

   /*
   **  Stop on signals
   */
   if ( signal(SIGTERM, sig_handler) == SIG_IGN)
      signal(SIGTERM, SIG_IGN);
   if ( signal(SIGHUP, sig_handler) == SIG_IGN)
      signal(SIGHUP, SIG_IGN);
   if ( signal(SIGINT, sig_handler) == SIG_IGN)
      signal(SIGINT, SIG_IGN);

   if ( proc_info.graphics != 0 )
   {
      cvNamedWindow("Original Image", CV_WINDOW_AUTOSIZE);
   }

   /*
   **  Setup camera capture
   **   0 = /dev/video0
   **   1 = /dev/video1
   */
   if ( args->argc == 2 ) {
     camera=cvCaptureFromFile( args->argv[1] );
   }
   else {
     printf("Capture video from camera (%d)\n", camera_info.camera_id);
     camera=cvCaptureFromCAM( camera_info.camera_id );
   }

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

    printf("image width: %d  image height: %d\n",
              camera_img_width, camera_img_height);

    /*
    **  Specify how long to wait for a key press (in msec)
    */
    waitkey_delay = proc_info.wait_time;  /* don't make less than 20 */

    /*
    **  Loop, process camera image until any key is pressed
    */
    t1 = (double)cvGetTickCount();

    while (cvWaitKey(waitkey_delay) < 0)
    {
       /*
       **  Calculate thread index
       */
       thread_index = framenum % proc_info.nthreads;
       if (thread_index < 0 ) thread_index = 0;
       /*
       **  Grab initial frame from image or movie file
       */
       pthread_mutex_lock( &targ_msg_mutex);
          image[ thread_index ] = cvQueryFrame(camera);
       pthread_mutex_unlock( &targ_msg_mutex);

       /*
       **  Check and see if we reached the end of the movie or received a null
       **   image from the camera
       */
       if ( !(image[ thread_index ]) ) {
          printf("total frames: %d\n", frame_sum);
          printf("average fps: %lf\n", fps_sum / (double)frame_sum);
          printf("max fps: %f\n", maxfps);
          printf("min fps: %f\n", minfps);
 
          printf("total detects: %d\n", ball_detects);
          framenum = -1;
          return(0);
       }

       /*
       **  Display results
       */
       if ( proc_info.graphics != 0 )
       {
          cvShowImage("Original Image",image[ thread_index ]);
       }

       /*  
       ** keep time of processing 
       */
       t2 = (double)cvGetTickCount();
       fps = 1000.0 / ((t2-t1)/(cvGetTickFrequency()*1000.));
       fps_sum += fps;
       if ( (framenum % 5) == 0 )
          printf("time: %6.2fms  fps: %6.2f\n",(t2-t1)/(cvGetTickFrequency()*1000.),fps);

       if ( fps > maxfps ) maxfps = fps;
       if ( fps < minfps ) minfps = fps;

       t1 = t2;

       /*
       **  Update frame number
       */
       framenum++;
       frame_sum = framenum;

       /*
       **  If we catch a stop or kill signal
       */
       if ( STOP ) {
         break;
       }

    }
    framenum = -1;

cvSaveImage("framegrab.jpg", image[0], 0 );


   /*
   **  Release control of camera
   */
   cvReleaseCapture(&camera);

   printf("total frames: %d\n", frame_sum);
   printf("average fps: %lf\n", fps_sum / (double)frame_sum);
   printf("max fps: %f\n", maxfps);
   printf("min fps: %f\n", minfps);

   printf("total detections: %d\n", ball_detects);
}
