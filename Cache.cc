/*
 * Dusty Mabe - 2014
 * cache.cc - Implementation of cache (either L1 or L2)
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <cmath>
#include "cache.h"
#include "bus.h"
#include "CCSM.h"
using namespace std;

/*
 * Cache::Cache - create a new cache object.
 * Arguments:
 *      - l - the cache level (L1 or L2)
 *      - s - the size of the cache
 *      - a - the associativity of the cache
 *      - b - the size of each block (line) in the cache
 *
 */
Cache::Cache(int l, int s, int a, int b, Bus *bs) {

    CCSM * ccsm;
    ulong i, j;

    // Initialize all counters
    currentCycle = 0;
    writeBacks   = 0;
    readMisses   = 0;
    writeMisses  = 0;
    reads = writes = 0;

    // Initialize the victimAddr variable
    victimAddr = 0;

    // Process arguments
    cacheLevel = l;
    size       = (ulong)(s);
    lineSize   = (ulong)(b);
    assoc      = (ulong)(a);       // assoc = # of lines within a set 
    numSets    = (ulong)((s/b)/a); // How many sets in a cache?
    numLines   = (ulong)(s/b);     // How many lines in the cache?

    indexbits  = log2(numSets);   //log2 from cmath
    offsetbits = log2(lineSize);  //log2 from cmath
    tagbits    = 32 - indexbits - offsetbits;
    
    // Generate a bit mask that will mask off all of the 
    // tag bits from the address when ANDed with the addr.
    // This will generate something like 0b11110000...
    tagMask  = 1 << (indexbits + offsetbits);
    tagMask -= 1;
  
    // create a two dimentional cache, sized as cache[sets][assoc]
    cacheLines = new CacheLine*[numSets];
    for (i=0; i < numSets; i++) {
        cacheLines[i] = new CacheLine[assoc];
    }

    // If this is an L2 cache then we will create
    // a CCSM for each cache line. Since our L1 is write-through
    // we don't need a CCSM for L1 and can just keep up with the
    // state at the L2 cache. 
    if (cacheLevel == L2)
        for (i=0; i < numSets; i++)
            for (j=0; j< assoc; j++) {
                ccsm = new MESI(bus, this, &(lines[i][j]));
                lines[i][j].init(ccsm);
            }
}

/*
 * Cache::calcTag
 *     - Return the tag from addr. This is done by
 *       right shifting (indexbits + offsetbits) from
 *       the address so that you are just left with the
 *       tag bits. 
 */
void Cache::calcTag(ulong addr) {
    return (addr >> (indexbits + offsetbits));
}

/*
 * Cache::calcIndex
 *     - Return the index from addr. This is done by 
 *       first masking off the tag bits (using the 
 *       previously calculated tagMask) and then right 
 *       shifting (offsetbits) from the address so that 
 *       we are just left with the index bits. 
 */
void Cache::calcIndex(ulong addr) {
    return ((addr & tagMask) >> offsetbits);
}


/*
 * Cache::Access
 *     - Perform an access (r/w) to a particular addr. 
 *
 * Returns MISS if miss
 * Returns HIT  if hit
 */
void Cache::Access(ulong addr, uchar op) {
    CacheLine * line;
    int state;

    // Per cache global counter to maintain LRU order
    // among cache ways; updated on every cache access.
    currentCycle++;

    // Clear the bus indicator that a flush has been performed
    bus->clearFlushed();
            
    // Update w/r counters
    if (op == 'w')
        writes++;
    else
        reads++;
    
    // See if the block that contains addr is already 
    // in the cache. 
    if (!(line = findLine(addr)))
        state = MISS;
    else
        state = HIT;

    // If not in the cache then fetch it and
    // update the counters
    if (state == MISS) {
        line = fillLine(addr);
        if (op == 'w') 
            writeMisses++;
        else
            readMisses++;
    }

    // If a write then set the flag to be DIRTY
    if (op == 'w')
        line->setFlags(DIRTY);    

    // If cache hit then update LRU
    if (state == HIT)
        updateLRU(line);

    // Update the cache coherence protocol state machine
    // for this line in the cache
    if (op == 'w')
        line->ccsm->procInitWr(addr);
    else
        line->ccsm->procInitRd(addr);

    // Return an indication of if we hit or miss.
    return state;
}

/*
 * Cache::findLine
 *     - Find a line within the cache that corresponds
 *       to the address addr. 
 *
 * Returns a CacheLine object or NULL if not found.
 */
CacheLine * Cache::findLine(ulong addr) {
    ulong index, j, tag;

    // Calculate tag and index from addr
    tag   = calcTag(addr);   // Tag value
    index = calcIndex(addr); // Set index
  
    // Iterate through set to see if we have a hit.
    for(j=0; j<assoc; j++) {

        // If not valid then continue
        if (cache[index][j].isValid() == 0)
            continue;

        // Does the tag match.. If so then score!
        if (cache[index][j].getTag() == tag) {
            return &(cache[index][j]);
        }
    }

    // If we made it here then !found. 
    return NULL;
}

/*
 * Cache::updateLRU
 *     - Update the sequence for line to be 
 *       the current cycle.
 */
void Cache::updateLRU(CacheLine *line) {
    line->setSeq(currentCycle);  
}

/*
 * Cache::getLRU
 *     - Get the LRU cache line for the set that addr 
 *       maps to. If an invalid line exists in the set
 *       then return it. If not then choose the LRU line
 *       as the victim and return it.
 *
 * Returns a CacheLine object that represents the victim.
 */
CacheLine * Cache::getLRU(ulong addr) {
    ulong index, j, victim, min;

    // set victim = assoc (an impossible value)
    victim = assoc;

    // set min to currentCycle (max possible seq)
    min   = currentCycle;

    // Calculate set index
    index = calcIndex(addr);
   
    // First see if there are any invalid blocks
    for(j=0;j<assoc;j++) { 
      if(cache[index][j].isValid() == 0)
          return &(cache[index][j]);     
    }   

    // No invalid lines. Find LRU. 
    for(j=0;j<assoc;j++) {
        if (cache[index][j].getSeq() <= min) { 
            victim = j; 
            min = cache[index][j].getSeq();
        }
    } 
    
    // Verify a victim was found
    assert(victim != assoc);

    // Return the victim
    return &(cache[index][victim]);
}

/*
 * Cache::fillLine
 *     - Allocate a new line in the cache for the block
 *       that contains addr.
 *
 * Returns a CacheLine object that represents the filled line.
 */
CacheLine *Cache::fillLine(ulong addr) { 
    ulong tag;
    CacheLine *victim;

    // Get the LRU block (or invalid block)
    victim = getLRU(addr);
    assert(victim);

    // If the chosen victim is valid then update the cache's
    // victimAddr variable so that the Tile can know there was an
    // eviction and can send invalidations to the L1s
    if (victim->isValid())
        victimAddr = ((victim->tag << (indexbits)) & victim->index) << offsetbits;

    // If the chosen victim is valid then mark as invalid 
    // in the CCSM
    if (victim->isValid())
        victim->ccsm->evict();

    // If the chosen victim is dirty then update writeBack
    if (victim->getFlags() == DIRTY)
        writeBack();

    // Since we are placing data into this line
    // then update the LRU information to indicate
    // it was accessed this cycle.
    updateLRU(victim);

    // Update information for this cache line.
    tag   = calcTag(addr);   
    index = calcIndex(addr);   
    victim->setTag(tag);
    victim->setIndex(index);
    victim->setFlags(VALID);    

    return victim;
}
