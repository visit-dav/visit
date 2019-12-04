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

#include <vtkCreateTriangleHelpers.h>

#include <math.h>
#include <vector>


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

void vtkVisItContourFilter::SetCellList(const vtkIdType *cl, vtkIdType size)
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


vtkDataArray *
vtkVisItContourFilter::GetPointScalars(vtkDataSet *in_ds)
{
    vtkDataArray *arr = in_ds->GetPointData()->GetScalars();
    if (arr == NULL)
    {
        vtkErrorMacro( << "No scalar point data.");
        return NULL;
    }

    if (arr->GetNumberOfComponents() != 1)
    {
        vtkErrorMacro( << "Scalar point data is not really scalar.");
        return NULL;
    }

    return arr;
}

// ****************************************************************************
// Class: IsoDistanceFunction
//
// Purpose:
//   This functor computes a distance function as something of a callback
//   to vtkStructuredCreateTriangles.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 12 17:10:44 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

template <typename T>
class IsoDistanceFunction
{
public:
    IsoDistanceFunction(const int *pt_dims, vtkDataArray *v, T iso) : 
        var((const T *)v->GetVoidPointer(0)), Isovalue(iso)
    {
        ptstrideY = (vtkIdType)pt_dims[0];
        ptstrideZ = (vtkIdType)(pt_dims[0] * pt_dims[1]);
    }

    inline T operator()(vtkIdType cellI,   vtkIdType cellJ,   vtkIdType cellK,
                        vtkIdType iOffset, vtkIdType jOffset, vtkIdType kOffset) const
    {
        vtkIdType ptId = (cellI + iOffset) + (cellJ + jOffset) * ptstrideY +
                         (cellK + kOffset) * ptstrideZ;
        return var[ptId] - Isovalue;
    }

    inline T operator()(vtkIdType ptId) const
    {
        return var[ptId] - Isovalue;
    }

    const T  *var;
    T         Isovalue;
    vtkIdType ptstrideY, ptstrideZ; 
};

// ****************************************************************************
// Class: IsoDistanceFunction
//
// Purpose:
//   This functor computes a distance function as something of a callback
//   to vtkStructuredCreateTriangles.
//
// Notes:      Use GetTuple1 to access data.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 12 17:10:44 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

class GeneralIsoDistanceFunction
{
public:
    GeneralIsoDistanceFunction(const int *pt_dims, vtkDataArray *v, double iso) : 
        var(v), Isovalue(iso)
    {
        ptstrideY = (vtkIdType)pt_dims[0];
        ptstrideZ = (vtkIdType)(pt_dims[0] * pt_dims[1]);
    }

    inline double operator()(vtkIdType cellI,   vtkIdType cellJ,   vtkIdType cellK,
                        vtkIdType iOffset, vtkIdType jOffset, vtkIdType kOffset) const
    {
        vtkIdType ptId = (cellI + iOffset) + (cellJ + jOffset) * ptstrideY +
                         (cellK + kOffset) * ptstrideZ;
        return var->GetTuple1(ptId) - Isovalue;
    }

    inline double operator()(vtkIdType ptId) const
    {
        return var->GetTuple1(ptId) - Isovalue;
    }

    vtkDataArray *var;
    double        Isovalue;
    vtkIdType     ptstrideY, ptstrideZ; 
};

// ****************************************************************************
//  Modifications:
//
//    Brad Whitlock, Thu Aug 12 14:45:24 PST 2004
//    Added float casts to the pow() arguments so it builds on MSVC7.Net.
//
//    Hank Childs, Wed Aug 25 13:21:02 PDT 2004
//    Better checks for degenerate datasets.
//
//    Brad Whitlock, Mon Mar  5 16:43:50 PST 2012
//    Moved code into StructuredCreateTriangles and instantiated for float
//    and double. Move distance function into IsoDistanceFunction.
//
//    Brad Whitlock, Thu Jul 23 16:01:46 PDT 2015
//    Support for non-standard memory layout.
//
// ****************************************************************************

int
vtkVisItContourFilter::StructuredGridExecute(vtkDataSet *input, 
                                             vtkPolyData *output)
{
    vtkStructuredGrid *sg = (vtkStructuredGrid *)input;
    int pt_dims[3];
    sg->GetDimensions(pt_dims);
    if (pt_dims[0] <= 1 || pt_dims[1] <= 1 || pt_dims[2] <= 1)
    {
        return GeneralExecute(input, output);
    }
    vtkIdType          nCells = sg->GetNumberOfCells();
    vtkCellData       *inCD   = sg->GetCellData();
    vtkPointData      *inPD   = sg->GetPointData();

    vtkIdType ptSizeGuess = (this->CellList == NULL
                         ? (vtkIdType) pow(float(nCells), 0.6667f) * 5 + 100
                         : CellListSize*5 + 100);

    vtkSurfaceFromVolume sfv(ptSizeGuess);

    vtkDataArray *var = GetPointScalars(input);
    if (var == NULL)
        return 0;

    int accessMethod = 0;
    if(var->HasStandardMemoryLayout())
    {
        if(var->GetDataType() == VTK_FLOAT)
            accessMethod = 1;
        else if(var->GetDataType() == VTK_DOUBLE)
            accessMethod = 2;
    }

    if(accessMethod == 1)
    {
        vtkStructuredCreateTriangles<float, IsoDistanceFunction<float> >(
            sfv, this->CellList, this->CellListSize, nCells,
            pt_dims, IsoDistanceFunction<float>(pt_dims, var, (float)this->Isovalue));
    }
    else if(accessMethod == 2)
    {
        vtkStructuredCreateTriangles<double, IsoDistanceFunction<double> >(
            sfv, this->CellList, this->CellListSize, nCells,
            pt_dims, IsoDistanceFunction<double>(pt_dims, var, this->Isovalue));
    }
    else
    {
        vtkStructuredCreateTriangles<double, GeneralIsoDistanceFunction>(
            sfv, this->CellList, this->CellListSize, nCells,
            pt_dims, GeneralIsoDistanceFunction(pt_dims, var, this->Isovalue));
    }

    sfv.ConstructPolyData(inPD, inCD, output, sg->GetPoints());
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
//    Brad Whitlock, Mon Mar  5 16:43:50 PST 2012
//    Moved code into StructuredCreateTriangles and instantiated for float
//    and double. Pass coordinate data arrays directly to sfv so we don't
//    assume just float.
//
// ****************************************************************************

int 
vtkVisItContourFilter::RectilinearGridExecute(vtkDataSet *input,
                                              vtkPolyData *output)
{
    vtkRectilinearGrid *rg = (vtkRectilinearGrid *)input;
    int pt_dims[3];
    rg->GetDimensions(pt_dims);
    if (pt_dims[0] <= 1 || pt_dims[1] <= 1 || pt_dims[2] <= 1)
    {
        return GeneralExecute(input, output);
    }

    vtkIdType     nCells = rg->GetNumberOfCells();
    vtkCellData  *inCD   = rg->GetCellData();
    vtkPointData *inPD   = rg->GetPointData();

    vtkIdType ptSizeGuess = (this->CellList == NULL
                         ? (vtkIdType) pow(float(nCells), 0.6667f) * 5 + 100
                         : this->CellListSize*5 + 100);

    vtkSurfaceFromVolume sfv(ptSizeGuess);

    vtkDataArray *var = GetPointScalars(input);
    if (var == NULL)
        return 0;

    if(var->GetDataType() == VTK_FLOAT)
    {
        vtkStructuredCreateTriangles<float, IsoDistanceFunction<float> >(
            sfv, this->CellList, this->CellListSize, nCells,
            pt_dims, IsoDistanceFunction<float>(pt_dims, var, (float)this->Isovalue));
    }
    else if(var->GetDataType() == VTK_DOUBLE)
    {
        vtkStructuredCreateTriangles<double, IsoDistanceFunction<double> >(
            sfv, this->CellList, this->CellListSize, nCells,
            pt_dims, IsoDistanceFunction<double>(pt_dims, var, this->Isovalue));
    }
    else
    {
        vtkStructuredCreateTriangles<double, GeneralIsoDistanceFunction >(
            sfv, this->CellList, this->CellListSize, nCells,
            pt_dims, GeneralIsoDistanceFunction(pt_dims, var, this->Isovalue));
    }

    sfv.ConstructPolyData(inPD, inCD, output, pt_dims, 
        rg->GetXCoordinates(), rg->GetYCoordinates(), rg->GetZCoordinates());
    return 1;
}

// Modifications:
//   Brad Whitlock, Thu Aug 12 14:45:24 PST 2004
//   Added float casts to the pow() arguments so it builds on MSVC7.Net.
//
//   Brad Whitlock, Wed Apr 11 11:37:18 PDT 2012
//   When we can't contour a cell, insert faces too for polyhedral cells.
//
//   Kathleen Biagas, Fri Jan 25 16:04:46 PST 2013
//   Call Update on the filter, not the data object.
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

    vtkUnstructuredGrid *ug = (vtkUnstructuredGrid *)input;

    vtkIdType          nCells = ug->GetNumberOfCells();
    vtkPoints         *inPts  = ug->GetPoints();
    vtkCellData       *inCD   = ug->GetCellData();
    vtkPointData      *inPD   = ug->GetPointData();

    vtkIdType ptSizeGuess = (this->CellList == NULL
                         ? (vtkIdType) pow(float(nCells), 0.6667f) * 5 + 100
                         : CellListSize*5 + 100);

    vtkSurfaceFromVolume sfv(ptSizeGuess);

    vtkUnstructuredGrid *stuff_I_cant_contour = vtkUnstructuredGrid::New();
    stuff_I_cant_contour->SetPoints(ug->GetPoints());
    stuff_I_cant_contour->GetPointData()->ShallowCopy(ug->GetPointData());
    stuff_I_cant_contour->Allocate(nCells);

    vtkDataArray *arr = GetPointScalars(input);
    if (arr == NULL)
        return 0;

    int accessMethod = 0;
    if(arr->HasStandardMemoryLayout())
    {
        if(arr->GetDataType() == VTK_FLOAT)
            accessMethod = 1;
        else if(arr->GetDataType() == VTK_DOUBLE)
            accessMethod = 2;
    }

    int nToProcess = (CellList != NULL ? CellListSize : nCells);
    int numIcantContour = 0;
    for (vtkIdType i = 0 ; i < nToProcess ; i++)
    {
        vtkIdType  cellId = (CellList != NULL ? CellList[i] : i);
        int        cellType = ug->GetCellType(cellId);
        vtkIdType  npts;
        vtkIdType *pts;
        ug->GetCellPoints(cellId, npts, pts);
        const int *triangulation_table = NULL;
        const int *vertices_from_edges = NULL;
        int tt_step = 0;
        bool canContour = false;
        switch (cellType)
        {
          case VTK_TETRA:
            triangulation_table = (const int *) tetTriangulationTable;
            vertices_from_edges = (const int *) tetVerticesFromEdges;
            tt_step = 7;
            canContour = true;
            break;
 
          case VTK_PYRAMID:
            triangulation_table = (const int *) pyramidTriangulationTable;
            vertices_from_edges = (const int *) pyramidVerticesFromEdges;
            tt_step = 13;
            canContour = true;
            break;
 
          case VTK_WEDGE:
            triangulation_table = (const int *) wedgeTriangulationTable;
            vertices_from_edges = (const int *) wedgeVerticesFromEdges;
            tt_step = 13;
            canContour = true;
            break;
 
          case VTK_HEXAHEDRON:
            triangulation_table = (const int *) hexTriangulationTable;
            vertices_from_edges = (const int *) hexVerticesFromEdges;
            tt_step = 16;
            canContour = true;
            break;

          default:
            canContour = false;
            break;
        }
 
        if (canContour)
        {
            int tmp[3] = {0,0,0};
            if(accessMethod == 1)
            {
                vtkUnstructuredCreateTriangles<float, IsoDistanceFunction<float> >(
                    sfv, cellId, pts, npts,
                    triangulation_table, vertices_from_edges, tt_step,
                    IsoDistanceFunction<float>(tmp, arr, (float)this->Isovalue)
                );
            }
            else if(accessMethod == 2)
            {
                vtkUnstructuredCreateTriangles<double, IsoDistanceFunction<double> >(
                    sfv, cellId, pts, npts,
                    triangulation_table, vertices_from_edges, tt_step,
                    IsoDistanceFunction<double>(tmp, arr, this->Isovalue)
                );
            }
            else
            {
                vtkUnstructuredCreateTriangles<double, GeneralIsoDistanceFunction>(
                    sfv, cellId, pts, npts,
                    triangulation_table, vertices_from_edges, tt_step,
                    GeneralIsoDistanceFunction(tmp, arr, this->Isovalue)
                );
            }
        }
        else
        {
            if (numIcantContour == 0)
                stuff_I_cant_contour->GetCellData()->
                                       CopyAllocate(ug->GetCellData(), nCells);

            if(cellType == VTK_POLYHEDRON)
            {
                vtkIdType nFaces, *facePtIds;
                ug->GetFaceStream(cellId, nFaces, facePtIds);
                stuff_I_cant_contour->InsertNextCell(cellType, npts, pts, 
                     nFaces, facePtIds);
            }
            else
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
        sfv.ConstructPolyData(inPD, inCD, just_from_zoo, inPts);

        vtkAppendPolyData *appender = vtkAppendPolyData::New();
        appender->AddInputData(not_from_zoo);
        appender->AddInputData(just_from_zoo);
        appender->Update();

        output->ShallowCopy(appender->GetOutput());
        appender->Delete();
        not_from_zoo->Delete();
        just_from_zoo->Delete();
    }
    else
    {
        sfv.ConstructPolyData(inPD, inCD, output, inPts);
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

    contour->SetInputData(in_ds);
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
