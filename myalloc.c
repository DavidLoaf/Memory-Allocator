#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myalloc.h"
#include "list.h"

struct KAllocator {
	enum allocation_algorithm aalgorithm;
	int size;
	void* memory;
	// Some other data members you want, 
	// such as lists to record allocated/free memory
	struct nodeStruct* allocated_blocks;
	struct nodeStruct* free_blocks;
	int available;
	int used;
};

struct KAllocator kallocator;


void initialize_allocator(int _size, enum allocation_algorithm _aalgorithm) {
	assert(_size > 0);
	kallocator.aalgorithm = _aalgorithm;
	kallocator.size = _size;
	kallocator.memory = malloc((size_t)kallocator.size);
	// Add some other initialization
	kallocator.allocated_blocks = NULL;
	List_insertHead(&kallocator.free_blocks, List_createNode(kallocator.memory, _size)); 
	kallocator.available = _size;
	kallocator.used = 0;
}

void destroy_allocator() {
	free(kallocator.memory);
	struct nodeStruct* cur = kallocator.allocated_blocks;
	struct nodeStruct* cur2 = kallocator.free_blocks;
	struct nodeStruct* prev = NULL;
	struct nodeStruct* prev2 = NULL;
	while(cur != NULL)
	{
		prev = cur;
		cur = cur->next;
		List_deleteNode(&kallocator.allocated_blocks, prev);
		
	}
	while(cur2 != NULL)
	{   
		prev2 = cur2;
		cur2 = cur2->next;
		List_deleteNode(&kallocator.free_blocks, prev2);    
	}
	// free other dynamic allocated memory to avoid memory leak
}

void* kalloc(int _size) {
	void* ptr = NULL;

	// Allocate memory from kallocator.memory 
	// ptr = address of allocated memory
	struct nodeStruct* cur_free = kallocator.free_blocks;
	switch(kallocator.aalgorithm)
	{
		case 0:
		{
			void* min_addr = kallocator.memory + kallocator.size + 1;
			struct nodeStruct* temp = NULL;
			while(cur_free != NULL)
			{
				if (cur_free->size >= _size && cur_free->block < min_addr)
				{
					min_addr = cur_free->block;
					temp = cur_free;
				}
				cur_free = cur_free->next;
			}
			if (temp != NULL)
			{
				ptr = temp->block;
				if (temp->size > _size)
				{
					struct nodeStruct* remainder = List_createNode(temp->block + _size, temp->size - _size);
					List_insertTail(&kallocator.free_blocks, remainder);
				}
				List_deleteNode(&kallocator.free_blocks, temp);
			}
			if (ptr == NULL)
			{
				// printf("Not enough memory.\n");
				break;
			}
			
			struct nodeStruct* block = List_createNode(ptr, _size);
			List_insertTail(&kallocator.allocated_blocks, block);
			kallocator.available -= _size;
			kallocator.used += _size;
			break;
		}
		case 1:
		{
			int min = kallocator.size+1;
			struct nodeStruct* temp = NULL;
			while(cur_free != NULL)
			{
				if (cur_free->size >= _size && cur_free->size < min)
				{
					min = cur_free->size;
					temp = cur_free;
				}
				cur_free = cur_free->next;
			}
			if (temp != NULL)
			{
				ptr = temp->block;
				if (temp->size > _size)
				{
					struct nodeStruct* remainder = List_createNode(temp->block + _size, temp->size - _size);
					List_insertTail(&kallocator.free_blocks, remainder);
				}
				List_deleteNode(&kallocator.free_blocks, temp);
			}
			if (ptr == NULL)
			{
				// printf("Not enough memory.\n");
				break;
			}
			struct nodeStruct* block = List_createNode(ptr, _size);
			List_insertTail(&kallocator.allocated_blocks, block);
			kallocator.available -= _size;
			kallocator.used += _size;
			break;
		}
		case 2:
		{
			int max = 0;
			struct nodeStruct* temp = NULL;
			while(cur_free != NULL)
			{
				if (cur_free->size >= _size && cur_free->size > max)
				{
					max = cur_free->size;
					temp = cur_free;
				}
				cur_free = cur_free->next;
			}
			if (temp != NULL)
			{
				ptr = temp->block;
				if (temp->size > _size)
				{
					struct nodeStruct* remainder = List_createNode(temp->block + _size, temp->size - _size);
					List_insertTail(&kallocator.free_blocks, remainder);
				}
				List_deleteNode(&kallocator.free_blocks, temp);
			}
			if (ptr == NULL)
			{
				// printf("Not enough memory.\n");
				break;
			}
			struct nodeStruct* block = List_createNode(ptr, _size);
			List_insertTail(&kallocator.allocated_blocks, block);
			kallocator.available -= _size;
			kallocator.used += _size;
			break;
		}
		default:
		// printf("WTF\n");
		break;
	}
	return ptr;
}

void kfree(void* _ptr) {
    assert(_ptr != NULL);
    struct nodeStruct* target = List_findNode(kallocator.allocated_blocks, _ptr);
    struct nodeStruct* free_block = List_createNode(target->block, target->size);
    struct nodeStruct* cur = kallocator.free_blocks;
    struct nodeStruct* merged = List_createNode(NULL, 0);
    struct nodeStruct* garbage[4] = {NULL};
    while(cur != NULL)
    {
        if ((cur->block + cur->size) == target->block && merged->block == NULL)
        {
            // printf("left = %p\n", cur->block);
            merged->block = cur->block;
            merged->size = cur->size + target->size;
            garbage[0] = cur;
        } else if ((cur->block + cur->size) == target->block && merged->block != NULL)
        {
            // printf("left = %p right = %p\n", cur->block, merged->block);
            merged->block = cur->block;
            merged->size += cur->size;
            garbage[1] = cur;
        }
        if ((target->block + target->size) == cur->block && merged->block == NULL)
        {
            // printf("right = %p\n", cur->block);
            merged->block = target->block;
            merged->size = target->size + cur->size;
            garbage[2] = cur;
        } else if ((target->block + target->size) == cur->block && merged->block != NULL){
            // printf("right = %p left = %p\n", cur->block, merged->block);
            merged->size += cur->size;
            garbage[3] = cur;
        }
        cur = cur->next;
    }

    for (int i = 0; i < 4; ++i)
    {
        if (garbage[i] != NULL)
        {
            List_deleteNode(&kallocator.free_blocks, garbage[i]);
        }
    }
    if (merged->block != NULL)
    {
        List_insertTail(&kallocator.free_blocks, merged);
        free(free_block);
    } else
    {
        List_insertTail(&kallocator.free_blocks, free_block);
        free(merged);
    }
    kallocator.available += target->size;
    kallocator.used -= target->size;
    List_deleteNode(&kallocator.allocated_blocks, target);
}

int compact_allocation(void** _before, void** _after) {
  int compacted_size = 0;
  int numalloc = 0;
  int numfree = 0;

/////////////////////////////////////// PRE ALLOC ////////////////////////////////////////////
  struct nodeStruct* preAlloc = kallocator.allocated_blocks;
  struct nodeStruct* preFree = kallocator.free_blocks;
  // printf("MEMEORY: %p\n", kallocator.memory);
  int a = 0;
  while(preAlloc != NULL)
  {
   // printf("PRE ALLOC Block: %p Node size: %d Value: %d\n", preAlloc->block, preAlloc->size,*(int*)(preAlloc->block));
   _before[a] = preAlloc->block;
   preAlloc = preAlloc->next;
   a++;
  }
  while(preFree != NULL)
  {
	_before[a] = preFree->block;
   // printf("PRE FREE Block: %p Node size: %d Value: %d\n", preFree->block, preFree->size,*(int*)(preFree->block));
   preFree = preFree->next;
   a++;
  }
/////////////////////////////////////// PRE ALLOC ////////////////////////////////////////////
  struct nodeStruct* curAlloc = kallocator.allocated_blocks;
  struct nodeStruct* curFree = kallocator.free_blocks;
  void* temp = NULL;

  int totalFree = 0; // finding the total size of free
  void* end = NULL;

  	if(kallocator.used != 0)
  	{
  		// printf("WTF BRUH %d\n", *(int*)(curAlloc->block));
  		memmove(kallocator.memory,curAlloc->block, curAlloc->size);
  		
  		curAlloc->block = kallocator.memory; // move the allocated block to the first address in memory
  		
		// printf("BEGINNING ADDRESS: %p\n", curAlloc->block);

  	
  		if (curAlloc->next == NULL) // if there is only one allocated block
  		{
	  		end = curAlloc->block + curAlloc->size;
			memmove(end,curFree->block,curFree->size);
	   		//printf("curAlloc->block: %p\ncurAlloc->size: %d\n", curAlloc->block, curAlloc->size);
			//printf("END Allocated Block: %p\n", end);
			
			curFree->block = end;
			
			temp = end;
			//printf("WTF BRUH: %p\n", curFree->block);
	  	}
	  	else // if there is more than 1 allocated block
	  	{
	  		//printf("bruh why %d\n", kallocator.available);
			while(curAlloc->next != NULL) // moves together the allocated array
		  	{
				//printf("Allocated Block: %p Node size: %d\n", curAlloc->block, curAlloc->size);
				memmove(curAlloc->block + curAlloc->size, curAlloc->next->block, curAlloc->size);
				// printf("ADDRESS: %p VALUE: %d\n", curAlloc->block,*(int*)(curAlloc->block));
				curAlloc->next->block = curAlloc->block + curAlloc->size; // the next address block 
				
				// printf("curAlloc->block + curAlloc->size: %p curAlloc->next->block: %p\n", curAlloc->block + curAlloc->size,curAlloc->next->block);

				end = curAlloc->block + curAlloc->size; // keeps the end position of the alloc block
				// printf("old curAlloc: %p\n", curAlloc->block);
				curAlloc = curAlloc->next;
				// printf("new curAlloc: %p\n", curAlloc->block);
				numalloc++;

		  	}
		  	if(kallocator.available != 0) //if there is free space in the memory
		  	{
		  		memmove(end + curAlloc->size, curFree->block, curFree->size);
		  		curFree->block = end + curAlloc->size;
		  		
		  		temp = end + curAlloc->size;
			}
	  	}

  		numalloc++;
	}
	else
	{	  
		temp = curFree->block; // if there is nothing in alloc, set temp to the beginning of address
	}
  	//  printf("%d\n", numalloc);
  	// printf("FREE Block: %p Node size: %d\n", curFree->block, curFree->size);
  	// printf("TEMP WTF BRO: %p\n", temp);
  	if(kallocator.available != 0)
  	{
	  	while(curFree->next != NULL) // moves together the free array
	  	{
			// printf("FREE Block: %p Node size: %d\n", curFree->block, curFree->size);

			memmove(curFree->block + curFree->size, curFree->next->block, curFree->size);
			curFree->next->block = curFree->block + curFree->size;
			
			curFree = curFree->next;
	  	}

	  struct nodeStruct* endFree = kallocator.free_blocks;

	  while(endFree != NULL) //calculates the total size of free array//
	  {
	   // printf("FREE Block Node size: %d\n", endFree->size);
	   totalFree = endFree->size + totalFree;
	   endFree = endFree->next;
	  }

	  struct nodeStruct* deleteFree = kallocator.free_blocks;
	  struct nodeStruct* freedom = kallocator.free_blocks;
	  while (deleteFree != NULL) //deletes nodes to put it all together
	  {
		freedom = deleteFree;
		deleteFree = deleteFree->next;
	   	List_deleteNode(&kallocator.free_blocks, freedom);

	  }
	  List_insertHead(&kallocator.free_blocks, List_createNode(temp,totalFree)); //create the new node
  	}
  numfree++;

/////////////////////////////// PRINT TEST /////////////////////////////////////
  struct nodeStruct* endAlloc2 = kallocator.allocated_blocks;
  struct nodeStruct* endFree2 = kallocator.free_blocks;
  int b = 0;
  while(endAlloc2 != NULL)
  {
  	// printf("TEST ALLOC Block: %p Node size: %d\n", endAlloc2->block, endAlloc2->size);
  	_after[b] = endAlloc2->block;
  	// memmove(_after[b],endAlloc2->block, endAlloc2->size);
  	// printf("%d\n", *(int*)(_after+b));

	//printf("%p\n", _after[b]);
   endAlloc2 = endAlloc2->next;
   b++;
  }
  while(endFree2 != NULL)
  {
  	// printf("TEST FREE Block: %p Node size: %d\n", endFree2->block, endFree2->size);
  	_after[b] = endFree2->block;
  	// memmove(_after[b],endFree2->block, endFree2->size);
  	// printf("%d\n", *(int*)(_after+b));
   endFree2 = endFree2->next;
   b++;
  }
/////////////////////////////// PRINT TEST /////////////////////////////////////
  compacted_size = numalloc/numfree;
  return compacted_size;
}

int available_memory() {
	int available_memory_size = 0;
	// Calculate available memory size
	available_memory_size = kallocator.available;
	return available_memory_size;
}

void print_statistics() {
	int allocated_size = 0;
	int allocated_chunks = 0;
	int free_size = 0;
	int free_chunks = 0;
	int smallest_free_chunk_size = kallocator.size;
	int largest_free_chunk_size = 0;

	// Calculate the statistics
	// struct nodeStruct* XD = kallocator.allocated_blocks; // allocated blocks
	// while(XD != NULL)
	// {
	//     printf("Block: %p Node size: %d Value: %d\n", XD->block, XD->size, *(int*)(XD->block));
	//     XD = XD->next;
	// }	

	// struct nodeStruct* lol = kallocator.free_blocks; // free blocks
	// while(lol != NULL)
	// {
	//     printf("Block: %p Node size: %d\n", lol->block, lol->size);
	//     lol = lol->next;
	// }
	allocated_size = kallocator.used;
	allocated_chunks = List_countNodes(kallocator.allocated_blocks);
	free_size = kallocator.available;
	free_chunks = List_countNodes(kallocator.free_blocks);
	struct nodeStruct* cur = kallocator.free_blocks;
	if (kallocator.free_blocks == NULL)
	{
		smallest_free_chunk_size = 0;
	}
	while(cur != NULL)
	{
		if (cur->size < smallest_free_chunk_size)
		{
			smallest_free_chunk_size = cur->size;
		}
		if (cur->size > largest_free_chunk_size)
		{
			largest_free_chunk_size = cur->size;
		}
		cur = cur->next;
	}

	printf("Allocated size = %d\n", allocated_size);
	printf("Allocated chunks = %d\n", allocated_chunks);
	printf("Free size = %d\n", free_size);
	printf("Free chunks = %d\n", free_chunks);
	printf("Largest free chunk size = %d\n", largest_free_chunk_size);
	printf("Smallest free chunk size = %d\n", smallest_free_chunk_size);
}

// valgrind --leak-check=full --show-leak-kinds=all --num-callers=20 ./kallocation
