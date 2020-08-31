#include <stdio.h>  // needed for size_t
#include <sys/mman.h> // needed for mmap
#include <assert.h> // needed for asserts
#include "dmm.h"

/* You can improve the below metadata structure using the concepts from Bryant
 * and OHallaron book (chapter 9).
 */

typedef struct metadata {
  /* size_t is the return type of the sizeof operator. Since the size of an
   * object depends on the architecture and its implementation, size_t is used
   * to represent the maximum size of any object in the particular
   * implementation. size contains the size of the data object or the number of
   * free bytes
   */
  size_t size;
  struct metadata* next;
  struct metadata* prev;
  
} metadata_t;

metadata_t prologue = {0,NULL,NULL};//,0};
metadata_t epilogue = {0,NULL,NULL};//,0};

/* freelist maintains all the blocks which are not in use; freelist is kept
 * sorted to improve coalescing efficiency 
 */

static void* heap_region = NULL;
static metadata_t* freelist = NULL;

void splitBlock(metadata_t* block, metadata_t* leftoverLocation, size_t numbytes);
void coalesceList();
void* run_dmalloc(size_t numbytes);
bool isAllocated(metadata_t* block);

void* dmalloc(size_t numbytes) {

  return run_dmalloc(numbytes);
  // First loop failed: Coalesce and try again
}

bool isAllocated(metadata_t* block)
{
  if((block->size & 1) == 1)
      return true;
  return false;
}

void* run_dmalloc(size_t numbytes)
{
  numbytes = ALIGN(numbytes);

  if(freelist == NULL) {      
      if(!dmalloc_init())
        return NULL;
  }

  assert(numbytes > 0);

  //Testing how big the struct is: printf("Size of metadata_t: %lssd", sizeof(metadata_t));
  /* your code here */
  // Find Block to Split
  metadata_t* current = prologue.next;
  int offset = 0; // Don't ccount for the prologue header, its stored in global data
  while(current != &epilogue) // Loop through the free list
  {
    offset += METADATA_T_ALIGNED; // Account for current block header
    if(!isAllocated(current) && (current->size >> 1) >= numbytes + METADATA_T_ALIGNED)
    {
      //SPLIT
      splitBlock(current,(metadata_t*)(heap_region + offset + numbytes),numbytes);

      return heap_region + offset;
    } //ELSE

    offset += (current->size >> 1); // Account for block
    current = current->next;
  } 

  return NULL;
}

void splitBlock(metadata_t* block, metadata_t* leftoverLocation, size_t numbytes)
{
  //SPLIT
    metadata_t* current = block;
      int leftoverSize = (current->size >> 1) - numbytes - METADATA_T_ALIGNED; // We know this is not negative by the condition above

     if(leftoverSize > METADATA_T_ALIGNED + ALIGN(1)) // There is enough space to store a header do so
     {
        metadata_t* leftover = leftoverLocation;
        leftover->next = current->next;
        leftover->prev = current;
        leftover->size = (leftoverSize << 1 ) + 0;


        current->next = leftover;
        current->size = (numbytes << 1) + 1;
      }
      else
      {
        current->size = ((numbytes+leftoverSize) << 1) + 1;
      }
      // current->prev is already set
}

void coalesceList()
{
  metadata_t* current = prologue.next;
  while(current->next != &epilogue)
  {
    if(!isAllocated(current) && !isAllocated(current->next))
    {
      current->next->prev = NULL;
      current->size = ((current->size >> 1) + (current->next->size >> 1) + METADATA_T_ALIGNED) << 1;
      current->next->next->prev = current;
      current->next = current->next->next;
    }
    else
    {
      current = current->next;
    }
  }
}

void dfree(void* ptr) 
{
  /* your code here */
  // ptr - heap_pointer = offset
  // pointerlocation = offset - METADATA_T_ALIGNED (header)
  metadata_t* block = (metadata_t*)(ptr - METADATA_T_ALIGNED);

  if(block == NULL)
      return;

  metadata_t* current = prologue.next;
  int ptrOffset = ptr - heap_region;
  int offset = 0;
  while(current != &epilogue)
  {
    offset += METADATA_T_ALIGNED;

    if(offset == ptrOffset && isAllocated(current))
    {
       current->size = current->size - 1;
       break;
    }

    offset += (current->size >> 1);
    current = current->next;
  }

  coalesceList();
}


bool dmalloc_init() {
  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);

  freelist = (metadata_t*) mmap(NULL, max_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  heap_region = (void*)freelist;
  if (freelist == (void *)-1)
    return false;

  freelist->next = &epilogue;
  freelist->prev = &prologue;
  freelist->size = (max_bytes-METADATA_T_ALIGNED) << 1;

  // Set up prologue and epilogue
  prologue.next = freelist; 
  epilogue.prev = freelist;

  freelist = freelist->prev; // Set Prologue as the beginning of the list

  print_freelist();

  return true;
}

/* for debugging; can be turned off through -NDEBUG flag*/
void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL) {
    DEBUG("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p\t",
    freelist_head->size >> 1,
    freelist_head,
    freelist_head->prev,
    freelist_head->next);
    printf("\tFreelist Size:%zd Allocated: %ld  Head:%p, Prev:%p, Next:%p\n",
    freelist_head->size >> 1,
    freelist_head->size & 1,
    freelist_head,
    freelist_head->prev,
    freelist_head->next);
    freelist_head = freelist_head->next;
  }
  DEBUG("\n");
}
