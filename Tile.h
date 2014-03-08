/*
 * Dusty Mabe - 2014
 * Tile.h - 
 */
#ifndef TILE_H
#define TILE_H

#include "Cache.h"
#include "types.h"

class Tile {
protected:

    Cache * l1cache;
    Cache * l2cache;
////partTable tbl;

    unsigned int index;
////unsigned int xindex;
////unsigned int yindex;
   
public:
    Tile(int number);
    ~Tile() {delete l1cache; delete l2cache; };
    void Access(ulong addr, uchar op);
    void PrintStats();
};

#endif

