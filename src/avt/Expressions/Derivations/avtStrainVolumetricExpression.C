// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtStrainVolumetricExpression.C                     //
// ************************************************************************* //

#include <avtStrainVolumetricExpression.h>
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
//  Method: avtStrainVolumetricExpression constructor
//
//  Programmer: Justin Privitera
//  Creation:   Fri Sep  9 10:37:12 PDT 2022
//
// ****************************************************************************

avtStrainVolumetricExpression::avtStrainVolumetricExpression()
{
    ;
}

// ****************************************************************************
//  Method: avtStrainVolumetricExpression destructor
//
//  Programmer: Justin Privitera
//  Creation:   Fri Sep  9 10:37:12 PDT 2022
//
// ****************************************************************************

avtStrainVolumetricExpression::~avtStrainVolumetricExpression()
{
    ;
}

// ****************************************************************************
//  Method: avtStrainVolumetricExpression::DeriveVariable
//
//  Purpose:
//      Calls base class volumetric strain calculation.
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
avtStrainVolumetricExpression::DeriveVariable (vtkDataSet *in_ds, 
                                             int currentDomainsIndex)
{
    bool do_vol_strain{true};
    return avtStrainExpression::CalculateEvolOrRelvol(in_ds, do_vol_strain);
}
