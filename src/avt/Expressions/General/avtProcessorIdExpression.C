// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtProcessorIdExpression.C                          //
// ************************************************************************* //

#include <avtProcessorIdExpression.h>

#include <vtkIntArray.h>
#include <vtkDataSet.h>

#include <avtParallel.h>


// ****************************************************************************
//  Method: avtProcessorIdExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtProcessorIdExpression::avtProcessorIdExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtProcessorIdExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtProcessorIdExpression::~avtProcessorIdExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtProcessorIdExpression::DeriveVariable
//
//  Purpose:
//      Assigns the processor Id to each variable.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     November 19, 2002
//
// ****************************************************************************

vtkDataArray *
avtProcessorIdExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    int procId = PAR_Rank();
    vtkIdType npts   = in_ds->GetNumberOfPoints();

    vtkIntArray *rv = vtkIntArray::New();
    rv->SetNumberOfTuples(npts);
    for (vtkIdType i = 0 ; i < npts ; ++i)
    {
        rv->SetValue(i, procId);
    }

    return rv;
}


