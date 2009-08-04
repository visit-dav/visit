/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
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
avtFunctionExpr::CreateVectorMatrixFilters(const string &functionName) const
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
