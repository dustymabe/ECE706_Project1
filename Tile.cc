/*
 * Dusty Mabe - 2014
 * cache.cc - Implementation of cache (either L1 or L2)
 */

#include "Tile.h"
#include "params.h"


Tile::Tile() {

    l1cache = new Cache(L1, L1SIZE, L1ASSOC, BLKSIZE);

    l2cache = new Cache(L2, L2SIZE, L2ASSOC, BLKSIZE);

}


Tile::Access(ulong addr, uchar op) {

    int state;

    // L1 cache is write-through!!

    // L1: Check L1 to see if hit
    state = l1cache->Access(addr, op);

    // If hit and write then write through to L2
    //
    XXX need to make this check aggregate L2 of partition
    // can use partition table to determine which L2 cache
    // the block will be in.
    if (state == HIT && op == 'w')
        l2cache->Access(addr, op)

    //////////////
    // done with hit
    //////////////
    
    // L2: Check aggregate L2 (logical sharing) to see
    //     if the block is in the aggregate L2.
    state = l2cache->Access(addr, op);

    // If hit then done.
    return;

    // If miss in L1 and L2.. then it is possible a block was 
    // evicted from the L2. If that is the case then we need to 
    // broadcast an invalidation for that block to all L1 caches in the 
    // partition.
    if (state == MISS && l2cache->victimAddr != 0) {

        // broadcast to all L1s the invalidation

        // reset victimAddr
        l2cache->victimAddr = 0
    }

}
