// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtPolarCoordinatesExpression.C                       //
// ************************************************************************* //

#include <avtPolarCoordinatesExpression.h>

#include <math.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtPolarCoordinatesExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//
//    Alister Maguire, Fri Oct  9 11:46:22 PDT 2020
//    Set canApplyToDirectDatabaseQOT to false.
//
// ****************************************************************************

avtPolarCoordinatesExpression::avtPolarCoordinatesExpression()
{
    canApplyToDirectDatabaseQOT = false;
}


// ****************************************************************************
//  Method: avtPolarCoordinatesExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtPolarCoordinatesExpression::~avtPolarCoordinatesExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtPolarCoordinatesExpression::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.  The variable is the
//      polar coordinates of the point list.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     November 18, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Nov 15 15:49:07 PST 2005
//    Make phi be 0 for 2D plots.
//
// ****************************************************************************

vtkDataArray *
avtPolarCoordinatesExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkIdType npts = in_ds->GetNumberOfPoints();
    vtkDataArray *rv = CreateArrayFromMesh(in_ds);
    rv->SetNumberOfComponents(3);
    rv->SetNumberOfTuples(npts);
    bool in3D = 
            (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 3);
    for (vtkIdType i = 0 ; i < npts ; i++)
    {
        double pt[3];
        in_ds->GetPoint(i, pt);
        
        double r = sqrt(pt[0]*pt[0] + pt[1]*pt[1] + pt[2]*pt[2]);
        rv->SetComponent(i, 0, r);

        double theta = atan2(pt[1], pt[0]);
        rv->SetComponent(i, 1, theta);

        double phi = 0.;
        if (in3D && r != 0)
            phi = acos(pt[2] / r);
        else
            phi = 0;
        rv->SetComponent(i, 2, phi);
    }
    
    return rv;
}


