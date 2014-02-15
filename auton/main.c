
#include <stdio.h>

int ball_color = 0;  /* 0  = red, 1 = blue */

void target_tracking( int, char** );

int main(int argc, char** argv)
{
   if ( argc == 2 ) {
     /*  two arguments, can either be a number or a file */
     if ( isdigit(argv[1][0]) )
     {
        ball_color = argv[1][0] - '0';

        if ( (ball_color > 1) || (ball_color < 0 ) ) {
           fprintf(stderr,"Incorrect ball color (%d): setting to default\n",
                     ball_color );
           ball_color = 0;
        }

        if ( ball_color == 0 )
           fprintf(stderr,"Tracking RED ball\n");
        else
           fprintf(stderr,"Tracking BLUE ball\n");
     }
   }

   target_tracking( argc, argv );
   return(1);
}
