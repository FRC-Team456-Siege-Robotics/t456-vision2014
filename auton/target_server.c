
#include <stdio.h>
#include<stdlib.h> //exit(0);
#include <string.h>

/* includes for udp option with threads */
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include<string.h> //memset
#include<arpa/inet.h>
#include<sys/socket.h>

#define SERVER "10.4.56.2"
#define BUFLEN 100  //Max length of buffer
#define PORT 8888   //The port on which to send data

/*  include for http server option */
#include "mongoose.h"

extern char target_message[100];
extern int  target_message_length;

extern pthread_mutex_t  targ_msg_mutex;        /* locking variable */

/*
**  This function will be called by mongoose on every new request.
*/
static int begin_request_handler(struct mg_connection *conn) {

  // Send HTTP reply to the client
  mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %d\r\n"        // Always set Content-Length
            "\r\n"
            "%s",
            target_message_length, target_message);

  // Returning non-zero tells mongoose that our function has replied to
  // the client, and mongoose should not send client any more data.
  return 1;
}

struct mg_context *ctx;

void T456_start_http_server(void) {
  struct mg_callbacks callbacks;

  target_message_length =
     snprintf(target_message, sizeof(target_message),
     "00,000000,000000,0000,000000");

  // List of options. Last element must be NULL.
  const char *options[] = {"listening_ports", "8080", NULL};

  // Prepare callbacks structure. We have only one callback, the rest are NULL.
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.begin_request = begin_request_handler;

  // Start the web server.
  ctx = mg_start(&callbacks, NULL, options);

  return;
}

void T456_stop_http_server(void) {

  // Stop the server.
  mg_stop(ctx);

  return;
}

/*
**  Start the udp message server
*/
void T456_start_udp_server(void) {
}


void T456_udp_die(char *s)
{
    perror(s);
    exit(1);
}
 
void *T456_send_udp_message_func()
{
    struct sockaddr_in si_other;
    int s, slen=sizeof(si_other);
    char buf[BUFLEN];
 
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        T456_udp_die("socket");
    }
 
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
     
    if (inet_aton(SERVER , &si_other.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
 
    while( target_message_length > 0 )
    {
        usleep(33333.33);  /* sleep at roughly 30 fps */
         
        pthread_mutex_lock( &targ_msg_mutex);

        //send the message
        if (sendto(s, target_message, strlen(target_message) , 0 , (struct sockaddr *) &si_other, slen)==-1)
        {
            T456_udp_die("sendto()");
        }

       pthread_mutex_unlock( &targ_msg_mutex);
         
        //clear the buffer by filling null, it might have previously received data
        memset(buf,'\0', BUFLEN);
    }
 
    close(s);
    return 0;
}
