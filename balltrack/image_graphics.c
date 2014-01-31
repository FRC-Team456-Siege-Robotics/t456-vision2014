/*
**  FRC Team 456 Siege Robotics
**  Vicksburg, MS
**  2014 Competition Code
**
*/
#include "opencv2/legacy/legacy.hpp"

/*
**  Define function prototype
*/
void draw_target_center( CvPoint , IplImage *, CvScalar );


/*
**   Draw target crosshair on image
*/
void draw_target_center( CvPoint target, IplImage *draw_image,
                         CvScalar cross_color )
{
   int      cross_thickness;
   CvPoint pt1, pt2;
   register int cross_half_width = 8;

   cross_thickness = 2;

      pt1.x = target.x + cross_half_width;
      pt1.y = target.y;
      pt2.x = target.x - cross_half_width;
      pt2.y = target.y;

      cvLine( draw_image, pt1, pt2, cross_color,
              cross_thickness, 8, 0);

      pt1.x = target.x;
      pt1.y = target.y + cross_half_width;
      pt2.x = target.x;
      pt2.y = target.y - cross_half_width;
      cvLine( draw_image, pt1, pt2, cross_color,
              cross_thickness, 8, 0);

}

