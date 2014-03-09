/*
 * Dusty Mabe - 2014
 * CCSM.h - Header file for Cache Coherence State Machine for
 *          the MESI protocol.
 */
#ifndef CCSM_H
#define CCSM_H

class Cache;     // Forward Declaration
class CacheLine; // Forward Declaration
class Tile;      // Forward Declaration

class CCSM {
    private:
    public:
        int state;
        Cache * cache;
        CacheLine * line;
        Tile * tile;

        CCSM(Tile *t, Cache *c, CacheLine *l); 
       // CCSM(Bus *b, Cache *c, cacheLine *l); 
        ~CCSM();
        void setState(int s);
        void evict();
        void getFromBus(unsigned long action);
        void busInitBusRd();
        void busInitBusRdX();
        void busInitBusUpgr();
        void procInitRd(unsigned long addr);
        void procInitWr(unsigned long addr);
};

#endif
