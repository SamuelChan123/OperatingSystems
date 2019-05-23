#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <queue>

static ucontext_t a, b, c;

std::vector<ucontext_t> v;

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)


void func1(void)
{
   printf("func1: started\n");
   printf("func1: swapcontext(&v[1], &v[2])\n");
   if (swapcontext(&v[1], &v[2]) == -1)
       handle_error("swapcontext");
   printf("func1: returning\n");
}


void func2(void)
{
   printf("func2: started\n");
   printf("func2: swapcontext(&v[2], &v[1])\n");
   if (swapcontext(&v[2], &v[1]) == -1)
       handle_error("swapcontext");
   printf("func2: returning\n");
}


int main(int argc, char *argv[])
{
  // for(int i = 0; i < 3; i++) {
  //   // ucontext_t u;
  //   v.push_back(new ucontext_t());
  // }
  v.push_back(a);
  v.push_back(b);
  v.push_back(c);
   char func1_stack[16384];
   char func2_stack[16384];

   if(getcontext(&v[1]) == -1)
      handle_error("swapcontext");
   v[1].uc_stack.ss_sp = func1_stack;
   v[1].uc_stack.ss_size = sizeof(func1_stack);
   v[1].uc_link = &v[0];
   makecontext(&v[1], func1, 0);

   if (getcontext(&v[2]) == -1)
      handle_error("getcontext");
    v[2].uc_stack.ss_sp = func2_stack;
    v[2].uc_stack.ss_size = sizeof(func2_stack);
    v[2].uc_link = (argc > 1) ? NULL : &v[1];
   makecontext(&v[2], func2, 0);

   printf("main: swapcontext(&uctx_main, &v[2])\n");
   if (swapcontext(&v[0], &v[2]) == -1)
       handle_error("swapcontext");

   printf("main: exiting\n");
   exit(EXIT_SUCCESS);
}