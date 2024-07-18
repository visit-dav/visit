// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkVisItPolyDataNormals.h"

#include <vtkCellArray.h>
#include <vtkCellArrayIterator.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkTriangle.h>
#include <vtkTriangleStrip.h>
#include <vtkPoints.h>

vtkStandardNewMacro(vtkVisItPolyDataNormals);

// ****************************************************************************
//  Constructor:  vtkVisItPolyDataNormals::vtkVisItPolyDataNormals
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 13, 2003
//
//  Modifications:
//    Alister Maguire, Mon Apr 27 11:25:42 PDT 2020
//    Added initialization of StripsHaveBeenDecomposed.
//
// ****************************************************************************
vtkVisItPolyDataNormals::vtkVisItPolyDataNormals()
{
    FeatureAngle             = 45.0;
    Splitting                = true;
    ComputePointNormals      = true;
    StripsHaveBeenDecomposed = false;
}


// ****************************************************************************
//  Method:  vtkVisItPolyDataNormals::Execute
//
//  Purpose:
//    main update function; decide which type of algorithm to perform
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 13, 2003
//
//  Modifications:
//    Hank Childs, Fri Feb 15 12:03:46 PST 2008
//    Remove some downcast calls.
//
//    Eric Brugger, Thu Oct  2 16:08:56 PDT 2008
//    Added code to not calculate the normals if they were already present.
//
// ****************************************************************************
int
vtkVisItPolyDataNormals::RequestData(vtkInformation *vtkNotUsed(request),
    vtkInformationVector **vtkNotUsed(inputVector),
    vtkInformationVector *vtkNotUsed(outputVector))
{
    // get the input and output
    vtkPolyData *input = GetPolyDataInput(0);
    vtkPolyData *output = GetOutput();

    if (ComputePointNormals)
    {
        if (input->GetPointData()->GetNormals() != NULL)
        {
            GetOutput()->ShallowCopy(this->GetInput());
            return 1;
        }

        // Point normals
        if (Splitting)
        {
            ExecutePointWithSplitting(input, output);
        }
        else
        {
            ExecutePointWithoutSplitting(input, output);
        }
    }
    else
    {
        // Cell normals
        if (input->GetCellData()->GetNormals() != NULL)
        {
            GetOutput()->ShallowCopy(this->GetInput());
            return 1;
        }

        ExecuteCell(input, output);
    }
    return 1;
}


// ****************************************************************************
//  Method:  vtkVisItPolyDataNormals::ExecutePointWithoutSplitting
//
//  Purpose:
//    Create normals at the points by averaging from neighboring cells.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 12, 2003
//
//  Modifications:
//    Kathleen Bonnell, Fri Aug 22 16:48:20 PDT 2003
//    Pass along Verts and Lines.
//
//    Jeremy Meredith, Fri May 28 12:30:41 PDT 2004
//    Renormalize the vectors here because VTK can't do it without
//    under/overflow on small/big vectors.  Use double precision for
//    critical math operations.  Added note of concern about using
//    VTK's normal calculation at all.
//
//    Hank Childs, Fri Jul 30 09:00:51 PDT 2004
//    Copy along cell data from verts and lines as well.
//
//    Kathleen Bonnell, Fri Feb 24 09:43:20 PST 2006
//    Avoid divide-by-zero errors.
//
//    Kathleen Bonnell, Wed Mar 29 10:52:43 PST 2006
//    VTK api computes normals using double, so create double array to store
//    normal accumulation, then store back to vtkFloatArray.
//
//    Hank Childs, Wed May 17 09:07:21 PDT 2006
//    Fix uninitialized memory read.
//
//    Hank Childs, Thu Jun  5 15:00:05 PDT 2008
//    Call ShallowCopy, not SetFieldData.
//
//    Alister Maguire, Wed Apr  1 14:49:06 PDT 2020
//    Restructured to handle triangle strips.
//
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK: use vktCellArrayIterator.
//
// ****************************************************************************
void
vtkVisItPolyDataNormals::ExecutePointWithoutSplitting(
  vtkPolyData *input, vtkPolyData *output)
{
    //
    // First, we need to transfer over our cell data. This includes
    // decomposing triangle strips.
    //
    TransferCellData(input, output);

    //
    // Get all the input and output objects we'll need to reference.
    //
    vtkPointData *inPD     = input->GetPointData();
    vtkCellData  *inCD     = input->GetCellData();
    vtkPoints    *inPts    = input->GetPoints();
    vtkCellArray *outPolys = output->GetPolys();
    vtkPointData *outPD    = output->GetPointData();

    //
    // Pass through things which will be unchanged.
    //
    output->GetFieldData()->ShallowCopy(input->GetFieldData());

    //
    // Allocate and copy the output points; there will be no extras.
    //
    vtkPoints *outPts = vtkPoints::New(inPts->GetDataType());
    int nPoints = input->GetNumberOfPoints();
    outPts->SetNumberOfPoints(nPoints);
    outPD->CopyAllocate(inPD,nPoints);
    int ptIdx = 0;
    for (int i = 0 ; i < nPoints ; i++)
    {
        double pt[3];
        inPts->GetPoint(i, pt);
        outPts->SetPoint(ptIdx, pt);
        outPD->CopyData(inPD, i, ptIdx);
        ptIdx++;
    }
    output->SetPoints(outPts);
    outPts->Delete();

    // Create and initialize the normals array
    vtkFloatArray *newNormals = vtkFloatArray::New();
    newNormals->SetNumberOfComponents(3);
    newNormals->SetNumberOfTuples(nPoints);
    newNormals->SetName("Normals");
    // Accumulate in double-array since VTK computes double normal
    double *dnormals = new double[nPoints*3];
    for (int i = 0 ; i < nPoints ; i++)
    {
        dnormals[i*3+0] = 0.;
        dnormals[i*3+1] = 0.;
        dnormals[i*3+2] = 0.;
    }

    //
    // Iterate over our newly generated polys, and create normals.
    // At this point, all triangle strips have been decomposed into
    // polys, so this is all we need to worry about.
    //
    auto iter = vtk::TakeSmartPointer(outPolys->NewIterator());
    for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal(); iter->GoToNextCell())
    {
        //
        // Technically, we can use the first three vertices only,
        // (i.e. vtkTriangle::ComputeNormal), but this is not a big
        // hit, and it accomodates for degenerate quads.  Note -- this
        // will probably fail for very large/small polygons because of
        // over/underflow.  If so, we need to switch to double precision
        // math and avoid using the VTK code.
        //
        double normal[3];
        vtkIdType nPts;
        const vtkIdType *cell = nullptr;
        iter->GetCurrentCell(nPts, cell);
        vtkPolygon::ComputeNormal(inPts, nPts, cell, normal);
        for (int j = 0 ; j < nPts ; j++)
        {
            int p = cell[j];
            dnormals[p*3+0] += normal[0];
            dnormals[p*3+1] += normal[1];
            dnormals[p*3+2] += normal[2];
        }
    }

    // Renormalize the normals; they've only been accumulated so far,
    // and store in the vtkFloatArray.
    float *newNormalPtr = (float*)newNormals->GetPointer(0);
    for (int i = 0 ; i < nPoints ; i++)
    {
        double nx = dnormals[i*3+0];
        double ny = dnormals[i*3+1];
        double nz = dnormals[i*3+2];
        double length = sqrt(nx*nx + ny*ny + nz*nz);
        if (length != 0.0)
        {
            newNormalPtr[i*3+0] = (float)(nx/length);
            newNormalPtr[i*3+1] = (float)(ny/length);
            newNormalPtr[i*3+2] = (float)(nz/length);
        }
        else
        {
            newNormalPtr[i*3+0] = 0.;
            newNormalPtr[i*3+1] = 0.;
            newNormalPtr[i*3+2] = 0.;
        }
    }
    outPD->SetNormals(newNormals);
    newNormals->Delete();
    delete [] dnormals;
}

// ****************************************************************************
//  Struct:  NormalEntry
//
//  Purpose:
//    Stores an original or duplicated point as a linked list node.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 13, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Aug 27 14:37:31 PDT 2010
//    Change to double precision to accomodate accumulating very large normals.
//
// ****************************************************************************
struct NormalEntry
{
    int          oldId;  // The id of this point before it was split
    int          newId;  // The new id of this node
    double       n[3];   // The accumulated normal
    double      nn[3];  // The normalized normal
    NormalEntry *next;   // The next entry in the linked list for this oldId

    // Constructor
    NormalEntry() : oldId(-1), newId(-1), next(NULL)
    {
        // No need to initialize n or nn.
    }
};

// ****************************************************************************
//  Class:  NormalList
//
//  Purpose:
//    Stores all original and duplicated points as a nPoints-sized array
//    of linked lists.  Most lists will be of length 1 except for those
//    points that lie along feature edges.  This class also efficiently
//    allocates new linked-list nodes ('NormalEntry's).
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 13, 2003
//
//  Modifications:
//      Sean Ahern, Mon Mar  5 14:47:42 EST 2007
//      Removed fixed length on normal lists.
//
// ****************************************************************************
class NormalList
{
#define NORMAL_LIST_LEN  4096
public:
    NormalEntry *normals;

    // Constructor
    NormalList(int initialSize)
    {
        originalSize = initialSize;
        normals = new NormalEntry[initialSize];

        poolListIndex = 0;
        poolIndex = 0;
        poolSize = 1024;    // Initial
        pool = new NormalEntry*[poolSize];
        pool[0] = new NormalEntry[NORMAL_LIST_LEN];
    }

    // Destructor
    ~NormalList()
    {
        delete[] normals;
        for (int i=0; i<=poolListIndex; i++)
            delete[] pool[i];
        delete[] pool;
    }

    // Gets a new entry from the allocation pool
    NormalEntry *GetNewEntry()
    {
        NormalEntry *ret = &pool[poolListIndex][poolIndex];
        poolIndex++;

        if (poolIndex >= NORMAL_LIST_LEN)
        {
            poolListIndex++;
            poolIndex = 0;

            if (poolListIndex >= poolSize)
            {
                // Double the length of the pool to make space for the next
                // one we might want.
                NormalEntry **newPool = new NormalEntry*[poolSize*2];
                for(int i=0;i<poolListIndex;i++)
                    newPool[i] = pool[i];
                poolSize *= 2;
                delete [] pool;
                pool = newPool;
            }

            pool[poolListIndex] = new NormalEntry[NORMAL_LIST_LEN];
        }

        return ret;
    }

    // Calculate the size of the output arrays
    int GetTotalNumberOfEntries()
    {
        return originalSize + (poolListIndex*NORMAL_LIST_LEN) + poolIndex;
    }

    // Return the number of lists, (last one is possibly partially used;
    // check GetTotalNumberOfEntries() to find out when to stop).
    int GetNumberOfExtraLists()
    {
        return poolListIndex+1;
    }

    // Return the nth array of extra NormalEntries
    NormalEntry *GetExtraList(int n)
    {
        return pool[n];
    }

protected:
    int           originalSize;
    NormalEntry  **pool;
    int           poolSize;
    int           poolListIndex;
    int           poolIndex;
};


// ****************************************************************************
//  Method: vtkVisItPolyDataNormals::ExecutePointWithSplitting
//
//  Purpose:
//    Create normals at the points, and duplicate nodes along
//    feature edges (as many times as necessary).
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 12, 2003
//
//  Modifications:
//    Kathleen Bonnell, Fri Aug 22 16:48:20 PDT 2003
//    Pass along Verts and Lines.  Added test for ne->oldId < 0 when
//    adding original points and normals.
//
//    Hank Childs, Fri Jan 30 09:35:46 PST 2004
//    Use pointer arithmetic to avoid VTK calls.
//
//    Jeremy Meredith, Fri May 28 09:46:14 PDT 2004
//    Use double precision math in critical sections.
//
//    Hank Childs, Fri Jul 30 09:00:51 PDT 2004
//    Account for cell data in verts and lines.
//
//    Kathleen Bonnell, Tue Nov 16 10:29:25 PST 2004
//    Make nOtherCells be the count of verts and lines (not polys and lines).
//
//    Kathleen Bonnell, Fri Feb 24 09:43:20 PST 2006
//    Avoid divide-by-zero errors.
//
//    Hank Childs, Thu Jun  5 15:00:05 PDT 2008
//    Call ShallowCopy, not SetFieldData.
//
//    Alister Maguire, Wed Apr  1 14:49:06 PDT 2020
//    Restructured to handle triangle strips.
//
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK: use vktCellArrayIterator.
//
// ****************************************************************************
void
vtkVisItPolyDataNormals::ExecutePointWithSplitting(vtkPolyData *input,
                                                   vtkPolyData *output)
{
    //
    // First, we need to transfer over our cell data. This includes
    // decomposing triangle strips.
    //
    TransferCellData(input, output);

    //
    // Get all the input and output objects we'll need to reference.
    //
    vtkPointData *inPD     = input->GetPointData();
    vtkPoints    *inPts    = input->GetPoints();
    vtkCellArray *outPolys = output->GetPolys();
    vtkPointData *outPD    = output->GetPointData();

    //
    // Our output data should now have the correct cells transfered
    // over, including any decomposed strips. Point data has not
    // yet been transfered.
    //
    int nPoints       = input->GetNumberOfPoints();
    int newPointIndex = nPoints;

    // Pass through things which will be unchanged
    output->GetFieldData()->ShallowCopy(input->GetFieldData());

    // Precalculate the cosine of the feature angle
    float cosAngle = cos(FeatureAngle * 3.1415926536 / 180.);

    // This is the data structure we will rely on to hold
    // out intermediate normals calculations and to allocate
    // the duplicated points (where a feature edge was found).
    NormalList normalList(nPoints);

    //
    // Iterate over our newly generated polys, and create normals.
    // At this point, all triangle strips have been decomposed into
    // polys, so this is all we need to worry about.
    //
    vtkIdType nPts = 0;
    vtkIdList *currentCellPts = nullptr;
    auto iter = vtk::TakeSmartPointer(outPolys->NewIterator());
    for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal(); iter->GoToNextCell())
    {
        bool replaceCurrentCell = false;
        currentCellPts = iter->GetCurrentCell();
        nPts = currentCellPts->GetNumberOfIds();
        vtkIdType *pts = currentCellPts->GetPointer(0);
        //
        // Technically, we can always use only the first three vertices, but
        // it is not a big hit to do the quads better, and it accomodates for
        // degenerate quads directly.  The code is the same algorithm as
        // vtkPolygon::ComputeNormal, but changed to make it work better.
        //
        double p0[3], p1[3], p2[3];
        double normal[3] = {0, 0, 0};
        if (nPts == 3)
        {
            inPts->GetPoint(pts[0], p0);
            inPts->GetPoint(pts[1], p1);
            inPts->GetPoint(pts[2], p2);
            vtkTriangle::ComputeNormalDirection(p0, p1, p2, normal);
        }
        else
        {
            // Accumulate the normals calculated from every adjacent edge pair.
            inPts->GetPoint(pts[0],p1);
            inPts->GetPoint(pts[1],p2);

            double ax, ay, az, bx, by, bz;
            for (vtkIdType j = 0 ; j < nPts ; j++)
            {
                p0[0] = p1[0]; p0[1] = p1[1]; p0[2] = p1[2];
                p1[0] = p2[0]; p1[1] = p2[1]; p1[2] = p2[2];
                inPts->GetPoint(pts[(j+2) % nPts],p2);

                ax = p2[0] - p1[0]; ay = p2[1] - p1[1]; az = p2[2] - p1[2];
                bx = p0[0] - p1[0]; by = p0[1] - p1[1]; bz = p0[2] - p1[2];

                normal[0] += (ay * bz - az * by);
                normal[1] += (az * bx - ax * bz);
                normal[2] += (ax * by - ay * bx);
            }
            normal[0] /= nPts;
            normal[1] /= nPts;
            normal[2] /= nPts;
        }

        // Calculate the length, and throw out degenerate cases
        double nx = normal[0];
        double ny = normal[1];
        double nz = normal[2];
        double length = sqrt(nx*nx + ny*ny + nz*nz);

        if (length == 0) continue;

        // Store the normalized version separately
        double nnormal[3] = {nx/length, ny/length, nz/length};

        // Loop over all points of the cell, deciding if we need
        // to split it or can merge with an old one.  Use the feature
        // angle set before execution.
        for (vtkIdType j = 0 ; j < nPts ; j++)
        {
            vtkIdType p = pts[j];
            bool found = false;
            NormalEntry *ne = &normalList.normals[p];
            while (ne->oldId >= 0 && ne != NULL)
            {
                if (vtkMath::Dot(nnormal, ne->nn) > cosAngle)
                {
                    found = true;
                    break;
                }
                if (ne->next == NULL)
                    break;

                ne      = ne->next;
                pts[j] = ne->newId;
                replaceCurrentCell = true;
            }

            if (ne->oldId < 0) // first cell adjacent to this point in space
            {
                // This is essentially initialization for the "normal" case
                // without any splitting.  (The pun was unavoidable. Sorry.)
                ne->oldId = p;
                ne->newId = p;
                ne->n[0] = normal[0];
                ne->n[1] = normal[1];
                ne->n[2] = normal[2];
                ne->nn[0]= nnormal[0];
                ne->nn[1]= nnormal[1];
                ne->nn[2]= nnormal[2];
            }
            else if (found) // not the first, but we found a match
            {
                // This is where we accumulate the normals at the nodes
                double *n = ne->n;
                n[0] += normal[0];
                n[1] += normal[1];
                n[2] += normal[2];

                double nx = n[0];
                double ny = n[1];
                double nz = n[2];
                double newlength = sqrt(nx*nx + ny*ny + nz*nz);

                double *nn = ne->nn;
                if (newlength != 0.0)
                {
                    nn[0] = n[0]/newlength;
                    nn[1] = n[1]/newlength;
                    nn[2] = n[2]/newlength;
                }
                else
                {
                    nn[0] = n[0];
                    nn[1] = n[1];
                    nn[2] = n[2];
                }
            }
            else // no match found; duplicate the point
            {
                ne->next = normalList.GetNewEntry();
                ne = ne->next;

                pts[j]   = newPointIndex;
                ne->oldId = p;
                ne->newId = newPointIndex;
                ne->n[0]  = normal[0];
                ne->n[1]  = normal[1];
                ne->n[2]  = normal[2];
                ne->nn[0] = nnormal[0];
                ne->nn[1] = nnormal[1];
                ne->nn[2] = nnormal[2];

                newPointIndex++;
                replaceCurrentCell = true;
            }
        }
        if(replaceCurrentCell)
            iter->ReplaceCurrentCell(currentCellPts);
    }

    // Create the output points array
    int nOutPts = normalList.GetTotalNumberOfEntries();
    vtkPoints *outPts = vtkPoints::New(inPts->GetDataType());
    outPts->SetNumberOfPoints(nOutPts);
    outPD->CopyAllocate(inPD,nOutPts);

    // Create the normals array
    vtkFloatArray *newNormals;
    newNormals = vtkFloatArray::New();
    newNormals->SetNumberOfComponents(3);
    newNormals->SetNumberOfTuples(nOutPts);
    newNormals->SetName("Normals");
    float *newNormalPtr = (float*)newNormals->GetPointer(0);

    // Add all the original points and normals
    for (int i = 0 ; i < nPoints ; i++)
    {
        NormalEntry *ne = &normalList.normals[i];
        if (ne->oldId < 0)
        {
            outPts->SetPoint(i, inPts->GetPoint(i));
            outPD->CopyData(inPD, i, i);
            newNormalPtr[3*i+0] = 0;
            newNormalPtr[3*i+1] = 0;
            newNormalPtr[3*i+2] = 1;
        }
        else
        {
            outPts->SetPoint(i, inPts->GetPoint(ne->oldId));
            outPD->CopyData(inPD, ne->oldId, i);
            newNormalPtr[3*i+0] = ne->nn[0];
            newNormalPtr[3*i+1] = ne->nn[1];
            newNormalPtr[3*i+2] = ne->nn[2];
        }
    }

    // Add all the new (duplicated) points
    int outIndex = nPoints;
    int nLists = normalList.GetNumberOfExtraLists();
    for (int i = 0 ; i < nLists ; i++)
    {
        NormalEntry *list = normalList.GetExtraList(i);
        for (int j = 0 ; j < NORMAL_LIST_LEN ; j++)
        {
            if (outIndex >= nOutPts)
                break;
            NormalEntry *ne = &list[j];
            outPts->SetPoint(outIndex, inPts->GetPoint(ne->oldId));
            outPD->CopyData(inPD, ne->oldId, outIndex);
            newNormalPtr[3*outIndex+0] = ne->nn[0];
            newNormalPtr[3*outIndex+1] = ne->nn[1];
            newNormalPtr[3*outIndex+2] = ne->nn[2];
            outIndex++;
        }
    }

    output->SetPoints(outPts);
    outPts->Delete();

    outPD->SetNormals(newNormals);
    newNormals->Delete();
}


// ****************************************************************************
//  Method:  vtkVisItPolyDataNormals::ExecuteCell
//
//  Purpose:
//    Create normals for each cell.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 12, 2003
//
//  Modifications:
//    Hank Childs, Fri Jan 30 09:26:38 PST 2004
//    Performance improvements.
//
//    Jeremy Meredith, Fri May 28 12:29:29 PDT 2004
//    Calculate the normals here because VTK can't do it without
//    under/overflow on small/big vectors.
//
//    Hank Childs, Wed Jun  2 07:35:27 PDT 2004
//    Make sure there is one normal for every primitive, not just one normal
//    for every polygon.
//
//    Alister Maguire, Wed Apr  1 13:39:03 PDT 2020
//    Restructured to properly handle triangle strips.
//
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK: use vktCellArrayIterator.
//
// ****************************************************************************
void
vtkVisItPolyDataNormals::ExecuteCell(vtkPolyData *input, vtkPolyData *output)
{
    //
    // First, we need to transfer over our cell data. This includes
    // decomposing triangle strips.
    //
    TransferCellData(input, output);

    //
    // If our input contained triangle strips, we likely
    // have a new cell count.
    //
    int numOutCells = output->GetNumberOfCells();
    vtkCellArray *outPolys = output->GetPolys();

    //
    // Our cell arrays have a specific ordering: vertices and lines,
    // polys, and triangle strips (which have now been replaced with
    // triangles).
    //
    int numPrimitivesWithoutNormals = 0;
    numPrimitivesWithoutNormals    += input->GetVerts()->GetNumberOfCells();
    numPrimitivesWithoutNormals    += input->GetLines()->GetNumberOfCells();

    //
    // All of this data can just be copied over.
    //
    output->SetPoints(input->GetPoints());
    output->GetPointData()->CopyNormalsOff();
    output->GetPointData()->ShallowCopy(input->GetPointData());
    output->GetFieldData()->ShallowCopy(input->GetFieldData());

    //
    // Create our normals array.
    //
    vtkFloatArray *newNormals;
    newNormals = vtkFloatArray::New();
    newNormals->SetNumberOfComponents(3);
    newNormals->SetNumberOfTuples(numOutCells);
    newNormals->SetName("Normals");
    float *newNormalPtr = (float*)newNormals->GetPointer(0);

    //
    // Add normals for verts and lines.
    //
    for (vtkIdType i = 0; i < numPrimitivesWithoutNormals; ++i)
    {
        newNormalPtr[0] = 0.;
        newNormalPtr[1] = 0.;
        newNormalPtr[2] = 1.;
        newNormalPtr   += 3;
    }

    //
    // Iterate over our newly generated polys, and create normals.
    // At this point, all triangle strips have been decomposed into
    // polys, so this is all we need to worry about.
    //
    vtkPoints *inPts = input->GetPoints();
    vtkIdType nPts   = 0;
    const vtkIdType *pts = nullptr;
    auto iter = vtk::TakeSmartPointer(outPolys->NewIterator());
    for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal(); iter->GoToNextCell())
    {
        iter->GetCurrentCell(nPts, pts);
        double p0[3], p1[3], p2[3];
        double normal[3] = {0, 0, 0};
        if (nPts == 3)
        {
            inPts->GetPoint(pts[0], p0);
            inPts->GetPoint(pts[1], p1);
            inPts->GetPoint(pts[2], p2);
            vtkTriangle::ComputeNormalDirection(p0, p1, p2, normal);
        }
        else
        {
            // Accumulate the normals calculated from every adjacent edge pair.
            inPts->GetPoint(pts[0],p1);
            inPts->GetPoint(pts[1],p2);

            double ax, ay, az, bx, by, bz;
            for (int j = 0 ; j < nPts ; j++)
            {
                p0[0] = p1[0]; p0[1] = p1[1]; p0[2] = p1[2];
                p1[0] = p2[0]; p1[1] = p2[1]; p1[2] = p2[2];
                inPts->GetPoint(pts[(j+2) % nPts],p2);

                ax = p2[0] - p1[0]; ay = p2[1] - p1[1]; az = p2[2] - p1[2];
                bx = p0[0] - p1[0]; by = p0[1] - p1[1]; bz = p0[2] - p1[2];

                normal[0] += (ay * bz - az * by);
                normal[1] += (az * bx - ax * bz);
                normal[2] += (ax * by - ay * bx);
            }
            normal[0] /= nPts;
            normal[1] /= nPts;
            normal[2] /= nPts;
        }

        // Calculate the length, and throw out degenerate cases.
        double nx = normal[0];
        double ny = normal[1];
        double nz = normal[2];
        double length = sqrt(nx*nx + ny*ny + nz*nz);

        if (length != 0)
        {
            newNormalPtr[0] = (float)(nx/length);
            newNormalPtr[1] = (float)(ny/length);
            newNormalPtr[2] = (float)(nz/length);
        }
        else
        {
            newNormalPtr[0] = 0.f;
            newNormalPtr[1] = 0.f;
            newNormalPtr[2] = 1.f;
        }
        newNormalPtr += 3;
    }

    output->GetCellData()->SetNormals(newNormals);
    newNormals->Delete();
}


// ****************************************************************************
//  Method:  vtkVisItPolyDataNormals::TransferCellData
//
//  Purpose:
//    Transfer cell data from our input data to our output. In the
//    simplest cases, this is merely a copy. When triangle strips are
//    present, we need to decompose them.
//
//  Arguments:
//    input     Our input poly data.
//    output    Our output poly data.
//
//  Programmer:  Alister Maguire
//  Creation:    Wed Apr  1 13:39:03 PDT 2020
//
//  Modifications:
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK: use vktCellArrayIterator.
//
// ****************************************************************************

void
vtkVisItPolyDataNormals::TransferCellData(vtkPolyData *input, vtkPolyData *output)
{
    //
    // Gather the data arrays we'll need for later.
    //
    vtkCellArray *inPolys  = input->GetPolys();
    int nPolys             = inPolys->GetNumberOfCells();

    vtkCellArray *inStrips = input->GetStrips();
    int nStrips            = inStrips->GetNumberOfCells();

    vtkCellData *outCD     = output->GetCellData();
    vtkCellData *inCD      = input->GetCellData();
    vtkCellArray *outPolys = vtkCellArray::New();

    //
    // The cell arrays have a specific structure: vertices and lines come
    // first, then polygons, then triangle strips.
    //
    int numPrimitivesWithoutNormals = 0;
    numPrimitivesWithoutNormals    += input->GetVerts()->GetNumberOfCells();
    numPrimitivesWithoutNormals    += input->GetLines()->GetNumberOfCells();
    int nNonStripCells              = nPolys + numPrimitivesWithoutNormals;

    //
    // If we have triangle strips, we need to decompose them into
    // triangles and extend our cell array so that we can compute
    // proper normals.
    //
    if (nStrips > 0)
    {
        outCD->CopyAllocate(inCD);

        if (nPolys > 0)
        {
            //
            // We have polys AND strips. Let's first copy over
            // the polys.
            //
            outPolys->DeepCopy(inPolys);
            vtkIdList *ids = vtkIdList::New();
            ids->SetNumberOfIds(nPolys);
            for (vtkIdType i = numPrimitivesWithoutNormals; i < nPolys; i++)
            {
                ids->SetId(i, i);
            }
            outCD->CopyData(inCD, ids, ids);
            ids->Delete();
        }
        else
        {
            //
            // We only have strips. We'll likely need more space then this,
            // but our VTK version doesn't have AllocateEstimate. This is
            // a starting point.
            //
            //TODO: change to AllocateEstimate after VTK upgrade.
            outPolys->Allocate(nStrips);
        }

        vtkIdType nTriPts    = 0;
        const vtkIdType *triPts    = NULL;
        vtkIdType inCellIdx  = nNonStripCells;
        vtkIdType outCellIdx = nNonStripCells;

        //
        // Decompose our triangle strips, append the resulting trianlges
        // to outPolys, and copy over the cell data.
        //
        auto stripIter = vtk::TakeSmartPointer(inStrips->NewIterator());
        for (stripIter->GoToFirstCell(); !stripIter->IsDoneWithTraversal(); stripIter->GoToNextCell(),
             inCellIdx++)
        {
            stripIter->GetCurrentCell(nTriPts, triPts);
            vtkTriangleStrip::DecomposeStrip(nTriPts, triPts, outPolys);

            for (vtkIdType i = 0; i < nTriPts - 2; ++i)
            {
                outCD->CopyData(inCD, inCellIdx, outCellIdx++);
            }
        }

        //
        // Since we're manually copying everything, grab the verts and lines
        // here.
        //
        for (vtkIdType i = 0; i < numPrimitivesWithoutNormals; ++i)
        {
            outCD->CopyData(inCD, i, i);
        }

        //
        // Check if we've split cells; VisIt might need this information.
        //
        if (outCellIdx > nNonStripCells)
        {
            StripsHaveBeenDecomposed = true;
        }
    }
    else
    {
        //
        // No strips. Just copy everything over.
        //
        outCD->ShallowCopy(inCD);
        outPolys->DeepCopy(inPolys);
    }

    //
    // We can copy all of this data straight over.
    //
    output->SetPolys(outPolys);
    output->SetVerts(input->GetVerts());
    output->SetLines(input->GetLines());
    outPolys->Delete();
}
