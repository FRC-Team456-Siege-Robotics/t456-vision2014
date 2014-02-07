
/*
**  Structure definition for camera information 
**   and camera dependent image processing parameters
*/
typedef struct {
  /*
  **  camera hardware specs
  */
  int   camera_id;    /* camera id */
  float h_fov;        /* camera horizontal field of view (degrees) */
  float v_fov;        /* camera vertical field of view (degrees) */
  int h_pixels;       /* camera horizontal image pixels */
  int v_pixels;       /* camera vertical image pixels */
  float h_ifov;       /* camera horizontal instaneous field of view (degrees) */
  float v_ifov;       /* camera vertical instaneous field of view (degrees) */

} camera_struct;

