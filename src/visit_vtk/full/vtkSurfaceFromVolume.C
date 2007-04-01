// ************************************************************************* //
//                           vtkSurfaceFromVolume.C                          //
// ************************************************************************* //

#include <vtkSurfaceFromVolume.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
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
 
 
// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Thu Oct 21 15:32:07 PDT 2004
//    Fix bug when we get low on memory.
//
// ****************************************************************************

void
vtkSurfaceFromVolume::TriangleList::AddTriangle(int cellId, int v1, int v2,
                                                int v3)
{
    if (currentTriangle >= trianglesPerList)
    {
        if ((currentList+1) >= listSize)
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


// ****************************************************************************
//  Modications:
//
//    Hank Childs, Fri Jan 30 08:50:44 PST 2004
//    Speed up construction by doing pointer arithmetic.
//
//    Kathleen Bonnell, Fri Nov 12 16:43:11 PST 2004 
//    Don't interoplate avtOriginalNodeNumbers, instead use the closest
//    point to the slice point.
//
// ****************************************************************************

void
vtkSurfaceFromVolume::ConstructPolyData(vtkPointData *inPD, vtkCellData *inCD,
                                        vtkPolyData *output, float *pts_ptr)
{
    int   i, j;

    vtkPointData *outPD = output->GetPointData();
    vtkCellData  *outCD = output->GetCellData();

    vtkIntArray *newOrigNodes = NULL;
    vtkIntArray *origNodes = vtkIntArray::SafeDownCast(
              inPD->GetArray("avtOriginalNodeNumbers"));

    //
    // Set up the output points and its point data.
    //
    vtkPoints *outPts = vtkPoints::New();
    int nOutPts = pt_list.GetTotalNumberOfPoints();
    outPts->SetNumberOfPoints(nOutPts);
    outPD->CopyAllocate(inPD, nOutPts);
    if (origNodes != NULL)
    {
        newOrigNodes = vtkIntArray::New();
        newOrigNodes->SetNumberOfComponents(origNodes->GetNumberOfComponents());
        newOrigNodes->SetNumberOfTuples(nOutPts);
        newOrigNodes->SetName(origNodes->GetName());
    }
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
            outPD->InterpolateEdge(inPD, ptIdx, pe.ptIds[0], pe.ptIds[1], bp);
            if (newOrigNodes)
            {
                int id = (bp <= 0.5 ? pe.ptIds[0] : pe.ptIds[1]);
                newOrigNodes->SetTuple(ptIdx, origNodes->GetTuple(id));
            }
            ptIdx++;
        }
    }
    output->SetPoints(outPts);
    outPts->Delete();
    if (newOrigNodes)
    {
        // AddArray will overwrite an already existing array with 
        // the same name, exactly what we want here.
        outPD->AddArray(newOrigNodes);
        newOrigNodes->Delete();
    }

    //
    // Now set up the triangles and the cell data.
    //
    int ntris = tris.GetTotalNumberOfTriangles();
    vtkIdTypeArray *nlist = vtkIdTypeArray::New();
    nlist->SetNumberOfValues(3*ntris + ntris);
    vtkIdType *nl = nlist->GetPointer(0);

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
            *nl++ = 3;
            *nl++ = list[1];
            *nl++ = list[2];
            *nl++ = list[3];
            list += 4;
            cellId++;
        }
    }
    vtkCellArray *cells = vtkCellArray::New();
    cells->SetCells(ntris, nlist);
    nlist->Delete();

    output->SetPolys(cells);
    cells->Delete();
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
//  Modications:
//
//    Hank Childs, Fri Jan 30 08:50:44 PST 2004
//    Speed up construction by doing pointer arithmetic.
//
//    Kathleen Bonnell, Tue Nov 23 14:07:10 PST 2004 
//    Don't interoplate avtOriginalNodeNumbers, instead use the closest
//    point to the slice point.
//
// ****************************************************************************

void
vtkSurfaceFromVolume::ConstructPolyData(vtkPointData *inPD, vtkCellData *inCD,
                  vtkPolyData *output, int *dims, float *X, float *Y, float *Z)
{
    int   i, j;

    vtkPointData *outPD = output->GetPointData();
    vtkCellData  *outCD = output->GetCellData();

    vtkIntArray *newOrigNodes = NULL;
    vtkIntArray *origNodes = vtkIntArray::SafeDownCast(
              inPD->GetArray("avtOriginalNodeNumbers"));

    //
    // Set up the output points and its point data.
    //
    vtkPoints *outPts = vtkPoints::New();
    int nOutPts = pt_list.GetTotalNumberOfPoints();
    outPts->SetNumberOfPoints(nOutPts);
    outPD->CopyAllocate(inPD, nOutPts);

    if (origNodes != NULL)
    {
        newOrigNodes = vtkIntArray::New();
        newOrigNodes->SetNumberOfComponents(origNodes->GetNumberOfComponents());
        newOrigNodes->SetNumberOfTuples(nOutPts);
        newOrigNodes->SetName(origNodes->GetName());
    }

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
            outPD->InterpolateEdge(inPD, ptIdx, pe.ptIds[0], pe.ptIds[1], bp);
            if (newOrigNodes)
            {
                int id = (bp <= 0.5 ? pe.ptIds[0] : pe.ptIds[1]);
                newOrigNodes->SetTuple(ptIdx, origNodes->GetTuple(id));
            }
            ptIdx++;
        }
    }
    output->SetPoints(outPts);
    outPts->Delete();

    if (newOrigNodes)
    {
        // AddArray will overwrite an already existing array with 
        // the same name, exactly what we want here.
        outPD->AddArray(newOrigNodes);
        newOrigNodes->Delete();
    }

    //
    // Now set up the triangles and the cell data.
    //
    int ntris = tris.GetTotalNumberOfTriangles();
    vtkIdTypeArray *nlist = vtkIdTypeArray::New();
    nlist->SetNumberOfValues(3*ntris + ntris);
    vtkIdType *nl = nlist->GetPointer(0);

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
            *nl++ = 3;
            *nl++ = list[1];
            *nl++ = list[2];
            *nl++ = list[3];
            list += 4;
            cellId++;
        }
    }
    vtkCellArray *cells = vtkCellArray::New();
    cells->SetCells(ntris, nlist);
    nlist->Delete();

    output->SetPolys(cells);
    cells->Delete();
}


