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

class Tile; // Forward Declaration


class Net {
private:
    Tile ** tiles;



public:
    Net(Tile ** tiless);
    ~Net();

    ulong sendToTile(ulong msg, ulong tileindex, ulong addr);
    //ulong sendToMem(ulong msg, ulong addr);

};

//Net *NETWORK;

//  class Bus {
//  private:
//      int num_processors;
//      int flushed;
//      Cache ** cacheArray;

//  public:
//      Bus(int nprocs, Cache ** array);
//      ~Bus();
//      int isFlushed()     { return flushed; }
//      void setFlushed()   { flushed = 1;    }
//      void clearFlushed() { flushed = 0;    }
//      void transaction(Cache * cache, unsigned long addr, unsigned long val);
//      int  isBlockInAnotherCache(Cache * cache, unsigned long addr);
//      void flush(Cache *cache, cacheLine *line);
//      void flushOpt();
//  };

#endif
