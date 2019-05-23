#include <ucontext.h>
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <new>
#include <iostream>
#include <fstream>
#include <vector>
#include <iostream>
#include <assert.h>
#include "thread.h"
#include "interrupt.h"

// Compile with: g++ -o deli thread.cc deli.cc libinterrupt.a -I. -Wall -lm -ldl -no-pie

// Set up certain helper functions

typedef void (*thread_startfunc_t) (void *);
void addToReadyQueue(ucontext_t* newContext);

ucontext_t* initControlContext();
ucontext_t* initCleanupContext();
ucontext_t* initSpawnContext();


static void control(void);
static void cleanup(void);
static void spawn(thread_startfunc_t func, void* arg);

#define STACK_SIZE 262144	/* size of each thread's stack */

// Set up some global variables
static ucontext_t* ctrl_ctx_ptr;
static ucontext_t* cur_ctx_ptr;
static ucontext_t* clnup_ctx_ptr;
int libInitCalled = 0;
int noMoreMemory = 0;

// make the queue of ucontext
static std::queue<ucontext_t*> readyQueue;

// Make vector of locks
std::vector<ucontext_t*> zombies;


/**********************************************************
************************Structures*************************
**********************************************************/
// Define Condition variable
struct CondVar{
	unsigned int value;
	std::queue<ucontext_t*> cvWait;
};

// Create Lock list
struct lock_t{
	unsigned int val;
	std::queue<ucontext_t*> waiting;
	std::vector<CondVar*> cvList;
	int isLocked;
	ucontext_t* owner;
};
/********************************************************
*********************End Structures*********************
********************************************************/






/*******************************************************
**********************Utilities************************
*******************************************************/
// Vector of all locks
std::vector<lock_t*> allLocks;

// Print queue
void printQueue(std::queue<ucontext_t*> q)
{
	if(q.empty())
	{
	}
	std::queue<ucontext_t*> test = q;
	std::queue<ucontext_t*> replacement;
	while(!test.empty())
	{
		ucontext_t* temp = test.front();
		replacement.push(temp);
		test.pop();
	}
	q = replacement;
}

// Pushes new pointer to the ready queue
void addToReadyQueue(ucontext_t* new_ctx_ptr)
{
	// Assumes that everytime a context is created it links to the control thread
	readyQueue.push(new_ctx_ptr);
}

// Gets the lock given the number, returns null if can't find
lock_t* getLock(unsigned int lock)
{
	 int size = allLocks.size();
	 for (int i = 0; i < size; i++)
	 {

	 	if(allLocks[i]->val == lock)
	 	{
	 		return allLocks[i];
	 	}
	 }
	 return NULL;
}

// Gets CV associated with lock, returns NULL otherwise
CondVar* getCV(lock_t* lock, unsigned int cv)
{
	 int cvSize = lock->cvList.size();
	 for(int j = 0; j < cvSize; j++)
	 {
	 	if(lock->cvList[j]->value == cv)
	 	{
	 		return lock->cvList[j];
	 	}
	 }
	 return NULL;
}

// Lock function
void lockWithoutInterrupts(unsigned int lock)
{
	lock_t* justMyLock = getLock(lock);

	if(justMyLock == NULL)
	{

		// Add lock if it's not already in the vector
	 	lock_t* newLock = new lock_t;
		newLock->val = lock;

	 	allLocks.push_back(newLock);
	 	justMyLock = newLock;
	}

	if(justMyLock->isLocked == 0)
	{
		justMyLock->isLocked = 1;
		justMyLock->owner = cur_ctx_ptr;
	}
	else
	{

		// Put this queue on the wait queue
		justMyLock->waiting.push(cur_ctx_ptr);
		swapcontext(cur_ctx_ptr,ctrl_ctx_ptr);
		// lockWithoutInterrupts(lock);
	}
}

// Unlock function
void unlockWithoutInterrupts(unsigned int lock)
{
	lock_t* justMyLock = getLock(lock);

	justMyLock->isLocked = 0;

	if(!justMyLock->waiting.empty())
	{
		ucontext_t* next = justMyLock->waiting.front();
		justMyLock->waiting.pop();
    justMyLock->owner = next;
    justMyLock->isLocked = 1;
		addToReadyQueue(next);
    return;
		// justMyLock->isLocked = 1;
	}
	justMyLock->owner = NULL;
}

// Main control context
ucontext_t* initControlContext()
{
	ctrl_ctx_ptr = new ucontext_t;
	char * ctrl_stack = NULL;
	try{
		ctrl_stack = new char[STACK_SIZE];
	}
	catch (std::bad_alloc& ba) {
		noMoreMemory = 1;
		return NULL;
	}
	// set up a main context that we make with func
	if(getcontext(ctrl_ctx_ptr) == -1){
		// handle error
		return NULL;
	}
	ctrl_ctx_ptr->uc_stack.ss_sp = ctrl_stack;
	ctrl_ctx_ptr->uc_stack.ss_size = STACK_SIZE;
	ctrl_ctx_ptr->uc_link = NULL;

	makecontext(ctrl_ctx_ptr,(void(*)())(control),0);
	return ctrl_ctx_ptr;
}

// Cleanup
ucontext_t* initCleanupContext()
{
	clnup_ctx_ptr = new ucontext_t;
	char * ctrl_stack = NULL;
	try {
		ctrl_stack = new char[STACK_SIZE];
	}
	catch (std::bad_alloc& ba) {
		noMoreMemory = 1;
		return NULL;
	}
	// set up a main context that we make with func
	if(getcontext(clnup_ctx_ptr) == -1){
		// handle error
		return NULL;
	}
	clnup_ctx_ptr->uc_stack.ss_sp = ctrl_stack;
	clnup_ctx_ptr->uc_stack.ss_size = STACK_SIZE;
	clnup_ctx_ptr->uc_link = NULL;

	makecontext(clnup_ctx_ptr,(void(*)())(cleanup),0);
	return clnup_ctx_ptr;
}

// Initial spawn context
ucontext_t* initSpawnContext(thread_startfunc_t func, void* arg)
{
	ucontext_t* spawn_ctx_ptr = new ucontext_t;
	char * ctrl_stack = NULL;
	try {
		ctrl_stack = new char[STACK_SIZE];
	}
	catch (std::bad_alloc& ba) {
		noMoreMemory = 1;
		return NULL;
	}
	// set up a main context that we make with func
	if(getcontext(spawn_ctx_ptr) == -1){
		// handle error
		return NULL;
	}
	spawn_ctx_ptr->uc_stack.ss_sp = ctrl_stack;
	spawn_ctx_ptr->uc_stack.ss_size = STACK_SIZE;
	spawn_ctx_ptr->uc_link = clnup_ctx_ptr;

	makecontext(spawn_ctx_ptr,(void(*)())(spawn),2,func, arg);
	return spawn_ctx_ptr;
}
/***********************************************************
**********************End Utilities*************************
***********************************************************/










/***********************************************************
*******************Thread Controller Contexts***************
***********************************************************/
// Our "Thread Control Block"
static void control(void)
{

	//Run the next one of the ready queue (if there is one)
	if(!readyQueue.empty())
	{
		// printf("Transfering control\n");
		ucontext_t* next = readyQueue.front();
		readyQueue.pop();
		cur_ctx_ptr = next;
		// interrupt_enable();
		setcontext(next);
	}
	else
	{
		//Let program die
		printf("Thread library exiting.\n");
		exit(0);

	}
	//Add logic for freeing thread if it is done.
	//DO NOT FREE IF IT WAS SIMPLY PUT ON THE WAIT QUEUE
}
// More cleanup
static void cleanup(void)
{
	interrupt_disable();
	delete cur_ctx_ptr->uc_stack.ss_sp;
	delete cur_ctx_ptr;
	setcontext(ctrl_ctx_ptr);
}

static void spawn(thread_startfunc_t func, void *arg)
{
	// printf("We arriving!");
	interrupt_enable();
	func(arg);
}
/**************************************************************
****************End Thread Controller Contexts*****************
**************************************************************/











/**************************************************************
********************Thread Library Functions*******************
**************************************************************/

// Inits thread library
extern int thread_libinit(thread_startfunc_t func, void *arg)
{
	interrupt_disable();

	if(libInitCalled != 0) {
		interrupt_enable();
		return -1;
	}
	// printf("In our libinit\n");
	libInitCalled = 1;
	ucontext_t* ctrl_ctx_ptr = initControlContext();
	ucontext_t* clnup_ctx_ptr = initCleanupContext();
	cur_ctx_ptr = initSpawnContext(func,arg);

	// printf("Cur context: %p\n",cur_ctx_ptr);
	// printf("clnup_ctx_ptr: %p\n",clnup_ctx_ptr);
	// printf("ctrl_ctx_ptr: %p\n",ctrl_ctx_ptr);
	if(!cur_ctx_ptr || !ctrl_ctx_ptr || !clnup_ctx_ptr) {
		interrupt_enable();
		return -1;
	}
	//interrupt_enable()
	setcontext(cur_ctx_ptr);

}


/*
	Assumptions:
	 - currentContext will not be NULL, that would mean the thread has ended
	Potential Errors:
	 - What if we don't have enough space in memory to make another stack?
*/

// Creates thread and transfers control to func
extern int thread_create(thread_startfunc_t func, void *arg)
{
	interrupt_disable();

	if(libInitCalled == 0) {
		interrupt_enable();
		return -1;
	}

	if(noMoreMemory == 1) {
		interrupt_enable();
		return -1;
	}

	ucontext_t* newThread = initSpawnContext(func,arg);
	if (!newThread) {
		interrupt_enable();
		return -1;
	}

	addToReadyQueue(newThread);
	interrupt_enable();
	return 0;
}




extern int thread_yield(void){
  interrupt_disable();

	if(libInitCalled == 0) {
		interrupt_enable();
		return -1;
	}

  addToReadyQueue(cur_ctx_ptr);
  swapcontext(cur_ctx_ptr,ctrl_ctx_ptr);
  interrupt_enable();
  return 0;
}


// Thread locks given the specific lock number

extern int thread_lock(unsigned int lock)
{
	interrupt_disable();

	if(libInitCalled == 0) {
		interrupt_enable();
		return -1;
	}
	lock_t* justMyLock = getLock(lock);

	if(justMyLock == NULL)
	{

		// Add lock if it's not already in the vector
	 	lock_t* newLock = new lock_t;
		newLock->val = lock;

	 	allLocks.push_back(newLock);
	 	justMyLock = newLock;
	}

	if(justMyLock->isLocked == 0)
	{
		justMyLock->isLocked = 1;
		justMyLock->owner = cur_ctx_ptr;
	}
	else
	{
		// Put this queue on the wait queue
		if(justMyLock->owner && cur_ctx_ptr == justMyLock->owner)
		{
			interrupt_enable();
			return -1;
		}
		justMyLock->waiting.push(cur_ctx_ptr);
		swapcontext(cur_ctx_ptr,ctrl_ctx_ptr);
		// lockWithoutInterrupts(lock);
	}
	interrupt_enable();
	return 0;

}


// Thread unlocks given the specific lock number

extern int thread_unlock(unsigned int lock)
{
	interrupt_disable();

	if(libInitCalled == 0) {
		interrupt_enable();
		return -1;
	}

	lock_t* justMyLock = getLock(lock);

	if(!justMyLock || justMyLock->isLocked == 0 || justMyLock->owner != cur_ctx_ptr) {
		interrupt_enable();
		return -1;
	}
	else
	{
		justMyLock->isLocked = 0;

		if(!justMyLock->waiting.empty())
		{
			ucontext_t* next = justMyLock->waiting.front();
			justMyLock->waiting.pop();
      justMyLock->owner = next;
      justMyLock->isLocked = 1;
			addToReadyQueue(next);
      interrupt_enable();
      return 0;
		}
		justMyLock->owner = NULL;
		interrupt_enable();
		return 0;
	}

}


// Thread waits on the lock and CV

extern int thread_wait(unsigned int lock, unsigned int cond)
{
	interrupt_disable();

	if(libInitCalled == 0) {
		interrupt_enable();
		return -1;
	}

  lock_t* lockStruct = getLock(lock);
  if (!lockStruct || lockStruct->owner != cur_ctx_ptr)
  {
    interrupt_enable();
    return -1;
  }
	else
	{
    unlockWithoutInterrupts(lock);
		CondVar* cv = getCV(lockStruct,cond);
		if(cv == NULL)
		{
			CondVar* newCV = new CondVar;
			newCV->value = cond;
			cv = newCV;
		}
		cv->cvWait.push(cur_ctx_ptr);
		lockStruct->cvList.push_back(cv);
		swapcontext(cur_ctx_ptr,ctrl_ctx_ptr);
	}

	lockWithoutInterrupts(lock);
	interrupt_enable();
	return 0;
}

// Thread signal based on lock, cv and puts the thread on the ready queue
extern int thread_signal(unsigned int lock, unsigned int cond)
{
	interrupt_disable();

	if(libInitCalled == 0) {
		interrupt_enable();
		return -1;
	}

	lock_t* johnLock = getLock(lock);

	if(johnLock)
	{
		CondVar* cv = getCV(johnLock,cond);
		if(cv)
		{
			if (!cv->cvWait.empty()) {
        ucontext_t* next = cv->cvWait.front();
				cv->cvWait.pop();
				addToReadyQueue(next);
			}
		}
	}

	interrupt_enable();
	return 0;
}

// Broadcast based on lock, CV, and putting them all on the ready queue
extern int thread_broadcast(unsigned int lock, unsigned int cond)
{
	interrupt_disable();

	if(libInitCalled == 0) {
		interrupt_enable();
		return -1;
	}

	lock_t* johnLock = getLock(lock);

	if(johnLock)
	{
		CondVar* cv = getCV(johnLock,cond);

		if(cv)
		{
			while(!cv->cvWait.empty())
			{
				ucontext_t* next = cv->cvWait.front();
				cv->cvWait.pop();
				addToReadyQueue(next);
			}
		}
	}
	interrupt_enable();
	return 0;
}
/*************************************************************
**********************End Thread Library*********************
*************************************************************/
