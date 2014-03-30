#!/bin/bash

TAB="_tab"

SHARING=(
    0
    1
)


TRACES=(
#   CG_TRACE.t.100M
    CG_TRACE.t
    FT_TRACE.t
    BT_TRACE.t
)

PARTS=(
    1   # 1 Tile per partition
    2   # 2 Tiles per part
    4
    8
    16
)

for trace in ${TRACES[@]}; do
    file=~/Desktop/traces/$trace
    for sharing in ${SHARING[@]}; do
        for part in ${PARTS[@]}; do
            outfile="./${trace}_part${part}_share${sharing}${TAB}.txt"
            cmd="../sim $part $sharing $file $TAB"
            echo "$cmd > $outfile"
            $cmd > $outfile
        done
    done
done 
