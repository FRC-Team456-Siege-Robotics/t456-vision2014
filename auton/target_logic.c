
/*
**  FRC Team 456 Siege Robotics
**  Vicksburg, MS
**  2014 Competition Code
**
**  Target tracking logic code
*/

#include <math.h>

#include "target_externs.h"

/*
**  External global variables used:
**
**  From "target_externs.h"
**    num_tracked_targets - number of tracked targets in the image
**    tracked_targets[]   - tracked target information [0..num_tracked_targets]
**
*/

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

/*
** We have two ways to determine if the goal is hot or not.
** 1) simply see if we have a horizontal and vertical 
**     target at about the same distance
** 2) calculate the slope line between centers of the 
**     horizontal and vertical target and see it matches our calculations
**
** Code below implements method #1.
*/

#define TRUE 1
#define FALSE 0


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
      return( HOT_GOAL );
   } 

   /*  debug print section to verify orientation */
   for (i = 0; i < num_tracked_targets; i++ )
   { 
      for (j = 0; j < num_tracked_targets; j++)
      {
          if ( j != i) 
          {
              /*
              **  If targets have opposite orientation (vert and horz)
              **    or (horz and vert), then continue
              */
              if ( tracked_targets[i].orientation != 
                   tracked_targets[j].orientation) 
              {
                /*
                **  If they have opposite orientation and are roughly
                **   the same distance away (with some error) then
                **   consider them a "hot" target
                **
                **      24.0 = 24 inches
                */
                if ( fabsf(tracked_targets[i].distance -
                           tracked_targets[j].distance ) < 24.0 )
                 {
                    HOT_GOAL = TRUE;
                    return(HOT_GOAL);
                 }

              }  /* endif i.orientation != j.orientation */

          } /* endif j != i */

      } /* end for j = 0..num_tracked_targets */

   } /* end for i = 0..num_tracked_targets */
    
   /*
   ** pseudo code for method #2
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
//      }
//   }

   /* end function, return result */
   return( HOT_GOAL );
}
