// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtFacePlanarity.C                             //
// ************************************************************************* //

#include <avtFacePlanarity.h>

#include <float.h>

#include <vtkCell.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPoints.h>

static double
GetFacePlanarityForFace(vtkCell *face, bool takeRel)
{
    vtkIdType nPoints = face->GetNumberOfPoints();

    if (nPoints <= 3)
        return 0;

    //
    // Compute average edge length first if necessary.
    //
    double len_sum = 1;
    if (takeRel)
    {
        len_sum = 0;
        for (vtkIdType i = 0; i < nPoints; i++)
        {
            vtkIdType ptids[2];
            double ptx[2][3];
            for (int j = 0; j < 2; j++)
            {
                ptids[j] = (i+j) % nPoints;
                face->GetPoints()->GetPoint(ptids[j],ptx[j]);
            }
            double n[3] = {ptx[0][0] - ptx[1][0], 
                           ptx[0][1] - ptx[1][1], 
                           ptx[0][2] - ptx[1][2]};
            double n_len = sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
            len_sum += n_len;
        }
        len_sum /= nPoints;
    }

    double distMax = 0;
    for (vtkIdType i = 0; i < nPoints; i++)
    {
        vtkIdType ptids[4];
        double ptx[4][3];
        for (int j = 0; j < 4; j++)
        {
            ptids[j] = (i+j) % nPoints;
            face->GetPoints()->GetPoint(ptids[j],ptx[j]);
        }

        //
        // Compute a vector normal to plane of first 3 points
        // n1 = p0 - p1, n2 = p2 - p1
        //
        double n1[3] = {ptx[0][0] - ptx[1][0], 
                        ptx[0][1] - ptx[1][1], 
                        ptx[0][2] - ptx[1][2]};
        double n2[3] = {ptx[2][0] - ptx[1][0], 
                        ptx[2][1] - ptx[1][1], 
                        ptx[2][2] - ptx[1][2]};
        double n1Xn2[3] = {  n1[1]*n2[2] - n1[2]*n2[1],
                           -(n1[0]*n2[2] - n1[2]*n2[0]),
                             n1[0]*n2[1] - n1[1]*n2[0]};

        //
        // Normalize it.
        //
        double n1Xn2_len = 
                   sqrt(n1Xn2[0]*n1Xn2[0]+n1Xn2[1]*n1Xn2[1]+n1Xn2[2]*n1Xn2[2]);
        if (n1Xn2_len > 0)
        {
            n1Xn2[0] /= n1Xn2_len;
            n1Xn2[1] /= n1Xn2_len;
            n1Xn2[2] /= n1Xn2_len;
        }

        //
        // Compute distance to 4th point via dot-product with normal computed 
        // above.
        //
        double p4[3] = {ptx[3][0] - ptx[1][0], 
                        ptx[3][1] - ptx[1][1], 
                        ptx[3][2] - ptx[1][2]};
        double dist = p4[0] * n1Xn2[0] + p4[1] * n1Xn2[1] + p4[2] * n1Xn2[2];
        if (dist < 0) dist = -dist;
        if (takeRel)
            dist /= len_sum;
        if (dist > distMax)
            distMax = dist;
    }
    return distMax;
}

// ****************************************************************************
//  Funcion: GetFacePlanarityForCell
//
//  Purpose: Compute Face planarity measure over whole cell.
//
//  Programmer: Mark C. Miller 
//  Creation:   June 3, 2009 
//
// ****************************************************************************
 
static double
GetFacePlanarityForCell(vtkCell *cell, bool takeRel)
{
    int celltype = cell->GetCellType();
    if (celltype == VTK_EMPTY_CELL ||
        celltype == VTK_VERTEX ||
        celltype == VTK_POLY_VERTEX ||
        celltype == VTK_LINE ||
        celltype == VTK_POLY_LINE ||
        celltype == VTK_TRIANGLE ||
        celltype == VTK_TRIANGLE_STRIP ||
        celltype == VTK_POLYGON || // assumed 2d
        celltype == VTK_PIXEL ||
        celltype == VTK_TETRA)
    {
        return 0;
    }
    else if (celltype == VTK_QUAD)
    {
        return GetFacePlanarityForFace(cell, takeRel);
    }
    else
    {
        int nFaces = cell->GetNumberOfFaces();
        if (nFaces == 0)
            return 0;
        double distMax = 0;
        for (int f = 0; f < nFaces; f++)
        {
            double dist = GetFacePlanarityForFace(cell->GetFace(f), takeRel);
            if (dist > distMax)
                distMax = dist;
        }
        return distMax;
    }
}

// ****************************************************************************
//  Method: avtFacePlanarity constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2008
//
//  Modifications:
//
//    Alister Maguire, Thu Jun 18 10:02:58 PDT 2020
//    Set canApplyToDirectDatabaseQOT to false.
//
//    Alister Maguire, Fri Nov  6 08:53:20 PST 2020
//    Removed direct database QOT disabler as they are now compatible.
//
// ****************************************************************************

avtFacePlanarity::avtFacePlanarity()
{
    takeRel = false;
}

// ****************************************************************************
//  Method: avtFacePlanarity::DeriveVariable
//
//  Purpose:
//      Find the minimum or maximum corner angle.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2008
//
// ****************************************************************************

vtkDataArray *
avtFacePlanarity::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkDataArray *arr = CreateArrayFromMesh(in_ds);
    vtkIdType ncells = in_ds->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    for (vtkIdType i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        double vol = GetFacePlanarityForCell(cell, takeRel);
        arr->SetTuple1(i, vol);
    }

    return arr;
}
