/*
 * Dusty Mabe - 2014
 * PartTable.cc 
 *     - Function definitions for the partition table class.
 */

PartTable::PartTable() {

    // Create a new table. There can be at most NPROCS 
    // different partitions so we need that many rows 
    // in the table. 
    table = new int[NPROCS];

    // For now create generic mapping
    for (i=0; i < NPROCS; i++) {
        table[i] = (1 << i);
    }
}

PartTable::getFirstCPUinPart() {

}

PartTable::getNumCPUsInPart() {

}
