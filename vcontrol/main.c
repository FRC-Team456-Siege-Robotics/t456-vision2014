
/*
**  FRC Team 456 Siege Robotics
**  Control code for 2014 competition
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include<string.h> //memset
#include<arpa/inet.h>
#include<sys/socket.h>

#define BUFLEN 1024  //Max length of buffer
#define PORT 1140   //The port on which to listen for incoming data
                     // UDP 1140 is incoming UDP data from CRIO 
                     // UDP 1130 is outgoing UDP data from Vision System

/*
**  GLOBAL variables
*/
int socket_fd;    /* incoming data socket file descriptor */

void T456_UDP_init();
void T456_UDP_read( char *);

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

   /*
   **  Initialize and define current state of processes and match
   **  The possible states are:
   **   0) Startup  (Initial state from system boot)
   **   1) Begin Auton
   **   2) End Auton
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
      "my_auto_stub",   /*  argv[0], the name of the program */
      NULL    /* the argument list must terminate with NULL */
   };
   char *arg_balllist[] = {
      "balltrack",   /*  argv[0], the name of the program */
      NULL    /* the argument list must terminate with NULL */
   };

   printf("parent process ID %d\n", (int) getpid() );

   /*
   **  Initialize the UDP communication
   */
   T456_UDP_init();


   /*
   **  Loop and process UDP messages that change our current state
   */
   while(1)
   {
      printf("CURRENT STATE: %d\n", CURRENT_STATE);

      if (new_state != CURRENT_STATE )
      {
         prev_state = CURRENT_STATE;
         CURRENT_STATE = new_state;

         switch(CURRENT_STATE)
         {
            case -1:   /* kill all processes and quit */
               printf("Received quit state: killing child processes.\n"); 
               if ( auton_pid != -1 ) 
                  kill(auton_pid, SIGTERM);
               if ( balltrack_pid != -1 ) 
                  kill(balltrack_pid, SIGTERM);
               exit(0);
               break;

            case 0:   /* startup, begin auton program */
               if ( auton_pid == -1 ) {
                  /* Spawn the auton process */
                  printf("State 0: spawning auton program\n");
                  auton_pid = spawn("auto_stub", arg_list);
                  printf("auton process id: %d\n", auton_pid );
               }
               break;

            case 1: // **   1) Begin Auton
               if ( auton_pid == -1 ) {
                  /* Spawn the auton process */
                  printf("State 1: spawning auton program\n");
                  auton_pid = spawn("auto_stub", arg_list);
                  printf("auton process id: %d\n", auton_pid );
               }
               break;

            case 2: // **   2) End Auton
               if ( auton_pid != -1 ) 
                  kill(auton_pid, SIGTERM);
               if ( balltrack_pid == -1 ) {
                  /* Spawn the auton process */
                  printf("State 2: spawning balltrack program\n");
                  balltrack_pid = spawn("balltrack_stub", arg_balllist);
                  printf("balltrack process id: %d\n", balltrack_pid );
               }
               break;

            case 3: // **   3) End Match (shutdown)
               if ( auton_pid != -1 ) 
                  kill(auton_pid, SIGTERM);
               if ( balltrack_pid != -1 ) 
                  kill(balltrack_pid, SIGTERM);
               break;

            case 4: // **   4) Testing: Start Ball Tracking
               break;

            case 5: // **   5) Testing: Stop Ball Tracking
               break;

            case 6: // **   6) Testing: Start Auton Tracking
               break;

            case 7: // **   7) Testing: Stop Auton Tracking
               break;

            default:
               printf("Unknown state! %d\n", CURRENT_STATE);
               CURRENT_STATE = prev_state;  /* reset back to previous */
               break;
         }
      }

      /*  
      ** processed current state, go wait for additional messages 
      */
      T456_UDP_read(udp_message);
      printf("message: %s\n", udp_message );
      sscanf(udp_message,"%d", &new_state);
      
   }  /* end while */

   printf("done with the main program\n");
   

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

   printf("Waiting for incoming data...");
   fflush(stdout);
         
   //try to receive some data, this is a blocking call
   if ((recv_len = recvfrom(socket_fd, buf, BUFLEN, 
                    0, (struct sockaddr *) &si_other, &slen)) == -1)
   {
      die("recvfrom()");
   }
         
   //print details of the client/peer and the data received
   printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
   printf("Data: %s\n" , buf);
         
   strcpy( message, buf );
}
