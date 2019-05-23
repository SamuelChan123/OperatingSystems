#include "thread.h"
#include <stdlib.h>
#include <stdio.h>
#include <queue>

int MAX = 3;
std::queue<char*> list;
std::queue<char*> bList;
std::queue<char*> clist;
int counter = 0;
int counterLock = 11;
int listLock = 10;
int clistLock = 11;
int waitingCV = 15;

void printing(void* argv) 
{

  // printf("Printing activated!\n");
   // int arg = (int)argv;
   printf("Thread %d\n", ((char*)argv));

   thread_lock(listLock);
   thread_yield();
   // printf("Yield 1 Thread %d\n", ((char*)argv));
   while(list.size() > MAX)
   {
    thread_yield();
    // printf("Yield 2 Thread %d\n", ((char*)argv));
      thread_wait(listLock,waitingCV);
      // printf("Woken Thread %d\n", ((char*)argv));
   }
   thread_yield();

   thread_unlock(counterLock);

   thread_lock(counterLock);

   counter = counter + 1;

   bList.push(((char*)argv));
   printf("Thread %d to bList\n",((char*)argv));
   
   thread_unlock(counterLock);
   // printf("Yield 3 Thread %d\n", ((char*)argv));

   printf("Thread %d adding to the list\n",((char*)argv));
   list.push(((char*)argv));
   thread_yield();
   // printf("Yield 4 Thread %d\n", ((char*)argv));

   thread_unlock(listLock);


   printf("cThread %d\n", ((char*)argv));

   thread_lock(clistLock);
   thread_yield();
   // printf("Yield 1 Thread %d\n", ((char*)argv));
   while(clist.size() > MAX)
   {
    thread_yield();
    // printf("Yield 2 Thread %d\n", ((char*)argv));
      thread_wait(clistLock,waitingCV);
      // printf("Woken Thread %d\n", ((char*)argv));
   }
   thread_yield();

   printf("cThread %d adding to the clist\n",((char*)argv));
   clist.push(((char*)argv));
   thread_yield();
   // printf("Yield 4 Thread %d\n", ((char*)argv));

   thread_unlock(clistLock);


   //thread_unlock(10);
}

void delegating(void* argv)
{
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
    thread_unlock(listLock);
    thread_lock(clistLock);
    while(clist.size() > 0)
    {
      thread_yield();
      char* next = clist.front();
      clist.pop();
      printf("I've cccsssssonsumed %d\n",next);
        
    }
    printf("WE GET HERE!\n");
    thread_yield();
    for (int i = 0; i < 6; i++) {
      thread_yield();
      thread_signal(listLock,waitingCV);
    }
    thread_yield();
    thread_broadcast(clistLock,waitingCV);

    thread_unlock(clistLock);
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
