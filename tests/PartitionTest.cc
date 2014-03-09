
#include <stdio.h>
#include "Partition.h"


int main() {

    Partition * p1;
    Partition * p2;

    p1 = new Partition(1<<10);

    printf("First CPU  in p1: %d\n", p1->getFirstCPUinPart());
    printf("Num   CPUs in p1: %d\n", p1->getNumCPUsInPart());

    p2 = new Partition(0xf0);

    printf("First CPU  in p2: %d\n", p2->getFirstCPUinPart());
    printf("Num   CPUs in p2: %d\n", p2->getNumCPUsInPart());

}
