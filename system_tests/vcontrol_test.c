
/*
**  Test program for vcontrol (which will exercise auton and ball tracking)
**
**  FRC Team 456 Siege Robotics
**  Vicksburg, MS
**  2014 Competition Code
**
** 
*/

#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>

#include <unistd.h>
 
#define SERVER "127.0.0.1"
#define BUFLEN 1024  //Max length of buffer
#define PORT 8800   //The port on which to send data

void die(char *s)
{
    perror(s);
    exit(1);
}

#define SLEEP_TIME 100000.0
 
int main(void)
{
    int i;

    struct sockaddr_in si_other;
    int s, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
 

    /*
    **  Open socket
    */
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
 
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
     
    if (inet_aton(SERVER , &si_other.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
 

/*
Examples:
0,0,1.0,0,1,1   startup, red, 1 sec elaped time, no tracking, ball loaded, checksum
2,1,30.5,1,0,5  ball tracking, blue, 30. sec elat, tracking, no ball loaded, checksum

checksum = stat + ball_color + in_tracking + ball_loaded
  
*/

    printf(" Exercise test for vcontrol code.\n\n");

    /*
    **  Test 1:  Startup State 0
    */
    printf("Running Test 1: Startup State 0 (3 sec @ 10Hz) ");
    fflush(stdout);
    for ( i = 0; i < 30; i++ )
    {
       sprintf(message,"0,0,%d,0,1,1", i);
       //send the message
       if (sendto(s, message, strlen(message) , 0 , 
                   (struct sockaddr *) &si_other, slen)==-1)
       {
           die("sendto()");
       }
       usleep(SLEEP_TIME);  
         
    }
    printf(" COMPLETE\n");
    sleep(2);

    /*
    **  Test 2:  State 1  begin autonomous
    */
    printf("Running Test 2a: Autonomous State 1, red ball, (6 sec @ 10Hz) ");
    fflush(stdout);
    for ( i = 0; i < 60; i++ )
    {
       sprintf(message,"1,0,%d,0,1,2", i);
       //send the message
       if (sendto(s, message, strlen(message) , 0 , 
                   (struct sockaddr *) &si_other, slen)==-1)
       {
           die("sendto()");
       }
       usleep(SLEEP_TIME);  
         
    }
    printf(" COMPLETE\n");
    sleep(2);

    printf("Running Test 2b Autonomous State 1, blue ball, (6 sec @ 10Hz) ");
    fflush(stdout);
    for ( i = 0; i < 60; i++ )
    {
       sprintf(message,"1,1,%d,0,1,3", i);
       //send the message
       if (sendto(s, message, strlen(message) , 0 , 
                   (struct sockaddr *) &si_other, slen)==-1)
       {
           die("sendto()");
       }
       usleep(SLEEP_TIME);  
         
    }
    printf(" COMPLETE\n");
    sleep(2);

    /*
    **  Test 3:  State 2  start ball tracking
    */
    printf("Running Test 3a: Ball Tracking State 2, red ball, (6 sec @ 10Hz) ");
    fflush(stdout);
    for ( i = 0; i < 60; i++ )
    {
       sprintf(message,"2,0,%d,0,1,3", i);
       //send the message
       if (sendto(s, message, strlen(message) , 0 , 
                   (struct sockaddr *) &si_other, slen)==-1)
       {
           die("sendto()");
       }
       usleep(SLEEP_TIME);  
         
    }
    printf(" COMPLETE\n");

    printf("Running Test 3b: Ball Tracking State 2, red ball, no ball loaded, (6 sec @ 10Hz) ");
    fflush(stdout);
    for ( i = 0; i < 60; i++ )
    {
       sprintf(message,"2,0,%d,0,0,2", i);
       //send the message
       if (sendto(s, message, strlen(message) , 0 , 
                   (struct sockaddr *) &si_other, slen)==-1)
       {
           die("sendto()");
       }
       usleep(SLEEP_TIME);  
         
    }
    printf(" COMPLETE\n");
    sleep(2);

    /*
    **  Test 4: State 3  end match (shutdown)
    */

    printf("Running Test 4 Stop Vcontrol (no shutdown) (1 sec @ 10Hz) ");
    fflush(stdout);
    for ( i = 0; i < 10; i++ )
    {
       sprintf(message,"-1,1,%d,0,1,1", i);
       //send the message
       if (sendto(s, message, strlen(message) , 0 , 
                   (struct sockaddr *) &si_other, slen)==-1)
       {
           die("sendto()");
       }
       usleep(SLEEP_TIME);  
         
    }
    printf(" COMPLETE\n");
    sleep(2);
 

    printf("ALL TESTS COMPLETE\n");
    /*
    **  Test complete, close socket and exit
    */
    close(s);
    return 0;
}
