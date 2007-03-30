// ************************************************************************* //
//                            avtDataObjectSink.C                            //
// ************************************************************************* //

#include <avtDataObjectSink.h>

#include <DebugStream.h>


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
    avtDataObject_p currentInput = GetInput();
    if (*in == *currentInput)
    {
        debug1 << "A data object sink's current input was fed back into "
               << "itself, ignoring..." << endl;
        return;
    }

    //
    // This will throw an exception if any problems occur, so we can just
    // assume that it works.
    //
    SetTypedInput(in);

    ChangedInput();
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
avtDataObjectSink::UpdateInput(avtPipelineSpecification_p spec)
{
    return GetInput()->Update(spec);
}


