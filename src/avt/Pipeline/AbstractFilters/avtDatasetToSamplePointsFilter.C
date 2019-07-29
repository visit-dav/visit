// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtDatasetToSamplePointsFilter.C                     //
// ************************************************************************* //

#include <avtDatasetToSamplePointsFilter.h>

#include <avtDatasetExaminer.h>
#include <avtParallel.h>

#include <DebugStream.h>

#include <cstring>

#include <string>
#include <vector>

// ****************************************************************************
//  Method: avtDatasetToSamplePointsFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDatasetToSamplePointsFilter::avtDatasetToSamplePointsFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDatasetToSamplePointsFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDatasetToSamplePointsFilter::~avtDatasetToSamplePointsFilter()
{
    ;
}


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
//    Hank Childs, Wed Jul 23 14:59:55 PDT 2003
//    Account for not sampling VTK and AVT variables.
//
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
//    Hank Childs, Thu May 31 13:47:16 PDT 2007
//    Add support for non-scalar variables.
//
// ****************************************************************************

void
avtDatasetToSamplePointsFilter::PreExecute(void)
{
    avtDatasetToDataObjectFilter::PreExecute();

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
        std::vector<std::string> varnames;
        std::vector<int>    varsize;
        int realNVars = 0;
        for (int i = 0 ; i < vl.nvars ; i++)
        {
            const char *vname = vl.varnames[i].c_str();
            if (strstr(vname, "vtk") != NULL)
                continue;
            if (strstr(vname, "avt") != NULL)
                continue;
            varnames.push_back(vl.varnames[i]);
            varsize.push_back(vl.varsizes[i]);
            realNVars++;
        }

        // Some contortions here to use existing calls in avtParallel.
        int nvars = UnifyMaximumValue((int)varnames.size());
        GetListToRootProc(varnames, nvars);
        BroadcastStringVector(varnames, PAR_Rank());

        while (varsize.size() < (size_t)nvars)
            varsize.push_back(0);
        std::vector<int> varsize2(nvars);
        UnifyMaximumValue(varsize, varsize2);

        sp->SetNumberOfVariables(varsize2, varnames);
    }
}


