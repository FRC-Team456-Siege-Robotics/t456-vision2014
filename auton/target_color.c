/*
**   Process video, find, and track rectangles for FRC 2013 competition
**   Team 456, Vicksburg, MS
**   www.seigerobotics.org
** 
**   These series of functions deal with color conversions 
**    and image thresholding
*/

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/legacy/legacy.hpp"
#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "target_externs.h"
#include "camera_info.h"

/*
**  External global tracking parameters
*/
extern tracking_struct tracking;

/*
**  Local MACRO defines
*/
#define MIN3(x,y,z)  ((y) <= (z) ? \
                         ((x) <= (y) ? (x) : (y)) \
                     : \
                         ((x) <= (z) ? (x) : (z)))

#define MAX3(x,y,z)  ((y) >= (z) ? \
                         ((x) >= (y) ? (x) : (y)) \
                     : \
                         ((x) >= (z) ? (x) : (z)))

/* 
** prototypes of functions
*/
void T456_change_RGB_to_HV( IplImage *, CvMat *, CvMat *);

void T456_rgb_to_hsv( unsigned char , unsigned char , unsigned char , 
                      unsigned char *, unsigned char * );

void T456_change_RGB_to_binary( IplImage *, CvMat *);
void T456_filter_image( unsigned char , unsigned char , unsigned char , 
                 unsigned char *);

/* 
**  functions 
*/
void T456_change_RGB_to_HV( IplImage *rgb, CvMat *hue, CvMat *val )
{
  register int y;
  register unsigned char r,g,b;
  register char *data;
  register uchar *hue_data;
  register uchar *val_data;

  register int total_vals;

  data = (char*)rgb->imageData;
  hue_data = (uchar*)hue->data.ptr;
  val_data = (uchar*)val->data.ptr;

  
  total_vals = rgb->height * rgb->width;

  for ( y = 0; y < total_vals; y++ )  /* rows */
  {
     b = data[0];
     g = data[1];
     r = data[2];
     data += 3;

     T456_rgb_to_hsv( r,g,b, hue_data, val_data );

     hue_data++;
     val_data++;

  }

}



void T456_rgb_to_hsv( unsigned char r, unsigned char g, unsigned char b, 
                 unsigned char *hue, unsigned char *val )
{
   unsigned char rgb_min, rgb_max, rgb_diff;
   unsigned char sat = 0; 

   rgb_min = MIN3( r, g, b );
   rgb_max = MAX3( r, g, b );

   rgb_diff = rgb_max - rgb_min;

   *val = rgb_max;

   if ( (rgb_diff == 0) || (*val == 0) ) {
     *hue = 0;
      return;
   }

   sat = 255 * (rgb_diff) / *val;
  
   if ( sat == 0 ) {
     *hue = 0;
     return;
   }

   /* Compute hue */
   if (rgb_max == r) {
       *hue = 0 + 43*(g - b)/(rgb_diff);
   } else if (rgb_max == g) {
       *hue = 85 + 43*(b - r)/(rgb_diff);
   } else /* rgb_max == b */ {
       *hue = 171 + 43*(r - g)/(rgb_diff);
   }
 
   return;
}


/*
**  Filter RGB image, HSV convert, threshold, etc...
*/

void T456_change_RGB_to_binary( IplImage *rgb, CvMat *binary)
{
  register int y;
  register unsigned char r,g,b;
  register char *data;
  register uchar *bin_data;

  register int total_vals;

  data = (char*)rgb->imageData;
  bin_data = (uchar*)binary->data.ptr;
  
  total_vals = rgb->height * rgb->width;

  for ( y = 0; y < total_vals; y++ )  /* rows */
  {
     b = data[0];
     g = data[1];
     r = data[2];
     data += 3;

     T456_filter_image( r,g,b, bin_data );

     bin_data++;

  }

}


void T456_filter_image( unsigned char r, unsigned char g, unsigned char b, 
                 unsigned char *binary )
{
   unsigned char rgb_min, rgb_max, rgb_diff;
   unsigned char hue = 0; 
   unsigned char val = 0; 

   /*
   **  set the default return value to zero 
   */
   *binary = 0;

   /*
   **  get the min and max values of the RGB
   **   pixel
   */
   rgb_min = MIN3( r, g, b );
   rgb_max = MAX3( r, g, b );

   rgb_diff = rgb_max - rgb_min;

   val = rgb_max;


   /* 
   **  This is the trivial case:
   **    zero pixels or value is less than VAL_THRESH
   */
   if ( (val == 0) || (val < tracking.val_thresh) ) {
      return;   /* binary = 0 */
   }

   /*
   **  Zero out white pixels 
   **   WARNING (use only if camera is not oversaturated)
   */
   if ( (val >= tracking.val_thresh) && (rgb_diff == 0 ) ) 
   {
      *binary = 0;
      return;
   }

   /*
   **  Pull out saturated white pixels 
   */
//   if ( (val >= tracking.val_thresh) && (rgb_diff <= 30 ) ) 
//   {
//      *binary = 0;
//      return;
//   }



   /* calculate sat 
   **  rgb_diff test should help here
   */
   //   sat = 255 * (rgb_diff) / val;
  

   /* 
   ** Compute hue 
   */
   if (rgb_max == r) {
       hue = 0 + 43 * (g - b)/(rgb_diff);
       // hue = 0;
   } else if (rgb_max == g) {
       hue = 85 + 43*(b - r)/(rgb_diff);
   } else /* rgb_max == b */ {
       hue = 171 + 43*(r - g)/(rgb_diff);
   }

// DEBUG 
//   if (  (hue > 20) && (hue < 60) ) return;
//

//*binary = 255;
//return;

   /* 
   **  to get to this point, val > tracking.val_thresh
   */
   if (    (hue >= ( tracking.hue_mid_thresh - tracking.hue_mid_span)) 
       && ( hue <= (tracking.hue_mid_thresh + tracking.hue_mid_span) ) )
   {
       *binary = 255;
   }
 
   return;
}
