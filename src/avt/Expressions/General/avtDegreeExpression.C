// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtDegreeExpression.C                             //
// ************************************************************************* //

#include <avtDegreeExpression.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkIntArray.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtDegreeExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDegreeExpression::avtDegreeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtDegreeExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDegreeExpression::~avtDegreeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtDegreeExpression::DeriveVariable
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
//  Programmer:   Hank Childs
//  Creation:     June 7, 2002
//
// ****************************************************************************

vtkDataArray *
avtDegreeExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    //
    // Set up an array that we can do bookkeepping in.  Entry i will be the
    // number of cells incident to point i.
    //
    vtkIdType nPoints = in_ds->GetNumberOfPoints();
    int *degree = new int[nPoints];
    for (vtkIdType i = 0 ; i < nPoints ; i++)
    {
        degree[i] = 0;
    }

    //
    // Iterate over each cell in the mesh and ask it which points it is
    // incident to.  Reflect that in our counts of how many cells each point
    // is incident to (ie the degree).
    //
    vtkIdType nCells = in_ds->GetNumberOfCells();
    for (vtkIdType i = 0 ; i < nCells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        vtkIdType numPointsForThisCell = cell->GetNumberOfPoints();
        for (vtkIdType j = 0 ; j < numPointsForThisCell ; j++)
        {
            vtkIdType id = cell->GetPointId(j);
            if (id >= 0 && id < nPoints)
            {
                degree[id]++;
            }
        }
    }

    //
    // Set up a VTK variable reflecting the degrees we have calculated.
    //
    vtkIntArray *dv = vtkIntArray::New();
    dv->SetNumberOfTuples(nPoints);
    for (vtkIdType i = 0 ; i < nPoints ; i++)
    {
        dv->SetValue(i, degree[i]);
    }
    delete [] degree;

    return dv;
}


