// ************************************************************************* //
//                             avtDatasetSink.C                              //
// ************************************************************************* //

#include <avtDatasetSink.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <NoInputException.h>


// ****************************************************************************
//  Method: avtDatasetSink constructor
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
// ****************************************************************************

avtDatasetSink::avtDatasetSink()
{
    input = NULL;
}


// ****************************************************************************
//  Method: avtDatasetSink::SetTypedInput
//
//  Purpose:
//      Sets the input of the sink and performs some type checking.
//
//  Arguments:
//      in      The data set as a data object.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//
//    Brad Whitlock, Thu Apr 4 14:53:33 PST 2002
//    Changed CopyTo to an inline template function.
//
//    Hank Childs, Tue Sep 10 09:02:17 PDT 2002
//    Do not assume that an input is non-NULL.
//
// ****************************************************************************

void
avtDatasetSink::SetTypedInput(avtDataObject_p in)
{
    if (*in != NULL && strcmp(in->GetType(), "avtDataset") != 0)
    {
        //
        // Should create a new exception here, but I'm under time constraints.
        //
        debug1 << "Looking for avtDataset, but found type \"" << in->GetType()
               << "\"." << endl;
        EXCEPTION0(ImproperUseException);
    }

    CopyTo(input, in);
}


// ****************************************************************************
//  Method: avtDatasetSink::GetInput
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
//    Brad Whitlock, Thu Apr 4 14:53:33 PST 2002
//    Changed CopyTo to an inline template function.
//
// ****************************************************************************

avtDataObject_p
avtDatasetSink::GetInput(void)
{
    avtDataObject_p rv;
    CopyTo(rv, input);
    return rv;
}


// ****************************************************************************
//  Method: avtDatasetSink::GetInputDataTree
//
//  Purpose:
//      Gets the data tree of the input.
//
//  Returns:    The avtDataTree corresponding to the input.
//
//  Programmer: Hank Childs
//  Creation:   September 19, 2000
//
// ****************************************************************************

avtDataTree_p
avtDatasetSink::GetInputDataTree()
{
    if (*input == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    return input->GetDataTree();
}



