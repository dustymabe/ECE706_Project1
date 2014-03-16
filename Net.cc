/*
 * Dusty Mabe - 2014
 * Net.cc - Implementation of interconnection network.
 */

#include "Net.h"
#include "Dir.h"
#include "Tile.h"
#include "types.h"

Net::Net(Dir * dirr, Tile ** tiless) {
    dir   = dirr;
    tiles = tiless;
}

ulong Net::sendTileToTile(ulong msg, ulong addr, ulong fromtile, ulong totile) {
    tiles[totile]->getFromNetwork(msg, addr);
    return 1;
}

ulong Net::sendTileToDir(ulong msg, ulong addr, ulong fromtile) {
    return dir->getFromNetwork(msg, addr, fromtile);
}

ulong Net::sendDirToTile(ulong msg, ulong addr, ulong totile) {
    tiles[totile]->getFromNetwork(msg, addr);
    return 1;
}

