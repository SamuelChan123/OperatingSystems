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

    thread_yield();
    while(isFull() == 0) {
      // printf("We waiting?\n");
      thread_wait(lock, makerCond);
      // printf("We woke?\n");
  }
  thread_yield();
  if(aliveCashiers > 0) {
      int diff = 1000;
      int index = 0;

      for(int i = 0; i < corkBoard.size(); i ++) {
        if(abs((corkBoard.at(i))[0] - prevOrder) < diff) {
          diff = abs((corkBoard.at(i))[0] - prevOrder);
          index = i;
        }
      }
      thread_yield();
      prevOrder = (corkBoard.at(index))[0];
      int curCashier = (corkBoard.at(index))[1];

      corkBoard.erase(corkBoard.begin() + index);

      // printf("Corkboard size after erase: %d\n",corkBoard.size());

      std::cout << "READY: cashier " << curCashier << " sandwich " << prevOrder << std::endl;
      thread_yield();
      thread_signal(lock, curCashier);
      thread_broadcast(lock, broadcastCV);
      thread_yield();
      thread_unlock(lock);
  } else {
    thread_unlock(lock);
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
      thread_yield();
      std::vector<int> entry;
      entry.push_back(sandwichNum);
      entry.push_back(myCashierNum);
      corkBoard.push_back(entry);
      thread_yield();
      // Output cashier, sandwich info to the console
      std::cout << "POSTED: cashier " << myCashierNum << " sandwich " << sandwichNum << std::endl;
      if(isFull() == 1) {
        // printf("NOTIFY FULL!\n");
        thread_signal(lock,makerCond);
      }
      thread_yield();
      // printf("WAIT(CASHIER)\n");
      thread_wait(lock, myCashierNum);
      thread_yield();

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

thread_libinit((thread_startfunc_t) create_cashier, argv);
// start_preemptions(false,true,10);
}

