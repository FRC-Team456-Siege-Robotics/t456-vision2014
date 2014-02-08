
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
extern tracking_struct tracking_info;     /*  tracking parameters */

extern pthread_mutex_t  targ_msg_mutex;   /* locking variable */
extern char target_message[100];          /* target information message */
extern int  target_message_length;

extern int  ball_color;     /* ball color 0 = red, 1 = blue */

extern int  num_tracked_targets;
extern int  num_detected_targets[MAXTHREADS];
extern int  targets_processed[MAXTHREADS];
target_struct  detected_targets[MAXTHREADS][MAX_TRACKED_TARGETS];
target_struct  tracked_targets[MAX_TRACKED_TARGETS];

/*
**  External function prototypes
*/
extern void T456_change_RGB_to_binary( IplImage *, CvMat *, int, int, int);
extern void draw_target_center( CvPoint , IplImage *, CvScalar );


/*
**  Local Global variables
*/
extern IplImage      *image[MAXTHREADS];   /* image from webcam */
extern int framenum;
extern int camera_img_width, camera_img_height;

extern int ball_detects;


/*
**   Process camera image (frame) and find large ball
** 
**   This is a threaded function that runs async with the other code
*/
void *T456_find_ball(void * idp)
{
   char framename[120];
   int  i;
   long id = (long) idp;
   int  myid = (int) idp;
   int local_framenum;
   int prev_frame = -2;
   CvMat *image_thresh = 0;

   CvSeq *contours = 0;
   CvSeq *seq = 0;
   CvSeq *polydp_contours = 0;
   CvMemStorage *storage = NULL;

   CvPoint2D32f center;
   float radius;
   int   thread_index = 0;
   double t1, t2; 

   IplImage  *local_image = 0;

   float theta_rad, targ_dist;

   /*
   **  Print out a friendly message to say this is working
   */
   fprintf(stderr,"(find_ball) image_proc id: %ld\n", id);
   fprintf(stderr,"***** Thread image width: %d  image height: %d\n",
              camera_img_width, camera_img_height);

   /*
   ** Initialize contour storage variables
   */
   storage = cvCreateMemStorage(0);

   local_image = cvCreateImage(cvSize(camera_img_width,camera_img_height),
                               IPL_DEPTH_8U, 3);
   image_thresh = cvCreateMat(camera_img_height, camera_img_width, CV_8UC1);


#ifdef WRITEVIDEO
   /*   TEMPORARY VIDEO DEBUG */
   CvVideoWriter *writer;

   if ( myid == 0 )
   {
      writer = cvCreateVideoWriter(
                   "video_out.avi",
                   CV_FOURCC('M','J','P','G'),
                   60,
                   cvSize(camera_img_width, camera_img_height),
                   1
                ); 
   }
#endif

   /* ========================================== */
   /*    MAIN PROCESSING SECTION                 */
   /* ========================================== */

  
   /*
   **  Since this operation will begin before the camera
   **   starts to read frames, spin/sleep here until
   **   camera starts and has filled an initial 
   **   nthread frames
   */
   while( framenum < proc_info.nthreads ) {
    // wait until the camera starts sending frames
     usleep(1000.0);  // sleep for 1/30 sec
   }

   /*
   **  Camera is now sending data, 
   **     find out what frame number it currently is on.
   */
   pthread_mutex_lock( &targ_msg_mutex);
      local_framenum = framenum;
   pthread_mutex_unlock( &targ_msg_mutex);

   /*
   **  Process image frames forever until the frame number is
   **   reset to -1
   */
   while( local_framenum != -1 )
   {
      /*
      **   Check and see if this is the frame we are supposed
      **    to process.  
      **    Jedi mind trick with the mod (%) operator
      **       (these are not the droids you are looking for)
      **
      **   and check to make sure this is different frame from last time
      */
      if ( ((local_framenum % proc_info.nthreads)  == myid ) 
            && (local_framenum != prev_frame))
      {

         /*  grab current tick count for timing */
         t1 = (double)cvGetTickCount();

         /*
         **  lock memory and quickly (we hope) copy the camera frame
         **   to a local image frame
         */
         pthread_mutex_lock( &targ_msg_mutex);
            cvCopy(image[myid], local_image, NULL);
         pthread_mutex_unlock( &targ_msg_mutex);

         /*
         **  Convert RGB image frame to HSV and threshold (all at once)
         **   based on ball color we are looking for
         */
         if ( ball_color == 0 )  /* RED ball */
         {
            T456_change_RGB_to_binary(local_image, image_thresh, 
                                             tracking_info.red_val_thresh,
                                             tracking_info.red_hue_mid_thresh,
                                             tracking_info.red_hue_mid_span );
//                                     /* value threshold */    90,
//                                     /* hue mid threshold */ 252,
//                                     /* hue span */          60 );

         } 
         else  /* BLUEBALL */
         {
            T456_change_RGB_to_binary(local_image, image_thresh, 
                                             tracking_info.blue_val_thresh,
                                             tracking_info.blue_hue_mid_thresh,
                                             tracking_info.blue_hue_mid_span);
//                                     /* value threshold */    33,
//                                     /* hue mid threshold */ 160, 
//                                     /* hue span */          60 );
         }

         /*
         **   NOTE: the following two calls require the most processing
         **          time in this function.  Only increase iterations with
         **          extreme caution.
         */

         /*
         **   Erode threshold image to remove small speckles and non-ball
         **    objects
         */
         cvErode(image_thresh, image_thresh, NULL, 15);

         /*
         **   Dilate threshold image back to original sized blobs
         **    always about two iterations larger than Erode
         */
         cvDilate(image_thresh, image_thresh, NULL, 17);

         /*
         ** Find the contours in the input image and store in the
         ** contours list structure.
         */
         cvFindContours( image_thresh, storage, &contours , sizeof(CvContour),
                         CV_RETR_LIST, 
                         CV_CHAIN_APPROX_TC89_KCOS, 
                         cvPoint(0,0) );

         /*  
         **  reset detected target counter 
         **   this is done here to let the target tracking code keep up
         */
         num_detected_targets[myid] = 0;
         targets_processed[myid] = 0;

         /*
         ** Loop through contours and extract the interesting ones
         */
         seq = contours;
         for ( ; seq != 0; seq = seq->h_next )
         {
            /*
            ** Approximate a polygonal shape the contours
            **
            ** eps, method
            ** eps = 4.0;
            */
            polydp_contours = cvApproxPoly(seq, sizeof(CvContour), storage,
                                            CV_POLY_APPROX_DP, 3.0, 0);

            /*
            **  Determine the minimum enclosing circle around the polygon
            **    Note: a half-moon polygonal approximation will reshape
            **          to a full circle.
            */
            cvMinEnclosingCircle(polydp_contours, &center, &radius );

            /*
            **  Filter out circles that are too small
            */
            if ( (int) radius > 40 )
            {
               /* 
               **  calculate target distance 
               **   ball size is 24" diameter (12" radius)
               */          
                       /* calculate angular width of target in radians */
               theta_rad = (camera_info.h_ifov * (float) radius) * 0.0174533f;
                       /* calculate distance (ft) */
                      /*  ball radius / theta / in-to-ft-conversion */
               targ_dist = (12.0f / tanf(theta_rad)) / 12.0f ;

               /*
               **  Draw graphics on local image
               */
               cvCircle( local_image, cvPointFrom32f(center), (int) radius,
                       CV_RGB(0,255,0), 1, 4, 0);

               draw_target_center( cvPointFrom32f(center), 
                                      local_image, CV_RGB(0,255,0) );

               /*
               **  Fill in target detection data structure
               */
               detected_targets[myid][num_detected_targets[myid]].xcent 
                                   = (int) round(center.x);
               detected_targets[myid][num_detected_targets[myid]].ycent 
                                   = (int) round(center.y);
               detected_targets[myid][num_detected_targets[myid]].radius 
                                   = (float) radius;
               detected_targets[myid][num_detected_targets[myid]].dist 
                                   = (float) targ_dist;
         
               detected_targets[myid][num_detected_targets[myid]].frame_tracked 
                                   = (float) local_framenum;

                /*
                **  Lock and increment ball detection counter
                */
                pthread_mutex_lock( &targ_msg_mutex);
                   num_detected_targets[myid]++;
                   ball_detects++;
                pthread_mutex_unlock( &targ_msg_mutex);
   
                /*
                **  Check for num_targets > array size
                */
                if (num_detected_targets[myid] >= MAX_TRACKED_TARGETS )
                {
                   num_detected_targets[myid] = MAX_TRACKED_TARGETS - 1;
                }
            }

         }
         targets_processed[myid] = 1;

         /*
         **  Process timing if required
         */
         if ( proc_info.timing_check != 0 ) 
         {
            t2 = (double)cvGetTickCount();
            if ( ((local_framenum % 5) == 0) && (id == 0) )
            {
               fprintf(stderr," thread elapsed time %6.2fms\n",
                        (t2-t1)/(cvGetTickFrequency()*1000.));
            }
         }

         if ( proc_info.save_frames != 0 ) 
         {
            sprintf(framename,"frames/frame%04d.jpg", local_framenum);
            cvSaveImage(framename, local_image, 0 );
         }

         /*
         **  Copy local_framenum to previous frame number
         */
         prev_frame = local_framenum;

         /*
         **  Display image frame if required
         */
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

      }  /* end if (local_framenum % proc_info.nthreads)  == id  */

      /* 
      **  lock memory and get updated global frame number 
      **   that the webcam is processing
      */
      pthread_mutex_lock( &targ_msg_mutex);
         local_framenum = framenum;
      pthread_mutex_unlock( &targ_msg_mutex);

   } /* end while while( local_framenum != -1 ) */
   
   /*
   **  For diagnostic/error checking, dump out last image frame processed
   */
   sprintf(framename,"framegrab_thresh%02d.jpg", (int)id);
   cvSaveImage(framename, local_image, 0 );
 
#ifdef WRITEVIDEO
   if ( id == 0 )
   {
      cvReleaseVideoWriter(&writer);
   }
#endif

   /*
   ** Clear memory storage  (important!)
   */
   cvClearMemStorage(storage);

   /*
   ** Release memory storage (important!)
   */
   cvReleaseMemStorage(&storage);
   cvReleaseImage( &local_image );
   cvReleaseMat( &image_thresh );

   /*
   ** Tell everyone we are stopping/exiting
   */
   fprintf(stderr,"image_proc id: %ld (exiting)\n", id);

}
