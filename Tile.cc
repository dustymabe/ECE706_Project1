/*
 * Dusty Mabe - 2014
 * cache.cc - Implementation of cache (either L1 or L2)
 */

#include <assert.h>
#include <stdio.h>
#include "Tile.h"
#include "Cache.h"
#include "Partition.h"
#include "Net.h"
#include "params.h"


// Global NETWORK is defined in simulator.cc
extern Net *NETWORK;


Tile::Tile(int number) {

    index = number;

    l1cache = new Cache(this, L1, L1SIZE, L1ASSOC, BLKSIZE);
    assert(l1cache);

    l2cache = new Cache(this, L2, L2SIZE, L2ASSOC, BLKSIZE);
    assert(l2cache);

    part = new Partition(1 << index); // Need to pass in partition information
}


/*
 * Tile::Access()
 *     - Access is the function that gets called when a value 
 *       is to be read/written by the proc in this tile.
 */
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
        broadCastToPartition(L1INV, l2cache->getVictimAddr());

        // reset victimAddr
        l2cache->setVictimAddr(0);
    }

}

/*
 * Tile::PrintStats()
 *     - Print a header and then query the L1 and L2 to print
 *       stats about hit/miss rates. etc.
 */
void Tile::PrintStats() {
    printf("===== Simulation results (Cache %d L1) =============\n", index);
    l1cache->PrintStats();
    printf("===== Simulation results (Cache %d L2) =============\n", index);
    l2cache->PrintStats();
}


/*
 * Tile::getFromNetwork
 *     - This function will be called by the Net class and
 *       represents when a message has been received by this tile
 *       from the network.
 */
void Tile::getFromNetwork(ulong msg, ulong addr) {

    // Network will call this function when sending a message to this
    // tile. 
    
    if (msg == L1INV) {
        l1cache->invalidateLineIfExists(addr);
    }

    // Should not get here.
    assert(0);
}

/*
 * Tile::broadCastToPartition
 *     - Broadcast a message to all Tiles in a partition
 *       regarding the provided addr. Utilizes the partition
 *       table to determine which tiles are within our partition
 *       and utilizes the network to send the message.  
 *
 */
#define MAX(x,y) ((x > y) ? x : y);
void Tile::broadCastToPartition(ulong msg, ulong addr) {

    int i;
    int max = 0;
    int vector = part->getVector();

    for(i=0; i < NPROCS; i++)
        if (vector & (1 << i))
            max = MAX(max, NETWORK->sendToTile(msg, i, addr));

    // XXX do some housekeeping with max
}
