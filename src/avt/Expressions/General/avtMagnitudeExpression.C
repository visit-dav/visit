// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Modifications:
//
//    Alister Maguire, Mon Feb 24 14:25:20 MST 2020
//    Removed canApplyToDirectDatabaseQOT. It now defaults to true.
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

