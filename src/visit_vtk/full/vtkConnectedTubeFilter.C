// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkConnectedTubeFilter.h"
#include <visit-config.h>
#include <vtkCellArray.h>
#if LIB_VERSION_GE(VTK, 9,1,0)
#include <vtkCellArrayIterator.h>
#endif
#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyLine.h>
#include <vtkTubeFilter.h>
// ----------------------------------------------------------------------------
//                            class PointSequence
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Constructor:  PointSequence::PointSequence
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
// ****************************************************************************
vtkConnectedTubeFilter::PointSequence::PointSequence()
{
    length    = 0;
    index     = NULL;
    cellindex = NULL;
}

// ****************************************************************************
//  Denstructor:  PointSequence::~PointSequence
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
// ****************************************************************************
vtkConnectedTubeFilter::PointSequence::~PointSequence()
{
    if (index)
        delete[] index;
    index = NULL;
    if (cellindex)
        delete[] cellindex;
    cellindex = NULL;
}

// ****************************************************************************
//  Method:  PointSequence::Init
//
//  Purpose:
//    Initialize the data structures now that we know how big it might be
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
//  Modifications:
//    Hank Childs (for Jeremy Meredith), Mon Apr  7 11:46:51 PDT 2003
//    Increased size of cellindex.
//
//
//  Jean Favre, Tue May  7 16:38:37 CEST 2013
//  Used vtkIdType where needed
// ****************************************************************************
void
vtkConnectedTubeFilter::PointSequence::Init(int maxlen)
{
    if (index)
        delete[] index;
    if (cellindex)
        delete[] cellindex;

    length = 0;
    index  = new vtkIdType[maxlen];
    cellindex = new vtkIdType[maxlen];
}

// ****************************************************************************
//  Method:  PointSequence::Add
//
//  Purpose:
//    Adds a new point, with its cell index, to the current sequence
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
//  Jean Favre, Tue May  7 16:38:37 CEST 2013
//  Used vtkIdType where needed
// ****************************************************************************
void
vtkConnectedTubeFilter::PointSequence::Add(vtkIdType i, vtkIdType ci)
{
    index[length]     = i;
    cellindex[length] = ci;
    length++;
}


// ----------------------------------------------------------------------------
//                          class PointSequenceList
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Constructor:  PointSequenceList::PointSequenceList
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
//  Modifications:
//    Rich Cook and Hank Childs, Thu Oct  2 16:32:55 PDT 2008
//    Initialized data member used for supporting loops.
//
// ****************************************************************************
vtkConnectedTubeFilter::PointSequenceList::PointSequenceList()
{
    pts             = NULL;
    len             = 0;
    numneighbors    = NULL;
    connectivity[0] = NULL;
    connectivity[1] = NULL;
    cellindex       = NULL;

    visited = NULL;
    index   = -1;
    lookforloops = false;
}

// ****************************************************************************
//  Denstructor:  PointSequenceList::~PointSequenceList
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
// ****************************************************************************
vtkConnectedTubeFilter::PointSequenceList::~PointSequenceList()
{
    if (visited)
        delete[] visited;
    if (numneighbors)
        delete[] numneighbors;
    if (connectivity[0])
        delete[] connectivity[0];
    if (connectivity[1])
        delete[] connectivity[1];
    if (cellindex)
        delete[] cellindex;
    // Don't delete 'pts', because we don't own it.
}

// ****************************************************************************
//  Method:  PointSequenceList::Build
//
//  Purpose:
//    Extract the
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
//  Jean Favre, Tue May  7 16:38:37 CEST 2013
//  Used vtkIdType where needed
//
//  Kathleen Biagas, Thu Aug 11, 2022
//  Support VTK9: use vtkCellArrayIterator.
//
// ****************************************************************************
bool
vtkConnectedTubeFilter::PointSequenceList::Build(vtkPoints *points,
                                                 vtkCellArray *lines)
{
    pts             = points;
    len             = points->GetNumberOfPoints();
    numneighbors    = new vtkIdType[len];
    connectivity[0] = new vtkIdType[len];
    connectivity[1] = new vtkIdType[len];
    cellindex       = new vtkIdType[len];

    // Initalize all points to be disconnected from each other

    for (int i=0; i<len; i++)
    {
        numneighbors[i] = 0;
    }

#if LIB_VERSION_LE(VTK, 8,1,0)
    vtkIdType *cells = lines->GetPointer();
    int numCells = lines->GetNumberOfCells();
    for (int i=0; i<numCells; i++)
    {
        // We assume all cells are two-point lines (i.e. not polylines)
        if (cells[i*3] != 2)
        {
            return false;
        }
        // Get the begin and end index for this segment
        vtkIdType a = cells[i*3 + 1];
        vtkIdType b = cells[i*3 + 2];
#else

    auto iter = vtk::TakeSmartPointer(lines->NewIterator());
    for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal(); iter->GoToNextCell())
    {
        vtkIdType cellSize;
        const vtkIdType *currCell = nullptr;
        iter->GetCurrentCell(cellSize, currCell);
        // We assume all cells are two-point lines (i.e. not polylines)
        if (cellSize != 2)
        {
            return false;
        }

        // Get the begin and end index for this segment
        const vtkIdType a = currCell[0];
        const vtkIdType b = currCell[1];
#endif

        // If we have two neighbors already, this is a T intersection
        if (numneighbors[a] >= 2 || numneighbors[b] >= 2)
        {
            return false;
        }

        // Set the neighbors of each endpoint to be each other
        connectivity[numneighbors[a]][a] = b;
        connectivity[numneighbors[b]][b] = a;
        numneighbors[a]++;
        numneighbors[b]++;
#if LIB_VERSION_LE(VTK, 8,1,0)
        cellindex[a] = i;
        cellindex[b] = i;
#else
        cellindex[a] = iter->GetCurrentCellId();
        cellindex[b] = iter->GetCurrentCellId();
#endif
    }
    return true;
}

// ****************************************************************************
//  Method:  PointSequenceList::InitTraversal
//
//  Purpose:
//    Create a new 'visited' array and reset the iterator index
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
//  Modifications:
//    Rich Cook and Hank Childs, Thu Oct  2 16:32:55 PDT 2008
//    Initialized data member used for supporting loops.
//
// ****************************************************************************
void
vtkConnectedTubeFilter::PointSequenceList::InitTraversal()
{
    if (visited)
        delete[] visited;

    visited = new bool[len];
    for (int i=0; i<len; i++)
        visited[i] = false;

    index = 0;
    lookforloops = false;
}

// ****************************************************************************
//  Method:  PointSequenceList::GetNextSequence
//
//  Purpose:
//    Extract another sequence from the connectivity array and return it
//    as a PointSequence.  Skip repeated adjacent points.
//
//  Returns:  true if we found another sequence; false otherwise
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
//  Modifications:
//    Rich Cook and Hank Childs, Thu Oct  2 16:32:55 PDT 2008
//    Added support for loops.
//
//    Eric Brugger, Tue Dec 29 16:35:34 PST 2009
//    I modified the logic that looks for loops to only consider points
//    that have 2 neighbors.  Previously it would have considered points
//    with no neighbors, which caused it to reference uninitialized memory.
//
//    Jean Favre, Tue May  7 16:38:37 CEST 2013
//    I modified the calls to GetPoint() to use the other variant of the call
//    with two arguments. The previous version would never succeed in the test
//    to remove sequential identical points.
//    Used vtkIdType where needed
// ****************************************************************************
bool
vtkConnectedTubeFilter::PointSequenceList::GetNextSequence(PointSequence &seq)
{
    // index is already set by InitTraversal and previous calls to this fn
    for (; index < len; index++)
    {
        // if numneighbors is 1, then this is a start point
        if (((lookforloops && numneighbors[index] == 2) ||
              numneighbors[index] == 1) && !visited[index])
        {
            vtkIdType current = index;
            vtkIdType previous = -1;
            seq.Init(len);
            seq.Add(current, cellindex[current]);
            visited[current] = true;
            while (true)
            {
                vtkIdType n1       = connectivity[0][current];
                vtkIdType n2       = connectivity[1][current];
                vtkIdType next     = (n1 == previous) ? n2 : n1;
                previous = current;
                current = next;


                // we must skip any sequential identical points:
                // 1) they are useless, and 2) they mess up calculations
                double prePt[3];
                double curPt[3];
                pts->GetPoint(previous, prePt);
                pts->GetPoint(current, curPt);
                if (prePt[0] != curPt[0] ||
                    prePt[1] != curPt[1] ||
                    prePt[2] != curPt[2])
                {
                    seq.Add(current, cellindex[current]);
                }

                // check for a loop (AFTER adding the node again...)
                if (lookforloops && visited[current])
                {
                    break;
                }
                visited[current] = true;

                bool endpoint = (numneighbors[current] == 1);
                if (endpoint)
                    break;
            }

            // We may have one duplicated cell index due to the way the
            // connectivity was built -- shift them all down by one from
            // the right spot, so the only duplicated one is the last one.
            for (int i=1; i<seq.length-1; i++)
            {
                if (seq.cellindex[i-1] == seq.cellindex[i])
                    seq.cellindex[i] = seq.cellindex[i+1];
            }

            // true ==> success; got another sequence
            return true;
        }
    }

    if (index == len && !lookforloops)
    {
        lookforloops = true;
        index=0;
        return GetNextSequence(seq);
    }

    // false ==> failed; no more sequences
    return false;
}


// ----------------------------------------------------------------------------
//                        class vtkConnectedTubeFilter
// ----------------------------------------------------------------------------


vtkStandardNewMacro(vtkConnectedTubeFilter);

// ****************************************************************************
//  Constructor:  vtkConnectedTubeFilter::vtkConnectedTubeFilter
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
// ****************************************************************************
vtkConnectedTubeFilter::vtkConnectedTubeFilter()
{
    this->Radius = 0.5;
    this->NumberOfSides = 3;
    this->CreateNormals = false;
    this->Capping = false;

    pseqlist = NULL;
}

// ****************************************************************************
//  Denstructor:  vtkConnectedTubeFilter::~vtkConnectedTubeFilter
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
// ****************************************************************************
vtkConnectedTubeFilter::~vtkConnectedTubeFilter()
{
    if (pseqlist)
        delete pseqlist;
    pseqlist = NULL;
}

// ****************************************************************************
//  Method:  vtkConnectedTubeFilter::BuildConnectivityArrays
//
//  Purpose:
//    Create our new style connectivity arrays.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
//  Modifications:
//    Eric Brugger, Wed Jan  9 11:31:17 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************
bool vtkConnectedTubeFilter::BuildConnectivityArrays(vtkPolyData *input)
{
    vtkPoints    *inPts   = NULL;
    vtkCellArray *inLines = NULL;
    int numPts;
    int numCells;
    vtkDebugMacro(<<"Building tube connectivity arrays");

    if (!(inPts=input->GetPoints())               ||
        (numPts = inPts->GetNumberOfPoints()) < 1 ||
        !(inLines = input->GetLines())            ||
        (numCells = inLines->GetNumberOfCells()) < 1)
    {
        vtkDebugMacro(<< ": No input data!\n");
        return false;
    }

    // Build the new connectivity, and check for errors while we're at it
    pseqlist = new PointSequenceList;
    bool success = pseqlist->Build(inPts, inLines);

    if (!success)
    {
        // We know we can't use it anyway; delete it now
        delete pseqlist;
        pseqlist = NULL;
    }

    return success;
}

// ****************************************************************************
//  Method:  vtkConnectedTubeFilter::RequestData
//
//  Purpose:
//    Normal vtk filter execution.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
//  Modifications:
//    Hank Childs, Mon Apr  7 10:02:31 PDT 2003
//    Allocate memory for points, because VTK does not do that for you.
//
//    Jeremy Meredith, Wed Oct 15 15:56:34 EDT 2008
//    It appears we were creating the tube points incorrectly (creating
//    degenerate polys between 360 and 0 degrees) by partitioning into
//    npts-1 sides, instead of npts sides.  Fixed that.
//
//    Kathleen Biagas, Thu Sep 6 11:15:29 MST 2012
//    Preserve coordinate data type.
//
//    Eric Brugger, Wed Jan  9 11:31:17 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
//    Jean Favre, Tue May  7 16:38:37 CEST 2013
//    I modified the calls to GetPoint() to use the other variant of the call
//    with two arguments. The previous version would fail getting the right values.
//    Used vtkIdType where needed
//
//    Jean Favre, Tue May 28 13:28:41 CEST 2013
//    I added a quaternion interpolation to smoothly interpolate the direction vectors
//    used to construct the tube. This eliminates abrupt twists along the tube
//
// ****************************************************************************
int vtkConnectedTubeFilter::RequestData(
    vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    //
    // Initialize some frequently used values.
    //
    vtkPolyData  *input = vtkPolyData::SafeDownCast(
        inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkPolyData *output = vtkPolyData::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));

    // Get all the appropriate input arrays
    vtkPoints    *inPts   = NULL;
    vtkCellArray *inLines = NULL;
    vtkCellData  *inCD    = input->GetCellData();
    vtkPointData *inPD    = input->GetPointData();
    int numPts;
    int numCells;
    vtkDebugMacro(<<"Creating tube");

    // We assume BuildConnectivityArrays was already called
    if (!pseqlist)
    {
        vtkErrorMacro(<< ": Connectivity was not built yet; need to call "
                         "vtkConnectedTubeFilter::BuildConnectivityArrays()\n");
        return 1;
    }

    if (!(inPts=input->GetPoints())               ||
        (numPts = inPts->GetNumberOfPoints()) < 1 ||
        !(inLines = input->GetLines())            ||
        (numCells = inLines->GetNumberOfCells()) < 1)
    {
        vtkDebugMacro(<< ": No input data!\n");
        return 1;
    }

    // Set up the output arrays
    int maxNewCells  = numCells * (NumberOfSides + 2);
    int maxNewPoints = numCells * NumberOfSides * 2;
    vtkPoints     *newPts     = vtkPoints::New(inPts->GetDataType());
    newPts->Allocate(maxNewPoints);
    vtkCellArray  *newCells   = vtkCellArray::New();
    newCells->Allocate(maxNewCells, 4*maxNewCells + 2*NumberOfSides);
    vtkDataArray *newNormals = NULL;

    vtkPointData  *newPD      = NULL;
    newPD = output->GetPointData();
    newPD->CopyNormalsOff();
    newPD->CopyAllocate(inPD, maxNewPoints);

    vtkCellData   *newCD      = NULL;
    newCD = output->GetCellData();
    newCD->CopyAllocate(inCD, maxNewCells);

    if (CreateNormals)
    {
        newNormals = inPts->GetData()->NewInstance();
        newNormals->SetNumberOfComponents(3);
        newNormals->SetName("Normals");
        newPD->SetNormals(newNormals);
        newNormals->Delete();
    }

    // Iterate over all connected point sequences
    PointSequence seq;
    pseqlist->InitTraversal();
    while (pseqlist->GetNextSequence(seq))
    {
        // Skip any standalone points.
        if (seq.length == 1)
            continue;

        double prev_dir[3] = {0., 0., 0.}, prev_v1[3], prev_v2[3];
        for (int i=0; i<seq.length; i++)
        {
            bool firstPoint = (i==0);
            bool lastPoint  = (i==seq.length-1);

            // Get the current, previous, and next indices
            vtkIdType ix  = seq.index[i];
            vtkIdType ix1 = (firstPoint ?  seq.index[i] : seq.index[i-1]);
            vtkIdType ix2 = (lastPoint  ?  seq.index[i] : seq.index[i+1]);

            // Use a centered difference approximation for direction
            double v0[3], v1[3], v2[3], dir[3], pt[3];
            inPts->GetPoint(ix2, v2);
            inPts->GetPoint(ix1, v1);
            vtkMath::Subtract(v2, v1, dir);

            // If our centered difference was zero, do a forward
            // difference instead.  We ensured no sequential points
            // are identical, so this can't fail.
            if (dir[0]==0 && dir[1]==0 && dir[2]==0)
            {
               inPts->GetPoint(ix, v0);
               vtkMath::Subtract(v2, v0, dir);
            }
            //inNormals->GetTuple(i, v1);
            vtkMath::Cross(dir, v1, v2);
            vtkMath::Normalize(v2);

            if(firstPoint)
            {
              // Get a couple vectors orthogonal to our direction
              //double v1[3], v2[3];
              vtkMath::Perpendiculars(dir, v1,v2, 0.0);
              vtkMath::Normalize(v1);
              vtkMath::Normalize(v2);
            }
            else
            {
              // we can't just get two new orthogonal vectors each time because of abrupt changes
              // We will construct the matrix which xforms the "previous dir" vector
              // into this new dir vector, and then use this rigid body motion to
              // xform v1 and v2 from the previous point
              double Xform[3][3];
              double vec[3];
              vtkMath::Cross(prev_dir, dir, vec);
              double costheta = vtkMath::Dot(prev_dir, dir);
              double sintheta = vtkMath::Norm(vec);
              double theta = atan2(sintheta, costheta);
              if (sintheta != 0)
                {
                vec[0] /= sintheta;
                vec[1] /= sintheta;
                vec[2] /= sintheta;
                }
              // convert to quaternion
              costheta = cos(0.5*theta);
              sintheta = sin(0.5*theta);
              double quat[4];
              quat[0] = costheta;
              quat[1] = vec[0]*sintheta;
              quat[2] = vec[1]*sintheta;
              quat[3] = vec[2]*sintheta;
              // convert to matrix
              vtkMath::QuaternionToMatrix3x3(quat, Xform); // Xform will xform prev_dir into dir
              // now use it
              vtkMath::Multiply3x3 (Xform, prev_v1, v1);
              vtkMath::Multiply3x3 (Xform, prev_v2, v2);
              vtkMath::Normalize(v1);
              vtkMath::Normalize(v2);
            }

            prev_dir[0] = dir[0]; prev_dir[1] = dir[1]; prev_dir[2] = dir[2];
            prev_v1[0]  = v1[0];  prev_v1[1]  = v1[1];  prev_v1[2]  = v1[2];
            prev_v2[0]  = v2[0];  prev_v2[1]  = v2[1];  prev_v2[2]  = v2[2];

            // Hang on to the first point index we create; we need it
            // to create the cells
            vtkIdType firstIndex = newPts->GetNumberOfPoints();

            //double *pt = inPts->GetPoint(ix);
            inPts->GetPoint(ix, pt);
            for (int j = 0 ; j < NumberOfSides ; j++)
            {
                double q = (j * 2. * vtkMath::Pi()) / double(NumberOfSides);
                double sq = sin(q);
                double cq = cos(q);
                double normal[3] = { v1[0]*cq + v2[0]*sq,
                                    v1[1]*cq + v2[1]*sq,
                                    v1[2]*cq + v2[2]*sq};
                double x = pt[0] + Radius * normal[0];
                double y = pt[1] + Radius * normal[1];
                double z = pt[2] + Radius * normal[2];
                vtkIdType id = newPts->InsertNextPoint(x,y,z);
                if (CreateNormals)
                    newNormals->InsertNextTuple(normal);
                newPD->CopyData(inPD, ix, id);
            }

            // Do the start cap
            if (firstPoint && Capping)
            {
                vtkIdType id = newCells->InsertNextCell(NumberOfSides);
                for (int j=0; j<NumberOfSides; j++)
                    newCells->InsertCellPoint(firstIndex + j);
                newCD->CopyData(inCD, seq.cellindex[i], id);
            }

            // Do the next segment
            if (!firstPoint)
            {
                for (int j=0; j<NumberOfSides; j++)
                {
                    vtkIdType p[4] =
                    {
                        firstIndex + j,
                        firstIndex + j - NumberOfSides,
                        firstIndex + ((j+1) % NumberOfSides) - NumberOfSides,
                        firstIndex + ((j+1) % NumberOfSides)
                    };
                    vtkIdType id = newCells->InsertNextCell(4, p);
                    newCD->CopyData(inCD, seq.cellindex[i-1], id);
                }
            }

            // Do the end cap
            if (lastPoint && Capping)
            {
                vtkIdType id = newCells->InsertNextCell(NumberOfSides);
                for (int j=0; j<NumberOfSides; j++)
                    newCells->InsertCellPoint((firstIndex+NumberOfSides-1) - j);
                newCD->CopyData(inCD, seq.cellindex[i-1], id);
            }
        }
    }

    // Final cleanup
    newPD->Squeeze();
    newCD->Squeeze();

    output->SetPoints(newPts);
    newPts->Delete();

    output->SetPolys(newCells);
    newCells->Delete();

    // don't forget the sequence list; we're done with it
    delete pseqlist;
    pseqlist = NULL;

    return 1;
}

// ****************************************************************************
//  Method:  vtkConnectedTubeFilter::PrintSelf
//
//  Purpose:
//    Print myself.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
// ****************************************************************************
void vtkConnectedTubeFilter::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    os << indent << "Radius: " << this->Radius << "\n";
    os << indent << "Number Of Sides: " << this->NumberOfSides << "\n";
    os << indent << "Create Normals: "
       << (this->CreateNormals ? "On\n" : "Off\n");
    os << indent << "Capping: " << this->Capping << endl;
}
