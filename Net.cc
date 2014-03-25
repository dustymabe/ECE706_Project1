/*
 * Dusty Mabe - 2014
 * Net.cc - Implementation of interconnection network.
 */

#include <stdlib.h>
#include <assert.h>
#include "Net.h"
#include "Dir.h"
#include "Tile.h"
#include "types.h"
#include "params.h"

// Global delay counter for the current outstanding memory request.
extern int CURRENTDELAY;

Net::Net(Dir * dirr, Tile ** tiless) {
    dir   = dirr;
    tiles = tiless;
}

ulong Net::sendReqTileToTile(ulong msg, ulong addr, ulong fromtile, ulong totile) {
    // Add in the delay
    if (fromtile != totile)
        CURRENTDELAY += HOPDELAY(calcTileToTileHops(fromtile, totile));

    // Service the request
    return tiles[totile]->getFromNetwork(msg, addr, fromtile);
}

ulong Net::sendReqDirToTile(ulong msg, ulong addr, ulong totile) {
    // Add in the delay
    CURRENTDELAY += HOPDELAY(calcTileToDirHops(addr, totile));
    // Service the request
    tiles[totile]->getFromNetwork(msg, addr, -1); // Use invalid tile
    return 1;
}

ulong Net::sendReqTileToDir(ulong msg, ulong addr, ulong fromtile) {
    // Add in the delay
    CURRENTDELAY += HOPDELAY(calcTileToDirHops(addr, fromtile));
    // Service the request
    return dir->getFromNetwork(msg, addr, fromtile);
}

ulong Net::fakeReqDirToTile(ulong addr, ulong totile) {
    // Add in the delay
    CURRENTDELAY += HOPDELAY(calcTileToDirHops(addr, totile));
    return 1;
}

ulong Net::fakeDataTileToTile(ulong fromtile, ulong totile) {
    // Add in the delay
    if (fromtile != totile)
        CURRENTDELAY += DATAHOPDELAY(calcTileToTileHops(fromtile, totile));
    return 1;
}

ulong Net::fakeDataDirToTile(ulong addr, ulong totile) {
    // Add in the delay
    CURRENTDELAY += DATAHOPDELAY(calcTileToDirHops(addr, totile));
    return 1;
}

ulong Net::flushToMem(ulong addr, ulong fromtile) {

    // Don't add mem access time to delay because
    // we don't need to wait.. just calculate time to
    // send data to mem.

    // Don't need to actually send a message to mem
    // just calculate # hops and then delay
    int hops  = calcTileToDirHops(addr, fromtile);
    int delay = DATAHOPDELAY(hops);

    CURRENTDELAY += delay; 
}

ulong Net::calcTileToDirHops(ulong addr, ulong tile) {

    int dirnum = BLKADDR(addr) % 4;
    int hops   = 0;
    switch (dirnum) {
        case 0:
            hops = calcTileToTileHops(tile, 0);
            break;
        case 1:
            hops = calcTileToTileHops(tile, 3);
            break;
        case 2:
            hops = calcTileToTileHops(tile, 12);
            break;
        case 3:
            hops = calcTileToTileHops(tile, 15);
            break;
        default :
            assert(0); // Should not get here.
    }

    return hops;
}

ulong Net::calcTileToTileHops(ulong fromtile, ulong totile) {
    int x0 = tiles[fromtile]->xindex;
    int y0 = tiles[fromtile]->yindex;
    int x1 = tiles[totile]->xindex;
    int y1 = tiles[totile]->yindex;
    int hops = abs(x1-x0) + abs(y1-y0);
    return hops;
}
