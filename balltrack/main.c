
/*
**  FRC Team 456 Siege Robotics
**  Vicksburg, MS
**  2014 Competition Code
**
**  Usage: exec_name config_file
**
**  NOTES:
**  This is multi-threaded (multi-core) code 
**  
**  Threads:
**    1) UDP communication to send out target information
**    2..n) Camera I/O and target processing
**   
*/
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/legacy/legacy.hpp"

#include "t456-vision.h"


/*
**  External function prototypes
*/
extern void *send_udp_message_func();
extern void *T456_camera_funcs(void *);
extern void T456_parse_vision( char *);
extern void T456_print_settings();
extern void *T456_write_video ();
extern void *T456_find_ball(void *);
extern void *T456_track_ball();

/*
**  Global variables for messages
*/
pthread_mutex_t  targ_msg_mutex;        /* locking variable */
char target_message[100];               /* message between processes */
int  target_message_length;             /* length of message */

int  REDBALL = 1;
int ball_detects = 0;

/*
**  Global variables about the camera and processing
*/
CvCapture*    camera = 0;
camera_struct camera_info;              /* information about the camera */
proc_struct   proc_info;                /* information about processing */
int camera_img_width, camera_img_height;

int  num_tracked_targets = 0;
int  num_detected_targets[MAXTHREADS];
int  targets_processed[MAXTHREADS];
target_struct  detected_targets[MAXTHREADS][MAX_TRACKED_TARGETS];
target_struct  tracked_targets[MAX_TRACKED_TARGETS];

/* 
** =================================================================
**  MAIN program
** =================================================================
*/
int main( int argc, char **argv)
{
   int i;
   arg_struct args;

   pthread_t udp_msg_thread;      /* thread for messages out via UDP */
   pthread_t video_record_thread;      /* thread for messages out via UDP */
   pthread_t cam_thread;          /* thread for camera processing */
   pthread_attr_t attr;           /* attribute thread */

   int  msg_ret_val;
   int  cam_ret_val;
   int  cam_ret_val2;

   int return_val[4];
   pthread_t threads[4];
   pthread_t ball_track_thread;

   /*
   **  Parse the config file and get parameters for the program
   */
   T456_parse_vision( "t456-vision.ini" );
//   T456_print_settings();

   /*
   **  Setup camera capture
   **   0 = /dev/video0
   **   1 = /dev/video1
   */
   if ( argc == 2 ) {
     camera=cvCaptureFromFile( argv[1] );
   }
   else {
     fprintf(stderr,"Capture video from camera (%d)\n", camera_info.camera_id);
     camera=cvCaptureFromCAM( camera_info.camera_id );
   }

   /*
   **   Check and see if camera/file capture is valid
   */
   if (!camera) {
       fprintf(stderr,"camera or image is null\n");
       return;
   }

   //  Get camera information (image height and width)
   camera_img_width = cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH);
   camera_img_height = cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT);

   fprintf(stderr,"image width: %d  image height: %d\n",
              camera_img_width, camera_img_height);


   /*
   **  One time function call to XInitThreads if threads need to 
   **   share a X11 graphic
   */
   if ( proc_info.graphics != 0 )
   {
      XInitThreads();        
   }

   /*
   **  Copy command line arguments into data structure
   */
   args.argc = argc;               
   args.argv = argv;

   /*  initialize and set attribute thread variable */
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

   /*
   **  Initialize initial message to control system
   **   this is done before spawning communication thread
   */
   target_message_length =
                   snprintf(target_message, sizeof(target_message),
                  "-1,00,000000,000000,000000,0000");

   /*
   **  Initialize the mutex (locking/unlocking for message info)
   */
   if (pthread_mutex_init(&targ_msg_mutex, NULL))
   {
      printf("Unable to initialize a mutex for message\n");
      exit(-1);
   }

   /*
   **  Create and execute communication thread 
   */
   msg_ret_val = pthread_create( &udp_msg_thread, NULL,
                                 &send_udp_message_func, NULL);

   /*
   **  Setup and launch camera frame image processing threads
   */
   for ( i = 0; i < proc_info.nthreads; i++ )
   {
       return_val[i] = pthread_create( &threads[i], NULL,
                                 &T456_find_ball, (void *) i );
   }

   /*
   **  Setup and launch ball tracking process
   */
   msg_ret_val = pthread_create( &ball_track_thread, NULL,
                                 &T456_track_ball, NULL);

   /*
   **  Start reading image frames from webcam (or video)
   **   main loop is in this function call.
   */
   T456_read_webcam( (void *) &args);

   /*
   **  Wait nicely for image processing threads to finish
   */
   for ( i = 0; i < proc_info.nthreads; i++ )
   {
      pthread_join( threads[i], NULL);
   }
   fprintf(stderr,"total detects: %d\n", ball_detects);

   /*
   **  Wait for ball tracking to finish
   */
   pthread_join( ball_track_thread, NULL);

   /*
   **  Just kill the udp message thread (sorry)
   */
   pthread_kill( udp_msg_thread, NULL);

   /*
   **  All finished, exit nicely
   */
   exit(0);
}

