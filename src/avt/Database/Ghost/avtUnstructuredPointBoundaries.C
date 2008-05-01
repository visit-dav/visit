/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <avtUnstructuredPointBoundaries.h>

#include <vtkCellType.h>
#include <vtkIdList.h>
#include <vtkUnstructuredGrid.h>

#include <map>
#include <set>

using namespace std;

// ****************************************************************************
//  Constructor:  avtUnstructuredPointBoundaries::
//                  avtUnstructuredPointBoundaries
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

avtUnstructuredPointBoundaries::avtUnstructuredPointBoundaries()
{
}

// ****************************************************************************
//  Destructor:  avtUnstructuredPointBoundaries::
//                  ~avtUnstructuredPointBoundaries
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

avtUnstructuredPointBoundaries::~avtUnstructuredPointBoundaries()
{
}


// ****************************************************************************
//  Destructor:  avtUnstructuredPointBoundaries::Destruct
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

void
avtUnstructuredPointBoundaries::Destruct(void *p)
{
    avtUnstructuredPointBoundaries *u = (avtUnstructuredPointBoundaries *)p;
    delete u;
}

 
// ****************************************************************************
//  Method:  avtUnstructuredPointBoundaries::ExchangeMesh
//
//  Purpose:
//    Generates information for parent class before calling its
//    ExchangeMesh method.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    mesh         an array of meshes
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

vector<vtkDataSet*>
avtUnstructuredPointBoundaries::ExchangeMesh(vector<int>       domainNum,
                                             vector<vtkDataSet*> meshes)
{
    Generate(domainNum, meshes);
    return avtUnstructuredDomainBoundaries::ExchangeMesh(domainNum, meshes);
}


// ****************************************************************************
//  Method:  avtUnstructuredPointBoundaries::ConfirmMesh
//
//  Purpose:
//    Generates information for parent class before calling its
//    ConfirmMesh method.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    mesh         an array of meshes
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

bool
avtUnstructuredPointBoundaries::ConfirmMesh(vector<int>       domainNum,
                                            vector<vtkDataSet*> meshes)
{
    Generate(domainNum, meshes);
    return avtUnstructuredDomainBoundaries::ConfirmMesh(domainNum, meshes);
}


// ****************************************************************************
//  Method:  avtUnstructuredPointBoundaries::CheckGenerated
//
//  Purpose:
//    Check if the information for two domains has been generated or not.
//    Also ensures that the generated vector is resized to modify those
//    settings.
//
//  Arguments:
//    d1        The domain sending.
//    d2        The domain receiving.
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

bool
avtUnstructuredPointBoundaries::CheckGenerated(int d1, int d2)
{
    if (d1 >= generated.size())
        generated.resize(d1 + 1);
    if (d2 >= generated[d1].size())
        generated[d1].resize(d2 + 1, false);
    return generated[d1][d2];
}


// ****************************************************************************
//  Method:  avtUnstructuredPointBoundaries::Generate
//
//  Purpose:
//    Generate given cell and point information based on the shared point
//    list and datasets. Does not generate information for pairs that have
//    been generated before.
//
//  Arguments:
//    domainNum     The domain numbers associated.
//    meshes        The datasets for the domains.
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
//  Modifications:
//    Brad Whitlock, Thu Sep 16 10:55:38 PDT 2004
//    I added separate Windows coding to make it work with the MSVC6 compiler.
//
// ****************************************************************************

void
avtUnstructuredPointBoundaries::Generate(vector<int> domainNum,
                                         vector<vtkDataSet *> meshes)
{
    map<int, int>::iterator miter;
    vtkIdList *cl = vtkIdList::New();
    vtkIdList *pl = vtkIdList::New();
    for (size_t i = 0; i < domainNum.size(); ++i)
    {
        int sendDom = domainNum[i];
        for (int recvDom = 0; recvDom < nTotalDomains; ++recvDom)
        {
            if (sendDom == recvDom || CheckGenerated(sendDom, recvDom))
                continue;

            generated[sendDom][recvDom] = true;

            int index = GetGivenIndex(sendDom, recvDom);

            // If there's no shared data, there's no given data.
            if (index < 0)
                continue; 

            map<int, int> &smap = sharedPointsMap[index];
            miter = smap.begin();

            vtkDataSet *ds = meshes[i];
            
            set<int> cells;
            set<int> points;

            // For each shared point, find the cells to give.
            // From those cells, find what points to give.
            for (; miter != smap.end(); ++miter)
            {
                ds->GetPointCells(miter->first, cl);

                for (int k = 0; k < cl->GetNumberOfIds(); ++k)
                {
                    int cellId = cl->GetId(k);
                    
                    //
                    // We shouldn't ghost 2D cells.
                    //
                    int type = ds->GetCellType(cellId);
                    if (type == VTK_LINE || type == VTK_TRIANGLE
                        || type == VTK_VERTEX || type == VTK_QUAD)
                        continue;
                    
                    cells.insert(cellId);
                    ds->GetCellPoints(cellId, pl);
                    for (int m = 0; m < pl->GetNumberOfIds(); ++m)
                        points.insert(pl->GetId(m));
                }
            }

            vector<int> givenCells, givenPoints;
#if defined(_MSC_VER) && (_MSC_VER <= 1200) // MSVC 6
            for(std::set<int>::const_iterator cell_it = cells.begin();
                cell_it != cells.end(); ++cell_it)
                givenCells.push_back(*cell_it);

            for(std::set<int>::const_iterator point_it = points.begin();
                point_it != points.end(); ++point_it)
                givenPoints.push_back(*point_it);
#else
            givenCells.assign(cells.begin(), cells.end());
            givenPoints.assign(points.begin(), points.end());
#endif
            // sendDom gives to recvDom with point filter on.
            SetGivenCellsAndPoints(sendDom, recvDom, givenCells, 
                                                     givenPoints, true);
        }
    }
    cl->Delete();
    pl->Delete();
}
