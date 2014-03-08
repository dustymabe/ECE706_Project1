/*
 * Dusty Mabe - 2014
 * main.cc
 *
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fstream>
#include "bus.h"
#include "cache.h"


#define ONEKBYTE 1024           // 1024 bytes
#define L1SIZE (32 *  ONEKBYTE) // 32  KiB  
#define L2SIZE (256 * ONEKBYTE) // 256 KiB  
#define L1ASSOC 8  // 8 cache lines in a set  
#define L2ASSOC 8  // 8 cache lines in a set
#define BLKSIZE 64 // 64 bytes
#define NPROCS  16 // 16 procs



using namespace std;

int main(int argc, char *argv[]) {
    
    int i;
    FILE * fp;
    char buf[256];
    char * token;
    char * op;
    int   proc;
    int   tabular;
    ulong addr;
    Bus * bus;
    Cache ** cacheArray;
    char delimit[4] = " \t\n"; // tokenize based on "space", "tab", eol
    char strHeader[2048];
    char strStats[2048];

    if (argv[1] == NULL) {
        printf("input format: ");
        printf("./smp <L1_size> <L2_size> <assoc> <block_size> <#procs> <trace_file> <partitions> \n");
        exit(0);
    }

    int l1size = atoi(argv[1]);
    int l2size = atoi(argv[2]);
    int assoc  = atoi(argv[3]);
    int blksize= atoi(argv[4]);
    int nprocs = atoi(argv[5]); // Only going to use 16

    char *fname =  (char *)malloc(100);
    fname = argv[6];

    if (argv[7] == NULL)
        tabular = 0;
    else
        tabular = 1;

#define L1SIZE (32 *  ONEKBYTE) // 32  KiB  
#define L2SIZE (256 * ONEKBYTE) // 256 KiB  
#define L1ASSOC 8  // 8 cache lines in a set  
#define L2ASSOC 8  // 8 cache lines in a set
#define BLKSIZE 64 // 64 bytes
#define NPROCS  16 // 16 procs

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


 
    // Create an array to hold a cache object for each proc
    cacheArray = new Cache * [nprocs];

    // Create a new bus
    bus = new Bus(num_processors, cacheArray);

    // Create a cache object for each proc
    for(i=0; i < nprocs; i++)
        cacheArray[i] = new Cache(cache_size, cache_assoc, blk_size, protocol, bus);

    
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
        assert(proc < num_processors);
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
        //printf("address is: %x\n", addr);

        Tiles[proc]->Access(addr, op[0]);

    }
    fclose(fp);

    // Print out statistics for each cache

    // If asked to print out stats in tabular form
    // then do so, else print out normal output.
    if (tabular) {
        cacheArray[0]->printHeaderTabular(strHeader);
        for(i=0; i<num_processors; i++)
            cacheArray[i]->printStatsTabular(strStats, i);
        printf("%s\tTotals\n", strStats);
    } else {
        for(i=0; i<num_processors; i++)
            cacheArray[i]->printStats(i);
    }
    
}
