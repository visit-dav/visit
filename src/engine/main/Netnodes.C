#include <Netnodes.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

// ****************************************************************************
//  Method: NetnodeDB::GetOutput
//
//  Purpose:
//      Gets the output from the database.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
// ****************************************************************************

avtDataObject_p
NetnodeDB::GetOutput(void)
{
    if (*output == NULL)
    {
        output = db->GetOutput(var.c_str(), time);
    }

    return output;
}


// ****************************************************************************
//  Method: NetnodeDB::ReleaseData
//
//  Purpose:
//      Tells the database to release the data associated with this output.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
// ****************************************************************************

void
NetnodeDB::ReleaseData(void)
{
    GetOutput()->ReleaseData();
}


// ****************************************************************************
//  Method:  NetnodeFilter::GetOutput
//
//  Purpose:
//    Get the final output of a network.  This has the effect of walking up
//    the line, hooking everything up.
//
// ****************************************************************************
avtDataObject_p
NetnodeFilter::GetOutput(void)
{
    // This is mostly a hack.  Filters that have more than one "input"
    // actually ignore all inputs other than the first.
    // Ahern - Wed Jun 26 14:27:42 PDT 2002
#if 0
    // We don't yet support more than one input
    if (inputNodes.size() != 1)
    {
        debug1 << "NetnodeFilter::GetOutput: Unable to add function with more than one argument." << endl;
        EXCEPTION0(ImproperUseException);
    }
#endif

    filter->SetInput(inputNodes[0]->GetOutput());

    return filter->GetOutput();
}

// ****************************************************************************
//  Method:  NetnodeFilter::ReleaseData
//
//  Purpose:
//    Releases the data associated with the filter.
//
// ****************************************************************************
void
NetnodeFilter::ReleaseData(void)
{
    // Release the data for my filter (and its inputs).
    filter->ReleaseData();

    // Release the data for my inputs.
    for (int i = 0; i < inputNodes.size(); i++)
        inputNodes[i]->ReleaseData();
}
