// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtSamplePointsToSamplePointsFilter.C                  //
// ************************************************************************* //

#include <avtSamplePointsToSamplePointsFilter.h>

avtSamplePointsToSamplePointsFilter::avtSamplePointsToSamplePointsFilter()
{
}

avtSamplePointsToSamplePointsFilter::~avtSamplePointsToSamplePointsFilter()
{
}

// ****************************************************************************
//  Method: avtSamplePointsToSamplePointsFilter::PreExecute
//
//  Purpose:
//      Executes before the avtFilter calls Execute.  The sample points need
//      to know how many variables they will be sampling over before they get
//      going.  This is a chance to set that up.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
//    Hank Childs, Thu May 31 13:54:52 PDT 2007
//    Added support for non-scalar variables.
//
//    Hank Childs, Thu Jun 21 11:40:04 PDT 2007
//    Fix stupid bug where variables weren't being set up correctly.
//
// ****************************************************************************

void
avtSamplePointsToSamplePointsFilter::PreExecute(void)
{
    avtSamplePointsToDataObjectFilter::PreExecute();

    avtSamplePoints_p in  = GetTypedInput();
    avtSamplePoints_p out = GetTypedOutput();
    int nvar = in->GetNumberOfRealVariables();
    std::vector<std::string> varnames;
    std::vector<int>         varsizes;
    for (int i = 0 ; i < nvar ; i++)
    {
         varnames.push_back(in->GetVariableName(i));
         varsizes.push_back(in->GetVariableSize(i));
    }

    out->SetNumberOfVariables(varsizes, varnames);
}


