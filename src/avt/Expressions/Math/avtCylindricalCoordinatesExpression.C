// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                  avtCylindricalCoordinatesExpression.C                    //
// ************************************************************************* //

#include <avtCylindricalCoordinatesExpression.h>

#include <math.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtCylindricalCoordinatesExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
//  Modifications:
//
//    Alister Maguire, Fri Oct  9 11:46:22 PDT 2020
//    Set canApplyToDirectDatabaseQOT to false.
//
// ****************************************************************************

avtCylindricalCoordinatesExpression::avtCylindricalCoordinatesExpression()
{
    canApplyToDirectDatabaseQOT = false;
}


// ****************************************************************************
//  Method: avtCylindricalCoordinatesExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

avtCylindricalCoordinatesExpression::~avtCylindricalCoordinatesExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCylindricalCoordinatesExpression::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.  The variable is the
//      cylindrical coordinates of the point list.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     June 30, 2005
//
// ****************************************************************************

vtkDataArray *
avtCylindricalCoordinatesExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkIdType npts = in_ds->GetNumberOfPoints();
    vtkDataArray *rv = CreateArrayFromMesh(in_ds);
    rv->SetNumberOfComponents(3);
    rv->SetNumberOfTuples(npts);
    for (vtkIdType i = 0 ; i < npts ; i++)
    {
        double pt[3];
        in_ds->GetPoint(i, pt);
        
        double r = sqrt(pt[0]*pt[0] + pt[1]*pt[1]);
        rv->SetComponent(i, 0, r);

        double theta = atan2(pt[1], pt[0]);
        rv->SetComponent(i, 1, theta);

        rv->SetComponent(i, 2, pt[2]);
    }
    
    return rv;
}


