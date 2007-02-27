/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "vtkVisItPolyDataNormals.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
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
int
vtkVisItPolyDataNormals::RequestData(vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    // get the input and output
    vtkPolyData *input = vtkPolyData::SafeDownCast(
        inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkPolyData *output = vtkPolyData::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));

    if (ComputePointNormals)
    {
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
// ****************************************************************************
void
vtkVisItPolyDataNormals::ExecutePointWithoutSplitting(
  vtkPolyData *input, vtkPolyData *output)
{
    int i;

    // Get all the input and output objects we'll need to reference
    vtkCellArray *inCA  = input->GetPolys();
    vtkPointData *inPD  = input->GetPointData();
    vtkCellData  *inCD  = input->GetCellData();
    vtkPoints    *inPts = input->GetPoints();

    int nCells  = inCA->GetNumberOfCells();
    int nOtherCells = input->GetVerts()->GetNumberOfCells() + 
                      input->GetLines()->GetNumberOfCells();
    int nTotalCells = nCells + nOtherCells;

    int nPoints = input->GetNumberOfPoints();

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
    for (i = 0 ; i < nPoints ; i++)
    {
        dnormals[i*3+0] = 0.;
        dnormals[i*3+1] = 0.;
        dnormals[i*3+2] = 0.;
    }

    // Create the output cells, accumulating cell normals to the points
    output->Allocate(inCA->GetNumberOfConnectivityEntries());
    outCD->CopyAllocate(inCD, nTotalCells);

    vtkIdType *connPtr = inCA->GetPointer();
    for (i = 0 ; i < nCells ; i++)
    {
        outCD->CopyData(inCD, nOtherCells+i, nOtherCells+i);
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
        // hit, and it accomodates for degenerate quads.  Note -- this
        // will probably fail for very large/small polygons because of
        // over/underflow.  If so, we need to switch to double precision
        // math and avoid using the VTK code.
        //
        double normal[3];
        vtkPolygon::ComputeNormal(inPts, nVerts, connPtr, normal);

        for (int j = 0 ; j < nVerts ; j++)
        {
            int p = connPtr[j];
            dnormals[p*3+0] += normal[0];
            dnormals[p*3+1] += normal[1];
            dnormals[p*3+2] += normal[2];
        }

        // Increment our connectivity pointer
        connPtr += nVerts;
    }

    // Renormalize the normals; they've only been accumulated so far,
    // and store in the vtkFloatArray.
    float *newNormalPtr = (float*)newNormals->GetPointer(0);
    for (i = 0 ; i < nPoints ; i++)
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

    // copy the original vertices and lines to the output
    output->SetVerts(input->GetVerts());
    output->SetLines(input->GetLines());

    // copy the data from the lines and vertices now.
    for (i = 0 ; i < nOtherCells ; i++)
        outCD->CopyData(inCD, i, i);
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
// ****************************************************************************
void
vtkVisItPolyDataNormals::ExecutePointWithSplitting(vtkPolyData *input,
                                                   vtkPolyData *output)
{
    int i, j;

    // Get all the input and output objects we'll need to reference
    vtkCellArray *inCA  = input->GetPolys();
    vtkPointData *inPD  = input->GetPointData();
    vtkCellData  *inCD  = input->GetCellData();
    vtkPoints    *inPts = input->GetPoints();

    int nCells  = inCA->GetNumberOfCells();
    int nOtherCells = input->GetVerts()->GetNumberOfCells() +
                      input->GetLines()->GetNumberOfCells();
    int nTotalCells = nCells + nOtherCells;

    int nPoints = input->GetNumberOfPoints();

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

    outCD->CopyAllocate(inCD, nTotalCells);
    vtkIdTypeArray *list = vtkIdTypeArray::New();
    list->SetNumberOfValues(inCA->GetNumberOfConnectivityEntries());
    vtkIdType *nl = list->GetPointer(0);
    vtkIdType *connPtr = inCA->GetPointer();
    vtkIdType *cell = NULL;

    int newPointIndex = nPoints;
    for (i = 0 ; i < nCells ; i++)
    {
        outCD->CopyData(inCD, i+nOtherCells, i+nOtherCells);
        int nVerts;
        nVerts = *connPtr++;

        // Extract the cell vertices
        *nl++ = nVerts;
        cell = nl;
        for (j = 0 ; j < nVerts ; j++)
        {
            *nl++ = *connPtr++;
        }

        //
        // Technically, we can always use only the first three vertices, but
        // it is not a big hit to do the quads better, and it accomodates for
        // degenerate quads directly.  The code is the same algorithm as
        // vtkPolygon::ComputeNormal, but changed to make it work better.
        //
        double v0[3], v1[3], v2[3];
        double normal[3] = {0, 0, 0};
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
            
            double ax, ay, az, bx, by, bz;
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
        double nx = normal[0];
        double ny = normal[1];
        double nz = normal[2];
        double length = sqrt(nx*nx + ny*ny + nz*nz);

        if (length == 0) continue;

        // Store the normalized version separately
        float nnormal[3] = {nx/length,
                            ny/length,
                            nz/length};

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

                double nx = n[0];
                double ny = n[1];
                double nz = n[2];
                double newlength = sqrt(nx*nx + ny*ny + nz*nz);

                float *nn = ne->nn;
                if (newlength != 0.0)
                {
                    nn[0] = float(double(n[0])/newlength);
                    nn[1] = float(double(n[1])/newlength);
                    nn[2] = float(double(n[2])/newlength);
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
    }

    vtkCellArray *polys = vtkCellArray::New();
    polys->SetCells(nCells, list);
    list->Delete();
    output->SetPolys(polys);
    polys->Delete();

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
    float *newNormalPtr = (float*)newNormals->GetPointer(0);

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

    // copy the data from the lines and vertices now.
    for (i = 0 ; i < nOtherCells ; i++)
        outCD->CopyData(inCD, i, i);
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
// ****************************************************************************
void
vtkVisItPolyDataNormals::ExecuteCell(vtkPolyData *input, vtkPolyData *output)
{
    int  i;

    // Get all the input and output objects we'll need to reference
    output->ShallowCopy(input);

    vtkPoints    *inPts = input->GetPoints();

    int nCells  = input->GetNumberOfCells();

    // Create the normals array
    vtkFloatArray *newNormals;
    newNormals = vtkFloatArray::New();
    newNormals->SetNumberOfComponents(3);
    newNormals->SetNumberOfTuples(nCells);
    newNormals->SetName("Normals");
    float *newNormalPtr = (float*)newNormals->GetPointer(0);

    // The verts and lines come before the polys.  So add normals for them.
    int numPrimitivesWithoutNormals = 0;
    numPrimitivesWithoutNormals += input->GetVerts()->GetNumberOfCells();
    numPrimitivesWithoutNormals += input->GetLines()->GetNumberOfCells();
    for (i = 0 ; i < numPrimitivesWithoutNormals ; i++)
    {
        newNormalPtr[0] = 0.;
        newNormalPtr[1] = 0.;
        newNormalPtr[2] = 1.;
        newNormalPtr += 3;
    }

    vtkCellArray *inCA  = input->GetPolys();
    vtkIdType *connPtr = inCA->GetPointer();
    int nPolys = inCA->GetNumberOfCells();
    for (i = 0 ; i < nPolys ; i++)
    {
        //
        // Technically, we can always use only the first three vertices, but
        // it is not a big hit to do the quads better, and it accomodates for
        // degenerate quads directly.  The code is the same algorithm as
        // vtkPolygon::ComputeNormal, but changed to make it work better.
        //
        int nVerts = *connPtr++;
        int *cell = connPtr;

        double v0[3], v1[3], v2[3];
        double normal[3] = {0, 0, 0};
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
            
            double ax, ay, az, bx, by, bz;
            for (int j = 0 ; j < nVerts ; j++) 
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

        //
        // Step through connectivity
        //
        connPtr += nVerts;
    }
        
    // The triangle strips come after the polys.  So add normals for them.
    numPrimitivesWithoutNormals = 0;
    numPrimitivesWithoutNormals += input->GetStrips()->GetNumberOfCells();
    for (i = 0 ; i < numPrimitivesWithoutNormals ; i++)
    {
        newNormalPtr[0] = 0.f;
        newNormalPtr[1] = 0.f;
        newNormalPtr[2] = 1.f;
        newNormalPtr += 3;
    }

    output->GetCellData()->SetNormals(newNormals);
    newNormals->Delete();
}
