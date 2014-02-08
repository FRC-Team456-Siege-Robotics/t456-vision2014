
#include <stdio.h>

void main( int argc, char **argv )
{
  printf("Balltracking program running\n");

  if ( argc == 2 )
  {
     if ((argv[1][0] - '0') == 0 )
        printf("tracking RED ball\n");
     else
        printf("tracking BLUE ball\n");
  }

  while(1)
  {
     sleep(1);  /* sleep for 1 sec */
     printf("balltracking still running\n");
  }
}
