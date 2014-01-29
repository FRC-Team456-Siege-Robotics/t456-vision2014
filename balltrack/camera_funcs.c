
/*
   FRC Team 456 Siege Robotics
   2014 Competition Code

   Camera and target tracking code
*/

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/legacy/legacy.hpp"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

#include "t456-vision.h"

/*
**  External global variables 
*/
extern camera_struct camera_info;         /* information about camera */
extern proc_struct proc_info;             /* information about processes */

extern pthread_mutex_t  targ_msg_mutex;   /* locking variable */
extern char target_message[100];          /* target information message */
extern int  target_message_length;


/*
**  External function prototypes
*/
extern void T456_change_RGB_to_binary( IplImage *, CvMat *, int, int, int);
extern void draw_target_center( CvPoint , IplImage *, CvScalar );


/*
**  Local Global variables
*/
IplImage      *image[MAXTHREADS];   /* image from webcam */
int framenum = -1;
int camera_img_width, camera_img_height;
int STOP = FALSE;

int ball_detects = 0;
/*
**  Local function prototypes
*/
static void *T456_image_proc(void * idp);

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
void *T456_camera_funcs( void *arguments)
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
//      cvNamedWindow("Thresh Image",CV_WINDOW_AUTOSIZE);
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
    **  initialize and set attribute thread variable 
    **    this is for mult-core processing of images
    */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    /*
    **  Time estimation variable
    */
    t1 = (double)cvGetTickCount();

    /*
    **  Specify how long to wait for a key press (in msec)
    */
    waitkey_delay = proc_info.wait_time;  /* don't make less than 20 */

    /*
    **  Setup and launch image processing threads
    */
    for ( i = 0; i < proc_info.nthreads; i++ )
    {
       return_val[i] = pthread_create( &threads[i], NULL,
                                 &T456_image_proc, (void *) i );
    }

    /*
    **  Loop, process camera image until any key is pressed
    */
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
       **  Update message string
             NOTE: not sure if this belongs here....
       */
       pthread_mutex_lock( &targ_msg_mutex);
       target_message_length =
                   snprintf(target_message, sizeof(target_message),
                  "%d,00,000000,000000,000000,0000",framenum);
       pthread_mutex_unlock( &targ_msg_mutex);

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

    /*
    **  Wait for threads to finish
    */
    for ( i = 0; i < proc_info.nthreads; i++ )
    {
       pthread_join( threads[i], NULL);
    }


}

/*
**   Process camera image (frame) main function
*/
static void *T456_image_proc(void * idp)
{
   char framename[120];
   char stringid[40];
   int  i;
   long id = (long) idp;
   int local_framenum;
   int prev_frame = -2;
   CvMat *image_thresh = 0;

   CvSeq *convex_contours = 0;
   CvSeq *contours = 0;
   CvSeq *seq = 0;
   CvSeq *polydp_contours = 0;
   CvSeq *detected_circles = 0;
   CvMemStorage *storage = NULL;
   CvMemStorage *hull_storage = NULL;
   CvMemStorage *circle_storage = NULL;
   double area = 0;
   CvPoint2D32f center;
   float radius;
   int   thread_index = 0;

   double t1, t2; 

   IplImage  *local_image = 0;

   IplConvKernel *morph_kernel;

   CvVideoWriter *writer;
   CvSize imgSize;

   printf("image_proc id: %ld\n", id);
   sprintf(stringid,"id %ld\n", id);


   /*
   ** Initialize contour storage variables
   */
   storage = cvCreateMemStorage(0);
   hull_storage = cvCreateMemStorage(0);
   circle_storage = cvCreateMemStorage(0);

   local_image = cvCreateImage(cvSize(camera_img_width,camera_img_height),
                               IPL_DEPTH_8U, 3);
   image_thresh = cvCreateMat(camera_img_height, camera_img_width, CV_8UC1);

   /*
   **  Construct a morphological kernel
   */
   morph_kernel = cvCreateStructuringElementEx(9, 9, 1, 1,
                                                CV_SHAPE_ELLIPSE, NULL);
 
   imgSize.width = camera_img_width;
   imgSize.height = camera_img_height;

   /*   TEMPORARY VIDEO DEBUG */
#ifdef WRITEVIDEO
   if ( id == 0 )
   {
      writer = cvCreateVideoWriter(
                   "video_out.avi",
                   CV_FOURCC('M','J','P','G'),
                   60,
                   imgSize, 1
                ); 
   }
#endif

   /* ========================================== */

   while( framenum < proc_info.nthreads ) {
    // wait until the camera starts sending frames
     usleep(66666.0);  // sleep for 1/30 sec
   }

   pthread_mutex_lock( &targ_msg_mutex);
      local_framenum = framenum;
   pthread_mutex_unlock( &targ_msg_mutex);

   while( local_framenum != -1 )
   {

      if (    ((local_framenum % proc_info.nthreads)  == id ) 
            && (local_framenum != prev_frame))
      {
         t1 = (double)cvGetTickCount();
         printf("image_proc id: %ld process frame %d\n", id, local_framenum);

         pthread_mutex_lock( &targ_msg_mutex);
            cvCopy(image[id], local_image, NULL);
         pthread_mutex_unlock( &targ_msg_mutex);

         T456_change_RGB_to_binary(local_image, image_thresh, 
                                     /* value threshold */    15,
                                     /* hue mid threshold */ 252, 
                                     /* hue span */          60 );


         cvErode(image_thresh, image_thresh, NULL, 19);

         cvDilate(image_thresh, image_thresh, NULL, 21);

         /*
         **  Find the circles in the input image and store in the 
         **   circle list structure.
         */
//         detected_circles = cvHoughCircles( image_thresh, circle_storage,
//                    CV_HOUGH_GRADIENT,
//                    1,
//                    200,     /* DON't CHANGE minimum distance between centers */
//                    645,    /* DON't CHANGE upper threshold for detector */
//                    11,     /* DON't CHANGE threshold for center detection */
//                    20,      /* min radius */
//                    240);     /* max radius */


//         if ( detected_circles->total != 0 )
         if ( FALSE )
         {
            printf("num circles: %d\n",detected_circles->total);
   
            for ( i = 0; i < detected_circles->total; i++ )
            {
               float *p = (float *) cvGetSeqElem( detected_circles, i);
   
               if ( proc_info.graphics != 0 )
               {
                  if ( (id == 0) || (id == 1) )
                  {
                     cvCircle( local_image, 
                       cvPoint(cvRound(p[0]), cvRound(p[1])), cvRound(p[2]),
                       CV_RGB(180,180,180), 1, 8, 0);
             draw_target_center( cvPoint(cvRound(p[0]), cvRound(p[1])),
                                 local_image,
                                 CV_RGB(255,255,255) );
                  }
               }
            }
         }

        /*
        ** Find the contours in the input image and store in the
        ** contours list structure.
        */
        cvFindContours( image_thresh, storage, &contours , sizeof(CvContour),
                    CV_RETR_LIST, CV_CHAIN_APPROX_TC89_KCOS, cvPoint(0,0) );

        /*
        ** Loop through contours and extract the interesting ones
        */
         seq = contours;
         for ( ; seq != 0; seq = seq->h_next )
         {

            /*
            ** Approximate polygonal shape from contours
            **
            ** eps, method
            ** eps = 4.0;
            */
            polydp_contours = cvApproxPoly(seq, sizeof(CvContour), storage,
                                            CV_POLY_APPROX_DP, 3.0, 0);

            cvMinEnclosingCircle(polydp_contours, &center, &radius );

             if ( (int) radius > 40 )
             {
               printf("  circle at (%d,%d)\n", (int) round(center.x), 
                                               (int) round(center.y) );
               cvCircle( local_image, cvPointFrom32f(center), (int) radius,
                       CV_RGB(0,255,0), 1, 4, 0);

                draw_target_center( cvPointFrom32f(center), 
                                      local_image, CV_RGB(0,255,0) );

                pthread_mutex_lock( &targ_msg_mutex);
                ball_detects++;
                pthread_mutex_unlock( &targ_msg_mutex);
             }

         }

         if ( proc_info.timing_check != 0 ) 
         {
            t2 = (double)cvGetTickCount();
            if ( ((local_framenum % 5) == 0) && (id == 0) )
            {
               printf(" thread elapsed time %6.2fms\n",
                        (t2-t1)/(cvGetTickFrequency()*1000.));
            }
         }


         /*
         **  Show image
         */
         if ( proc_info.save_frames != 0 ) 
         {
            sprintf(framename,"frames/frame%04d.jpg", local_framenum);
            cvSaveImage(framename, local_image, 0 );
         }

         prev_frame = local_framenum;

         if ( proc_info.graphics != 0 )
         {
            if ( id == 0 )
            {
               cvShowImage("Thresh Image",local_image);
#ifdef WRITEVIDEO
               cvWriteFrame(writer, local_image);
#endif
            }
         }
      }



      /* get global frame number */
      pthread_mutex_lock( &targ_msg_mutex);
         local_framenum = framenum;
      pthread_mutex_unlock( &targ_msg_mutex);

   } 
   
   sprintf(framename,"framegrab_thresh%02d.jpg", (int)id);
   cvSaveImage(framename, local_image, 0 );
 
#ifdef WRITEVIDEO
   if ( id == 0 )
   {
      cvReleaseVideoWriter(&writer);
   }
#endif

   /*
   ** Clear mem storage  (important!)
   */
   cvClearMemStorage(storage);
   cvClearMemStorage(hull_storage);
   cvClearMemStorage(circle_storage);

   /*
   ** Release mem storage (important!)
   */
   cvReleaseMemStorage(&storage);
   cvReleaseMemStorage(&hull_storage);
   cvReleaseMemStorage( &circle_storage );


   cvReleaseImage( &local_image );
   cvReleaseMat( &image_thresh );

   printf("image_proc id: %ld (exiting)\n", id);

}
