#include "thread.h"
#include <stdlib.h>
#include <stdio.h>

void printing(void* argv) {

   printf("Printing activated!\n");
   // int arg = (int)argv;
   printf("Thread %d\n", ((char*)argv));
}

void func(void* argv[]) {
  printf("Function created!\n");
  for (int i = 0; i < 6; i++) {
  	printf("Loop: %d\n",i);
    thread_create((thread_startfunc_t)printing, (void*)i);
  }
}

int main(int argc, char* argv[]) {
  thread_libinit((thread_startfunc_t)func, argv);
  printf("We shouldn't be here!\n");
}
  