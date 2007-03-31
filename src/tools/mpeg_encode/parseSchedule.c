/*

SCHEDULE:  MACHINE 0 FRAMES 0-2 TIME 750230115-750230122
SCHEDULE:  MACHINE 1 FRAMES 3-5 TIME 750230115-750230124
SCHEDULE:  MACHINE 1 FRAME 18-27 TIME 750230124-750230147
SCHEDULE:  MACHINE 1 FRAME 28-29 TIME 750230147-750230147
SCHEDULE:  MACHINE 0 FRAME 6-17 TIME 750230122-750230148

*/

#include <stdio.h>

int main(int argc, char **argv)
{
    register int index;
    char input[256];
    long    IOtimes[256];
    int machineNum;
    int frameStart, frameEnd;
    long timeStart, timeEnd;
    long    minTime = 0x7fffffff;
    FILE    *filePtr;
    long    IOtime;

    for ( index = 0; index < 256; index++ )
    IOtimes[index] = 0;

    filePtr = fopen(argv[1], "r");

    while ( fgets(input, 256, filePtr) != NULL )
    {
    sscanf(input, "SCHEDULE:  MACHINE %d FRAMES %d-%d TIME %ld-%ld IOTIME %ld",
           &machineNum, &frameStart, &frameEnd, &timeStart, &timeEnd,
           &IOtime);

    if ( timeStart < minTime )
        minTime = timeStart;
    }

    fclose(filePtr);

    filePtr = fopen(argv[1], "r");

    while ( fgets(input, 256, filePtr) != NULL )
    {
    sscanf(input, "SCHEDULE:  MACHINE %d FRAMES %d-%d TIME %ld-%ld IOTIME %ld",
           &machineNum, &frameStart, &frameEnd, &timeStart, &timeEnd,
           &IOtime);

    fprintf(stdout, "SCHEDULE:  MACHINE %2d FRAMES %2d-%2d TIME %3d-%3d IOTIME %2d F/T/I %2d/%2d/%2d\n",
        machineNum, frameStart, frameEnd,
        timeStart-minTime, timeEnd-minTime,
        IOtime-IOtimes[machineNum],
        frameEnd-frameStart+1,
        timeEnd-timeStart,
        IOtime-IOtimes[machineNum]);

    IOtimes[machineNum] = IOtime;
    }

    fclose(filePtr);

    return 0;
}
