/*
    Simple udp client
*/
#include<stdio.h> //printf
#include<stdlib.h> //exit(0);

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include<string.h> //memset
#include<arpa/inet.h>
#include<sys/socket.h>
 
#define SERVER "127.0.0.1"
#define BUFLEN 1024  //Max length of buffer
#define PORT 8888   //The port on which to send data
 
extern pthread_mutex_t  targ_msg_mutex;

extern char target_message[100];
extern int  target_message_length;

void die(char *s)
{
    perror(s);
    exit(1);
}
 
void *send_udp_message_func()
{
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
 
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
 
    while( target_message_length > 0 )
    {
        usleep(33333.33);  /* sleep at roughly 30 fps */
         
        pthread_mutex_lock( &targ_msg_mutex);

        //send the message
        if (sendto(s, target_message, strlen(target_message) , 0 , (struct sockaddr *) &si_other, slen)==-1)
        {
            die("sendto()");
        }

       pthread_mutex_unlock( &targ_msg_mutex);
         
        //clear the buffer by filling null, it might have previously received data
        memset(buf,'\0', BUFLEN);
    }
 
    close(s);
    return 0;
}
