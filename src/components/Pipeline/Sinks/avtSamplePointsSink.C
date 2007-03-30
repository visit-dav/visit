// ************************************************************************* //
//                          avtSamplePointsSink.C                            //
// ************************************************************************* //

#include <avtSamplePointsSink.h>

#include <DebugStream.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtSamplePointsSink constructor
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
// ****************************************************************************

avtSamplePointsSink::avtSamplePointsSink()
{
    input = NULL;
}


// ****************************************************************************
//  Method: avtSamplePointsSink::SetTypedInput
//
//  Purpose:
//      Sets the input of the sink and performs some type checking.
//
//  Arguments:
//      in      The sample points as a data object.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Apr 4 14:50:16 PST 2002
//    Made CopyTo an inline template function.
//
// ****************************************************************************

void
avtSamplePointsSink::SetTypedInput(avtDataObject_p in)
{
    if (strcmp(in->GetType(), "avtSamplePoints") != 0)
    {
        //
        // Should create a new exception here, but I'm under time constraints.
        //
        debug1 << "Looking for avtSamplePoints, but found type \""
               << in->GetType() << "\"." << endl;
        EXCEPTION0(ImproperUseException);
    }

    CopyTo(input, in);
}


// ****************************************************************************
//  Method: avtSamplePointsSink::GetInput
//
//  Purpose:
//      Gets the input of the sink (properly typed as an avtDataObject).
//
//  Returns:    The input of the sink.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Apr 4 14:50:50 PST 2002
//    Made CopyTo an inline template function.
//
// ****************************************************************************

avtDataObject_p
avtSamplePointsSink::GetInput(void)
{
    avtDataObject_p rv;
    CopyTo(rv, input);
    return rv;
}


