
/*
**  FRC Team 456 Siege Robotics
**  Control code for 2014 competition
**
**  Reads input commands from UDP port and controls/directs vision processing
**
**  CAUTION:
**   anything printed out to stdio (normal printf) will be redirected to the Arduino
**   place debug statements and general information status to stderr
**
**  FORMAT:
**   Match Status = AKA State  (integer)
**   Alliance Color = AKA Ball Color ( integer, 0 = red, 1 = blue )
**   Match Time = elapsed match time (float)
**   Ball Tracking = Ball tracking enabled (1 = yes, 0 = no )
**   Ball Loaded = Ball loaded (1 = yes, 0 = no)
**   Checksum (error checking) = Status + Color + Tracking + Loaded
**
**  The possible states are:
**   0) Startup  (Initial state from system boot)
**   1) Begin Auton
**   2) Start Ball Tracking
**   3) End Match (shutdown)
**   4) Testing: Start Ball Tracking
**   5) Testing: Stop Ball Tracking
**   6) Testing: Start Auton Tracking
**   7) Testing: Stop Auton Tracking
**  -1) Exit w no shutdown (kill all processes)
**
Examples:
0,0,1.0,0,1,1   startup, red, 1 sec elaped time, no tracking, ball loaded, checksum
2,1,30.5,1,0,5  ball tracking, blue, 30. sec elat, tracking, no ball loaded, checksum
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include<string.h> //memset
#include<arpa/inet.h>
#include<sys/socket.h>

#include "iniparser.h"

#define BUFLEN 1024  //Max length of buffer
#define PORT 8881   //The port on which to listen for incoming data
                     // UDP 8880 is incoming UDP data from CRIO 
                     // UDP 8888 is outgoing UDP data from Vision System
#define RED_BALL = 0
#define BLUE_BALL = 1

struct {
  char auton[120];
  char balltrack[120];
} process_info; 

void  T456_parse_config(char *);
void  T456_set_default_settings();

/*
**  GLOBAL variables
*/
int socket_fd;    /* incoming data socket file descriptor */

void T456_UDP_init();
void T456_UDP_read( char *);
void parse_message(char *, int *, int *, float *, int *, int *);

/*
**  exit/die function
*/
void die(char *s)
{
    perror(s);
    exit(1);
}
 
/*
    Spawn function:  This is where external programs are started
                       based on the current state of the match
    Spawn a child process running the new program.
    Returns the process ID of the spawned process.
*/
int spawn (char* program, char** arg_list)
{
   pid_t child_pid;

   /* Duplicate this process. */
   child_pid = fork ();
   if (child_pid != 0)
      /* This is the parent process. */
      return child_pid;
   else {
      /* Now execute PROGRAM, searching for it in the path. */
      execvp (program, arg_list);
      /* The execvp function returns only if an error occurs. */
      fprintf(stderr,"an error occured in execvp\n");
      abort ();
   }
}

/*
**  MAIN PROGRAM
*/
int main( int argc, char **argv)
{
   int child_pid, new_state = 0;
   char udp_message[BUFLEN];
   int prev_state;
   int auton_pid = -1;
   int balltrack_pid = -1;
   int ball_color = 0;     /* default color ( 0 = red, 1 = blue ) */
   float game_time;

   int tracking = 0;
   int ball_loaded = 0;

   /*
   **  Initialize and define current state of processes and match
   **  The possible states are:
   **   0) Startup  (Initial state from system boot)
   **   1) Start Auton target tracking
   **   2) Start Ball Tracking
   **   3) End Match (shutdown)
   **   4) Testing: Start Ball Tracking
   **   5) Testing: Stop Ball Tracking
   **   6) Testing: Start Auton Tracking
   **   7) Testing: Stop Auton Tracking
   **  -1) Exit w no shutdown (kill all processes)
   */
   int CURRENT_STATE = -999;

   /* 
   **  setup the argument list 
   */
   char *arg_list[] = {
      "t456_auto_tracking",   /*  argv[0], the name of the program */
      NULL    /* the argument list must terminate with NULL */
   };
   char *arg_balllist[3] = {
      "t456_balltrack",   /*  argv[0], the name of the program */
      "0",              /*  0 = red, default ball color */
      NULL    /* the argument list must terminate with NULL */
   };

   fprintf(stderr,"parent process ID %d\n", (int) getpid() );

   T456_parse_config("t456-vcontrol.ini");

   fprintf(stderr, "   using auton exec: %s\n", process_info.auton);
   fprintf(stderr,"   using balltrack exec: %s\n", process_info.balltrack);

   /*
   **  Initialize the UDP communication
   */
   T456_UDP_init();


   /*
   **  Loop and process UDP messages that change our current state
   */
   while(1)
   {

      if (new_state != CURRENT_STATE )
      {
         prev_state = CURRENT_STATE;
         CURRENT_STATE = new_state;
         fprintf(stderr, "CURRENT STATE: %d\n", CURRENT_STATE);

         switch(CURRENT_STATE)
         {
            case -1:   /* kill all processes and quit */
               fprintf(stderr, "Received quit state: killing child processes.\n"); 
               if ( auton_pid != -1 ) 
                  kill(auton_pid, SIGTERM);
               if ( balltrack_pid != -1 ) 
                  kill(balltrack_pid, SIGTERM);
               exit(0);
               break;

            case 0:   /* startup, begin auton program */
               /* just in case we are reversing states, kill balltracking */
               if ( balltrack_pid != -1 ) 
               {
                  kill(balltrack_pid, SIGTERM);
                  balltrack_pid = -1;
                  sleep(1);
               }
               if ( auton_pid == -1 ) 
               {
                  /* Spawn the auton process */
                  fprintf(stderr,"State 0: spawning auton program\n");
                  auton_pid = spawn(process_info.auton, arg_list);
                  fprintf(stderr, "auton process id: %d\n", auton_pid );
               }
               break;

            case 1: // **   1) Begin Auton
               /* just in case we are reversing states, kill balltracking */
               if ( balltrack_pid != -1 ) 
               {
                  kill(balltrack_pid, SIGTERM);
                  balltrack_pid = -1;
                  sleep(1);
               }
               if ( auton_pid == -1 ) {
                  /* Spawn the auton process */
                  fprintf(stderr,"State 1: spawning auton program\n");
                  auton_pid = spawn(process_info.auton, arg_list);
                  fprintf(stderr,"auton process id: %d\n", auton_pid );
               }
               break;

            case 2: // **   2) End Auton
               if ( auton_pid != -1 ) {
                  kill(auton_pid, SIGTERM);
                  auton_pid = -1;
                  sleep(1);
               }
               if ( balltrack_pid == -1 ) {
                  /* Spawn the auton process */
                  fprintf(stderr,"State 2: spawning balltrack program\n");
                  fprintf(stderr,"           ball color: %d\n", ball_color);
                  if ( ball_color == 0 ) arg_balllist[1] = "0";
                  else arg_balllist[1] = "1";
                  balltrack_pid = spawn(process_info.balltrack, arg_balllist);
                  fprintf(stderr,"balltrack process id: %d\n", balltrack_pid );
               }
               break;

            case 3: // **   3) End Match (shutdown)
               fprintf(stderr,"End Match. SHUTDOWN\n");
               if ( auton_pid != -1 ) {
                  kill(auton_pid, SIGTERM);
                  auton_pid = -1;
               }
               if ( balltrack_pid != -1 ) {
                  kill(balltrack_pid, SIGTERM);
                  balltrack_pid = -1;
               }
               sleep(5);
               fprintf(stderr,"Issuing shutdown command\n");
               system("/sbin/shutdown -h now");
               break;

            case 4: // **   4) Testing: Start Ball Tracking
               if ( balltrack_pid == -1) 
               {
                  fprintf(stderr,"State 4: spawning ball track program\n");
                  if ( ball_color == 0 ) arg_balllist[1] = "0";
                  else arg_balllist[1] = "1";
                  balltrack_pid = spawn(process_info.balltrack, arg_balllist);
                  fprintf(stderr,"balltrack process id: %d\n", balltrack_pid);
               }
               break;

            case 5: // **   5) Testing: Stop Ball Tracking
               if ( balltrack_pid != -1) 
               {
                  fprintf(stderr,"State 5: killing ball track program\n");
                  kill(balltrack_pid, SIGTERM);
                  balltrack_pid = -1;
               }
               break;

            case 6: // **   6) Testing: Start Auton Tracking
               /* just in case we are reversing states, kill balltracking */
               if ( balltrack_pid != -1 ) 
               {
                  kill(balltrack_pid, SIGTERM);
                  balltrack_pid = -1;
                  sleep(1);
               }
               if ( auton_pid == -1 ) 
               {
                  fprintf(stderr,"State 5: spawning auton program\n");
                  auton_pid = spawn(process_info.auton, arg_list);
                  fprintf(stderr,"auton pid: %d\n", auton_pid);
               }
               break;

            case 7: // **   7) Testing: Stop Auton Tracking
               if ( auton_pid != -1 ) {
                  fprintf(stderr,"State 7: killing auton program\n");
                  kill(auton_pid, SIGTERM);
                  auton_pid = -1;
               }
               break;

            default:
               fprintf(stderr,"Unknown state! %d\n", CURRENT_STATE);
               CURRENT_STATE = prev_state;  /* reset back to previous */
               break;
         }
      }

      /*  
      ** processed current state, go wait for additional messages 
      */
      T456_UDP_read(udp_message);
      fprintf(stderr, "message: %s\n", udp_message );
      parse_message(udp_message, &ball_color, &new_state, 
                                 &game_time, &tracking, &ball_loaded);

   }  /* end while */

   fprintf(stderr,"done with the main program\n");
   

   return(0);
}


void T456_UDP_init()
{
   struct sockaddr_in si_me;

   /*
   **  create the UDP messagesocket
   */
   if ((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
   {
       die("socket");
   }

   // zero out the structure
   memset((char *) &si_me, 0, sizeof(si_me));
    
   si_me.sin_family = AF_INET;
   si_me.sin_port = htons(PORT);
   si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    
   //bind socket to port
   if( bind(socket_fd , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
   {
        die("bind");
   }
     
}

void T456_UDP_read( char *message )
{
   int  i, recv_len;
   struct sockaddr_in si_other;
   int  slen = sizeof(si_other);
   char buf[BUFLEN];

   for ( i = 0; i < BUFLEN; i++ )
      buf[i] = '\0';

   fprintf(stderr,"Waiting for incoming data...");
   fflush(stdout);
         
   //try to receive some data, this is a blocking call
   if ((recv_len = recvfrom(socket_fd, buf, BUFLEN, 
                    0, (struct sockaddr *) &si_other, &slen)) == -1)
   {
      die("recvfrom()");
   }
         
   //print details of the client/peer and the data received
   fprintf(stderr,"Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
   fprintf(stderr,"Data: %s\n" , buf);

   strcpy( message, buf );
}

void parse_message(char *message, int *color, int *status, float *time,
                   int *tracking, int *ball_loaded)
{
   // local message parameters
   int ball_color, stat, checksum;
   int in_tracking, in_ball_loaded;
   float game_time;

   // message will be of the format: MATCH_STATUS,COLOR,TIME,CHECK_SUM
   // Parse message
   sscanf(message,"%d,%d,%f,%d,%d,%d",  &stat, &ball_color, &game_time, 
                                    &in_tracking, &in_ball_loaded, &checksum);
   /* we don't won't to introduce values to the system if they are corrupted,
      so verify with checksum first.
	  Checksum is parsed as an int so take the tens digit by dividing
	  and the ones by modulo.  Take absolute of the ones digit because
	  we don't want it to inherit the negative sign from the 
	  "shutdown" signal */
   if ( (stat + ball_color + in_tracking + in_ball_loaded) == checksum )
   {
       *color = ball_color;
       *status = stat;
       *time = game_time;
       *tracking = in_tracking;
       *ball_loaded = in_ball_loaded;
   } else {
       fprintf(stderr, "Recieved corrupted data from CRIO, checksum did not match\n");
   }
}


/*
**  Parse input configuration file
*/

void T456_parse_config(char *input_config_file)
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
         " ***WARNING***    Using DEFAULT camera and image settings\n");
      /*  
      ** set camera default settings 
      */
      T456_set_default_settings();
      return;
   }
   else   /* parse the input file */
   {
      /* get process settings */
      strcpy(process_info.auton, iniparser_getstring( dict, 
              "process:auton", "./auton"));
      strcpy(process_info.balltrack, iniparser_getstring( dict, 
              "process:balltrack", "./balltrack"));
   }

   iniparser_freedict( dict );
}

/*
**  Set camera defaults if no config file is available
*/
void T456_set_default_settings()
{
   strcpy(process_info.auton,"../auton/auton");
   strcpy(process_info.balltrack,"../balltrack/balltrack");
}
