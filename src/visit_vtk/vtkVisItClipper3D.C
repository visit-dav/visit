#include "vtkVisItClipper3D.h"
#include "vtkFloatArray.h"
#include <vtkAppendFilter.h>
#include <vtkCutter.h>
#include <vtkImplicitFunction.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkVolumeFromVolume.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkClipDataSet.h>
#include <vtkDataSetWriter.h>
#include <ImproperUseException.h>

#include <DebugStream.h>

#include <math.h>
#include <vector>

#include <ClipCases.h>
#include <vtkTriangulationTables.h>

vtkCxxRevisionMacro(vtkVisItClipper3D, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkVisItClipper3D);

vtkVisItClipper3D::vtkVisItClipper3D()
{
    CellList = NULL;
    CellListSize = 0;
    insideOut = false;
    clipFunction = NULL;
}

vtkVisItClipper3D::~vtkVisItClipper3D()
{
}

void
vtkVisItClipper3D::SetCellList(int *cl, int size)
{
    CellList = cl;
    CellListSize = size;
}

void
vtkVisItClipper3D::SetClipFunction(vtkImplicitFunction *func)
{
    clipFunction = func;
}

void
vtkVisItClipper3D::SetInsideOut(bool io)
{
    insideOut = io;
}

void
vtkVisItClipper3D::Execute()
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
        debug1 << "vtkVisItClipper3D: Can't operate on this dataset\n";
        GeneralExecute();
    }
}


// ****************************************************************************
//  Method:  vtkVisItClipper3D::StructuredGridExecute
//
//  Purpose:
//    Clips a structured grid.
//
//  Arguments:
//    none
//
//  Note:  most of the setup logic came from Hank's new vtkSlicer.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 11, 2003
//
// ****************************************************************************
void
vtkVisItClipper3D::StructuredGridExecute(void)
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
    vtkUnstructuredGrid *output = (vtkUnstructuredGrid*)GetOutput();

    int ptSizeGuess = (CellList == NULL
                         ? (int) pow(nCells, 0.6667) * 5 + 100
                         : ptSizeGuess = CellListSize*5 + 100);

    vtkVolumeFromVolume vfv(sg->GetNumberOfPoints(), ptSizeGuess);

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
            dist[j] = clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
            if (dist[j] >= 0)
                lookup_case++;
            if (j > 0)
                lookup_case *= 2;
        }

        unsigned char *splitCase = &clipShapesHex[
                                             startClipShapesHex[lookup_case]];
        int            numOutput = numClipShapesHex[lookup_case];
        int            interpIDs[4];
        for (j = 0 ; j < numOutput ; j++)
        {
            unsigned char shapeType = *splitCase++;
            {
                int npts;
                int interpID = -1;
                int color    = -1;
                switch (shapeType)
                {
                  case ST_HEX:
                    npts = 8;
                    color = *splitCase++;
                    break;
                  case ST_WDG:
                    npts = 6;
                    color = *splitCase++;
                    break;
                  case ST_PYR:
                    npts = 5;
                    color = *splitCase++;
                    break;
                  case ST_TET:
                    npts = 4;
                    color = *splitCase++;
                    break;
                  case ST_PNT:
                    interpID = *splitCase++;
                    npts     = *splitCase++;
                    break;
                  default:
                    EXCEPTION1(ImproperUseException,
                               "An invalid output shape was found in "
                               "the ClipCases.");
                }

                if ((!insideOut && color == COLOR0) ||
                    ( insideOut && color == COLOR1))
                {
                    // We don't want this one; it's the wrong side.
                    splitCase += npts;
                    continue;
                }

                int shape[8];
                for (int p = 0 ; p < npts ; p++)
                {
                    unsigned char pt = *splitCase++;
                    if (pt <= P7)
                    {
                        // We know pt P0 must be >P0 since we already
                        // assume P0 == 0.  This is why we do not
                        // bother subtracting P0 from pt here.
                        shape[p] = ((cellI + X_val[pt]) +
                                    (cellJ + Y_val[pt])*ptstrideY +
                                    (cellK + Z_val[pt])*ptstrideZ);
                    }
                    else if (pt >= EA && pt <= EL)
                    {
                        int pt1 = hexVerticesFromEdges[pt-EA][0];
                        int pt2 = hexVerticesFromEdges[pt-EA][1];
                        if (pt2 < pt1)
                        {
                            int tmp = pt2;
                            pt2 = pt1;
                            pt1 = tmp;
                        }
                        float dir = dist[pt2] - dist[pt1];
                        float amt = 0. - dist[pt1];
                        float percent = 1. - (amt / dir);

                        // We may have physically (though not logically)
                        // degenerate cells if percent==0 or percent==1.
                        // We could pretty easily and mostly safely clamp
                        // percent to the range [1e-4, 1. - 1e-4] right here.
                        int ptId1 = ((cellI + X_val[pt1]) +
                                     (cellJ + Y_val[pt1])*ptstrideY +
                                     (cellK + Z_val[pt1])*ptstrideZ);
                        int ptId2 = ((cellI + X_val[pt2]) +
                                     (cellJ + Y_val[pt2])*ptstrideY +
                                     (cellK + Z_val[pt2])*ptstrideZ);
                        shape[p] = vfv.AddPoint(ptId1, ptId2, percent);
                    }
                    else if (pt >= N0 && pt <= N3)
                    {
                        shape[p] = interpIDs[pt - N0];
                    }
                    else
                    {
                        EXCEPTION1(ImproperUseException,
                                   "An invalid output point value "
                                   "was found in the ClipCases.");
                    }
                }

                switch (shapeType)
                {
                  case ST_HEX:
                    vfv.AddHex(cellId,
                               shape[0], shape[1], shape[2], shape[3],
                               shape[4], shape[5], shape[6], shape[7]);
                    break;
                  case ST_WDG:
                    vfv.AddWedge(cellId,
                                 shape[0], shape[1], shape[2],
                                 shape[3], shape[4], shape[5]);
                    break;
                  case ST_PYR:
                    vfv.AddPyramid(cellId, shape[0], shape[1],
                                   shape[2], shape[3], shape[4]);
                    break;
                  case ST_TET:
                    vfv.AddTet(cellId, shape[0], shape[1], shape[2], shape[3]);
                    break;
                  case ST_PNT:
                    interpIDs[interpID] = vfv.AddCentroidPoint(npts, shape);
                    break;
                }
            }

        }
    }

    vfv.ConstructDataSet(inPD, inCD, output, pts_ptr);
}

// ****************************************************************************
//  Method:  vtkVisItClipper3D::RectilinearGridExecute
//
//  Purpose:
//    Clips a rectilinear grid.
//
//  Arguments:
//    none
//
//  Note:  most of the setup logic came from Hank's new vtkSlicer.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 11, 2003
//
// ****************************************************************************
void vtkVisItClipper3D::RectilinearGridExecute(void)
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
    vtkUnstructuredGrid *output = (vtkUnstructuredGrid*)GetOutput();

    int ptSizeGuess = (CellList == NULL
                         ? (int) pow(nCells, 0.6667) * 5 + 100
                         : ptSizeGuess = CellListSize*5 + 100);

    vtkVolumeFromVolume vfv(rg->GetNumberOfPoints(), ptSizeGuess);

    int cell_dims[3];
    cell_dims[0] = pt_dims[0]-1;
    cell_dims[1] = pt_dims[1]-1;
    cell_dims[2] = pt_dims[2]-1;
    int strideY = cell_dims[0];
    int strideZ = cell_dims[0]*cell_dims[1];
    int ptstrideY = pt_dims[0];
    int ptstrideZ = pt_dims[0]*pt_dims[1];
    int X_val[8] = { 0, 1, 1, 0, 0, 1, 1, 0 };
    int Y_val[8] = { 0, 0, 0, 0, 1, 1, 1, 1 };
    int Z_val[8] = { 0, 0, 1, 1, 0, 0, 1, 1 };
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
            dist[j] = clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
            if (dist[j] >= 0)
                lookup_case++;
            if (j > 0)
                lookup_case *= 2;
        }

        unsigned char *splitCase = &clipShapesHex[
                                             startClipShapesHex[lookup_case]];
        int            numOutput = numClipShapesHex[lookup_case];
        int            interpIDs[4];
        for (j = 0 ; j < numOutput ; j++)
        {
            unsigned char shapeType = *splitCase++;
            {
                int npts;
                int interpID = -1;
                int color    = -1;
                switch (shapeType)
                {
                  case ST_HEX:
                    npts = 8;
                    color = *splitCase++;
                    break;
                  case ST_WDG:
                    npts = 6;
                    color = *splitCase++;
                    break;
                  case ST_PYR:
                    npts = 5;
                    color = *splitCase++;
                    break;
                  case ST_TET:
                    npts = 4;
                    color = *splitCase++;
                    break;
                  case ST_PNT:
                    interpID = *splitCase++;
                    npts     = *splitCase++;
                    break;
                  default:
                    EXCEPTION1(ImproperUseException,
                               "An invalid output shape was found in "
                               "the ClipCases.");
                }

                if ((!insideOut && color == COLOR0) ||
                    ( insideOut && color == COLOR1))
                {
                    // We don't want this one; it's the wrong side.
                    splitCase += npts;
                    continue;
                }

                int shape[8];
                for (int p = 0 ; p < npts ; p++)
                {
                    unsigned char pt = *splitCase++;
                    if (pt <= P7)
                    {
                        // We know pt P0 must be >P0 since we already
                        // assume P0 == 0.  This is why we do not
                        // bother subtracting P0 from pt here.
                        shape[p] = ((cellI + X_val[pt]) +
                                    (cellJ + Y_val[pt])*ptstrideY +
                                    (cellK + Z_val[pt])*ptstrideZ);
                    }
                    else if (pt >= EA && pt <= EL)
                    {
                        int pt1 = hexVerticesFromEdges[pt-EA][0];
                        int pt2 = hexVerticesFromEdges[pt-EA][1];
                        if (pt2 < pt1)
                        {
                            int tmp = pt2;
                            pt2 = pt1;
                            pt1 = tmp;
                        }
                        float dir = dist[pt2] - dist[pt1];
                        float amt = 0. - dist[pt1];
                        float percent = 1. - (amt / dir);

                        // We may have physically (though not logically)
                        // degenerate cells if percent==0 or percent==1.
                        // We could pretty easily and mostly safely clamp
                        // percent to the range [1e-4, 1. - 1e-4] right here.
                        int ptId1 = ((cellI + X_val[pt1]) +
                                     (cellJ + Y_val[pt1])*ptstrideY +
                                     (cellK + Z_val[pt1])*ptstrideZ);
                        int ptId2 = ((cellI + X_val[pt2]) +
                                     (cellJ + Y_val[pt2])*ptstrideY +
                                     (cellK + Z_val[pt2])*ptstrideZ);
                        shape[p] = vfv.AddPoint(ptId1, ptId2, percent);
                    }
                    else if (pt >= N0 && pt <= N3)
                    {
                        shape[p] = interpIDs[pt - N0];
                    }
                    else
                    {
                        EXCEPTION1(ImproperUseException,
                                   "An invalid output point value "
                                   "was found in the ClipCases.");
                    }
                }

                switch (shapeType)
                {
                  case ST_HEX:
                    vfv.AddHex(cellId,
                               shape[0], shape[1], shape[2], shape[3],
                               shape[4], shape[5], shape[6], shape[7]);
                    break;
                  case ST_WDG:
                    vfv.AddWedge(cellId,
                                 shape[0], shape[1], shape[2],
                                 shape[3], shape[4], shape[5]);
                    break;
                  case ST_PYR:
                    vfv.AddPyramid(cellId, shape[0], shape[1],
                                   shape[2], shape[3], shape[4]);
                    break;
                  case ST_TET:
                    vfv.AddTet(cellId, shape[0], shape[1], shape[2], shape[3]);
                    break;
                  case ST_PNT:
                    interpIDs[interpID] = vfv.AddCentroidPoint(npts, shape);
                    break;
                }
            }
        }
    }

    vfv.ConstructDataSet(inPD, inCD, output, pt_dims, X, Y, Z);
}

// ****************************************************************************
//  Method:  vtkVisItClipper3D::UnstructuredGridExecute
//
//  Purpose:
//    Clips an unstructured grid.
//
//  Arguments:
//    none
//
//  Note:  most of the setup logic came from Hank's new vtkSlicer.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 11, 2003
//
// ****************************************************************************
void vtkVisItClipper3D::UnstructuredGridExecute(void)
{
    // The routine here is a bit trickier than for the Rectilinear or
    // Structured grids.  We want to clip an unstructured grid -- but that
    // could mean any cell type.  We only have triangulation tables for
    // the finite element zoo.  So the gameplan is to clip any of the
    // elements of the finite element zoo.  If there are more elements left
    // over, clip them using the conventional VTK filters.  Finally,
    // append together the clips from the zoo with the clips from the
    // non-zoo elements.  If all the elements are from the zoo, then just
    // clip them with no appending.

    int   i, j;

    vtkUnstructuredGrid *ug = (vtkUnstructuredGrid *) GetInput();

    int                nCells = ug->GetNumberOfCells();
    vtkPoints         *inPts  = ug->GetPoints();
    vtkCellData       *inCD   = ug->GetCellData();
    vtkPointData      *inPD   = ug->GetPointData();
    vtkUnstructuredGrid *output = (vtkUnstructuredGrid*)GetOutput();

    int ptSizeGuess = (CellList == NULL
                         ? (int) pow(nCells, 0.6667) * 5 + 100
                         : ptSizeGuess = CellListSize*5 + 100);

    vtkVolumeFromVolume vfv(ug->GetNumberOfPoints(), ptSizeGuess);

    vtkUnstructuredGrid *stuff_I_cant_clip = vtkUnstructuredGrid::New();
    stuff_I_cant_clip->SetPoints(ug->GetPoints());
    stuff_I_cant_clip->GetPointData()->ShallowCopy(ug->GetPointData());
    stuff_I_cant_clip->Allocate(nCells);

    float *pts_ptr = (float *) inPts->GetVoidPointer(0);

    int nToProcess = (CellList != NULL ? CellListSize : nCells);
    int numIcantClip = 0;
    for (i = 0 ; i < nToProcess ; i++)
    {
        int        cellId = (CellList != NULL ? CellList[i] : i);
        int        cellType = ug->GetCellType(cellId);
        int        npts;
        vtkIdType *pts;
        ug->GetCellPoints(cellId, npts, pts);
        bool canClip = false;
        switch (cellType)
        {
          case VTK_TETRA:
          case VTK_PYRAMID:
          case VTK_WEDGE:
          case VTK_HEXAHEDRON:
            canClip = true;
            break;

          default:
            canClip = false;
            break;
        }
 
        if (canClip)
        {
            const int max_pts = 8;
            float dist[max_pts];
            int lookup_case = 0;
            for (j = npts-1 ; j >= 0 ; j--)
            {
                float *pt = pts_ptr + 3*pts[j];
                dist[j] = clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
                if (dist[j] >= 0)
                    lookup_case++;
                if (j > 0)
                    lookup_case *= 2;
            }

            int             startIndex = 0;
            unsigned char  *splitCase = NULL;
            int             numOutput = 0;
            typedef int     edgeIndices[2];
            edgeIndices    *vertices_from_edges = NULL;

            switch (cellType)
            {
              case VTK_TETRA:
                startIndex = startClipShapesTet[lookup_case];
                splitCase  = &clipShapesTet[startIndex];
                numOutput  = numClipShapesTet[lookup_case];
                vertices_from_edges = tetVerticesFromEdges;
                break;
              case VTK_PYRAMID:
                startIndex = startClipShapesPyr[lookup_case];
                splitCase  = &clipShapesPyr[startIndex];
                numOutput  = numClipShapesPyr[lookup_case];
                vertices_from_edges = pyramidVerticesFromEdges;
                break;
              case VTK_WEDGE:
                startIndex = startClipShapesWdg[lookup_case];
                splitCase  = &clipShapesWdg[startIndex];
                numOutput  = numClipShapesWdg[lookup_case];
                vertices_from_edges = wedgeVerticesFromEdges;
                break;
              case VTK_HEXAHEDRON:
                startIndex = startClipShapesHex[lookup_case];
                splitCase  = &clipShapesHex[startIndex];
                numOutput  = numClipShapesHex[lookup_case];
                vertices_from_edges = hexVerticesFromEdges;
                break;
            }

            int            interpIDs[4];
            for (j = 0 ; j < numOutput ; j++)
            {
                unsigned char shapeType = *splitCase++;
                {
                    int npts;
                    int interpID = -1;
                    int color    = -1;
                    switch (shapeType)
                    {
                      case ST_HEX:
                        npts = 8;
                        color = *splitCase++;
                        break;
                      case ST_WDG:
                        npts = 6;
                        color = *splitCase++;
                        break;
                      case ST_PYR:
                        npts = 5;
                        color = *splitCase++;
                        break;
                      case ST_TET:
                        npts = 4;
                        color = *splitCase++;
                        break;
                      case ST_PNT:
                        interpID = *splitCase++;
                        npts     = *splitCase++;
                        break;
                      default:
                        EXCEPTION1(ImproperUseException,
                                   "An invalid output shape was found in "
                                   "the ClipCases.");
                    }

                    if ((!insideOut && color == COLOR0) ||
                        ( insideOut && color == COLOR1))
                    {
                        // We don't want this one; it's the wrong side.
                        splitCase += npts;
                        continue;
                    }

                    int shape[8];
                    for (int p = 0 ; p < npts ; p++)
                    {
                        unsigned char pt = *splitCase++;
                        if (pt <= P7)
                        {
                            // We know pt P0 must be >P0 since we already
                            // assume P0 == 0.  This is why we do not
                            // bother subtracting P0 from pt here.
                            shape[p] = pts[pt];
                        }
                        else if (pt >= EA && pt <= EL)
                        {
                            int pt1 = vertices_from_edges[pt-EA][0];
                            int pt2 = vertices_from_edges[pt-EA][1];
                            if (pt2 < pt1)
                            {
                                int tmp = pt2;
                                pt2 = pt1;
                                pt1 = tmp;
                            }
                            float dir = dist[pt2] - dist[pt1];
                            float amt = 0. - dist[pt1];
                            float percent = 1. - (amt / dir);

                            // We may have physically (though not logically)
                            // degenerate cells if percent==0 or percent==1.
                            // We could pretty easily and mostly safely clamp
                            // percent to the range [1e-4, 1. - 1e-4] here.
                            int ptId1 = pts[pt1];
                            int ptId2 = pts[pt2];
                            shape[p] = vfv.AddPoint(ptId1, ptId2, percent);
                        }
                        else if (pt >= N0 && pt <= N3)
                        {
                            shape[p] = interpIDs[pt - N0];
                        }
                        else
                        {
                            EXCEPTION1(ImproperUseException,
                                       "An invalid output point value "
                                       "was found in the ClipCases.");
                        }
                    }

                    switch (shapeType)
                    {
                      case ST_HEX:
                        vfv.AddHex(cellId,
                                   shape[0], shape[1], shape[2], shape[3],
                                   shape[4], shape[5], shape[6], shape[7]);
                        break;
                      case ST_WDG:
                        vfv.AddWedge(cellId,
                                     shape[0], shape[1], shape[2],
                                     shape[3], shape[4], shape[5]);
                        break;
                      case ST_PYR:
                        vfv.AddPyramid(cellId, shape[0], shape[1],
                                       shape[2], shape[3], shape[4]);
                        break;
                      case ST_TET:
                        vfv.AddTet(cellId, shape[0], shape[1], shape[2], shape[3]);
                        break;
                      case ST_PNT:
                        interpIDs[interpID] = vfv.AddCentroidPoint(npts, shape);
                        break;
                    }
                }
            }
        }
        else
        {
            if (numIcantClip == 0)
                stuff_I_cant_clip->GetCellData()->
                                       CopyAllocate(ug->GetCellData(), nCells);

            stuff_I_cant_clip->InsertNextCell(cellType, npts, pts);
            stuff_I_cant_clip->GetCellData()->
                            CopyData(ug->GetCellData(), cellId, numIcantClip);
            numIcantClip++;
        }
    }

    if (numIcantClip > 0)
    {
        vtkUnstructuredGrid *not_from_zoo  = vtkUnstructuredGrid::New();
        ClipDataset(stuff_I_cant_clip, not_from_zoo);
        
        vtkUnstructuredGrid *just_from_zoo = vtkUnstructuredGrid::New();
        vfv.ConstructDataSet(inPD, inCD, just_from_zoo, pts_ptr);

        vtkAppendFilter *appender = vtkAppendFilter::New();
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
        vfv.ConstructDataSet(inPD, inCD, output, pts_ptr);
    }

    stuff_I_cant_clip->Delete();
}

void vtkVisItClipper3D::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os,indent);
}

void vtkVisItClipper3D::GeneralExecute(void)
{
    ClipDataset(GetInput(), (vtkUnstructuredGrid*)GetOutput());
}

void vtkVisItClipper3D::ClipDataset(vtkDataSet *in_ds,
                                    vtkUnstructuredGrid *out_ds)
{
    vtkClipDataSet *clipData = vtkClipDataSet::New();
    clipData->SetInput(in_ds);
    clipData->SetOutput(out_ds);
    clipData->SetClipFunction(clipFunction);
    clipData->GenerateClipScalarsOff();
    clipData->SetInsideOut(insideOut);
    clipData->Update();
    clipData->Delete();
    clipData = NULL;
}

