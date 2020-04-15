// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
    if ((size_t)d1 >= generated.size())
        generated.resize(d1 + 1);
    if ((size_t)d2 >= generated[d1].size())
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
//    Brad Whitlock, Thu Jun 26 14:21:20 PDT 2014
//    Permit ghosting for 2D cells so wireframe material plots work properly.
//
//    Eric Brugger, Fri Mar 13 15:20:08 PDT 2020
//    Modify to handle NULL meshes.
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
        if (meshes[i] == NULL)
            continue;
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
            
            set<vtkIdType> cells;
            set<vtkIdType> points;

            // For each shared point, find the cells to give.
            // From those cells, find what points to give.
            for (; miter != smap.end(); ++miter)
            {
                ds->GetPointCells(miter->first, cl);

                for (int k = 0; k < cl->GetNumberOfIds(); ++k)
                {
                    vtkIdType cellId = cl->GetId(k);
#if 0
                    //
                    // We shouldn't ghost 2D cells.
                    //
                    int type = ds->GetCellType(cellId);
                    if (type == VTK_LINE || type == VTK_TRIANGLE
                        || type == VTK_VERTEX || type == VTK_QUAD)
                        continue;
#endif
                    cells.insert(cellId);
                    ds->GetCellPoints(cellId, pl);
                    for (vtkIdType m = 0; m < pl->GetNumberOfIds(); ++m)
                        points.insert(pl->GetId(m));
                }
            }

            vector<int> givenCells, givenPoints;
#if defined(_MSC_VER) && (_MSC_VER <= 1200) // MSVC 6
            for(std::set<vtkIdType>::const_iterator cell_it = cells.begin();
                cell_it != cells.end(); ++cell_it)
                givenCells.push_back(*cell_it);

            for(std::set<vtkIdType>::const_iterator point_it = points.begin();
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
