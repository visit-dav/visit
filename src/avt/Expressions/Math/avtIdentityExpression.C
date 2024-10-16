// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtIdentityExpression.C                     //
// ************************************************************************* //

#include <avtIdentityExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtIdentityExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtIdentityExpression::avtIdentityExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtIdentityExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtIdentityExpression::~avtIdentityExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtIdentityExpression::DoOperation
//
//  Purpose:
//      Copies each component, tuple of a data array.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   December 9, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Sep 14 11:45:55 PDT 2007
//    Do not assume that the input variable is valid.  If someone makes
//    the expression "d = quadmesh2d", we will end up here with a NULL
//    input.
//
// ****************************************************************************
 
void
avtIdentityExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                   int ncomponents, int ntuples, vtkDataSet *in_ds)
{
    void *out_ptr = out->GetVoidPointer(0);

    if( in == NULL )
    {
        size_t numToCopy = out->GetDataTypeSize() * out->GetNumberOfComponents() * out->GetNumberOfTuples();
        memset(out_ptr, 0, numToCopy);
    }
    else if(in->GetDataType() == out->GetDataType())
    {
        void *in_ptr = in->GetVoidPointer(0);
        size_t numToCopy = in->GetDataTypeSize() * in->GetNumberOfComponents() * in->GetNumberOfTuples();
        memcpy(out_ptr, in_ptr, numToCopy);
    }
    else
    {
        for(int i = 0; i < ntuples; ++i)
            for(int j = 0; j < ncomponents; ++j)
                out->SetComponent(i, j, in->GetComponent(i, j));
    }
}

