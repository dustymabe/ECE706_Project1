/*
 * Dusty Mabe - 2014
 * Tile.h - 
 */
#ifndef TILE_H
#define TILE_H

#include "types.h"

class Cache;     // Forward Declaration
class Partition; // Forward Declaration



// Define some message types that will be passed
// to/from the network to a tile.
enum{
    L1INV = 0,
    NETRDX,
    NETUPGR,
};


class Tile {
protected:

    Cache * l1cache;
    Cache * l2cache;
    Partition * part;

////unsigned int xindex;
////unsigned int yindex;
   
public:
    unsigned int index;

    Tile(int number);
    ~Tile() {delete l1cache; delete l2cache; };
    void Access(ulong addr, uchar op);
    void PrintStats();

    void broadCastToPartition(ulong msg, ulong addr);
    void getFromNetwork(ulong msg, ulong addr);
};

#endif

