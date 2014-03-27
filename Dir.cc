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

// Global delay counter for the current outstanding memory request.
extern int CURRENTDELAY;
extern int CURRENTMEMDELAY;

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
Dir::Dir(int partscheme) {
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

    // Calculate the # of partitions in the system.
    numparts = NPROCS/partscheme;

    // We need a table of partition vectors.
    parttable = new BitVector*[numparts];


    // Based on the partition scheme file in the appropriate
    // vectors with information.
    switch (partscheme) {

        case 1:
            for (i=0; i < numparts; i++)
                parttable[i] = new BitVector(0b1 << i);
            break;

        case 2:
            for (i=0; i < numparts; i++)
                parttable[i] = new BitVector(0b11 << 2*i);
            break;
                                      //   111111 
        case 4:                       //   5432109876543210
            parttable[0] = new BitVector(0b0000000000110011);
            parttable[1] = new BitVector(0b0000000011001100);
            parttable[2] = new BitVector(0b0011001100000000);
            parttable[3] = new BitVector(0b1100110000000000);
            break;
                                      //   111111 
        case 8:                       //   5432109876543210
            parttable[0] = new BitVector(0b1111111100000000);
            parttable[1] = new BitVector(0b0000000011111111);
            break;

                                      //   111111 
        case 16:                      //   5432109876543210
            parttable[0] = new BitVector(0b1111111111111111);
            break;

        default:
            assert(0); // Should not get here
    }
}

/*
 * Dir::mapAddrToTile
 *     - Given an address and a partition ID, map them
 *       to a specific tile within the partition. 
 */
int Dir::mapAddrToTile(int partid, int addr) {

    // Get the vector representing the partition
    BitVector *bv = parttable[partid];

    // Get the number of tiles within the partition
    int numtiles = bv->getNumSetBits();

    // Since the tiles logically share L2 the blocks are 
    // interleaved among the tiles. Find the tile offset
    // within the partition.
    int tileoffset = ADDRHASH(addr) % numtiles; 

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
    for (i=0; i < numparts; i++)
        if (parttable[i]->getBit(tileid))
            return i;

    assert(0); // Should not get here
}

/*
 * Dir::invalidateSharers
 *     - Given a block address use the partitions vectors to determine
 *       what partitions share the block and send invalidations to all
 *       of them. Skip the pid partition.
 */
int Dir::invalidateSharers(int addr, int pid) {
    int max = 0;

    // Lets play a game with CURRENTDELAY. Since this stuff is
    // done in parallel we will save off the original value and
    // then find the max delay of all parallel requests. 
    ulong origDelay = CURRENTDELAY;
    CURRENTDELAY  = 0;

    // Get the bitvector of sharers.
    DirEntry  *de = directory[BLKADDR(addr)];
    BitVector *bv = de->sharers;

    //printf("Sharers are %x\n", bv->vector);

    int tileid;
    int partid;

    // Iterate over sharers and send INV to any that
    // exist. Also clear bit from vector.
    for(partid=0; partid < bv->size; partid++) {

        if (partid == pid)
            continue;

        if (bv->getBit(partid)) {

            // Get the actual tileid of the tile within the
            // partition that is responsible for addr
            tileid = mapAddrToTile(partid, addr);
            NETWORK->sendReqDirToTile(INV, addr, tileid);
            bv->clearBit(partid);

            // Update max and reset
            max = MAX(max, CURRENTDELAY);
            CURRENTDELAY = 0; // Reset for next iter
        }
    }

    // Add the max to the original delay
    CURRENTDELAY = origDelay + max;

}

/*
 * Dir::interveneOwner
 *     - Given a block address use the DirEntry sharers bitvector
 *       to find the partition that owns the block. Map the addr
 *       and partid to a specific tile and then send an intervention
 *       to the tile.
 */
int Dir::interveneOwner(int addr) {
    // Get the bitvector of sharers.
    DirEntry  *de = directory[BLKADDR(addr)];
    BitVector *bv = de->sharers;

    int tileid;
    int partid;

    // Iterate over sharers and send INT to any that
    // exist.
    for(partid=0; partid < bv->size; partid++) {
        if (bv->getBit(partid)) {
            // Get the actual tileid of the tile within the
            // partition that is responsible for addr
            tileid = mapAddrToTile(partid, addr);
            NETWORK->sendReqDirToTile(INT, addr, tileid);
        }
    }
}

/*
 * Dir::setState
 *     - This function serves to change the state of the Directory
 *       entry to s. If we are transitioning to an invalid state then
 *       there is some housekeeping to do.
 */
void Dir::setState(ulong addr, int s) {

    DirEntry * de = directory[BLKADDR(addr)];
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
    ulong blockaddr = BLKADDR(addr);

    if (directory[blockaddr] == NULL)
        directory[blockaddr] = new DirEntry(blockaddr);

    switch (msg) {
        case RD: 
            netInitRd(addr, fromtile);
            break;
        case RDX: 
            netInitRdX(addr, fromtile);
            break;
        case UPGR: 
            netInitUpgr(addr, fromtile);
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
void Dir::netInitRdX(ulong addr, ulong fromtile) {

    DirEntry * de = directory[BLKADDR(addr)];

    // Get the partition that the tile belongs to
    ulong partid = mapTileToPart(fromtile); 

    // For now we are going to assume we will just
    // get the data from memory
    CURRENTMEMDELAY += MEMATIME;

    switch (de->state) {

        // For EM we need to invalidate the current owner
        // and reply with data to new owner. Will stay in M state.
        case DSTATEEM: 
            // Invalidate current owner.
            invalidateSharers(addr, partid);
            // Reply Data
            NETWORK->fakeDataDirToTile(addr, fromtile);
            // Add new owner to bit map.
            de->sharers->setBit(partid);
            break;

        // For S we need to transition to M and invalidate all
        // sharers.
        case DSTATES: 
            // Invalidate all sharers
            invalidateSharers(addr, partid);
            // Add new owner to bit map.
            de->sharers->setBit(partid);
            // Transition to EM
            setState(addr, DSTATEEM);
            break;

        // For invalid state just transition to M
        case DSTATEI: 
            // Add new owner to bit map.
            de->sharers->setBit(partid);
            // Transition to EM
            setState(addr, DSTATEEM);
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
void Dir::netInitRd(ulong addr, ulong fromtile) {

    DirEntry * de = directory[BLKADDR(addr)];

    // Get the partition that the tile belongs to
    ulong partid = mapTileToPart(fromtile); 

    // For now we are going to assume we will just
    // get the data from memory
    CURRENTMEMDELAY += MEMATIME;

    switch (de->state) {

        // For EM we need to transistion to shared state 
        // and send an intervention to previous owner.
        case DSTATEEM: 
            // send intervention
            interveneOwner(addr);
            // Reply Data
            NETWORK->fakeDataDirToTile(addr, fromtile);
            // Add new sharer to bit map.
            de->sharers->setBit(partid);
            // Transition to S
            setState(addr, DSTATES);
            break;

        // For S, no need to change state
        case DSTATES: 
            // Reply Data
            NETWORK->fakeDataDirToTile(addr, fromtile);
            // Add new sharer to bit map.
            de->sharers->setBit(partid);
            break;

        // For I, transition to EM
        case DSTATEI: 
            // Reply Data
            NETWORK->fakeDataDirToTile(addr, fromtile);
            // Add new sharer to bit map.
            de->sharers->setBit(partid);
            // Transition to EM
            setState(addr, DSTATEEM);
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
void Dir::netInitUpgr(ulong addr, ulong fromtile) {

    DirEntry * de = directory[BLKADDR(addr)];
    // Get the partition that the tile belongs to
    ulong partid = mapTileToPart(fromtile); 

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
            invalidateSharers(addr, partid);
            // Reply - no data
            NETWORK->fakeReqDirToTile(addr, fromtile);
            // Transition to EM
            setState(addr, DSTATEEM);
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
