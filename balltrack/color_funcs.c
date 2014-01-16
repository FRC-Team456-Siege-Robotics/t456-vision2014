
/*
**   Color functions
**
**   Team 456, Vicksburg, MS
**   www.seigerobotics.org
** 
**   These series of functions deal with color conversions 
**    and image thresholding
*/

#include "opencv2/legacy/legacy.hpp"


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

void T456_change_RGB_to_binary( IplImage *, CvMat *, int, int, int);
void T456_filter_image( unsigned char , unsigned char , unsigned char , 
                 unsigned char *, int, int, int);

/*
**  Filter RGB image, HSV convert, threshold, etc...
*/

void T456_change_RGB_to_binary( IplImage *rgb, CvMat *binary,
                                int val_thresh, int hue_mid_thresh, 
                                int hue_mid_span )
{
  register int y;
  register unsigned char r,g,b;
  register char *data;
  register uchar *bin_data;

  register int total_vals;

  /* 
  **  Point the data pointer into the beginning of the image data
  */
  data = (char*)rgb->imageData;

  /*
  **  Point the output binary image pointer to the beginning of the image
  */
  bin_data = (uchar*)binary->data.ptr;
  
  total_vals = rgb->height * rgb->width;

  for ( y = 0; y < total_vals; y++ )  /* rows */
  {
     /* grab the bgr values */
     b = data[0];
     g = data[1];
     r = data[2];
     data += 3;

     T456_filter_image( r,g,b, bin_data, val_thresh, 
                        hue_mid_thresh, hue_mid_span );

     /* increment output pointer */
     bin_data++;
  }

}


void T456_filter_image( unsigned char r, unsigned char g, unsigned char b, 
                 unsigned char *binary ,
                 int val_thresh, int hue_mid_thresh, int hue_mid_span)
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
   if ( (val == 0) || (val < val_thresh) ) {
      return;   /* binary = 0 */
   }

   /*
   **  Zero out white pixels 
   **   WARNING (use only if camera is not oversaturated)
   */
   if ( (val >= val_thresh) && (rgb_diff == 0 ) ) 
   {
      *binary = 0;
      return;
   }

   /*
   **  Pull out saturated white pixels 
   */
//   if ( (val >= val_thresh) && (rgb_diff <= 30 ) ) 
//   {
//      *binary = 0;
//      return;
//   }


   /* 
   ** Compute hue 
   */
   if (rgb_max == r) {
       hue = 0 + 43 * (g - b)/(rgb_diff);
   } else if (rgb_max == g) {
       hue = 85 + 43*(b - r)/(rgb_diff);
   } else /* rgb_max == b */ {
       hue = 171 + 43*(r - g)/(rgb_diff);
   }

   /* 
   **  to get to this point, val > val_thresh
   */
   if (    (hue >= ( hue_mid_thresh - hue_mid_span)) 
       && ( hue <= ( hue_mid_thresh + hue_mid_span) ) )
   {
       *binary = 255;
   }
 
   return;
}
