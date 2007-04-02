/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

// ************************************************************************* //
//                             avtEulerianQuery.C                           //
// ************************************************************************* //

#include <avtEulerianQuery.h>

#include <snprintf.h>

#include <set>
#include <vector>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkGeometryFilter.h>
#include <vtkPolyData.h>

#include <avtQueryableSource.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <NonQueryableInputException.h>

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

using std::set;
using std::vector;
using std::string;



// ****************************************************************************
//  Method: avtEulerianQuery::avtEulerianQuery
//
//  Purpose:
//      Construct an avtEulerianQuery object.
//
//  Programmer:   Akira Haddox
//  Creation:     June 28, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 12:37:11 PST 2002 
//    Moved from avtEulerianFilter.  
//
// ****************************************************************************

avtEulerianQuery::avtEulerianQuery()
{
    gFilter = vtkGeometryFilter::New();
}

// ****************************************************************************
//  Method: avtEulerianQuery::~avtEulerianQuery
//
//  Purpose:
//      Destruct an avtEulerianQuery object.
//
//  Programmer:   Akira Haddox
//  Creation:     June 28, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 12:37:11 PST 2002 
//    Moved from avtEulerianFilter. 
//
// ****************************************************************************

avtEulerianQuery::~avtEulerianQuery()
{
    if (gFilter)
        gFilter->Delete();
}


// ****************************************************************************
//  Method: avtEulerianQuery::VerifyInput
//
//  Purpose:
//    Rejects non-queryable input && input that has topological dimension == 0
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 3, 2004
//
//  Modifications:
//
// ****************************************************************************

void
avtEulerianQuery::VerifyInput()
{
    avtDataObjectQuery::VerifyInput();
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
    {
        EXCEPTION1(NonQueryableInputException,
            "Requires plot with topological dimension > 0.");
    }
}





// ****************************************************************************
//  Method: avtEulerianQuery::Execute
//
//  Purpose:
//      Computes the Eulerian number of the input dataset. 
//
//  Arguments:
//      inDS      The input dataset.
//      dom       The domain number.
//
//  Programmer:   Akira Haddox
//  Creation:     June 28, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 12:37:11 PST 2002 
//    Moved from avtEulerianFilter::DeriveVariable.
//    Instead of creating a data array, keep track of the Eulerian
//    for this domain in domToEulerMap. 
//
//    Hank Childs, Tue Aug 24 08:47:57 PDT 2004
//    Manually remove ghost cells, since the geometry filter only removes
//    vtkGhostLevels, not vtkGhostNodes.
//
//    Kathleen Bonnell, Mon May 16 10:28:15 PDT 2005 
//    Fix memory leak. 
//
//    Kathleen Bonnell, Wed May 17 15:22:06 PDT 2006 
//    Remove call to SetSource(NULL) as it now removes information necessary
//    for the dataset. 
//
// ****************************************************************************

void 
avtEulerianQuery::Execute(vtkDataSet *in_ds, const int dom)
{
    vtkDataSet *nds = (vtkDataSet*) in_ds->NewInstance();
    nds->ShallowCopy(in_ds);
    //nds->SetSource(NULL);

    gFilter->SetInput(nds);
    vtkDataSetRemoveGhostCells *ghost_remover =
                                             vtkDataSetRemoveGhostCells::New();
    ghost_remover->SetInput(gFilter->GetOutput());
    vtkDataSet *out = ghost_remover->GetOutput();
    if (out->GetDataObjectType() != VTK_POLY_DATA)
    {
        // The output of a geometry filter should always be poly-data.
        EXCEPTION0(ImproperUseException);
    }

    vtkPolyData *pds = (vtkPolyData *) out;
    pds->Update();

    // I believe this isn't good enough. I believe the facelist filter
    // simply passes points through, and only modifies the cell structure.
    // As such, the number of points is greater than the number of points
    // that are actually used.
    int nPoints = pds->GetNumberOfPoints();
    // So we'll keep track manually of which points are used.
    vector<bool> pointsUsed(nPoints,false);

    int nCells = pds->GetNumberOfCells();

    int Eulerian;

    // We go through all the cells, and for each cell, add each edge of
    // that cell into our set of edges. Due to the nature of a set, and
    // that the ordering of nodes in an edgepair not mattering was specified
    // in the edgepair class, we will basically be able to tell how many
    // edges are in the mesh simply by the size of the set.

    set<edgepair> edges;

    int i,j;
    for (i = 0; i < nCells; i++)
    {
        vtkCell *cell=pds->GetCell(i);
        int numCellPoints = cell->GetNumberOfPoints();

        // Marked all the points in the cell as being used in our records
        for (int m = 0; m < numCellPoints; m++)
            pointsUsed[cell->GetPointId(m)] = true;

        switch (cell->GetCellType())
        {
        // Triangles, straight forward. Three points, three edges
            case VTK_TRIANGLE:
                edges.insert(edgepair(cell->GetPointId(0),
                                      cell->GetPointId(1)));
                edges.insert(edgepair(cell->GetPointId(1),
                                      cell->GetPointId(2)));
                edges.insert(edgepair(cell->GetPointId(2),
                                      cell->GetPointId(0)));
                break;

        // Quads, four points, four edges.
            case VTK_QUAD:
                edges.insert(edgepair(cell->GetPointId(0),
                                      cell->GetPointId(1)));
                edges.insert(edgepair(cell->GetPointId(1),
                                      cell->GetPointId(2)));
                edges.insert(edgepair(cell->GetPointId(2),
                                      cell->GetPointId(3)));
                edges.insert(edgepair(cell->GetPointId(3),
                                      cell->GetPointId(0)));
                break;

        // Polygons are also easy. There's an edge between node 0 and N,
        // and all the other edges are bewtween edge n and n+1
            case VTK_POLYGON:
                edges.insert(edgepair(cell->GetPointId(0),
                                      cell->GetPointId(numCellPoints-1)));
                for (j = 1; j < numCellPoints-1; j++)
                    edges.insert(edgepair(cell->GetPointId(j),
                                          cell->GetPointId(j+1)));
                break;

        //Each node [save for outer ones] is connected to two nodes
        //before them, and two after. Since we don't repeat edges,
        //we can just count edges infront of us to get them all
            case VTK_TRIANGLE_STRIP:
                for (j = 0; j < numCellPoints-2; j++)
                {
                    edges.insert(edgepair(cell->GetPointId(j),
                                          cell->GetPointId(j+1)));
                    edges.insert(edgepair(cell->GetPointId(j),
                                          cell->GetPointId(j+2)));
                }
                break;

            default:
                debug5 << "Geometry filter returned unexpected type." << endl;
                debug5 << "Type is: " << cell->GetCellType() << endl;
        }
    }    

    int numUsedPoints = 0;
    for (i = 0; i < pointsUsed.size(); i++)
        if (pointsUsed[i])
            ++numUsedPoints;

    // Now for the magic: Euler-Descartes formula
    Eulerian = numUsedPoints - edges.size() + nCells;

    domToEulerMap.insert(DomainToEulerMap::value_type(dom, Eulerian));
    ghost_remover->Delete();
    nds->Delete();
}



// ****************************************************************************
//  Method: avtEulerianQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//      If in parallel, gathers the Eulerian number for each domain onto one 
//      processor. 
//      Constructs an output string listing Eulerian number for each domain, 
//      for output to the gui window.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 11 16:31:45 PDT 2003
//    Renamed 'SetMessage' to 'SetResultMessage'.
//
//    Kathleen Bonnell, Mon Dec 22 16:45:56 PST 2003 
//    Use retrieved DomainName if available. 
//
//    Mark C. Miller, Wed Jun  9 21:50:12 PDT 2004
//    Eliminated use of MPI_ANY_TAG and modified to use GetUniqueMessageTags
//
// ****************************************************************************

void
avtEulerianQuery::PostExecute(void)
{
#ifdef PARALLEL
    int myRank, numProcs;
    int size, i, j;
    int *buf;
 
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    int mpiSizeTag = GetUniqueMessageTag();
    int mpiDataTag = GetUniqueMessageTag();
    if (myRank == 0)
    {
        for (i = 1; i < numProcs; i++)
        {
            MPI_Status stat, stat2;
             
            MPI_Recv(&size, 1, MPI_INT, MPI_ANY_SOURCE,
                     mpiSizeTag, MPI_COMM_WORLD, &stat);
            if (size > 0)
            {
                buf = new int [size];
                MPI_Recv(buf, size, MPI_INT, stat.MPI_SOURCE, mpiDataTag,
                         MPI_COMM_WORLD, &stat2);
                for (j = 0; j < size/2; j++)
                { 
                    domToEulerMap.insert(DomainToEulerMap::value_type(
                        buf[j*2], buf[j*2+1]));
                } 
                delete [] buf;
            }
        }
    }
    else
    {
        size = domToEulerMap.size() * 2;
        MPI_Send(&size, 1, MPI_INT, 0, mpiSizeTag, MPI_COMM_WORLD);
        if (size > 0)
        {
            buf = new int[size];
            DomainToEulerMap::iterator iter;
            for (iter = domToEulerMap.begin(), i = 0; 
                 iter != domToEulerMap.end(); iter++, i++)
            {
                buf[2*i] = (*iter).first;
                buf[2*i+1] = (*iter).second;
            }
            MPI_Send(buf, size, MPI_INT, 0, mpiDataTag, MPI_COMM_WORLD);
            delete [] buf;
        }
        return;
    }
#endif

    string msg;
    char msgBuff[500];
    DomainToEulerMap::iterator iter;
    int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
    for (iter = domToEulerMap.begin(); iter != domToEulerMap.end(); iter++)
    {
        string domainName;
        GetInput()->GetQueryableSource()->GetDomainName(queryAtts.GetVariables()[0],
            queryAtts.GetTimeStep(), (*iter).first, domainName);
        if (domainName.size() > 0)
        {
            SNPRINTF(msgBuff, 500, "Eulerian for %s is %d\n", 
                     domainName.c_str(), (*iter).second);
        }
        else
        {
            SNPRINTF(msgBuff, 500, "Eulerian for domain %d is %d\n", 
                     (*iter).first + blockOrigin, (*iter).second);
        }
        msg += msgBuff;
    }
    if (msg.size() == 0)
        msg = "Eulerian could not compute.\n" ;
    SetResultMessage(msg);
}


// ****************************************************************************
//  Method: avtEulerianQuery::PreExecute
//
//  Purpose:
//      This is called before any of the domains are executed.
//      Ensures the storage for Eulerian numbers is clear. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002
//
// ****************************************************************************

void
avtEulerianQuery::PreExecute()
{
   if (!domToEulerMap.empty())
       domToEulerMap.clear();
}


