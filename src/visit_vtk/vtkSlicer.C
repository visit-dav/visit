/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicer.cxx,v $
  Language:  C++
  Date:      $Date: 2002/02/22 21:16:54 $
  Version:   $Revision: 1.66 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSlicer.h"
#include "vtkFloatArray.h"
#include <vtkAppendPolyData.h>
#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkTriangulationTables.h>
#include <vtkSurfaceFromVolume.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <math.h>
#include <vector>


vtkCxxRevisionMacro(vtkSlicer, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkSlicer);

vtkSlicer::vtkSlicer()
{
  this->CellList = NULL;
  this->CellListSize = 0;
  Origin[0] = Origin[1] = Origin[2] = 0.;
  Normal[0] = Normal[1] = Normal[2] = 0.;
}

vtkSlicer::~vtkSlicer()
{
}

void vtkSlicer::SetCellList(int *cl, int size)
{
    this->CellList = cl;
    this->CellListSize = size;
}

void vtkSlicer::Execute()
{
    vtkDataSet *input  = GetInput();

    int do_type = input->GetDataObjectType();
    if (do_type == VTK_RECTILINEAR_GRID)
    {
        RectilinearGridExecute();
    }
    else if (do_type == VTK_STRUCTURED_GRID)
    {
        StructuredGridExecute();
    }
    else if (do_type == VTK_UNSTRUCTURED_GRID)
    {
        UnstructuredGridExecute();
    }
    else
    {
        GeneralExecute();
    }
}


void
vtkSlicer::StructuredGridExecute(void)
{
    int  i, j;

    vtkStructuredGrid *sg = (vtkStructuredGrid *) GetInput();
    int pt_dims[3];
    sg->GetDimensions(pt_dims);
    if (pt_dims[2] <= 1)
    {
        GeneralExecute();
        return;
    }

    int                nCells = sg->GetNumberOfCells();
    vtkPoints         *inPts  = sg->GetPoints();
    vtkCellData       *inCD   = sg->GetCellData();
    vtkPointData      *inPD   = sg->GetPointData();
    vtkPolyData       *output = GetOutput();

    int ptSizeGuess = (this->CellList == NULL
                         ? (int) pow(nCells, 0.6667) * 5 + 100
                         : ptSizeGuess = CellListSize*5 + 100);

    vtkSurfaceFromVolume sfv(ptSizeGuess);

    float D = Origin[0]*Normal[0] + Origin[1]*Normal[1] + Origin[2]*Normal[2];

    float *pts_ptr = (float *) inPts->GetVoidPointer(0);

    int cell_dims[3];
    cell_dims[0] = pt_dims[0]-1;
    cell_dims[1] = pt_dims[1]-1;
    cell_dims[2] = pt_dims[2]-1;
    int strideY = cell_dims[0];
    int strideZ = cell_dims[0]*cell_dims[1];
    int ptstrideY = pt_dims[0];
    int ptstrideZ = pt_dims[0]*pt_dims[1];
    int X_val[8] = { 0, 1, 1, 0, 0, 1, 1, 0 };
    int Y_val[8] = { 0, 0, 1, 1, 0, 0, 1, 1 };
    int Z_val[8] = { 0, 0, 0, 0, 1, 1, 1, 1 };
    int nToProcess = (CellList != NULL ? CellListSize : nCells);
    for (i = 0 ; i < nToProcess ; i++)
    {
        int cellId = (CellList != NULL ? CellList[i] : i);
        int cellI = cellId % cell_dims[0];
        int cellJ = (cellId/strideY) % cell_dims[1];
        int cellK = (cellId/strideZ);
        int lookup_case = 0;
        float dist[8];
        for (j = 7 ; j >= 0 ; j--)
        {
            int ptId = (cellI + X_val[j]) + (cellJ + Y_val[j])*ptstrideY +
                       (cellK + Z_val[j])*ptstrideZ;
            float *pt = pts_ptr + 3*ptId;
            dist[j] = pt[0]*Normal[0] + pt[1]*Normal[1] + pt[2]*Normal[2] - D;
            if (dist[j] >= 0)
                lookup_case++;
            if (j > 0)
                lookup_case *= 2;
        }

        int *triangulation_case = hexTriangulationTable[lookup_case];
        while (*triangulation_case != -1)
        {
            int tri[3];
            for (j = 0 ; j < 3 ; j++)
            {
                int pt1 = hexVerticesFromEdges[triangulation_case[j]][0];
                int pt2 = hexVerticesFromEdges[triangulation_case[j]][1];
                if (pt2 < pt1)
                {
                   int tmp = pt2;
                   pt2 = pt1;
                   pt1 = tmp;
                }
                float dir = dist[pt2] - dist[pt1];
                float amt = 0. - dist[pt1];
                float percent = 1. - (amt / dir);
                int ptId1 = (cellI + X_val[pt1]) +
                            (cellJ + Y_val[pt1])*ptstrideY +
                            (cellK + Z_val[pt1])*ptstrideZ;
                int ptId2 = (cellI + X_val[pt2]) +
                            (cellJ + Y_val[pt2])*ptstrideY +
                            (cellK + Z_val[pt2])*ptstrideZ;
                tri[j] = sfv.AddPoint(ptId1, ptId2, percent);
            }
            sfv.AddTriangle(cellId, tri[0], tri[1], tri[2]);
            triangulation_case += 3;
        }
    }

    sfv.ConstructPolyData(inPD, inCD, output, pts_ptr);
}

void vtkSlicer::RectilinearGridExecute(void)
{
    int  i, j;

    vtkRectilinearGrid *rg = (vtkRectilinearGrid *) GetInput();
    int pt_dims[3];
    rg->GetDimensions(pt_dims);
    if (pt_dims[2] <= 1)
    {
        GeneralExecute();
        return;
    }

    int           nCells = rg->GetNumberOfCells();
    float        *X      = (float* ) rg->GetXCoordinates()->GetVoidPointer(0);
    float        *Y      = (float* ) rg->GetYCoordinates()->GetVoidPointer(0);
    float        *Z      = (float* ) rg->GetZCoordinates()->GetVoidPointer(0);
    vtkCellData  *inCD   = rg->GetCellData();
    vtkPointData *inPD   = rg->GetPointData();
    vtkPolyData  *output = GetOutput();

    int ptSizeGuess = (this->CellList == NULL
                         ? (int) pow(nCells, 0.6667) * 5 + 100
                         : ptSizeGuess = CellListSize*5 + 100);

    vtkSurfaceFromVolume sfv(ptSizeGuess);

    float D = Origin[0]*Normal[0] + Origin[1]*Normal[1] + Origin[2]*Normal[2];

    int cell_dims[3];
    cell_dims[0] = pt_dims[0]-1;
    cell_dims[1] = pt_dims[1]-1;
    cell_dims[2] = pt_dims[2]-1;
    int strideY = cell_dims[0];
    int strideZ = cell_dims[0]*cell_dims[1];
    int ptstrideY = pt_dims[0];
    int ptstrideZ = pt_dims[0]*pt_dims[1];
    int X_val[8] = { 0, 1, 1, 0, 0, 1, 1, 0 };
    int Y_val[8] = { 0, 0, 1, 1, 0, 0, 1, 1 };
    int Z_val[8] = { 0, 0, 0, 0, 1, 1, 1, 1 };
    int nToProcess = (CellList != NULL ? CellListSize : nCells);
    for (i = 0 ; i < nToProcess ; i++)
    {
        int cellId = (CellList != NULL ? CellList[i] : i);
        int cellI = cellId % cell_dims[0];
        int cellJ = (cellId/strideY) % cell_dims[1];
        int cellK = (cellId/strideZ);
        int lookup_case = 0;
        float dist[8];
        for (j = 7 ; j >= 0 ; j--)
        {
            float pt[3];
            pt[0] = X[cellI + X_val[j]];
            pt[1] = Y[cellJ + Y_val[j]];
            pt[2] = Z[cellK + Z_val[j]];
            dist[j] = pt[0]*Normal[0] + pt[1]*Normal[1] + pt[2]*Normal[2] - D;
            if (dist[j] >= 0)
                lookup_case++;
            if (j > 0)
                lookup_case *= 2;
        }

        int *triangulation_case = hexTriangulationTable[lookup_case];
        while (*triangulation_case != -1)
        {
            int tri[3];
            for (j = 0 ; j < 3 ; j++)
            {
                int pt1 = hexVerticesFromEdges[triangulation_case[j]][0];
                int pt2 = hexVerticesFromEdges[triangulation_case[j]][1];
                if (pt2 < pt1)
                {
                   int tmp = pt2;
                   pt2 = pt1;
                   pt1 = tmp;
                }
                float dir = dist[pt2] - dist[pt1];
                float amt = 0. - dist[pt1];
                float percent = 1. - (amt / dir);
                int ptId1 = (cellI + X_val[pt1]) +
                            (cellJ + Y_val[pt1])*ptstrideY +
                            (cellK + Z_val[pt1])*ptstrideZ;
                int ptId2 = (cellI + X_val[pt2]) +
                            (cellJ + Y_val[pt2])*ptstrideY +
                            (cellK + Z_val[pt2])*ptstrideZ;
                tri[j] = sfv.AddPoint(ptId1, ptId2, percent);
            }
            sfv.AddTriangle(cellId, tri[0], tri[1], tri[2]);
            triangulation_case += 3;
        }
    }

    sfv.ConstructPolyData(inPD, inCD, output, pt_dims, X, Y, Z);
}

void vtkSlicer::UnstructuredGridExecute(void)
{
    // The routine here is a bit trickier than for the Rectilinear or
    // Structured grids.  We want to slice an unstructured grid -- but that
    // could mean any cell type.  We only have triangulation tables for
    // the finite element zoo.  So the gameplan is to slice any of the
    // elements of the finite element zoo.  If there are more elements left
    // over, slice them using the conventional VTK filters.  Finally,
    // append together the slices from the zoo with the slices from the
    // non-zoo elements.  If all the elements are from the zoo, then just
    // slice them with no appending.

    int   i, j;

    vtkUnstructuredGrid *ug = (vtkUnstructuredGrid *) GetInput();

    int                nCells = ug->GetNumberOfCells();
    vtkPoints         *inPts  = ug->GetPoints();
    vtkCellData       *inCD   = ug->GetCellData();
    vtkPointData      *inPD   = ug->GetPointData();
    vtkPolyData       *output = GetOutput();

    int ptSizeGuess = (this->CellList == NULL
                         ? (int) pow(nCells, 0.6667) * 5 + 100
                         : ptSizeGuess = CellListSize*5 + 100);

    vtkSurfaceFromVolume sfv(ptSizeGuess);

    vtkUnstructuredGrid *stuff_I_cant_slice = vtkUnstructuredGrid::New();
    stuff_I_cant_slice->SetPoints(ug->GetPoints());
    stuff_I_cant_slice->GetPointData()->ShallowCopy(ug->GetPointData());
    stuff_I_cant_slice->Allocate(nCells);

    float D = Origin[0]*Normal[0] + Origin[1]*Normal[1] + Origin[2]*Normal[2];
    float *pts_ptr = (float *) inPts->GetVoidPointer(0);

    int nToProcess = (CellList != NULL ? CellListSize : nCells);
    int numIcantSlice = 0;
    for (i = 0 ; i < nToProcess ; i++)
    {
        int        cellId = (CellList != NULL ? CellList[i] : i);
        int        cellType = ug->GetCellType(cellId);
        int        npts;
        vtkIdType *pts;
        ug->GetCellPoints(cellId, npts, pts);
        int *triangulation_table = NULL;
        int *vertices_from_edges = NULL;
        int tt_step = 0;
        bool canSlice = false;
        switch (cellType)
        {
          case VTK_TETRA:
            triangulation_table = (int *) tetTriangulationTable;
            vertices_from_edges = (int *) tetVerticesFromEdges;
            tt_step = 7;
            canSlice = true;
            break;
 
          case VTK_PYRAMID:
            triangulation_table = (int *) pyramidTriangulationTable;
            vertices_from_edges = (int *) pyramidVerticesFromEdges;
            tt_step = 13;
            canSlice = true;
            break;
 
          case VTK_WEDGE:
            triangulation_table = (int *) wedgeTriangulationTable;
            vertices_from_edges = (int *) wedgeVerticesFromEdges;
            tt_step = 13;
            canSlice = true;
            break;
 
          case VTK_HEXAHEDRON:
            triangulation_table = (int *) hexTriangulationTable;
            vertices_from_edges = (int *) hexVerticesFromEdges;
            tt_step = 16;
            canSlice = true;
            break;

          default:
            canSlice = false;
            break;
        }
 
        if (canSlice)
        {
            const int max_pts = 8;
            float dist[max_pts];
            int lookup_case = 0;
            for (j = npts-1 ; j >= 0 ; j--)
            {
                float *pt = pts_ptr + 3*pts[j];
                dist[j] = pt[0]*Normal[0] + pt[1]*Normal[1] +
                          pt[2]*Normal[2] - D;
                if (dist[j] >= 0)
                    lookup_case++;
                if (j > 0)
                    lookup_case *= 2;
            }

            int *triangulation_case = triangulation_table +lookup_case*tt_step;
            while (*triangulation_case != -1)
            {
                int tri[3];
                for (j = 0 ; j < 3 ; j++)
                {
                    int pt1 = vertices_from_edges[2*triangulation_case[j]];
                    int pt2 = vertices_from_edges[2*triangulation_case[j]+1];
                    if (pt2 < pt1)
                    {
                       int tmp = pt2;
                       pt2 = pt1;
                       pt1 = tmp;
                    }
                    float dir = dist[pt2] - dist[pt1];
                    float amt = 0. - dist[pt1];
                    float percent = 1. - (amt / dir);
                    int ptId1 = pts[pt1];
                    int ptId2 = pts[pt2];
                    tri[j] = sfv.AddPoint(ptId1, ptId2, percent);
                }
                sfv.AddTriangle(cellId, tri[0], tri[1], tri[2]);
                triangulation_case += 3;
            }
        }
        else
        {
            if (numIcantSlice == 0)
                stuff_I_cant_slice->GetCellData()->
                                       CopyAllocate(ug->GetCellData(), nCells);

            stuff_I_cant_slice->InsertNextCell(cellType, npts, pts);
            stuff_I_cant_slice->GetCellData()->
                            CopyData(ug->GetCellData(), cellId, numIcantSlice);
            numIcantSlice++;
        }
    }

    if (numIcantSlice > 0)
    {
        vtkPolyData *not_from_zoo  = vtkPolyData::New();
        SliceDataset(stuff_I_cant_slice, not_from_zoo);
        
        vtkPolyData *just_from_zoo = vtkPolyData::New();
        sfv.ConstructPolyData(inPD, inCD, just_from_zoo, pts_ptr);

        vtkAppendPolyData *appender = vtkAppendPolyData::New();
        appender->AddInput(not_from_zoo);
        appender->AddInput(just_from_zoo);
        appender->SetOutput(output);
        output->Update();

        output->SetSource(NULL);
        appender->Delete();
        not_from_zoo->Delete();
        just_from_zoo->Delete();
    }
    else
    {
        sfv.ConstructPolyData(inPD, inCD, output, pts_ptr);
    }

    stuff_I_cant_slice->Delete();
}


void vtkSlicer::GeneralExecute(void)
{
    SliceDataset(GetInput(), GetOutput());
}

void vtkSlicer::SliceDataset(vtkDataSet *in_ds, vtkPolyData *out_pd)
{
    vtkCutter *cutter = vtkCutter::New();
    vtkPlane  *plane  = vtkPlane::New();
    plane->SetOrigin(Origin[0], Origin[1], Origin[2]);
    plane->SetNormal(Normal[0], Normal[1], Normal[2]);
    cutter->SetCutFunction(plane);

    cutter->SetInput(in_ds);
    cutter->Update();

    out_pd->ShallowCopy(cutter->GetOutput());
    
    cutter->Delete();
    plane->Delete();
}

void vtkSlicer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Normal: " << Normal[0] << ", " << Normal[1] << ", "
     << Normal[2] << "\n";
  os << indent << "Origin: " << Origin[0] << ", " << Origin[1] << ", "
     << Origin[2] << "\n";
}
