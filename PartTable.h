/*
 * Dusty Mabe - 2014
 * PartTable.h 
 *     - Header file for the partition table class. This 
 *       class keeps up with the partition table mappings that
 *       map which tiles map to what partitions.
 */

#ifndef PARTTABLE_H
#define PARTTABLE_H

class PartTable {
protected:
    int * table;

   
public:
    PartTable();
    ~PartTable();
    getFirstCPUinPart();
    getNumCPUsInPart();
    void Access();

};

#endif

