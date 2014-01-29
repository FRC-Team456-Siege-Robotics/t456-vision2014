
/*
  write_video.c

  Threaded function to write processed frame video to a video file

  FRC Team 456
  2014

*/

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/legacy/legacy.hpp"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "t456-vision.h"

extern proc_struct proc_info;             /* information about processes */

extern pthread_mutex_t  targ_msg_mutex;   /* locking variable */
// extern IplImage      *image;   /* image from webcam */
IplImage      *image[MAXTHREADS];   /* image from webcam */
extern int camera_img_width;
extern int camera_img_height;
extern int framenum;

void *T456_write_video ()
{

   CvVideoWriter *writer;
   CvSize         imgSize;
   int            local_framenum;
   IplImage      *local_image = NULL;
   int            prev_frame = -2;
   int            thread_index;

   imgSize.width = camera_img_width;
   imgSize.height = camera_img_height;

   writer = cvCreateVideoWriter(
                "video_out.avi",
                CV_FOURCC('M','J','P','G'),
                60,
                imgSize, 1
             ); 

   pthread_mutex_lock( &targ_msg_mutex);
   local_framenum = framenum;
   pthread_mutex_unlock( &targ_msg_mutex);

   while( framenum <= proc_info.nthreads ) {
      // wait until the camera starts sending frames
      usleep(66666.0);  // sleep for 1/30 sec

   }


   pthread_mutex_lock( &targ_msg_mutex);
   local_framenum = framenum;
   pthread_mutex_unlock( &targ_msg_mutex);

   while( local_framenum != -1 )
   {
      pthread_mutex_lock( &targ_msg_mutex);
         local_framenum = framenum;
      pthread_mutex_unlock( &targ_msg_mutex);

      if ( local_framenum != prev_frame )
      {
         thread_index = local_framenum % proc_info.nthreads;
         pthread_mutex_lock( &targ_msg_mutex);
            cvCopy(image[thread_index], local_image, NULL);
         pthread_mutex_unlock( &targ_msg_mutex);

         cvWriteFrame(writer, local_image);

         prev_frame = local_framenum;
      }

   }

   cvReleaseVideoWriter(&writer);
   cvReleaseImage( &local_image );

}
