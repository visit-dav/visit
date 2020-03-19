// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtThreadIdExpression.C                          //
// ************************************************************************* //

#include <avtThreadIdExpression.h>

#include <vtkIntArray.h>
#include <vtkDataSet.h>

#include <avtParallel.h>


// ****************************************************************************
//  Method: avtThreadIdExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: David Camp
//  Creation:   February 13, 2013
//
// ****************************************************************************

avtThreadIdExpression::avtThreadIdExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtThreadIdExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: David Camp
//  Creation:   February 13, 2013
//
// ****************************************************************************

avtThreadIdExpression::~avtThreadIdExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtThreadIdExpression::DeriveVariable
//
//  Purpose:
//      Assigns the thread Id to each variable.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   David Camp
//  Creation:     February 13, 2013
//
// ****************************************************************************

vtkDataArray *
avtThreadIdExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
#if defined(VISIT_THREADS)
   #if defined(__APPLE__)

      int threadId = *((int*) pthread_self());
   #else
      int threadId = pthread_self();
   #endif
#else
   int threadId = 0;
#endif
    vtkIdType npts   = in_ds->GetNumberOfPoints();

    vtkIntArray *rv = vtkIntArray::New();
    rv->SetNumberOfTuples(npts);
    for (vtkIdType i = 0 ; i < npts ; ++i)
    {
        rv->SetValue(i, threadId);
    }

    return rv;
}

