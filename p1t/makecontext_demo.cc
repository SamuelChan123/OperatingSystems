/* Demo comes from man.he.net/man3/makecontext */

#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>

static ucontext_t uctx_main, uctx_func1, uctx_func2;

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)

static void func1(void)
{
	printf("func1: started\n");
	printf("func1: swapcontext(&uctx_func1,&uctx_func2)\n");
	if(swapcontext(&uctx_func1,&uctx_func2) == -1)
	{
		handle_error("swapcontext");
	}
	printf("func1: returned\n");
}

static void func2(void)
{
	printf("func2: started\n");
	printf("func2: swapcontext(&uctx_func2,&uctx_func1)\n");
	if(swapcontext(&uctx_func2,&uctx_func1) == -1)
		handle_error("swapcontext");
	printf("func2: returned\n");
}

int main(int argc, char* argv[])
{
	char func1_stack[16384];
	char func2_stack[16384];

	if(getcontext(&uctx_func1) == -1)
	makecontext(&uctx_func2, func2, 0);

	printf("main: swapcontext(&uctx_main, &uctx_func2)\n");
	if(swapcontext(&uctx_main,&uctx_func2) == -1)
		handle_error("swapcontext");

	printf("main: exiting\n");
	exit(EXIT_SUCCESS);
}