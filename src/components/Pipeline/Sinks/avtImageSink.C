// ************************************************************************* //
//                               avtImageSink.C                              //
// ************************************************************************* //

#include <avtImageSink.h>

#include <DebugStream.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtImageSink constructor
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
// ****************************************************************************

avtImageSink::avtImageSink()
{
    input = NULL;
}


// ****************************************************************************
//  Method: avtImageSink destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtImageSink::~avtImageSink()
{
    ;
}


// ****************************************************************************
//  Method: avtImageSink::SetTypedInput
//
//  Purpose:
//      Sets the input of the sink and performs some type checking.
//
//  Arguments:
//      in      The image as a data object.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//
//    Brad Whitlock, Thu Apr 4 14:57:22 PST 2002
//    Changed CopyTo to an inline template function.
//
//    Hank Childs, Tue Sep 10 09:02:17 PDT 2002
//    Do not assume that an input is non-NULL.
//
// ****************************************************************************

void
avtImageSink::SetTypedInput(avtDataObject_p in)
{
    if (*in != NULL && strcmp(in->GetType(), "avtImage") != 0)
    {
        //
        // Should create a new exception here, but I'm under time constraints.
        //
        debug1 << "Looking for avtImage, but found type \""
               << in->GetType() << "\"." << endl;
        EXCEPTION0(ImproperUseException);
    }

    CopyTo(input, in);
}


// ****************************************************************************
//  Method: avtImageSink::GetInput
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
//    Brad Whitlock, Thu Apr 4 14:57:22 PST 2002
//    Changed CopyTo to an inline template function.
//
// ****************************************************************************

avtDataObject_p
avtImageSink::GetInput(void)
{
    avtDataObject_p rv;
    CopyTo(rv, input);
    return rv;
}


// ****************************************************************************
//  Method: avtImageSink::GetImageRep
//
//  Purpose:
//      Gets the image representation.
//
//  Returns:     The representation of an image.
//
//  Programmer:  Hank Childs
//  Creation:    June 20, 2001
//
// ****************************************************************************

avtImageRepresentation &
avtImageSink::GetImageRep(void)
{
    return input->GetImage();
}


