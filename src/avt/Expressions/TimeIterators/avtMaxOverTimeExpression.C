// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtMaxOverTimeExpression.C                        //
// ************************************************************************* //

#include <avtMaxOverTimeExpression.h>

#include <vtkDataArray.h>


// ****************************************************************************
//  Method: avtMaxOverTimeExpression constructor
//
//  Programmer: Hank Childs
//  Creation:   February 16, 2009
//
// ****************************************************************************

avtMaxOverTimeExpression::avtMaxOverTimeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMaxOverTimeExpression destructor
//
//  Programmer: Hank Childs
//  Creation:   February 16, 2009
//
// ****************************************************************************

avtMaxOverTimeExpression::~avtMaxOverTimeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMaxOverTimeExpression::ExecuteDataset
//
//  Purpose:
//      Finds the maximum over time.  This is the method that looks examines a
//      single VTK data set.
//
//  Programmer:   Hank Childs
//  Creation:     February 16, 2009
//
// ****************************************************************************

void
avtMaxOverTimeExpression::ExecuteDataset(std::vector<vtkDataArray *> &inVars,
                                         vtkDataArray *outVar, int ts)
{
    vtkDataArray *iv = inVars[0];
    int nvars = iv->GetNumberOfTuples();
    for (int i = 0 ; i < nvars ; i++)
    {
        if (ts == 0)
            outVar->SetTuple1(i, iv->GetTuple1(i));
        else
        {
            double v1 = outVar->GetTuple1(i);
            double v2 = iv->GetTuple1(i);
            if (v2 > v1)
                outVar->SetTuple1(i, v2);
        }
    }
}


