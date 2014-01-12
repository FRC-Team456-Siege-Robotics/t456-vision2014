
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/legacy/legacy.hpp"
#include <stdio.h>
#include <math.h>

#include "camera_info.h"
#include "target_externs.h"

/*
**  External global camera parameters
*/
extern camera_struct camera_info;
extern tracking_struct tracking;
extern IplImage *image;
extern lut_struct lut_3pt;
extern lut_struct lut_2pt;

/*
**  Local function prototypes
*/
int T456_select_main_target ( int );
float T456_target_scoring ( int );
void T456_calculate_aimpoint( int );


int T456_select_main_target ( int frame_cnt )
{
  int i;
  float score, max_score;
  int max_indx = 0;

  /*
  **  Update target score
  */

  max_score = -1.0;

  printf("scores: ");

  for ( i = 0; i < num_tracked_targets; i++ )
  {
     score = T456_target_scoring(i); 

     tracked_targets[i].score = score;
    
     if ( score > max_score ) {
         max_indx = i;
         max_score = score;
     }

     printf("(%d %.2f) ", tracked_targets[i].type, score);
  }
  printf("\n");

#ifdef DIAG
  printf("selected target: %d (%.2f)\n", 
            tracked_targets[max_indx].type, 
            tracked_targets[max_indx].score);
#endif /* DIAG */
  
  return(max_indx);
}

/*
**   Defines used for target scoring
*/
#define TS_TYPE 0.3333

#define TS_TYPE_WGT    0.18
#define TS_HPOS_WGT    0.50
#define TS_VPOS_WGT    0.32

#define TS_HDEG_SZ_WGT 0.1
#define TS_VPOS_SZ_WGT 0.1

/*
**    Score each tracked target by a weighted
**     score based on target type and aim location
*/

float T456_target_scoring ( int i )
{
   float h_angle_wt, v_angle_wt;
   float type_wt;
   float score;

   /*
   **  if the center of aim is inside a target, then
   **   that is the target selected regardless of other 
   **   factors.
   */
//   printf("h_angle: %f h_len_deg %f\n", tracked_targets[i].h_angle, 
//                                       tracked_targets[i].h_len_deg);
//   printf("v_angle: %f v_len_deg %f\n", tracked_targets[i].v_angle, 
//                                       tracked_targets[i].v_len_deg);

   if ( (fabsf(tracked_targets[i].h_angle) < (tracked_targets[i].h_len_deg/2.0)) &&
        (fabsf(tracked_targets[i].v_angle) < (tracked_targets[i].v_len_deg)/2.0) )
   { 
      return(1.0);
   }

   /*  h_angle  smaller is better */
   h_angle_wt = fabsf(tracked_targets[i].h_angle / (camera_info.h_fov / 2.0f));
   h_angle_wt = fabsf( 1.0 - h_angle_wt );

   /*  v_angle  smaller is better */
   v_angle_wt = fabsf(tracked_targets[i].v_angle / (camera_info.v_fov / 2.0));
   v_angle_wt = fabsf( 1.0 - v_angle_wt );

   /*  target type  larger is better */
   type_wt = fabsf( (float) (tracked_targets[i].type)  * TS_TYPE );

   score =   TS_HPOS_WGT * h_angle_wt 
           + TS_VPOS_WGT * v_angle_wt
           + TS_TYPE_WGT * type_wt;

   return( score );
}

/*
**  Select aim point of the target based on type and distance
*/
void T456_calculate_aimpoint( int frame_cnt )
{
  int i,j;
  
  float diff, intdiff;
  register float maxdist, maxoffset;
  register  float targdist;
  CvPoint pt1;

  pt1.x = camera_info.h_pixels / 2;
  pt1.y = camera_info.v_pixels / 2;
  cvCircle( image, pt1, 20, CV_RGB(0,0,255), 2 , 8, 0);

  for ( i = 0; i < num_tracked_targets; i++ )
  {
     pt1.x = tracked_targets[i].xcenter;
     pt1.y = tracked_targets[i].ycenter;

     targdist = tracked_targets[i].distance;

     /*
     **  Calculate aim point for a 2 point target
     */
     if ( tracked_targets[i].type == 2 ) 
     {
        maxdist = lut_2pt.dist[lut_2pt.numvals-1];
        maxoffset = lut_2pt.offset[lut_2pt.numvals-1];

        if ( targdist >= maxdist )
        {
           /*  
           ** if target range is beyod the table, set it to max offset
           */
           pt1.y =  tracked_targets[i].ycenter +
                       maxoffset / camera_info.v_ifov;
        }
        else
        {
           /*
           ** target range is somewhere in between, find
           ** closest interval in lookup table and calculate offset
           */
           for ( j = 0; j < (lut_2pt.numvals-1); j++ ) 
           {
              if ( ( targdist >= lut_2pt.dist[j]) 
                && ( targdist <= lut_2pt.dist[j+1]) )
              {
                 /*  calculate scaled difference between data points */
                 intdiff = targdist - lut_2pt.dist[j];
         
                 /*  scaled difference (0-1) */
                 diff = intdiff / lut_2pt.dist_delta[j];

                 pt1.y =  tracked_targets[i].ycenter +
                           (lut_2pt.offset[j] 
                              + lut_2pt.offset_delta[j] * diff)
                           / camera_info.v_ifov;

              }
           }
        }
     }

     /*
     **  Calculate aim point for a 3 point target
     */
     if ( tracked_targets[i].type == 3 ) 
     {
        maxdist = lut_3pt.dist[lut_3pt.numvals-1];
        maxoffset = lut_3pt.offset[lut_3pt.numvals-1];

        if ( targdist >= maxdist )
        {
           /*  
           ** if target range is beyod the table, set it to max offset
           */
           pt1.y =  tracked_targets[i].ycenter +
                       maxoffset / camera_info.v_ifov;
        }
        else
        {
           /*
           ** target range is somewhere in between, find
           ** closest interval in lookup table and calculate offset
           */
           for ( j = 0; j < (lut_3pt.numvals-1); j++ ) 
           {
              if ( ( targdist >= lut_3pt.dist[j]) 
                && ( targdist <= lut_3pt.dist[j+1]) )
              {
                 /*  calculate scaled difference between data points */
                 intdiff = targdist - lut_3pt.dist[j];
         
                 /*  scaled difference (0-1) */
                 diff = intdiff / lut_3pt.dist_delta[j];

                 pt1.y =  tracked_targets[i].ycenter +
                           (lut_3pt.offset[j] 
                              + lut_3pt.offset_delta[j] * diff)
                           / camera_info.v_ifov;

              }
           }
        }
     }

     /*
     **  Add horizontal bias for shooter
     */
     pt1.x = tracked_targets[i].xcenter + (tracking.h_ang_correction/camera_info.h_ifov);

     /*
     **  Assign aim points into tracked target list
     */
     tracked_targets[i].aimx = (float) pt1.x;
     tracked_targets[i].aimy = (float) pt1.y;

     tracked_targets[i].aim_h_angle = tracked_targets[i].h_angle 
                                      + tracking.h_ang_correction;
     tracked_targets[i].aim_v_angle = 
         ((float) pt1.y - camera_info.v_pixels/2.0) * camera_info.v_ifov * -1;

     /*
     **  Draw a red dot where to aim
     */
#ifdef GRAPHICS
     cvCircle( image, pt1, 4, CV_RGB(255,0,0), -2 , 8, 0);
#endif

  }
}
