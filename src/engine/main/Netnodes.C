#include <Netnodes.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

// ****************************************************************************
//  Method: NetnodeDB constructor
//
//  Arguments:
//      db_in   A pointer to a database.
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2003
//
// ****************************************************************************

NetnodeDB::NetnodeDB(avtDatabase *db_in)
{
    db = db_in;
    filename = "<unknown>";
    var = "<unknown>";
    time = -1;
    current_var = "<unknown>";
    current_time = -1;
}


// ****************************************************************************
//  Method: NetnodeDB constructor
//
//  Arguments:
//      db_in   A pointer to a database.
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2003
//
// ****************************************************************************

NetnodeDB::NetnodeDB(ref_ptr<avtDatabase> db_in)
{
    db = db_in;
    filename = "<unknown>";
    var = "<unknown>";
    time = -1;
    current_var = "<unknown>";
    current_time = -1;
}


// ****************************************************************************
//  Method: NetnodeDB::SetDBInfo
//
//  Purpose:
//      Sets the information about the database.
//
//  Notes:      This was previously an inlined function.  Creation date is
//              for when the function was defined as a non-inlined function.
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2003
//
// ****************************************************************************

void
NetnodeDB::SetDBInfo(std::string _filename, std::string _var, int _time)
{
    if ((time != _time) || (filename != _filename) || (var != _var))
    {
        if (*output != NULL)
        {
            output->ReleaseData();
        }
    }

    time = _time;
    filename = _filename;
    var = _var;
}


// ****************************************************************************
//  Method: NetnodeDB::GetOutput
//
//  Purpose:
//      Gets the output from the database.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
//  Modificiations:
//
//    Hank Childs, Mon Jul 28 13:49:58 PDT 2003
//    Returned the cached output if possible.
//
// ****************************************************************************
avtDataObject_p
NetnodeDB::GetOutput(void)
{
    if ((*output != NULL) &&
        (var == current_var) &&
        (time == current_time))
    {
        return output;
    }

    output = db->GetOutput(var.c_str(), time);
    current_var = var;
    current_time = time;

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
//  Modifications:
//
//    Hank Childs, Thu Apr  1 07:33:42 PST 2004
//    Don't try to release the data if we don't have a variable -- this can
//    happen if we never used the database, or if we are doing a re-open.
//
// ****************************************************************************

void
NetnodeDB::ReleaseData(void)
{
    if (var != "" && var != "<unknown>")
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

// ****************************************************************************
//  Method:  NetnodeTransition::Constructor
//
//  Purpose:
//    Acts as a transition node between one network and its clone.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    March 18, 2004
//
// ****************************************************************************

NetnodeTransition::NetnodeTransition(avtDataObject_p input) :
    NetnodeFilter(NULL, "")
{
    output = input;
}
