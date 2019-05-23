#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include "thread.h"

// 1. libinit cannot be called twice
// 2. libinit must be called before all other threads


int child2(void* argv) {
  thread_lock(1);
  printf("I am in child2!");
  return 0;
}

int child(void* argv) {
  printf("I am in child!");
  if(thread_create((thread_startfunc_t) child2, (void*) 50) == -1) {
    std::cerr << "Thread create call failed!";
  }
  thread_create((thread_startfunc_t) child2, (void*) 50);
  return 0;
}

int big(void* argv) {
  if(thread_libinit((thread_startfunc_t) child, (void*) 50) != -1) {
    std::cerr << "Libinit must be called first!";
    return -1;
  }
  printf("I am in big!");
  thread_create((thread_startfunc_t) child, (void*) 50);
  return 0;
}

int main() {
  if(thread_lock(1) != -1) {
    std::cerr << "Libinit must be called first!";
    exit(1);
  }
  if(thread_create((thread_startfunc_t) child, (void*) 50) != -1) {
    std::cerr << "Libinit must be called first!";
    exit(1);
  }
  thread_libinit((thread_startfunc_t) big, (void*) 50);
  std::cerr << "Libinit call failed!";
  return -1;
}
