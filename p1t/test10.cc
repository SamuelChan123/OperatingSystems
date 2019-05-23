
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


void printing(void* argv) {

   printf("Printing activated!\n");
   // int arg = (int)argv;
   printf("Thread %d\n", ((char*)argv));
}

void func(void* argv[]) {
  printf("Function created!\n");
  for (int i = 0; i < 1000000; i++) {
  	printf("Loop: %d\n",i);
    printf("Created? %d\n",thread_create((thread_startfunc_t)printing, (void*)i));
  }
}

int main(int argc, char* argv[]) {
  thread_libinit((thread_startfunc_t)func, argv);
  printf("We shouldn't be here!\n");
}