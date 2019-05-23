#include <ucontext.h>
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <fstream>
#include <iostream>
#include "thread.h"


void doSomething(void* argv){
	printf("Success!\n");
	// argv = (int*)argv;
	thread_unlock(10);
	printf("Integer: %d\n",*((char*)argv));
}

int main(int argc, char* argv[]) {

	//char** arg = new char*[1];
	//char c = 'd';
	//char* some = &c;
	//arg[0] = some;

	printf("Argv is: %d\n",*(char*)argv[0]);
	printf("Trying libint\n");
	thread_libinit((thread_startfunc_t) doSomething, argv[0]);
	printf("Failure. Try again.\n");
}
