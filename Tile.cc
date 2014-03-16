/*
 * Dusty Mabe - 2014
 * cache.cc - Implementation of cache (either L1 or L2)
 */

#include <assert.h>
#include <stdio.h>
#include "Tile.h"
#include "Cache.h"
#include "CacheLine.h"
#include "CCSM.h"
#include "BitVector.h"
#include "Net.h"
#include "params.h"


// Global NETWORK is defined in simulator.cc
extern Net *NETWORK;


Tile::Tile(int number, int partition) {

    index = number;

    l1cache = new Cache(this, L1, L1SIZE, L1ASSOC, BLKSIZE);
    assert(l1cache);

    l2cache = new Cache(this, L2, L2SIZE, L2ASSOC, BLKSIZE);
    assert(l2cache);

    part = new BitVector(partition);
}


/*
 * Tile::Access()
 *     - Access is the function that gets called when a value 
 *       is to be read/written by the proc in this tile.
 */
void Tile::Access(ulong addr, uchar op) {

    int state;

    // L1: Check L1 to see if hit
    state = l1cache->Access(addr, op);

    // If a hit then we are done (almost). Must make any write
    // hits in the L1 access the L2 as well (WRITETHROUGH).
    if (state == HIT && op == 'w')
        L2Access(addr, op); // Aggregate L2 access

    // Finally, if L1 hit then we are done
    if (state == HIT)
        return;
    

    // L2: Check aggregate L2 (logical sharing) to see
    //     if the block is in the aggregate L2.
    L2Access(addr, op);

}

/*
 * Tile::L2Access()
 *     - Provide a generic access function that will access the
 *       aggregate L2 (logically shared) for a partition. If HIT,
 *       then L2 returns quickly. If MISS, the L2 will contact the
 *       Memory controller (directory) and retrieve the value.
 */
void Tile::L2Access(ulong addr, uchar op) {

    int tileid = mapAddrToTile(addr);
    int msg    = (op == 'w') ? L2WR : L2RD;
    NETWORK->sendTileToTile(msg, addr, index, tileid);

}

/*
 * Tile::mapAddrToTile
 *     - Given an address map it to a specific tile 
 *       within the partition.
 */
int Tile::mapAddrToTile(ulong addr) {

    // Get the block address (i.e. minus offset bits)
    int blockaddr = addr >> OFFSETBITS;

    // Get the number of tiles within the partition
    int numtiles = part->getNumSetBits();

    // Since the tiles logically share L2 the blocks are
    // interleaved among the tiles. Find the tile offset
    // within the partition.
    int tileoffset = blockaddr % numtiles;

    // Find the actual tile id of the tile. Note: add
    // 1 because even if offset is 0 we want to find 1st
    // set bit.
    int tileid = part->getNthSetBit(tileoffset + 1);

    return tileid;
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

    CacheLine * line;

    // Handle L1 messages first
    if (msg == L1INV) {
        l1cache->invalidateLineIfExists(addr);
        return;
    }


    // Now handle L2 messages
    switch (msg) {

        case INV:
        case INT:

            // Get the L2 cache line that corresponds to addr
            line = l2cache->findLine(addr);

            // If the line has been evicted already then 
            // nothing to do.
            if (!line)
                return;

            // Pass the message on to the CCSM
            line->ccsm->getFromNetwork(msg);
            break;

        case L2RD:
            l2cache->Access(addr, 'r');
            break;

        case L2WR:
            l2cache->Access(addr, 'w');
            break;

        default:
            assert(0); // Should not get here
    }
}

/*
 * Tile::broadcastToPartition
 *     - Broadcast a message to all Tiles in a partition
 *       regarding the provided addr. Utilizes the partition
 *       table to determine which tiles are within our partition
 *       and utilizes the network to send the message.  
 *
 */
#define MAX(x,y) ((x > y) ? x : y);
void Tile::broadcastToPartition(ulong msg, ulong addr) {

    int i;
    int max = 0;

    for(i=0; i < part->size; i++)
        if (part->getBit(i)) 
            max = MAX(max, NETWORK->sendTileToTile(msg, addr, index, i));

    // XXX do some housekeeping with max
}
