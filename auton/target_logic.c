
/*
**  Target tracking logic code
*/

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/legacy/legacy.hpp"
#include <stdio.h>
#include <math.h>

#include "camera_info.h"
#include "target_externs.h"

#define TRUE 1
#define FALSE 0

#define HORIZONTAL 1
#define VERTICAL 0

/*
**  External global camera parameters
*/
extern camera_struct camera_info;
extern tracking_struct tracking;
extern IplImage *image;
extern lut_struct lut_3pt;
extern lut_struct lut_2pt;

extern int num_tracked_targets;
extern target_struct tracked_targets[MAX_TRACKED_TARGETS];

/*
**  Local function prototypes
*/
int determine_hot_goal ( int );


/*
**  Determine if the shooting goal is HOT or NOT HOT
**    based on if both horizontal and vertical visual targets
**    are visible
**
**    if we only see the vertical target, then the goal is not HOT
**    if both horizontal and vertical targets are seen then the 
**       goal is hot
**
**    Important variables:
**      num_tracked_targets  (integer, number of tracked targets)
**
**      tracked_targets[i].*    see target_info.h for full description

**      tracked_targets[i].xcenter
**      tracked_targets[i].ycenter  (float) x,y center of the target)
*/
int determine_hot_goal ( frame_cnt )
{
   int HOT_GOAL = FALSE;   /* setup default to no hot goal (false) */

   int i,j;

   /*
   **  First check and see if we have more than one target,
   **   if not, then skip all processing and return to loop in 
   **   target_tracking.c
   */ 
   if ( num_tracked_targets < 2 )
   {
      HOT_GOAL = FALSE;
      printf("one goal!\n");
      return( HOT_GOAL );
   } 

   /*  debug print section to verify orientation */
   for (i = 0; i < num_tracked_targets; i++ )
   {
      printf("target %d: orientation: %d\n",
        i, tracked_targets[i].orientation );
   }

   /*
   ** We have two ways to determine if the goal is hot or not.
   ** 1) simply see if we have a horizontal and vertical 
   **     target at about the same distance
   ** 2) calculate the slope line between centers of the 
   **     horizontal and vertical target and see it matches our calculations
   */

   /*
   ** pseudo code for #1
   */
//   for (i = 0; i < num_tracked_targets; i++ )
//   {
//      for ( j = 0; j < num_tracked_targets; j++ )
//      {
//         if ( i != j ) 
//         {
//           IF target[i] orientation is not equal to target[j] THEN
//             HOT_GOAL = TRUE (and return)
//           else
//             HOT_GOAL = FALSE
//         }
//      }
//   }

   /*
   ** pseudo code for #2
   */
//   for (i = 0; i < num_tracked_targets; i++ )
//   {
//      for ( j = 0; j < num_tracked_targets; j++ )
//      {
//         if ( i != j ) 
//          {
//             CALCULATE SLOPE BETWEN target i and j
//              slope = (j.ycenter - i.ycenter) / (j.xcenter - i.xcenter);
//
//            IF slope is close to 0.88 (or -.88) THEN
//               HOT_GOAL = TRUE (and return)
//            else
//               HOT_GOAL = FALSE
//          }
//        }
//      }

   /* end function, return result */
   return( HOT_GOAL );
}
