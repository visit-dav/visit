// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtNodeDegreeExpression.C                         //
// ************************************************************************* //

#include <avtNodeDegreeExpression.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkIntArray.h>

#include <DebugStream.h>
#include <ImproperUseException.h>

#include <vector>

// ****************************************************************************
//  Method: avtNodeDegreeExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//
//  Alister Maguire, Thu Jun 18 10:02:58 PDT 2020
//  Set canApplyToDirectDatabaseQOT to false.
//
// ****************************************************************************

avtNodeDegreeExpression::avtNodeDegreeExpression()
{
    canApplyToDirectDatabaseQOT = false;
}


// ****************************************************************************
//  Method: avtNodeDegreeExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNodeDegreeExpression::~avtNodeDegreeExpression()
{
    ;
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
//  Modifications:
//    Brad Whitlock, Mon Jan  8 16:52:41 PST 2024
//    Use vtkIdType for adj. Added 3 arg override.
//
// ****************************************************************************

inline void
GlobalPointAssign(vtkCell *cell, vtkIdType adj[], int _a, int _b, int _c, int _d)
{
    adj[0] = cell->GetPointId(_a);
    adj[1] = cell->GetPointId(_b);
    adj[2] = cell->GetPointId(_c);
    adj[3] = cell->GetPointId(_d);
}

inline void
GlobalPointAssign(vtkCell *cell, vtkIdType adj[], int _a, int _b, int _c)
{
    adj[0] = cell->GetPointId(_a);
    adj[1] = cell->GetPointId(_b);
    adj[2] = cell->GetPointId(_c);
    adj[3] = -1;
}

inline void
GlobalPointAssign(vtkCell *cell, vtkIdType adj[], int _a, int _b)
{
    adj[0] = cell->GetPointId(_a);
    adj[1] = cell->GetPointId(_b);
    adj[2] = -1;
    adj[3] = -1;
}

// ****************************************************************************
//  Method: avtNodeDegreeExpression::DeriveVariable
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
//  Modifications:
//    Brad Whitlock, Wed Feb 23 23:36:56 PST 2011
//    I added support for triangles, quads, lines.
//
//    Brad Whitlock, Mon Jan  8 17:04:39 PST 2024
//    I added support for polyhedra.
//
// ****************************************************************************

vtkDataArray *
avtNodeDegreeExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    const vtkIdType nPoints = in_ds->GetNumberOfPoints();
    
    // This is our connectivity list. It says that point P is connected to
    // the points in connectivity[P]. A point is not connected to itself.
    // Therefore, the degree of that node is connectivity[P].size().
    // But first we have to construct the list.
    std::vector<std::vector<vtkIdType> > connectivity(nPoints); 

    const vtkIdType nCells = in_ds->GetNumberOfCells();
    for (vtkIdType i = 0 ; i < nCells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        // Handle polyhedra specially.
        if(cell->GetCellType() == VTK_POLYHEDRON)
        {
            // For polyhedra, we walk around each face and for the current
            // point, add the next point to its connectivity list if it has
            // not already been added. Do the other direction too. After walking
            // all of the faces for a cell, each point should know all of its
            // neighbor points. This should work for most cell types but we limit
            // it to polyhedra for now.
            const vtkIdType nFaces = cell->GetNumberOfFaces();
            for(vtkIdType faceId = 0; faceId < nFaces; faceId++)
            {
                vtkCell *face = cell->GetFace(faceId);
                const vtkIdType nFacePts = face->GetNumberOfPoints();
                for(vtkIdType p = 0; p < nFacePts; p++)
                {
                    const vtkIdType next_p = (p + 1) % nFacePts;
                    const vtkIdType pid = face->GetPointId(p);
                    const vtkIdType next_pid = face->GetPointId(next_p);

                    // Forward
                    if(std::find(connectivity[pid].begin(),
                                 connectivity[pid].end(),
                                 next_pid) == connectivity[pid].end())
                    {
                        connectivity[pid].push_back(next_pid);
                    }
                    // Backward
                    if(std::find(connectivity[next_pid].begin(),
                                 connectivity[next_pid].end(),
                                 pid) == connectivity[next_pid].end())
                    {
                        connectivity[next_pid].push_back(pid);
                    }
                }
            }
            continue;
        }

        const vtkIdType numPointsForThisCell = cell->GetNumberOfPoints();
        for (vtkIdType localId = 0 ; localId < numPointsForThisCell ; localId++)
        {
            const vtkIdType id = cell->GetPointId(localId);

            // Most nodes have 3 adjacent nodes in that cell.
            // One pryamid node has 4, and is treated as a special case.
            // But we will store in adj[] the global ID of the points
            // that this point is adjacent to.

            vtkIdType adj[4];
            int nadj = 3;
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
                        case 0: GlobalPointAssign(cell,adj,1,3,4); break;
                        case 1: GlobalPointAssign(cell,adj,0,2,4); break;
                        case 2: GlobalPointAssign(cell,adj,1,3,4); break;
                        case 3: GlobalPointAssign(cell,adj,0,2,4); break;
                        case 4: GlobalPointAssign(cell,adj,0,1,2,3); nadj = 4; break;
                    }
                    break;

                case VTK_QUAD:
                    nadj = 2;
                    switch (localId)
                    {
                        case 0: GlobalPointAssign(cell,adj,1,3); break;
                        case 1: GlobalPointAssign(cell,adj,0,2); break;
                        case 2: GlobalPointAssign(cell,adj,1,3); break;
                        case 3: GlobalPointAssign(cell,adj,0,2); break;
                    }
                    break;

                case VTK_TRIANGLE:
                    nadj = 2;
                    switch (localId)
                    {
                        case 0: GlobalPointAssign(cell,adj,1,2); break;
                        case 1: GlobalPointAssign(cell,adj,0,2); break;
                        case 2: GlobalPointAssign(cell,adj,0,1); break;
                    }
                    break;

                case VTK_LINE:
                    nadj = 1;
                    switch (localId)
                    {
                        case 0: adj[0] = cell->GetPointId(1); break;
                        case 1: adj[0] = cell->GetPointId(0); break;
                    }
                    break;

                default:
                    EXCEPTION0(ImproperUseException);
            }

            // Now we add to the connectivity list.
            // Take the three points in adj[]
            // and if they're not already in the connectivity list, add them.
            for (int j = 0; j < nadj; j++)
            {
                auto &conn = connectivity[id];
                if(std::find(conn.begin(), conn.end(), adj[j]) == conn.end())
                {
                    conn.push_back(adj[j]);
                }
            }
        }
    }

    //
    // Set up a VTK variable reflecting the NodeDegrees we calculated, which
    // is found by taking the length of the connectivity lists.
    //
    vtkIntArray *dv = vtkIntArray::New();
    dv->SetNumberOfValues(nPoints);
    for(vtkIdType i = 0 ; i < nPoints ; i++)
    {
        dv->SetValue(i, (int)connectivity[i].size());
    }

    return dv;
}
