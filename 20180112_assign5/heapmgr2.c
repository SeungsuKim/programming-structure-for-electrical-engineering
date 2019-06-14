/*--------------------------------------------------------------------*/
/* heapmgr2.c                                                         */
/* Author: Seungsu Kim                                                */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "chunk.h"

#define FALSE 0
#define TRUE  1

#define SIZE_OF_BINS 1024
#define POINTER_UNIT 8

enum {
	MEMALLOC_MIN = 1024,
};

/* gHeapStart, gHeapEnd: start and end of the heap area.
 * gHeapEnd will move if you increase the heap */
static void *gHeapStart = NULL, *gHeapEnd = NULL;

static Chunk_T bins[SIZE_OF_BINS] = {NULL};

#ifndef NDEBUG
/*--------------------------------------------------------------------*/
/* CheckHeapValidity:
 * Validity check for entire data structures for chunks.
  * Returns 1 on success or 0 (zero) on failure.
  */
/*--------------------------------------------------------------------*/
static int
CheckHeapValidity(void)
{
	Chunk_T w;
	
	if (gHeapStart == NULL) {
		fprintf(stderr, "Uninitialized heap start\n");
		return FALSE;
	}

	if (gHeapEnd == NULL) {
		fprintf(stderr, "Uninitialized heap end\n");
		return FALSE;
	}

	if (gHeapStart == gHeapEnd) {
		if (gFreeHead == NULL)
			return 1;
		fprintf(stderr, "Inconsistent emtpy heap\n");
		return FALSE;
	}
	
	/* Check whether every memory block is valid. */
	for (w = (Chunk_T)gHeapStart;
		w && w < (Chunk_T)gHeapEnd;
		w = ChunkGetNextAdjacent(w, gHeapStart, gHeapEnd)) {
		
		if (!ChunkIsValid(w, gHeapStart, gHeapEnd))
			return FALSE;
	}
	
	/* Check whether every memory block in every bin is valid. */
	for (int i = 0; i < SIZE_OF_BINS; i++) {
		for (w = bins[i]; w; w = ChunkGetNextFreeChunk(w)) {
			Chunk_T n;
			
			if (ChunkGetStatus(w) != CHUNK_FREE) {
				fprintf(stderr, 
					    "Non-free chunk in the free chunk list\n");
				return FALSE;
			}
			
			if (!ChunkIsValid(w, gHeapStart, gHeapEnd))
				return FALSE;
			
			n = ChunkGetNextAdjacent(w, gHeapStart, gHeapEnd);
			if (n != NULL && n == ChunkGetNextFreeChunk(w)) {
				fprintf(stderr, "Uncoalseced chunks\n");
				return FALSE;
			}
		}
	}
	
	return TRUE;
}
#endif

/*--------------------------------------------------------------*/
/* SizeToUnits:
 * Returns capable number of units for 'size' bytes. 
 */
/*--------------------------------------------------------------*/
static int
SizeToUnits(int size)
{
	return (size + (CHUNK_UNIT-1))/CHUNK_UNIT;
}

/*--------------------------------------------------------------*/
/* GetChunkFromDataPtr:
 * Returns the header pointer that contains data 'm'. 
 */
/*--------------------------------------------------------------*/
static Chunk_T
GetChunkFromDataPtr(void *m)
{
	return (Chunk_T)((char *)m - CHUNK_UNIT);
}

static int
GetIndexFromUnits(int units)
{
	return units >= SIZE_OF_BINS ? SIZE_OF_BINS-1 : units-1;
}

/*--------------------------------------------------------------------*/
/* InitMyHeap: 
 * Initialize data structures and global variables for
 * chunk management. 
 */
/*--------------------------------------------------------------------*/
static void
InitMyHeap(void) 
{
	/* Initialize gHeapStart and gHeapEnd. */
	gHeapStart = gHeapEnd = sbrk(0);
	if (gHeapStart == (void *)-1) {
		fprintf(stderr, "sbrk(0) failed\n");
		exit(-1);
	}
}

/*--------------------------------------------------------------------*/
/* MergeChunk:
 * Merge two adjacent chunks and return the merged chunk.
 * Returns NULL on error. 
 */
/*--------------------------------------------------------------------*/
static Chunk_T
MergeChunk(Chunk_T c1, Chunk_T c2)
{
	/* c1 and c2 must be adjacent. */
	assert(c1 < c2 && ChunkGetNextAdjacent(c1, gHeapStart, gHeapEnd) == c2);
	
	/* Adjust the unit of c1. */
	ChunkSetUnits(c1, ChunkGetUnits(c1) + ChunkGetUnits(c2) + 2);
	
	return c1;
}

/*--------------------------------------------------------------------*/
/* RemoveChunkFromList:
 * Removes 'c' from the bin.
 */
/*--------------------------------------------------------------------*/
static void
RemoveChunkFromList(Chunk_T c, int index)
{
	/* Check whether the chunkc is in the free list. */
	assert (ChunkGetStatus(c) == CHUNK_FREE);
	/* Check the index has proper value. */
	assert ((0 <= index) && (index < SIZE_OF_BINS));
	
	Chunk_T prev, next;
	prev = ChunkGetPrevFreeChunk(c);
	next = ChunkGetNextFreeChunk(c);
	
	/* c is head of the bin. */
	if (prev == NULL) 
		bins[index] = next;
	/* c is in the middle of the bin. */
	else 
		ChunkSetNextFreeChunk(prev, next);
	/* c has next free chunk. */
	if (next != NULL) {
		ChunkSetPrevFreeChunk(next, prev);
	}
	
	/* c has next free chunk */
	if (next != NULL) {
		ChunkSetPrevFreeChunk(next, prev);
	}
	
	/* Clear next and prev chunk pointers of c to NULL. */
	ChunkSetNextFreeChunk(c, NULL);
	ChunkSetPrevFreeChunk(c, NULL);
	/* Mark this chunk as "IN_USE". */
	ChunkSetStatus(c, CHUNK_IN_USE);
}

/*--------------------------------------------------------------------*/
/* InsertChunk:
 * Insert a chunk, 'c', into the head of the proper bin, 
 * according to the size of 'c'.
 * 'c' will be merged with neighbor free chunks if possible.
 * The status of 'c' is set to CHUNK_FREE.
 * It will return 'c' (after the merge operation if possible).
 */
/*--------------------------------------------------------------------*/
static void
InsertChunk(Chunk_T c)
{
	Chunk_T prev, next;
	int units, index;
	/* Check whether chunk's data unit is larger or equal than 1. */
	units = ChunkGetUnits(c);
	assert(units >= 1);
	
	/* Set c's status to CHUNK_FREE. */
	ChunkSetStatus(c, CHUNK_FREE);
	
	/* Coalesce with neighbors if possible. */
	prev = ChunkGetPrevAdjacent(c, gHeapStart, gHeapEnd);
	if (prev != NULL && ChunkGetStatus(prev) == CHUNK_FREE) {
		index = GetIndexFromUnits(ChunkGetUnits(prev));
		RemoveChunkFromList(prev, index);
		c = MergeChunk(prev, c);
	}
	next = ChunkGetNextAdjacent(c, gHeapStart, gHeapEnd);
	if (next != NULL && ChunkGetStatus(next) == CHUNK_FREE) {
		index = GetIndexFromUnits(ChunkGetUnits(next));
		RemoveChunkFromList(next, index);
		c = MergeChunk(c, next);
	}
	
	/* Calculate the index of proper bin,
	 * according to the size of the chunk. */
	units = ChunkGetUnits(c);
	index = GetIndexFromUnits(units);
	
	/* Set c's status to CHUNK_FREE. */
	ChunkSetStatus(c, CHUNK_FREE);
	/* Adjust the pointers of chunks. */
	if (bins[index] != NULL)
		ChunkSetPrevFreeChunk(bins[index], c);
	ChunkSetNextFreeChunk(c, bins[index]);
	ChunkSetPrevFreeChunk(c, NULL);
}

/*--------------------------------------------------------------------*/
/* SplitChunk:
 * Split 'c' into two chunks s.t. the size of one chunk is 'units' and
 * the size of the other chunk is (original size - 'units' - 2).
 * returns the chunk with 'units'.
 * Returns the data chunk. 
 * Inserts the other chunk to the proper bin. */
/*--------------------------------------------------------------------*/
static Chunk_T
SplitChunk(Chunk_T c, int units)
{
	Chunk_T c2;
	int all_units, index;
	
	/* Check validity of the chunk to split. */
	assert(c >= (Chunk_T)gHeapStart && c <= (Chunk_T)gHeapEnd);
	assert(ChunkGetStatus(c) == CHUNK_FREE);
	/* Assume chunk with header and footer unit. */
	assert(ChunkGetUnits(c) > units + 2);
	
	/* Adjust the size of the first chunk. */
	all_units = ChunkGetUnits(c);
	index = GetIndexFromUnits(all_units);
	ChunkSetUnits(c, all_units - units - 2);
	
	/* Remove the other chunk and insert it to the proper bin. */
	RemoveChunkFromList(c, index);
	InsertChunk(c);
	
	/* Prepare for the second chunk. */
	c2 = ChunkGetNextAdjacent(c, gHeapStart, gHeapEnd);
	ChunkSetUnits(c2, units);
	ChunkSetStatus(c2, CHUNK_IN_USE);
	
	/* Adjust the pointers of c2. */
	ChunkSetNextFreeChunk(c2, NULL);
	ChunkSetPrevFreeChunk(c2, NULL);
	
	return c2;
}

/*--------------------------------------------------------------------*/
/* AllocateMoreMemory: 
 * Allocate a new chunk which is capable of holding 'units' chunk
 * units in memory by increasing the heap, and return the new
 * chunk.
*/
/*--------------------------------------------------------------------*/
static Chunk_T
AllocateMoreMemory(int units)
{
	Chunk_T c;
	
	/* Need to allocate two more units for header and footer. */
	c = (Chunk_T)sbrk((units + 2) * CHUNK_UNIT);
	/* Allocation failed */
	if (c == (Chunk_T)-1)
		return NULL;
	
	gHeapEnd = sbrk(0);
	ChunkSetUnits(c, units);
	ChunkSetNextFreeChunk(c, NULL);
	ChunkSetPrevFreeChunk(c, NULL);
	ChunkSetStatus(c, CHUNK_IN_USE);
	
	return c;
}

/*--------------------------------------------------------------------*/
/* HeapMgr_malloc:
 * Dynamically allocate a memory capable of holding size bytes. 
 * Substitute for GNU malloc().                                 
 */
/*--------------------------------------------------------------------*/
void *
HeapMgr_malloc(size_t size)
{
	static int isInit = FALSE;
	Chunk_T c;
	int units, index;
	
	if (size <= 0)
		return NULL;
	
	/* Initialize the heap at the first run. */
	if (isInit == FALSE) {
		InitMyHeap();
		isInit = TRUE;
	}
	
	/* Check whether every chunk is valid. */
	assert (CheckHeapValidity());
	
	/* Calculate the number of units required. */
	units = SizeToUnits(size);
	/* Calculate the index of proper bin,
	 * according to the size of the chunk. */
	index = GetIndexFromUnits(units);
	
	/* Look for the right bin to find the memeory block of unit size. */
	c = bins[index];
	/* If there's a memory block in the right bin,
	 * remove the chunk from the bin and return the first data byte. */
	if (c != NULL) {
		RemoveChunkFromList(c, index);
		return (void *)((char *)c + CHUNK_UNIT);
	}
	/* If there's not perfect fit,
	 * scan the upper bins, split it, and return the first data byte. */
	else {
		for (index += 3; index < SIZE_OF_BINS; index++) {
			/* For the non-last bins, scan the first chunk only. */
			if (index != SIZE_OF_BINS-1) {
				c = bins[index];
				if (c != NULL) {
					c = SplitChunk(c, units);
					// ChunkSetStatus(c, CHUNK_IN_USE);
					return (void *)((char *)c + CHUNK_UNIT);
				}
			}
			/* For the last bin, scan through the whole free list. */
			else {
				for (c = bins[index]; c; c = ChunkGetNextFreeChunk(c)) {
					/* Free chunk is big enough. */
					if (ChunkGetUnits(c) >= units) {
						/* If the free chunk is bigger than we need,
							 * split it. */
						if (ChunkGetUnits(c) > units + 2) {
							c = SplitChunk(c, units);
							return (void *)((char *)c + CHUNK_UNIT);
						}
						/* If it is a perfect fit,
						 * remove the chunk from the free list. */
						else {
							RemoveChunkFromList(c, index);
							return (void *)((char *)c + CHUNK_UNIT);
						}
					}
				}
			}
		}
	}
	
	/* If there is no proper free chunk in any bin, 
	 * allocate more memory. */
	c = AllocateMoreMemory(units);
	/* Allocation failed */
	if (c == NULL) {
		assert(CheckHeapValidity());
		return NULL;
	}
	assert(ChunkGetUnits(c) == units);
	
	assert(CheckHeapValidity());
	/* Return the memory adress of the first data byte. */
	return (void *)((char *)c + CHUNK_UNIT);
}

/*--------------------------------------------------------------------*/
/* HeapMgr_free:
 * Releases dynamically allocated memory.
 */
/*--------------------------------------------------------------------*/
void
HeapMgr_free(void *m)
{
	Chunk_T c;
	
	if (m == NULL)
		return;
	
	/* Check everything is OK before freeing 'm'. */
	assert(CheckHeapValidity());
	
	/* Get the chunk header pointer from 'm'. */
	c = GetChunkFromDataPtr(m);
	assert(ChunkGetStatus(c) != CHUNK_FREE);
	
	/* Inser the new free chunk at the first of the proper bin. */
	InsertChunk(c);
	
	/* Double check if everything is OK. */
	assert(CheckHeapValidity());
}
