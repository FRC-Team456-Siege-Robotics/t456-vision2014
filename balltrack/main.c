
/*
**  FRC Team 456 Siege Robotics
**  Vicksburg, MS
**  2014 Competition Code
**
**  Usage: exec_name config_file
**
**  NOTES:
**  This is multi-threaded (multi-core) code 
**  
**  Threads:
**    1) UDP communication to send out target information
**    2..n) Camera I/O and target processing
**   
*/
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include "t456-vision.h"


/*
**  External function prototypes
*/
extern void *send_udp_message_func();
extern void *T456_camera_funcs(void *);
extern void T456_parse_vision( char *);

/*
**  Global variables for messages
*/
pthread_mutex_t  targ_msg_mutex;        /* locking variable */
char target_message[100];               /* message between processes */
int  target_message_length;             /* length of message */

/*
**  Global variables about the camera and processing
*/
camera_struct camera_info;              /* information about the camera */
proc_struct   proc_info;                /* informaitno about processing */

/* 
** =================================================================
**  MAIN program
** =================================================================
*/
int main( int argc, char **argv)
{
   arg_struct args;

   pthread_t udp_msg_thread;      /* thread for messages out via UDP */
   pthread_t cam_thread;          /* thread for camera processing */
   pthread_attr_t attr;           /* attribute thread */

   int  msg_ret_val;
   int  cam_ret_val;
   int  cam_ret_val2;

   /*
   **  Parse the config file and get parameters for the program
   */
   T456_parse_vision( "t456-vision.ini" );

   /*
   **  One time function call to XInitThreads if threads need to 
   **   share a X11 graphic
   */
   if ( proc_info.graphics != 0 )
   {
      XInitThreads();        
   }

   /*
   **  Copy command line arguments into data structure
   */
   args.argc = argc;               
   args.argv = argv;

   /*  initialize and set attribute thread variable */
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

   /*
   **  Initialize initial message to control system
   **   this is done before spawning communication thread
   */
   target_message_length =
                   snprintf(target_message, sizeof(target_message),
                  "-1,00,000000,000000,000000,0000");

   /*
   **  Initialize the mutex (locking/unlocking for message info)
   */
   if (pthread_mutex_init(&targ_msg_mutex, NULL))
   {
      printf("Unable to initialize a mutex for message\n");
      exit(-1);
   }

   /*
   **  Create and execute communication thread 
   */
   msg_ret_val = pthread_create( &udp_msg_thread, NULL,
                                 &send_udp_message_func, NULL);

   T456_camera_funcs( (void *) &args);

   /*
   **  Wait for threads to finish
   */
   pthread_kill( udp_msg_thread, NULL);

   /*
   **  All finished, exit nicely
   */
   exit(0);
}

