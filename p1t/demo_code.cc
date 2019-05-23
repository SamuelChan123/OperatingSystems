/* This example shows the usage of getcontext() and setcontext() */
// Retrieved from www.ibm.com

#define _XOPEN_SOURCE_EXTENDED 1
#include <stdio.h>
#include <ucontext.h>

void func(void);

int x = 0;
ucontext_t context, *cp = &context;

int main(void) {
	getcontext(cp);
	printf("I'm here!\n");
	if (!x) {
		printf("getcontext has been called\n");
		func();
	}
	else {
		printf("setcontext has been called\n");
	}
}

void func(void) {
	x++;
	setcontext(cp);
}