// ************************************************************************* //
//                            avtNullDataSink.C                              //
// ************************************************************************* //

#include <avtNullDataSink.h>

#include <DebugStream.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtNullDataSink constructor
//
//  Programmer: Mark C. Miller
//  Creation:   January 7, 2003 
//
// ****************************************************************************

avtNullDataSink::avtNullDataSink()
{
    input = NULL;
}


// ****************************************************************************
//  Method: avtNullDataSink destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNullDataSink::~avtNullDataSink()
{
    ;
}


// ****************************************************************************
//  Method: avtNullDataSink::SetTypedInput
//
//  Purpose:
//      Sets the input of the sink and performs some type checking.
//
//  Arguments:
//      in      The null data as a data object.
//
//  Programmer: Mark C. Miller
//  Creation:   January 7, 2003 
//
// ****************************************************************************

void
avtNullDataSink::SetTypedInput(avtDataObject_p in)
{
    if (*in != NULL && 
        (strcmp(in->GetType(), "avtNullData") != 0) && 
        (strcmp(in->GetType(), AVT_NULL_IMAGE_MSG) != 0) && 
        (strcmp(in->GetType(), AVT_NULL_DATASET_MSG) != 0))
    {
        EXCEPTION0(ImproperUseException);
    }

    CopyTo(input, in);

}


// ****************************************************************************
//  Method: avtNullDataSink::GetInput
//
//  Purpose:
//      Gets the input of the sink (properly typed as an avtDataObject).
//
//  Returns:    The input of the sink.
//
//  Programmer: Mark C. Miller 
//  Creation:   January 7, 2003 
//
// ****************************************************************************

avtDataObject_p
avtNullDataSink::GetInput(void)
{
    avtDataObject_p rv;
    CopyTo(rv, input);
    return rv;
}

