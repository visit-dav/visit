#include "vtkConnectedTubeFilter.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkFloatArray.h>
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
//
//    Hank Childs (for Jeremy Meredith), Mon Apr  7 11:46:51 PDT 2003
//    Increased size of cellindex.
//
// ****************************************************************************
void
vtkConnectedTubeFilter::PointSequence::Init(int maxlen)
{
    if (index)
        delete[] index;
    if (cellindex)
        delete[] cellindex;

    length = 0;
    index  = new int[maxlen];
    cellindex = new int[maxlen];
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
// ****************************************************************************
void
vtkConnectedTubeFilter::PointSequence::Add(int i, int ci)
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
// ****************************************************************************
bool 
vtkConnectedTubeFilter::PointSequenceList::Build(vtkPoints *points,
                                                 vtkCellArray *lines)
{
    pts             = (float*)(points->GetVoidPointer(0));
    len             = points->GetNumberOfPoints();
    numneighbors    = new int[len];
    connectivity[0] = new int[len];
    connectivity[1] = new int[len];
    cellindex       = new int[len];

    vtkIdType *cells = lines->GetPointer();

    // Initalize all points to be disconnected from each other
    int i;
    for (i=0; i<len; i++)
    {
        numneighbors[i] = 0;
    }

    int numCells = lines->GetNumberOfCells();
    for (i=0; i<numCells; i++)
    {
        // We assume all cells are two-point lines (i.e. not polylines)
        if (cells[i*3] != 2)
        {
            return false;
        }

        // Get the begin and end index for this segment
        int a = cells[i*3 + 1];
        int b = cells[i*3 + 2];

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
        cellindex[a] = i;
        cellindex[b] = i;
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
// ****************************************************************************
bool
vtkConnectedTubeFilter::PointSequenceList::GetNextSequence(PointSequence &seq)
{
    // index is already set by InitTraversal and previous calls to this fn
    for (; index < len; index++)
    {
        // if numneighbors is 1, then this is a start point
        if (numneighbors[index] == 1 && !visited[index])
        {
            int current = index;
            int previous = -1;
            seq.Init(len);
            seq.Add(current, cellindex[current]);
            visited[current] = true;
            while (true)
            {
                int n1       = connectivity[0][current];
                int n2       = connectivity[1][current];
                int next     = (n1 == previous) ? n2 : n1;
                previous = current;
                current = next;

                visited[current] = true;

                // we must skip any sequential identical points:
                // 1) they are useless, and 2) they mess up calculations
                if (pts[previous*3+0] != pts[current*3+0] ||
                    pts[previous*3+1] != pts[current*3+1] ||
                    pts[previous*3+2] != pts[current*3+2])
                {
                    seq.Add(current, cellindex[current]);
                }

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

    // false ==> failed; no more sequences
    return false;
}


// ----------------------------------------------------------------------------
//                        class vtkConnectedTubeFilter
// ----------------------------------------------------------------------------


vtkCxxRevisionMacro(vtkConnectedTubeFilter, "$Revision: 2.00 $");
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
    this->CreateNormals = 0;
    this->Capping = 0;

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
// ****************************************************************************
bool vtkConnectedTubeFilter::BuildConnectivityArrays()
{
    vtkPolyData  *input   = this->GetInput();
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
//  Method:  vtkConnectedTubeFilter::Execute
//
//  Purpose:
//    Normal vtk filter execution.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Apr  7 10:02:31 PDT 2003
//    Allocate memory for points, because VTK does not do that for you.
//
// ****************************************************************************
void vtkConnectedTubeFilter::Execute()
{
    // Get all the appropriate input arrays
    vtkPolyData  *input   = this->GetInput();
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
        return;
    }

    if (!(inPts=input->GetPoints())               || 
        (numPts = inPts->GetNumberOfPoints()) < 1 ||
        !(inLines = input->GetLines())            ||
        (numCells = inLines->GetNumberOfCells()) < 1)
    {
        vtkDebugMacro(<< ": No input data!\n");
        return;
    }

    float *pts = (float*)(inPts->GetVoidPointer(0));

    // Set up the output arrays
    int maxNewCells  = numCells * (NumberOfSides + 2);
    int maxNewPoints = numCells * NumberOfSides * 2;
    vtkPolyData   *output     = this->GetOutput();
    vtkPoints     *newPts     = vtkPoints::New();
    newPts->Allocate(maxNewPoints);
    vtkCellArray  *newCells   = vtkCellArray::New();
    newCells->Allocate(maxNewCells, 4*maxNewCells + 2*NumberOfSides);
    vtkFloatArray *newNormals = NULL;

    vtkPointData  *newPD      = NULL;
    newPD = output->GetPointData();
    newPD->CopyNormalsOff();
    newPD->CopyAllocate(inPD, maxNewPoints);

    vtkCellData   *newCD      = NULL;
    newCD = output->GetCellData();
    newCD->CopyAllocate(inCD, maxNewCells);
    
    if (CreateNormals)
    {
        newNormals = vtkFloatArray::New();
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

        for (int i=0; i<seq.length; i++)
        {
            bool firstPoint = (i==0);
            bool lastPoint  = (i==seq.length-1);

            // Get the current, previous, and next indices
            int ix  = seq.index[i];
            int ix1 = (firstPoint ?  seq.index[i] : seq.index[i-1]);
            int ix2 = (lastPoint  ?  seq.index[i] : seq.index[i+1]);

            // Use a centered difference approximation for direction
            float dir[3] = {pts[ix2*3+0] - pts[ix1*3+0],
                            pts[ix2*3+1] - pts[ix1*3+1],
                            pts[ix2*3+2] - pts[ix1*3+2]};

            // If our centered difference was zero, do a forward
            // difference instead.  We ensured no sequential points
            // are identical, so this can't fail.
            if (dir[0]==0 && dir[1]==0 && dir[2]==0)
            {
                dir[0] = pts[ix2*3+0] - pts[ix*3+0];
                dir[1] = pts[ix2*3+1] - pts[ix*3+1];
                dir[2] = pts[ix2*3+2] - pts[ix*3+2];
            }

            // Get a couple vectors orthogonal to our direction
            float v1[3], v2[3];
            vtkMath::Perpendiculars(dir, v1,v2, 0.0);
            vtkMath::Normalize(v1);
            vtkMath::Normalize(v2);

            // Hang on to the first point index we create; we need it 
            // to create the cells
            vtkIdType firstIndex = newPts->GetNumberOfPoints();

            for (int j = 0 ; j < NumberOfSides ; j++)
            {
                float q = (j * 2. * vtkMath::Pi()) / float(NumberOfSides-1);
                float sq = sin(q);
                float cq = cos(q);
                float normal[3] = { v1[0]*cq + v2[0]*sq,
                                    v1[1]*cq + v2[1]*sq,
                                    v1[2]*cq + v2[2]*sq};
                float x = pts[ix*3+0] + Radius * normal[0];
                float y = pts[ix*3+1] + Radius * normal[1];
                float z = pts[ix*3+2] + Radius * normal[2];
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
                    int p[4] =
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
