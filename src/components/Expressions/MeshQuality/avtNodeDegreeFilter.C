// ************************************************************************* //
//                             avtNodeDegreeFilter.C                         //
// ************************************************************************* //

#include <avtNodeDegreeFilter.h>

#include <vector>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <DebugStream.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtNodeDegreeFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNodeDegreeFilter::avtNodeDegreeFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtNodeDegreeFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNodeDegreeFilter::~avtNodeDegreeFilter()
{
    ;
}


// ****************************************************************************
//  Function: VIntFint
//
//  Purpose:
//      Find a value in a vector of ints.
//
//  Arguments:
//    v    input vector
//    val  the value to look for
// 
//  Returns:      The first index, or -1 if not found.
//
//  Programmer:   Akira Haddox
//  Creation:     June 27, 2002
//
// ****************************************************************************

inline int
VIntFind(const vector<int>&v, int val)
{
    for (int i = 0; i < v.size(); i++)
        if (v[i] == val)
            return i;
    return -1;
}

// ****************************************************************************
//  Function: GlobalPointAssign
//
//  Purpose:
//      Find the global ids of nodes given their local cell node ids.
//
//  Arguments:
//      cell      The cell,
//      adj(o)    The array to fill with global ids
//      _a,_b,_c  The local cell node ids.
//      _d        Optional 4th node id.
//
//  Programmer:   Akira Haddox
//  Creation:     June 27, 2002
//
// ****************************************************************************

inline
void GlobalPointAssign(vtkCell *cell, int adj[], int _a, int _b, int _c,
                       int _d=0)
{
    adj[0] = cell->GetPointId(_a);
    adj[1] = cell->GetPointId(_b);
    adj[2] = cell->GetPointId(_c);
    adj[3] = cell->GetPointId(_d);
}

// ****************************************************************************
//  Method: avtNodeDegreeFilter::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Akira Haddox
//  Creation:     June 27, 2002
//
// ****************************************************************************

vtkDataArray *
avtNodeDegreeFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int i,localId;
    int nPoints = in_ds->GetNumberOfPoints();
    
    // This is our connectivity list. It says that point P is connected to
    // the points in connectivity[P]. A point is not connected to itself.
    // Therefore, the degree of that node is connectivity[P].size().
    // But first we have to construct the list.
    vector<vector<int> > connectivity(nPoints); 

    int nCells = in_ds->GetNumberOfCells();
    for (i = 0 ; i < nCells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        int numPointsForThisCell = cell->GetNumberOfPoints();
        for (localId = 0 ; localId < numPointsForThisCell ; localId++)
        {
            int id = cell->GetPointId(localId);

            // Most nodes have 3 adjacent nodes in that cell.
            // One pryamid node has 4, and is treated as a special case.
            // But we will store in adj[] the global ID of the points
            // that this point is adjacent to.

            int adj[4];
            switch (cell->GetCellType())
            {
                case VTK_TETRA:
                    switch (localId)
                    {
                        // Example reading:
                        // This first line says that
                        // CellLocalPointId 0 is adjacent to
                        // CellLocalPointIds 1,2 and 3. Please find what those
                        // point's global IDs are, and store them in adj[].
                        case 0:    GlobalPointAssign(cell,adj,1,2,3); break;
                        case 1:    GlobalPointAssign(cell,adj,0,2,3); break;
                        case 2:    GlobalPointAssign(cell,adj,0,1,3); break;
                        case 3:    GlobalPointAssign(cell,adj,0,1,2); break;
                    }
                    break;

                case VTK_HEXAHEDRON:
                    switch (localId)
                    {
                        case 0:    GlobalPointAssign(cell,adj,1,3,4); break;
                        case 1:    GlobalPointAssign(cell,adj,0,2,5); break;
                        case 2:    GlobalPointAssign(cell,adj,1,3,6); break;
                        case 3:    GlobalPointAssign(cell,adj,0,2,7); break;
                        case 4:    GlobalPointAssign(cell,adj,0,5,7); break;
                        case 5:    GlobalPointAssign(cell,adj,1,4,6); break;
                        case 6:    GlobalPointAssign(cell,adj,2,5,7); break;
                        case 7:    GlobalPointAssign(cell,adj,3,4,6); break;
                    }
                    break;

                case VTK_VOXEL:
                    switch (localId)
                    {
                        case 0: GlobalPointAssign(cell,adj,1,2,4); break;
                        case 1: GlobalPointAssign(cell,adj,0,3,5); break;
                        case 2: GlobalPointAssign(cell,adj,0,3,6); break;
                        case 3: GlobalPointAssign(cell,adj,1,2,7); break;
                        case 4: GlobalPointAssign(cell,adj,0,5,6); break;
                        case 5: GlobalPointAssign(cell,adj,1,4,7); break;
                        case 6: GlobalPointAssign(cell,adj,2,4,7); break;
                        case 7: GlobalPointAssign(cell,adj,3,5,6); break;
                    }
                    break;

                case VTK_WEDGE:
                    switch (localId)
                    {
                        case 0: GlobalPointAssign(cell,adj,1,2,3); break;
                        case 1: GlobalPointAssign(cell,adj,0,2,4); break;
                        case 2: GlobalPointAssign(cell,adj,0,1,5); break;
                        case 3: GlobalPointAssign(cell,adj,0,4,5); break;
                        case 4: GlobalPointAssign(cell,adj,1,3,5); break;
                        case 5: GlobalPointAssign(cell,adj,2,3,4); break;
                    }
                    break;

                case VTK_PYRAMID:
                    switch (localId)
                    {
                        case 0: GlobalPointAssign(cell,adj,1,3,4,-1); break;
                        case 1: GlobalPointAssign(cell,adj,0,2,4,-1); break;
                        case 2: GlobalPointAssign(cell,adj,1,3,4,-1); break;
                        case 3: GlobalPointAssign(cell,adj,0,2,4,-1); break;
                        case 4: GlobalPointAssign(cell,adj,0,1,2,3); break;
                    }
                    break;

                default:
                    EXCEPTION0(ImproperUseException);
            }

            // Now we add to the connectivity list.
            // Take the three points in adj[]
            // and if they're not already in the connectivity list, add them.
            for (int j = 0; j < 3; j++)
            {
                if (VIntFind(connectivity[id],adj[j]) == -1)
                    connectivity[id].push_back(adj[j]);
            }
            // If it's a pyramid, and we have a fourth point, process that.
            if (cell->GetCellType() == VTK_PYRAMID && adj[3] != -1)
                if (VIntFind(connectivity[id],adj[3]) == -1)
                    connectivity[id].push_back(adj[3]);
        }
    }

    //
    // Set up a VTK variable reflecting the NodeDegrees we calculated, which
    // is found by taking the length of the connectivity lists.
    //
    vtkFloatArray *dv = vtkFloatArray::New();
    dv->SetNumberOfComponents(1);
    dv->SetNumberOfValues(nPoints);
    for(i = 0 ; i < nPoints ; i++)
    {
        dv->SetValue(i, connectivity[i].size());
    }

    return dv;
}
