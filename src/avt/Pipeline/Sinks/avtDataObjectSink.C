// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtDataObjectSink.C                            //
// ************************************************************************* //

#include <avtDataObjectSink.h>

#include <DebugStream.h>

#include <chrono>
using namespace std::chrono;


// ****************************************************************************
//  Method: avtDataObjectSink constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataObjectSink::avtDataObjectSink()
{
    ;
}


// ****************************************************************************
//  Method: avtDataObjectSink destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataObjectSink::~avtDataObjectSink()
{
    ;
}


// ****************************************************************************
//  Method: avtDataObjectSink::SetInput
//
//  Purpose:
//      Sets the input of the data object.  This only takes care of bookkeeping
//      and passes the type checking on to the derived types.
//
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
// ****************************************************************************

void
avtDataObjectSink::SetInput(avtDataObject_p in)
{
    // auto start5 = high_resolution_clock::now(); // Start time 
    avtDataObject_p currentInput = GetInput();
    // auto stop5 = high_resolution_clock::now(); // Stop time
    // auto duration5 = duration_cast<milliseconds>(stop5 - start5); // Duration
    // std::cout << "\t\tGetInput took " << duration5.count() << " ms" << std::endl;

    // auto start6 = high_resolution_clock::now(); // Start time
    if (*in == *currentInput)
    {
        debug1 << "A data object sink's current input was fed back into "
               << "itself, ignoring..." << endl;
        return;
    }
    // auto stop6 = high_resolution_clock::now(); // Stop time
    // auto duration6 = duration_cast<milliseconds>(stop6 - start6); // Duration
    // std::cout << "\t\tif expr took " << duration6.count() << " ms" << std::endl;

    // auto start7 = high_resolution_clock::now(); // Start time 
    //
    // This will throw an exception if any problems occur, so we can just
    // assume that it works.
    //
    SetTypedInput(in);
    // auto stop7 = high_resolution_clock::now(); // Stop time
    // auto duration7 = duration_cast<milliseconds>(stop7 - start7); // Duration
    // std::cout << "\t\tSetTypedInput took " << duration7.count() << " ms" << std::endl;

    // JUSTIN this is the function that costs time; all others are instantaneous
    // auto start8 = high_resolution_clock::now(); // Start time 
    ChangedInput();
    // auto stop8 = high_resolution_clock::now(); // Stop time
    // auto duration8 = duration_cast<milliseconds>(stop8 - start8); // Duration
    // std::cout << "\t\tChangedInput took " << duration8.count() << " ms" << std::endl;
}


// ****************************************************************************
//  Method: avtDataObjectSink::ChangedInput
//
//  Purpose:
//      This is a hook for derived types to let them know that their input has
//      changed.  It is defined here to prevent all derived types from defining
//      it.
//
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
// ****************************************************************************

void
avtDataObjectSink::ChangedInput(void)
{
    ;
}


// ****************************************************************************
//  Method: avtDataObjectSink::UpdateInput
//
//  Purpose:
//      Updates the input.  This is defined here for all sinks that have one
//      input (ie all of them except the multiple input sink).
//
//  Arguments:
//      spec    The pipeline specification.
//
//  Returns:    Whether or not anything was modified up stream.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2001
//
// ****************************************************************************

bool
avtDataObjectSink::UpdateInput(avtContract_p spec)
{
    return GetInput()->Update(spec);
}


// ****************************************************************************
//  Method: avtDataObjectSink::ResetAllExtents
//
//  Purpose:
//      Resets the extents.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 5, 2017
//
// ****************************************************************************
void
avtDataObjectSink::ResetAllExtents()
{
    avtDataObject_p input = GetInput();
    if (*input != NULL)
        input->ResetAllExtents(); 
}


