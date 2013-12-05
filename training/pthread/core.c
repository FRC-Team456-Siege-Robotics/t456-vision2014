
#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"


void *Process(void *data)
{
     long hdata = (long)data;
     long id = (long)data;
     int  i, j;

     for (i=0;i<10000000;i++)
     {
         printf("Process %ld says Hello World\n",id);
         for (j = 0; j < 100000; j++)
         {
               hdata *= j;
               hdata *= j;
               hdata *= j;
               hdata *= j;
               hdata *= j;
               hdata *= j;
               hdata *= j;
               hdata *= j;
               hdata *= j;
               hdata *= j;
               hdata *= j;
         }
    }

    return (void*)hdata;
}


int main(int argc, char* argv[])
{
   int numCores = 1;
   int i;

   if (argc>1)
      numCores = strtol(&argv[1][0], NULL, 10);

   pthread_t *thread_ids = (pthread_t *)malloc(numCores*sizeof(pthread_t));

   for (i=0;i<numCores;i++)
   {
      pthread_create(&thread_ids[i],NULL,Process,(void *)i);
   }

   for (i=0;i<numCores;i++)
   {
      pthread_join(thread_ids[i],NULL);
   }

    return 0;
}
