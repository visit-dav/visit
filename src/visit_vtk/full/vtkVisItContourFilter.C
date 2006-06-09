/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItContourFilter.cxx,v $
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
#include "vtkVisItContourFilter.h"
#include <vtkAppendPolyData.h>
#include <vtkCellData.h>
#include <vtkContourFilter.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStructuredGrid.h>
#include <vtkSurfaceFromVolume.h>
#include <vtkTriangulationTables.h>
#include <vtkUnstructuredGrid.h>

#include <math.h>
#include <vector>


vtkCxxRevisionMacro(vtkVisItContourFilter, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkVisItContourFilter);

vtkVisItContourFilter::vtkVisItContourFilter()
{
  this->CellList = NULL;
  this->CellListSize = 0;
  this->Isovalue = 0.;
}

vtkVisItContourFilter::~vtkVisItContourFilter()
{
}

void vtkVisItContourFilter::SetCellList(int *cl, int size)
{
    this->CellList = cl;
    this->CellListSize = size;
}

int vtkVisItContourFilter::RequestData(
  vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkDataSet *input = vtkDataSet::SafeDownCast(
      inInfo->Get(vtkDataObject::DATA_OBJECT()));

    if (!input)
    {
        return 0;
    }

    vtkInformation *outInfo = outputVector->GetInformationObject(0);
    vtkPolyData *output = vtkPolyData::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT()));

    if (!output)
    {
        return 0;
    }

    int do_type = input->GetDataObjectType();
    if (do_type == VTK_RECTILINEAR_GRID)
    {
        return RectilinearGridExecute(input, output);
    }
    else if (do_type == VTK_STRUCTURED_GRID)
    {
        return StructuredGridExecute(input, output);
    }
    else if (do_type == VTK_UNSTRUCTURED_GRID)
    {
        return UnstructuredGridExecute(input, output);
    }
    else
    {
        return GeneralExecute(input, output);
    }
}


float *
vtkVisItContourFilter::GetPointScalars(vtkDataSet *in_ds)
{
    vtkDataArray *arr = in_ds->GetPointData()->GetScalars();
    if (arr == NULL)
    {
        vtkErrorMacro( << "No scalar point data.");
        return NULL;
    }

    if (arr->GetDataType() != VTK_FLOAT)
    {
        vtkErrorMacro( << "Scalar point data is not \"float\".");
        return NULL;
    }

    if (arr->GetNumberOfComponents() != 1)
    {
        vtkErrorMacro( << "Scalar point data is not really scalar.");
        return NULL;
    }

    return (float *) arr->GetVoidPointer(0);
}


// ****************************************************************************
//  Modifications:
//
//    Brad Whitlock, Thu Aug 12 14:45:24 PST 2004
//    Added float casts to the pow() arguments so it builds on MSVC7.Net.
//
//    Hank Childs, Wed Aug 25 13:21:02 PDT 2004
//    Better checks for degenerate datasets.
//
// ****************************************************************************

int
vtkVisItContourFilter::StructuredGridExecute(vtkDataSet *input, 
                                             vtkPolyData *output)
{
    int  i, j;

    vtkStructuredGrid *sg = (vtkStructuredGrid *)input;
    int pt_dims[3];
    sg->GetDimensions(pt_dims);
    if (pt_dims[0] <= 1 || pt_dims[1] <= 1 || pt_dims[2] <= 1)
    {
        return GeneralExecute(input, output);
    }
    int                nCells = sg->GetNumberOfCells();
    vtkPoints         *inPts  = sg->GetPoints();
    vtkCellData       *inCD   = sg->GetCellData();
    vtkPointData      *inPD   = sg->GetPointData();

    int ptSizeGuess = (this->CellList == NULL
                         ? (int) pow(float(nCells), 0.6667f) * 5 + 100
                         : CellListSize*5 + 100);

    vtkSurfaceFromVolume sfv(ptSizeGuess);

    float *pts_ptr = (float *) inPts->GetVoidPointer(0);

    float *var = GetPointScalars(input);
    if (var == NULL)
        return 0;

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
            dist[j] = var[ptId] - Isovalue;
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
    return 1;
}

// ****************************************************************************
//  Modifications:
//
//    Brad Whitlock, Thu Aug 12 14:45:24 PST 2004
//    Added float casts to the pow() arguments so it builds on MSVC7.Net.
//
//    Hank Childs, Wed Aug 25 13:21:02 PDT 2004
//    Better checks for degenerate datasets.
//
// ****************************************************************************

int 
vtkVisItContourFilter::RectilinearGridExecute(vtkDataSet *input,
                                              vtkPolyData *output)
{
    int  i, j;

    vtkRectilinearGrid *rg = (vtkRectilinearGrid *)input;
    int pt_dims[3];
    rg->GetDimensions(pt_dims);
    if (pt_dims[0] <= 1 || pt_dims[1] <= 1 || pt_dims[2] <= 1)
    {
        return GeneralExecute(input, output);
    }

    int           nCells = rg->GetNumberOfCells();
    float        *X      = (float* ) rg->GetXCoordinates()->GetVoidPointer(0);
    float        *Y      = (float* ) rg->GetYCoordinates()->GetVoidPointer(0);
    float        *Z      = (float* ) rg->GetZCoordinates()->GetVoidPointer(0);
    vtkCellData  *inCD   = rg->GetCellData();
    vtkPointData *inPD   = rg->GetPointData();

    int ptSizeGuess = (this->CellList == NULL
                         ? (int) pow(float(nCells), 0.6667f) * 5 + 100
                         : CellListSize*5 + 100);

    vtkSurfaceFromVolume sfv(ptSizeGuess);

    float *var = GetPointScalars(input);
    if (var == NULL)
        return 0;

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
            dist[j] = var[ptId] - Isovalue;
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
    return 1;
}

// Modifications:
//   Brad Whitlock, Thu Aug 12 14:45:24 PST 2004
//   Added float casts to the pow() arguments so it builds on MSVC7.Net.
//
///////////////////////////////////////////////////////////////////////////////

int 
vtkVisItContourFilter::UnstructuredGridExecute(vtkDataSet *input,
                                               vtkPolyData *output)
{
    // The routine here is a bit trickier than for the Rectilinear or
    // Structured grids.  We want to contour an unstructured grid -- but that
    // could mean any cell type.  We only have triangulation tables for
    // the finite element zoo.  So the gameplan is to contour any of the
    // elements of the finite element zoo.  If there are more elements left
    // over, contour them using the conventional VTK filters.  Finally,
    // append together the contours from the zoo with the contours from the
    // non-zoo elements.  If all the elements are from the zoo, then just
    // contour them with no appending.

    int   i, j;

    vtkUnstructuredGrid *ug = (vtkUnstructuredGrid *)input;

    int                nCells = ug->GetNumberOfCells();
    vtkPoints         *inPts  = ug->GetPoints();
    vtkCellData       *inCD   = ug->GetCellData();
    vtkPointData      *inPD   = ug->GetPointData();

    int ptSizeGuess = (this->CellList == NULL
                         ? (int) pow(float(nCells), 0.6667f) * 5 + 100
                         : CellListSize*5 + 100);

    vtkSurfaceFromVolume sfv(ptSizeGuess);

    vtkUnstructuredGrid *stuff_I_cant_contour = vtkUnstructuredGrid::New();
    stuff_I_cant_contour->SetPoints(ug->GetPoints());
    stuff_I_cant_contour->GetPointData()->ShallowCopy(ug->GetPointData());
    stuff_I_cant_contour->Allocate(nCells);

    float *pts_ptr = (float *) inPts->GetVoidPointer(0);
    float *var = GetPointScalars(input);
    if (var == NULL)
        return 0;


    int nToProcess = (CellList != NULL ? CellListSize : nCells);
    int numIcantContour = 0;
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
        bool canContour = false;
        switch (cellType)
        {
          case VTK_TETRA:
            triangulation_table = (int *) tetTriangulationTable;
            vertices_from_edges = (int *) tetVerticesFromEdges;
            tt_step = 7;
            canContour = true;
            break;
 
          case VTK_PYRAMID:
            triangulation_table = (int *) pyramidTriangulationTable;
            vertices_from_edges = (int *) pyramidVerticesFromEdges;
            tt_step = 13;
            canContour = true;
            break;
 
          case VTK_WEDGE:
            triangulation_table = (int *) wedgeTriangulationTable;
            vertices_from_edges = (int *) wedgeVerticesFromEdges;
            tt_step = 13;
            canContour = true;
            break;
 
          case VTK_HEXAHEDRON:
            triangulation_table = (int *) hexTriangulationTable;
            vertices_from_edges = (int *) hexVerticesFromEdges;
            tt_step = 16;
            canContour = true;
            break;

          default:
            canContour = false;
            break;
        }
 
        if (canContour)
        {
            const int max_pts = 8;
            float dist[max_pts];
            int lookup_case = 0;
            for (j = npts-1 ; j >= 0 ; j--)
            {
                dist[j] = var[pts[j]] - Isovalue;
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
            if (numIcantContour == 0)
                stuff_I_cant_contour->GetCellData()->
                                       CopyAllocate(ug->GetCellData(), nCells);

            stuff_I_cant_contour->InsertNextCell(cellType, npts, pts);
            stuff_I_cant_contour->GetCellData()->
                            CopyData(ug->GetCellData(), cellId, numIcantContour);
            numIcantContour++;
        }
    }

    if (numIcantContour > 0)
    {
        vtkPolyData *not_from_zoo  = vtkPolyData::New();
        ContourDataset(stuff_I_cant_contour, not_from_zoo);
        
        vtkPolyData *just_from_zoo = vtkPolyData::New();
        sfv.ConstructPolyData(inPD, inCD, just_from_zoo, pts_ptr);

        vtkAppendPolyData *appender = vtkAppendPolyData::New();
        appender->AddInput(not_from_zoo);
        appender->AddInput(just_from_zoo);
        appender->GetOutput()->Update();

        output->ShallowCopy(appender->GetOutput());
        appender->Delete();
        not_from_zoo->Delete();
        just_from_zoo->Delete();
    }
    else
    {
        sfv.ConstructPolyData(inPD, inCD, output, pts_ptr);
    }

    stuff_I_cant_contour->Delete();
    return 1;
}


int 
vtkVisItContourFilter::GeneralExecute(vtkDataSet *input, vtkPolyData* output)
{
    return ContourDataset(input, output);
}

int
vtkVisItContourFilter::ContourDataset(vtkDataSet *in_ds,
                                      vtkPolyData *out_pd)
{
    vtkContourFilter *contour = vtkContourFilter::New();
    contour->SetNumberOfContours(1);
    contour->SetValue(0, Isovalue);

    contour->SetInput(in_ds);
    contour->Update();

    out_pd->ShallowCopy(contour->GetOutput());
    
    contour->Delete();
    return 1;
}

void vtkVisItContourFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Isovalue: " << Isovalue << "\n";
}

int 
vtkVisItContourFilter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}
