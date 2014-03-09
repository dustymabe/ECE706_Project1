/*
 * Dusty Mabe - 2014
 * main.cc
 *
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fstream>
#include "Cache.h"
#include "Tile.h"
#include "Net.h"
#include "params.h"

Net *NETWORK;


int main(int argc, char *argv[]) {
    
    int i;
    FILE * fp;
    char buf[256];
    char * token;
    char * op;
    int   proc;
    int   tabular;
    ulong addr;
    Cache ** cacheArray;
    char delimit[4] = " \t\n"; // tokenize based on "space", "tab", eol
    char strHeader[2048];
    char strStats[2048];

    // Check input
    if (argv[1] == NULL) {
        printf("input format: ");
        printf("./sim <partitions> <trace_file> \n");
        exit(1);
    }

    // Store the filename
    char *fname =  (char *)malloc(100);
    fname = argv[2];

////if (argv[7] == NULL)
        tabular = 0;
////else
////    tabular = 1;


    // Print out the simulator configuration (if not tabular)
    if (!tabular) {
        printf("===== 706 SMP Simulator Configuration =====\n");
        printf("L1_SIZE:                        %d\n", L1SIZE);
        printf("L1_ASSOC:                       %d\n", L1ASSOC);
        printf("L2_SIZE:                        %d\n", L2SIZE);
        printf("L2_ASSOC:                       %d\n", L2ASSOC);
        printf("BLOCKSIZE:                      %d\n", BLKSIZE);
        printf("NUMBER OF PROCESSORS:           %d\n", NPROCS);
        printf("COHERENCE PROTOCOL:             %s\n", "MESI");
        printf("TRACE FILE:                     %s\n", basename(fname));
    } 
////else {
////    sprintf(strHeader, "csize\tassoc\tblk\tnprocs\tprot\tvrun");
////    sprintf(strStats, "%d\t%d\t%d\t%d\t%s\t%s", cache_size, cache_assoc,
////        blk_size, num_processors, CCPROTOCOLS[protocol], basename(fname));
////}


    // Create a 4x4 array of Tiles here
    Tile * tiles[NPROCS];
    for (i=0; i < NPROCS; i++)
        tiles[i] = new Tile(i);


    NETWORK = new Net(tiles);


 
////// Create an array to hold a cache object for each proc
////cacheArray = new Cache * [nprocs];

////// Create a new bus
////bus = new Bus(num_processors, cacheArray);

////// Create a cache object for each proc
////for(i=0; i < nprocs; i++)
////    cacheArray[i] = new Cache(cache_size, cache_assoc, blk_size, protocol, bus);

    // Open the trace file
    fp = fopen(fname,"r");
    if (fp == 0) {   
        printf("Trace file problem\n");
        exit(0);
    }

    // Read each line of trace file and call Access() for each entry.
    // Each line in the tracefile is of the form:
    //       processor(0-7) operation(r,w) address(8 hexa chars)
    //
    //      0 r 7fc61248
    //      0 w 7fc62c08
    //      0 r 7fc63738
    //
    //
    while (fgets(buf, 1024, fp)) {

        // The proc # is the first item on the line
        token = strtok(buf, delimit);
        assert(token != NULL);
        proc = atoi(token);
        assert(proc < NPROCS);
        //printf("processor is %d\n", proc);

        // The "operation" is next
        // NOTE: passing NULL to strtok here because
        //       we want to operate on same string
        token = strtok(NULL, delimit);
        assert(token != NULL);
        op = token;
        //printf("operation is: %s\n", op);

        // The mem addr is last
        // NOTE: passing NULL to strtok here because
        //       we want to operate on same string
        token = strtok(NULL, delimit);
        assert(token != NULL);
        addr = strtoul(token, NULL, 16);
        //printf("address is: %x\n", (uint) addr);

        tiles[proc]->Access(addr, op[0]);

    }
    fclose(fp);

    // Print out statistics for each cache

    // If asked to print out stats in tabular form
    // then do so, else print out normal output.
////if (tabular) {
////    cacheArray[0]->printHeaderTabular(strHeader);
////    for(i=0; i<num_processors; i++)
////        cacheArray[i]->printStatsTabular(strStats, i);
////    printf("%s\tTotals\n", strStats);
////} else {
////    for(i=0; i<num_processors; i++)
////        cacheArray[i]->printStats(i);
////}
    
    for (i=0; i < NPROCS; i++)
        tiles[i]->PrintStats();
}
