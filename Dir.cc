/*
 * Dusty Mabe - 2014
 * Dir.cc - Implementation of a directory to track sharing across
 *          partitions.
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "Dir.h"
#include "BitVector.h"
#include "Net.h"
#include "Tile.h"
#include "types.h"

// Global NETWORK is defined in simulator.cc
extern Net *NETWORK;

/*
 * DirEntry constructor
 *    - Build up the data structures that belong to a
 *      directory entry pertaining to the block with base
 *      address blockaddr.
 */
DirEntry::DirEntry(ulong blockaddr) {
    blockaddr = blockaddr;
    state     = DSTATEI;
    sharers   = new BitVector(0);
    location  = 0; //XXX 
}

/*
 * DirEntry destructor
 *    - Free mem related to DirEntry object.
 */
DirEntry::~DirEntry() {
    delete sharers;
}


/*
 * Dir constructor
 *    - Build up the data structures that belong to a
 *      directory.
 */
Dir::Dir() {
    int i;

    // We need a directory for every block. How many do we need
    // for a 32 bit address space and 64 byte blocks?
    //
    //      2^32 bytes * (1 block / 2^6 bytes) = 2^26 blocks
    //
    // Therefore we need 2^26 directory entries. What we will
    // do now is just allocoate the pointers for that many 
    // directories but we won't allocate the directories until
    // they are accessed.  
    directory = new DirEntry*[(1 << 26) - 1];

    // Verify each direntry is initialized to NULL
    for (i=0; i < (1<<26); i++)
        assert(directory[i] == NULL);

    // We need a table of partition vectors. There can be at most
    // NPROCS partitions (i.e. each partition has only one tile)
    parttable = new BitVector*[NPROCS];

    // For now create generic mapping
    // XXX
    for (i=0; i < NPROCS; i++) {
        parttable[i] = new BitVector(1 << i);
    }

}

/*
 * Dir::mapAddrToTile
 *     - Given an address and a partition ID, map them
 *       to a specific tile within the partition. 
 */
int Dir::mapAddrToTile(int partid, int blockaddr) {

    // Get the vector representing the partition
    BitVector *bv = parttable[partid];

    // Get the number of tiles within the partition
    int numtiles = bv->getNumSetBits();

    // Since the tiles logically share L2 the blocks are 
    // interleaved among the tiles. Find the tile offset
    // within the partition.
    int tileoffset = blockaddr % numtiles; 

    // Find the actual tile id of the tile. Note: add
    // 1 because even if offset is 0 we want to find 1st
    // set bit. 
    int tileid = bv->getNthSetBit(tileoffset + 1);

    return tileid;
}

/*
 * Dir::mapTileToPart
 *     - Given a tile index find the partition it belongs to
 */
int Dir::mapTileToPart(int tileid) {
    int i;
    for (i=0; i < NPROCS; i++)
        if (parttable[i]->getBit(tileid))
            return i;

    assert(0); // Should not get here
}

/*
 * Dir::invalidateSharers
 *     - Given a block address use the DirEntry sharers bitvector
 *       to find what partitions share the block. Map the addr
 *       and partid to a specific tile and then send an invalidation
 *       to the tile.
 */
int Dir::invalidateSharers(int blockaddr) {

    // Get the bitvector of sharers.
    DirEntry  *de = directory[blockaddr];
    BitVector *bv = de->sharers;

    //printf("Sharers are %x\n", bv->vector);

    int tileid;
    int partid;

    // Iterate over sharers and send INV to any that
    // exist. Also clear bit from vector.
    for(partid=0; partid < bv->size; partid++) {
        if (bv->getBit(partid)) {
            // Get the actual tileid of the tile within the
            // partition that is responsible for blockaddr
            tileid = mapAddrToTile(partid, blockaddr);
            NETWORK->sendDirToTile(INV, blockaddr << OFFSETBITS, tileid);
            bv->clearBit(partid);
        }
    }
}

/*
 * Dir::interveneOwner
 *     - Given a block address use the DirEntry sharers bitvector
 *       to find the partition that owns the block. Map the addr
 *       and partid to a specific tile and then send an intervention
 *       to the tile.
 */
int Dir::interveneOwner(int blockaddr) {
    // Get the bitvector of sharers.
    DirEntry  *de = directory[blockaddr];
    BitVector *bv = de->sharers;

    int tileid;
    int partid;

    // Iterate over sharers and send INT to any that
    // exist.
    for(partid=0; partid < bv->size; partid++) {
        if (bv->getBit(partid)) {
            // Get the actual tileid of the tile within the
            // partition that is responsible for blockaddr
            tileid = mapAddrToTile(partid, blockaddr);
            NETWORK->sendDirToTile(INT, blockaddr << OFFSETBITS, tileid);
        }
    }
}

/*
 * Dir::setState
 *     - This function serves to change the state of the Directory
 *       entry to s. If we are transitioning to an invalid state then
 *       there is some housekeeping to do.
 */
void Dir::setState(ulong blockaddr, int s) {

    DirEntry * de = directory[blockaddr];
    assert(de); // verify de is not NULL

    de->state = s; // Set the new state

    // If we are going to the invalid state then delete the
    // memory associated with the directory entry.
    if (s == DSTATEI)
        delete de;
}

/*
 * Dir::getFromNetwork
 *     - This function serves to receive messages from the
 *       interconnection network.
 *
 * Returns the directory state to the caller
 */
ulong Dir::getFromNetwork(ulong msg, ulong addr, ulong fromtile) {

    // Get the blockaddr
    ulong blockaddr = addr >> OFFSETBITS;

    // Get the partition that the tile belongs to
    ulong partid = mapTileToPart(fromtile); 

    if (directory[blockaddr] == NULL)
        directory[blockaddr] = new DirEntry(blockaddr);

    switch (msg) {
        case RD: 
            netInitRd(blockaddr, partid);
            break;
        case RDX: 
            netInitRdX(blockaddr, partid);
            break;
        case UPGR: 
            netInitUpgr(blockaddr, partid);
            break;
        default :
            assert(0); // should not get here
    }

    return directory[blockaddr]->state;
}

/*
 * Dir::netInitRdX
 *     - This function handles the logic for when a RdX request
 *       is delivered to the directory.
 */
void Dir::netInitRdX(ulong blockaddr, ulong partid) {

    DirEntry * de = directory[blockaddr];

    switch (de->state) {

        // For EM we need to invalidate the current owner
        // and reply with data to new owner. Will stay in M state.
        case DSTATEEM: 
            // Invalidate current owner.
            invalidateSharers(blockaddr);
            // Add new owner to bit map.
            de->sharers->setBit(partid);
            break;

        // For S we need to transistion to M and invalidate all
        // sharers.
        case DSTATES: 
            // Invalidate all sharers
            invalidateSharers(blockaddr);
            // Add new owner to bit map.
            de->sharers->setBit(partid);
            // Transition to EM
            setState(blockaddr, DSTATEEM);
            break;

        // For invalid state just transition to M
        case DSTATEI: 
            // Add new owner to bit map.
            de->sharers->setBit(partid);
            // Transition to EM
            setState(blockaddr, DSTATEEM);
            break;

        default :
            assert(0); // should not get here
    }
}


/*
 * Dir::netInitRd
 *     - This function handles the logic for when a Rd request
 *       is delivered to the directory.
 */
void Dir::netInitRd(ulong blockaddr, ulong partid) {

    DirEntry * de = directory[blockaddr];

    switch (de->state) {

        // For EM we need to transistion to shared state 
        // and send an intervention to previous owner.
        case DSTATEEM: 
            // send intervention
            interveneOwner(blockaddr);
            // Add new sharer to bit map.
            de->sharers->setBit(partid);
            // Transition to S
            setState(blockaddr, DSTATES);
            break;

        // For S, no need to change state
        case DSTATES: 
            // Add new sharer to bit map.
            de->sharers->setBit(partid);
            break;

        // For I, transition to EM
        case DSTATEI: 
            // Add new sharer to bit map.
            de->sharers->setBit(partid);
            // Transition to EM
            setState(blockaddr, DSTATEEM);
            break;

        default :
            assert(0); // should not get here
    }
}

/*
 * Dir::netInitUpgr
 *     - This function handles the logic for when an Upgr request
 *       is delivered to the directory.
 */
void Dir::netInitUpgr(ulong blockaddr, ulong partid) {

    DirEntry * de = directory[blockaddr];

    switch (de->state) {
        // For ME we should never get UPGR since there
        // should not be more than one copy in the system
        case DSTATEEM: 
            assert(0);
            break;

        // For S, transistion to EM
        case DSTATES:
            // Invalidate all sharers, but first clear
            // the bit related to partid because that one 
            // shouldn't be invalidated.
            de->sharers->clearBit(partid);
            invalidateSharers(blockaddr);
            // Transition to EM
            setState(blockaddr, DSTATEEM);
            // Add partid back into sharers bit map.
            de->sharers->setBit(partid);
            break;

        // For I we should never get UPGR because there are
        // no copies in the system.
        case DSTATEI: 
            assert(0);
            break;

        default :
            assert(0); // should not get here
    }
}
