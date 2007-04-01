#ifdef PARALLEL
#include <unistd.h>
#include <parallel.h>

MPI_Datatype PAR_STATEBUFFER;

// *******************************************************************
// Function: PAR_CreateTypes
//
// Purpose:
//   Creates an MPI type that represents the buffer that is
//   communicated when new state information is sent to non-UI processes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 16:00:57 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
PAR_CreateTypes()
{
    PAR_StateBuffer buf;
    MPI_Aint        displacements[2];
    MPI_Datatype    types[2];
    int                lengths[2];

    //
    // Create a type matching the PAR_StateBuffer structure.  
    //
    MPI_Address (&buf.nbytes, &displacements[0]);
    MPI_Address (&buf.buffer, &displacements[1]);
    for (int i = 1; i >= 0; i--)
        displacements[i] -= displacements[0];

    types[0] = MPI_INT;
    types[1] = MPI_CHAR;
    lengths[0] = 1;
    lengths[1] = INPUT_BUFFER_SIZE;

    MPI_Type_struct (2, lengths, displacements, types, &PAR_STATEBUFFER);
    MPI_Type_commit (&PAR_STATEBUFFER);
}
#endif
