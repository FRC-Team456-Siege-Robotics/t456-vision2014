
/*
**  FRC Team 456 Siege Robotics
**  Vicksburg, MS
**  2014 Competition Code
**
**  arduino system test code
**   exercises all arduino states at 30 Hz
**
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "arduino-serial-lib.h"

/*  
**  Global file descriptor for serial comms with Arduino
*/
int serial_fd = -1;

#define SLEEP_TIME 33333.3

/* 
** =================================================================
**  MAIN program
** =================================================================
*/
int main( int argc, char **argv)
{
  int i,j;
  int ball_color = 0;
  int ball_dist = 0;
  char dataline[20];

   printf("BEGIN Arduino Test Sequence\n");
   printf("  Open Serial Port: ");
   /*
   **  Open serial port to Arduino
   */
   serial_fd = serialport_init( "/dev/ttyACM0", 9600);

   /* check for connection error to Arduino.  serial_fd < 0 is bad */
   if ( serial_fd < 0 ) {  
      fprintf(stderr,"BALLTRACKING: *Arduino ERROR* Unable to open com line");
      fprintf(stderr," to Arduino.\n");

      /* Arduino is not connected, so redirect output to /dev/null */
      serial_fd = serialport_init( "/dev/null", 9600);
   } 
   if ( serial_fd < 0 ) {
      printf(" FAILED\n");
      exit(-1);
   }
   printf(" PASSED\n");
   sleep(2);

   /*
   **  Pre-match Test State 0, initial state
   **  Format: 0 <ball_color>
   */

   printf("State 0: RED test (5 sec)\n");
   ball_color = 0;  /* RED */
   for ( i = 0; i < (5*30); i++ )
   {
     sprintf(dataline,"0 %d\n", ball_color);
     serialport_write(serial_fd, dataline );
     usleep(SLEEP_TIME);  
   }
   sleep(1);

   printf("State 0: BLUE test (5 sec)\n");
   ball_color = 1;  /* BLUE */
   for ( i = 0; i < (5*30); i++ )
   {
     sprintf(dataline,"0 %d\n", ball_color);
     serialport_write(serial_fd, dataline );
     usleep(SLEEP_TIME);  
   }
   sleep(1);


   /*
   **  Autonomous Test State 1,
   **  Format: 1 <ball_color> <hot_target>
   */
   printf("State 1: RED test no hot target (5 sec)\n");
   ball_color = 0;  /* RED */
   for ( i = 0; i < (5*30); i++ )
   {
     sprintf(dataline,"1 %d 0\n", ball_color);
     serialport_write(serial_fd, dataline );
     usleep(SLEEP_TIME);  
   }
   printf("State 1: RED test HOT target (5 sec)\n");
   for ( i = 0; i < (5*30); i++ )
   {
     sprintf(dataline,"1 %d 1\n", ball_color);
     serialport_write(serial_fd, dataline );
     usleep(SLEEP_TIME);  
   }

   printf("State 1: BLUE test no hot target (5 sec)\n");
   ball_color = 1;  /* BLUE */
   for ( i = 0; i < (5*30); i++ )
   {
     sprintf(dataline,"1 %d 0\n", ball_color);
     serialport_write(serial_fd, dataline );
     usleep(SLEEP_TIME);  
   }
   printf("State 1: BLUE test HOT target (5 sec)\n");
   for ( i = 0; i < (5*30); i++ )
   {
     sprintf(dataline,"1 %d 1\n", ball_color);
     serialport_write(serial_fd, dataline );
     usleep(SLEEP_TIME);  
   }
   sleep(1);

   /*
   **  Ball Tracking Test State 2,
   **  Format: 2 <ball_pos> <ball_dist> <ball_color>
   */
   printf("State 2: RED ball tracking\n");
   ball_color = 0;  /* RED */
   ball_dist = 20;
   for ( i = 0; i < 5; i++ )
   {
     ball_dist -= 3; 
     for ( j = 1; j < 640; j+=20)
     {
        sprintf(dataline,"2 %d %d %d\n", j, ball_dist, ball_color);
        serialport_write(serial_fd, dataline );
        usleep(SLEEP_TIME);  
     }
     for ( j = 640; j > 0; j-=20)
     {
        sprintf(dataline,"2 %d %d %d\n", j, ball_dist, ball_color);
        serialport_write(serial_fd, dataline );
        usleep(SLEEP_TIME);  
     }
   }
   sleep(1);

   printf("State 2: BLUE ball tracking\n");
   ball_color = 1;  /* BLUE */
   ball_dist = 20;
   for ( i = 0; i < 5; i++ )
   {
     ball_dist -= 3; 
     for ( j = 1; j < 640; j+=20)
     {
        sprintf(dataline,"2 %d %d %d\n", j, ball_dist, ball_color);
        serialport_write(serial_fd, dataline );
        usleep(SLEEP_TIME);  
     }
     for ( j = 640; j > 0; j-=20)
     {
        sprintf(dataline,"2 %d %d %d\n", j, ball_dist, ball_color);
        serialport_write(serial_fd, dataline );
        usleep(SLEEP_TIME);  
     }
   }
   sleep(1);


   /*
   **  End Match Test State 4,
   */
   printf("State 4: test (5 sec)\n");
   for ( i = 0; i < (5*30); i++ )
   {
     sprintf(dataline,"4\n");
     serialport_write(serial_fd, dataline );
     usleep(SLEEP_TIME);  
   }

   /*
   **  State 5 heartbeat
   */
   printf("State 5: heart beat test (5 sec)\n");
   ball_color = 1;  /* BLUE */
   for ( i = 0; i < (5*30); i++ )
   {
     sprintf(dataline,"5 %d\n", ball_color);
     serialport_write(serial_fd, dataline );
     usleep(SLEEP_TIME);  
   }
   /* 
   **  close serial port to Arduino
   */
   if ( serial_fd > 0 )
      serialport_close(serial_fd);

}
