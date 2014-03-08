/*
 * Dusty Mabe - 2014
 * CCSM.cc - Implementation of a MESI Cache Coherence State Machine (CCSM).
 */

#include "CCSM.h"
#include "cache.h"

// What are these?
enum{
    BUSRD = 0,
    BUSRDX,
    BUSUPGR,
};

enum{
    STATEM = 0,
    STATEE,
    STATES,
    STATEI,
};


CCSM::CCSM(Bus *b, Cache *c, cacheLine *l) {
    bus   = b;
    cache = c;
    line  = l;
    state = STATEI;
}

void CCSM::setState(int s) {
    state = s; // Set the new state
}

void CCSM::evict() {
    // On eviction set the state to invalid
    // but don't run through the setState() function
    // because we don't want to increment counters for
    // evictions
    state = STATEI;
}


void CCSM::busInitBusRdX() {
    switch (state) {

        // For M we need to transistion to Invalid state and flush. 
        case STATEM: 
            bus->flush(cache, line);
            setState(STATEI);
            break;

        // For E&S we need to transistion to Invalid state and optionally flush. 
        case STATEE: 
        case STATES: 
            bus->flushOpt();
            setState(STATEI);
            break;

        // For invalid state do nothing
        case STATEI: 
            break;

        default :
            assert(0); // should not get here
    }
}

void CCSM::busInitBusRd() {
    switch (state) {

        // For M we need to transistion to Shared state and flush. 
        case STATEM: 
            bus->flush(cache, line);
            setState(STATES);
            break;

        // For E we transition to Shared and optionally flush
        case STATEE:
            bus->flushOpt();
            setState(STATES);
            break;
        
        // For S, no need to change state, but we can optionally flush
        case STATES: 
            bus->flushOpt();
            break;

        // Nothing to do for I
        case STATEI: 
            break;

        default :
            assert(0); // should not get here
    }
}

void CCSM::busInitBusUpgr() {
    switch (state) {
        // For M&E we should never get BUSUPGR since there
        // should not be more than one copy in the system
        case STATEM: 
        case STATEE: 
            assert(0);
            break;

        // For S, transistion to I
        case STATES:
            setState(STATEI);

        // For I, do nothing
        case STATEI: 
            break;

        default :
            assert(0); // should not get here
    }
}

void CCSM::procInitWr(unsigned long addr) {
    switch (state) {
        // Nothing to do for modified state
        case STATEM: 
            break;

        // For Exclusive just migrate to Modified state
        case STATEE:
            setState(STATEM);
            break;

        // For S need to send BUSUPGR and go to modified
        case STATES: 
            bus->transaction(cache, addr, BUSUPGR);
            setState(STATEM);
            break;

        // For I need to send BUSRDX and go to modified
        // Also need to check if a flush was performed and
        // bump cache-to-cache transfers if so.
        case STATEI: 
            bus->transaction(cache, addr, BUSRDX);
            if (bus->isFlushed())
                cache->bumpTransfers();
            setState(STATEM);
            break;

        default :
            assert(0); // should not get here
    }
}

void CCSM::procInitRd(unsigned long addr) {
    switch (state) {
        // Nothing to do for M&E&S states
        case STATEM: 
        case STATEE: 
        case STATES: 
            break;

        // For I:
        //  If no other proc has block go to Exclusive
        //  else go to Shared
        //
        // Also need to check if a flush was performed and
        // bump cache-to-cache transfers if so.
        case STATEI: 
            if (!bus->isBlockInAnotherCache(cache, addr))
                setState(STATEE);
            else
                setState(STATES);

            bus->transaction(cache, addr, BUSRD);

            if (bus->isFlushed())
                cache->bumpTransfers();
            break;

        default :
            assert(0); // should not get here
    }
}

void CCSM::getFromBus(unsigned long action) {
    switch (action) {
        case BUSRD: 
            busInitBusRd();
            break;
        case BUSRDX: 
            busInitBusRdX();
            break;
        case BUSUPGR: 
            busInitBusUpgr();
            break;
        default :
            assert(0); // should not get here
    }
}
