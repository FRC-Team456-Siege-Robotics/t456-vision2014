
/*
**  Target tracking configuration parameters
*/

#define MAX_TRACKED_TARGETS 20      /*  maximum number of tracked targets */

/* 
**  Structure definition for detected targets
*/
typedef struct {
   int i;
   int type;   /* 5 = 5pt, 3 = 3 pt, 2 = 2 pt, 1 = 1 pt, 0 = unknown */
   int orientation;  /* 0 = vertical, 1 = horizontal */
   float xpt[4], ypt[4]; 
   float xcenter, ycenter;
   float aimx, aimy;
   float xmin, xmax, ymin, ymax;
   float h_length, v_length;
   float aspect_ratio;
   float h_angle, v_angle;    /* target position on the camera image */
   float aim_h_angle, aim_v_angle;    /* target position on the camera image */
   float h_len_deg, v_len_deg;
   float area;
   float distance;
   float dx, dy;
   int frame_tracked;
   int frame_missing;
   int time_tracked;
   float score;
} target_struct;

typedef struct {
  /*
  **  image processing specs
  */
  int hue_mid_thresh;             /* midpoint threshold value for hue image */ 
  int hue_mid_span;               /* span of hue threshold (+-) this number */
  int val_thresh;                 /* threshold for value image */
  int morph_closing_iterations;   /* morphological iterations */
  int min_rect_area;              /* minimum rectangle size (pixels^2) */
  int targ_match_dist;            /* target matching distance (pixels) */
  int targ_persist;               /* target persistence (number of frames) */
  float vert_targ_max_ratio;      /* vertical target maximum aspect ratio */
  float vert_targ_min_ratio;      /* vertical target minimum aspect ratio */
  float horz_targ_max_ratio;      /* horizontal target maximum aspect ratio */
  float horz_targ_min_ratio;      /* horizontal target minimum aspect ratio */
  int diag;                       /* tracking diagnostic level  0 = none */

} tracking_struct;

typedef struct {
  int numvals;
  float dist[20];
  float dist_delta[20];
  float offset[20];
  float offset_delta[20];
} lut_struct;
