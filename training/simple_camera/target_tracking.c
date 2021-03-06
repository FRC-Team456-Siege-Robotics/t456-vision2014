/*
**   Process video, find, and track rectangles for FRC 2013 competition
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
#include <signal.h>

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include "camera_info.h"
#include "target_info.h"

#define TRUE 1
#define FALSE 0

/*
**  GLOBAL variables
*/
int           STOP = FALSE;
CvCapture*    camera = 0;
int           num_detect_targets;
target_struct detected_targets[MAX_TRACKED_TARGETS];
int           num_tracked_targets;
target_struct tracked_targets[MAX_TRACKED_TARGETS];
double        fps_sum = 0.0;
int           frame_cnt = 1;
IplImage      *image = 0;

CvFont        font;

char target_message[100];
int  target_message_length;

double time_sum = 0;

camera_struct  camera_info;
tracking_struct tracking;
lut_struct lut_3pt;
lut_struct lut_2pt;

int  pid;
char filename[240];
/*
**  Local function prototypes
*/
void done();
void find_contours( CvMat *);
void Detect_Targets( CvSeq *, CvMat *);
static double cos_angle( float , float , float , float, float, float );
void targ_info_copy( target_struct *, target_struct *);

void track_targets_over_time( int );
void draw_target_center( target_struct , IplImage *, CvScalar );
void draw_target_dot( target_struct , IplImage *, CvScalar );

/*
**  External function prototypes
*/
extern int  T456_select_main_target( int );   /* located in target_logic.c */
extern void T456_calculate_aimpoint( int );      /* adjust aimpoint */
extern void T456_change_RGB_to_HV( IplImage *, CvMat *, CvMat * ); /* located in target_color.c */
extern void T456_change_RGB_to_binary( IplImage *, CvMat *);
extern void T456_filter_image( unsigned char , unsigned char , unsigned char , 
                 unsigned char *);

extern void T456_parse_vision( char * );
extern void T456_print_camera_and_tracking_settings();

/*
**  External server function prototypes
*/
void T456_start_server(void);
void T456_stop_server(void);

/*
**  Error trapping
*/

void sig_handler( int signo )
{
   /* signal detected, try to stop the program */
   STOP = TRUE;
}

/*
** ================ BEGIN MAIN SECTION =======================
*/
void target_tracking( int argc, char** argv )
{
    double t1, t2, t3, t4, fps;
    int  targ_selected = 0;
    int  camera_img_height, camera_img_width, camera_img_fps;
    int  waitkey_delay = 2;

    CvMat *image_gray = 0;
    CvMat *image_binary = 0;

    IplConvKernel *morph_kernel;

    CvVideoWriter *writer;
    CvSize imgSize;

    int i;
    
    pid = (int) getpid();
    /*
    **  See if we can catch signals
    */
    if ( signal(SIGTERM, sig_handler) == SIG_IGN) 
       signal(SIGTERM, SIG_IGN);
    if ( signal(SIGHUP, sig_handler) == SIG_IGN) 
       signal(SIGHUP, SIG_IGN);
    if ( signal(SIGINT, sig_handler) == SIG_IGN) 
       signal(SIGINT, SIG_IGN);

    /*
    **  Set the number of tracked targets to zero
    **   tracked targets are persistent targets for a sequence of frames/images
    **   detected targets are the targets detected from a single frame or image
    */
    num_tracked_targets = 0;

    /*  
    **  Capture images from webcam /dev/video0 
    **   /dev/video0 = 0
    **   /dev/video1 = 1
    */
    if ( argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
    {
       printf(" Capturing image from camera\n");
       camera=cvCaptureFromCAM( argc == 2 ? argv[1][0] - '0' : 0 );
    } 
    else 
    {
       /* 
       **  Capture image from file at command line
       **   useful for playback video and testing
       */
       camera = cvCaptureFromFile( argv[1] );
    }


    /*
    **   Check and see if camera/file capture is valid
    */
    if (!camera) {
        printf("camera or image is null\n");
        return;
    }

    /*
    **  Get camera properties
    */
    camera_img_width = cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH);
    camera_img_height = cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT);
    camera_img_fps = cvGetCaptureProperty(camera, CV_CAP_PROP_FPS);

    cvSetCaptureProperty( camera, CV_CAP_PROP_EXPOSURE, 500.0);
    cvSetCaptureProperty( camera, CV_CAP_PROP_EXPOSURE, 500.0);
    cvSetCaptureProperty( camera, CV_CAP_PROP_AUTO_EXPOSURE, 0.0);

    imgSize.width = camera_img_width;
    imgSize.height = camera_img_height;
    /*
    **  Parse the config file
    */
//    T456_parse_vision( "../config/t456-vision.ini" );
    T456_parse_vision( "/usr/local/config/t456-vision.ini" );
    T456_print_camera_and_tracking_settings();

    /*
    **  Start server listening on port 8080
    */
    T456_start_server();

    /*
    **  Setup graphic display windows using OpenCV
    */
#ifdef GRAPHICS
    cvNamedWindow("original", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("binary image", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("contours", CV_WINDOW_AUTOSIZE);
//    cvNamedWindow("original", CV_WINDOW_KEEPRATIO);
printf("camera_img_fps: %d\n", camera_img_fps);
 
    camera_img_fps = 30;

    if (camera_img_fps < 0 ) {
       camera_img_fps = 30;
       printf("camera_img_fps: %d\n", camera_img_fps);
    }

    sprintf(filename,"video_out.avi");

    writer = cvCreateVideoWriter(
                filename,
                CV_FOURCC('M','J','P','G'),
                camera_img_fps,
                imgSize, 1
             );    

    waitkey_delay = (int) ((1.0f / (float) camera_img_fps) * 1000.0f);
    printf("waitkey_delay: %d\n", waitkey_delay);
    if ( (waitkey_delay > 50) || (waitkey_delay == 0) ) waitkey_delay = 2;
#endif /* GRAPHICS */

    cvInitFont( &font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0.0, 1, 8);

    /*
    **  Construct a morphological kernel
    */
    morph_kernel = cvCreateStructuringElementEx(3, 3, 1, 1, 
                                                CV_SHAPE_RECT, NULL); 
    /*
    **
    */
    image_gray = cvCreateMat(camera_img_height, camera_img_width, CV_8UC1);
    image_binary = cvCreateMat(camera_img_height, camera_img_width, CV_8UC1);

    /*
    **  Time estimation variable
    */
    t1 = (double)cvGetTickCount();

    /*
    **   Process images until key press
    */
waitkey_delay = 2.0;

#ifdef GRAPHICS
    while (cvWaitKey(waitkey_delay) < 0)
#else
    while (1)
#endif
    {

       /*  
       ** reset detected targets 
       */
       num_detect_targets = 0;

       /*
       **  Grab initial frame from image or movie file
       */
       image = cvQueryFrame(camera);
       if ( !image ) {
          done();
          return;
       }

       /*
       **  Convert RGB image to Hue and Value images (local function)
       **
       **  Threshold out the lime green color from the hue image
       **    it is a bracket threshold, so two images are produced
       **      
       **  (HUE_MID_THRESH - 15) <  target_color < (HUE_MID_THRESH + 15)
       **
       ** Threshold value image. Because of illumination, the targets should
       **  be very bright in the image.
       **
       **  If pixel value of the image is > VAL_THRESH
       **  then output 255
       **  else output 0
       **
       **  Combine hue1, hue2, and value images with an AND operation
       **   output = hue1 AND hue1 AND value
       **  if a pixel in any of these images is pixel = 0 
       **      then the output is 0
       */

       T456_change_RGB_to_binary( image, image_binary );
        
        
       t4 = (double)cvGetTickCount();
       /*
       **   Use the Morph function to join broken or speckled rectangles
       */
       cvMorphologyEx( image_binary, image_gray, NULL,
                         morph_kernel, CV_MOP_CLOSE, 
                         tracking.morph_closing_iterations);

       t3 = (double)cvGetTickCount();
       time_sum += (t3-t4)/(cvGetTickFrequency()*1000.);
 
        /*
        **  Process contours and detect targets of interest
        */
        find_contours( image_gray );

#ifdef GRAPHICS
        cvShowImage("contours",image_gray);
#endif

        /*
        **  Process raw target detections and compare with current 
        **  tracked targets
        */
        track_targets_over_time( frame_cnt );

        /*
        **  Rank and select highest scoring target
        */
        if ( num_tracked_targets > 0 )
        {
           T456_calculate_aimpoint( frame_cnt );
           targ_selected = T456_select_main_target( frame_cnt );
           draw_target_dot( tracked_targets[targ_selected], 
                                 image, CV_RGB(0,255,0) );
        }

        /*
        **  Print out tracked targets
        */
        printf("%d ", frame_cnt);
        for ( i = 0; i < num_tracked_targets; i++ ) 
        {
             printf("%d ", tracked_targets[i].type);
             printf("%.2f %.2f ", tracked_targets[i].h_angle, 
                                  tracked_targets[i].v_angle);
             printf("%.2f %.2f ", tracked_targets[i].xcenter, 
                                  tracked_targets[i].ycenter);
             printf("(%.2f) ", tracked_targets[i].aspect_ratio);
             printf("(d: %.2f) ", tracked_targets[i].distance);
             printf("%d ", tracked_targets[i].time_tracked);

             draw_target_center( tracked_targets[i],
                                  image, CV_RGB(255,255,0) );
        }
        printf("\n");

        /*
        **  pass selected target information into target message
        **   for the webservice
        */
        if ( num_tracked_targets == 0 ) {
           target_message_length =
              snprintf(target_message, sizeof(target_message),
              "%06d,00,000000,000000,000000,0000", frame_cnt);
        } else {
           target_message_length =
              snprintf(target_message, sizeof(target_message),
               "%06d,%02d,%06.2f,%06.2f,%06.1f,1.0", 
                    frame_cnt,
                    tracked_targets[targ_selected].type,
                    tracked_targets[targ_selected].aim_h_angle, 
                    tracked_targets[targ_selected].aim_v_angle,
                    tracked_targets[targ_selected].distance);
        }


       cvWriteFrame(writer, image);

#ifdef GRAPHICS
        /*
        **  Display images
        */
        cvShowImage("original",image);
        cvShowImage("binary image",image_binary);
#endif /* GRAPHICS */

        /*  
        ** keep time of processing 
        */
        t2 = (double)cvGetTickCount();
        fps = 1000.0 / ((t2-t1)/(cvGetTickFrequency()*1000.));
        fps_sum = fps_sum + fps;
        frame_cnt++;

#ifdef DIAG
        printf("time: %gms  fps: %.2g\n",(t2-t1)/(cvGetTickFrequency()*1000.),fps);
#endif

        t1 = t2;

        /*
        **  If we catch a stop or kill signal
        */
        if ( STOP ) {
          break;
        }


    }

cvSaveImage("framegrab.jpg", image, 0 );
cvSaveImage("framegrab_bin.jpg", image_binary, 0 );
cvSaveImage("framegrab_gray.jpg", image_gray, 0 );

    /*
    **  Release camera resources
    */
    cvReleaseVideoWriter(&writer);
    cvReleaseCapture(&camera);

    /*
    **  Print out timing information
    */
    done();

    /*
    **  Stop server listening on port 8080
    */
    T456_stop_server();

}

/*
** ==================== END MAIN SECTION =======================
*/

void done()
{
  printf("total frames: %d\n", frame_cnt);
  printf("average fps: %lf\n", fps_sum / (double)frame_cnt);
}


void find_contours( CvMat *input_image )
{
   CvSeq *contours = 0;
   CvMemStorage *storage = NULL;
   CvSeq *seq = 0;

   CvSeq *convex_contours = 0;
   CvSeq *polydp_contours = 0;
   CvMemStorage *hull_storage = NULL;
   float area;
   

   storage = cvCreateMemStorage(0);
   hull_storage = cvCreateMemStorage(0);

   /*
   **  Find the contours in the input image and store in the 
   **   contours list structure.
   */
   cvFindContours( input_image, storage, &contours , sizeof(CvContour),
                     CV_RETR_LIST, CV_CHAIN_APPROX_TC89_KCOS, cvPoint(0,0) );

   /*
   **  Now that we have found contours, zero out the original image to remove noise
   */

   seq = contours;
   for ( ; seq != 0; seq = seq->h_next ) 
   {
      /*
      **  Calculate convex hull of the contour
      */
      convex_contours = cvConvexHull2(seq, storage, CV_CLOCKWISE, 1 );

      /* 
      **  Calculate area of the geometry
      */ 
      area = fabsf(cvContourArea( convex_contours, CV_WHOLE_SEQ, 0));

      /*  
      **  Filter out only shapes that are target sized > MIN_RECT_AREA
      */
      if ( area > tracking.min_rect_area ) 
      {
         /*
         **  Approximate polygonal shape
         */
         polydp_contours = cvApproxPoly(seq, sizeof(CvContour), storage, 
                                        CV_POLY_APPROX_DP, 4.0, 0);

         /*
         **  Draw poly contours on image for diagnostics
         */
#ifdef GRAPHICS
         cvDrawContours( input_image, polydp_contours, 
                         CV_RGB(255,255,255), CV_RGB(255,255,255), 
                         0, 1, 8, cvPoint(0,0));
#endif
         /*
         **  If the shape has four corners, check and see it is a target
         */
         if ( polydp_contours->total == 4 )
            Detect_Targets( polydp_contours, input_image );

      }
   }

   /*
   ** Clear mem storage  (important!)
   */
   cvClearMemStorage(storage);
   cvClearMemStorage(hull_storage);

   /*
   ** Release mem storage (important!)
   */
   cvReleaseMemStorage(&storage);
   cvReleaseMemStorage(&hull_storage);

}

/*
**  Target Detection Subroutine
**
**     contours that are passed in here have similar area to 
**     the targets and have 4 corners
*/
void Detect_Targets( CvSeq *raw_contours, CvMat *input_image )
{
   int i,j;
   float xpt[4], ypt[4], xcenter, ycenter;
   float xmin, xmax, ymin, ymax;
   float length_1, length_2;
   float aspect_ratio;
   float distance;
   float dx1, dy1, dx2, dy2;

   CvPoint *point;

   int      isTarget = FALSE;

   xmax = ymax = 0.0;
   xmin = ymin = 1e20;

   float cosine1, cosine2, max_cosine;

   /*
   **
   */
   if ( raw_contours->total == 4 )
   {
      /*
      **  Determine min and max extents
      */
      for ( i = 0; i < raw_contours->total; i++)
      {
         point = CV_GET_SEQ_ELEM( CvPoint, raw_contours, i);

         xpt[i] = (float) point->x;
         ypt[i] = (float) point->y;
         xmin = MIN(xmin, xpt[i]);
         xmax = MAX(xmax, xpt[i]);
         ymin = MIN(ymin, ypt[i]);
         ymax = MAX(ymax, ypt[i]);
      }

      dx1 = fabsf(xpt[0] - xpt[1]);
      dy1 = fabsf(ypt[0] - ypt[1]);
      dx2 = fabsf(xpt[1] - xpt[2]);
      dy2 = fabsf(ypt[1] - ypt[2]);

      length_1 = sqrtf( dx1*dx1 + dy1*dy1 );

      length_2 = sqrtf( dx2*dx2 + dy2*dy2 );

      /*
      **  Look at only horizontal rectangles
      **  If the rectangle is not horizontal, it isn't a target.
      */
      if ( (MAX(dx1,dx2) < MAX(dy1,dy2)) )
      {
         return;
      }


      /*
      **  Calculate the angle of the first corner
      **   if it isn't 90 deg (+- 15 deg) then we don't have
      **   a rectangle and reject
      */
      cosine1 =  fabsf( cos_angle( xpt[0], ypt[0],  
                                  xpt[2], ypt[2],
                                  xpt[1], ypt[1] ) );
      cosine2 =  fabsf( cos_angle( xpt[1], ypt[1],  
                                  xpt[3], ypt[3],
                                  xpt[2], ypt[2] ) );
      max_cosine = MAX( cosine1, cosine2 );
      // was 0.25
      if ( max_cosine > 0.45 ) {
         return;
      }

      /* 
      **  Calculate aspect ratio
      */
      aspect_ratio = MAX( length_1/length_2, length_2/length_1);

      /*
      **  Calculate center of object
      */
      xcenter = ((xmax - xmin) / 2.0) + xmin;
      ycenter = ((ymax - ymin) / 2.0) + ymin;

      /*  
      **  check for high goal opening (3pt) target.
      **   aspect ratio = 4.5
      */
      if ( ((aspect_ratio > 4.2) && (aspect_ratio < 7.6)) ) // was 4.8
//        || ((aspect_ratio > 5.1) && (aspect_ratio < 7.6)) ) 
      {
         /*
         **  Calculate distance to target
         **    the opening target 3 is 54" wide by 12" tall
         */
         distance = 0.01745 * ((MAX(length_1,length_2) / 640.0) * 48.8) / 2.0;
         distance = (54.0/2.0) / tanf(distance);

         if ( (distance/12.0) < 45 ) /* field is 54 ft long */
         {
            isTarget = TRUE;
            detected_targets[num_detect_targets].type = 3;
            detected_targets[num_detect_targets].distance = distance/12.0;
         }
      }

      /*  
      **  check for high goal outside boundary (3pt) target.
      **    62" by 20"
      **   aspect ratio = 3.1
      */
      if ( ((aspect_ratio >= 2.8) && (aspect_ratio < 3.2)) ) 
      {
//  printf("outside 3 pt boundary detect\n");
         /*
         **  Calculate distance to target
         **    the outside of target 3 is 62"wide by 20" tall
         */
         distance = 0.01745 * ((MAX(length_1,length_2) / 640.0) * 48.8) / 2.0;
         distance = (62.0/2.0) / tanf(distance);

         if ( (distance/12.0) < 45 ) 
         {
            isTarget = TRUE;
            detected_targets[num_detect_targets].type = 3;
            detected_targets[num_detect_targets].distance = distance/12.0;
         }

      }

      /*  
      **  check for middle goal opening (2pt) target.
      **   54" x 21"
      **   aspect ratio = 2.6
      */
      if ( ( (aspect_ratio > 2.4) && (aspect_ratio < 2.8) )  )
//        || ((aspect_ratio >= 2.8) && (aspect_ratio < 3.3)) )
      {
         /*
         **  Calculate distance to target
         **    the opening 2pt target is 54"wide by 21" tall
         */
         distance = 0.01745 * ((MAX(length_1,length_2) / 640.0) * 48.8) / 2.0;
         distance = (54.0/2.0) / tanf(distance);

         if ( (distance/12.0) < 45 ) /* field is 54 ft long */
         {
            isTarget = TRUE;
            detected_targets[num_detect_targets].type = 2;
            detected_targets[num_detect_targets].distance = distance/12.0;
         }
      }

      /*  
      **  check for middle goal border (2pt) target.
      **    62" x 29"
      **   aspect ratio = 2.14
      **    2.27, 2.3, 2.29
      */
      if ( ( (aspect_ratio > 1.8) && (aspect_ratio <= 2.4) )  ) 
      {
         /*
         **  Calculate distance to target
         **    the outside of 2pt target is 62" wide by 29" tall
         */
         distance = 0.01745 * ((MAX(length_1,length_2) / 640.0) * 48.8) / 2.0;
         distance = (62.0/2.0) / tanf(distance);

         if ( (distance/12.0) < 45 ) 
         {
            isTarget = TRUE;
            detected_targets[num_detect_targets].type = 2;
            detected_targets[num_detect_targets].distance = distance/12.0;
         }
      }

      /*
      **  See if we determined that one of the previous conditions
      **   were met, if so (isTarget = TRUE) then save all the 
      **   target information
      */
      if ( isTarget ) 
      {
         detected_targets[num_detect_targets].aspect_ratio = aspect_ratio;
         detected_targets[num_detect_targets].xcenter = xcenter;
         detected_targets[num_detect_targets].ycenter = ycenter;

         detected_targets[num_detect_targets].xmin = xmin;
         detected_targets[num_detect_targets].ymin = ymin;
         detected_targets[num_detect_targets].xmax = xmax;
         detected_targets[num_detect_targets].ymax = ymax;

         detected_targets[num_detect_targets].time_tracked = 1;

         if ( length_1 > length_2 ) {
            detected_targets[num_detect_targets].h_length = length_1;
            detected_targets[num_detect_targets].v_length = length_2;
         } else {
            detected_targets[num_detect_targets].h_length = length_2;
            detected_targets[num_detect_targets].v_length = length_1;
         }

         for ( j = 0; j < 4; j++ ) {
            detected_targets[num_detect_targets].xpt[j] = xpt[j];
            detected_targets[num_detect_targets].ypt[j] = ypt[j];
         }

         detected_targets[num_detect_targets].area = 
             fabsf(cvContourArea( raw_contours, CV_WHOLE_SEQ, 0));
         
         detected_targets[num_detect_targets].h_angle =
              (xcenter - camera_info.h_pixels/2.0) * camera_info.h_ifov;
         detected_targets[num_detect_targets].v_angle =
              (ycenter - camera_info.v_pixels/2.0) * camera_info.v_ifov * -1;
         
         /*
         **  calculate angular size 
         */
         detected_targets[num_detect_targets].h_len_deg = 
            detected_targets[num_detect_targets].h_length * camera_info.h_ifov;
         detected_targets[num_detect_targets].v_len_deg = 
            detected_targets[num_detect_targets].v_length * camera_info.v_ifov;
           
         
         /* draw target center */

//         cross_color = CV_RGB(255,255,255);
//         draw_target_center( detected_targets[num_detect_targets],
//                             image, cross_color );

         if ( num_detect_targets < (MAX_TRACKED_TARGETS -1) )
         {
            num_detect_targets++;
         }  else {

         }

      }

    if (isTarget)
      cvDrawContours( input_image, raw_contours, 
                       CV_RGB(255,255,255), CV_RGB(200,255,255), 
                       0, 1, 8, cvPoint(0,0));
   }

}

/*
**  Find cosine of angle between vectors
**  from pt1 and pt2
*/
static double cos_angle( float x1, float y1, float x2, float y2,
                     float x0, float y0 )
{
   float dx1 = x1 - x0;
   float dy1 = y1 - y0;
   float dx2 = x2 - x0;
   float dy2 = y2 - y0;
   return (dx1*dx2 + dy1*dy2)/
            sqrtf((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

/*
**  Process raw target detections and compare with current 
**  tracked targets.  Also mitigate target blinking caused by 
**  rapid movement or temporary obscuration
**
**  Uses global variables:
**  int           num_detect_targets;
**  target_struct detected_targets[10];
**  int           num_tracked_targets;
**  target_struct tracked_targets[10];
**
**  Plan:
**   1) filter out duplicate targets
**   2) match to current targets
**   3) if no match, determine permance
*/
void track_targets_over_time ( int frame_cnt )
{
  int i, j;
  float dx, dy, maxd;
  int duplicate[MAX_TRACKED_TARGETS];
  int found;
  int time_tracked;
   CvPoint pt1;
   char  screentext[80];
 
#ifdef DO_PRED
  float pred_x, pred_y;
#endif

  dx = dy = 0.0;

  /*
  **  zero out duplicate flags for each target
  */
  for ( i = 0; i < num_detect_targets; i++ )
   duplicate[i] = 0;

  /*
  **  Find and label all the duplicate targets
  **   if we have a match, duplicate[i] = 1;
  */
  for ( i = 0; i < num_detect_targets; i++ )
  {
     for ( j = i+1; j < num_detect_targets; j++ )
     {
        dx = fabsf(detected_targets[i].xcenter - 
                  detected_targets[j].xcenter);
        dy = fabsf(detected_targets[i].ycenter - 
                  detected_targets[j].ycenter);
        maxd = MAX(dx, dy);

        if ( maxd < tracking.targ_match_dist ) {
           duplicate[j] = 1;
        }
     }
  }

  /*
  **  See if we can match to a current target
  */
  if ( num_tracked_targets == 0 )
  {
     /*  
     **  if tracked_targets = 0, then we have no targets tracked, 
     **     no comparisions needed, so just remove duplicates 
     */
     for ( i = 0; i < num_detect_targets; i++ )
     {
        if ( !duplicate[i] ) 
        {
           targ_info_copy( &detected_targets[i], 
                      &tracked_targets[num_tracked_targets]);
           tracked_targets[num_tracked_targets].frame_tracked = frame_cnt;
           tracked_targets[num_tracked_targets].frame_missing = frame_cnt;
           tracked_targets[num_tracked_targets].dx = 0.0;
           tracked_targets[num_tracked_targets].dy = 0.0;

           num_tracked_targets++;
        }
     }
  }
  else  /* we have tracked targets, identify if these are new or old */
  {
     /*
     **  in this case we have existing targets tracked, so
     **    we need to compare the new detected targets with the tracked
     **    targets and remove the duplicates or add new targets.
     **  Also, check for a tracked target with no current new detection
     **    and see how long it has been since it was tracked.
     */
     for ( i = 0; i < num_detect_targets; i++ )
     {
        if ( !duplicate[i] ) 
        {
           found = 0;
           for ( j = 0; j < num_tracked_targets; j++ )
           {
              dx = detected_targets[i].xcenter - 
                        tracked_targets[j].xcenter;
              dy = detected_targets[i].ycenter - 
                        tracked_targets[j].ycenter;
              maxd = MAX(fabsf(dx), fabsf(dy));
              if ( (maxd < (tracking.targ_match_dist * 2)) && !found ) 
              {
                 /* 
                 ** found a match: update target info 
                 */
                 found = 1;

                 time_tracked = tracked_targets[j].time_tracked;

//               printf("found match dt %d tt %d %f\n", i, j, dx);

                 targ_info_copy( &detected_targets[i], 
                            &tracked_targets[j]);
                 tracked_targets[j].frame_tracked = frame_cnt;
                 tracked_targets[j].frame_missing = frame_cnt;
                 tracked_targets[j].dx = dx;
                 tracked_targets[j].dy = dy;
                 tracked_targets[j].time_tracked = time_tracked + 1;

                 pt1.x = tracked_targets[j].xcenter + 20;
                 pt1.y = tracked_targets[j].ycenter;
            sprintf(screentext,"%d pt", tracked_targets[j].type);
            cvPutText( image, screentext, pt1, &font, CV_RGB(0,255,0));
            sprintf(screentext,"(%.0f ft.)", tracked_targets[j].distance);
                 pt1.x = tracked_targets[j].xcenter + 20;
                 pt1.y = tracked_targets[j].ycenter + 20;
            cvPutText( image, screentext, pt1, &font, CV_RGB(255,255,255));

                 j = MAX_TRACKED_TARGETS + 1;
              }
           }
           if ( !found )  /* no match, must be a new target */
           {
//              printf("no match: %d (%d)\n", i, detected_targets[i].type);
              targ_info_copy( &detected_targets[i], 
                            &tracked_targets[num_tracked_targets]);
              tracked_targets[num_tracked_targets].frame_tracked = frame_cnt;
              tracked_targets[num_tracked_targets].frame_missing = frame_cnt;
              tracked_targets[num_tracked_targets].dx = 0.0;
              tracked_targets[num_tracked_targets].dy = 0.0;
              tracked_targets[num_tracked_targets].time_tracked = 1;

              num_tracked_targets++;
           }
        }
    }

  }

  /*
  **  Check on tracked targets that weren't identified during
  **  this current image frame from camera
  */

  for ( i = 0; i < num_tracked_targets; i++ )
  {
     if ( tracked_targets[i].frame_tracked != frame_cnt ) 
     {

        /*  
        ** see if we need to delete a lost target if
        ** not detected after TARG_PERSIST frames
        **  
        */
        if ( (frame_cnt - tracked_targets[i].frame_tracked) > tracking.targ_persist ) 
        {
           printf("Lost target: %d (%.1f %.1f) ratio: %.1f %.1f\n",
              tracked_targets[i].type, tracked_targets[i].xcenter, 
              tracked_targets[i].ycenter,
              tracked_targets[i].h_length / tracked_targets[i].v_length,
              tracked_targets[i].aspect_ratio );
           /* 
           **  shift targets up in the list
           */
           for ( j = i; j < (num_tracked_targets - 1); j++ )
           {
              targ_info_copy( &tracked_targets[j+1], 
                             &tracked_targets[j]);
           }
           num_tracked_targets--;
        }
        /*
        **  Else, don't delete it yet, but guess
        **  where it should be using previous dx, dy.
        */
        else {
           tracked_targets[i].time_tracked++;

                 pt1.x = tracked_targets[i].xcenter + 20;
                 pt1.y = tracked_targets[i].ycenter;
            sprintf(screentext,"%d pt", tracked_targets[i].type);
            cvPutText( image, screentext, pt1, &font, CV_RGB(0,255,0));
            sprintf(screentext,"(%.0f ft.)", tracked_targets[i].distance);
                 pt1.x = tracked_targets[i].xcenter + 20;
                 pt1.y = tracked_targets[i].ycenter + 20;
            cvPutText( image, screentext, pt1, &font, CV_RGB(255,255,255));

#ifdef DO_PRED
           pred_x = tracked_targets[i].xcenter + tracked_targets[i].dx; 
           pred_y = tracked_targets[i].ycenter + tracked_targets[i].dy; 
           if ( (pred_x > camera_info.h_pixels) || (pred_y > camera_info.v_pixels) 
                || (pred_x < 0) || (pred_y < 0 ) )
           {
              /*  target predicted off image */
              for ( j = i; j < (num_tracked_targets - 1); j++ )
              {
                 targ_info_copy( &tracked_targets[j+1], 
                                &tracked_targets[j]);
              }
              num_tracked_targets--;
           }
           else 
           {
              tracked_targets[i].xcenter =
                 tracked_targets[i].xcenter + tracked_targets[i].dx; 
              tracked_targets[i].ycenter =
                 tracked_targets[i].ycenter + tracked_targets[i].dy; 
           }
#endif  /* DO_PRED */
        }
     }
  }

return;
}

/* ========================================= */

/*
**  Copy target info from one position to another
*/

void targ_info_copy( target_struct *src, target_struct *dest )
{
  int j;

  dest->i = src->i; 

  dest->type = src->type; 
  for ( j = 0; j < 4; j++) {
    dest->xpt[j] = src->xpt[j];
    dest->ypt[j] = src->ypt[j];
  }

  dest->xcenter = src->xcenter; 
  dest->ycenter = src->ycenter; 
  dest->xmin = src->xmin; 
  dest->xmax = src->xmax; 
  dest->ymin = src->ymin; 
  dest->ymax = src->ymax; 

  dest->h_length = src->h_length; 
  dest->v_length = src->v_length; 

  dest->h_len_deg = src->h_len_deg; 
  dest->v_len_deg = src->v_len_deg; 

  dest->aspect_ratio = src->aspect_ratio; 

  dest->h_angle = src->h_angle; 
  dest->v_angle = src->v_angle; 
  dest->area = src->area; 
  dest->distance = src->distance; 
  dest->time_tracked = src->time_tracked; 

  dest->dx =  src->dx; 
  dest->dy = src->dy; 
}

/*
**   Draw target crosshair on image
*/
void draw_target_center( target_struct target, IplImage *draw_image, 
                         CvScalar cross_color )
{
   int      cross_thickness;
   CvPoint pt1, pt2;

   cross_thickness = 2;

      pt1.x = target.xcenter+8; 
      pt1.y = target.ycenter;
      pt2.x = target.xcenter-8; 
      pt2.y = target.ycenter;

      cvLine( draw_image, pt1, pt2, cross_color,
              cross_thickness, 8, 0);

      pt1.x = target.xcenter; 
      pt1.y = target.ycenter+8;
      pt2.x = target.xcenter; 
      pt2.y = target.ycenter-8;
      cvLine( draw_image, pt1, pt2, cross_color,
              cross_thickness, 8, 0);

}

void draw_target_dot( target_struct target, IplImage *draw_image, 
                         CvScalar circle_color )
{
   CvPoint pt1;

   pt1.x = target.xcenter; 
   pt1.y = target.ycenter;

   cvCircle( draw_image, pt1, 15, circle_color, 2 , 8, 0);

}
