/*
 * Dusty Mabe - 2014
 * params.h
 *     - Define common parameters for the caches.
 */
#ifndef PARAMS_H
#define PARAMS_H

#define ONEKBYTE 1024           // 1024 bytes
#define L1SIZE (32 *  ONEKBYTE) // 32  KiB  
#define L2SIZE (256 * ONEKBYTE) // 256 KiB  
#define L1ASSOC 8    // 8 cache lines in a set  
#define L2ASSOC 8    // 8 cache lines in a set
#define BLKSIZE 64   // 64 bytes
#define OFFSETBITS 6 // 6 bits (64 = 2^6)
#define NPROCS  16   // 16 procs
#define SQRTNPROCS  4 // Tiles will be in SQRTNPROCSxSQRTNPROCS matrix

#endif
