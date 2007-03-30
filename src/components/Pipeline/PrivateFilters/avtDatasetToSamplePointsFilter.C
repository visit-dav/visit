// ************************************************************************* //
//                      avtDatasetToSamplePointsFilter.C                     //
// ************************************************************************* //

#include <avtDatasetToSamplePointsFilter.h>

#include <avtDatasetExaminer.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtDatasetToSamplePointsFilter::PreExecute
//
//  Purpose:
//      Executes before the avtFilter calls Execute.  The sample points need
//      to know how many variables they will be sampling over before they get
//      going.  This is a chance to set that up.
//
//  Programmer: Hank Childs
//  Creation:   November 15, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Feb  5 09:17:12 PST 2002
//    Add better support for datasets with no variables.
//
//    Hank Childs, Fri Mar 15 17:40:02 PST 2002
//    Account for new avtDatasetExaminer.
//
// ****************************************************************************

void
avtDatasetToSamplePointsFilter::PreExecute(void)
{
    avtDataset_p ds = GetTypedInput();
    VarList vl;
    vl.nvars = -1;
    avtDatasetExaminer::GetVariableList(ds, vl);

    avtSamplePoints_p sp = GetTypedOutput();
    if (vl.nvars <= 0)
    {
        debug1 << "!!! Converting a dataset that has no variables to sample "
               << "points" << endl;
        vl.nvars = 0;
    }

    bool leaveAsIs = false;
    if (vl.nvars == 0 && sp->GetNumberOfVariables() > 0)
    {
        //
        // Someone came in and set the output so that it had more variables --
        // this is common practice if we are executing in parallel and we
        // have more processors than domains.
        //
        debug1 << "!!! The sample points already believed that it had "
               << "variables -- leaving as is." << endl;
        leaveAsIs = true;
    }

    if (!leaveAsIs)
    {
        sp->SetNumberOfVariables(vl.nvars);
    }
}


