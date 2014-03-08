/*
 * Dusty Mabe - 2014
 * cache.h - 
 */
#ifndef CACHE_H
#define CACHE_H

#include "types.h"
#include "CacheLine.h"

#define L1 0
#define L2 1

#define  HIT 0
#define MISS 1

class CCSM; // Forward Declaration

class Cache {
protected:
    // Some parameters
    ulong size, lineSize, assoc;
    ulong numSets, tagMask, numLines;
    ulong indexbits, offsetbits, tagbits;
    ulong cacheLevel; //L1 or L2

    // Some counters
    ulong reads, readMisses;
    ulong writes, writeMisses;
    ulong flushes, writeBacks;
    ulong interventions, invalidations;
    ulong transfers;

    // The 2-dimensional cache
    CacheLine **cacheArray;

    // Variable to hold the base addr of the last evicted block
    ulong victimAddr;
   
public:
    ulong currentCycle;  
     
    Cache(int l, int s, int a, int b);
    ~Cache() { delete cacheArray;}

    CacheLine * fillLine(ulong addr);
    CacheLine * findLine(ulong addr);
    CacheLine * getLRU(ulong);

    ulong getRM()       { return readMisses;  }
    ulong getWM()       { return writeMisses; }
    ulong getReads()    { return reads;       }
    ulong getWrites()   { return writes;      }
    ulong getWB()       { return writeBacks;  }
    void writeBack()    { writeBacks++;       }

    ulong getVictimAddr()        { return victimAddr;  }
    void  setVictimAddr(ulong a) { victimAddr = a;     }

    ulong Access(ulong, uchar);
    void PrintStats();
    void PrintHeaderTabular(char *strHeader);
    void PrintStatsTabular(char *strStats, int proc); 
    void updateLRU(CacheLine *);

    ulong calcTag(ulong addr);
    ulong calcIndex(ulong addr);
};

#endif
