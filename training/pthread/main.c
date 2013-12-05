
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

extern void *send_udp_message_func();
void *process_images_func();

pthread_mutex_t  targ_msg_mutex = PTHREAD_MUTEX_INITIALIZER;

char target_message[100];
int  target_message_length;

main( int argc, char **argv )
{
   pthread_t udp_msg_thread;
   pthread_t vis_thread;

   int  msg_ret_val;
   int  vis_ret_val;

   /*
   **  Initialize initial message to control system
   **   this is done before spawing threads
   */
   target_message_length =
                   snprintf(target_message, sizeof(target_message),
                  "-1,00,000000,000000,000000,0000");
   /*
   **  Create independent threads
   **   each will execute a function
   */
   msg_ret_val = pthread_create( &udp_msg_thread, NULL, 
                                 &send_udp_message_func, NULL);

   vis_ret_val = pthread_create( &vis_thread, NULL, 
                                 &process_images_func, NULL);

   /*
   **  Wait here at the pthread_join until all threads are complete
   */
   pthread_join( udp_msg_thread, NULL );
   pthread_join( vis_thread, NULL );


   exit(0);
}

// void *send_udp_message_func()
// {
//    printf("udp func\n");
//    while( target_message_length > 0) 
//    {
//      pthread_mutex_lock( &targ_msg_mutex);
//      printf("%s\n", target_message);
//      pthread_mutex_unlock( &targ_msg_mutex);
//    
//    }
//    
// }

void *process_images_func()
{
   int i;
   unsigned int usecs;

   usecs = 33333.33;  /* simulate 30 fps */

   printf("process image func\n");

 while(1)
 {
   i = 0;
   while (  i < 300 )
   {
      usleep(usecs);

      printf("process_images_func: %d\n", i);

      /*  memory lock shared message variable */
      pthread_mutex_lock( &targ_msg_mutex);

      target_message_length =
                   snprintf(target_message, sizeof(target_message),
                  "%06d,00,%06d,%06d,000000,0000", i, (i % 30), (i % 60) );
  
      /*  memory unlock shared message variable */
      pthread_mutex_unlock( &targ_msg_mutex);

      i++;
   }
 }

   /*
   **  All done, signal UDP message to shutdown
   */
   /*  memory lock shared message variable */
   pthread_mutex_lock( &targ_msg_mutex);
   target_message_length = -1;
   /*  memory unlock shared message variable */
   pthread_mutex_unlock( &targ_msg_mutex);

}

