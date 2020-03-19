// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              degen_meshes.C                               //
// ************************************************************************* //

#include <vtkDataSetWriter.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>


// ****************************************************************************
//  Function: WriteOutRectilinearMesh
//
//  Purpose:
//      Writes out a rectilinear mesh with the specified dimensions.
//
//  Programmer: Hank Childs
//  Creation:   August 25, 2004
//
// ****************************************************************************

static void
WriteOutRectilinearMesh(const char *fname, int dimI, int dimJ, int dimK)
{
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(dimI, dimJ, dimK);

    vtkFloatArray *x = vtkFloatArray::New();
    x->SetNumberOfTuples(dimI);
    if (dimI > 1)
    {
        for (int i = 0 ; i < dimI ; i++)
        {
            x->SetTuple1(i, ((float) i) / ((float) dimI)+1.);
        }
    }
    else
        x->SetTuple1(0, 1.);
    rgrid->SetXCoordinates(x);
    x->Delete();

    vtkFloatArray *y = vtkFloatArray::New();
    y->SetNumberOfTuples(dimJ);
    if (dimJ > 1)
    {
        for (int i = 0 ; i < dimJ ; i++)
        {
            y->SetTuple1(i, ((float) i) / ((float) dimJ)+1.);
        }
    }
    else
        y->SetTuple1(0, 1.);
    rgrid->SetYCoordinates(y);
    y->Delete();

    vtkFloatArray *z = vtkFloatArray::New();
    z->SetNumberOfTuples(dimK);
    if (dimK > 1)
    {
        for (int i = 0 ; i < dimK ; i++)
        {
            z->SetTuple1(i, ((float) i) / ((float) dimK)+1.);
        }
    }
    else
        z->SetTuple1(0, 1.);
    rgrid->SetZCoordinates(z);
    z->Delete();

    vtkDataSetWriter *wrtr = vtkDataSetWriter::New();
    wrtr->SetFileName(fname);
    wrtr->SetInputData(rgrid);
    wrtr->Write();
 
    wrtr->Delete();
    rgrid->Delete();
}


// ****************************************************************************
//  Function: WriteOutCurvilinearMesh
//
//  Purpose:
//      Writes out a curvilinear mesh with the specified dimensions.
//
//  Programmer: Hank Childs
//  Creation:   August 25, 2004
//
// ****************************************************************************

static void
WriteOutCurvilinearMesh(const char *fname, int dimI, int dimJ, int dimK)
{
    vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
    sgrid->SetDimensions(dimI, dimJ, dimK);

    int npts = dimI*dimJ*dimK;
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(npts);
    int ptIdx = 0;
    for (int k = 0 ; k < dimK ; k++)
        for (int j = 0 ; j < dimJ ; j++)
            for (int i = 0 ; i < dimI ; i++)
            {
                float x = 1.;
                if (dimI > 1)
                   x = ((float) i) / ((float) dimI) + 1.;
                float y = 1.;
                if (dimJ > 1)
                   y = ((float) j) / ((float) dimJ) + 1.;
                float z = 1.;
                if (dimK > 1)
                   z = ((float) k) / ((float) dimK) + 1.;
                pts->SetPoint(ptIdx++, x, y, z);
            }
    sgrid->SetPoints(pts);
    pts->Delete();

    vtkDataSetWriter *wrtr = vtkDataSetWriter::New();
    wrtr->SetFileName(fname);
    wrtr->SetInputData(sgrid);
    wrtr->Write();
 
    wrtr->Delete();
    sgrid->Delete();
}


// ****************************************************************************
//  Function: main
//
//  Purpose:
//      Writes out some degenerate meshes that can be used for regression
//      testing.
//
//  Programmer: Hank Childs
//  Creation:   August 25, 2004
//
// ****************************************************************************

int main()
{
    WriteOutRectilinearMesh("rect_flat_i.vtk", 1, 25, 25);
    WriteOutRectilinearMesh("rect_flat_j.vtk", 25, 1, 25);
    WriteOutRectilinearMesh("rect_flat_k.vtk", 25, 25, 1);
    WriteOutCurvilinearMesh("curv_flat_i.vtk", 1, 25, 25);
    WriteOutCurvilinearMesh("curv_flat_j.vtk", 25, 1, 25);
    WriteOutCurvilinearMesh("curv_flat_k.vtk", 25, 25, 1);
}


