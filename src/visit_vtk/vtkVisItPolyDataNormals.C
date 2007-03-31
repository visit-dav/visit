#include "vtkVisItPolyDataNormals.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkTriangle.h>

vtkCxxRevisionMacro(vtkVisItPolyDataNormals, "$Revision: 1.00 $");
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
// ****************************************************************************
vtkVisItPolyDataNormals::vtkVisItPolyDataNormals()
{
    FeatureAngle        = 45.0;
    Splitting           = true;
    ComputePointNormals = true;
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
// ****************************************************************************
void
vtkVisItPolyDataNormals::Execute()
{
    if (ComputePointNormals)
    {
        // Point normals
        if (Splitting)
        {
            ExecutePointWithSplitting();
        }
        else
        {
            ExecutePointWithoutSplitting();
        }
    }
    else
    {
        // Cell normals
        ExecuteCell();
    }
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
// ****************************************************************************
void
vtkVisItPolyDataNormals::ExecutePointWithoutSplitting()
{
    int i;

    // Get all the input and output objects we'll need to reference
    vtkPolyData  *input = GetInput();
    vtkCellArray *inCA  = input->GetPolys();
    vtkPointData *inPD  = input->GetPointData();
    vtkCellData  *inCD  = input->GetCellData();
    vtkPoints    *inPts = input->GetPoints();

    int nCells  = inCA->GetNumberOfCells();
    int nPoints = input->GetNumberOfPoints();

    vtkPolyData *output = GetOutput();
    vtkPointData *outPD = output->GetPointData();
    vtkCellData  *outCD = output->GetCellData();

    // Pass through things which will be unchanged
    output->SetFieldData(input->GetFieldData());

    // Allocate and copy the output points; there will be no extras
    vtkPoints *outPts = vtkPoints::New();
    outPts->SetNumberOfPoints(nPoints);
    outPD->CopyAllocate(inPD,nPoints);
    int ptIdx = 0;
    for (i = 0 ; i < nPoints ; i++)
    {
        float pt[3];
        inPts->GetPoint(i, pt);
        outPts->SetPoint(ptIdx, pt);
        outPD->CopyData(inPD, i, ptIdx);
        ptIdx++;
    }
    output->SetPoints(outPts);
    outPts->Delete();

    // Create and initialize the normals array
    vtkFloatArray *newNormals;
    newNormals = vtkFloatArray::New();
    newNormals->SetNumberOfComponents(3);
    newNormals->SetNumberOfTuples(nPoints);
    newNormals->SetName("Normals");
    float *newNormalPtr = (float*)newNormals->GetVoidPointer(0);
    for (i = 0 ; i < nPoints ; i++)
    {
        newNormalPtr[i*3+0] = 0.;
        newNormalPtr[i*3+1] = 0.;
        newNormalPtr[i*3+2] = 0.;
    }

    // Create the output cells, accumulating cell normals to the points
    output->Allocate(inCA->GetNumberOfConnectivityEntries());
    outCD->CopyAllocate(inCD, nCells);
    vtkIdType *connPtr = inCA->GetPointer();
    for (i = 0 ; i < nCells ; i++)
    {
        outCD->CopyData(inCD, i, i);
        int nVerts = *connPtr++;
        if (nVerts == 3)
        {
            output->InsertNextCell(VTK_TRIANGLE, 3,
                                   connPtr);
        }
        else if (nVerts == 4)
        {
            output->InsertNextCell(VTK_QUAD, 4,
                                   connPtr);
        }
        else
        {
            output->InsertNextCell(VTK_POLYGON, nVerts,
                                   connPtr);
        }

        //
        // Technically, we can use the first three vertices only,
        // (i.e. vtkTriangle::ComputeNormal), but this is not a big
        // hit, and it accomodates for degenerate quads.
        //
        float normal[3];
        vtkPolygon::ComputeNormal(inPts, nVerts, connPtr, normal);

        for (int j = 0 ; j < nVerts ; j++)
        {
            int p = connPtr[j];
            newNormalPtr[p*3+0] += normal[0];
            newNormalPtr[p*3+1] += normal[1];
            newNormalPtr[p*3+2] += normal[2];
        }

        // Increment our connectivity pointer
        connPtr += nVerts;
    }

    // Renormalize the normals; ther've only been accumulated so far
    for (i = 0 ; i < nPoints ; i++)
    {
        vtkMath::Normalize(&newNormalPtr[i*3]);
    }

    outPD->SetNormals(newNormals);
    newNormals->Delete();

    // copy the original vertices and lines to the output
    output->SetVerts(input->GetVerts());
    output->SetLines(input->GetLines());
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
// ****************************************************************************
struct NormalEntry
{
    int          oldId;  // The id of this point before it was split
    int          newId;  // The new id of this node
    float        n[3];   // The accumulated normal
    float        nn[3];  // The normalized normal
    NormalEntry *next;   // The next entry in the linked list for this oldId

    // Constructor
    NormalEntry() : oldId(-1), newId(-1), next(NULL)
    {
        // No need to initialize n or nn.
    }
};

// These #defines are good for up to 4 million feature-edge nodes
// along the surface of an object.  I really hope this will last
// us for a while....
#define MAX_NORMAL_LISTS 1024
#define NORMAL_LIST_LEN  4096

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
// ****************************************************************************
class NormalList
{
  public:
    NormalEntry *normals;

    // Constructor
    NormalList(int initialSize)
    {
        originalSize = initialSize;
        normals = new NormalEntry[initialSize];

        poolListIndex = 0;
        poolIndex = 0;
        pool[poolListIndex] = new NormalEntry[NORMAL_LIST_LEN];
    }

    // Destructor
    ~NormalList()
    {
        delete[] normals;
        for (int i=0; i<=poolListIndex; i++)
            delete[] pool[i];
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
    NormalEntry  *pool[MAX_NORMAL_LISTS];
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
// ****************************************************************************
void
vtkVisItPolyDataNormals::ExecutePointWithSplitting()
{
    int i, j;

    // Get all the input and output objects we'll need to reference
    vtkPolyData  *input = GetInput();
    vtkCellArray *inCA  = input->GetPolys();
    vtkPointData *inPD  = input->GetPointData();
    vtkCellData  *inCD  = input->GetCellData();
    vtkPoints    *inPts = input->GetPoints();

    int nCells  = inCA->GetNumberOfCells();
    int nPoints = input->GetNumberOfPoints();

    vtkPolyData *output = GetOutput();
    vtkPointData *outPD = output->GetPointData();
    vtkCellData  *outCD = output->GetCellData();

    // Pass through things which will be unchanged
    output->SetFieldData(input->GetFieldData());

    // Precalculate the cosine of the feature angle
    float cosAngle = cos(FeatureAngle * 3.1415926536 / 180.);

    // This is the data structure we will rely on to hold
    // out intermediate normals calculations and to allocate
    // the duplicated points (where a feature edge was found).
    NormalList normalList(nPoints);

    output->Allocate(inCA->GetNumberOfConnectivityEntries());
    outCD->CopyAllocate(inCD, nCells);
    vtkIdType *connPtr = inCA->GetPointer();
    vtkIdType cell[100];

    int newPointIndex = nPoints;
    for (i = 0 ; i < nCells ; i++)
    {
        outCD->CopyData(inCD, i, i);
        int nVerts;
        nVerts = *connPtr++;

        // Extract the cell vertices
        for (j = 0 ; j < nVerts ; j++)
        {
            cell[j] = *connPtr++;
        }

        //
        // Technically, we can always use only the first three vertices, but
        // it is not a big hit to do the quads better, and it accomodates for
        // degenerate quads directly.  The code is the same algorithm as
        // vtkPolygon::ComputeNormal, but without the built-in renormalization.
        //
        float v0[3], v1[3], v2[3];
        float normal[3] = {0, 0, 0};
        if (nVerts == 3)
        {
            inPts->GetPoint(cell[0], v0);
            inPts->GetPoint(cell[1], v1);
            inPts->GetPoint(cell[2], v2);
            vtkTriangle::ComputeNormalDirection(v0, v1, v2, normal);
        }
        else
        {
            // Accumulate the normals calculated from every adjacent edge pair.
            inPts->GetPoint(cell[0],v1);
            inPts->GetPoint(cell[1],v2);
            
            float ax, ay, az, bx, by, bz;
            for (j = 0 ; j < nVerts ; j++) 
            {
                v0[0] = v1[0]; v0[1] = v1[1]; v0[2] = v1[2];
                v1[0] = v2[0]; v1[1] = v2[1]; v1[2] = v2[2];
                inPts->GetPoint(cell[(j+2) % nVerts],v2);

                ax = v2[0] - v1[0]; ay = v2[1] - v1[1]; az = v2[2] - v1[2];
                bx = v0[0] - v1[0]; by = v0[1] - v1[1]; bz = v0[2] - v1[2];

                normal[0] += (ay * bz - az * by);
                normal[1] += (az * bx - ax * bz);
                normal[2] += (ax * by - ay * bx);
            }
            normal[0] /= nVerts;
            normal[1] /= nVerts;
            normal[2] /= nVerts;
        }

        // Calculate the length, and throw out degenerate cases
        float length = sqrt(normal[0]*normal[0] +
                            normal[1]*normal[1] + 
                            normal[2]*normal[2]);

        if (length == 0) continue;

        // Store the normalized version separately
        float nnormal[3] = {normal[0]/length,
                            normal[1]/length,
                            normal[2]/length};

        // Loop over all points of the cell, deciding if we need
        // to split it or can merge with an old one.  Use the feature
        // angle set before execution.
        for (j = 0 ; j < nVerts ; j++)
        {
            int p = cell[j];
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
                cell[j] = ne->newId;
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
                float *n = ne->n;
                n[0] += normal[0];
                n[1] += normal[1];
                n[2] += normal[2];

                float newlength = sqrt(n[0]*n[0] + n[1]*n[1] +  n[2]*n[2]);

                float *nn = ne->nn;
                nn[0] = n[0]/newlength;
                nn[1] = n[1]/newlength;
                nn[2] = n[2]/newlength;
            }
            else // no match found; duplicate the point
            {
                ne->next = normalList.GetNewEntry();
                ne = ne->next;

                cell[j] = newPointIndex;
                ne->oldId = p;
                ne->newId = newPointIndex;
                ne->n[0] = normal[0];
                ne->n[1] = normal[1];
                ne->n[2] = normal[2];
                ne->nn[0]= nnormal[0];
                ne->nn[1]= nnormal[1];
                ne->nn[2]= nnormal[2];

                newPointIndex++;
            }
        }

        // Now add the cell
        if (nVerts == 3)
        {
            output->InsertNextCell(VTK_TRIANGLE, 3, cell);
        }
        else if (nVerts == 4)
        {
            output->InsertNextCell(VTK_QUAD, 4, cell);
        }
        else
        {
            output->InsertNextCell(VTK_POLYGON, nVerts, cell);
        }
    }

    // Create the output points array
    int nOutPts = normalList.GetTotalNumberOfEntries();
    vtkPoints *outPts = vtkPoints::New();
    outPts->SetNumberOfPoints(nOutPts);
    outPD->CopyAllocate(inPD,nOutPts);

    // Create the normals array
    vtkFloatArray *newNormals;
    newNormals = vtkFloatArray::New();
    newNormals->SetNumberOfComponents(3);
    newNormals->SetNumberOfTuples(nOutPts);
    newNormals->SetName("Normals");
    float *newNormalPtr = (float*)newNormals->GetVoidPointer(0);

    // Add all the original points and normals
    for (i = 0 ; i < nPoints ; i++)
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
    for (i = 0 ; i < nLists ; i++)
    {
        NormalEntry *list = normalList.GetExtraList(i);
        for (j = 0 ; j < NORMAL_LIST_LEN ; j++)
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

    // copy the original vertices and lines to the output
    output->SetVerts(input->GetVerts());
    output->SetLines(input->GetLines());

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
// ****************************************************************************
void
vtkVisItPolyDataNormals::ExecuteCell()
{
    int i;

    // Get all the input and output objects we'll need to reference
    vtkPolyData  *input = GetInput();
    vtkCellArray *inCA  = input->GetPolys();
    vtkPointData *inPD  = input->GetPointData();
    vtkCellData  *inCD  = input->GetCellData();
    vtkPoints    *inPts = input->GetPoints();

    int nCells  = inCA->GetNumberOfCells();
    int nPoints = input->GetNumberOfPoints();

    vtkPolyData *output = GetOutput();
    vtkPointData *outPD = output->GetPointData();
    vtkCellData  *outCD = output->GetCellData();

    // Pass through things which will be unchanged
    output->SetFieldData(input->GetFieldData());

    // Allocate and copy the output points; there will be no extras
    vtkPoints *outPts = vtkPoints::New();
    outPts->SetNumberOfPoints(nPoints);
    outPD->CopyAllocate(inPD,nPoints);
    int ptIdx = 0;
    for (i = 0 ; i < nPoints ; i++)
    {
        float pt[3];
        inPts->GetPoint(i, pt);
        outPts->SetPoint(ptIdx, pt);
        outPD->CopyData(inPD, i, ptIdx);
        ptIdx++;
    }
    output->SetPoints(outPts);
    outPts->Delete();

    // Create the normals array
    vtkFloatArray *newNormals;
    newNormals = vtkFloatArray::New();
    newNormals->SetNumberOfComponents(3);
    newNormals->SetNumberOfTuples(nCells);
    newNormals->SetName("Normals");
    float *newNormalPtr = (float*)newNormals->GetVoidPointer(0);

    // Create the output cell array
    output->Allocate(inCA->GetNumberOfConnectivityEntries());
    outCD->CopyAllocate(inCD, nCells);
    vtkIdType *connPtr = inCA->GetPointer();
    for (i = 0 ; i < nCells ; i++)
    {
        outCD->CopyData(inCD, i, i);
        int nVerts = *connPtr++;
        if (nVerts == 3)
        {
            output->InsertNextCell(VTK_TRIANGLE, 3,
                                   connPtr);
        }
        else if (nVerts == 4)
        {
            output->InsertNextCell(VTK_QUAD, 4,
                                   connPtr);
        }
        else
        {
            output->InsertNextCell(VTK_POLYGON, nVerts,
                                   connPtr);
        }

        //
        // Technically, we can use the first three vertices only,
        // (i.e. vtkTriangle::ComputeNormal), but this is not a big
        // hit, and it accomodates for degenerate quads.
        //
        vtkPolygon::ComputeNormal(inPts, nVerts, connPtr, &newNormalPtr[i*3]);
        connPtr += nVerts;
    }
        
    outCD->SetNormals(newNormals);
    newNormals->Delete();
}
