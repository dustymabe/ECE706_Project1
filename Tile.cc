/*
 * Dusty Mabe - 2014
 * cache.cc - Implementation of cache (either L1 or L2)
 */

#include <assert.h>
#include <stdio.h>
#include "Tile.h"
#include "Cache.h"
#include "params.h"


Tile::Tile(int number) {

    index = number;

    l1cache = new Cache(this, L1, L1SIZE, L1ASSOC, BLKSIZE);
    assert(l1cache);

    l2cache = new Cache(this, L2, L2SIZE, L2ASSOC, BLKSIZE);
    assert(l2cache);

}


void Tile::Access(ulong addr, uchar op) {

    int state;

    // L1 cache is write-through!!

    // L1: Check L1 to see if hit
    state = l1cache->Access(addr, op);


    // If a hit then we are done (almost). 
    //
    //XXX need to make this check aggregate L2 of partition
    // can use partition table to determine which L2 cache
    // the block will be in.
    if (state == HIT) {

        // Must make any write hits in the L1 access the 
        // L2 as well (writethrough)
        if (op == 'w')
            l2cache->Access(addr, op); // writethrough

        // Now we can return
        return;
    }

    //////////////
    // done with hit
    //////////////
    
    // L2: Check aggregate L2 (logical sharing) to see
    //     if the block is in the aggregate L2.
    state = l2cache->Access(addr, op);

    // If hit then done.
    if (state == HIT)
        return;

    // If miss in L1 and L2.. then it is possible a block was 
    // evicted from the L2. If that is the case then we need to 
    // broadcast an invalidation for that block to all L1 caches in the 
    // partition.
    if (state == MISS && l2cache->getVictimAddr() != 0) {

        // broadcast to all L1s the invalidation

        // reset victimAddr
        l2cache->setVictimAddr(0);
    }

}

void Tile::PrintStats() {
    printf("===== Simulation results (Cache %d L1) =============\n", index);
    l1cache->PrintStats();
    printf("===== Simulation results (Cache %d L2) =============\n", index);
    l2cache->PrintStats();
}


////void Tile::getFromNetwork() {

////    // Network will call this function when sending a message to this
////    // tile. 

////}

////void Tile::broadCastToPartition() {
////    // Broadcast to all tiles in partition
////}
