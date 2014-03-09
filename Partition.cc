/*
 * Dusty Mabe - 2014
 * Partition.cc 
 *     - Function definitions for the partition class.
 */

#include "Partition.h"
#include "params.h"

Partition::Partition(int mapping) {

    // Set the bitvector equal to the provided
    // mapping
    vector = mapping;
}

ulong Partition::getFirstCPUinPart() {
    int i;

    for(i=0; i < NPROCS; i++) {
        if (vector & (1 << i))
            return i;
    }
}

ulong Partition::getNumCPUsInPart() {
    int i;
    int count = 0;
    for(i=0; i < NPROCS; i++) {
        if (vector & (1 << i))
            count++;
    }
    return count;
}

/*
 * Partition::getVector
 *     - Return a copy of the raw partition vector for
 *       callers that want to operate on it themselves.
 */
ulong Partition::getVector() {
    return vector;
}
