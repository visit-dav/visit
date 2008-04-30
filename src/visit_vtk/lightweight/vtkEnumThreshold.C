/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkThreshold.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkEnumThreshold.h"

#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkRectilinearGrid.h"
#include "vtkPolyData.h"
#include "vtkUniformGrid.h"
#include "vtkStructuredPoints.h"
#include "vtkVertex.h"

#include "BJHash.h"

#include <float.h>

#include <map>
#include <list>
#include <vector>

using std::map;
using std::vector;
using std::list;

// function to compute bi-nomial coefficient ('n-choose-r')
// without computing really large, intermediate factorials
static double
choose(int n, int r)
{
    if (n == 0)
    {
        if (r == 0)
            return 1;
        else
            return 0;
    }

    double result = 1;
    int k = 1;
    for (int i = r+1; i <=n; i++)
    {
        result *= (double) i;
        if (k <= n-r)
        {
            result /= (double) k;
            k++;
        }
    }
    return result;
}

// function to convert a single (possibly large) number representing
// an index into all possible combinations of r items from n things
// for values of r from 1 to maxr into the actual list of items
static void
ComboDigitsFromId(double id, int n, int maxr, const vector<vector<int> > &ptMap, list<int> &digits)
{
    int row, col, grp;

    for (grp = 1; grp <= maxr; grp++)
    {
        double grpc = choose(n,grp);
        if (id < grpc)
            break;
        else
            id -= grpc;
    }
    if (grp == maxr+1)
        return;

    int rowmin = 0;
    for (col = grp-1; col >= 0; col--)
    {
        for (row = rowmin; row < n; row++)
        {
            int seglen = ptMap[row][col];
            if (id < seglen)
            {
                rowmin = row+1;
                digits.push_back(row);
                break;
            }
            id -= seglen;
        }
    }
}


vtkCxxRevisionMacro(vtkEnumThreshold, "$Revision: 1.68 $");
vtkStandardNewMacro(vtkEnumThreshold);

//  Modifications:  
//    Jeremy Meredith, Tue Aug 22 16:20:41 EDT 2006
//    Taken from 5.0.0.a vtkThreshold source, renamed to vtkEnumThreshold, and
//    made it work on an enumerated scalar selection.
//
//    Mark C. Miller, Tue Mar 18 14:55:30 PDT 2008
//    Added alwaysExclude and alwaysInclude values
vtkEnumThreshold::vtkEnumThreshold()
{
    // by default process active point scalars
    this->SetInputArrayToProcess(0,0,0,
                                 vtkDataObject::FIELD_ASSOCIATION_POINTS_THEN_CELLS,
                                 vtkDataSetAttributes::SCALARS);
    minEnumerationValue = +DBL_MAX;
    maxEnumerationValue = -DBL_MAX;
    alwaysExcludeMin = +DBL_MAX;
    alwaysExcludeMax = -DBL_MAX;
    alwaysIncludeMin = +DBL_MAX;
    alwaysIncludeMax = -DBL_MAX;
    partialCellMode = Exclude;
    enumMode = ByRange;
    lastRangeBin = -1;
    enumerationMap = 0;
    pascalsTriangleN = -1;
    pascalsTriangleR = -1;
    returnEmptyIfAllCellsKept = false;
    allCellsKeptInLastRequestData = false;
    selectedEnumMask = 0; 
}

vtkEnumThreshold::~vtkEnumThreshold()
{
}

// Need for argument to qsort
static int CompareIds(const void *a, const void *b)
{
    vtkIdType *pa = (vtkIdType *) a;
    vtkIdType *pb = (vtkIdType *) b;
    if (*pa < *pb)
        return -1;
    else if (*pa > *pb)
        return 1;
    else
        return 0;
}

//
// Given a vtkCell object, compute a hash for it using a
// buffer of sorted node ids of the cell. Returned the
// hash key and the sorted list of node ids.
//
static void HashCell(vtkCell *theCell, unsigned int &hashVal,
    vector<vtkIdType> &pids)
{
    int npts = theCell->GetNumberOfPoints();

    for (int i = 0; i < npts; i++)
        pids.push_back(theCell->GetPointId(i));

    // we sort these to prevent node-order from effecting
    // the hash value
    qsort(&pids[0], npts, sizeof(vtkIdType), CompareIds);

    hashVal = BJHash::Hash((unsigned char*)&pids[0],
        npts * sizeof(vtkIdType), 0xdeadbeef);
}

//
// Check if the given cell is already in a map by hashing
// the cell and looking it up in the map. Take care for
// possibility (though really, really small) of hash collisions.
//
static bool AlreadyAddedCell(vtkCell *theCell,
    const map<unsigned int, vector<vtkIdType> > &theMap)
{
    unsigned int hval;
    vector<vtkIdType> pids;
    HashCell(theCell, hval, pids);

    map<unsigned int, vector<vtkIdType> >::const_iterator
        cellPtIdsAtKey = theMap.find(hval);

    if (cellPtIdsAtKey == theMap.end())
        return false;
    else
    {
        // We could have collisions. So, confirm the input cell's
        // point ids match the point ids of one of the cells we've
        // got stored at this key.
        const vector<vtkIdType> fpids = cellPtIdsAtKey->second;
        for (int i = 0; i < fpids.size(); i += fpids[i]+1)
        {
            bool haveMatch = true;
            for (int j = 0; haveMatch && (j < fpids[i]); j++)
            {
                if (fpids[i+1+j] != pids[j])
                    haveMatch = false;
            }

            if (haveMatch)
                return true;
        }
    }
    return false;
}

//
// Check to see if a given node is already in the nodeMap.
// Note, since node ids are already guarenteed to be unique
// and a node is wholly defined by just one vtkIdType, this
// is pretty simple.
//
static bool AlreadyAddedNode(vtkIdType ptId, 
    const map<vtkIdType, unsigned char> &theMap)
{
    return theMap.find(ptId) != theMap.end();
}

//
// Add the specified node id to the node map
//
static void AddNodeToMaps(vtkIdType ptId,
    map<vtkIdType, unsigned char> &nodeMap)
{
    // node ids are guarenteed to be unique
    nodeMap[ptId] = 0x1;
}

//
// Add the given edge cell to the edge map and
// also its nodes to the node map.
//
static void AddEdgeToMaps(vtkCell *edgeCell,
    map<unsigned int, vector<int> > &edgeMap,
    map<vtkIdType, unsigned char> &nodeMap)
{
    int i;
    unsigned int hval;
    vector<vtkIdType> pids;
    HashCell(edgeCell, hval, pids);

    // put this edge in the edge map
    edgeMap[hval].push_back((vtkIdType) pids.size());
    for (i = 0; i < pids.size(); i++)
        edgeMap[hval].push_back(pids[i]);

    // put this edge's nodes in the node map
    for (int i = 0; i < pids.size(); i++)
        AddNodeToMaps(pids[i], nodeMap);
}

//
// Add the given face cell to the face map, its
// edges to the edge map and its nodes to the node
// map.
//
static void AddFaceToMaps(vtkCell *faceCell,
    map<unsigned int, vector<int> > &faceMap,
    map<unsigned int, vector<int> > &edgeMap,
    map<vtkIdType, unsigned char> &nodeMap)
{
    int i;
    unsigned int hval;
    vector<vtkIdType> pids;
    HashCell(faceCell, hval, pids);

    // put this face in the face map
    faceMap[hval].push_back((vtkIdType) pids.size());
    for (i = 0; i < pids.size(); i++)
        faceMap[hval].push_back(pids[i]);

    // put this face's edges in the edge map
    for (i = 0; i < faceCell->GetNumberOfEdges(); i++)
    {
        vtkCell *edgeCell = faceCell->GetEdge(i);
        AddEdgeToMaps(edgeCell, edgeMap, nodeMap);
    }

    // put this face's nodes in the node map
    for (i = 0; i < pids.size(); i++)
        AddNodeToMaps(pids[i], nodeMap);
}

//
// Macro to add cell and its points to the output mesh
// There were too many internal variables in the main loop of the
// algorithm to make a nice function.
//
#define ADD_CELL_POINTS_AND_CELL(theCell)                                       \
    for (int kk=0; kk < theCell->GetNumberOfPoints(); kk++)                     \
    {                                                                           \
        ptId = theCell->GetPointIds()->GetId(kk);                               \
        if ( (newId = pointMap->GetId(ptId)) < 0 )                              \
        {                                                                       \
            input->GetPoint(ptId, x);                                           \
            newId = newPoints->InsertNextPoint(x);                              \
            pointMap->SetId(ptId,newId);                                        \
            outPD->CopyData(pd,ptId,newId);                                     \
        }                                                                       \
        newCellPts->InsertId(kk,newId);                                         \
    }                                                                           \
    newCellId = output->InsertNextCell(theCell->GetCellType(),newCellPts);      \
    outCD->CopyData(cd,cellId,newCellId);                                       \
    newCellPts->Reset();

//  Modifications:  
//    Jeremy Meredith, Tue Aug 22 16:20:41 EDT 2006
//    Taken from 5.0.0.a vtkThreshold source, renamed to vtkEnumThreshold, and
//    made it work on an enumerated scalar selection.
int vtkEnumThreshold::RequestData(
                              vtkInformation *vtkNotUsed(request),
                              vtkInformationVector **inputVector,
                              vtkInformationVector *outputVector)
{
    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    // get the input and ouptut
    vtkDataSet *input = vtkDataSet::SafeDownCast(
                                                 inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
                                                                    outInfo->Get(vtkDataObject::DATA_OBJECT()));

    vtkIdType cellId, newCellId;
    vtkIdList *cellPts, *pointMap;
    vtkIdList *newCellPts;
    vtkCell *cell;
    vtkPoints *newPoints;
    int i, ptId, newId, numPts;
    int numCellPts;
    double x[3];
    vtkPointData *pd=input->GetPointData(), *outPD=output->GetPointData();
    vtkCellData *cd=input->GetCellData(), *outCD=output->GetCellData();
    int keepCell, usePointScalars;

    vtkDebugMacro(<< "Executing threshold filter");

    allCellsKeptInLastRequestData = true;
  
    vtkDataArray *inScalars = this->GetInputArrayToProcess(0,inputVector);
  
    if (!inScalars)
    {
        vtkDebugMacro(<<"No scalar data to threshold");
        return 1;
    }

    outPD->CopyAllocate(pd);
    outCD->CopyAllocate(cd);

    numPts = input->GetNumberOfPoints();
    output->Allocate(input->GetNumberOfCells());
    newPoints = vtkPoints::New();
    newPoints->Allocate(numPts);

    pointMap = vtkIdList::New(); //maps old point ids into new
    pointMap->SetNumberOfIds(numPts);
    for (i=0; i < numPts; i++)
    {
        pointMap->SetId(i,-1);
    }

    newCellPts = vtkIdList::New();     

    //
    // If we dissect cells, we use these maps for what amounts to
    // unlimited in size hashes. Every unique key will get its own
    // entry in the map. So, a map can grow in size to the total
    // number of faces/edges/nodes we've visited during cell dissection
    // (note that this is often much, much smaller than the total number
    // of faces/edges/nodes in the mesh). However, the objects we hash
    // are NOT guarenteed to result in unique keys. So, we can maybe get
    // collisions.  Although this is highly unlikely for any small set
    // of dissected faces/edges/nodes, we do need to accomodate it.
    //
    map<unsigned int, vector<vtkIdType> > faceMap;
    map<unsigned int, vector<vtkIdType> > edgeMap;
    map<vtkIdType, unsigned char> nodeMap;

    // are we using pointScalars?
    usePointScalars = (inScalars->GetNumberOfTuples() == numPts);
  
    // Check that the scalars of each cell satisfy the threshold criterion
    for (cellId=0; cellId < input->GetNumberOfCells(); cellId++)
    {
        cell = input->GetCell(cellId);
        cellPts = cell->GetPointIds();
        numCellPts = cell->GetNumberOfPoints();
    
        if ( usePointScalars )
        {
            if (true) // was "this->allScalars in vtkEnumThreshold
            {
                ptId = cellPts->GetId(0);
                int keepFirst = this->EvaluateComponents( inScalars, ptId );
                keepCell = keepFirst;
                for ( i=1; (keepCell == keepFirst) && (i < numCellPts); i++)
                {
                    ptId = cellPts->GetId(i);
                    keepCell = this->EvaluateComponents( inScalars, ptId );
                }

                //
                // If we have a partial cell, handle that by iterating over
                // faces, then edges and then nodes, keeping as much of the
                // connectivity intact as possible.
                //
                if      (keepCell != keepFirst && partialCellMode == Exclude)
                    keepCell = 0; // turn off whole cell logic below
                else if (keepCell != keepFirst && partialCellMode == Include)
                    keepCell = 1; // turn ON whole cell logic below
                else if (keepCell != keepFirst && partialCellMode == Dissect)
                {
                    keepCell = 0; // turn off whole-cell logic below

                    // iterate over faces
                    for (i = 0; i < cell->GetNumberOfFaces(); i++)
                    {
                        vtkCell *faceCell = cell->GetFace(i);
                        if (AlreadyAddedCell(faceCell, faceMap))
                            continue;

                        int keepFace = 1;
                        for (int j = 0; keepFace && j < faceCell->GetNumberOfPoints(); j++)
                            keepFace = this->EvaluateComponents( inScalars, faceCell->GetPointId(j));

                        if (keepFace) // keep this face
                        {
                            ADD_CELL_POINTS_AND_CELL(faceCell);
                            AddFaceToMaps(faceCell, faceMap, edgeMap, nodeMap);
                        }
                    }

                    // iterate over edges
                    for (i = 0; i < cell->GetNumberOfEdges(); i++)
                    {
                        vtkCell *edgeCell = cell->GetEdge(i);
                        if (AlreadyAddedCell(edgeCell, edgeMap))
                            continue;

                        int keepEdge = 1;
                        for (int j = 0; keepEdge && j < edgeCell->GetNumberOfPoints(); j++)
                            keepEdge = this->EvaluateComponents( inScalars, edgeCell->GetPointId(j));

                        if (keepEdge) // keep this edge 
                        {
                            ADD_CELL_POINTS_AND_CELL(edgeCell);
                            AddEdgeToMaps(edgeCell, edgeMap, nodeMap);
                        }
                    }

                    // iterate over nodes
                    for (i = 0; i < cell->GetNumberOfPoints(); i++)
                    {
                        ptId = cellPts->GetId(i);
                        if (AlreadyAddedNode(ptId, nodeMap))
                            continue;

                        int keepNode = this->EvaluateComponents( inScalars, ptId);

                        if (keepNode) // keep this node 
                        {
                            // Build a temporary vertex cell so it
                            // can be added by the macro.
                            vtkVertex *tmpVert = vtkVertex::New();
                            vtkPoints *dummyPoints = vtkPoints::New();
                            dummyPoints->InsertNextPoint(0.0, 0.0, 0.0);
                            tmpVert->Initialize(1, &ptId, dummyPoints);

                            ADD_CELL_POINTS_AND_CELL(tmpVert);
                            AddNodeToMaps(ptId, nodeMap);

                            dummyPoints->Delete();
                            tmpVert->Delete();
                        } // keepNode
                    } // iterate over nodes
                } // dissect partial cell
            } // true
        } // usePointScalars
        else //use cell scalars
        {
            keepCell = this->EvaluateComponents( inScalars, cellId );
        }

        if (keepCell == 0)
            allCellsKeptInLastRequestData = false;
    
        if (  numCellPts > 0 && keepCell )
        {
            // satisfied thresholding (also non-empty cell, i.e. not VTK_EMPTY_CELL)
            ADD_CELL_POINTS_AND_CELL(cell);

        } // satisfied thresholding
    } // for all cells

    if (faceMap.size() || edgeMap.size() || nodeMap.size())
    {
        int idCount = 0;
        map<unsigned int, vector<vtkIdType> >::iterator it;

        for (it = faceMap.begin(); it != faceMap.end(); it++)
            idCount += it->second.size();
        for (it = edgeMap.begin(); it != edgeMap.end(); it++)
            idCount += it->second.size();
        idCount += nodeMap.size();

        vtkDebugMacro(<< "Stored " << idCount << " node ids (%" 
                      << (100.0 * idCount / numPts) << " of input nodes)"
                      << " in maps for partial cell dissection.");
    }

    vtkDebugMacro(<< "Extracted " << output->GetNumberOfCells() 
                  << " number of cells.");

    // now clean up / update ourselves
    pointMap->Delete();
    newCellPts->Delete();
  
    output->SetPoints(newPoints);
    newPoints->Delete();

    if (returnEmptyIfAllCellsKept && allCellsKeptInLastRequestData)
    {
        vtkDebugMacro(<< "Kept all cells. At caller's request, returning empty ugrid.");
        
        //
        // clear out the output
        //
        output->Reset();
    }

    output->GetFieldData()->PassData(input->GetFieldData());
    output->Squeeze();

    return 1;
}

bool vtkEnumThreshold::IsInEnumerationRanges(double val)
{
    //
    // First, try last range bin
    //
    if (lastRangeBin != -1)
    {
        if (enumerationRanges[2*lastRangeBin  ] <= val &&
            enumerationRanges[2*lastRangeBin+1] >= val)
            return true;
    }

    //
    // Ok, binary search. We divide the size by 2 here to get to
    // the 'end' of the array because it is an array of min/max
    // pairs. This search works because we've sorted the range bins.
    //
    register int bot = 0;
    register int top = enumerationRanges.size() / 2 - 1;
    register int mid;
    while (bot <= top)
    {
        mid = (bot + top) >> 1;

        if (val > enumerationRanges[2*mid+1])
            bot = mid + 1;
        else if (val < enumerationRanges[2*mid])
            top = mid - 1;
        else
        {
            lastRangeBin = mid;
            return true;
        }
    }

    lastRangeBin = -1;
    return false;
}

bool vtkEnumThreshold::HasBitsSetInEnumerationMask(double val)
{
    unsigned long long lval = (unsigned long long) val;

    if (double(lval) != val)
    {
        vtkDebugMacro(<< "Value " << val << " for ByBitMask enumeration mode is too large.");
        return true;
    }

    if (lval & selectedEnumMask)
        return true;

    return false;
}

bool vtkEnumThreshold::HasValuesInEnumerationMap(double val)
{
    list<int> values;
    ComboDigitsFromId(val, pascalsTriangleN, pascalsTriangleR, pascalsTriangleMap, values);
    
    list<int>::iterator it = values.begin();
    for (it = values.begin(); it != values.end(); it++)
    {
        if (enumerationMap[*it])
            return true;
    }

    return false;
}


//  Modifications:  
//    Jeremy Meredith, Tue Aug 22 16:20:41 EDT 2006
//    Taken from 5.0.0.a vtkThreshold source, renamed to vtkEnumThreshold, and
//    made it work on an enumerated scalar selection.
//
//    Mark C. Miller and Jeremy Meredith, Tue Jul 10 08:45:44 PDT 2007
//    Made it work correctly in presence of negative enumeration values.
//
//    Mark C. Miller, Tue Mar 18 14:55:30 PDT 2008
//    Added alwaysExclude and alwaysInclude values
int vtkEnumThreshold::EvaluateComponents( vtkDataArray *scalars, vtkIdType id )
{
    int numComp = scalars->GetNumberOfComponents();

    double val = scalars->GetComponent(id, 0);

    int keepCell = false;
    if      (alwaysExcludeMin <= val && val <= alwaysExcludeMax)
        keepCell = false;
    else if (alwaysIncludeMin <= val && val <= alwaysIncludeMax)
        keepCell = true;
    else if ((enumMode == ByValue || enumMode == ByRange) &&
             (val < minEnumerationValue || maxEnumerationValue < val))
        keepCell = false;
    else
    {
        if (enumMode == ByValue && enumerationMap[int(val-minEnumerationValue)])
            keepCell = true;
        else if (enumMode == ByRange && IsInEnumerationRanges(val))
            keepCell = true;
        else if (enumMode == ByBitMask && HasBitsSetInEnumerationMask(val))
            keepCell = true;
        else if (enumMode == ByNChooseR && HasValuesInEnumerationMap(val))
            keepCell = true;
    }
    return keepCell;
}

int vtkEnumThreshold::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

void vtkEnumThreshold::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//  Modifications:  
//    Jeremy Meredith, Tue Aug 22 16:20:41 EDT 2006
//    Taken from 5.0.0.a vtkThreshold source, renamed to vtkEnumThreshold, and
//    made it work on an enumerated scalar selection.
void vtkEnumThreshold::SetEnumerationRanges(const std::vector<double> &vals)
{
    enumerationRanges = vals;
}

static int CompareRanges(const void *a, const void *b)
{
    double *pa = (double *) a;
    double *pb = (double *) b;

    if (pa[1] < pb[0])
        return -1;
    else if (pa[0] > pb[1])
        return 1;
    else
        return 0;
}

//  Modifications:  
//    Jeremy Meredith, Tue Aug 22 16:20:41 EDT 2006
//    Taken from 5.0.0.a vtkThreshold source, renamed to vtkEnumThreshold, and
//    made it work on an enumerated scalar selection.
//
//    Mark C. Miller, Mon Jul  9 14:35:29 PDT 2007
//    Eliminated early return when value is negative and changed error message
//    for that case. It appears to work with negative values too though it
//    wasn't thoroughly tested.
//
//    Mark C. Miller and Jeremy Meredith, Tue Jul 10 08:45:44 PDT 2007
//    Made it work correctly in presence of negative enumeration values.
void vtkEnumThreshold::SetEnumerationSelection(const std::vector<bool> &sel)
{
    int i;

    switch (enumMode)
    {
        case ByValue:
        case ByBitMask:
        case ByNChooseR:
        {
            int i;
            minEnumerationValue = +DBL_MAX;
            maxEnumerationValue = -DBL_MAX;
            for (i=0; i<enumerationRanges.size(); i += 2)
            {
                if (enumerationRanges[i+0] > maxEnumerationValue)
                    maxEnumerationValue = enumerationRanges[i+0];
                if (enumerationRanges[i+1] < minEnumerationValue)
                    minEnumerationValue = enumerationRanges[i+1];
            }

            if (enumMode != ByBitMask)
            {
                if (maxEnumerationValue - minEnumerationValue > 1e7)
                {
                    vtkErrorMacro(<<"Extraordinarily large value in enumeration range (>1e7).");
                    return;
                }

                if (enumerationMap)
                    delete[] enumerationMap;
                enumerationMap = new unsigned char[int(maxEnumerationValue-minEnumerationValue)+1];
                for (i=0; i<=maxEnumerationValue-minEnumerationValue; i++)
                    enumerationMap[i] = 0;
            }

            //
            // Build the enumeration map (and enum mask for ByBitMask mode)
            //
            selectedEnumMask = 0;
            for (i=0; i<enumerationRanges.size(); i += 2)
            {
                if (sel[i/2])
                {
                    if (enumMode == ByBitMask)
                        selectedEnumMask |= (((unsigned long long)1)<<(i/2));
                    else
                        enumerationMap[int(enumerationRanges[i]-minEnumerationValue)] = 1;
                }
            }

            break;
        }
        case ByRange:
        default:
        {
            double selectedMin = +DBL_MAX;
            double selectedMax = -DBL_MAX;
            vector<double> selectedRanges;
            for (i=0; i<sel.size(); i++)
            {
                double rMin = enumerationRanges[2*i  ];
                double rMax = enumerationRanges[2*i+1];

                if (sel[i])
                {
                    if (rMin < selectedMin) selectedMin = rMin;
                    if (rMax > selectedMax) selectedMax = rMax;
                    selectedRanges.push_back(rMin);
                    selectedRanges.push_back(rMax);
                }
            }

            minEnumerationValue = selectedMin;
            maxEnumerationValue = selectedMax;
            enumerationRanges = selectedRanges;

            //
            // For binary search purposes, sort the range bins
            //
            qsort(&enumerationRanges[0], enumerationRanges.size()/2, 2*sizeof(double), CompareRanges);

            //
            // reset the last range bin found to unknown
            //
            lastRangeBin = -1;

            break;
        }
    }
}

void vtkEnumThreshold::SetAlwaysExcludeRange(double min, double max)
{
    alwaysExcludeMin = min;
    alwaysExcludeMax = max;
}

void vtkEnumThreshold::SetAlwaysIncludeRange(double min, double max)
{
    alwaysIncludeMin = min;
    alwaysIncludeMax = max;
}

vtkEnumThreshold::PartialCellMode vtkEnumThreshold::SetPartialCellMode(PartialCellMode m)
{
    PartialCellMode oldVal = partialCellMode;
    partialCellMode = m;
    return oldVal;
}

vtkEnumThreshold::EnumerationMode vtkEnumThreshold::SetEnumerationMode(EnumerationMode m)
{
    EnumerationMode oldVal = enumMode;
    enumMode = m;
    return oldVal;
}

void vtkEnumThreshold::SetNAndMaxRForNChooseRMode(int n, int maxr)
{
    int row, col;

    for (row = 0; row < pascalsTriangleMap.size(); row++)
        pascalsTriangleMap[row].clear();
    pascalsTriangleMap.clear();

    for (row = 0; row < n; row++)
    {
        vector<int> tmpRow;
        for (col = 0; col <= maxr; col++)
            tmpRow.push_back(0);
        pascalsTriangleMap.push_back(tmpRow);
    }

    for (row = 0; row < n; row++)
        for (col = 0; col <= maxr; col++)
            pascalsTriangleMap[row][col] = int(choose(n-row-1,col));

    pascalsTriangleN = n;
    pascalsTriangleR = maxr;
}

bool vtkEnumThreshold::SetReturnEmptyIfAllCellsKept(bool set)
{
    bool oldval = returnEmptyIfAllCellsKept;
    returnEmptyIfAllCellsKept = set;
    return oldval;
}

bool vtkEnumThreshold::GetReturnEmptyIfAllCellsKept() const
{
    return returnEmptyIfAllCellsKept;
}

bool vtkEnumThreshold::GetAllCellsKeptInLastRequestData() const
{
    return allCellsKeptInLastRequestData;
}
