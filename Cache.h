/*
 * Dusty Mabe - 2014
 * cache.h - 
 */

#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>

#define L1 = 0
#define L2 = 1

#define  HIT = 0
#define MISS = 1

class Bus;  // Forward Declaration
class CCSM; // Forward Declaration

typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned int uint;

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
    CacheLine **cache;

    // Variable to hold the base addr of the last evicted block
    ulong victimAddr;
   
public:
    ulong currentCycle;  
    Bus *bus;
     
    Cache::Cache(int s, int a, int b, Bus *bs) {
    ~Cache() { delete cache;}

    CacheLine * fillLine(ulong addr);
    CacheLine * findLine(ulong addr);
    CacheLine * getLRU(ulong);

    ulong getRM()       { return readMisses;  }
    ulong getWM()       { return writeMisses; }
    ulong getReads()    { return reads;       }
    ulong getWrites()   { return writes;      }
    ulong getWB()       { return writeBacks;  }
    void writeBack()    { writeBacks++;       }

    ulong Access(ulong,uchar);
    void printStats(int proc);
    void printHeaderTabular(char *strHeader);
    void printStatsTabular(char *strStats, int proc); 
    void updateLRU(CacheLine *);

    ulong calcTag(ulong addr);
    ulong calcIndex(ulong addr);
};

#endif
