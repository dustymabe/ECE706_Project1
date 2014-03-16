/*
 * Dusty Mabe - 2014
 * Net.h - Header file for an implementation of an interconnection
 *         network. Caches and Mem Controllers will pass messages to
 *         the network and they will be delivered to other caches and
 *         memory contorllers. 
 *        
 *         The network will return the amount of time it took to retrieve
 *         the value.
 *
 *         The network is global in that everything will have access to
 *         it without having to explicity have it be a part of their
 *         class data. 
 */
#ifndef NET_H
#define NET_H

#include "types.h"

class Dir;  // Forward Declaration
class Tile; // Forward Declaration


// Message types to be passed back and forth over the network. 
// Messages go between Directory and Tiles (L2 CCSM), between two
// Tiles, etc..
enum {

    // Dir -> Tile (L2 CCSM) messages
    INV=100, // Invalidation
    INT,     // Intervention

    // Tile (L2 CCSM) -> Dir messages
    RD,      // Read
    RDX,     // Read Exclusive
    UPGR,    // Upgrade from S to M

    // Tile (L2) -> Tile (L1) messages
    // Note: happens when L2 line is evicted
    L1INV,
};


class Net {
private:
    Tile ** tiles;
    Dir  *  dir;

public:
    Net(Dir * dirr, Tile ** tiless);
    ~Net();
    ulong sendTileToTile(ulong msg, ulong addr, ulong fromtile, ulong totile);
    ulong sendTileToDir(ulong msg, ulong addr, ulong fromtile);
    ulong sendDirToTile(ulong msg, ulong addr, ulong totile);
};

#endif
