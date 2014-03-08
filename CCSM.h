// Cache Coherence State Machine Classes (CCSM.h)



// Make sure this file only gets sourced in once by using #ifndef 
#ifndef CCSM_H
#define CCSM_H

class Bus;   // Forward Declaration
class Cache; // Forward Declaration
class cacheLine; // Forward Declaration



class CCSM {
    private:
    public:
        int state;
        Net * net;
        Cache * cache;
        cacheLine * line;

        CCSM();
        CCSM(Bus *b, Cache *c, cacheLine *l); 
        ~CCSM();
        void setState(int s);
        void evict();
        virtual void procInitWr(unsigned long addr) =0;
        virtual void procInitRd(unsigned long addr) =0;
        virtual void getFromBus(unsigned long action) =0;
        void getFromBus(unsigned long action);
        void busInitBusRd();
        void busInitBusRdX();
        void busInitBusUpgr();
        void procInitRd(unsigned long addr);
        void procInitWr(unsigned long addr);
};

#endif
