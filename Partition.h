/*
 * Dusty Mabe - 2014
 * Partition.h
 *     - Header file for the partition class. All a partition
 *       is is a bitvector that represents tiles within single
 *       partition. If a bit is '1' then the corresponding Tile
 *       is a member of the partition. If a bit is '0' then it is
 *       not.
 */

#ifndef PARTITION_H
#define PARTITION_H

#include "types.h"

class Partition {
protected:
    int vector;

   
public:
    Partition(int mapping);
    ~Partition();
    ulong getFirstCPUinPart();
    ulong getNumCPUsInPart();
    ulong getVector();
};

#endif

