#include <ucontext.h>
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <fstream>
#include <iostream>


// make the queue of ucontext
static ucontext_t uctx_main;
static std::queue<ucontext_t*> readyQueue;

int thread_libinit(thread_startfunc_t func, void *arg) 
{
	// set up a main context that we make with func
	if(getcontext(&uctx_main) == -1)
		// handle error
	uctx_main.uc_link = NULL;
	// How do I count the numnber of args in *arg?
	int argc = 0;
	makecontext(&uctx_main,func,argc,arg);
	// call setContext

}

/*
	Potential Errors:
	 - What if we don't have enough space in memory to make another stack?
*/
extern int thread_create(thread_startfunc_t func, void *arg) 
{
	// make a new context
	// Set up stack and stuff

	// makecontext(pair new context with func)
	// add new context to queue
}


extern int thread_yield(void){
	//create a new ucontext_t
	//ucontext yieldTo = readyQueue.front();
	//readyQueue.pop();
	//How to get pointer of current context?
	//swapcontext(std::this_thread::get_id(),yieldTo);

}
extern int thread_lock(unsigned int lock)
{

}

extern int thread_unlock(unsigned int lock)
{

}

extern int thread_wait(unsigned int lock, unsigned int cond)
{

}

extern int thread_signal(unsigned int lock, unsigned int cond)
{

}

extern int thread_broadcast(unsigned int lock, unsigned int cond)
{

}

void printQueue(std::queue<ucontext_t*> q)
{
	std::queue<ucontext_t*> test = q;
	std::queue<ucontext_t*> replacement;
	while(!test.empty())
	{
		ucontext_t* temp = test.front();
		printf("%d\n",temp);
		replacement.push(temp);
		test.pop();
	}
	q = replacement;
}

int main()
{
	ucontext_t* test1 = new ucontext_t;
	printf("test1: %d",test1);
	ucontext_t* test2 = new ucontext_t;
	printf("\ntest2: %d",test2);
	readyQueue.push(test1);
	readyQueue.push(test2);
	printf("\n");
	printQueue(readyQueue);

	
}