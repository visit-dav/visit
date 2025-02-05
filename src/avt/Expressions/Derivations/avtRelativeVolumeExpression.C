// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtRelativeVolumeExpression.C                       //
// ************************************************************************* //

#include <avtRelativeVolumeExpression.h>
#include <math.h>

#include <vtkCellType.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>

#include <ExpressionException.h>

#include <vector>

// ****************************************************************************
//  Method: avtRelativeVolumeExpression constructor
//
//  Programmer: Justin Privitera
//  Creation:   Fri Sep  9 10:37:12 PDT 2022
//
// ****************************************************************************

avtRelativeVolumeExpression::avtRelativeVolumeExpression()
{
    ;
}

// ****************************************************************************
//  Method: avtRelativeVolumeExpression destructor
//
//  Programmer: Justin Privitera
//  Creation:   Fri Sep  9 10:37:12 PDT 2022
//
// ****************************************************************************

avtRelativeVolumeExpression::~avtRelativeVolumeExpression()
{
    ;
}

// ****************************************************************************
//  Method: avtRelativeVolumeExpression::DeriveVariable
//
//  Purpose:
//      Calls the base class relative volume calculation.
//
//  Programmer: Justin Privitera
//  Creation:   Fri Sep  9 10:37:12 PDT 2022
//
//  Modifications:
//     Justin Privitera, Thu Sep 29 15:22:38 PDT 2022
//     Replaced auto with bool.
//
// ****************************************************************************

vtkDataArray *
avtRelativeVolumeExpression::DeriveVariable (vtkDataSet *in_ds, 
                                             int currentDomainsIndex)
{
    bool do_vol_strain{false};
    return avtStrainExpression::CalculateEvolOrRelvol(in_ds, do_vol_strain);
}
