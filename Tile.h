/*
 * Dusty Mabe - 2014
 * Tile.h - 
 */
#ifndef TILE_H
#define TILE_H

#include "types.h"

class Cache;     // Forward Declaration
class BitVector; // Forward Declaration


class Tile {
protected:

    Cache * l1cache;
    Cache * l2cache;
    BitVector * part;

////unsigned int xindex;
////unsigned int yindex;
   
public:
    unsigned int index;

    Tile(int number, int partition);
    ~Tile() {delete l1cache; delete l2cache; };
    void Access(ulong addr, uchar op);
    void L2Access(ulong addr, uchar op);
    void PrintStats();

    void broadcastToPartition(ulong msg, ulong addr);
    void getFromNetwork(ulong msg, ulong addr);
    int mapAddrToTile(ulong addr);
};

#endif

