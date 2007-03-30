// ************************************************************************* //
//                             MeshtvPrepLib.h                               //
// ************************************************************************* //

#ifndef MESHTV_PREP_LIB
#define MESHTV_PREP_LIB

#include <silo.h>

#ifdef PARALLEL
#include <mpi.h>
#endif


//
// Enumerated types
//

typedef enum
{
    STATE_FILE                = 0,
    MESH_FILE,             // = 1
    MAX_RESOURCES          // = 2
} RESOURCE_e;


//
// Function prototypes
//

void        CreateSiloOutputFile(char *);
void        ForceControlToRoot(void);
int         GetOutputFileNum(int, int);
void        GetNFileRange(int *, int *);
void        GetDomainRange(int, int *, int *);
DBfile     *GetSiloOutputFile(char *);
void        NumToString(char *, int, int, int);
void        SetNFiles(int);
void        SetOutputDirectory(char *);
void        SortListByNumber(char **, int);
void        ObtainResource(RESOURCE_e);
void        RelinquishResource(RESOURCE_e);
bool        RootWithinGroup(RESOURCE_e);
void        SplitAmongProcessors(int, int *, int *);

#ifdef PARALLEL
MPI_Comm   &GetCommunicator(RESOURCE_e);
#endif


#endif


