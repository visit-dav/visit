// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtNormalizeExpression.C                           //
// ************************************************************************* //

#include <avtNormalizeExpression.h>

#include <math.h>
#include <vector>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtNormalizeExpression constructor
//
//  Programmer: Sean Ahern
//  Creation:   Wed Feb  7 19:03:09 EST 2007
//
//  Modifications:
//
// ****************************************************************************

avtNormalizeExpression::avtNormalizeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtNormalizeExpression destructor
//
//  Programmer: Sean Ahern
//  Creation:   Wed Feb  7 19:03:22 EST 2007
//
//  Modifications:
//
// ****************************************************************************

avtNormalizeExpression::~avtNormalizeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtNormalizeExpression::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Sean Ahern (mostly copied from avtMagnitudeExpression)
//  Creation:     Wed Feb  7 19:14:48 EST 2007
//
//  Modifications:
//
//    Hank Childs, Mon Jan 14 18:23:40 PST 2008
//    Clean up wrapped lines.
//
// ****************************************************************************

vtkDataArray *
avtNormalizeExpression::DeriveVariable(vtkDataSet *in, int currentDomainsIndex)
{
    // The base class will set the variable of interest to be the
    // 'activeVariable'.  This is a by-product of how the base class sets its
    // input.  If that method should change (SetActiveVariable), this
    // technique for inferring the variable name may stop working.

    const char *varname = activeVariable;

    vtkDataArray *vectorValues = in->GetPointData()->GetArray(varname);

    if (vectorValues == NULL)
    {
        vectorValues = in->GetCellData()->GetArray(varname);
    }
    if (vectorValues == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Unable to locate variable for normalize expression");
    }

    if (vectorValues->GetNumberOfComponents() != 3)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Can only normalize vectors.");
    }

    int ntuples = vectorValues->GetNumberOfTuples();

    vtkDataArray *results = vectorValues->NewInstance();
    results->SetNumberOfComponents(3);
    results->SetNumberOfTuples(ntuples);

    for (int i=0; i<ntuples; i++)
    {
        double xin = vectorValues->GetComponent(i, 0);
        double yin = vectorValues->GetComponent(i, 1);
        double zin = vectorValues->GetComponent(i, 2);

        double mag = sqrt(xin*xin + yin*yin + zin*zin);

        double xout, yout, zout;
        if (mag == 0.)
            xout = yout = zout = 0.;
        else
        {
            xout = xin / mag;
            yout = yin / mag;
            zout = zin / mag;
        }

        results->SetComponent(i, 0, xout);
        results->SetComponent(i, 1, yout);
        results->SetComponent(i, 2, zout);
    }

    return results;
}
