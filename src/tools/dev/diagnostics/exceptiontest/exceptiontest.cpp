#include <stdio.h>
#include <string.h>

#include <DebugStream.h>
#include <VisItInit.h>
#include <ImproperUseException.h>
#include <SiloException.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

// ****************************************************************************
// Class: Tester
//
// Purpose:
//   This Tester class tests throwing and catching exceptions.
//
// Notes:    Some compilers suck at exceptions or generate code that will abort
//           at runtime. Let's see if we have a compiler like that.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 14:06:07 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class Tester
{
public:
    Tester()
    {
    }

    ~Tester()
    {
    }

    bool Initialize(int *argc, char **argv[])
    {
        return true;
    }

    bool Execute()
    {
        // Simple try/catch.
        TRY
        {
            EXCEPTION0(VisItException);
        }
        CATCH(VisItException)
        {
            debug5 << "Caught VisItException." << endl;
        }
        ENDTRY

        // Try/catch with argument, catch base class exception type.
        TRY
        {
            EXCEPTION1(ImproperUseException, "argument");
        }
        CATCH2(VisItException, e)
        {
            debug5 << "Caught ImproperUseException: " << e.Message() << endl;
        }
        ENDTRY

        // Try/catch with argument, provide different catch statements.
        TRY
        {
            EXCEPTION1(SiloException, "Invalid file");
        }
        CATCH2(DatabaseException, e)
        {
            debug5 << "Caught DatabaseException: " << e.GetExceptionType()
                   << ":"<< e.Message() << endl;
        }
        CATCH2(VisItException, e)
        {
            debug5 << "Caught " << e.GetExceptionType() << ": " << e.Message() << endl;
            // If we got here then exceptions are not working.
            CATCH_RETURN2(1,false);
        }
        ENDTRY

        // Nested try/catch, rethrow.
        TRY
        {
            TRY
            {
                EXCEPTION1(SiloException, "Invalid file");
            }
            CATCHALL
            {
                debug5 << "Rethrowing the exception." << endl;
                RETHROW;
            }
            ENDTRY
        }
        CATCH2(DatabaseException, e)
        {
            debug5 << "Caught DatabaseException: " << e.GetExceptionType()
                   << ":" << e.Message() << endl;
        }
        CATCH2(VisItException, e)
        {
            debug5 << "Caught " << e.GetExceptionType() << ": " << e.Message() << endl;
            // If we got here then exceptions are not working.
            CATCH_RETURN2(1, false);
        }
        ENDTRY

        return true;
    }
};


int
main(int argc, char *argv[])
{
    int par_rank = 0, par_size = 1;
#ifdef PARALLEL
    /* Init MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &par_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &par_size);
#endif
    Tester app;

    // Initialize
    VisItInit::SetComponentName(argv[0]);
    VisItInit::Initialize(argc, argv, par_rank, par_size, true, true);

    bool retval = false;
    if(app.Initialize(&argc, &argv))
    {
        retval = app.Execute();
    }

    VisItInit::Finalize();

#ifdef PARALLEL
    /* Finalize MPI */
    MPI_Finalize();
#endif

    if(par_rank == 0)
    {
        cout << (retval ? "Success" : "Failure") << endl;
    }

    return retval ? 0 : -1;
}
