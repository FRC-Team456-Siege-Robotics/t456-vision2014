
#include <stdio.h>

int ball_color = 0;  /* 0  = red, 1 = blue */
int serial_fd = -1;  /* file descriptor for serial output */

void target_tracking( int, char** );

int main(int argc, char** argv)
{
   if ( argc == 2 ) {
     /*  two arguments, can either be a number or a file */
     if ( isdigit(argv[1][0]) )
     {
        ball_color = argv[1][0] - '0';

        if ( (ball_color > 1) || (ball_color < 0 ) ) {
           fprintf(stderr,"AUTON: Incorrect ball color (%d): setting to default\n",
                     ball_color );
           ball_color = 0;
        }

        if ( ball_color == 0 )
           fprintf(stderr,"AUTON: Tracking RED ball\n");
        else
           fprintf(stderr,"AUTON: Tracking BLUE ball\n");
     }
   }  else {
      fprintf(stderr,"AUTON: No ball color specified, defaulting to RED\n");
      ball_color = 0;
   }

   /*
   **  Open serial port to Arduino
   */
   serial_fd = serialport_init( "/dev/ttyACM0", 9600);

   /* check for connection error to Arduino.  serial_fd < 0 is bad */
   if ( serial_fd < 0 ) {  
      fprintf(stderr,"AUTON: *Arduino ERROR* Unable to open com line");
      fprintf(stderr," to Arduino, writing to /dev/null instead\n");

      /* Arduino is not connected, so redirect output to /dev/null */
      // serial_fd = serialport_init( "/dev/tty2", 9600);
   } 
   sleep(1);

   target_tracking( argc, argv );

   serialport_close(serial_fd);

   return(1);
}
