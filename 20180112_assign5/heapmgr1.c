/*--------------------------------------------------------------------*/
/* heapmgr1.c                                                         */
/* Author: Seungsu Kim                                                */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "chunk.h"

#define FALSE 0
#define TRUE  1

enum {
	MEMALLOC_MIN = 1024,
};

/* gFreeHead: point to first chunk in the free list */
static Chunk_T gFreeHead = NULL;

/* gHeapStart, gHeapEnd: start and end of the heap area.
 * gHeapEnd will move if you increase the heap */
static void *gHeapStart = NULL, *gHeapEnd = NULL;

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
	for (w = gFreeHead; w; w = ChunkGetNextFreeChunk(w)) {
		Chunk_T n;

		if (ChunkGetStatus(w) != CHUNK_FREE) {
			fprintf(stderr, "Non-free chunk in the free chunk list\n");
			return FALSE;
		}

		if (!ChunkIsValid(w, gHeapStart, gHeapEnd))
			return FALSE;

		n = ChunkGetNextAdjacent(w, gHeapStart, gHeapEnd);
		if (n != NULL && n == ChunkGetNextFreeChunk(w)) {
			fprintf(stderr, "Uncoalesced chunks\n");
			return FALSE;
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

/*--------------------------------------------------------------------*/
/* InitMyHeap: 
 * Initialize data structures and global variables for
 * chunk management. 
 */
/*--------------------------------------------------------------------*/
static void
InitMyHeap(void)
{
	/* Initialize gHeapStart and gHeapEnd */
	gHeapStart = gHeapEnd = sbrk(0);
	if (gHeapStart == (void *)-1) {
		fprintf(stderr, "sbkr(0) failed\n");
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

	assert(ChunkGetStatus(c1) == CHUNK_IN_USE);
	assert(ChunkGetStatus(c2) == CHUNK_IN_USE);

	/* Adjust the unit of c1. */
	ChunkSetUnits(c1, ChunkGetUnits(c1) + ChunkGetUnits(c2) + 2);

	return c1;
}

/*--------------------------------------------------------------------*/
/* SplitChunk:
 * Split 'c' into two chunks s.t. the size of one chunk is 'units' and
 * the size of the other chunk is (original size - 'units' - 2).
 * returns the chunk with 'units'
 * Returns the data chunk. */
/*--------------------------------------------------------------------*/
static Chunk_T
SplitChunk(Chunk_T c, int units)
{
	Chunk_T c2;
	int all_units;

	/* Check validity of the chunk to split. */
	assert(c >= (Chunk_T)gHeapStart && c <= (Chunk_T)gHeapEnd);
	assert(ChunkGetStatus(c) == CHUNK_FREE);
	/* Assume chunk with header and footer unit. */
	assert(ChunkGetUnits(c) > units + 2);

	/* Adjust the size of the first chunk. */
	all_units = ChunkGetUnits(c);
	ChunkSetUnits(c, all_units - units - 2);

	/* Prepare for the second chunk. */
	c2 = ChunkGetNextAdjacent(c, gHeapStart, gHeapEnd);
	ChunkSetUnits(c2, units);
	ChunkSetStatus(c2, CHUNK_IN_USE);
	
	/* Adjust the pointers of both chunks. */
	ChunkSetPrevFreeChunk(c, ChunkGetPrevFreeChunk(c2));
	ChunkSetNextFreeChunk(c2, NULL);
	ChunkSetPrevFreeChunk(c2, NULL);

	return c2;
}

/*--------------------------------------------------------------------*/
/* RemoveChunkFromList:
 * Removes 'c' from the free chunk list.
 */
/*--------------------------------------------------------------------*/
static void
RemoveChunkFromList(Chunk_T c)
{
	assert (ChunkGetStatus(c) == CHUNK_FREE);

	Chunk_T prev, next;

	prev = ChunkGetPrevFreeChunk(c);
	next = ChunkGetNextFreeChunk(c);

	/* c is head of the list. */
	if (prev == NULL)
		gFreeHead = next;
	/* c is in the middle of the list. */
	if (prev != NULL)
		ChunkSetNextFreeChunk(prev, next);
	/* c has next free chunk. */
	if (next != NULL)
		ChunkSetPrevFreeChunk(next, prev);

	/* Clear next and prev chunk pointers of c to NULL. */
	ChunkSetNextFreeChunk(c, NULL);
	ChunkSetPrevFreeChunk(c, NULL);
	/* Mark this chunk as "IN_USE". */
	ChunkSetStatus(c, CHUNK_IN_USE);
}

/*--------------------------------------------------------------------*/
/* InsertChunk:
 * Insert a chunk, 'c', into the head of the free chunk list. 
 * The status of 'c' is set to CHUNK_FREE
 */
/*--------------------------------------------------------------------*/
static void
InsertChunk(Chunk_T c)
{
	Chunk_T prev, next;
	
	/* Check whether chunk's data unit is larger or equal than 1. */
	assert(ChunkGetUnits(c) >= 1);
	ChunkSetNextFreeChunk(c,NULL);
	ChunkSetPrevFreeChunk(c, NULL);

	/* Coalesce with neighbors if possible. */
	prev = ChunkGetPrevAdjacent(c, gHeapStart, gHeapEnd);
	if (prev != NULL && ChunkGetStatus(prev) == CHUNK_FREE) {
		RemoveChunkFromList(prev);
		c = MergeChunk(prev, c);
	}
	next = ChunkGetNextAdjacent(c, gHeapStart, gHeapEnd);
	if (next != NULL && ChunkGetStatus(next) == CHUNK_FREE) {
		RemoveChunkFromList(next);
		c = MergeChunk(c, next);
	}

	/* Set c's status to CHUNK_FREE. */
	ChunkSetStatus(c, CHUNK_FREE);

	/* Adjust the pointers of c and gFreeHead */
	if (gFreeHead)
		ChunkSetPrevFreeChunk(gFreeHead, c);
	ChunkSetNextFreeChunk(c, gFreeHead);
	ChunkSetPrevFreeChunk(c, NULL);
	
	/* Update gFreeHead. */
	gFreeHead = c;
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

	if (units < MEMALLOC_MIN)
		units = MEMALLOC_MIN;

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

	/* Insert the newly allocated chunk 'c' to the free list. */
	InsertChunk(c);

	assert (CheckHeapValidity());
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
	int units;

	if (size <= 0)
		return NULL;

	/* Initialize the heap at the first run. */
	if (isInit == FALSE)  {
		InitMyHeap();
		isInit = TRUE;
	}

	/* Check whether every chunk is valid. */
	assert (CheckHeapValidity());

	/* Calculate the number of units required. */
	units = SizeToUnits(size);

	/* Scan the free chunk list for the proper free meory block. */
	for (c = gFreeHead;
		 c != NULL;
		 c = ChunkGetNextFreeChunk(c)) {

		/* Free chunk is big enough */
		if (ChunkGetUnits(c) >= units) {
			/* If the free chunk is bigger than we need, split it. */
			if (ChunkGetUnits(c) > units + 2) {
				c = SplitChunk(c, units);
				ChunkSetStatus(c, CHUNK_IN_USE);
			}
			/* If it is a perfect fit, 
			 * remove the chunk from the free list. */
			else
				RemoveChunkFromList(c);

			assert(CheckHeapValidity());
			/* Return the memory adress of the first data byte. */
			return (void *)((char *)c + CHUNK_UNIT);
		}
	}

	/* If there is no proper free chunk, allocate more memory. */
	c = AllocateMoreMemory(units);
	if (c == NULL) {
		assert(CheckHeapValidity());
		return NULL;
	}
	assert(ChunkGetUnits(c) >= units);

	/* If the free chunk is bigger than we need, split it. */
	if (ChunkGetUnits(c) > units + 2)
		c = SplitChunk(c, units);
	/* If it is a perfect fit, 
	 * remove the chunk from the free chunk list. */
	else
		RemoveChunkFromList(c);

	assert(CheckHeapValidity());
	/* Return the memory adress of the first data byte. */
	return (void *)((char *)c + CHUNK_UNIT);
}

/*--------------------------------------------------------------*/
/* HeapMgr_free:
 * Releases dynamically allocated memory.
 */
/*--------------------------------------------------------------*/
void
HeapMgr_free(void *m)
{
	Chunk_T c;
	
	if (m == NULL)
		return;

	/* Check everything is OK before freeing 'm'. */
	assert(CheckHeapValidity());

	/* Get the chunk header pointer from m. */
	c = GetChunkFromDataPtr(m);
	assert(ChunkGetStatus(c) != CHUNK_FREE);

	/* Insert the new free chunk at the first of the free list. */
	InsertChunk(c);

	/* Double check if everything is OK. */
	assert(CheckHeapValidity());
}
