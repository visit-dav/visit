// ************************************************************************* //
//                             avtDegreeFilter.C                             //
// ************************************************************************* //

#include <avtDegreeFilter.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtDegreeFilter::DeriveVariable
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
avtDegreeFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int  i, j;

    //
    // Set up an array that we can do bookkeepping in.  Entry i will be the
    // number of cells incident to point i.
    //
    int nPoints = in_ds->GetNumberOfPoints();
    int *degree = new int[nPoints];
    for (i = 0 ; i < nPoints ; i++)
    {
        degree[i] = 0;
    }

    //
    // Iterate over each cell in the mesh and ask it which points it is
    // incident to.  Reflect that in our counts of how many cells each point
    // is incident to (ie the degree).
    //
    int nCells = in_ds->GetNumberOfCells();
    for (i = 0 ; i < nCells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        int numPointsForThisCell = cell->GetNumberOfPoints();
        for (j = 0 ; j < numPointsForThisCell ; j++)
        {
            int id = cell->GetPointId(j);
            if (id >= 0 && id < nPoints)
            {
                degree[id]++;
            }
        }
    }

    //
    // Set up a VTK variable reflecting the degrees we have calculated.
    //
    vtkFloatArray *dv = vtkFloatArray::New();
    dv->SetNumberOfTuples(nPoints);
    for (i = 0 ; i < nPoints ; i++)
    {
        float f = (float) degree[i];
        dv->SetTuple(i, &f);
    }
    delete [] degree;

    return dv;
}


