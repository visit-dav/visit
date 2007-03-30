// ************************************************************************* //
//                           vtkSurfaceFromVolume.C                          //
// ************************************************************************* //

#include <vtkSurfaceFromVolume.h>

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

using std::vector;



vtkSurfaceFromVolume::TriangleList::TriangleList()
{
    listSize = 4096;
    trianglesPerList = 1024;
 
    list = new int*[listSize];
    list[0] = new int[4*trianglesPerList];
    for (int i = 1 ; i < listSize ; i++)
        list[i] = NULL;
 
    currentList = 0;
    currentTriangle = 0;
}
 
 
vtkSurfaceFromVolume::TriangleList::~TriangleList()
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
vtkSurfaceFromVolume::TriangleList::GetList(int listId, const int *&outlist)
    const
{
    if (listId < 0 || listId > currentList)
    {
        outlist = NULL;
        return 0;
    }
 
    outlist = list[listId];
    return (listId == currentList ? currentTriangle : trianglesPerList);
}
 
 
int
vtkSurfaceFromVolume::TriangleList::GetNumberOfLists(void) const
{
    return currentList+1;
}


int
vtkSurfaceFromVolume::TriangleList::GetTotalNumberOfTriangles(void) const
{
    int numFullLists = currentList;  // actually currentList-1+1
    int numExtra = currentTriangle;  // again, currentTriangle-1+1
 
    return numFullLists*trianglesPerList + numExtra;
}
 
 
void
vtkSurfaceFromVolume::TriangleList::AddTriangle(int cellId, int v1, int v2,
                                                int v3)
{
    if (currentTriangle >= trianglesPerList)
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
        list[currentList] = new int[4*trianglesPerList];
        currentTriangle = 0;
    }
 
    int idx = 4*currentTriangle;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    list[currentList][idx+2] = v2;
    list[currentList][idx+3] = v3;
    currentTriangle++;
}


vtkSurfaceFromVolume::PointList::PointList()
{
    listSize = 4096;
    pointsPerList = 1024;
 
    list = new PointEntry*[listSize];
    list[0] = new PointEntry[pointsPerList];
    for (int i = 1 ; i < listSize ; i++)
        list[i] = NULL;
 
    currentList = 0;
    currentPoint = 0;
}
 
 
vtkSurfaceFromVolume::PointList::~PointList()
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
vtkSurfaceFromVolume::PointList::GetList(int listId,
                                         const PointEntry *&outlist) const
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
vtkSurfaceFromVolume::PointList::GetNumberOfLists(void) const
{
    return currentList+1;
}
 
 
int
vtkSurfaceFromVolume::PointList::GetTotalNumberOfPoints(void) const
{
    int numFullLists = currentList;  // actually currentList-1+1
    int numExtra = currentPoint;  // again, currentPoint-1+1
 
    return numFullLists*pointsPerList + numExtra;
}


int
vtkSurfaceFromVolume::PointList::AddPoint(int pt0, int pt1, float percent)
{
    if (currentPoint >= pointsPerList)
    {
        if (currentList >= listSize+1)
        {
            PointEntry **tmpList = new PointEntry*[2*listSize];
            for (int i = 0 ; i < listSize ; i++)
            {
                tmpList[i] = list[i];
            }
            listSize *= 2;
            delete [] list;
            list = tmpList;
        }
 
        currentList++;
        list[currentList] = new PointEntry[pointsPerList];
        currentPoint = 0;
    }
 
    list[currentList][currentPoint].ptIds[0] = pt0;
    list[currentList][currentPoint].ptIds[1] = pt1;
    list[currentList][currentPoint].percent = percent;
    currentPoint++;
 
    return (GetTotalNumberOfPoints()-1);
}


vtkSurfaceFromVolume::EdgeHashEntry::EdgeHashEntry()
{
    id1  = -1;
    id2  = -1;
    ptId = -1;
    next = NULL;
}
 
 
void
vtkSurfaceFromVolume::EdgeHashEntry::SetInfo(int i1, int i2, int pId)
{
    id1  = i1;
    id2  = i2;
    ptId = pId;
    next = NULL;
}


vtkSurfaceFromVolume::EdgeHashEntryMemoryManager::EdgeHashEntryMemoryManager()
{
    freeEntryindex = 0;
}
 
 
vtkSurfaceFromVolume::EdgeHashEntryMemoryManager::~EdgeHashEntryMemoryManager()
{
    int npools = edgeHashEntrypool.size();
    for (int i = 0 ; i < npools ; i++)
    {
        EdgeHashEntry *pool = edgeHashEntrypool[i];
        delete [] pool;
    }
}
 
 
void
vtkSurfaceFromVolume::EdgeHashEntryMemoryManager
::AllocateEdgeHashEntryPool(void)
{
    if (freeEntryindex == 0)
    {
        EdgeHashEntry *newlist = new EdgeHashEntry[POOL_SIZE];
        edgeHashEntrypool.push_back(newlist);
        for (int i = 0 ; i < POOL_SIZE ; i++)
        {
            freeEntrylist[i] = &(newlist[i]);
        }
        freeEntryindex = POOL_SIZE;
    }
}


vtkSurfaceFromVolume::EdgeHashTable::EdgeHashTable(int nh, PointList &p)
    : pointlist(p)
{
    nHashes = nh;
    hashes = new EdgeHashEntry*[nHashes];
    for (int i = 0 ; i < nHashes ; i++)
        hashes[i] = NULL;
}
 
 
vtkSurfaceFromVolume::EdgeHashTable::~EdgeHashTable()
{
    delete [] hashes;
}
 
 
int
vtkSurfaceFromVolume::EdgeHashTable::GetKey(int p1, int p2)
{
    int rv = ((p1*18457 + p2*234749) % nHashes);
 
    // In case of overflows and modulo with negative numbers.
    if (rv < 0)
       rv += nHashes;
 
    return rv;
}


int
vtkSurfaceFromVolume::EdgeHashTable::AddPoint(int ap1, int ap2, float apercent)
{
    int p1, p2;
    float percent;
    if (ap2 < ap1)
    {
        p1 = ap2;
        p2 = ap1;
        percent = 1. - apercent;
    }
    else
    {
        p1 = ap1;
        p2 = ap2;
        percent = apercent;
    }

    int key = GetKey(p1, p2);
 
    //
    // See if we have any matches in the current hashes.
    //
    EdgeHashEntry *cur = hashes[key];
    while (cur != NULL)
    {
        if (cur->IsMatch(p1, p2))
        {
            //
            // We found a match.
            //
            return cur->GetPointId();
        }
        cur = cur->GetNext();
    }
 
    //
    // There was no match.  We will have to add a new entry.
    //
    EdgeHashEntry *new_one = emm.GetFreeEdgeHashEntry();
 
    int newPt = pointlist.AddPoint(p1, p2, percent);
    new_one->SetInfo(p1, p2, newPt);
    new_one->SetNext(hashes[key]);
    hashes[key] = new_one;
 
    return newPt;
}


void
vtkSurfaceFromVolume::ConstructPolyData(vtkPointData *inPD, vtkCellData *inCD,
                                        vtkPolyData *output, float *pts_ptr)
{
    int   i, j;

    vtkPointData *outPD = output->GetPointData();
    vtkCellData  *outCD = output->GetCellData();

    //
    // Set up the output points and its point data.
    //
    vtkPoints *outPts = vtkPoints::New();
    int nOutPts = pt_list.GetTotalNumberOfPoints();
    outPts->SetNumberOfPoints(nOutPts);
    outPD->CopyAllocate(inPD, nOutPts);
    int nLists = pt_list.GetNumberOfLists();
    int ptIdx = 0;
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
            outPD->InterpolateEdge(inPD, ptIdx, pe.ptIds[0], pe.ptIds[1],
                                   pe.percent);
            ptIdx++;
        }
    }
    output->SetPoints(outPts);
    outPts->Delete();

    //
    // Now set up the triangles and the cell data.
    //
    int ntris = tris.GetTotalNumberOfTriangles();
    output->Allocate(ntris*(3+1));
    outCD->CopyAllocate(inCD, ntris);
    int cellId = 0;
    int nlists = tris.GetNumberOfLists();
    for (i = 0 ; i < nlists ; i++)
    {
        const int *list;
        int listSize = tris.GetList(i, list);
        for (j = 0 ; j < listSize ; j++)
        {
            outCD->CopyData(inCD, list[0], cellId);
            vtkIdType tri[3];
            tri[0] = list[1];
            tri[1] = list[2];
            tri[2] = list[3];
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

void
vtkSurfaceFromVolume::ConstructPolyData(vtkPointData *inPD, vtkCellData *inCD,
                  vtkPolyData *output, int *dims, float *X, float *Y, float *Z)
{
    int   i, j;

    vtkPointData *outPD = output->GetPointData();
    vtkCellData  *outCD = output->GetCellData();

    //
    // Set up the output points and its point data.
    //
    vtkPoints *outPts = vtkPoints::New();
    int nOutPts = pt_list.GetTotalNumberOfPoints();
    outPts->SetNumberOfPoints(nOutPts);
    outPD->CopyAllocate(inPD, nOutPts);
    int nLists = pt_list.GetNumberOfLists();
    int ptIdx = 0;
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
            outPD->InterpolateEdge(inPD, ptIdx, pe.ptIds[0], pe.ptIds[1],
                                   pe.percent);
            ptIdx++;
        }
    }
    output->SetPoints(outPts);
    outPts->Delete();

    //
    // Now set up the triangles and the cell data.
    //
    int ntris = tris.GetTotalNumberOfTriangles();
    output->Allocate(ntris*(3+1));
    outCD->CopyAllocate(inCD, ntris);
    int cellId = 0;
    int nlists = tris.GetNumberOfLists();
    for (i = 0 ; i < nlists ; i++)
    {
        const int *list;
        int listSize = tris.GetList(i, list);
        for (j = 0 ; j < listSize ; j++)
        {
            outCD->CopyData(inCD, list[0], cellId);
            vtkIdType tri[3];
            tri[0] = list[1];
            tri[1] = list[2];
            tri[2] = list[3];
            output->InsertNextCell(VTK_TRIANGLE, 3, tri);
            list += 4;
            cellId++;
        }
    }
}


