#ifdef PARALLEL
#include <unistd.h>
#include <parallel.h>

// Variables to hold process size inforamyino
static int   par_rank = 0, par_size = 1;
MPI_Datatype PAR_STATEBUFFER;

// *******************************************************************
// Function: PAR_Rank
//
// Purpose:
//   Returns the process's rank within the process group.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 16:05:14 PST 2000
//
// Modifications:
//   
// *******************************************************************

int
PAR_Rank()
{
    return par_rank;
}

// *******************************************************************
// Function: PAR_UIProcess
//
// Purpose:
//   Returns true if the process's rank is 0.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 11:47:10 PDT 2000
//
// Modifications:
//   
// *******************************************************************

bool
PAR_UIProcess()
{
    return (par_rank == 0);
}

// *******************************************************************
// Function: PAR_Size
//
// Purpose:
//   Returns the size of the process group.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 16:04:41 PST 2000
//
// Modifications:
//   
// *******************************************************************

int
PAR_Size()
{
    return par_size;
}

// *******************************************************************
// Function: PAR_Init
//
// Purpose:
//   Initializes parallel state information.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 16:03:46 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
PAR_Init (int &argc, char **&argv)
{

    MPI_Init (&argc, &argv);

    //
    // Find the current process rank and the size of the process pool.
    //
    MPI_Comm_rank (MPI_COMM_WORLD, &par_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &par_size);

    //
    // Create the derived types and operators for sending messages
    // and collective operations.
    //
    PAR_CreateTypes();
}

// *******************************************************************
// Function: PAR_Exit
//
// Purpose:
//   Exits the program gracefully.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 16:02:58 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
PAR_Exit()
{
    MPI_Finalize();
}

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
