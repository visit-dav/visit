// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtSumOverTimeExpression.C                        //
// ************************************************************************* //

#include <avtSumOverTimeExpression.h>

#include <vtkDataArray.h>


// ****************************************************************************
//  Method: avtSumOverTimeExpression constructor
//
//  Programmer: Hank Childs
//  Creation:   February 16, 2009
//
// ****************************************************************************

avtSumOverTimeExpression::avtSumOverTimeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSumOverTimeExpression destructor
//
//  Programmer: Hank Childs
//  Creation:   February 16, 2009
//
// ****************************************************************************

avtSumOverTimeExpression::~avtSumOverTimeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSumOverTimeExpression::ExecuteDataset
//
//  Purpose:
//      Finds the summation over time.  This is the method that looks examines
//      a single VTK data set.
//
//  Programmer:   Hank Childs
//  Creation:     February 16, 2009
//
//  Modifications:
//    Jeremy Meredith, Wed Mar 18 14:03:33 EDT 2009
//    Allowed it to sum multiple-component arrays over time.
//
// ****************************************************************************

void
avtSumOverTimeExpression::ExecuteDataset(std::vector<vtkDataArray *> &inVars,
                                         vtkDataArray *outVar, int ts)
{
    vtkDataArray *iv = inVars[0];
    int nvars = iv->GetNumberOfTuples();
    int ncomps = iv->GetNumberOfComponents();
    if (outVar->GetNumberOfComponents() < ncomps)
        ncomps = outVar->GetNumberOfComponents();
    for (int i = 0 ; i < nvars ; i++)
    {
        for (int c = 0 ; c < ncomps ; c++)
        {
            if (ts == 0)
                outVar->SetComponent(i, c, iv->GetComponent(i, c));
            else
            {
                double v1 = outVar->GetComponent(i, c);
                double v2 = iv->GetComponent(i, c);
                outVar->SetComponent(i, c, v1+v2);
            }
        }
    }
}


