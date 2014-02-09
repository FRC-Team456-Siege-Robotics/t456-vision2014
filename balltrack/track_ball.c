/*
   FRC Team 456 Siege Robotics
   2014 Competition Code

   Track ball(s) across time and frames
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

extern int   ball_color;    /* 0 = RED, 1 = BLUE */

extern int  num_tracked_targets;
extern int  num_detected_targets[MAXTHREADS];
extern int  targets_processed[MAXTHREADS];
target_struct  detected_targets[MAXTHREADS][MAX_TRACKED_TARGETS];
target_struct  tracked_targets[MAX_TRACKED_TARGETS];

extern int framenum;
extern int ball_detects;

/*
**  Process all detected balls in a frame and keep
**   a running account of where they are.
**
**  Send results to message function
*/
void *T456_track_ball()
{
   int local_framenum, prev_frame;
   int frame_indx;
  
   int dropped_frames = 0;
   int skipped_frames = 0;

   /*
   **  Print out a friendly message to say this is working
   */
//   printf("track_ball thread is running\n");

   /*
   **  Update message string
   */
   pthread_mutex_lock( &targ_msg_mutex);
   target_message_length =
               snprintf(target_message, sizeof(target_message),
              "0,0,00.0,00.0");
   pthread_mutex_unlock( &targ_msg_mutex);

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
   **  Pause a bit and let the ball finder code start working
   */
   usleep(133333.33); 

   /*
   **  Camera is now sending data, ball finder code is running, 
   **     find out what frame number it currently is on.
   */
   pthread_mutex_lock( &targ_msg_mutex);
      local_framenum = framenum;
   pthread_mutex_unlock( &targ_msg_mutex);

   prev_frame = -1;
   /*
   **  Process image frames forever until the frame number is
   **   reset to -1
   */
   while( (local_framenum != -1)  )
   {
      if ( local_framenum != prev_frame )
      {
         if ( (local_framenum - prev_frame) > 1 ) 
         {
            fprintf(stderr,"**** DROPPED FRAME**** (%d %d)\n", local_framenum,
                      prev_frame);
            dropped_frames++;
         }

         /*
         **  See if the current frame has a target detected
         */
         frame_indx = (local_framenum-1) % proc_info.nthreads;

         /*
         **  Send out updated target tracking message
         */
         pthread_mutex_lock( &targ_msg_mutex);
         if ( targets_processed[frame_indx] == 1 )
         {
            if ( num_detected_targets[frame_indx] == 0 )
            {
               //  print string for arduino and led lights
               printf("2 0 0\n");
  
               // set message string 
               target_message_length =
                  snprintf(target_message, sizeof(target_message),
                           "%06d,0,00.0,00.0",local_framenum-1);
            }
            else
            {
               //  print string for arduino and led lights
               printf("2 %d %.0f\n", 
                  detected_targets[frame_indx][0].xcent,
                  detected_targets[frame_indx][0].dist);

               // set message string 
               target_message_length =
                  snprintf(target_message, sizeof(target_message),
                           "%06d,1,%3.1f,%3.1f",
                           local_framenum-1,
                    ((float) detected_targets[frame_indx][0].xcent - 320.0f)
                                * camera_info.h_ifov,
                            detected_targets[frame_indx][0].dist
                                );
            }
         } 
         else
         {
           /* we grabbed data before it was ready */
           skipped_frames++;
         }
         pthread_mutex_unlock( &targ_msg_mutex);

//         usleep(33333.33);  /* sleep at roughly 30 fps */
         usleep(proc_info.wait_time * 1000);  /* sleep at same delay as camera */
   
         prev_frame = local_framenum;
      }

      /* 
      **  lock memory and get updated global frame number 
      **   that the webcam is processing
      */
      pthread_mutex_lock( &targ_msg_mutex);
         local_framenum = framenum;
      pthread_mutex_unlock( &targ_msg_mutex);
   }
   pthread_mutex_lock( &targ_msg_mutex);
   target_message_length =
               snprintf(target_message, sizeof(target_message),
              "0,0,00.0,00.0");
   pthread_mutex_unlock( &targ_msg_mutex);
   usleep(99999);  /* sleep at same delay as camera */

   fprintf(stderr," **** Number of dropped frames: %d **** \n", dropped_frames);
   fprintf(stderr," **** Number of skipped frames: %d **** \n", skipped_frames);
   fprintf(stderr," **** Ball tracking thread finished\n");
}
  

