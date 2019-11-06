// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

#include <math.h>
#include <vector>

#include <vtkAppendPolyData.h>
#include <vtkCellData.h>
#include <vtkCutter.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkSurfaceFromVolume.h>
#include <vtkTriangulationTables.h>
#include <vtkUnstructuredGrid.h>

#include <vtkCreateTriangleHelpers.h>
#include <vtkVisItCutter.h>

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

void
vtkSlicer::SetCellList(vtkIdType *cl, vtkIdType size)
{
    this->CellList = cl;
    this->CellListSize = size;
}

// ****************************************************************************
//  Method: vtkSlicer::RequestData.
//
//  Modifications:
//    Eric Brugger, Thu Jan 10 10:24:20 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************

int
vtkSlicer::RequestData(
    vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
    vtkDebugMacro(<<"Executing vtkSlicer");

    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    //
    // Initialize some frequently used values.
    //
    input  = vtkDataSet::SafeDownCast(
        inInfo->Get(vtkDataObject::DATA_OBJECT()));
    output = vtkPolyData::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));

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

    return 1;
}

// ****************************************************************************
//  Method: vtkSlicer::FillInputPortInformation
//
// ****************************************************************************

int
vtkSlicer::FillInputPortInformation(int, vtkInformation *info)
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    return 1;
}

// ****************************************************************************
//  Class: SliceFunction
//
//  Purpose:
//    Slice functor that accesses points directly as array.
//
//  Notes:      
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Mar 13 10:53:27 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
class SliceFunction
{
public:
    SliceFunction(const int *pt_dims, vtkPoints *pts, const double O[3], const double N[3])
    {
        pts_ptr = (const T *)pts->GetVoidPointer(0);
        ptstrideY = (vtkIdType)pt_dims[0];
        ptstrideZ = (vtkIdType)(pt_dims[0] * pt_dims[1]);
        Origin[0] = O[0]; Origin[1] = O[1]; Origin[2] = O[2];
        Normal[0] = N[0]; Normal[1] = N[1]; Normal[2] = N[2];
        D = Origin[0]*Normal[0] + Origin[1]*Normal[1] + Origin[2]*Normal[2];
    }

    inline T operator()(vtkIdType cellI,   vtkIdType cellJ,   vtkIdType cellK,
                        vtkIdType iOffset, vtkIdType jOffset, vtkIdType kOffset) const
    {
        vtkIdType ptId = (cellI + iOffset) + (cellJ + jOffset) * ptstrideY +
                         (cellK + kOffset) * ptstrideZ;
        const T *pt = pts_ptr + 3 * ptId;
        return pt[0]*Normal[0] + pt[1]*Normal[1] + pt[2]*Normal[2] - D;
    }

    inline T operator()(vtkIdType ptId) const
    {
        const T *pt = pts_ptr + 3 * ptId;
        return pt[0]*Normal[0] + pt[1]*Normal[1] + pt[2]*Normal[2] - D;
    }
private:
    vtkIdType ptstrideY, ptstrideZ; 
    const T *pts_ptr;
    double   Normal[3];
    double   Origin[3];
    double   D;
};

// ****************************************************************************
//  Class: GeneralSliceFunction
//
//  Purpose:
//    Slice functor that uses GetPoint to access points.
//
//  Notes:      
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Mar 13 10:53:27 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

class GeneralSliceFunction
{
public:
    GeneralSliceFunction(const int *pt_dims, vtkPoints *p, const double O[3], const double N[3])
    {
        pts = p;
        ptstrideY = (vtkIdType)pt_dims[0];
        ptstrideZ = (vtkIdType)(pt_dims[0] * pt_dims[1]);
        Origin[0] = O[0]; Origin[1] = O[1]; Origin[2] = O[2];
        Normal[0] = N[0]; Normal[1] = N[1]; Normal[2] = N[2];
        D = Origin[0]*Normal[0] + Origin[1]*Normal[1] + Origin[2]*Normal[2];
    }

    inline double operator()(vtkIdType cellI,   vtkIdType cellJ,   vtkIdType cellK,
                             vtkIdType iOffset, vtkIdType jOffset, vtkIdType kOffset) const
    {
        vtkIdType ptId = (cellI + iOffset) + (cellJ + jOffset) * ptstrideY +
                         (cellK + kOffset) * ptstrideZ;
        const double *pt = pts->GetPoint(ptId);
        return pt[0]*Normal[0] + pt[1]*Normal[1] + pt[2]*Normal[2] - D;
    }

    inline double operator()(vtkIdType ptId) const
    {
        const double *pt = pts->GetPoint(ptId);
        return pt[0]*Normal[0] + pt[1]*Normal[1] + pt[2]*Normal[2] - D;
    }
private:
    vtkIdType  ptstrideY, ptstrideZ; 
    vtkPoints *pts;
    double     Normal[3];
    double     Origin[3];
    double     D;
};

// ****************************************************************************
//  Method: vtkSlicer::StructuredGridExecute
//
//  Modifications:
//    Brad Whitlock, Thu Aug 12 14:51:27 PST 2004
//    Added float casts to the pow() arguments so it builds on MSVC7.Net.
//
//    Hank Childs, Sat Jan 27 12:45:03 PST 2007
//    Add check for 1xJxK and Ix1xK meshes (instead of crashing).
//
//    Brad Whitlock, Tue Mar 13 10:52:52 PDT 2012
//    I moved the implementation into vtkStructuredCreateTriangles and I added
//    different implementations.
//
//    Eric Brugger, Thu Jan 10 10:24:20 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
//    Brad Whitlock, Thu Jul 23 16:01:46 PDT 2015
//    Support for non-standard memory layout.
//
// ****************************************************************************

void
vtkSlicer::StructuredGridExecute(void)
{
    vtkStructuredGrid *sg = (vtkStructuredGrid *)input;
    int pt_dims[3];
    sg->GetDimensions(pt_dims);
    if (pt_dims[0] <= 1 || pt_dims[1] <= 1 || pt_dims[2] <= 1)
    {
        GeneralExecute();
        return;
    }

    vtkIdType          nCells = sg->GetNumberOfCells();
    vtkPoints         *inPts  = sg->GetPoints();
    vtkCellData       *inCD   = sg->GetCellData();
    vtkPointData      *inPD   = sg->GetPointData();

    vtkIdType ptSizeGuess = (this->CellList == NULL
                         ? (int) pow(float(nCells), 0.6667f) * 5 + 100
                         : CellListSize*5 + 100);

    vtkSurfaceFromVolume sfv(ptSizeGuess);

    int accessMethod = 0;
    if(inPts->GetData()->HasStandardMemoryLayout())
    {
        if(inPts->GetDataType() == VTK_FLOAT)
            accessMethod = 1;
        else if(inPts->GetDataType() == VTK_DOUBLE)
            accessMethod = 2;
    }

    if(accessMethod == 1)
    {
        vtkStructuredCreateTriangles<float, SliceFunction<float> >(
            sfv, this->CellList, this->CellListSize, nCells,
            pt_dims, SliceFunction<float>(pt_dims, inPts, this->Origin, this->Normal)
        );
    }
    else if(accessMethod == 2)
    {
        vtkStructuredCreateTriangles<double, SliceFunction<double> >(
            sfv, this->CellList, this->CellListSize, nCells,
            pt_dims, SliceFunction<double>(pt_dims, inPts, this->Origin, this->Normal)
        );
    }
    else
    {
        vtkStructuredCreateTriangles<double, GeneralSliceFunction >(
            sfv, this->CellList, this->CellListSize, nCells,
            pt_dims, GeneralSliceFunction(pt_dims, inPts, this->Origin, this->Normal)
        );
    }

    sfv.ConstructPolyData(inPD, inCD, output, inPts);
}

// ****************************************************************************
//  Class: RectSliceFunction
//
//  Purpose:
//    Rectilinear slice function that accesses the coordinate arrays directly.
//
//  Notes:      
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Mar 13 10:50:45 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

template <typename T>
class RectSliceFunction
{
public:
    RectSliceFunction(vtkDataArray *Xc, vtkDataArray *Yc, 
        vtkDataArray *Zc, const double O[3], const double N[3])
    {
        X = NULL;
        if(Xc != NULL)
            X = (const T *)Xc->GetVoidPointer(0);
        Y = NULL;
        if(Yc != NULL)
            Y = (const T *)Yc->GetVoidPointer(0);
        Z = NULL;
        if(Zc != NULL)
            Z = (const T *)Zc->GetVoidPointer(0);

        Origin[0] = O[0]; Origin[1] = O[1]; Origin[2] = O[2];
        Normal[0] = N[0]; Normal[1] = N[1]; Normal[2] = N[2];
        D = Origin[0]*Normal[0] + Origin[1]*Normal[1] + Origin[2]*Normal[2];
    }

    inline T operator()(vtkIdType cellI,   vtkIdType cellJ,   vtkIdType cellK,
                        vtkIdType iOffset, vtkIdType jOffset, vtkIdType kOffset) const
    {
        T pt[3];
        pt[0] = X[cellI + iOffset];
        pt[1] = Y[cellJ + jOffset];
        pt[2] = Z[cellK + kOffset];
        return pt[0]*Normal[0] + pt[1]*Normal[1] + pt[2]*Normal[2] - D;
    }
private:
    const T *X;
    const T *Y;
    const T *Z;
    double   Normal[3];
    double   Origin[3];
    double   D;
};

// ****************************************************************************
//  Class: GeneralRectSliceFunction
//
//  Purpose:
//    Rectilinear slice function that uses GetTuple to access the coordinate
//     arrays.
//
//  Notes:      
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Mar 13 10:50:19 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

class GeneralRectSliceFunction
{
public:
    GeneralRectSliceFunction(vtkDataArray *Xc, vtkDataArray *Yc, 
        vtkDataArray *Zc, const double O[3], const double N[3])
    {
        X = Xc;
        Y = Yc;
        Z = Zc;
        Origin[0] = O[0]; Origin[1] = O[1]; Origin[2] = O[2];
        Normal[0] = N[0]; Normal[1] = N[1]; Normal[2] = N[2];
        D = Origin[0]*Normal[0] + Origin[1]*Normal[1] + Origin[2]*Normal[2];
    }

    inline double operator()(vtkIdType cellI,   vtkIdType cellJ,   vtkIdType cellK,
                             vtkIdType iOffset, vtkIdType jOffset, vtkIdType kOffset) const
    {
        double pt[3];
        pt[0] = X->GetTuple1(cellI + iOffset);
        pt[1] = Y->GetTuple1(cellJ + jOffset);
        pt[2] = Z->GetTuple1(cellK + kOffset);
        return pt[0]*Normal[0] + pt[1]*Normal[1] + pt[2]*Normal[2] - D;
    }
private:
    vtkDataArray *X;
    vtkDataArray *Y;
    vtkDataArray *Z;
    double        Normal[3];
    double        Origin[3];
    double        D;
};

// ****************************************************************************
//  Method: vtkSlicer::RectilinearGridExecute
//
//  Modifications:
//    Brad Whitlock, Thu Aug 12 14:51:27 PST 2004
//    Added float casts to the pow() arguments so it builds on MSVC7.Net.
//
//    Hank Childs, Sat Jan 27 12:45:03 PST 2007
//    Add check for 1xJxK and Ix1xK meshes (instead of crashing).
//
//    Brad Whitlock, Tue Mar 13 10:48:45 PDT 2012
//    Moved the implementation into vtkStructuredCreateTriangles and added
//    different implementations for float/double and a general method that
//    uses GetTuple to get the values.
//
//    Eric Brugger, Thu Jan 10 10:24:20 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
//    Brad Whitlock, Thu Jul 23 16:01:46 PDT 2015
//    Support for non-standard memory layout.
//
// ****************************************************************************

void
vtkSlicer::RectilinearGridExecute(void)
{
    vtkRectilinearGrid *rg = (vtkRectilinearGrid *)input;
    int pt_dims[3];
    rg->GetDimensions(pt_dims);
    if (pt_dims[0] <= 1 || pt_dims[1] <= 1 || pt_dims[2] <= 1)
    {
        GeneralExecute();
        return;
    }

    vtkIdType     nCells = rg->GetNumberOfCells();
    vtkCellData  *inCD   = rg->GetCellData();
    vtkPointData *inPD   = rg->GetPointData();

    vtkIdType ptSizeGuess = (this->CellList == NULL
                         ? (int) pow(float(nCells), 0.6667f) * 5 + 100
                         : CellListSize*5 + 100);

    vtkSurfaceFromVolume sfv(ptSizeGuess);

    int tx = rg->GetXCoordinates()->GetDataType();
    int ty = rg->GetYCoordinates()->GetDataType();
    int tz = rg->GetZCoordinates()->GetDataType();
    bool smlx = rg->GetXCoordinates()->HasStandardMemoryLayout();
    bool smly = rg->GetYCoordinates()->HasStandardMemoryLayout();
    bool smlz = rg->GetZCoordinates()->HasStandardMemoryLayout();
    bool sameTypes = tx == ty && ty == tz;
    bool sameML = smlx == smly && smly == smlz;
    bool same = sameTypes && sameML;

    if(same && tx == VTK_FLOAT)
    {
        vtkStructuredCreateTriangles<float, RectSliceFunction<float> >(
            sfv, this->CellList, this->CellListSize, nCells, pt_dims,
            RectSliceFunction<float>(
                rg->GetXCoordinates(), rg->GetYCoordinates(), rg->GetZCoordinates(),
                this->Origin, this->Normal)
        );
    }
    else if(same && tx == VTK_DOUBLE)
    {
        vtkStructuredCreateTriangles<double, RectSliceFunction<double> >(
            sfv, this->CellList, this->CellListSize, nCells, pt_dims,
            RectSliceFunction<double>(
                rg->GetXCoordinates(), rg->GetYCoordinates(), rg->GetZCoordinates(),
                this->Origin, this->Normal)
        );
    }
    else
    {
        vtkStructuredCreateTriangles<double, GeneralRectSliceFunction >(
            sfv, this->CellList, this->CellListSize, nCells, pt_dims,
            GeneralRectSliceFunction(
                rg->GetXCoordinates(), rg->GetYCoordinates(), rg->GetZCoordinates(),
                this->Origin, this->Normal)
        );
    }

    sfv.ConstructPolyData(inPD, inCD, output, pt_dims, 
        rg->GetXCoordinates(), rg->GetYCoordinates(), rg->GetZCoordinates());
}

// ****************************************************************************
//  Method: vtkSlicer::UnstructuredGridExecute
//
//  Modifications:
//    Hank Childs, Tue Mar 30 07:07:42 PST 2004
//    Add support for slicing vertices.
//
//    Brad Whitlock, Thu Aug 12 14:51:27 PST 2004
//    Added float casts to the pow() arguments so it builds on MSVC7.Net.
//
//    Brad Whitlock, Tue Mar 13 16:55:36 PDT 2012
//    I moved code to generate triangles into vtkUnstructuredCreateTriangles.
//
//    Brad Whitlock, Wed Apr 11 11:37:18 PDT 2012
//    When we can't slice a cell, insert faces too for polyhedral cells.
//
//    Eric Brugger, Thu Jan 10 10:24:20 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
//    Kathleen Biagas, Fri Jan 25 16:04:46 PST 2013
//    Call Update on the filter, not the data object.
//
// ****************************************************************************

void
vtkSlicer::UnstructuredGridExecute(void)
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

    vtkUnstructuredGrid *ug = (vtkUnstructuredGrid *)input;

    vtkIdType          nCells = ug->GetNumberOfCells();
    vtkPoints         *inPts  = ug->GetPoints();
    vtkCellData       *inCD   = ug->GetCellData();
    vtkPointData      *inPD   = ug->GetPointData();

    vtkIdType ptSizeGuess = (this->CellList == NULL
                         ? (int) pow(float(nCells), 0.6667f) * 5 + 100
                         : CellListSize*5 + 100);

    vtkSurfaceFromVolume sfv(ptSizeGuess);

    vtkUnstructuredGrid *stuff_I_cant_slice = vtkUnstructuredGrid::New();
    vtkPolyData *vertices_on_slice = vtkPolyData::New();

    double D = Origin[0]*Normal[0] + Origin[1]*Normal[1] + Origin[2]*Normal[2];

    int accessMethod = 0;
    if(inPts->GetData()->HasStandardMemoryLayout())
    {
        if(inPts->GetData()->GetDataType() == VTK_FLOAT)
            accessMethod = 1;
        else if(inPts->GetData()->GetDataType() == VTK_DOUBLE)
            accessMethod = 2;
    }

    vtkIdType nToProcess = (CellList != NULL ? CellListSize : nCells);
    vtkIdType numIcantSlice = 0;
    vtkIdType numVertices = 0;
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
        bool canSlice = false;
        bool isVertex = false;
        switch (cellType)
        {
          case VTK_TETRA:
            triangulation_table = (const int *) tetTriangulationTable;
            vertices_from_edges = (const int *) tetVerticesFromEdges;
            tt_step = 7;
            canSlice = true;
            break;
 
          case VTK_PYRAMID:
            triangulation_table = (const int *) pyramidTriangulationTable;
            vertices_from_edges = (const int *) pyramidVerticesFromEdges;
            tt_step = 13;
            canSlice = true;
            break;
 
          case VTK_WEDGE:
            triangulation_table = (const int *) wedgeTriangulationTable;
            vertices_from_edges = (const int *) wedgeVerticesFromEdges;
            tt_step = 13;
            canSlice = true;
            break;
 
          case VTK_HEXAHEDRON:
            triangulation_table = (const int *) hexTriangulationTable;
            vertices_from_edges = (const int *) hexVerticesFromEdges;
            tt_step = 16;
            canSlice = true;
            break;
 
          case VTK_VERTEX:
            isVertex = true;
            break;

          default:
            canSlice = false;
            break;
        }
 
        if (canSlice)
        {
            int tmp[3] = {0,0,0};
            if(accessMethod == 1)
            {
                vtkUnstructuredCreateTriangles<float, SliceFunction<float> >(
                    sfv, cellId, pts, npts,
                    triangulation_table, vertices_from_edges, tt_step,
                    SliceFunction<float>(tmp, inPts, this->Origin, this->Normal)
                );
            }
            else if(accessMethod == 2)
            {
                vtkUnstructuredCreateTriangles<double, SliceFunction<double> >(
                    sfv, cellId, pts, npts,
                    triangulation_table, vertices_from_edges, tt_step,
                    SliceFunction<double>(tmp, inPts, this->Origin, this->Normal)
                );
            }
            else
            {
                vtkUnstructuredCreateTriangles<double, GeneralSliceFunction >(
                    sfv, cellId, pts, npts,
                    triangulation_table, vertices_from_edges, tt_step,
                    GeneralSliceFunction(tmp, inPts, this->Origin, this->Normal)
                );
            }
        }
        else if (isVertex)
        {
            //
            // Determine if the vertex is even on the plane.
            //
            const double *pt = inPts->GetPoint(pts[0]);
            double dist_from_plane = Normal[0]*pt[0] + Normal[1]*pt[1]
                                   + Normal[2]*pt[2] - D;
            if (fabs(dist_from_plane) < 1e-12)
            {
                if (numVertices == 0)
                {
                    vertices_on_slice->SetPoints(ug->GetPoints());
                    vertices_on_slice->GetPointData()->ShallowCopy(
                                                           ug->GetPointData());
                    vertices_on_slice->Allocate(nCells);
                    vertices_on_slice->GetCellData()->
                                       CopyAllocate(ug->GetCellData(), nCells);
                }
                vertices_on_slice->InsertNextCell(VTK_VERTEX, 1, pts);
                vertices_on_slice->GetCellData()->
                              CopyData(ug->GetCellData(), cellId, numVertices);
                numVertices++;
            }
        }
        else
        {
            if (numIcantSlice == 0)
            {
                stuff_I_cant_slice->SetPoints(ug->GetPoints());
                stuff_I_cant_slice->GetPointData()->ShallowCopy(
                                                           ug->GetPointData());
                stuff_I_cant_slice->Allocate(nCells);
                stuff_I_cant_slice->GetCellData()->
                                       CopyAllocate(ug->GetCellData(), nCells);
            }

            if(cellType == VTK_POLYHEDRON)
            {
                vtkIdType nFaces, *facePtIds;
                ug->GetFaceStream(cellId, nFaces, facePtIds);
                stuff_I_cant_slice->InsertNextCell(cellType, npts, pts, 
                    nFaces, facePtIds);
            }
            else
                stuff_I_cant_slice->InsertNextCell(cellType, npts, pts);
            stuff_I_cant_slice->GetCellData()->
                            CopyData(ug->GetCellData(), cellId, numIcantSlice);
            numIcantSlice++;
        }
    }

    if ((numIcantSlice > 0) || (numVertices > 0))
    {
        vtkAppendPolyData *appender = vtkAppendPolyData::New();

        if (numIcantSlice > 0)
        {
            vtkPolyData *not_from_zoo  = vtkPolyData::New();
            SliceDataset(stuff_I_cant_slice, not_from_zoo, true);
            appender->AddInputData(not_from_zoo);
            not_from_zoo->Delete();
        }

        if (numVertices > 0)
        {
            appender->AddInputData(vertices_on_slice);
        }

        vtkPolyData *just_from_zoo = vtkPolyData::New();
        sfv.ConstructPolyData(inPD, inCD, just_from_zoo, inPts);
        appender->AddInputData(just_from_zoo);
        just_from_zoo->Delete();

        appender->Update();

        output->ShallowCopy(appender->GetOutput());
        appender->Delete();
    }
    else
    {
        sfv.ConstructPolyData(inPD, inCD, output, inPts);
    }

    stuff_I_cant_slice->Delete();
    vertices_on_slice->Delete();
}

// ****************************************************************************
//  Modifications:
//    Eric Brugger, Thu Jan 10 10:24:20 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************

void
vtkSlicer::GeneralExecute(void)
{
    SliceDataset(input, output, false);
}

// ****************************************************************************
//  Modifications:
//    Kathleen Bonnell, Wed Apr 27 18:47:18 PDT 2005
//    Use vtkVisItCutter, which has modifications to correctly handle CellData. 
//
//    Brad Whitlock, Wed Apr 11 11:34:16 PDT 2012
//    Use vtkCutter when we're taking this path so we can slice general VTK
//    datasets containing cells that vtkVisItCutter can't slice.
//
// ****************************************************************************

void
vtkSlicer::SliceDataset(vtkDataSet *in_ds, vtkPolyData *out_pd,
    bool useVTKFilter)
{
    vtkPlane  *plane  = vtkPlane::New();
    plane->SetOrigin(Origin[0], Origin[1], Origin[2]);
    plane->SetNormal(Normal[0], Normal[1], Normal[2]);

    if(useVTKFilter)
    {
        vtkCutter *cutter = vtkCutter::New();
        cutter->SetCutFunction(plane);
        cutter->SetInputData(in_ds);
        cutter->Update();

        out_pd->ShallowCopy(cutter->GetOutput());
        cutter->Delete();
    }
    else
    {
        vtkVisItCutter *cutter = vtkVisItCutter::New();
        cutter->SetCutFunction(plane);
        cutter->SetInputData(in_ds);
        cutter->Update();

        out_pd->ShallowCopy(cutter->GetOutput());
        cutter->Delete();
    }

    plane->Delete();
}

// ****************************************************************************
//  Method: vtkSlicer::PrintSelf
//
// ****************************************************************************

void
vtkSlicer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Normal: " << Normal[0] << ", " << Normal[1] << ", "
     << Normal[2] << "\n";
  os << indent << "Origin: " << Origin[0] << ", " << Origin[1] << ", "
     << Origin[2] << "\n";
}
