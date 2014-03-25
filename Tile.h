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

   
public:
    unsigned int index;
    unsigned int xindex;
    unsigned int yindex;
    unsigned int cycle;
    unsigned int ctocxfer;
    unsigned int memxfer;

    Tile(int number, int partition);
    ~Tile() {delete l1cache; delete l2cache; };
    void Access(ulong addr, uchar op);
    void L2Access(ulong addr, uchar op);
    void PrintStats();

    void broadcastToPartition(ulong msg, ulong addr);
    int getFromNetwork(ulong msg, ulong addr, ulong fromtile);
    int mapAddrToTile(ulong addr);
};

#endif

