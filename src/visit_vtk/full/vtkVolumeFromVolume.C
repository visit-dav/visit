// ************************************************************************* //
//                           vtkVolumeFromVolume.C                          //
// ************************************************************************* //

#include <vtkVolumeFromVolume.h>

#include <vtkIdList.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>

using std::vector;

vtkVolumeFromVolume::CentroidPointList::CentroidPointList()
{
    listSize = 4096;
    pointsPerList = 1024;
 
    list = new CentroidPointEntry*[listSize];
    list[0] = new CentroidPointEntry[pointsPerList];
    for (int i = 1 ; i < listSize ; i++)
        list[i] = NULL;
 
    currentList = 0;
    currentPoint = 0;
}
 
 
vtkVolumeFromVolume::CentroidPointList::~CentroidPointList()
{
    for (int i = 0 ; i < listSize ; i++)
    {
        if (list[i] != NULL)
            delete [] list[i];
        else
            break;
    }
    delete [] list;
}


int
vtkVolumeFromVolume::CentroidPointList::GetList(int listId,
                                    const CentroidPointEntry *&outlist) const
{
    if (listId < 0 || listId > currentList)
    {
        outlist = NULL;
        return 0;
    }
 
    outlist = list[listId];
    return (listId == currentList ? currentPoint : pointsPerList);
}
 
 
int
vtkVolumeFromVolume::CentroidPointList::GetNumberOfLists(void) const
{
    return currentList+1;
}
 
 
int
vtkVolumeFromVolume::CentroidPointList::GetTotalNumberOfPoints(void) const
{
    int numFullLists = currentList;  // actually currentList-1+1
    int numExtra = currentPoint;  // again, currentPoint-1+1
 
    return numFullLists*pointsPerList + numExtra;
}


int
vtkVolumeFromVolume::CentroidPointList::AddPoint(int npts, int *pts)
{
    if (currentPoint >= pointsPerList)
    {
        if (currentList >= listSize+1)
        {
            CentroidPointEntry **tmpList = new CentroidPointEntry*[2*listSize];
            for (int i = 0 ; i < listSize ; i++)
            {
                tmpList[i] = list[i];
            }
            listSize *= 2;
            delete [] list;
            list = tmpList;
        }
 
        currentList++;
        list[currentList] = new CentroidPointEntry[pointsPerList];
        currentPoint = 0;
    }
 
    list[currentList][currentPoint].nPts = npts;
    for (int i = 0 ; i < npts ; i++)
    {
        list[currentList][currentPoint].ptIds[i] = pts[i];
    }
    currentPoint++;
 
    return (GetTotalNumberOfPoints()-1);
}


vtkVolumeFromVolume::ShapeList::ShapeList(int size)
{
    shapeSize = size;
    listSize = 4096;
    shapesPerList = 1024;
 
    list = new int*[listSize];
    list[0] = new int[(shapeSize+1)*shapesPerList];
    for (int i = 1 ; i < listSize ; i++)
        list[i] = NULL;
 
    currentList = 0;
    currentShape = 0;
}
 
 
vtkVolumeFromVolume::ShapeList::~ShapeList()
{
    for (int i = 0 ; i < listSize ; i++)
    {
        if (list[i] != NULL)
            delete [] list[i];
        else
            break;
    }
    delete [] list;
}
 
 
int
vtkVolumeFromVolume::ShapeList::GetList(int listId, const int *&outlist)
    const
{
    if (listId < 0 || listId > currentList)
    {
        outlist = NULL;
        return 0;
    }
 
    outlist = list[listId];
    return (listId == currentList ? currentShape : shapesPerList);
}
 
 
int
vtkVolumeFromVolume::ShapeList::GetNumberOfLists(void) const
{
    return currentList+1;
}


int
vtkVolumeFromVolume::ShapeList::GetTotalNumberOfShapes(void) const
{
    int numFullLists = currentList;  // actually currentList-1+1
    int numExtra = currentShape;  // again, currentShape-1+1
 
    return numFullLists*shapesPerList + numExtra;
}

vtkVolumeFromVolume::HexList::HexList()
    : vtkVolumeFromVolume::ShapeList(8)
{
}
 

vtkVolumeFromVolume::HexList::~HexList()
{
}
 
void
vtkVolumeFromVolume::HexList::AddHex(int cellId,
                                     int v1, int v2, int v3, int v4,
                                     int v5, int v6, int v7, int v8)
{
    if (currentShape >= shapesPerList)
    {
        if (currentList >= listSize+1)
        {
            int **tmpList = new int*[2*listSize];
            for (int i = 0 ; i < listSize ; i++)
            {
                tmpList[i] = list[i];
            }
            listSize *= 2;
            delete [] list;
            list = tmpList;
        }
 
        currentList++;
        list[currentList] = new int[(shapeSize+1)*shapesPerList];
        currentShape = 0;
    }
 
    int idx = (shapeSize+1)*currentShape;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    list[currentList][idx+2] = v2;
    list[currentList][idx+3] = v3;
    list[currentList][idx+4] = v4;
    list[currentList][idx+5] = v5;
    list[currentList][idx+6] = v6;
    list[currentList][idx+7] = v7;
    list[currentList][idx+8] = v8;
    currentShape++;
}

vtkVolumeFromVolume::WedgeList::WedgeList()
    : vtkVolumeFromVolume::ShapeList(6)
{
}
 

vtkVolumeFromVolume::WedgeList::~WedgeList()
{
}
 
void
vtkVolumeFromVolume::WedgeList::AddWedge(int cellId,
                                         int v1, int v2, int v3,
                                         int v4, int v5, int v6)
{
    if (currentShape >= shapesPerList)
    {
        if (currentList >= listSize+1)
        {
            int **tmpList = new int*[2*listSize];
            for (int i = 0 ; i < listSize ; i++)
            {
                tmpList[i] = list[i];
            }
            listSize *= 2;
            delete [] list;
            list = tmpList;
        }
 
        currentList++;
        list[currentList] = new int[(shapeSize+1)*shapesPerList];
        currentShape = 0;
    }
 
    int idx = (shapeSize+1)*currentShape;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    list[currentList][idx+2] = v2;
    list[currentList][idx+3] = v3;
    list[currentList][idx+4] = v4;
    list[currentList][idx+5] = v5;
    list[currentList][idx+6] = v6;
    currentShape++;
}

vtkVolumeFromVolume::PyramidList::PyramidList()
    : vtkVolumeFromVolume::ShapeList(5)
{
}
 

vtkVolumeFromVolume::PyramidList::~PyramidList()
{
}
 
void
vtkVolumeFromVolume::PyramidList::AddPyramid(int cellId, int v1, int v2,
                                             int v3, int v4, int v5)
{
    if (currentShape >= shapesPerList)
    {
        if (currentList >= listSize+1)
        {
            int **tmpList = new int*[2*listSize];
            for (int i = 0 ; i < listSize ; i++)
            {
                tmpList[i] = list[i];
            }
            listSize *= 2;
            delete [] list;
            list = tmpList;
        }
 
        currentList++;
        list[currentList] = new int[(shapeSize+1)*shapesPerList];
        currentShape = 0;
    }
 
    int idx = (shapeSize+1)*currentShape;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    list[currentList][idx+2] = v2;
    list[currentList][idx+3] = v3;
    list[currentList][idx+4] = v4;
    list[currentList][idx+5] = v5;
    currentShape++;
}

vtkVolumeFromVolume::TetList::TetList()
    : vtkVolumeFromVolume::ShapeList(4)
{
}
 

vtkVolumeFromVolume::TetList::~TetList()
{
}
 
void
vtkVolumeFromVolume::TetList::AddTet(int cellId, int v1,int v2,int v3,int v4)
{
    if (currentShape >= shapesPerList)
    {
        if (currentList >= listSize+1)
        {
            int **tmpList = new int*[2*listSize];
            for (int i = 0 ; i < listSize ; i++)
            {
                tmpList[i] = list[i];
            }
            listSize *= 2;
            delete [] list;
            list = tmpList;
        }
 
        currentList++;
        list[currentList] = new int[(shapeSize+1)*shapesPerList];
        currentShape = 0;
    }
 
    int idx = (shapeSize+1)*currentShape;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    list[currentList][idx+2] = v2;
    list[currentList][idx+3] = v3;
    list[currentList][idx+4] = v4;
    currentShape++;
}

vtkVolumeFromVolume::QuadList::QuadList()
    : vtkVolumeFromVolume::ShapeList(4)
{
}
 

vtkVolumeFromVolume::QuadList::~QuadList()
{
}
 
void
vtkVolumeFromVolume::QuadList::AddQuad(int cellId, int v1,int v2,int v3,int v4)
{
    if (currentShape >= shapesPerList)
    {
        if (currentList >= listSize+1)
        {
            int **tmpList = new int*[2*listSize];
            for (int i = 0 ; i < listSize ; i++)
            {
                tmpList[i] = list[i];
            }
            listSize *= 2;
            delete [] list;
            list = tmpList;
        }
 
        currentList++;
        list[currentList] = new int[(shapeSize+1)*shapesPerList];
        currentShape = 0;
    }
 
    int idx = (shapeSize+1)*currentShape;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    list[currentList][idx+2] = v2;
    list[currentList][idx+3] = v3;
    list[currentList][idx+4] = v4;
    currentShape++;
}

vtkVolumeFromVolume::TriList::TriList()
    : vtkVolumeFromVolume::ShapeList(3)
{
}
 

vtkVolumeFromVolume::TriList::~TriList()
{
}
 
void
vtkVolumeFromVolume::TriList::AddTri(int cellId, int v1,int v2,int v3)
{
    if (currentShape >= shapesPerList)
    {
        if (currentList >= listSize+1)
        {
            int **tmpList = new int*[2*listSize];
            for (int i = 0 ; i < listSize ; i++)
            {
                tmpList[i] = list[i];
            }
            listSize *= 2;
            delete [] list;
            list = tmpList;
        }
 
        currentList++;
        list[currentList] = new int[(shapeSize+1)*shapesPerList];
        currentShape = 0;
    }
 
    int idx = (shapeSize+1)*currentShape;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    list[currentList][idx+2] = v2;
    list[currentList][idx+3] = v3;
    currentShape++;
}



// ****************************************************************************
//  Modifications:
//    Jeremy Meredith, Mon Feb 16 19:11:34 PST 2004
//    Added polygonal cell support.
//
// ****************************************************************************
void
vtkVolumeFromVolume::ConstructDataSet(vtkPointData *inPD, vtkCellData *inCD,
                                      vtkUnstructuredGrid *output,
                                      float *pts_ptr)
{
    int   i, j;

    vtkPointData *outPD = output->GetPointData();
    vtkCellData  *outCD = output->GetCellData();

    //
    // Set up the output points and its point data.
    //
    vtkPoints *outPts = vtkPoints::New();
    int centroidStart = numPrevPts + pt_list.GetTotalNumberOfPoints();
    int nOutPts = centroidStart + centroid_list.GetTotalNumberOfPoints();
    outPts->SetNumberOfPoints(nOutPts);
    outPD->CopyAllocate(inPD, nOutPts);

    int ptIdx = 0;
    for (i = 0 ; i < numPrevPts ; i++)
    {
        outPts->SetPoint(ptIdx, &pts_ptr[3*i]);
        outPD->CopyData(inPD, i, ptIdx);
        ptIdx++;
    }

    int nLists = pt_list.GetNumberOfLists();
    for (i = 0 ; i < nLists ; i++)
    {
        const PointEntry *pe_list = NULL;
        int nPts = pt_list.GetList(i, pe_list);
        for (j = 0 ; j < nPts ; j++)
        {
            const PointEntry &pe = pe_list[j];
            float pt[3];
            int idx1 = pe.ptIds[0]*3;
            int idx2 = pe.ptIds[1]*3;
            float p  = pe.percent;
            float bp = 1. - p;
            pt[0] = pts_ptr[idx1]*p + pts_ptr[idx2]*bp;
            idx1++; idx2++;
            pt[1] = pts_ptr[idx1]*p + pts_ptr[idx2]*bp;
            idx1++; idx2++;
            pt[2] = pts_ptr[idx1]*p + pts_ptr[idx2]*bp;
            idx1++; idx2++;
            outPts->SetPoint(ptIdx, pt);
            outPD->InterpolateEdge(inPD, ptIdx, pe.ptIds[0], pe.ptIds[1], bp);
            ptIdx++;
        }
    }

    nLists = centroid_list.GetNumberOfLists();
    for (i = 0 ; i < nLists ; i++)
    {
        const CentroidPointEntry *ce_list = NULL;
        int nPts = centroid_list.GetList(i, ce_list);
        vtkIdList *idList = vtkIdList::New();
        for (j = 0 ; j < nPts ; j++)
        {
            const CentroidPointEntry &ce = ce_list[j];
            idList->SetNumberOfIds(ce.nPts);
            float pts[8][3];
            float weights[8];
            float pt[3] = {0., 0., 0.};
            for (int k = 0 ; k < ce.nPts ; k++)
            {
                idList->SetId(k, ce.ptIds[k]);
                weights[k] = 1.0 / float(ce.nPts);
                int id = (ce.ptIds[k]<0) ? ((centroidStart-1)-ce.ptIds[k])
                                                                 : ce.ptIds[k];
                outPts->GetPoint(id, pts[k]);
                pt[0] += pts[k][0];
                pt[1] += pts[k][1];
                pt[2] += pts[k][2];
            }
            pt[0] /= float(ce.nPts);
            pt[1] /= float(ce.nPts);
            pt[2] /= float(ce.nPts);

            outPts->SetPoint(ptIdx, pt);
            outPD->InterpolatePoint(outPD, ptIdx, idList, weights);
            ptIdx++;
        }
        idList->Delete();
    }

    output->SetPoints(outPts);
    outPts->Delete();

    //
    // Now set up the shapes and the cell data.
    //
    int cellId = 0;
    int nlists;

    int ntets     = tets.GetTotalNumberOfShapes();
    int npyramids = pyramids.GetTotalNumberOfShapes();
    int nwedges   = wedges.GetTotalNumberOfShapes();
    int nhexes    = hexes.GetTotalNumberOfShapes();
    int nshapes   = ntets + npyramids + nwedges + nhexes;

    output->Allocate(ntets*(4+1) +
                     npyramids*(5+1) +
                     nwedges*(6+1) +
                     nhexes*(8+1));
    outCD->CopyAllocate(inCD, nshapes);

    nlists = tets.GetNumberOfLists();
    for (i = 0 ; i < nlists ; i++)
    {
        const int *list;
        int listSize = tets.GetList(i, list);
        for (j = 0 ; j < listSize ; j++)
        {
            outCD->CopyData(inCD, list[0], cellId);
            vtkIdType tet[4];
            tet[0] = (list[1] < 0) ? (centroidStart-1 - list[1]) : (list[1]);
            tet[1] = (list[2] < 0) ? (centroidStart-1 - list[2]) : (list[2]);
            tet[2] = (list[3] < 0) ? (centroidStart-1 - list[3]) : (list[3]);
            tet[3] = (list[4] < 0) ? (centroidStart-1 - list[4]) : (list[4]);
            output->InsertNextCell(VTK_TETRA, 4, tet);
            list += 5;
            cellId++;
        }
    }

    nlists = pyramids.GetNumberOfLists();
    for (i = 0 ; i < nlists ; i++)
    {
        const int *list;
        int listSize = pyramids.GetList(i, list);
        for (j = 0 ; j < listSize ; j++)
        {
            outCD->CopyData(inCD, list[0], cellId);
            vtkIdType pyramid[5];
            pyramid[0] = (list[1] < 0) ? (centroidStart-1 - list[1]) : (list[1]);
            pyramid[1] = (list[2] < 0) ? (centroidStart-1 - list[2]) : (list[2]);
            pyramid[2] = (list[3] < 0) ? (centroidStart-1 - list[3]) : (list[3]);
            pyramid[3] = (list[4] < 0) ? (centroidStart-1 - list[4]) : (list[4]);
            pyramid[4] = (list[5] < 0) ? (centroidStart-1 - list[5]) : (list[5]);
            output->InsertNextCell(VTK_PYRAMID, 5, pyramid);
            list += 6;
            cellId++;
        }
    }

    nlists = wedges.GetNumberOfLists();
    for (i = 0 ; i < nlists ; i++)
    {
        const int *list;
        int listSize = wedges.GetList(i, list);
        for (j = 0 ; j < listSize ; j++)
        {
            outCD->CopyData(inCD, list[0], cellId);
            vtkIdType wedge[6];
            wedge[0] = (list[1] < 0) ? (centroidStart-1 - list[1]) : (list[1]);
            wedge[1] = (list[2] < 0) ? (centroidStart-1 - list[2]) : (list[2]);
            wedge[2] = (list[3] < 0) ? (centroidStart-1 - list[3]) : (list[3]);
            wedge[3] = (list[4] < 0) ? (centroidStart-1 - list[4]) : (list[4]);
            wedge[4] = (list[5] < 0) ? (centroidStart-1 - list[5]) : (list[5]);
            wedge[5] = (list[6] < 0) ? (centroidStart-1 - list[6]) : (list[6]);
            output->InsertNextCell(VTK_WEDGE, 6, wedge);
            list += 7;
            cellId++;
        }
    }

    nlists = hexes.GetNumberOfLists();
    for (i = 0 ; i < nlists ; i++)
    {
        const int *list;
        int listSize = hexes.GetList(i, list);
        for (j = 0 ; j < listSize ; j++)
        {
            outCD->CopyData(inCD, list[0], cellId);
            vtkIdType hex[8];
            hex[0] = (list[1] < 0) ? (centroidStart-1 - list[1]) : (list[1]);
            hex[1] = (list[2] < 0) ? (centroidStart-1 - list[2]) : (list[2]);
            hex[2] = (list[3] < 0) ? (centroidStart-1 - list[3]) : (list[3]);
            hex[3] = (list[4] < 0) ? (centroidStart-1 - list[4]) : (list[4]);
            hex[4] = (list[5] < 0) ? (centroidStart-1 - list[5]) : (list[5]);
            hex[5] = (list[6] < 0) ? (centroidStart-1 - list[6]) : (list[6]);
            hex[6] = (list[7] < 0) ? (centroidStart-1 - list[7]) : (list[7]);
            hex[7] = (list[8] < 0) ? (centroidStart-1 - list[8]) : (list[8]);
            output->InsertNextCell(VTK_HEXAHEDRON, 8, hex);
            list += 9;
            cellId++;
        }
    }

    nlists = quads.GetNumberOfLists();
    for (i = 0 ; i < nlists ; i++)
    {
        const int *list;
        int listSize = quads.GetList(i, list);
        for (j = 0 ; j < listSize ; j++)
        {
            outCD->CopyData(inCD, list[0], cellId);
            vtkIdType quad[4];
            quad[0] = (list[1] < 0) ? (centroidStart-1 - list[1]) : (list[1]);
            quad[1] = (list[2] < 0) ? (centroidStart-1 - list[2]) : (list[2]);
            quad[2] = (list[3] < 0) ? (centroidStart-1 - list[3]) : (list[3]);
            quad[3] = (list[4] < 0) ? (centroidStart-1 - list[4]) : (list[4]);
            output->InsertNextCell(VTK_QUAD, 4, quad);
            list += 5;
            cellId++;
        }
    }

    nlists = tris.GetNumberOfLists();
    for (i = 0 ; i < nlists ; i++)
    {
        const int *list;
        int listSize = tris.GetList(i, list);
        for (j = 0 ; j < listSize ; j++)
        {
            outCD->CopyData(inCD, list[0], cellId);
            vtkIdType tri[3];
            tri[0] = (list[1] < 0) ? (centroidStart-1 - list[1]) : (list[1]);
            tri[1] = (list[2] < 0) ? (centroidStart-1 - list[2]) : (list[2]);
            tri[2] = (list[3] < 0) ? (centroidStart-1 - list[3]) : (list[3]);
            output->InsertNextCell(VTK_TRIANGLE, 3, tri);
            list += 4;
            cellId++;
        }
    }
}


inline void GetPoint(float *pt, const float *X, const float *Y,
                     const float *Z, const int *dims, const int &index)
{
    int cellI = index % dims[0];
    int cellJ = (index/dims[0]) % dims[1];
    int cellK = index/(dims[0]*dims[1]);
    pt[0] = X[cellI];
    pt[1] = Y[cellJ];
    pt[2] = Z[cellK];
}


// ****************************************************************************
//  Modifications:
//    Jeremy Meredith, Mon Feb 16 19:11:34 PST 2004
//    Added polygonal cell support.
//
// ****************************************************************************
void
vtkVolumeFromVolume::ConstructDataSet(vtkPointData *inPD, vtkCellData *inCD,
                                      vtkUnstructuredGrid *output,
                                      int *dims, float *X, float *Y, float *Z)
{
    int   i, j;

    vtkPointData *outPD = output->GetPointData();
    vtkCellData  *outCD = output->GetCellData();

    //
    // Set up the output points and its point data.
    //
    vtkPoints *outPts = vtkPoints::New();
    int centroidStart = numPrevPts + pt_list.GetTotalNumberOfPoints();
    int nOutPts = centroidStart + centroid_list.GetTotalNumberOfPoints();
    outPts->SetNumberOfPoints(nOutPts);
    outPD->CopyAllocate(inPD, nOutPts);

    int ptIdx = 0;
    for (i = 0 ; i < numPrevPts ; i++)
    {
        float pt[3];
        GetPoint(pt, X, Y, Z, dims, i);
        outPts->SetPoint(ptIdx, pt);
        outPD->CopyData(inPD, i, ptIdx);
        ptIdx++;
    }

    int nLists = pt_list.GetNumberOfLists();
    for (i = 0 ; i < nLists ; i++)
    {
        const PointEntry *pe_list = NULL;
        int nPts = pt_list.GetList(i, pe_list);
        for (j = 0 ; j < nPts ; j++)
        {
            const PointEntry &pe = pe_list[j];
            float pt[3], pt1[3], pt2[3];
            GetPoint(pt1, X, Y, Z, dims, pe.ptIds[0]);
            GetPoint(pt2, X, Y, Z, dims, pe.ptIds[1]);
            float p  = pe.percent;
            float bp = 1. - p;
            pt[0] = pt1[0]*p + pt2[0]*bp;
            pt[1] = pt1[1]*p + pt2[1]*bp;
            pt[2] = pt1[2]*p + pt2[2]*bp;
            outPts->SetPoint(ptIdx, pt);
            outPD->InterpolateEdge(inPD, ptIdx, pe.ptIds[0], pe.ptIds[1], bp);
            ptIdx++;
        }
    }

    nLists = centroid_list.GetNumberOfLists();
    for (i = 0 ; i < nLists ; i++)
    {
        const CentroidPointEntry *ce_list = NULL;
        int nPts = centroid_list.GetList(i, ce_list);
        vtkIdList *idList = vtkIdList::New();
        for (j = 0 ; j < nPts ; j++)
        {
            const CentroidPointEntry &ce = ce_list[j];
            idList->SetNumberOfIds(ce.nPts);
            float pts[8][3];
            float weights[8];
            float pt[3] = {0., 0., 0.};
            for (int k = 0 ; k < ce.nPts ; k++)
            {
                idList->SetId(k, ce.ptIds[k]);
                weights[k] = 1.0 / float(ce.nPts);
                int id = (ce.ptIds[k]<0) ? ((centroidStart-1)-ce.ptIds[k])
                                                                 : ce.ptIds[k];
                outPts->GetPoint(id, pts[k]);
                pt[0] += pts[k][0];
                pt[1] += pts[k][1];
                pt[2] += pts[k][2];
            }
            pt[0] /= float(ce.nPts);
            pt[1] /= float(ce.nPts);
            pt[2] /= float(ce.nPts);

            outPts->SetPoint(ptIdx, pt);
            outPD->InterpolatePoint(outPD, ptIdx, idList, weights);
            ptIdx++;
        }
        idList->Delete();
    }

    output->SetPoints(outPts);
    outPts->Delete();

    //
    // Now set up the shapes and the cell data.
    //
    int cellId = 0;
    int nlists;

    int ntets     = tets.GetTotalNumberOfShapes();
    int npyramids = pyramids.GetTotalNumberOfShapes();
    int nwedges   = wedges.GetTotalNumberOfShapes();
    int nhexes    = hexes.GetTotalNumberOfShapes();
    int nshapes   = ntets + npyramids + nwedges + nhexes;

    output->Allocate(ntets*(4+1) +
                     npyramids*(5+1) +
                     nwedges*(6+1) +
                     nhexes*(8+1));
    outCD->CopyAllocate(inCD, nshapes);

    nlists = tets.GetNumberOfLists();
    for (i = 0 ; i < nlists ; i++)
    {
        const int *list;
        int listSize = tets.GetList(i, list);
        for (j = 0 ; j < listSize ; j++)
        {
            outCD->CopyData(inCD, list[0], cellId);
            vtkIdType tet[4];
            tet[0] = (list[1] < 0) ? (centroidStart-1 - list[1]) : (list[1]);
            tet[1] = (list[2] < 0) ? (centroidStart-1 - list[2]) : (list[2]);
            tet[2] = (list[3] < 0) ? (centroidStart-1 - list[3]) : (list[3]);
            tet[3] = (list[4] < 0) ? (centroidStart-1 - list[4]) : (list[4]);
            output->InsertNextCell(VTK_TETRA, 4, tet);
            list += 5;
            cellId++;
        }
    }

    nlists = pyramids.GetNumberOfLists();
    for (i = 0 ; i < nlists ; i++)
    {
        const int *list;
        int listSize = pyramids.GetList(i, list);
        for (j = 0 ; j < listSize ; j++)
        {
            outCD->CopyData(inCD, list[0], cellId);
            vtkIdType pyramid[5];
            pyramid[0] = (list[1] < 0) ? (centroidStart-1 - list[1]) : (list[1]);
            pyramid[1] = (list[2] < 0) ? (centroidStart-1 - list[2]) : (list[2]);
            pyramid[2] = (list[3] < 0) ? (centroidStart-1 - list[3]) : (list[3]);
            pyramid[3] = (list[4] < 0) ? (centroidStart-1 - list[4]) : (list[4]);
            pyramid[4] = (list[5] < 0) ? (centroidStart-1 - list[5]) : (list[5]);
            output->InsertNextCell(VTK_PYRAMID, 5, pyramid);
            list += 6;
            cellId++;
        }
    }

    nlists = wedges.GetNumberOfLists();
    for (i = 0 ; i < nlists ; i++)
    {
        const int *list;
        int listSize = wedges.GetList(i, list);
        for (j = 0 ; j < listSize ; j++)
        {
            outCD->CopyData(inCD, list[0], cellId);
            vtkIdType wedge[6];
            wedge[0] = (list[1] < 0) ? (centroidStart-1 - list[1]) : (list[1]);
            wedge[1] = (list[2] < 0) ? (centroidStart-1 - list[2]) : (list[2]);
            wedge[2] = (list[3] < 0) ? (centroidStart-1 - list[3]) : (list[3]);
            wedge[3] = (list[4] < 0) ? (centroidStart-1 - list[4]) : (list[4]);
            wedge[4] = (list[5] < 0) ? (centroidStart-1 - list[5]) : (list[5]);
            wedge[5] = (list[6] < 0) ? (centroidStart-1 - list[6]) : (list[6]);
            output->InsertNextCell(VTK_WEDGE, 6, wedge);
            list += 7;
            cellId++;
        }
    }

    nlists = hexes.GetNumberOfLists();
    for (i = 0 ; i < nlists ; i++)
    {
        const int *list;
        int listSize = hexes.GetList(i, list);
        for (j = 0 ; j < listSize ; j++)
        {
            outCD->CopyData(inCD, list[0], cellId);
            vtkIdType hex[8];
            hex[0] = (list[1] < 0) ? (centroidStart-1 - list[1]) : (list[1]);
            hex[1] = (list[2] < 0) ? (centroidStart-1 - list[2]) : (list[2]);
            hex[2] = (list[3] < 0) ? (centroidStart-1 - list[3]) : (list[3]);
            hex[3] = (list[4] < 0) ? (centroidStart-1 - list[4]) : (list[4]);
            hex[4] = (list[5] < 0) ? (centroidStart-1 - list[5]) : (list[5]);
            hex[5] = (list[6] < 0) ? (centroidStart-1 - list[6]) : (list[6]);
            hex[6] = (list[7] < 0) ? (centroidStart-1 - list[7]) : (list[7]);
            hex[7] = (list[8] < 0) ? (centroidStart-1 - list[8]) : (list[8]);
            output->InsertNextCell(VTK_HEXAHEDRON, 8, hex);
            list += 9;
            cellId++;
        }
    }

    nlists = quads.GetNumberOfLists();
    for (i = 0 ; i < nlists ; i++)
    {
        const int *list;
        int listSize = quads.GetList(i, list);
        for (j = 0 ; j < listSize ; j++)
        {
            outCD->CopyData(inCD, list[0], cellId);
            vtkIdType quad[4];
            quad[0] = (list[1] < 0) ? (centroidStart-1 - list[1]) : (list[1]);
            quad[1] = (list[2] < 0) ? (centroidStart-1 - list[2]) : (list[2]);
            quad[2] = (list[3] < 0) ? (centroidStart-1 - list[3]) : (list[3]);
            quad[3] = (list[4] < 0) ? (centroidStart-1 - list[4]) : (list[4]);
            output->InsertNextCell(VTK_QUAD, 4, quad);
            list += 5;
            cellId++;
        }
    }

    nlists = tris.GetNumberOfLists();
    for (i = 0 ; i < nlists ; i++)
    {
        const int *list;
        int listSize = tris.GetList(i, list);
        for (j = 0 ; j < listSize ; j++)
        {
            outCD->CopyData(inCD, list[0], cellId);
            vtkIdType tri[3];
            tri[0] = (list[1] < 0) ? (centroidStart-1 - list[1]) : (list[1]);
            tri[1] = (list[2] < 0) ? (centroidStart-1 - list[2]) : (list[2]);
            tri[2] = (list[3] < 0) ? (centroidStart-1 - list[3]) : (list[3]);
            output->InsertNextCell(VTK_TRIANGLE, 3, tri);
            list += 4;
            cellId++;
        }
    }
}
