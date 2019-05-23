#include "thread.h"
#include <stdlib.h>
#include <stdio.h>
#include <queue>
#include <limits.h>

int MAX = 3;
std::queue<char*> list;
int listLock = 10;
int otherLock = 11;
int waitingCV2 = 16;
int waitingCV = 15;

void printing(void* argv)
{

  // printf("Printing activated!\n");
   // int arg = (int)argv;
   printf("Thread %d\n", ((char*)argv));

   thread_wait(1000,0);
   thread_lock(listLock);
   thread_lock(listLock);
   thread_yield();
   // printf("Yield 1 Thread %d\n", ((char*)argv));
   while(list.size() > MAX)
   {
    thread_yield();
    // printf("Waiting");
      thread_wait(listLock,waitingCV);
      // printf("Woken Thread %d\n", ((char*)argv));
   }
   thread_yield();
   // printf("Yield 3 Thread %d\n", ((char*)argv));

   printf("Thread %d adding to the list\n",((char*)argv));
   list.push(((char*)argv));
   thread_yield();
   // printf("Yield 4 Thread %d\n", ((char*)argv));

   thread_unlock(listLock);
   if(thread_unlock(listLock) == -1) {
     printf("correct: double unlock failed\n");
   }
   else {
     printf("incorrect: double unlock succeeded\n");
   }
   thread_wait(listLock,0);

   //thread_unlock(10);
}

void delegating(void* argv)
{
    printf("Delegating\n");
    thread_lock(listLock);
    while(list.size() > 0)
    {
        thread_yield();
        char* next = list.front();
        thread_yield();
        list.pop();
        printf("I've consumed %d\n",next);
        thread_yield();
    }
    printf("WE GET HERE!\n");
    thread_yield();
    for (int i = 0; i < 6; i++) {
      thread_yield();
      thread_signal(listLock,waitingCV);
    }
    thread_yield();
    // thread_broadcast(listLock,waitingCV);

    thread_unlock(listLock);
}

void func(void* argv[]) {
  printf("Function created!\n");
  thread_yield();
  for (int i = 0; i < 6; i++) {
  	printf("Loop: %d\n",i);
    thread_create((thread_startfunc_t)printing, (void*)i);
  }
  thread_yield();
  thread_create((thread_startfunc_t)delegating,(void*)4);
}

int main(int argc, char* argv[]) {
  if(thread_libinit((thread_startfunc_t)func, argv) == -1)
  {
    printf("We fucked up\n");
  }
  printf("We shouldn't be here!\n");
}
