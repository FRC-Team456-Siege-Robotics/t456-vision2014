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

#define TRUE 1
#define FALSE 0

#include "camera_info.h"
#include "target_info.h"

/*
**  GLOBAL variables
*/
int           num_detect_targets;
target_struct detected_targets[MAX_TRACKED_TARGETS];
int           num_tracked_targets;
target_struct tracked_targets[MAX_TRACKED_TARGETS];
double        fps_sum = 0.0;
int           frame_cnt = 0;
IplImage      *image = 0;

CvFont        font;

double        ratio_sum = 0.0;
int           ratio_cnt = 0;

/*
**  Function prototypes
*/
void done();
CvSeq *find_contours( CvMat *);
void Detect_Targets( CvSeq *, CvMat *);
static double cos_angle( float , float , float , float, float, float );
void targ_info_copy( target_struct *, target_struct *);

void track_targets_over_time( int );
void draw_target_center( target_struct , IplImage *, CvScalar );
void draw_target_dot( target_struct , IplImage *, CvScalar );

/*
**  External function prototypes
*/
int T456_select_main_target( int );   /* located in target_logic.c */

/*
** ================ BEGIN MAIN SECTION =======================
*/
int main(int argc, char** argv){

    double t1, t2, fps;
    int  targ_selected = 0;

    CvCapture* camera = 0;
    CvMat *image_gray = 0;
    CvMat *image_hsv = 0;
    CvMat *image_hue,*image_sat, *image_val;
    CvMat *image_thresh_hue1, *image_thresh_hue2 = 0;
    CvMat *image_thresh_val;
    CvMat *image_binary;
    CvMat *image_morph_rect;
    IplImage* canny_image = 0;

    IplConvKernel *morph_kernel;

    CvSeq *contours;

    double result;
    int from_to[] = {3,3};

    int i;
    
    num_tracked_targets = 0;

    /*  
    **  Capture images from webcam /dev/video0 
    **   /dev/video0 = 0
    **   /dev/video1 = 1
    */
//    camera=cvCaptureFromCAM( 0 );

    /* 
    **  Capture image from file at command line
    **   useful for playback video and testing
    */
    camera = cvCaptureFromFile( argv[1] );


    /*
    **   Check and see if camera/file capture is valid
    */
    if (!camera) {
        printf("camera or image is null\n");
        exit(-1);
    }

    /*
    **  Write tracking data to a file
    */
#ifdef FILE_OUTPUT
    FILE *fp;
    fp = fopen("targ_loc.txt", "w+");
#endif /* FILE_OUTPUT */

    /*
    **  Setup graphic display windows using OpenCV
    */
#ifdef GRAPHICS
    cvNamedWindow("original", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("processed", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("binary", CV_WINDOW_AUTOSIZE);
#endif /* GRAPHICS */

    cvInitFont( &font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0.0, 1, 8);

    /*
    **  Construct a morphological kernel
    */
    morph_kernel = cvCreateStructuringElementEx(3, 3, 1, 1, 
                                                CV_SHAPE_RECT, NULL);
    /*
    **  Time estimation variable
    */
    t1 = (double)cvGetTickCount();

    /*
    **   Process images until key press
    */
#ifdef GRAPHICS
    while (cvWaitKey(2) < 0)
#else
    while (1)
#endif
    {

       /*
       **  Grab initial frame from image or movie file
       */
       image = cvQueryFrame(camera);
       if ( !image ) {
          done();
          exit(-1);
       }
 
       /*  reset detected targets */
       num_detect_targets = 0;

       if (!image_gray )
       {
        image_gray = cvCreateMat(image->height, image->width, CV_8UC1);
        image_hsv = cvCreateMat(image->height, image->width, CV_8UC3);
        image_hue = cvCreateMat(image->height, image->width, CV_8UC1);
        image_sat = cvCreateMat(image->height, image->width, CV_8UC1);
        image_val = cvCreateMat(image->height, image->width, CV_8UC1);
        image_thresh_hue1 = cvCreateMat(image->height, image->width, CV_8UC1);
        image_thresh_hue2 = cvCreateMat(image->height, image->width, CV_8UC1);
        image_thresh_val = cvCreateMat(image->height, image->width, CV_8UC1);
        image_binary = cvCreateMat(image->height, image->width, CV_8UC1);
        image_morph_rect = cvCreateMat(image->height, image->width, CV_8UC1);
       }


        cvCvtColor( image, image_hsv, CV_BGR2HSV ); 
        
        cvSplit( image_hsv, image_hue, image_sat, image_val, NULL);

        result = cvThreshold( image_hue, image_thresh_hue1, 
                              HUE_MID_THRESH - 15, 255, CV_THRESH_BINARY);
        result = cvThreshold( image_hue, image_thresh_hue2, 
                              HUE_MID_THRESH + 15, 255, CV_THRESH_BINARY_INV);

        /*  
        ** Threshold value image 
        **  if pixel value of the image is > VAL_THRESH
        **  then output 255
        **  else output 0
        */
        result = cvThreshold( image_val, image_thresh_val, 
                               VAL_THRESH, 255, CV_THRESH_BINARY);

        /*
        **  Combine hue1, hue2, and value images with an AND operation
        **   output = hue1 AND hue1 AND value
        **  if a pixel in any of these images is pixel = 0 
        **      then the output is 0
        */
        cvAnd( image_thresh_hue1, image_thresh_hue2, image_binary, NULL);
        cvAnd( image_thresh_val, image_binary, image_binary, NULL);
        
#ifdef DO_CANNY
        /*
        **  Use the Canny filter to determine edges and features
        **   in our case, doesn't work as well as Morphology
        **   and increases processing time
        */
        cvCanny( image_binary, image_sat, 10, 10*3, 3);
        cvCopy( image_sat, image_gray, NULL);
#endif /* DO_CANNY */

#ifdef DO_MORPH
        /* 
        **   Use the Morph function to join broken rectangles 
        */
        cvMorphologyEx( image_binary, image_morph_rect, NULL,
                         morph_kernel, CV_MOP_CLOSE, MORPH_CLOSING_ITERATIONS);
        cvCopy( image_morph_rect, image_gray, NULL);
#endif /* DO_MORPH */

        /*
        **  Process contours and detect targets of interest
        */
        contours = find_contours( image_gray );

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
           targ_selected = T456_select_main_target( frame_cnt );
           draw_target_dot( tracked_targets[targ_selected], 
                                 image, CV_RGB(0,255,0) );
        }

        printf("%d ", frame_cnt);

        for ( i = 0; i < num_tracked_targets; i++ ) 
        {
             printf("%d ", tracked_targets[i].type);
             printf("%.2f %.2f ", tracked_targets[i].h_angle, 
                                  tracked_targets[i].v_angle);
             printf("%.2f %.2f ", tracked_targets[i].xcenter, 
                                  tracked_targets[i].ycenter);
//             printf("%.2f %.2f ", tracked_targets[i].h_length, 
//                                  tracked_targets[i].v_length);
//             printf("%.2f %.2f ", tracked_targets[i].h_len_deg, 
//                                  tracked_targets[i].v_len_deg);
             printf("%d ", tracked_targets[i].time_tracked);

             draw_target_center( tracked_targets[i],
                                  image, CV_RGB(255,255,0) );
        }
        printf("\n");

#ifdef FILE_OUTPUT
        rewind(fp);
        fprintf(fp,"%05d,",frame_cnt);
        if ( num_tracked_targets == 0 ) {
           fprintf(fp,"00,000000,000000,0000,000000");
        } 
        else {
              fprintf(fp,"%02d,", tracked_targets[targ_selected].type);
              fprintf(fp,"%06.2f,%06.2f,3000,%06.1f", 
                               tracked_targets[targ_selected].h_angle, 
                               tracked_targets[targ_selected].v_angle,
                               tracked_targets[targ_selected].distance);
        }
        fflush(fp);
#endif /* FILE_OUTPUT */


#ifdef GRAPHICS
        /*
        **  Display images
        */
        cvShowImage("original",image);
        cvShowImage("processed",image_morph_rect);
        cvShowImage("binary",image_gray);
#endif /* GRAPHICS */

        /*  keep time of processing */
        t2 = (double)cvGetTickCount();
        fps = 1000.0 / ((t2-t1)/(cvGetTickFrequency()*1000.));
        fps_sum = fps_sum + fps;
        frame_cnt++;

#ifdef DIAG
        printf("time: %gms  fps: %.2g\n",(t2-t1)/(cvGetTickFrequency()*1000.),fps);
#endif

        t1 = t2;
    }

    cvReleaseCapture(&camera);

    done();

#ifdef FILE_OUTPUT
    fclose(fp);
#endif /* FILE_OUTPUT */
}

/*
** ==================== END MAIN SECTION =======================
*/

void done()
{
  printf("total frames: %d\n", frame_cnt);
  printf("average fps: %lf\n", fps_sum / (double)frame_cnt);
  printf("average aspect_ratio: %lf\n", ratio_sum / ratio_cnt);
}


CvSeq *find_contours( CvMat *input_image )
{
   CvSeq *contours = 0;
   CvMemStorage *storage = NULL;
   CvSeq *seq = 0;

   int i;
   CvPoint *point;
   CvMat *input_image_copy;

   CvSeq *convex_contours = 0;
   CvSeq *polydp_contours = 0;
   CvMemStorage *hull_storage = NULL;
   float area;
   

   input_image_copy = 
          cvCreateMat(input_image->rows, input_image->cols, CV_8UC1);

   cvCopy( input_image, input_image_copy, NULL);

   cvZero(input_image);

   storage = cvCreateMemStorage(0);
   hull_storage = cvCreateMemStorage(0);

   cvFindContours( input_image_copy, storage, &contours , sizeof(CvContour),
                     CV_RETR_LIST, CV_CHAIN_APPROX_TC89_KCOS, cvPoint(0,0) );

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
      area = fabs(cvContourArea( convex_contours, CV_WHOLE_SEQ, 0));

      /*  
      **  Filter out only shapes that are target sized > MIN_RECT_AREA
      */
      if ( area > MIN_RECT_AREA ) 
      {
         /*
         **  Approximate polygonal shape
         */
         polydp_contours = cvApproxPoly(seq, sizeof(CvContour), storage, 
                                        CV_POLY_APPROX_DP, 4.0, 0);

//      cvDrawContours( input_image, polydp_contours, 
//                       CV_RGB(255,255,255), CV_RGB(200,255,255), 
//                       0, 1, 8, cvPoint(0,0));
         /*
         **  If the shape has four corners, check and see it is a target
         */
         if ( polydp_contours->total == 4 )
            Detect_Targets( polydp_contours, input_image );

      }
   }

   cvClearMemStorage(storage);
   cvClearMemStorage(hull_storage);

// new
   cvReleaseMemStorage(&storage);
   cvReleaseMemStorage(&hull_storage);

   cvReleaseMat( &input_image_copy);

   return(contours);
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

   CvPoint *point;
   CvPoint pt1, pt2;

   CvScalar cross_color;
   int      cross_thickness;
   int      isTarget = FALSE;

   xmax = ymax = 0.0;
   xmin = ymin = 1e20;

   float cosine1, cosine2, max_cosine;

   char  screentext[80];
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
      length_1 = sqrt( (xpt[0]-xpt[1])*(xpt[0]-xpt[1]) 
                      +(ypt[0]-ypt[1])*(ypt[0]-ypt[1]) );

      length_2 = sqrt( (xpt[1]-xpt[2])*(xpt[1]-xpt[2]) 
                      +(ypt[1]-ypt[2])*(ypt[1]-ypt[2]) );

      /*
      **  Calculate the angle of the first corner
      **   if it isn't 90 deg (+- 15 deg) then we don't have
      **   a rectangle and reject
      */
      cosine1 =  fabs( cos_angle( xpt[0], ypt[0],  
                                  xpt[2], ypt[2],
                                  xpt[1], ypt[1] ) );
      cosine2 =  fabs( cos_angle( xpt[1], ypt[1],  
                                  xpt[3], ypt[3],
                                  xpt[2], ypt[2] ) );
      max_cosine = MAX( cosine1, cosine2 );
      if ( max_cosine > 0.25 ) {
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
      if ( ( (aspect_ratio > 4.3) && (aspect_ratio < 4.7) ) ) 
      {
         /*
         **  Calculate distance to target
         **    the opening target 3 is 54"wide by 12" tall
         */
         distance = 0.01745 * ((MAX(length_1,length_2) / 640.0) * 48.8) / 2.0;
         distance = (54.0/2.0) / tanf(distance);

#ifdef DIAG
         printf("*** 3 pt target opening  ***\n");
         printf("3pt: distance: %f\n\n", distance/12.0);
#endif

         if ( (distance/12.0) < 65 ) /* field is 54 ft long */
         {
            isTarget = TRUE;
            detected_targets[num_detect_targets].type = 3;
            detected_targets[num_detect_targets].distance = distance/12.0;
            pt1.x = xcenter+20;  pt1.y = ycenter;
//            cvPutText( image, "3pt", pt1, &font, CV_RGB(0,255,0));
//            sprintf(screentext,"3pt dist: %.1f ft.", distance/12.0);
//            cvPutText( input_image, screentext, pt1, &font, 
//                           CV_RGB(255,255,255));
         }
      }

      /*  
      **  check for high goal outside boundary (3pt) target.
      **    62" by 20"
      **   aspect ratio = 3.1
      */
      if ( ((aspect_ratio > 3.0) && (aspect_ratio < 3.3)) ) 
      {
         /*
         **  Calculate distance to target
         **    the outside of target 3 is 62"wide by 20" tall
         */
         distance = 0.01745 * ((MAX(length_1,length_2) / 640.0) * 48.8) / 2.0;
         distance = (62.0/2.0) / tanf(distance);

#ifdef DIAG
         printf("*** 3 pt target outside  ***\n");
         printf("3pt outside: distance: %f\n\n", distance/12.0);
#endif
         
         if ( (distance/12.0) < 65 ) 
         {
            isTarget = TRUE;
            detected_targets[num_detect_targets].type = 3;
            detected_targets[num_detect_targets].distance = distance/12.0;
            pt1.x = xcenter+20;  pt1.y = ycenter+25;
//            cvPutText( image, "3pt border", pt1, &font, CV_RGB(0,255,0));
//            sprintf(screentext,"3pt border dist: %.1f ft.", distance/12.0);
//            cvPutText( input_image, screentext, pt1, &font, CV_RGB(255,255,255));
         }

      }

      /*  
      **  check for middle goal opening (2pt) target.
      **   54" x 21"
      **   aspect ratio = 2.6
      */
      if ( ( (aspect_ratio > 2.4) && (aspect_ratio < 2.8) ) ) 
      {
         /*
         **  Calculate distance to target
         **    the opening 2pt target is 54"wide by 21" tall
         */
         distance = 0.01745 * ((MAX(length_1,length_2) / 640.0) * 48.8) / 2.0;
         distance = (54.0/2.0) / tanf(distance);

//         printf("2pt opening: (%.1f, %.1f), ratio: %.3f\n", 
//                      xcenter, ycenter, aspect_ratio );

         if ( (distance/12.0) < 65 ) /* field is 54 ft long */
         {
            isTarget = TRUE;
            detected_targets[num_detect_targets].type = 2;
            detected_targets[num_detect_targets].distance = distance/12.0;
            pt1.x = xcenter;  pt1.y = ycenter;
//            cvPutText( image, "2pt", pt1, &font, CV_RGB(0,255,0));
//            sprintf(screentext,"2pt dist: %.1f ft.", distance/12.0);
//            cvPutText( input_image, screentext, pt1, &font, 
//                           CV_RGB(255,255,255));
         }
      }

      /*  
      **  check for middle goal border (2pt) target.
      **    62" x 29"
      **   aspect ratio = 2.14
      **    2.27, 2.3, 2.29
      */
      if ( ( (aspect_ratio > 1.8) && (aspect_ratio < 2.4) )  ) 
      {
         /*
         **  Calculate distance to target
         **    the outside of 2pt target is 62" wide by 29" tall
         */
         distance = 0.01745 * ((MAX(length_1,length_2) / 640.0) * 48.8) / 2.0;
         distance = (62.0/2.0) / tanf(distance);

         if ( (distance/12.0) < 65 ) 
         {
            isTarget = TRUE;
            detected_targets[num_detect_targets].type = 2;
            detected_targets[num_detect_targets].distance = distance/12.0;
            pt1.x = xcenter+25;  pt1.y = ycenter+25;
//            cvPutText( image, "2pt border", pt1, &font, CV_RGB(0,255,0));
//            sprintf(screentext,"2pt border dist: %.1f ft.", distance/12.0);
//            cvPutText( input_image, screentext, 
//                          pt1, &font, CV_RGB(255,255,255));
         }
      }

      /*
      **  See if we determined that one of the previous conditions
      **   were met, if so (isTarget = TRUE) then save all the 
      **   target information
      */
      if ( isTarget ) 
      {
         ratio_cnt++;
         ratio_sum += aspect_ratio;

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
             fabs(cvContourArea( raw_contours, CV_WHOLE_SEQ, 0));
         
         detected_targets[num_detect_targets].h_angle =
              (xcenter - CAM_HPIXELS/2.0) * CAM_H_IFOV;
         detected_targets[num_detect_targets].v_angle =
              (ycenter - CAM_VPIXELS/2.0) * CAM_V_IFOV * -1;
         
         /*
         **  calculate angular size 
         */
         detected_targets[num_detect_targets].h_len_deg = 
            detected_targets[num_detect_targets].h_length * CAM_H_IFOV;
         detected_targets[num_detect_targets].v_len_deg = 
            detected_targets[num_detect_targets].v_length * CAM_V_IFOV;
           
         
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
            sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
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
  float pred_x, pred_y;
  int duplicate[MAX_TRACKED_TARGETS];
  int found;
  int time_tracked;
 
  dx = dy = 0.0;

  for ( i = 0; i < num_detect_targets; i++ )
   duplicate[i] = 0;

  /*
  **  Find and label all the duplicate targets
  */
  for ( i = 0; i < num_detect_targets; i++ )
  {
     for ( j = i+1; j < num_detect_targets; j++ )
     {
        dx = fabs(detected_targets[i].xcenter - 
                  detected_targets[j].xcenter);
        dy = fabs(detected_targets[i].ycenter - 
                  detected_targets[j].ycenter);
        maxd = MAX(dx, dy);

        if ( maxd < TARG_MATCH_DIST ) {
           duplicate[j] = 1;
        }
     }
  }

  /*
  **  See if we can match to a current target
  */
  if ( num_tracked_targets == 0 )
  {
     /*  no targets tracked, so filter out duplicates */
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
              maxd = MAX(fabs(dx), fabs(dy));
              if ( (maxd < (TARG_MATCH_DIST * 2)) && !found ) 
              {
                 /* found: updated target info */
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
//  if ( num_tracked_targets > 1 ) {
//     cvWaitKey(500);
//  }


  /*
  **  Check on tracked targets that weren't identified during
  **  this frame
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
        if ( (frame_cnt - tracked_targets[i].frame_tracked) > TARG_PERSIST ) 
        {
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
#ifdef DO_PRED
           pred_x = tracked_targets[i].xcenter + tracked_targets[i].dx; 
           pred_y = tracked_targets[i].ycenter + tracked_targets[i].dy; 
           if ( (pred_x > CAM_HPIXELS) || (pred_y > CAM_VPIXELS) 
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
   int      cross_thickness;
   CvPoint pt1;

   pt1.x = target.xcenter; 
   pt1.y = target.ycenter;


//   cvCircle( draw_image, pt1, 10, circle_color, CV_FILLED, 8, 0);
   cvCircle( draw_image, pt1, 15, circle_color, 2 , 8, 0);

}
