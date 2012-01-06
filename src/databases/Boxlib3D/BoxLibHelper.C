#include "BoxLibHelper.H"
#include <BoxLib.H>

// We need to call BoxLib::Initialize without BL_USE_MPI being set to 1 (as done
// in avtBoxLibFileFormat.C to avoid conflicts between the MPI implementation
// VisIt uses and BoxLib's serial dummy MPI implementation) as this potentially
// changes the signature of this function (different types for MPI communicator).

void BoxLibHelper::InitializeBoxLib()
{
    int dummyArgC = 1;
    char *dummyArgV[] = { "visit" };
    char **argv = dummyArgV; // Avoid implicit type conversion
    BoxLib::Initialize(dummyArgC, argv);
}
