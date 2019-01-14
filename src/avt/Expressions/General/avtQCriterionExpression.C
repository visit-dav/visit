/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

// ************************************************************************* //
//                     avtQCriterionExpression.C                          //
// ************************************************************************* //

#include <avtQCriterionExpression.h>
#include <avtVariableCache.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkDataSet.h>
#include <vtkInformation.h>
#include <vtkInformationDoubleVectorKey.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <StackTimer.h>

#ifdef _OPENMP
# include <omp.h>
#endif

// ****************************************************************************
//  Method: avtQCriterionExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Kevin Griffin
//  Creation:   Tue Jul 29 10:08:40 PDT 2014
//
// ****************************************************************************

avtQCriterionExpression::avtQCriterionExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtQCriterionExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Kevin Griffin
//  Creation:   Tue Jul 29 10:08:40 PDT 2014
//
// ****************************************************************************

avtQCriterionExpression::~avtQCriterionExpression()
{
    ;
}

// ****************************************************************************
//  Method: avtQCriterionExpression::QCriterion
//
//  Purpose:
//      Generates the Q-criterion value developed by Hunt et al. It is based on
//      the observation that, in regions where the Q-criterion is greater than
//      zero, rotation exceeds strain and, in conjuction with a pressure min,
//      indicates the presence of a vortex.
//
//  Arguments:
//      du      The gradient vector of X
//      dv      The gradient vector of Y
//      dw      The gradient vector of Z
//
//  Returns:      The Q-criterion value
//
//  Programmer:   Kevin Griffin
//  Creation:     Tue Jul 29 10:08:40 PDT 2014
//
//  Modifications:
//
// ****************************************************************************

#define COMPUTE_Q_CRIT(VARTYPE, VAR) \
{ \
    VARTYPE s1 = 0.5 * (du[1] + dv[0]); \
    VARTYPE s2 = 0.5 * (du[2] + dw[0]); \
    VARTYPE s3 = 0.5 * (dv[0] + du[1]); \
\
    VARTYPE s5 = 0.5 * (dv[2] + dw[1]); \
    VARTYPE s6 = 0.5 * (dw[0] + du[2]); \
    VARTYPE s7 = 0.5 * (dw[1] + dv[2]); \
\
    VARTYPE w1 = 0.5 * (du[1] - dv[0]); \
    VARTYPE w2 = 0.5 * (du[2] - dw[0]); \
    VARTYPE w3 = 0.5 * (dv[0] - du[1]); \
\
    VARTYPE w5 = 0.5 * (dv[2] - dw[1]); \
    VARTYPE w6 = 0.5 * (dw[0] - du[2]); \
    VARTYPE w7 = 0.5 * (dw[1] - dv[2]); \
\
    VARTYPE sNorm = du[0] * du[0] + s1 * s1 + s2 * s2 + s3 * s3 + dv[1] * dv[1] + s5 * s5 + s6 * s6 + s7 * s7 + dw[2] * dw[2]; \
    VARTYPE wNorm = w1 * w1 + w2 * w2 + w3 * w3 + w5 * w5 + w6 * w6 + w7 * w7; \
\
    VAR = 0.5 * (wNorm - sNorm); \
}

double
avtQCriterionExpression::QCriterion(const double * du, const double * dv, const double * dw)
{
    double qCrit;
    COMPUTE_Q_CRIT(double, qCrit);
    return qCrit;
}

// ****************************************************************************
//  Method: ompCalculateQCriterion
//
//  Purpose:
//
//  Arguments:
//      gradX   The gradient vector of X
//      gradY   The gradient vector of Y
//      gradZ   The gradient vector of Z
//      qCrit   The resulting q-criterion array
//
//  Returns:    None
//
//  Programmer:   Christopher P. Stone
//  Creation:     Sat Apr 01 13:00:00 EDT 2017
//
//  Modifications:
//
// ****************************************************************************
#ifdef _OPENMP
template <typename InputType>
static void
ompCalculateQCriterion(const InputType * gradX, const InputType * gradY, const InputType * gradZ,
    double *qCrit, const int numTuples)
{
    #pragma omp parallel for
    for (int i = 0; i < numTuples; ++i)
    {
        const int offset = 3*i;
        const InputType du[3] = { gradX[offset], gradX[offset+1], gradX[offset+2] };
        const InputType dv[3] = { gradY[offset], gradY[offset+1], gradY[offset+2] };
        const InputType dw[3] = { gradZ[offset], gradZ[offset+1], gradZ[offset+2] };

        COMPUTE_Q_CRIT(InputType, qCrit[i]);
    }
    
    return;
}
#endif

// ****************************************************************************
//  Method: avtQCriterionExpression::DeriveVariable
//
//  Purpose:
//      Creates scalar values from velocity gradients.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived q-criterion scalar values.  The calling class 
//                must free this memory.
//
//  Programmer:   Kevin Griffin
//  Creation:     Tue Jul 29 10:08:40 PDT 2014
//
//  Modifications:
//
// ****************************************************************************
vtkDataArray *
avtQCriterionExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    if(varnames.size() != 3)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Expecting 3 inputs (gradX, gradY, gradZ)");
    }

    debug1 << "Inside avtQCriterionExpression: varnames= " << varnames[0] << ", " << varnames[1] << ", " << varnames[2] << endl;

    vtkDataArray *var1 = NULL;  /* du */
    vtkDataArray *var2 = NULL;  /* dv */
    vtkDataArray *var3 = NULL;  /* dw */
    
    var1 = in_ds->GetPointData()->GetArray(varnames[0]);
    
    if(var1 != NULL)
    {
        var2 = in_ds->GetPointData()->GetArray(varnames[1]);
        var3 = in_ds->GetPointData()->GetArray(varnames[2]);
    }
    else
    {
        var1 = in_ds->GetCellData()->GetArray(varnames[0]);
        var2 = in_ds->GetCellData()->GetArray(varnames[1]);
        var3 = in_ds->GetCellData()->GetArray(varnames[2]);
    }
    
    // Check that the variable(s) exist and that they have the same centering
    if(var1 == NULL || var2 == NULL || var3 == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Variable for q_criterion must exist and have the same centering");
    }
    
    // Check that the input variables are vectors
    if(var1->GetNumberOfComponents() != 3)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "q_criterion takes only vectors as input parameters");
    }
    
    int numTuples = var1->GetNumberOfTuples();
    
    // Q-Criterion Results
    vtkDataArray *results = vtkDoubleArray::New();
    results->SetNumberOfComponents(1);
    results->SetNumberOfTuples(numTuples);

#ifdef _OPENMP
#pragma message("Compiling for OpenMP.")
    bool hasValidData = (var1->GetDataType() == var2->GetDataType()) &&
                        (var2->GetDataType() == var3->GetDataType()) &&
                       ((var1->GetDataType() == VTK_DOUBLE) || (var1->GetDataType() == VTK_FLOAT)) &&
                        (var1->HasStandardMemoryLayout() && var2->HasStandardMemoryLayout() && var3->HasStandardMemoryLayout());

    if (hasValidData)
    {
        StackTimer t0("avtQCriterionExpression OpenMP");
        if (var1->GetDataType() == VTK_DOUBLE)
        {
            ompCalculateQCriterion( (const double *) var1->GetVoidPointer(0),
                                    (const double *) var2->GetVoidPointer(0),
                                    (const double *) var3->GetVoidPointer(0),
                                    (double *) results->GetVoidPointer(0),
                                    numTuples);
        }
        else// if (var1->GetDataType() == VTK_FLOAT)
        {
            ompCalculateQCriterion( (const float *) var1->GetVoidPointer(0),
                                    (const float *) var2->GetVoidPointer(0),
                                    (const float *) var3->GetVoidPointer(0),
                                    (double *) results->GetVoidPointer(0),
                                     numTuples);
        }
    }
    else
#endif
    {
        StackTimer t1("avtQCriterionExpression");
        for(int i=0; i<numTuples; i++)
        {
            results->SetTuple1(i, avtQCriterionExpression::QCriterion(var1->GetTuple(i), var2->GetTuple(i), var3->GetTuple(i)));
        }
    }

    return results;
}
