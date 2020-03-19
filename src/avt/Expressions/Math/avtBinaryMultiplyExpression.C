// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtBinaryMultiplyExpression.C                        //
// ************************************************************************* //

#include <avtBinaryMultiplyExpression.h>

#include <vtkDataArray.h>
#include <vtkDataArray.h>

#include <ExpressionException.h>
#ifdef _OPENMP
#include <StackTimer.h>
#include <omp.h>
#endif

// ****************************************************************************
//  Method: avtBinaryMultiplyExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBinaryMultiplyExpression::avtBinaryMultiplyExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryMultiplyExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBinaryMultiplyExpression::~avtBinaryMultiplyExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryMultiplyExpression::DoOperation
//
//  Purpose:
//      Multiplies two arrays into a third array.
//
//  Arguments:
//      in1           The first input data array.
//      in2           The second input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Sean Ahern          <Header added by Hank Childs>
//  Creation:   November 18, 2002   <Header creation date>
//
//  Modifications:
//
//    Hank Childs, Mon Nov 18 07:35:07 PST 2002
//    Added support for vectors and arbitrary data types.
//
//    Hank Childs, Thu Aug 14 13:37:27 PDT 2003
//    Added support for mixing scalars and vectors.
//
//    Hank Childs, Mon Sep 22 17:06:01 PDT 2003
//    Added support for tensors.
//
//    Hank Childs, Mon Jan 14 17:58:58 PST 2008
//    Add support for singleton constants.
//
//    Brad Whitlock, Fri Jan 12 13:07:04 PST 2018
//    OpenMP.
//
// ****************************************************************************
 
void
avtBinaryMultiplyExpression::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                     vtkDataArray *out, int ncomponents,
                                     int ntuples)
{
    bool var1IsSingleton = (in1->GetNumberOfTuples() == 1);
    bool var2IsSingleton = (in2->GetNumberOfTuples() == 1);
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();

    if ((in1ncomps == 9) && (in2ncomps == 9))
    {
        double vals[9];
        for (int i = 0 ; i < ntuples ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            vals[0] = in1->GetComponent(tup1, 0) * in2->GetComponent(tup2, 0) + 
                      in1->GetComponent(tup1, 1) * in2->GetComponent(tup2, 3) + 
                      in1->GetComponent(tup1, 2) * in2->GetComponent(tup2, 6); 
            vals[1] = in1->GetComponent(tup1, 0) * in2->GetComponent(tup2, 1) + 
                      in1->GetComponent(tup1, 1) * in2->GetComponent(tup2, 4) + 
                      in1->GetComponent(tup1, 2) * in2->GetComponent(tup2, 7); 
            vals[2] = in1->GetComponent(tup1, 0) * in2->GetComponent(tup2, 2) + 
                      in1->GetComponent(tup1, 1) * in2->GetComponent(tup2, 5) + 
                      in1->GetComponent(tup1, 2) * in2->GetComponent(tup2, 8); 
            vals[3] = in1->GetComponent(tup1, 3) * in2->GetComponent(tup2, 0) + 
                      in1->GetComponent(tup1, 4) * in2->GetComponent(tup2, 3) + 
                      in1->GetComponent(tup1, 5) * in2->GetComponent(tup2, 6); 
            vals[4] = in1->GetComponent(tup1, 3) * in2->GetComponent(tup2, 1) + 
                      in1->GetComponent(tup1, 4) * in2->GetComponent(tup2, 4) + 
                      in1->GetComponent(tup1, 5) * in2->GetComponent(tup2, 7); 
            vals[5] = in1->GetComponent(tup1, 3) * in2->GetComponent(tup2, 2) + 
                      in1->GetComponent(tup1, 4) * in2->GetComponent(tup2, 5) + 
                      in1->GetComponent(tup1, 5) * in2->GetComponent(tup2, 8); 
            vals[6] = in1->GetComponent(tup1, 6) * in2->GetComponent(tup2, 0) + 
                      in1->GetComponent(tup1, 7) * in2->GetComponent(tup2, 3) + 
                      in1->GetComponent(tup1, 8) * in2->GetComponent(tup2, 6); 
            vals[7] = in1->GetComponent(tup1, 6) * in2->GetComponent(tup2, 1) + 
                      in1->GetComponent(tup1, 7) * in2->GetComponent(tup2, 4) + 
                      in1->GetComponent(tup1, 8) * in2->GetComponent(tup2, 7); 
            vals[8] = in1->GetComponent(tup1, 6) * in2->GetComponent(tup2, 2) + 
                      in1->GetComponent(tup1, 7) * in2->GetComponent(tup2, 5) + 
                      in1->GetComponent(tup1, 8) * in2->GetComponent(tup2, 8); 
            out->SetTuple(i, vals);
        }
    }
    else if ((in1ncomps == 3) && (in2ncomps == 9))
    {
        double vals[3];
        for (int i = 0 ; i < ntuples ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            vals[0] = in1->GetComponent(tup1, 0) * in2->GetComponent(tup2, 0) + 
                      in1->GetComponent(tup1, 1) * in2->GetComponent(tup2, 3) + 
                      in1->GetComponent(tup1, 2) * in2->GetComponent(tup2, 6); 
            vals[1] = in1->GetComponent(tup1, 0) * in2->GetComponent(tup2, 1) + 
                      in1->GetComponent(tup1, 1) * in2->GetComponent(tup2, 4) + 
                      in1->GetComponent(tup1, 2) * in2->GetComponent(tup2, 7); 
            vals[2] = in1->GetComponent(tup1, 0) * in2->GetComponent(tup2, 2) + 
                      in1->GetComponent(tup1, 1) * in2->GetComponent(tup2, 5) + 
                      in1->GetComponent(tup1, 2) * in2->GetComponent(tup2, 8); 
            out->SetTuple(i, vals);
        }
    }
    else if ((in1ncomps == 9) && (in2ncomps == 3))
    {
        double vals[3];
        for (int i = 0 ; i < ntuples ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            vals[0] = in1->GetComponent(tup1, 0) * in2->GetComponent(tup2, 0) + 
                      in1->GetComponent(tup1, 1) * in2->GetComponent(tup2, 1) + 
                      in1->GetComponent(tup1, 2) * in2->GetComponent(tup2, 2); 
            vals[1] = in1->GetComponent(tup1, 3) * in2->GetComponent(tup2, 0) + 
                      in1->GetComponent(tup1, 4) * in2->GetComponent(tup2, 1) + 
                      in1->GetComponent(tup1, 5) * in2->GetComponent(tup2, 2); 
            vals[2] = in1->GetComponent(tup1, 6) * in2->GetComponent(tup2, 0) + 
                      in1->GetComponent(tup1, 7) * in2->GetComponent(tup2, 1) + 
                      in1->GetComponent(tup1, 8) * in2->GetComponent(tup2, 2);
            out->SetTuple(i, vals);
        }
    }
    else if (in1ncomps == in2ncomps)
    {
        bool handled = false;

        if(in1->GetDataType() == in2->GetDataType() &&
           in1->GetDataType() == out->GetDataType() &&
           in1->HasStandardMemoryLayout() &&
           in2->HasStandardMemoryLayout() &&
           // We don't want to add in these precisions because we'd probably
           // overflow when adding. These types will be handled the SetComponent
           // way.
           in1->GetDataType() != VTK_BIT &&
           in1->GetDataType() != VTK_CHAR &&
           in1->GetDataType() != VTK_SIGNED_CHAR &&
           in1->GetDataType() != VTK_UNSIGNED_CHAR &&
           in1->GetDataType() != VTK_SHORT &&
           in1->GetDataType() != VTK_UNSIGNED_SHORT)
        {

#define COMPUTE_MULT1(VARTYPE, LOW, HIGH) \
{ \
    VARTYPE *v1 = (VARTYPE *)in1->GetVoidPointer(0); \
    VARTYPE *v2 = (VARTYPE *)in2->GetVoidPointer(0); \
    VARTYPE *vout = (VARTYPE *)out->GetVoidPointer(0); \
    if(!var1IsSingleton && !var2IsSingleton) \
    { \
        for (int j = LOW ; j < HIGH ; j++) \
            vout[j] = v1[j] * v2[j]; \
    } \
    else if(!var1IsSingleton && var2IsSingleton) \
    { \
        for (int j = LOW ; j < HIGH ; j++) \
            vout[j] = v1[j] * v2[0]; \
    } \
    else if(var1IsSingleton && !var2IsSingleton) \
    { \
        for (int j = LOW ; j < HIGH ; j++) \
            vout[j] = v1[0] * v2[j]; \
    } \
    else/*if(var1IsSingleton && var2IsSingleton)*/ \
    { \
        for (int j = LOW ; j < HIGH ; j++) \
            vout[j] = v1[0] * v2[0]; \
    } \
}

#define COMPUTE_MULT3(VARTYPE, LOW, HIGH) \
{ \
    VARTYPE *v1 = (VARTYPE *)in1->GetVoidPointer(0); \
    VARTYPE *v2 = (VARTYPE *)in2->GetVoidPointer(0); \
    VARTYPE *vout = (VARTYPE *)out->GetVoidPointer(0); \
    if(!var1IsSingleton && !var2IsSingleton) \
    { \
        for (int j = LOW ; j < HIGH ; j++) \
        { \
            vout[j] = v1[j*3]   * v2[j*3] + \
                      v1[j*3+1] * v2[j*3+1] +  \
                      v1[j*3+2] * v2[j*3+2]; \
        } \
    } \
    else if(!var1IsSingleton && var2IsSingleton) \
    { \
        for (int j = LOW ; j < HIGH ; j++) \
        { \
            vout[j] = v1[j*3]   * v2[0*3] + \
                      v1[j*3+1] * v2[0*3+1] +  \
                      v1[j*3+2] * v2[0*3+2]; \
        } \
    } \
    else if(var1IsSingleton && !var2IsSingleton) \
    { \
        for (int j = LOW ; j < HIGH ; j++) \
        { \
            vout[j] = v1[0*3]   * v2[j*3] + \
                      v1[0*3+1] * v2[j*3+1] +  \
                      v1[0*3+2] * v2[j*3+2]; \
        } \
    } \
    else/*if(var1IsSingleton && var2IsSingleton)*/ \
    { \
        for (int j = LOW ; j < HIGH ; j++) \
        { \
            vout[j] = v1[0*3]   * v2[0*3] + \
                      v1[0*3+1] * v2[0*3+1] +  \
                      v1[0*3+2] * v2[0*3+2]; \
        } \
    } \
}

#ifdef _OPENMP
            StackTimer t0("avtBinaryMultipleExpression OpenMP");
            #pragma message("Compiling for OpenMP.")
            #pragma omp parallel
            {
                int threadnum = omp_get_thread_num();
                int numthreads = omp_get_num_threads();
                int low = ntuples*threadnum/numthreads;
                int high = ntuples*(threadnum+1)/numthreads;
#else
                int low = 0, high = ntuples;
#endif
                if(in1ncomps == 1)
                {
                    switch(in1->GetDataType())
                    {
                        vtkTemplateMacro(COMPUTE_MULT1(VTK_TT, low, high));
                    }
                    handled = true;
                }
                else if(in1ncomps == 3)
                {
                    switch(in1->GetDataType())
                    {
                        vtkTemplateMacro(COMPUTE_MULT3(VTK_TT, low, high));
                    }
                    handled = true;
                }
#ifdef _OPENMP
            }
#endif
        }

        if(!handled)
        {
            for (int i = 0 ; i < ntuples ; i++)
            {
                vtkIdType tup1 = (var1IsSingleton ? 0 : i);
                vtkIdType tup2 = (var2IsSingleton ? 0 : i);
                double dot = 0.;
                for (int j = 0 ; j < in1ncomps ; j++)
                {
                    double val1 = in1->GetComponent(tup1, j);
                    double val2 = in2->GetComponent(tup2, j);
                    dot += val1*val2;
                }
                out->SetTuple1(i, dot);
            }
        }
    }
    else if (in1ncomps > 1 && in2ncomps == 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val2 = in2->GetTuple1(tup2);
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                double val1 = in1->GetComponent(tup1, j);
                out->SetComponent(i, j, val1 * val2);
            }
        }
    }
    else if (in1ncomps == 1 && in2ncomps > 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val1 = in1->GetTuple1(tup1);
            for (int j = 0 ; j < in2ncomps ; j++)
            {
                double val2 = in2->GetComponent(tup2, j);
                out->SetComponent(i, j, val1 * val2);
            }
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                "Don't know how to multiply vectors of differing dimensions.");
    }
}


// ****************************************************************************
//  Method: avtBinaryMultiplyExpression::GetNumberOfComponentsInOutput
//
//  Purpose:
//      Sets the number of components in the output.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Sep 22 17:06:01 PDT 2003
//    Add matrix/tensor cases.
//
// ****************************************************************************

int
avtBinaryMultiplyExpression::GetNumberOfComponentsInOutput(int in1, int in2)
{
    if ((in1 == 9) && (in2 == 9))
        return 9;  // Matrix multiply
    else if ((in1 == 3) && (in2 == 9))
        return 3;  // Vector x Matrix = Vector
    else if ((in1 == 9) && (in2 == 3))
        return 3;  // Matrix x Vector = Vector
    else if (in1 == in2)
        return 1;  // We will do a dot product
    else
        return (in1 > in2 ? in1 : in2);
}
