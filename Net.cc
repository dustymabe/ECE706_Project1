/*
 * Dusty Mabe - 2014
 * Net.cc - Implementation of interconnection network.
 */

#include "Net.h"
#include "Tile.h"
#include "types.h"

Net::Net(Tile ** tiless) {
    tiles = tiless;
}

ulong Net::sendToTile(ulong msg, ulong tileindex, ulong addr) {
    tiles[tileindex]->getFromNetwork(msg, addr);
    return 1;
}



////Bus::Bus(int nprocs, Cache ** array) {
////    cacheArray     = array;
////    num_processors = nprocs;

////}
////void Bus::transaction(Cache * cache, unsigned long addr, unsigned long val) {
////    int i;
////    cacheLine * line;
////    for(i=0; i<num_processors; i++) {

////        // skip the cache that initiated the transaction
////        if (cache == cacheArray[i])
////            continue;

////        // See if the cache has the block and if so send
////        // the bus transaction to the CCSM for that line.
////        line = cacheArray[i]->findLine(addr);
////        if (line)
////            line->ccsm->getFromBus(val);
////    }
////}

////int Bus::isBlockInAnotherCache(Cache * cache, unsigned long addr) {
////    int i;
////    cacheLine * line;
////    for(i=0; i<num_processors; i++) {

////        // skip the cache that asked
////        if (cache == cacheArray[i])
////            continue;

////        // See if the cache has the block and if so return true
////        line = cacheArray[i]->findLine(addr);
////        if (line && line->isValid())
////            return 1; // Block is in another cache
////    }

////    return 0; // Block is not in another cache
////}

////// With a mandatory flush we are flushing to memory.
////// We will bump the counter for flushing to mem and mark
////// the cache line as being clean.
////void Bus::flush(Cache *cache, cacheLine *line) {
////    cache->bumpFlushes();
////    line->setFlags(VALID); // Mark as not dirty
////    setFlushed(); // Set the bus indicator that a flush has occurred
////}


////// With an optional flush we are not flushing to memory but just
////// to other caches. Do not bump the flush count and don't mark it
////// as clean but do set the bus indicator that a flush has occurred.
////void Bus::flushOpt() {
////    setFlushed(); // Set the bus indicator that a flush has occurred
////}
