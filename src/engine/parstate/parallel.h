#ifndef PARALLEL_H
#define PARALLEL_H
#include <mpi.h>
#include <MPIXfer.h>
#include <BufferConnection.h>
#include <QuitRPC.h>

#define INPUT_BUFFER_SIZE 1000

typedef struct
{
    int  nbytes;
    char buffer[INPUT_BUFFER_SIZE];
} PAR_StateBuffer;

extern MPI_Datatype PAR_STATEBUFFER;

// Parallel Prototypes
void PAR_CreateTypes();
void PAR_Exit();
void PAR_Init(int &argc, char **&argv);
int  PAR_Rank();
int  PAR_Size();
bool PAR_UIProcess();

#endif
