
#define TRUE 1
#define FALSE 0

#define MAXTHREADS 4
#define MAX_TRACKED_TARGETS 10

/*
**  Structure definition for camera information 
**   and camera dependent image processing parameters
*/
typedef struct {
  /*
  **  camera hardware specs
  */
  int   camera_id;    /* /dev/video# of camera 0,1,2 (or -1 for any) */
  float h_fov;        /* camera horizontal field of view (degrees) */
  float v_fov;        /* camera vertical field of view (degrees) */
  int h_pixels;       /* camera horizontal image pixels */
  int v_pixels;       /* camera vertical image pixels */
  float h_ifov;       /* camera horizontal instaneous field of view (degrees) */
  float v_ifov;       /* camera vertical instaneous field of view (degrees) */

} camera_struct;

/*
**  Structure definition for computer processing information
*/
typedef struct {
  int nthreads;      /* number of processing threads, default = 2 */
  int graphics;      /* display graphics? 0 = no, 1 = yes (default = 0) */
  int timing_check;  /* check timing of frames? 0 = no, 1 = yes (default = 0) */
  int save_frames;   /* save image frames? 0 = no, 1 = yes (default = 0) */
  int wait_time;     /* wait time (ms) to check for key press */
} proc_struct;


typedef struct {
   int argc;
   char **argv;
} arg_struct;

typedef struct {
  int xcent, ycent;
  float radius;
  float dist;
  int   frame_tracked;
  int   frame_missing;
  int   time_tracked;
} target_struct;

typedef struct {
  int red_hue_mid_thresh, red_hue_mid_span;
  int blue_hue_mid_thresh, blue_hue_mid_span;
  int   red_val_thresh;
  int   blue_val_thresh;
} tracking_struct;
