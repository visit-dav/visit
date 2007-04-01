// ************************************************************************* //
//                                Main.C                                     //
// ************************************************************************* //

//
// Includes
//

#include <MeshtvPrep.h>
#include <CommandLine.h>

#include <visitstream.h>
#include <new.h>
#include <stdlib.h>

#ifdef PARALLEL
    #include <mpi.h>
#endif


//
// Function Prototypes
//

static void    ExitHandler(void);
static void    NewHandler(void);


//
// Global Variables
//

#ifdef PARALLEL
  int    my_rank;
  int    num_processors;
#endif


// ****************************************************************************
//  Function: main
//
//  Purpose:
//      The main function for meshtvprep
//
//  Programmer: Hank Childs
//  Creation:   December 1, 1999
//
//  Modifications:
//
//      Hank Childs, Wed Jun 14 10:21:48 PDT 2000
//      Added barrier so no processor would finalize before the rest were done.
//
//      Brad Whitlock, Fri Jun 23 11:13:53 PDT 2000
//      Added a check to prevent processors other than rank 0 from printing
//      usage information. Also added call to MPI_Finalize before the exit
//      call so it doesn't look like the program crashed.
//
// ****************************************************************************

int main(int argc, char *argv[])
{
#ifdef PARALLEL
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processors);
#endif

    //
    // Set up handlers
    //
    atexit(ExitHandler);
    set_new_handler(NewHandler);

    CommandLine  cl;
    if (cl.Parse(argc, argv, cerr) < 0)
    {
#ifdef PARALLEL
        if(my_rank == 0)
            cl.Usage(cerr);
        MPI_Finalize();
        exit(EXIT_FAILURE);
#else
        cl.Usage(cerr);
        exit(EXIT_FAILURE);
#endif
    }

    //
    //  Do the preprocessing.  The MeshtvPrep object takes care of this.
    //
    MeshtvPrep  meshtvprep;
    meshtvprep.PreProcess(cl);

#ifdef PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
#endif
}


// ****************************************************************************
//  Function: ExitHandler
//
//  Purpose:  
//      Called at exit.  This will wrap up parallel issues if need be.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 1999
//
// ****************************************************************************

void
ExitHandler(void)
{
#ifdef PARALLEL
    //
    // This should take care of finalizing, but it just ends up causing 
    // problems for some MPI implementations.
    //
    //MPI_Finalize();
#endif
}


// ****************************************************************************
//  Function: NewHandler
//
//  Purpose:
//      Called if any new fails to obtain memory.  Currently does not try
//      to handle any errors, just prints a message and exits.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 1999
//
// ****************************************************************************

void 
NewHandler(void)
{
    cerr << "meshtvprep has run out of memory... (bailing out)" << endl;
    exit(EXIT_FAILURE);
}


