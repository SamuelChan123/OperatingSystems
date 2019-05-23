#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <fstream>
#include <iostream>
// #include <array>
#include <thread.h>

// What this file will do:
  // Passes inputs and outputs
  // Create cashier threads
  // Sets up Cork board
  // Create maker thread
  // Global states; # of live threads, corkboard, console?
/*
Thread library functions for Referencesdfsdf

extern int thread_libinit(thread_startfunc_t func, void *arg);
extern int thread_create(thread_startfunc_t func, void *arg);
extern int thread_yield(void);
extern int thread_lock(unsigned int lock);
extern int thread_unlock(unsigned int lock);
extern int thread_wait(unsigned int lock, unsigned int cond);
extern int thread_signal(unsigned int lock, unsigned int cond);
extern int thread_broadcast(unsigned int lock, unsigned int cond);
*/

// Declare Functions
// void create_cashier(void *argv);
// void maker();
// int isFull();
// void cashier(void* argv);

// Declare global variables
int corkSize = 0;
int numofCashiers = 0;
int aliveCashiers = 0;
std::vector<std::vector<int> > corkBoard;
//std::queue<int> orderQueue;
int cashierNum = 0;
int prevOrder = -1;
int lock = 3000;
int makerCond = 5000;
int broadcastCV = 6000;

////////////////////////////////////////////////////////////////////

int isFull() {
  if(corkBoard.size() < corkSize) {
    return 0;
  }
  // if(corkBoard.size() != aliveCashiers) {
  //   return 0;
  // }
  return 1;
}

////////////////////////////////////////////////////////////////////

void maker(void* argv) {
  // printf("maker Create**************************************8888\n");
  while(aliveCashiers > 0) {
    // printf("So we winnin?\n");
    // printf("Alive cashiers: %d\n",aliveCashiers);
    thread_lock(lock);

    while(isFull() == 0) {
      // printf("We waiting?\n");
      thread_wait(lock, makerCond);
      // printf("We woke?\n");
  }

  if(aliveCashiers > 0) {
      int diff = 1000;
      int index = 0;

      for(int i = 0; i < corkBoard.size(); i ++) {
        if(abs((corkBoard.at(i))[0] - prevOrder) < diff) {
          diff = abs((corkBoard.at(i))[0] - prevOrder);
          index = i;
        }
      }

      prevOrder = (corkBoard.at(index))[0];
      int curCashier = (corkBoard.at(index))[1];

      corkBoard.erase(corkBoard.begin() + index);

      // printf("Corkboard size after erase: %d\n",corkBoard.size());

      std::cout << "READY: cashier " << curCashier << " sandwich " << prevOrder << std::endl;

      thread_signal(lock, curCashier);
      thread_broadcast(lock, broadcastCV);

      thread_unlock(lock);
  } else {
    break;
  }
  }

}


////////////////////////////////////////////////////////////////////

void cashier(void* argv) {
  // Set filename to argv
  char* fileName = (char*) argv;
  // printf("Casher thread created\n");

  // Get cashier num, set my cashier num to it, increment by 1 to set up for next
  int myCashierNum = cashierNum;
  cashierNum++;

  //Increment aliveCashiers by 1
  // aliveCashiers++;
  // Get size of file
  std::ifstream countStream(fileName, std::ios_base::in);
  int size = 0;
  int sandwichNum;
  while(countStream >> sandwichNum)
  {
    size++;
    // printf("Next order: %d", sandwichNum);
  }
  countStream.close();

  // Printing sandwich list
  //int* sandwichList = (int*)(malloc(sizeof(size)));
  //int count = 0;
  //sandwichList[count] = a;
  //count++;

  // Reinitialize to -1
  sandwichNum = -1;

  // Iterate through each element of the file
  std::ifstream input(fileName, std::ios_base::in);
  while (input >> sandwichNum)
  {
    thread_lock(lock);

    // While corkboard is full
    while(corkBoard.size() == corkSize) {
      //orderQueue.push(sandwichNum);
      // printf("We waiting now boiiiis\n");
      // thread_signal(lock, makerCond);
      thread_wait(lock, broadcastCV);
      // printf("STAY WOKE!!\n");
    }
            // printf("STAY WOKE!!\n");

      // std::array<int, 2> entry = {sandwichNum, myCashierNum};
      std::vector<int> entry;
      //int entry[2];
      //entry[0] = sandwichNum;
      //entry[1] = myCashierNum;
      entry.push_back(sandwichNum);
      entry.push_back(myCashierNum);
      corkBoard.push_back(entry);
      // Output cashier, sandwich info to the console
      std::cout << "POSTED: cashier " << myCashierNum << " sandwich " << sandwichNum << std::endl;
      if(isFull() == 1) {
        // printf("NOTIFY FULL!\n");
        thread_signal(lock,makerCond);
      }
      // printf("WAIT(CASHIER)\n");
      thread_wait(lock, myCashierNum);

      thread_unlock(lock);
    }
    // printf("cashier %d checkpoint 3 \n",cashierNum);
    // Kill thread and decrement aliveCashiers
    aliveCashiers--;
    if (aliveCashiers < corkSize) {
      corkSize--;
    }
    if(corkBoard.size() == corkSize) {
      thread_signal(lock,makerCond);
    }
    // printf("Decremented to: %d\n",aliveCashiers);
    // printf("Cork size: %d\n",corkSize);
 }

////////////////////////////////////////////////////////////////////
void floop(void* argv)
{
  printf("Scoopity poop \n");
}

// void create_cashier(void* argv[]) {

//     // for each cashier, thread_create((thread_startfunc_t) cashier, void *arg);
//     //parse list for files for each cashier, set arg array to that file
//     for(int j = 2; j < 2 + numofCashiers; j++)
//     {
//       // char* fileName = (char*)(argv + j);
//       printf("Create cashier thread %d.\n",j-1);
//       thread_create((thread_startfunc_t) cashier, argv[j]);
//       printf("That was %d. Onto the next one!\n",j-1);
//     }
//     printf("Making new cashier\n");
//     thread_create((thread_startfunc_t) floop, argv[1]);
//     printf("New cashier made\n");
// thread_create((thread_startfunc_t) maker, argv[2 + numofCashiers]);
//     printf("I'm a dumbass \n");
// // printf("We in here now boi\n");
// // Call maker function

// }


// int main(int argc, char* argv[]) {


//   // set size of corkboard = argv[1]
//   // set num of cashiers and hence alive cashiers = argc - 2

// corkSize = atoi(argv[1]);
// numofCashiers = argc-2;
// aliveCashiers = argc-2;

// //corkBoard = create arraylist of 2 element arrays;
// //corkBoard = malloc(corkSize * sizeOf(int));
// // pass argv into thread init
// thread_libinit((thread_startfunc_t) create_cashier, argv);
// start_preemptions(false,true,10);

void create_cashier(void* argv[]) {

    // for each cashier, thread_create((thread_startfunc_t) cashier, void *arg);
    //parse list for files for each cashier, set arg array to that file
    for(int j = 2; j < 2 + numofCashiers; j++)
    {
      // char* fileName = (char*)(argv + j);
      thread_create((thread_startfunc_t) cashier, argv[j]);
    }

// printf("We in here now boi\n");
// Call maker function
thread_create((thread_startfunc_t) maker, argv[2 + numofCashiers]);

}


int main(int argc, char* argv[]) {


  // set size of corkboard = argv[1]
  // set num of cashiers and hence alive cashiers = argc - 2

corkSize = atoi(argv[1]);
numofCashiers = argc-2;
aliveCashiers = argc-2;

//corkBoard = create arraylist of 2 element arrays;
//corkBoard = malloc(corkSize * sizeOf(int));
// pass argv into thread init
thread_libinit((thread_startfunc_t) create_cashier, argv);
// start_preemptions(false,true,10);

}

////////////////////////////////////////////////////////////////////



/******************************************
************Cashier thread*****************
******************************************/
// void cashier() {
//  //spawn thread for this cashier (or should main do this?)
//  thread_create(pointerToThisFunctionOnStack?,arrayOfOrders);

//  //Lock the data
//  thread_lock(SomeInteger);

//  //Check the posted order board to see if there is space
//  while(corkboard.size == max_orders) {
//    thread_wait(SomeInteger,hasSpace);//Wait until lock is free if it isn't
//  }

//  corkboard.add(order);//Add your order once woken up

//  //Output to the console that the sandwich order has been posted
//  cout << "READY: cashier" << cashier << " sandwich" << sandwich << end1;

//  thread_signal(SomeInteger,corkboardIsNotEmpty);//Notify sandwich maker that board is not empty

//  thread_unlock(SomeInteger);
// }

/******************************************
**************Maker thread*****************
******************************************/
// void maker(void *arg) {
//  thread_lock(SomeInteger);

//  if (aliveCashiers != 0) {

//    //Always retrieve the first sandwich in the corkboar list
//    //assuming that it is always sorted and so we can always choose
//    //first entry but then we have to shift the list one back

//    while(vectorIsntFull || vector.size != aliveCashiers) {
//      thread.wait(SomeInteger,full);
//    }

//    //find closest sandwich using prevOrder
//    //and then update prevorder
//    for() {
//    }

//    //Get the next cashier that can post
//    queue.pop();

//    //remove order from list
//    vector.erase();

//    //
//    thread_signal(lock,corkboardHasSpace);
//    thread_signal(lock,orderComplete);

//    thread_unlock(SomeInteger);dgfgf
//  }

// }
