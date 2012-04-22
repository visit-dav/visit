/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
// ****************************************************************************

avtNodeDegreeExpression::avtNodeDegreeExpression()
{
    ;
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
VIntFind(const std::vector<int>&v, int val)
{
    for (size_t i = 0; i < v.size(); i++)
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

inline
void GlobalPointAssign2(vtkCell *cell, int adj[], int _a, int _b)
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
// ****************************************************************************

vtkDataArray *
avtNodeDegreeExpression::DeriveVariable(vtkDataSet *in_ds)
{
    vtkIdType nPoints = in_ds->GetNumberOfPoints();
    
    // This is our connectivity list. It says that point P is connected to
    // the points in connectivity[P]. A point is not connected to itself.
    // Therefore, the degree of that node is connectivity[P].size().
    // But first we have to construct the list.
    std::vector<std::vector<int> > connectivity(nPoints); 

    vtkIdType nCells = in_ds->GetNumberOfCells();
    for (vtkIdType i = 0 ; i < nCells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        vtkIdType numPointsForThisCell = cell->GetNumberOfPoints();
        for (int localId = 0 ; localId < numPointsForThisCell ; localId++)
        {
            vtkIdType id = cell->GetPointId(localId);

            // Most nodes have 3 adjacent nodes in that cell.
            // One pryamid node has 4, and is treated as a special case.
            // But we will store in adj[] the global ID of the points
            // that this point is adjacent to.

            int adj[4], nadj = 3;
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
                        case 4: GlobalPointAssign(cell,adj,0,1,2,3); nadj = 4; break;
                    }
                    break;

                case VTK_QUAD:
                    nadj = 2;
                    switch (localId)
                    {
                        case 0: GlobalPointAssign2(cell,adj,1,3); break;
                        case 1: GlobalPointAssign2(cell,adj,0,2); break;
                        case 2: GlobalPointAssign2(cell,adj,1,3); break;
                        case 3: GlobalPointAssign2(cell,adj,0,2); break;
                    }
                    break;

                case VTK_TRIANGLE:
                    nadj = 2;
                    switch (localId)
                    {
                        case 0: GlobalPointAssign2(cell,adj,1,2); break;
                        case 1: GlobalPointAssign2(cell,adj,0,2); break;
                        case 2: GlobalPointAssign2(cell,adj,0,1); break;
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
                if (VIntFind(connectivity[id],adj[j]) == -1)
                    connectivity[id].push_back(adj[j]);
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
        dv->SetValue(i, connectivity[i].size());
    }

    return dv;
}
