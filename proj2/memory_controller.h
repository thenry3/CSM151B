#ifndef MEMORY_CONTROLLER_H
#define MEMORY_CONTROLLER_H

#define MEM_SIZE 2048
#define CACHE_LINES 16
#define CACHE_WAYS 4
#define CACHE_SETS CACHE_LINES / CACHE_WAYS
#define BLOCK_SIZE 1 // bytes per block
#define DM 0
#define FA 1
#define SA 2

// struct for a cache line
struct cache_line
{
    int tag;          // you need to compute offset and index to find the tag.
    int lru_position; // for FA only
    int data;         // the actual data stored in the cache/memory
                      // add more things here if needed
};

// main memory controller function
int memoryController(
    bool memR,
    bool memW,
    int *data,
    int addr,
    int status,
    int *miss,
    int type,
    cache_line cache[CACHE_LINES],
    int mem[MEM_SIZE]);

#endif