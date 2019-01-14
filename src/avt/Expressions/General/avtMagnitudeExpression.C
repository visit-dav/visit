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
//                         avtMagnitudeExpression.C                          //
// ************************************************************************* //

#include <avtMagnitudeExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <StackTimer.h>

#ifdef _OPENMP
# include <omp.h>
#endif

// ****************************************************************************
//  Method: avtMagnitudeExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtMagnitudeExpression::avtMagnitudeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMagnitudeExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtMagnitudeExpression::~avtMagnitudeExpression()
{
    ;
}

// ****************************************************************************
//  Method: avtMagnitudeExpression::CalculateMagnitude
//
//  Purpose:
//      Do the operation over all cells/nodes quickly.
//
//  Arguments:
//      vectorIn      The input vector array.
//      scalarOut     The output scalar array.
//      numTuples     The input number of cells/nodes.
//
//  Returns:      None.
//
//  Programmer:   Christopher Stone
//  Creation:     April 01, 2017
//
//  Modifications:
//
// ****************************************************************************
#ifdef _OPENMP
template <typename ArrayType>
static void
ompCalculateMagnitude(const ArrayType *vectorIn, ArrayType *scalarOut, const int numTuples)
{
    #pragma omp parallel for
    for (vtkIdType i = 0; i < numTuples ; ++i)
    {
        const vtkIdType idx = 3*i;
        scalarOut[i] = sqrt((double)vectorIn[idx+0]*(double)vectorIn[idx+0]+
                            (double)vectorIn[idx+1]*(double)vectorIn[idx+1]+
                            (double)vectorIn[idx+2]*(double)vectorIn[idx+2]);
    }
}
#endif

// ****************************************************************************
//  Method: avtMagnitudeExpression::DeriveVariable
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
//  Programmer:   Matthew Haddox
//  Creation:     July 30, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Nov 18 15:11:11 PST 2002
//    Beefed up error code a little.
//
//    Kathleen Bonnell, Thu Jan  2 15:16:50 PST 2003 
//    Replace MakeObject() with NewInstance() to match new vtk api. 
//
//    Kathleen Bonnell, Wed Aug 18 16:55:49 PDT 2004 
//    Retrieve vectorValues array based on activeVariableName. (There may be
//    a Vectors array in both the CellData and the PointData, and the old way
//    would always retrieve the PointData vectors in this instance).  We are
//    assuming that the PoinData Vectors and CellData Vectors will have 
//    different names.
//
//    Hank Childs, Mon Jan 14 17:58:58 PST 2008
//    Clean up some wrapped lines.
//
//    Brad Whitlock, Tue Jul 21 13:50:55 PDT 2015
//    Support non-standard memory layout.
//
// ****************************************************************************

vtkDataArray *
avtMagnitudeExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    //
    // The base class will set the variable of interest to be the 
    // 'activeVariable'.  This is a by-product of how the base class sets its
    // input.  If that method should change (SetActiveVariable), this
    // technique for inferring the variable name may stop working.
    //
    const char *varname = activeVariable;

    vtkDataArray *vectorValues = in_ds->GetPointData()->GetArray(varname);
    
    if (vectorValues == NULL)
    {
        vectorValues = in_ds->GetCellData()->GetArray(varname);
    }
    if (vectorValues == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                  "Unable to locate variable for magnitude expression");
    }

    if (vectorValues->GetNumberOfComponents() != 3)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Can only take magnitude of vectors.");
    }
    vtkIdType ntuples = vectorValues->GetNumberOfTuples();

    vtkDataArray *results = vectorValues->NewInstance();
    results->SetNumberOfComponents(1);
    results->SetNumberOfTuples(ntuples);

#define COMPUTE_MAG(dtype) \
{ \
    dtype *vec = (dtype*)vectorValues->GetVoidPointer(0); \
    dtype *r   = (dtype*)results->GetVoidPointer(0); \
    for (vtkIdType i = 0; i < ntuples ; ++i) \
    { \
        const vtkIdType idx = 3*i; \
        r[i] = sqrt((double)vec[idx+0]*(double)vec[idx+0]+\
                    (double)vec[idx+1]*(double)vec[idx+1]+\
                    (double)vec[idx+2]*(double)vec[idx+2]); \
    } \
}

    if(vectorValues->HasStandardMemoryLayout())
    {
#ifdef _OPENMP
#pragma message("Compiling for OpenMP")
        if (vectorValues->GetDataType() == VTK_DOUBLE || vectorValues->GetDataType() == VTK_FLOAT)
        {
            StackTimer t1("avtMagnitudeExpression OpenMP");
            if (vectorValues->GetDataType() == VTK_DOUBLE)
                ompCalculateMagnitude( (double *)vectorValues->GetVoidPointer(0), (double *)results->GetVoidPointer(0), ntuples );
            else
                ompCalculateMagnitude( (float *)vectorValues->GetVoidPointer(0), (float *)results->GetVoidPointer(0), ntuples );
        }
        else
#endif
        {
            StackTimer t1("avtMagnitudeExpression vtkTemplateMacro");
            switch(vectorValues->GetDataType())
            {
                vtkTemplateMacro(COMPUTE_MAG(VTK_TT));
            }
        }
    }
    else
    {
        StackTimer t1("avtMagnitudeExpression");
        for(vtkIdType i = 0; i < ntuples ; i++)
        {
            const double *x = vectorValues->GetTuple(i);
            results->SetTuple1(i, sqrt(x[0]*x[0]+ x[1]*x[1]+ x[2]*x[2]));
        }
    }

    return results;
}

// ****************************************************************************
// Method: avtMagnitudeExpression::DeriveVariableVTKm
//
// Purpose:
//   Use VTKm to compute the vector magnitude.
//
// Arguments:
//   ds                  : The VTKm dataset.
//   currentDomainsIndex : Index of this domain.
//   activeVar           : The active input variable.
//   outputVar           : The name of the variable to create and add to ds.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar  9 16:52:59 PST 2017
//
// Modifications:
//
// ****************************************************************************

#ifdef HAVE_LIBVTKM
#include <vtkmExpressions.h>
#endif

void
avtMagnitudeExpression::DeriveVariableVTKm(vtkmDataSet *ds, int currentDomainsIndex,
    const std::string &activeVar, const std::string &outputVar)
{
#ifdef HAVE_LIBVTKM
    StackTimer t0("avtMagnitudeExpression::DeriveVariableVTKm");
    vtkmMagnitudeExpression(ds, currentDomainsIndex, activeVar, outputVar);
#endif
}

