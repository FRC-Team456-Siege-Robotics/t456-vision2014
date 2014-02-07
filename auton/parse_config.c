
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
**  Local include files
*/
#include "camera_info.h"
#include "target_externs.h"

/*
**  External include files
*/
#include "iniparser.h"

/*
**  Extern global variables
*/
#ifdef MAIN
   camera_struct  camera_info;
   tracking_struct tracking;
#else
   extern camera_struct  camera_info;
   extern tracking_struct tracking;
#endif

/*
**  local function prototypes
*/
void T456_set_camera_and_tracking_defaults();

/*
**  Parse input file for camera settings 
**   and image processing parameters
*/
void T456_parse_vision( char *input_config_file)
{
   dictionary *dict;

   fprintf(stderr, "Parsing input config file: %s\n", input_config_file);

   /*
   **  load and parse the input file
   */
   dict = iniparser_load( input_config_file );

   /*
   **  check and see if the file is valid
   */
   if ( dict == NULL ) 
   {
      fprintf(stderr, " ***WARNING*** Cannot parse or find configuration file\n");
      fprintf(stderr,
         " ***WARNING***    Using DEFAULT camera and image parameters\n");

      /*  
      ** set camera default settings 
      */
      T456_set_camera_and_tracking_defaults();
      return;
   }
   else   /* parse the input file */
   {
      iniparser_dump( dict, stderr); 

      /*
      **  GET CAMERA SETTINGS
      */
      camera_info.camera_id = iniparser_getint( dict, "camera:camera_id", 0);
      camera_info.h_fov = 
         (float) iniparser_getdouble( dict, "camera:h_fov", 48.8);
      camera_info.v_fov = 
         (float) iniparser_getdouble( dict, "camera:v_fov", 37.648);
      camera_info.h_pixels = iniparser_getint( dict, "camera:h_pixels", 640);
      camera_info.v_pixels = iniparser_getint( dict, "camera:v_pixels", 480);
      camera_info.h_ifov = 
         (float) iniparser_getdouble( dict, "camera:h_ifov", 0.07625);
      camera_info.v_ifov = 
         (float) iniparser_getdouble( dict, "camera:v_ifov", 0.07843);

      /*
      **  GET TARGET TRACKING SETTINGS
      */
      tracking.hue_mid_thresh = 
         iniparser_getint( dict, "tracking:hue_mid_thresh", 122);
      tracking.hue_mid_span = 
         iniparser_getint( dict, "tracking:hue_mid_span", 30);
      tracking.val_thresh = 
         iniparser_getint( dict, "tracking:val_thresh", 230);
      tracking.morph_closing_iterations = 
         iniparser_getint( dict, "tracking:morph_closing_iterations", 1);
      tracking.min_rect_area = 
         iniparser_getint( dict, "tracking:min_rect_area", 100);
      tracking.targ_match_dist = 
         iniparser_getint( dict, "tracking:targ_match_dist", 50);
      tracking.targ_persist = 
         iniparser_getint( dict, "tracking:targ_persist", 25);
      tracking.diag = 
         iniparser_getint( dict, "tracking:diag", 0);
      tracking.h_ang_correction = 
         (float) iniparser_getdouble( dict, "tracking:h_ang_correction", 0);

   }
   
   iniparser_freedict( dict );
}

/*
**  Set camera defaults if no config file is available
*/
void T456_set_camera_and_tracking_defaults()
{
   camera_info.camera_id = 0;
   camera_info.h_fov = 48.8;
   camera_info.v_fov = 37.648;
   camera_info.h_pixels = 640;
   camera_info.v_pixels = 480;
   camera_info.h_ifov = 0.07625;
   camera_info.v_ifov = 0.07843;

   tracking.hue_mid_thresh = 122;
   tracking.hue_mid_span = 30;
   tracking.val_thresh = 230;
   tracking.morph_closing_iterations = 1;
   tracking.min_rect_area = 100;
   tracking.targ_match_dist = 50;
   tracking.targ_persist = 25;
   tracking.diag = 0;
   tracking.h_ang_correction = 0.0;

}

/*
**  Print camera and target tracking settings
*/
void T456_print_camera_and_tracking_settings()
{
   printf("\n");
   printf("Camera Settings: \n");
   printf("  camera_info.camera_id = %d\n", camera_info.camera_id);
   printf("  camera_info.h_fov = %.2f\n", camera_info.h_fov);
   printf("  camera_info.v_fov = %.2f\n", camera_info.v_fov);
   printf("  camera_info.h_pixels = %d\n", camera_info.h_pixels);
   printf("  camera_info.v_pixels = %d\n", camera_info.v_pixels);
   printf("  camera_info.h_ifov = %.5f\n", camera_info.h_ifov);
   printf("  camera_info.v_ifov = %.5f\n", camera_info.v_ifov);

   printf("\n");
   printf("Target Tracking Settings: \n");
   printf("  tracking.hue_mid_thresh = %d\n", tracking.hue_mid_thresh);
   printf("  tracking.hue_mid_span = %d\n", tracking.hue_mid_span);
   printf("  tracking.val_thresh = %d\n", tracking.val_thresh);
   printf("  tracking.morph_closing_iterations = %d\n", 
                  tracking.morph_closing_iterations);
   printf("  tracking.min_rect_area = %d\n", tracking.min_rect_area);
   printf("  tracking.targ_match_dist = %d\n", tracking.targ_match_dist);
   printf("  tracking.targ_persist = %d\n", tracking.targ_persist);
   printf("  tracking.diag = %d\n", tracking.diag);
   printf("  tracking.h_ang_correction = %.1f\n", tracking.h_ang_correction);
   printf("\n");

}


/*
**  TEST MAIN SECTION
*/
#ifdef MAIN

int main(int argc, char **argv)
{

  if (argc == 2 ) {
     T456_parse_vision( argv[1] );
  }

  T456_print_camera_and_tracking_settings();

  exit(1);
}

#endif /* MAIN */

