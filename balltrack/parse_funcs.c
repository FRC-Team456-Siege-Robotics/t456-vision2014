
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
**  External include files
*/
#include "iniparser.h"

#include "t456-vision.h"

/*
**  Information structures
*/
extern camera_struct camera_info;
extern proc_struct proc_info;


void T456_set_default_settings();
void T456_print_settings();

/*
**  Parse input file for camera settings 
**   and image processing settings
*/
void T456_parse_vision( char *input_config_file)
{
   dictionary *dict;
   char  *dist_string_3pt;
   char  *ang_string_3pt;
   char  *dist_string_2pt;
   char  *ang_string_2pt;

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
         " ***WARNING***    Using DEFAULT camera and image settings\n");

      /*  
      ** set camera default settings 
      */
      T456_set_default_settings();
      return;
   }
   else   /* parse the input file */
   {
      /* get camera settings */
      camera_info.camera_id = iniparser_getint( dict, "camera:camera_id", -1);
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

      /* get processing settings */
      proc_info.nthreads = iniparser_getint( dict, "computer:nthreads", 2);
      proc_info.graphics = iniparser_getint( dict, "computer:graphics", 0 );
      proc_info.timing_check = iniparser_getint( dict, "computer:timing_check", 0 );
      proc_info.save_frames = iniparser_getint( dict, "computer:save_frames", 0 );
      proc_info.wait_time = iniparser_getint( dict, "computer:wait_time", 30 );

   }

   T456_print_settings();


   iniparser_freedict( dict );
}
 


/*
**  Set camera defaults if no config file is available
*/
void T456_set_default_settings()
{
   camera_info.camera_id = -1;
   camera_info.h_fov = 48.8;
   camera_info.v_fov = 37.648;
   camera_info.h_pixels = 640;
   camera_info.v_pixels = 480;
   camera_info.h_ifov = 0.07625;
   camera_info.v_ifov = 0.07843;

   proc_info.nthreads = 2;
   proc_info.graphics = 0;
   proc_info.timing_check = 0;
   proc_info.save_frames = 0;
   proc_info.wait_time = 30;
}

/*
**  Print settings
*/
void T456_print_settings()
{
   int i;

   printf("\n");
   printf("Camera Settings: \n");
   printf("  camera_info.camera_id = %d\n", camera_info.camera_id);
   printf("  camera_info.h_fov = %.2f\n", camera_info.h_fov);
   printf("  camera_info.v_fov = %.2f\n", camera_info.v_fov);
   printf("  camera_info.h_pixels = %d\n", camera_info.h_pixels);
   printf("  camera_info.v_pixels = %d\n", camera_info.v_pixels);
   printf("  camera_info.h_ifov = %.5f\n", camera_info.h_ifov);
   printf("  camera_info.v_ifov = %.5f\n", camera_info.v_ifov);

   printf("Computer Settings: \n");
   printf("  proc_info.nthreads = %d\n", proc_info.nthreads);
   printf("  proc_info.graphics = %d\n", proc_info.graphics);
   printf("  proc_info.timing_check = %d\n", proc_info.timing_check);
   printf("  proc_info.save_frames = %d\n", proc_info.save_frames);
   printf("  proc_info.wait_time = %d\n", proc_info.wait_time);
}


