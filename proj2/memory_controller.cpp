#include "memory_controller.h"

#include <math.h>
#include <iostream>
#include <bitset>

using namespace std;

// updates the lru position for SA and FA
void update(int cacheIdx, cache_line cache[CACHE_LINES], int type)
{
    if (type == DM)
        return;

    int originalPosition = cache[cacheIdx].lru_position;
    int maxPosition = ((type == FA) ? CACHE_LINES : CACHE_WAYS) - 1;
    cache[cacheIdx].lru_position = maxPosition;

    int startIdx = (type == FA) ? 0 : (int)(cacheIdx / CACHE_WAYS) * CACHE_WAYS;
    for (int i = startIdx; i < startIdx + maxPosition + 1; i++)
    {
        if (i != cacheIdx && cache[i].lru_position > originalPosition)
            cache[i].lru_position--;
    }
}

// evict cache line and replace with new data
void evict(int addr, int data, cache_line cache[CACHE_LINES], int type)
{
    int offsetSize = log2(BLOCK_SIZE);
    int indexSize = (type == FA)   ? 0
                    : (type == SA) ? log2(CACHE_SETS)
                                   : log2(CACHE_LINES);
    int tagSize = 32 - offsetSize - indexSize;

    int offset = (addr << (32 - offsetSize)) >> (32 - offsetSize);
    int index = (uint32_t)(addr << (32 - offsetSize - indexSize)) >> (32 - indexSize);
    int tag = (uint32_t)addr >> (32 - tagSize);

    if (type == DM)
    {
        cache[index].tag = tag;
        cache[index].data = data;
        return;
    }

    int idx = -1;
    if (type == FA)
    {
        for (int i = 0; i < CACHE_LINES; i++)
        {
            if (cache[i].lru_position == 0)
            {
                idx = i;
                break;
            }
        }
    }
    else
    {
        for (int i = index * CACHE_SETS; i < index * CACHE_SETS + CACHE_WAYS; i++)
        {
            if (cache[i].lru_position == 0)
            {
                idx = i;
                break;
            }
        }
    }

    cache[idx].tag = tag;
    cache[idx].data = data;
    update(idx, cache, type);
}

// called when there is a cache miss from search
void cacheMiss(int addr, int mem[MEM_SIZE], cache_line cache[CACHE_LINES], int type)
{
    evict(addr, mem[addr], cache, type);
}

// main function to search the cache
bool search(int &addr, int &data, int type, cache_line cache[CACHE_LINES], bool store)
{
    // get offset, index, tag from address
    int offsetSize = log2(BLOCK_SIZE);
    int indexSize = (type == FA)   ? 0
                    : (type == SA) ? log2(CACHE_SETS)
                                   : log2(CACHE_LINES);
    int tagSize = 32 - offsetSize - indexSize;

    int offset = (addr << (32 - offsetSize)) >> (32 - offsetSize);
    int index = (uint32_t)(addr << (32 - offsetSize - indexSize)) >> (32 - indexSize);
    int tag = (uint32_t)addr >> (32 - tagSize);

    // find the cache index if there is a cache hit
    int idx = -1;
    switch (type)
    {
    case DM:
    {
        if (cache[index].tag == tag)
            idx = index;

        break;
    }
    case SA:
    {
        for (int way = 0; way < CACHE_WAYS; way++)
        {
            int cache_idx = index * CACHE_SETS + way;
            if (cache[cache_idx].tag == tag)
            {
                idx = cache_idx;
                break;
            }
        }
        break;
    }
    case FA:
    {
        for (int cache_idx = 0; cache_idx < CACHE_LINES; cache_idx++)
        {
            if (cache[cache_idx].tag == tag)
            {
                idx = cache_idx;
                break;
            }
        }
        break;
    }
    default:
        return false;
    }

    // cache miss
    if (idx == -1)
        return false;

    // if we are doing a store, update data if hit, otherwise get the cache data
    if (store)
        cache[idx].data = data;
    else
    {
        data = cache[idx].data;

        // update LRU positions
        update(idx, cache, type);
    }

    return true;
}

int memoryController(
    bool memR,
    bool memW,
    int *data,
    int addr,
    int status,
    int *miss,
    int type,
    cache_line cache[CACHE_LINES],
    int mem[MEM_SIZE])
{
    if (status < 0)
        return status + 1;

    if (status == 0)
    {
        *data = mem[addr];
        return 1;
    }

    if (memR)
    {
        bool hit = search(addr, *data, type, cache, false);
        if (hit)
            return 1;

        *miss += 1;
        cacheMiss(addr, mem, cache, type);
        return -3;
    }
    else if (memW)
    {
        bool hit = search(addr, *data, type, cache, true);
        mem[addr] = *data;

        *data = 0;
        return 1;
    }

    return 1;
}