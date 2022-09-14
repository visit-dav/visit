// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtExprNode.h>

#include <avtBinaryMultiplyExpression.h>
#include <avtCurlExpression.h>
#include <avtDeterminantExpression.h>
#include <avtDivergenceExpression.h>
#include <avtEffectiveTensorExpression.h>
#include <avtEigenvalueExpression.h>
#include <avtEigenvectorExpression.h>
#include <avtGradientExpression.h>
#include <avtInverseExpression.h>
#include <avtMagnitudeExpression.h>
#include <avtMeshCoordinateExpression.h>
#include <avtNormalizeExpression.h>
#include <avtPolarCoordinatesExpression.h>
#include <avtPrincipalDeviatoricTensorExpression.h>
#include <avtPrincipalTensorExpression.h>
#include <avtRelativeVolumeExpression.h>
#include <avtStrainVolumetricExpression.h>
#include <avtStrainAlmansiExpression.h>
#include <avtStrainGreenLagrangeExpression.h>
#include <avtStrainInfinitesimalExpression.h>
#include <avtStrainRateExpression.h>
#include <avtTensorContractionExpression.h>
#include <avtTensorMaximumShearExpression.h>
#include <avtTraceExpression.h>
#include <avtTransposeExpression.h>
#include <avtVectorCrossProductExpression.h>
#include <avtViscousStressExpression.h>

#include <string>

// ****************************************************************************
// Method: avtFunctionExpr::CreateVectorMatrixFilters
//
// Purpose: 
//   Creates vector and matrix filters.
//
// Arguments:
//   functionName : The name of the expression filter to create.
//
// Returns:    An expression filter or 0 if one could not be created.
//
// Note:       
//
// Programmer: 
// Creation:   Thu May 21 08:55:58 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

avtExpressionFilter *
avtFunctionExpr::CreateVectorMatrixFilters(const std::string &functionName) const
{
    avtExpressionFilter *f = 0;

    if (functionName == "cross")
        f = new avtVectorCrossProductExpression();
    else if (functionName == "dot")
        f = new avtBinaryMultiplyExpression();
    else if (functionName == "det" || functionName == "determinant")
        f = new avtDeterminantExpression();
    else if (functionName == "eigenvalue")
        f = new avtEigenvalueExpression();
    else if (functionName == "eigenvector")
        f = new avtEigenvectorExpression();
    else if (functionName == "inverse")
        f = new avtInverseExpression();
    else if (functionName == "trace")
        f = new avtTraceExpression();
    else if (functionName == "effective_tensor")
        f = new avtEffectiveTensorExpression();
    else if (functionName == "tensor_maximum_shear")
        f = new avtTensorMaximumShearExpression();
    else if (functionName == "principal_tensor")
        f = new avtPrincipalTensorExpression();
    else if (functionName == "principal_deviatoric_tensor")
        f = new avtPrincipalDeviatoricTensorExpression();
    else if (functionName == "relative_volume")
        f = new avtRelativeVolumeExpression();
    else if (functionName == "strain_volumetric")
        f = new avtStrainVolumetricExpression();
    else if (functionName == "strain_almansi")
        f = new avtStrainAlmansiExpression();
    else if (functionName == "strain_green_lagrange")
        f = new avtStrainGreenLagrangeExpression();
    else if (functionName == "strain_infinitesimal")
        f = new avtStrainInfinitesimalExpression();
    else if (functionName == "strain_rate")
        f = new avtStrainRateExpression();
    else if (functionName == "contraction")
        f = new avtTensorContractionExpression();
    else if (functionName == "transpose")
        f = new avtTransposeExpression();
    else if (functionName == "viscous_stress")
        f = new avtViscousStressExpression();
    else if (functionName == "polar")
        f = new avtPolarCoordinatesExpression();
    else if (functionName == "coord" || functionName == "coords")
        f = new avtMeshCoordinateExpression();
    else if (functionName == "gradient")
        f = new avtGradientExpression();
    else if (functionName == "curl")
        f = new avtCurlExpression();
    else if (functionName == "divergence")
        f = new avtDivergenceExpression();
    else if (functionName == "magnitude")
        f = new avtMagnitudeExpression();
    else if (functionName == "normalize")
        f = new avtNormalizeExpression();

    return f;
}
