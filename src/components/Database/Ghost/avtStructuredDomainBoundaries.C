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

// ************************************************************************* //
//                       avtStructuredDomainBoundaries.C                     //
// ************************************************************************* //

#include <avtStructuredDomainBoundaries.h>

#include <algorithm>

#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>

#include <avtGhostData.h>
#include <avtIntervalTree.h>
#include <avtMaterial.h>
#include <avtMixedVariable.h>
#include <avtParallel.h>

#include <TimingsManager.h>
#include <VisItException.h>


#ifdef PARALLEL
#include <mpi.h>
#endif


using   std::string;
using   std::vector;
using   std::sort;


// ----------------------------------------------------------------------------
//                            private helper methods
// ----------------------------------------------------------------------------

#ifdef PARALLEL
template <class T> MPI_Datatype GetMPIDataType();
template <>        MPI_Datatype GetMPIDataType<int>()    { return MPI_INT;  }
template <>        MPI_Datatype GetMPIDataType<float>()  { return MPI_FLOAT;}
template <>        MPI_Datatype GetMPIDataType<unsigned char>()  { return MPI_UNSIGNED_CHAR;}
#endif

// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::CreateDomainToProcessorMap
//
//  Purpose:
//    Create an array of (ndomains) integers with the rank of the processor
//    (0..nproc-1) which owns each domain, or -1 if no processor owns it.
//
//  Arguments:
//    domainNum    an array of domain numbers owned by the current processor
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 20, 2001
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************
vector<int>
avtStructuredDomainBoundaries::CreateDomainToProcessorMap(const vector<int> &domainNum)
{
    // get the processor rank
    int rank = 0;
#ifdef PARALLEL
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
#endif

    // find the number of domains
    int ntotaldomains = wholeBoundary.size();

    // create the map
    vector<int> domain2proc(ntotaldomains, -1);
    for (int d=0; d<domainNum.size(); d++)
        domain2proc[domainNum[d]] = rank;
#ifdef PARALLEL
    vector<int> domain2proc_tmp(domain2proc);
    MPI_Allreduce(&domain2proc_tmp[0], &domain2proc[0], ntotaldomains, MPI_INT,
                  MPI_MAX, VISIT_MPI_COMM);
#endif

    return domain2proc;
}

// ****************************************************************************
//  Method: avtStructuredDomainBoundaries::CreateCurrentDomainBoundaryInformation
//
//  Purpose:
//    Create the boundaries appropriate for the current domain selection.
//
//  Arguments:
//    domain2proc   the map of domains to processors
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 20, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Jun 27 10:02:55 PDT 2005
//    Added timing info.
//
//    Hank Childs, Wed Jul  6 06:50:55 PDT 2005
//    Add argument to DeleteNeighbor.
//
// ****************************************************************************
void
avtStructuredDomainBoundaries::CreateCurrentDomainBoundaryInformation(
                                                const vector<int> &domain2proc)
{
    int t0 = visitTimer->StartTimer();
    boundary = wholeBoundary;
    for (int i=0; i<wholeBoundary.size(); i++)
    {
        Boundary &wbi = wholeBoundary[i];
        if (domain2proc[i] < 0)
        {
            boundary[i].neighbors.clear();
            boundary[i].domain = -1;
            continue;
        }

        for (int j=0; j<wbi.neighbors.size(); j++)
        {
            if (domain2proc[wbi.neighbors[j].domain] < 0)
                boundary[i].DeleteNeighbor(wbi.neighbors[j].domain, boundary);
        }
    }
    visitTimer->StopTimer(t0, "avtStructuredDomainBoundaries::CurrentDBI");
}

// ****************************************************************************
//  Method:  BoundaryHelperFunctions::InitializeBoundaryData
//
//  Purpose:
//    Allocate some temporary storage structure for the boundary data.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Dec 13 11:54:58 PST 2001
//    Templatized this function.
//
// ****************************************************************************
template <class T>
T***
BoundaryHelperFunctions<T>::InitializeBoundaryData()
{
    T ***data = new T**[sdb->boundary.size()];
    for (int b = 0; b < sdb->boundary.size(); b++)
    {
        Boundary *bi = &sdb->boundary[b];
        data[b] = new T*[bi->neighbors.size()];
        for (int n = 0; n < bi->neighbors.size(); n++)
            data[b][n] = NULL;
    }
    return data;
}

// ****************************************************************************
//  Method:  BoundaryHelperFunctions::FillBoundaryData
//
//  Purpose:
//    Fill the temporary boundary data from a single domain, allocating
//    the actual data storage array.
//
//  Arguments:
//    d1             the domain number
//    olddata        the mesh-sized array containing the original data
//    bnddata        the temporary boundary data
//    isPointData    true if this is node-centered, false if cell-centered
//    ncomp          1 for scalar data, >1 for vector data
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Dec 13 11:54:58 PST 2001
//    Templatized this function.
//
//    Hank Childs, Wed Jul  6 06:50:55 PDT 2005
//    Instead of calculating index into domain's neighbor list in a non-robust
//    way, use the "match", which is already pre-computed by the client for
//    this purpose.
//
// ****************************************************************************
template <class T>
void
BoundaryHelperFunctions<T>::FillBoundaryData(int      d1,
                                                const T *olddata,
                                                T     ***bnddata,
                                                bool     isPointData,
                                                int      ncomp)
{
    Boundary *bi = &sdb->boundary[d1];
    for (int n = 0; n < bi->neighbors.size(); n++)
    {
        Neighbor *n1 = &bi->neighbors[n];
        if (isPointData)
            bnddata[d1][n] = new T[n1->npts * ncomp];
        else
            bnddata[d1][n] = new T[n1->ncells * ncomp];

        int d2 = n1->domain;
        int mi = n1->match;
        Neighbor *n2 = &(sdb->boundary[d2].neighbors[mi]);
                
        int *n2extents = (isPointData ? n2->nextents : n2->zextents);
        int bndindex = 0;
        for (int k=n2extents[4]; k<=n2extents[5]; k++)
        {
            for (int j=n2extents[2]; j<=n2extents[3]; j++)
            {
                for (int i=n2extents[0]; i<=n2extents[1]; i++)
                {
                    int oldindex;
                    if (isPointData)
                        oldindex = bi->TranslatedPointIndex(n2,n1, i,j,k);
                    else
                        oldindex = bi->TranslatedCellIndex(n2,n1, i,j,k);
                    for (int c=0; c<ncomp; c++)
                    {
                        bnddata[d1][n][bndindex*ncomp + c] = olddata[oldindex*ncomp + c];
                    }
                    bndindex++;
                }
            }
        }
    }
}

// ****************************************************************************
//  Method:  BoundaryHelperFunctions::FillRectilinearBoundaryData
//
//  Purpose:
//    Fill the temporary boundary data from the coordinates of a single
//    rectilinear domain, as well as allocating the actual data storage array.
//
//  Arguments:
//    d1             the domain number
//    x              the x-coordinates
//    y              the y-coordinates
//    z              the z-coordinates
//    bnddata        the temporary boundary data
//
//  Programmer:  Hank Childs
//  Creation:    November 10, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Nov 13 08:56:18 PST 2003
//    Removed unused variables.
//
//    Hank Childs, Wed Jul  6 06:50:55 PDT 2005
//    Instead of calculating index into domain's neighbor list in a non-robust
//    way, use the "match", which is already pre-computed by the client for
//    this purpose.
//
// ****************************************************************************
template <class T>
void
BoundaryHelperFunctions<T>::FillRectilinearBoundaryData(int      d1,
                                                        const T *x,
                                                        const T *y,
                                                        const T *z,
                                                        T     ***bnddata)
{
    Boundary *bi = &sdb->boundary[d1];
    int *oldbiextents = bi->oldnextents;
    int nIold = oldbiextents[1] - oldbiextents[0] + 1;
    int nJold = oldbiextents[3] - oldbiextents[2] + 1;
    int nKold = oldbiextents[5] - oldbiextents[4] + 1;
    for (int n = 0; n < bi->neighbors.size(); n++)
    {
        Neighbor *n1 = &bi->neighbors[n];
        bnddata[d1][n] = new T[n1->npts*3];

        int d2 = n1->domain;
        int mi = n1->match;
        Neighbor *n2 = &(sdb->boundary[d2].neighbors[mi]);
                
        int *n2extents = n2->nextents;
        int bndindex = 0;
        for (int k=n2extents[4]; k<=n2extents[5]; k++)
        {
            for (int j=n2extents[2]; j<=n2extents[3]; j++)
            {
                for (int i=n2extents[0]; i<=n2extents[1]; i++)
                {
                    int ptId = bi->TranslatedPointIndex(n2, n1, i, j, k);
                    int oldI = (ptId % nIold);
                    int oldJ = ((ptId/nIold) % nJold);
                    int oldK = ((ptId/(nIold*nJold)) % nKold);
                    bnddata[d1][n][bndindex*3 + 0] = x[oldI];
                    bnddata[d1][n][bndindex*3 + 1] = y[oldJ];
                    bnddata[d1][n][bndindex*3 + 2] = z[oldK];
                    bndindex++;
                }
            }
        }
    }
}

// ****************************************************************************
//  Method:  BoundaryHelperFunctions::FillMixedBoundaryData
//
//  Purpose:
//    Fill the temporary mixed boundary data from a single domain, allocating
//    the actual data storage array.
//
//  Arguments:
//    d1             the domain number
//    oldmat         the old material
//    olddata        the mesh-sized array containing the original data
//    bnddata        the temporary boundary data
//    bndmixmat      the temporary boundary material numbers
//    bndmixzone     the temporary boundary zone numbers
//    bndmixlen      the temporary boundary mixlen
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
//  Note:  bnddata, bndmixmat, and bndmixzone may each be NULL.  
//         olddata may be NULL as well as long as the mixlen is zero.
//
//  Modifications:
//
//    Hank Childs, Wed Jul  6 06:50:55 PDT 2005
//    Instead of calculating index into domain's neighbor list in a non-robust
//    way, use the "match", which is already pre-computed by the client for
//    this purpose.
//
// ****************************************************************************
template <class T>
void
BoundaryHelperFunctions<T>::FillMixedBoundaryData(int          d1,
                                                     avtMaterial *oldmat,
                                                     const T     *olddata,
                                                     T         ***bnddata,
                                                     int       ***bndmixmat,
                                                     int       ***bndmixzone,
                                                     vector<int> &bndmixlen)
{
    Boundary *bi = &sdb->boundary[d1];
    int k;

    for (int n = 0; n < bi->neighbors.size(); n++)
    {
        Neighbor *n1 = &bi->neighbors[n];
        int d2 = n1->domain;
        int mi = n1->match;
        Neighbor *n2 = &(sdb->boundary[d2].neighbors[mi]);
        int *n2extents = n2->zextents;

        for (k=n2extents[4]; k<=n2extents[5]; k++)
        {
            for (int j=n2extents[2]; j<=n2extents[3]; j++)
            {
                for (int i=n2extents[0]; i<=n2extents[1]; i++)
                {
                    int oldindex = bi->TranslatedCellIndex(n2,n1, i,j,k);
                    int oldmatno = oldmat->GetMatlist()[oldindex];
                    if (oldmatno < 0)
                    {
                        int oldmixindex = -oldmatno - 1;
                        while (oldmixindex >= 0)
                        {
                            oldmixindex = oldmat->GetMixNext()[oldmixindex] - 1;
                            bndmixlen[n]++;
                        }
                    }
                }
            }
        }

        if (bnddata)
            bnddata[d1][n]    = new T[bndmixlen[n]];
        if (bndmixmat)
            bndmixmat[d1][n]  = new int[bndmixlen[n]];
        if (bndmixzone)
            bndmixzone[d1][n] = new int[bndmixlen[n]];

        int bndindex = 0;
        for (k=n2extents[4]; k<=n2extents[5]; k++)
        {
            for (int j=n2extents[2]; j<=n2extents[3]; j++)
            {
                for (int i=n2extents[0]; i<=n2extents[1]; i++)
                {
                    int oldindex;
                    oldindex = bi->TranslatedCellIndex(n2,n1, i,j,k);
                    int oldmatno = oldmat->GetMatlist()[oldindex];
                    if (oldmatno < 0)
                    {
                        int oldmixindex = -oldmatno - 1;
                        while (oldmixindex >= 0)
                        {
                            if (bnddata)
                                bnddata[d1][n][bndindex]    = olddata[oldmixindex];
                            if (bndmixmat)
                                bndmixmat[d1][n][bndindex]  = oldmat->GetMixMat()[oldmixindex];
                            if (bndmixzone)
                                bndmixzone[d1][n][bndindex] = oldmat->GetMixZone()[oldmixindex];
                            oldmixindex = oldmat->GetMixNext()[oldmixindex] - 1;
                            bndindex++;
                        }
                    }
                }
            }
        }
    }
}

// ****************************************************************************
//  Method:  BoundaryHelperFunctions::CommunicateBoundaryData
//
//  Purpose:
//    Send data needed by other processors to those processors, and receive
//    any boundary data we still need.
//
//  Arguments:
//    domain2proc    the map of domains to processors
//    bnddata        the temporary boundary data
//    isPointData    true if this is node-centered, false if cell-centered
//    ncomp          1 for scalar data, >1 for vector data
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Dec 13 11:54:58 PST 2001
//    Templatized this function.
//
//    Mark C. Miller, Wed Jun  9 21:50:12 PDT 2004
//    Eliminated use of MPI_ANY_TAG and modified to use GetUniqueMessageTags
//
//    Hank Childs, Thu Jun 23 10:27:01 PDT 2005
//    Re-wrote using all-to-all for efficiency.
//
//    Brad Whitlock, Mon Nov 7 09:26:56 PDT 2005
//    I made it use MPI_Datatype for the return type of GetMPIDataType so
//    it can build with LAM.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************
template <class T>
void
BoundaryHelperFunctions<T>::CommunicateBoundaryData(const vector<int> &domain2proc,
                                                       T     ***bnddata,
                                                       bool     isPointData,
                                                       int      ncomp)
{
#ifdef PARALLEL
    MPI_Datatype mpi_datatype = GetMPIDataType<T>();

    int mpiMsgTag = GetUniqueMessageTag();

    int rank;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);

    int nprocs;
    MPI_Comm_size(VISIT_MPI_COMM, &nprocs);

    int *sendcount = new int[nprocs];
    int *recvcount = new int[nprocs];
    int i;
    for (i = 0 ; i < nprocs ; i++)
    {
        sendcount[i] = 0;
        recvcount[i] = 0;
    }

    for (int d1 = 0; d1 < sdb->boundary.size(); d1++)
    {
        Boundary *bi = &sdb->boundary[d1];

        // find matching neighbors
        for (int n=0; n<bi->neighbors.size(); n++)
        {
            Neighbor *n1 = &bi->neighbors[n];
            int d2 = n1->domain;
            int size = ncomp * (isPointData ? n1->npts : n1->ncells);

            if (domain2proc[d1] != domain2proc[d2])
            {
                if (domain2proc[d1] == rank)
                    sendcount[domain2proc[d2]] += size;
                else if (domain2proc[d2] == rank)
                    recvcount[domain2proc[d1]] += size;
            }
        }
    }

    int *senddisp = new int[nprocs];
    int *recvdisp = new int[nprocs];
    senddisp[0] = recvdisp[0] = 0;
    for (i = 1 ; i < nprocs ; i++)
    {
        senddisp[i] = senddisp[i-1] + sendcount[i-1];
        recvdisp[i] = recvdisp[i-1] + recvcount[i-1];
    }
    int total_send = 0;
    int total_recv = 0;
    for (i = 0 ; i < nprocs ; i++)
    {
        total_send += sendcount[i];
        total_recv += recvcount[i];
    }

    T *sendbuff = new T[total_send];
    T *recvbuff = new T[total_recv];

    T **tmp_ptr = new T*[nprocs];
    for (i = 0 ; i < nprocs ; i++)
        tmp_ptr[i] = sendbuff + senddisp[i];
    for (int d1 = 0; d1 < sdb->boundary.size(); d1++)
    {
        Boundary *bi = &sdb->boundary[d1];

        // find matching neighbors
        for (int n=0; n<bi->neighbors.size(); n++)
        {
            Neighbor *n1 = &bi->neighbors[n];
            int d2 = n1->domain;
            int size = ncomp * (isPointData ? n1->npts : n1->ncells);

            if (domain2proc[d1] != domain2proc[d2])
                if (domain2proc[d1] == rank)
                    for (i = 0 ; i < size ; i++)
                    {
                        int p2 = domain2proc[d2];
                        *(tmp_ptr[p2]) = bnddata[d1][n][i];
                        (tmp_ptr[p2])++;
                    }
        }
    }

    MPI_Alltoallv(sendbuff, sendcount, senddisp, mpi_datatype,
                  recvbuff, recvcount, recvdisp, mpi_datatype,
                  VISIT_MPI_COMM);

    for (i = 0 ; i < nprocs ; i++)
        tmp_ptr[i] = recvbuff + recvdisp[i];
    for (int d1 = 0; d1 < sdb->boundary.size(); d1++)
    {
        Boundary *bi = &sdb->boundary[d1];

        // find matching neighbors
        for (int n=0; n<bi->neighbors.size(); n++)
        {
            Neighbor *n1 = &bi->neighbors[n];
            int d2 = n1->domain;
            int size = ncomp * (isPointData ? n1->npts : n1->ncells);

            if (domain2proc[d1] != domain2proc[d2])
            {
                if (domain2proc[d2] == rank)
                {
                    bnddata[d1][n] = new T[size];
                    for (i = 0 ; i < size ; i++)
                    {
                        int p2 = domain2proc[d1];
                        bnddata[d1][n][i] = *(tmp_ptr[p2]);
                        (tmp_ptr[p2])++;
                    }
                }
            }
        }
    }
    MPI_Barrier(VISIT_MPI_COMM);

    delete [] sendbuff;
    delete [] recvbuff;
    delete [] senddisp;
    delete [] recvdisp;
    delete [] sendcount;
    delete [] recvcount;
#endif
}

// ****************************************************************************
//  Method:  BoundaryHelperFunctions::CommunicateMixedBoundaryData
//
//  Purpose:
//    Communicate the boundary data for mixed cells.
//
//  Arguments:
//    domain2proc    the map of domains to processors
//    bnddata        the temporary boundary data
//    bndmixmat      the temporary boundary material numbers
//    bndmixzone     the temporary boundary zone numbers
//    bndmixlen      the temporary boundary mixlen
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
//  Note:  bndmixdata, bndmixmat, and bndmixzone may each be NULL
//
//  Modifications:
//
//    Mark C. Miller, Wed Jun  9 21:50:12 PDT 2004
//    Eliminated use of MPI_ANY_TAG and modified to use GetUniqueMessageTags
//
//    Hank Childs, Wed Jul  6 06:50:55 PDT 2005
//    Instead of calculating index into domain's neighbor list in a non-robust
//    way, use the "match", which is already pre-computed by the client for
//    this purpose.
//
//    Brad Whitlock, Mon Nov 7 09:26:56 PDT 2005
//    I made it use MPI_Datatype for the return type of GetMPIDataType so
//    it can build with LAM.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************
template <class T>
void
BoundaryHelperFunctions<T>::CommunicateMixedBoundaryData(const vector<int> &domain2proc,
                                                            T     ***bnddata,
                                                            int   ***bndmixmat,
                                                            int   ***bndmixzone,
                                                            vector< vector<int> > &bndmixlen)
{
#ifdef PARALLEL

    MPI_Datatype mpi_datatype = GetMPIDataType<T>();
    MPI_Status stat;

    int rank;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);

    int mpiMsgTag = GetUniqueMessageTag();

    for (int d1 = 0; d1 < sdb->boundary.size(); d1++)
    {
        Boundary *bi = &sdb->boundary[d1];

        // find matching neighbors
        for (int n=0; n<bi->neighbors.size(); n++)
        {
            Neighbor *n1 = &bi->neighbors[n];
            int d2 = n1->domain;
            int mi = n1->match;

            if (domain2proc[d1] != domain2proc[d2])
            {
                if (domain2proc[d1] == rank)
                {
                    MPI_Send(&(bndmixlen[d1][n]), 1, MPI_INT,
                             domain2proc[d2], mpiMsgTag, VISIT_MPI_COMM);
                }
                else if (domain2proc[d2] == rank)
                {
                    MPI_Recv(&(bndmixlen[d1][n]), 1, MPI_INT,
                             domain2proc[d1], mpiMsgTag, VISIT_MPI_COMM, &stat);
                }
            }
        }
    }

    int mpiBndDataTag    = GetUniqueMessageTag();
    int mpiBndMixMatTag  = GetUniqueMessageTag();
    int mpiBndMixZoneTag = GetUniqueMessageTag();

    for (int d1 = 0; d1 < sdb->boundary.size(); d1++)
    {
        Boundary *bi = &sdb->boundary[d1];

        // find matching neighbors
        for (int n=0; n<bi->neighbors.size(); n++)
        {
            Neighbor *n1 = &bi->neighbors[n];
            int d2 = n1->domain;
            int mi = n1->match;
            int size = bndmixlen[d1][n];

            if (domain2proc[d1] != domain2proc[d2])
            {
            
                if (domain2proc[d1] == rank)
                {
                    if (bnddata)
                        MPI_Send(bnddata[d1][n], size, mpi_datatype,
                                 domain2proc[d2], mpiBndDataTag, VISIT_MPI_COMM);
                    if (bndmixmat)
                        MPI_Send(bndmixmat[d1][n], size, MPI_INT,
                                 domain2proc[d2], mpiBndMixMatTag, VISIT_MPI_COMM);
                    if (bndmixzone)
                        MPI_Send(bndmixzone[d1][n], size, MPI_INT,
                                 domain2proc[d2], mpiBndMixZoneTag, VISIT_MPI_COMM);
                }
                else if (domain2proc[d2] == rank)
                {
                    if (bnddata)
                    {
                        bnddata[d1][n] = new T[size];
                        MPI_Recv(bnddata[d1][n], size, mpi_datatype,
                                 domain2proc[d1], mpiBndDataTag, VISIT_MPI_COMM, &stat);
                    }
                    if (bndmixmat)
                    {
                        bndmixmat[d1][n] = new int[size];
                        MPI_Recv(bndmixmat[d1][n], size, MPI_INT,
                                 domain2proc[d1], mpiBndMixMatTag, VISIT_MPI_COMM, &stat);
                    }
                    if (bndmixzone)
                    {
                        bndmixzone[d1][n] = new int[size];
                        MPI_Recv(bndmixzone[d1][n], size, MPI_INT,
                                 domain2proc[d1], mpiBndMixZoneTag, VISIT_MPI_COMM, &stat);
                    }
                }
            }
        }
    }

    MPI_Barrier(VISIT_MPI_COMM);
#endif
}

// ****************************************************************************
//  Method:  BoundaryHelperFunctions::CopyOldValues
//
//  Purpose:
//    Copy the values from the old mesh's data array to the new one.
//
//  Arguments:
//    d1             the domain number
//    olddata        the mesh-sized array which holds the original data
//    newdata        the mesh-sized array which holds the new data
//    isPointData    true if this is node-centered, false if cell-centered
//    ncomp          1 for scalar data, >1 for vector data
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Dec 13 11:54:58 PST 2001
//    Templatized this function.
//
// ****************************************************************************
template <class T>
void
BoundaryHelperFunctions<T>::CopyOldValues(int      d1,
                                             const T *olddata,
                                             T       *newdata,
                                             bool     isPointData,
                                             int      ncomp)
{
    Boundary *bi = &sdb->boundary[d1];
    int *biextents = (isPointData ? bi->oldnextents : bi->oldzextents);
    //int *biextents = (isPointData ? bi->newnextents : bi->newzextents);
    for (int k = biextents[4]; k <= biextents[5]; k++)
    {
        for (int j = biextents[2]; j <= biextents[3]; j++)
        {
            for (int i = biextents[0]; i <= biextents[1]; i++)
            {
                int oldindex, newindex;
                if (isPointData)
                {
                    oldindex = bi->OldPointIndex(i,j,k);
                    newindex = bi->NewPointIndex(i,j,k);
                }
                else
                {
                    oldindex = bi->OldCellIndex(i,j,k);
                    newindex = bi->NewCellIndex(i,j,k);
                }

                for (int c=0; c<ncomp; c++)
                    newdata[newindex*ncomp + c] = olddata[oldindex*ncomp + c];
            }
        }
    }
}

// ****************************************************************************
//  Method:  BoundaryHelperFunctions::CopyOldRectilinearValues
//
//  Purpose:
//    Copy the values from the old rectilinear mesh's data array to the new
//    one.
//
//  Arguments:
//    d1             the domain number
//    olddata        the floats for a certain dimension
//    newdata        the floats for a certain dimension
//    comp_num       0=X, 1=Y, 2=Z
//
//  Programmer:  Hank Childs
//  Creation:    November 11, 2003 
//
// ****************************************************************************
template <class T>
void
BoundaryHelperFunctions<T>::CopyOldRectilinearValues(int d1, const T *olddata, 
                                                     T *newdata, int comp_num)
{
    Boundary *bi = &sdb->boundary[d1];
    int *oldbiextents = bi->oldnextents;
    int *newbiextents = bi->newnextents;
    int nIold = oldbiextents[1] - oldbiextents[0] + 1;
    int nJold = oldbiextents[3] - oldbiextents[2] + 1;
    int nKold = oldbiextents[5] - oldbiextents[4] + 1;
    int nInew = newbiextents[1] - newbiextents[0] + 1;
    int nJnew = newbiextents[3] - newbiextents[2] + 1;
    int nKnew = newbiextents[5] - newbiextents[4] + 1;
    
    if (comp_num == 0)
    {
        //
        // We want to copy over all of the X-values.  Since we have existing
        // infrastructure that will translate indices for us in terms of
        // 3-tuples (i,j,k), use that infrastructure.  Copy over all of the
        // (I, j-min, k-min), where I goes from i-min to i-max.
        //
        int j_ind = oldbiextents[2];
        int k_ind = oldbiextents[4];
        for (int i_ind = oldbiextents[0] ; i_ind <= oldbiextents[1] ; i_ind++)
        {
            int oldindex = bi->OldPointIndex(i_ind, j_ind, k_ind);
            int newindex = bi->NewPointIndex(i_ind, j_ind, k_ind);
            int oldI = oldindex % nIold;
            int newI = newindex % nInew;
            newdata[newI] = olddata[oldI]; 
        }
    }
    else if (comp_num == 1)
    {
        int i_ind = oldbiextents[0];
        int k_ind = oldbiextents[4];
        for (int j_ind = oldbiextents[2] ; j_ind <= oldbiextents[3] ; j_ind++)
        {
            int oldindex = bi->OldPointIndex(i_ind, j_ind, k_ind);
            int newindex = bi->NewPointIndex(i_ind, j_ind, k_ind);
            int oldJ = (oldindex/nIold) % nJold;
            int newJ = (newindex/nInew) % nJnew;
            newdata[newJ] = olddata[oldJ]; 
        }
    }
    else if (comp_num == 2)
    {
        int i_ind = oldbiextents[0];
        int j_ind = oldbiextents[2];
        for (int k_ind = oldbiextents[4] ; k_ind <= oldbiextents[5] ; k_ind++)
        {
            int oldindex = bi->OldPointIndex(i_ind, j_ind, k_ind);
            int newindex = bi->NewPointIndex(i_ind, j_ind, k_ind);
            int oldK = (oldindex/(nIold*nJold)) % nKold;
            int newK = (newindex/(nInew*nJnew)) % nKnew;
            newdata[newK] = olddata[oldK]; 
        }
    }
}

// ****************************************************************************
//  Method:  BoundaryHelperFunctions::CopyOldMixedValues
//
//  Purpose:
//    Copy the values from the old mesh's data array to the new one.
//
//  Arguments:
//    oldmat         the old material
//    olddata        the mixlen-sized array which holds the original data
//    newdata        the mixlen-sized array which holds the new data
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
//  Note:  olddata may be NULL if the mixlen is zero.
//
// ****************************************************************************
template <class T>
void
BoundaryHelperFunctions<T>::CopyOldMixedValues(avtMaterial *oldmat,
                                                  const T     *olddata,
                                                  T           *newdata)
{
    for (int i=0; i<oldmat->GetMixlen(); i++)
        newdata[i] = olddata[i];
}

// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::SetExistence
//
//  Purpose:
//    Create a cell or node-length array containing false if a cell/node
//    does not actually exist, true otherwise.
//
//  Arguments:
//    d1             the domain number
//    isPointData    true if this is node-centered, false if cell-centered
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
bool*
avtStructuredDomainBoundaries::SetExistence(int      d1,
                                            bool     isPointData)
{
    Boundary *bi = &boundary[d1];
    int       len = (isPointData ? bi->newnpts : bi->newncells);
    bool     *exists = new bool[len];
    for (int i=0; i<len; i++)
        exists[i] = false;

    // set the old domain's cells to exist
    int *biextents = (isPointData ? bi->oldnextents : bi->oldzextents);
    for (int k = biextents[4]; k <= biextents[5]; k++)
    {
        for (int j = biextents[2]; j <= biextents[3]; j++)
        {
            for (int i = biextents[0]; i <= biextents[1]; i++)
            {
                int index;
                if (isPointData)
                    index = bi->NewPointIndex(i,j,k);
                else
                    index = bi->NewCellIndex(i,j,k);
                exists[index] = true;
            }
        }
    }

    // set any available boundary to exist
    for (int n=0; n<bi->neighbors.size(); n++)
    {
        Neighbor *n1 = &bi->neighbors[n];
        int *n1extents = (isPointData ? n1->nextents : n1->zextents);
        for (int k=n1extents[4]; k<=n1extents[5]; k++)
        {
            for (int j=n1extents[2]; j<=n1extents[3]; j++)
            {
                for (int i=n1extents[0]; i<=n1extents[1]; i++)
                {
                    int index = (isPointData ? 
                                 bi->NewPointIndexFromNeighbor(n1, i,j,k) :
                                 bi->NewCellIndexFromNeighbor(n1, i,j,k));
                    if (index >= 0)
                        exists[index] = true;
                }
            }
        }
    }

    return exists;
}

// ****************************************************************************
//  Method:  BoundaryHelperFunctions::SetNewBoundaryData
//
//  Purpose:
//    Set the ghost values of the given domain using the temporary 
//    boundary data.
//
//  Arguments:
//    d1             the domain number
//    bnddata        the temporary boundary data
//    newdata        the mesh-sized array which holds the new data
//    isPointData    true if this is node-centered, false if cell-centered
//    ncomp          1 for scalar data, >1 for vector data
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Dec 13 11:54:58 PST 2001
//    Templatized this function.
//
//    Hank Childs, Wed Jul  6 06:50:55 PDT 2005
//    Instead of calculating index into domain's neighbor list in a non-robust
//    way, use the "match", which is already pre-computed by the client for
//    this purpose.
//
// ****************************************************************************
template <class T>
void
BoundaryHelperFunctions<T>::SetNewBoundaryData(int       d1,
                                                  T      ***bnddata,
                                                  T        *newdata,
                                                  bool      isPointData,
                                                  int       ncomp)
{
    Boundary *bi = &sdb->boundary[d1];
    for (int n=0; n<bi->neighbors.size(); n++)
    {
        Neighbor *n1 = &bi->neighbors[n];
        int d2 = n1->domain;

        int mi = n1->match;
        T *data = bnddata[d2][mi];
        if (!data)
            EXCEPTION1(VisItException,"Null array");

        int bndindex = 0;
        int *n1extents = (isPointData ? n1->nextents : n1->zextents);
        for (int k=n1extents[4]; k<=n1extents[5]; k++)
        {
            for (int j=n1extents[2]; j<=n1extents[3]; j++)
            {
                for (int i=n1extents[0]; i<=n1extents[1]; i++)
                {
                    int newindex = (isPointData ? 
                                    bi->NewPointIndexFromNeighbor(n1, i,j,k) :
                                    bi->NewCellIndexFromNeighbor(n1, i,j,k));
                    if (newindex >= 0)
                    {
                        for (int c=0; c<ncomp; c++)
                            newdata[newindex*ncomp + c] = data[bndindex*ncomp + c];
                    }
                    bndindex++;
                }
            }
        }
    }
}

// ****************************************************************************
//  Method:  BoundaryHelperFunctions::SetNewRectilinearBoundaryData
//
//  Purpose:
//    Set the ghost values of the given domain using the temporary 
//    boundary data.
//
//  Arguments:
//    d1             the domain number
//    coord          the coordinates to set.
//    newx           the new x-coordinates.
//    newy           the new y-coordinates.
//    newz           the new z-coordinates.
//
//  Programmer:  Hank Childs
//  Creation:    November 11, 2003
//
//  Modifications:
//
//    Hank Childs, Wed Jul  6 06:50:55 PDT 2005
//    Instead of calculating index into domain's neighbor list in a non-robust
//    way, use the "match", which is already pre-computed by the client for
//    this purpose.
//
// ****************************************************************************
template <class T>
void
BoundaryHelperFunctions<T>::SetNewRectilinearBoundaryData(int d1,
                                         T ***coord, T *newx, T *newy, T *newz)
{
    Boundary *bi = &sdb->boundary[d1];
    int *newbiextents = bi->newnextents;
    int nInew = newbiextents[1] - newbiextents[0] + 1;
    int nJnew = newbiextents[3] - newbiextents[2] + 1;
    int nKnew = newbiextents[5] - newbiextents[4] + 1;

    for (int n=0; n<bi->neighbors.size(); n++)
    {
        Neighbor *n1 = &bi->neighbors[n];
        int d2 = n1->domain;
        int mi = n1->match;

        T *data = coord[d2][mi];
        if (!data)
            EXCEPTION1(VisItException,"Null array");

        int bndindex = 0;
        int *n1extents = n1->nextents;
        for (int k=n1extents[4]; k<=n1extents[5]; k++)
        {
            for (int j=n1extents[2]; j<=n1extents[3]; j++)
            {
                for (int i=n1extents[0]; i<=n1extents[1]; i++)
                {
                    int newindex = bi->NewPointIndexFromNeighbor(n1, i,j,k);
                    if (newindex >= 0)
                    {
                        int newI = (newindex % nInew);
                        int newJ = ((newindex/nInew) % nJnew);
                        int newK = ((newindex/(nInew*nJnew)) % nKnew);
                        newx[newI] = data[bndindex*3 + 0];
                        newy[newJ] = data[bndindex*3 + 1];
                        newz[newK] = data[bndindex*3 + 2];
                    }
                    bndindex++;
                }
            }
        }
    }
}

// ****************************************************************************
//  Method:  BoundaryHelperFunctions::SetNewMixedBoundaryData
//
//  Purpose:
//    Set the ghost values of the mixed data for the given domain using the
//    temporary mixed boundary data.
//
//  Arguments:
//    d1             the domain number
//    oldmat         the old material
//    bndmixlen      the temporary boundary mixlen
//    bnddata        the temporary boundary data
//    bndmixmat      the temporary boundary material numbers
//    bndmixzone     the temporary boundary zone numbers
//    newmatlist     the new mesh-sized array which holds the matlist
//    newdata        the new mixed array which holds the data
//    newmixmat      the new mixed array which holds the material numbers
//    newmixzone     the new mixed array which holds the zone numbers
//    newmixnext     the new mixed array which holds the mix_next data
//
//  Output Arguments:
//    newmixlen      the actual length of the new mixed array.
//
//  Note:  bndmixmat, newmixmat, newmixzone, and newmixnext may all be NULL
//         newmixlen must be returned because until it starts out as a
//          maximum mixlen of the new domain with ghost zones, but if we
//          have unselected some domains, it may be much shorter
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
//  Modifications:
//    Jeremy Meredith, Mon Oct 28 19:29:12 PST 2002
//    Added newmixlen as an output argument.
//
//    Hank Childs, Wed Jul  6 06:50:55 PDT 2005
//    Instead of calculating index into domain's neighbor list in a non-robust
//    way, use the "match", which is already pre-computed by the client for
//    this purpose.
//
// ****************************************************************************
template <class T>
void
BoundaryHelperFunctions<T>::SetNewMixedBoundaryData(int       d1,
                                                    avtMaterial *oldmat,
                                     const vector< vector<int> > &bndmixlen,
                                                       int    ***bndmatlist,
                                                       T      ***bnddata,
                                                       int    ***bndmixmat,
                                                       int    ***bndmixzone,
                                                       int      *newmatlist,
                                                       T        *newdata,
                                                       int      *newmixmat,
                                                       int      *newmixzone,
                                                       int      *newmixnext,
                                                       int      &newmixlen)
{
    int newmixindex = oldmat->GetMixlen();

    Boundary *bi = &sdb->boundary[d1];
    for (int n=0; n<bi->neighbors.size(); n++)
    {
        Neighbor *n1 = &bi->neighbors[n];
        int d2 = n1->domain;
        int mi = n1->match;

        if (!bndmatlist[d2][mi])
            EXCEPTION1(VisItException,"Null array");

        int bndindex = 0;
        int bndmixindex = 0;
        int *n1extents = n1->zextents;
        for (int k=n1extents[4]; k<=n1extents[5]; k++)
        {
            for (int j=n1extents[2]; j<=n1extents[3]; j++)
            {
                for (int i=n1extents[0]; i<=n1extents[1]; i++)
                {
                    int newindex = bi->NewCellIndexFromNeighbor(n1, i,j,k);
                    if (newindex >= 0 && bndmatlist[d2][mi][bndindex]<0)
                    {
                        newmatlist[newindex] = -newmixindex - 1;

                        int oldzone = bndmixzone[d2][mi][bndmixindex];
                        while (bndmixindex < bndmixlen[d2][mi] &&
                               bndmixzone[d2][mi][bndmixindex] == oldzone)
                        {
                            if (newdata)
                                newdata[newmixindex]    = bnddata[d2][mi][bndmixindex];
                            if (newmixmat)
                                newmixmat[newmixindex]  = bndmixmat[d2][mi][bndmixindex];
                            if (newmixzone)
                                newmixzone[newmixindex] = newindex; // +1 ???
                            if (newmixnext)
                                newmixnext[newmixindex] = (newmixindex+1)+1;
                            bndmixindex++;
                            newmixindex++;
                        }
                        if (newmixnext)
                            newmixnext[newmixindex-1] = 0;
                    }
                    bndindex++;
                }
            }
        }
    }

    // Return the real new mixed data array lengths.
    newmixlen = newmixindex;
}

// ****************************************************************************
//  Method:  BoundaryHelperFunctions::FreeBoundaryData
//
//  Purpose:
//    Deallocate the temporary storage for the boundary data.
//
//  Arguments:
//    bnddata    the temporary boundary data
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Dec 13 12:06:29 PST 2001
//    Templatized this function.
//
// ****************************************************************************
template <class T>
void
BoundaryHelperFunctions<T>::FreeBoundaryData(T ***bnddata)
{
    for (int b=0; b<sdb->boundary.size(); b++)
    {
        Boundary *bi = &sdb->boundary[b];

        for (int n=0; n<bi->neighbors.size(); n++)
            delete[] bnddata[b][n];
        delete[] bnddata[b];
    }
    delete[] bnddata;
}


// ****************************************************************************
//  Method:  BoundaryHelperFunctions::FakeNonexistentBoundaryData
//
//  Purpose:
//    Fill in non-existent zones with some guess for the value to place there.
//
//  Arguments:
//    d1             the domain number
//    newdata        the mesh-sized array which holds the new data
//    isPointData    true if this is node-centered, false if cell-centered
//    ncomp          1 for scalar data, >1 for vector data
//
//  Programmer:  Jeremy Meredith
//  Creation:    December  4, 2001
//
// ****************************************************************************
template <class T>
void
BoundaryHelperFunctions<T>::FakeNonexistentBoundaryData(int  d1,
                                                           T   *newdata,
                                                           bool isPointData,
                                                           int  ncomp)
{
    Boundary *bi = &sdb->boundary[d1];
    bool *newexists = sdb->SetExistence(d1, isPointData);

    int  *biextents = (isPointData ? bi->newnextents : bi-> newzextents);
    for (int k = biextents[4]; k <= biextents[5]; k++)
    {
        for (int j = biextents[2]; j <= biextents[3]; j++)
        {
            for (int i = biextents[0]; i <= biextents[1]; i++)
            {
                int newindex;
                if (isPointData)
                    newindex = bi->NewPointIndex(i,j,k);
                else
                    newindex = bi->NewCellIndex(i,j,k);
                if (!newexists[newindex])
                {
                    int copyindex = (isPointData ?
                                     bi->ClosestExistingNewPointIndex(newexists, i,j,k) :
                                     bi->ClosestExistingNewCellIndex(newexists, i,j,k));
                    for (int c=0; c<ncomp; c++)
                        newdata[newindex*ncomp+c] = newdata[copyindex*ncomp+c];
                }
            }
        }
    }
    delete[] newexists;
}
    

// ----------------------------------------------------------------------------
//                               public methods
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Constructor:  avtStructuredDomainBoundaries::avtStructuredDomainBoundaries
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 25, 2001
//
//  Modifications:
//    Mark C. Miller, Mon Jan 12 17:29:19 PST 2004
//    Added optional bool, canComputeNeighborsFromExtents
//
// ****************************************************************************
avtStructuredDomainBoundaries::avtStructuredDomainBoundaries(
   bool canComputeNeighborsFromExtents)
{
    bhf_int   = new BoundaryHelperFunctions<int>(this);
    bhf_float = new BoundaryHelperFunctions<float>(this);
    bhf_uchar = new BoundaryHelperFunctions<unsigned char>(this);
    shouldComputeNeighborsFromExtents = canComputeNeighborsFromExtents; 
}

// ****************************************************************************
//  Destructor:  avtStructuredDomainBoundaries::~avtStructuredDomainBoundaries
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 25, 2001
//
//  Modifications:
//    Mark C. Miller, ed Mar 23 15:29:56 PST 2005
//    Added code to delete stuff new'd in constructor 
//
// ****************************************************************************
avtStructuredDomainBoundaries::~avtStructuredDomainBoundaries()
{
    delete bhf_int;
    delete bhf_float;
    delete bhf_uchar;
}

// ****************************************************************************
//  Destructor:  avtStructuredDomainBoundaries::Destruct
//
//  Programmer:  Hank Childs
//  Creation:    September 25, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Oct  8 14:45:46 PDT 2002
//    Cleaned up stupid mistake where void pointer was being deleted.
//
// ****************************************************************************
void
avtStructuredDomainBoundaries::Destruct(void *p)
{
    avtStructuredDomainBoundaries *sdb = (avtStructuredDomainBoundaries *) p;
    delete sdb;
}

// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::SetNumDomains
//
//  Purpose:
//    Set the number of domains in this database.
//
//  Arguments:
//    nd         the number of domains
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 25, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Nov 11 10:08:51 PST 2003
//    Added call to DeclareNumDomains for the derived types.
//
//    Mark C. Miller, Mon Jan 12 17:29:19 PST 2004
//    Added code to populate extents/levels if
//    shouldComputeNeighborsFromExtents is true
//
// ****************************************************************************
void
avtStructuredDomainBoundaries::SetNumDomains(int nd)
{
    wholeBoundary.resize(nd);

    if (shouldComputeNeighborsFromExtents)
    {
        extents.resize(nd*6);
        levels.resize(nd);
    }
}

// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::SetExtents
//
//  Purpose:
//    Set the extents of one domain.
//
//  Arguments:
//    domain     the domain to set the extents of
//    e          the extents
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
void
avtStructuredDomainBoundaries::SetExtents(int domain, int e[6])
{
    if (domain >= wholeBoundary.size())
        EXCEPTION1(VisItException,
                   "avtStructuredDomainBoundaries: "
                   "targetted domain more than number of domains");

    wholeBoundary[domain].domain = domain;
    wholeBoundary[domain].SetExtents(e);
}

// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::AddNeighbor
//
//  Purpose:
//    Add a neighbor to a given domain.
//
//  Arguments:
//    domain     the current domain to add a neighbor for
//    d          the domain number of the new neigbor
//    mi         the current domain's index in the neighbor's neighbor list
//    o          the three orientation values
//    e          the extents of the matching boundary in the current domain
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
void
avtStructuredDomainBoundaries::AddNeighbor(int domain,
                                           int d, int mi,
                                           int o[3], int e[6])
{
    if (domain >= wholeBoundary.size())
        EXCEPTION1(VisItException,
                   "avtStructuredDomainBoundaries: "
                   "targetted domain more than number of domains");

    wholeBoundary[domain].AddNeighbor(d, mi, o, e);
}

// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::Finish
//
//  Purpose:
//    Finalize a domain
//
//  Arguments:
//    domain     the domain to finalize
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
void
avtStructuredDomainBoundaries::Finish(int domain)
{
    if (domain >= wholeBoundary.size())
        EXCEPTION1(VisItException,
                   "avtStructuredDomainBoundaries: "
                   "targetted domain more than number of domains");

    wholeBoundary[domain].Finish();
}

// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::ExchangeScalar
//
//  Purpose:
//    Exchange the ghost zone information for some scalars,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    isPointData  true if this is node-centered, false if cell-centered
//    vectors      an array of vectors
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  7, 2003
//
//  Modifications:
//    Jeremy Meredith, Wed Nov 19 12:20:52 PST 2003
//    Added code to get all processors to agree on one data type.
//    This was causing problems with more procs than domains.
//
//    Hank Childs, Tue Nov 25 17:32:53 PST 2003
//    Fix typo that comes up in parallel only.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************
vector<vtkDataArray*>
avtStructuredDomainBoundaries::ExchangeScalar(vector<int>           domainNum,
                                              bool                  isPointData,
                                              vector<vtkDataArray*> scalars)
{
    int dataType = (scalars.empty() ? -1 : scalars[0]->GetDataType());

#ifdef PARALLEL
    // Let's get them all to agree on one data type.
    int myDataType = dataType;
    MPI_Allreduce(&myDataType, &dataType, 1, MPI_INT, MPI_MAX, VISIT_MPI_COMM);
#endif

    if (dataType < 0)
        return scalars;

    switch (dataType)
    {
      case VTK_FLOAT:
        return ExchangeFloatScalar(domainNum, isPointData, scalars);
        break;
      case VTK_INT:
      case VTK_UNSIGNED_INT:
        return ExchangeIntScalar(domainNum, isPointData, scalars);
        break;
      case VTK_CHAR:
      case VTK_UNSIGNED_CHAR:
        return ExchangeUCharScalar(domainNum, isPointData, scalars);
        break;
      default:
        EXCEPTION1(VisItException, "Unknown scalar type in "
                   "avtStructuredDomainBoundaries::ExchangeScalar");
    }
}

// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::ExchangeFloatScalar
//
//  Purpose:
//    Exchange the ghost zone information for some scalars,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    isPointData  true if this is node-centered, false if cell-centered
//    scalars      an array of scalars
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Dec 13 12:06:54 PST 2001
//    Made use of templatized functions.  Added call to fake boundary
//    data when it is nonexistent.
//
//    Hank Childs, Wed Jan  2 09:28:05 PST 2002
//    Propagate variable names.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray 
//    and vtkFloatArray instead.
//
//    Jeremy Meredith, Fri Nov  7 15:13:56 PST 2003
//    Renamed to include the "Float" in the name.
//
//    Hank Childs, Wed Jun 29 15:24:35 PDT 2005
//    Cache domain2proc.
//
// ****************************************************************************
vector<vtkDataArray*>
avtStructuredDomainBoundaries::ExchangeFloatScalar(vector<int>     domainNum,
                                             bool                  isPointData,
                                             vector<vtkDataArray*> scalars)
{
    if (domain2proc.size() == 0)
    {
        domain2proc = CreateDomainToProcessorMap(domainNum);
        CreateCurrentDomainBoundaryInformation(domain2proc);
    }

    vector<vtkDataArray*> out(scalars.size(), NULL);

    //
    // Create the matching arrays for the given scalars
    //
    float ***vals = bhf_float->InitializeBoundaryData();
    int d;
    for (d = 0; d < scalars.size(); d++)
    {
        float *oldvals = (float*)scalars[d]->GetVoidPointer(0);
        bhf_float->FillBoundaryData(domainNum[d], oldvals, vals, isPointData);
    }

    bhf_float->CommunicateBoundaryData(domain2proc, vals, isPointData);

    for (d = 0; d < scalars.size(); d++)
    {
        Boundary *bi = &boundary[domainNum[d]];

        // Create the new VTK objects
        out[d] = vtkFloatArray::New(); 
        out[d]->SetName(scalars[d]->GetName());
        if (isPointData)
            out[d]->SetNumberOfTuples(bi->newnpts);
        else
            out[d]->SetNumberOfTuples(bi->newncells);

        float *oldvals = (float*)scalars[d]->GetVoidPointer(0);
        float *newvals = (float*)out[d]->GetVoidPointer(0);

        // Set the known ones
        bhf_float->CopyOldValues(domainNum[d], oldvals, newvals, isPointData);

        // Match the unknown ones
        bhf_float->SetNewBoundaryData(domainNum[d], vals, newvals, isPointData);

        // Set the remaining unset ones (reduced connectivity, etc.)
        bhf_float->FakeNonexistentBoundaryData(domainNum[d], newvals, isPointData);
    }

    bhf_float->FreeBoundaryData(vals);

    return out;
}


// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::ExchangeIntScalar
//
//  Purpose:
//    Exchange the ghost zone information for some scalars,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    isPointData  true if this is node-centered, false if cell-centered
//    scalars      an array of scalars
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  7, 2003
//
//  Note: direct copy of ExchangeFloatScalar with modifications for ints.
//
//  Modifications:
//
//    Hank Childs, Wed Jun 29 15:24:35 PDT 2005
//    Cache domain2proc.
//
// ****************************************************************************
vector<vtkDataArray*>
avtStructuredDomainBoundaries::ExchangeIntScalar(vector<int>       domainNum,
                                             bool                  isPointData,
                                             vector<vtkDataArray*> scalars)
{
    if (domain2proc.size() == 0)
    {
        domain2proc = CreateDomainToProcessorMap(domainNum);
        CreateCurrentDomainBoundaryInformation(domain2proc);
    }

    vector<vtkDataArray*> out(scalars.size(), NULL);

    //
    // Create the matching arrays for the given scalars
    //
    int ***vals = bhf_int->InitializeBoundaryData();
    int d;
    for (d = 0; d < scalars.size(); d++)
    {
        int *oldvals = (int*)scalars[d]->GetVoidPointer(0);
        bhf_int->FillBoundaryData(domainNum[d], oldvals, vals, isPointData);
    }

    bhf_int->CommunicateBoundaryData(domain2proc, vals, isPointData);

    for (d = 0; d < scalars.size(); d++)
    {
        Boundary *bi = &boundary[domainNum[d]];

        // Create the new VTK objects
        out[d] = vtkIntArray::New(); 
        out[d]->SetName(scalars[d]->GetName());
        if (isPointData)
            out[d]->SetNumberOfTuples(bi->newnpts);
        else
            out[d]->SetNumberOfTuples(bi->newncells);

        int *oldvals = (int*)scalars[d]->GetVoidPointer(0);
        int *newvals = (int*)out[d]->GetVoidPointer(0);

        // Set the known ones
        bhf_int->CopyOldValues(domainNum[d], oldvals, newvals, isPointData);

        // Match the unknown ones
        bhf_int->SetNewBoundaryData(domainNum[d], vals, newvals, isPointData);

        // Set the remaining unset ones (reduced connectivity, etc.)
        bhf_int->FakeNonexistentBoundaryData(domainNum[d], newvals, isPointData);
    }

    bhf_int->FreeBoundaryData(vals);

    return out;
}


// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::ExchangeUCharScalar
//
//  Purpose:
//    Exchange the ghost zone information for some scalars,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    isPointData  true if this is node-centered, false if cell-centered
//    scalars      an array of scalars
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  7, 2003
//
//  Note: direct copy of ExchangeFloatScalar with modifications for uchars.
//
//  Modifications:
//
//    Hank Childs, Wed Jun 29 15:24:35 PDT 2005
//    Cache domain2proc.
//
// ****************************************************************************
vector<vtkDataArray*>
avtStructuredDomainBoundaries::ExchangeUCharScalar(vector<int>     domainNum,
                                             bool                  isPointData,
                                             vector<vtkDataArray*> scalars)
{
    if (domain2proc.size() == 0)
    {
        domain2proc = CreateDomainToProcessorMap(domainNum);
        CreateCurrentDomainBoundaryInformation(domain2proc);
    }

    vector<vtkDataArray*> out(scalars.size(), NULL);

    //
    // Create the matching arrays for the given scalars
    //
    unsigned char ***vals = bhf_uchar->InitializeBoundaryData();
    int d;
    for (d = 0; d < scalars.size(); d++)
    {
        unsigned char *oldvals = (unsigned char*)scalars[d]->GetVoidPointer(0);
        bhf_uchar->FillBoundaryData(domainNum[d], oldvals, vals, isPointData);
    }

    bhf_uchar->CommunicateBoundaryData(domain2proc, vals, isPointData);

    for (d = 0; d < scalars.size(); d++)
    {
        Boundary *bi = &boundary[domainNum[d]];

        // Create the new VTK objects
        out[d] = vtkUnsignedCharArray::New(); 
        out[d]->SetName(scalars[d]->GetName());
        if (isPointData)
            out[d]->SetNumberOfTuples(bi->newnpts);
        else
            out[d]->SetNumberOfTuples(bi->newncells);

        unsigned char *oldvals = (unsigned char*)scalars[d]->GetVoidPointer(0);
        unsigned char *newvals = (unsigned char*)out[d]->GetVoidPointer(0);

        // Set the known ones
        bhf_uchar->CopyOldValues(domainNum[d], oldvals, newvals, isPointData);

        // Match the unknown ones
        bhf_uchar->SetNewBoundaryData(domainNum[d], vals, newvals, isPointData);

        // Set the remaining unset ones (reduced connectivity, etc.)
        bhf_uchar->FakeNonexistentBoundaryData(domainNum[d], newvals, isPointData);
    }

    bhf_uchar->FreeBoundaryData(vals);

    return out;
}


// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::ExchangeFloatVector
//
//  Purpose:
//    Exchange the ghost zone information for some vectors,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    isPointData  true if this is node-centered, false if cell-centered
//    vectors      an array of vectors
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Dec 13 12:06:54 PST 2001
//    Made use of templatized functions.  Added call to fake boundary
//    data when it is nonexistent.
//
//    Hank Childs, Wed Jan  2 09:28:05 PST 2002
//    Propagate variable names.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkVectors has been deprecated in VTK 4.0, use vtkDataArray 
//    and vtkFloatArray instead.
//
//    Kathleen Bonnell, Mon May 20 13:33:03 PDT 2002 
//    Change name to reflect underlying data type.  Allow for arbitrary
//    number of components in the array. 
//    
//    Hank Childs, Fri Dec  6 14:56:20 PST 2002
//    Do not assume that the number of vectors is > 0.
//
//    Hank Childs, Wed Jun 29 15:24:35 PDT 2005
//    Cache domain2proc.
//
// ****************************************************************************
vector<vtkDataArray*>
avtStructuredDomainBoundaries::ExchangeFloatVector(vector<int>      domainNum,
                                              bool                  isPointData,
                                              vector<vtkDataArray*> vectors)
{
    if (domain2proc.size() == 0)
    {
        domain2proc = CreateDomainToProcessorMap(domainNum);
        CreateCurrentDomainBoundaryInformation(domain2proc);
    }

    vector<vtkDataArray*> out(vectors.size(), NULL);

    //
    // Create the matching arrays for the given vectors
    //
    float ***vals = bhf_float->InitializeBoundaryData();

    int d;
    int nComp = (vectors.size() > 0 ? vectors[0]->GetNumberOfComponents() :-1);
    for (d = 0; d < vectors.size(); d++)
    {
        float *oldvals = (float*)vectors[d]->GetVoidPointer(0);
        bhf_float->FillBoundaryData(domainNum[d], oldvals, vals, isPointData, nComp);
    }

    bhf_float->CommunicateBoundaryData(domain2proc, vals, isPointData, nComp);

    for (d = 0; d < vectors.size(); d++)
    {
        // Create the new VTK objects
        out[d] = vtkFloatArray::New(); 
        out[d]->SetNumberOfComponents(nComp); 
        out[d]->SetName(vectors[d]->GetName());
        if (isPointData)
            out[d]->SetNumberOfTuples(boundary[domainNum[d]].newnpts);
        else
            out[d]->SetNumberOfTuples(boundary[domainNum[d]].newncells);

        float *oldvals = (float*)vectors[d]->GetVoidPointer(0);
        float *newvals = (float*)out[d]->GetVoidPointer(0);

        // Set the known ones
        bhf_float->CopyOldValues(domainNum[d], oldvals, newvals, isPointData, nComp);

        // Match the unknown ones
        bhf_float->SetNewBoundaryData(domainNum[d], vals, newvals, isPointData, nComp);

        // Set the remaining unset ones (reduced connectivity, etc.)
        bhf_float->FakeNonexistentBoundaryData(domainNum[d], newvals, isPointData, nComp);
    }

    bhf_float->FreeBoundaryData(vals);

    return out;
}


// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::ExchangeIntVector
//
//  Purpose:
//    Exchange the ghost zone information for some vectors,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    isPointData  true if this is node-centered, false if cell-centered
//    vectors      an array of vectors
//
//  Notes:
//    Taken from ExchangeFloatVector and modified for integer data types.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    May 20, 2002 
//
//  Modifications:
//
//    Hank Childs, Fri Dec  6 14:56:20 PST 2002
//    Do not assume that the number of vectors is > 0.
//
//    Kathleen Bonnell, Wed Dec 11 09:13:25 PST 2002 
//    Preserver underlying data type:  use MakeObject instead of New. 
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Use NewInstance instead of MakeObject, new vtk api. 
//
//    Hank Childs, Wed Jun 29 15:24:35 PDT 2005
//    Cache domain2proc.
//
// ****************************************************************************
vector<vtkDataArray*>
avtStructuredDomainBoundaries::ExchangeIntVector(vector<int>        domainNum,
                                                 bool               isPointData,
                                                 vector<vtkDataArray*> vectors)
{
    if (domain2proc.size() == 0)
    {
        domain2proc = CreateDomainToProcessorMap(domainNum);
        CreateCurrentDomainBoundaryInformation(domain2proc);
    }

    vector<vtkDataArray*> out(vectors.size(), NULL);

    //
    // Create the matching arrays for the given vectors
    //
    int ***vals = bhf_int->InitializeBoundaryData();

    int d;
    int nComp = (vectors.size() > 0 ? vectors[0]->GetNumberOfComponents(): -1);
    for (d = 0; d < vectors.size(); d++)
    {
        int *oldvals = (int*)vectors[d]->GetVoidPointer(0);
        bhf_int->FillBoundaryData(domainNum[d], oldvals, vals, isPointData, nComp);
    }

    bhf_int->CommunicateBoundaryData(domain2proc, vals, isPointData, nComp);

    for (d = 0; d < vectors.size(); d++)
    {
        // Create the new VTK objects
        out[d] = vectors[d]->NewInstance(); 
        out[d]->SetNumberOfComponents(nComp); 
        out[d]->SetName(vectors[d]->GetName());
        if (isPointData)
            out[d]->SetNumberOfTuples(boundary[domainNum[d]].newnpts);
        else
            out[d]->SetNumberOfTuples(boundary[domainNum[d]].newncells);

        int *oldvals = (int*)vectors[d]->GetVoidPointer(0);
        int *newvals = (int*)out[d]->GetVoidPointer(0);

        // Set the known ones
        bhf_int->CopyOldValues(domainNum[d], oldvals, newvals, isPointData, nComp);

        // Match the unknown ones
        bhf_int->SetNewBoundaryData(domainNum[d], vals, newvals, isPointData, nComp);

        // Set the remaining unset ones (reduced connectivity, etc.)
        bhf_int->FakeNonexistentBoundaryData(domainNum[d], newvals, isPointData, nComp);
    }

    bhf_int->FreeBoundaryData(vals);

    return out;
}

// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::ExchangeMaterial
//
//  Purpose:
//    Exchange the ghost zone information for some materials,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    material     an array of materials
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2001
//
//  Modifications:
//    Jeremy Meredith, Mon Oct 28 19:25:34 PST 2002
//    Added newmixlen as an argument to SetNewMixedBoundaryData since
//    it may get shortened when some neighbors are missing.
//
//    Hank Childs, Wed Jun 29 15:24:35 PDT 2005
//    Cache domain2proc.
//
//    Hank Childs, Wed Jul  6 06:50:55 PDT 2005
//    Instead of calculating index into domain's neighbor list in a non-robust
//    way, use the "match", which is already pre-computed by the client for
//    this purpose.
//
//    Hank Childs, Fri Jun  9 14:18:11 PDT 2006
//    Remove unused variable.
//
// ****************************************************************************
vector<avtMaterial*>
avtStructuredDomainBoundaries::ExchangeMaterial(vector<int>          domainNum,
                                                vector<avtMaterial*> mats)
{
    if (domain2proc.size() == 0)
    {
        domain2proc = CreateDomainToProcessorMap(domainNum);
        CreateCurrentDomainBoundaryInformation(domain2proc);
    }

    vector<avtMaterial*> out(mats.size(), NULL);

    //
    // Create the matching arrays for the given materials
    //
    int   ***matlist = bhf_int->InitializeBoundaryData();
    int   ***mixmat  = bhf_int->InitializeBoundaryData();
    int   ***mixzone = bhf_int->InitializeBoundaryData();
    float ***mixvf   = bhf_float->InitializeBoundaryData();
    vector<vector<int> > mixlen(boundary.size());
    for (int b = 0; b < boundary.size(); b++)
        mixlen[b] = vector<int>(boundary[b].neighbors.size(), 0);

    int d;
    for (d = 0; d < mats.size(); d++)
    {
        const int *oldmatlist = mats[d]->GetMatlist();
        bhf_int->FillBoundaryData(domainNum[d], oldmatlist, matlist, false);
    }

    for (d = 0; d < mats.size(); d++)
    {
        bhf_float->FillMixedBoundaryData(domainNum[d], mats[d], mats[d]->GetMixVF(),
                              mixvf, mixmat, mixzone, mixlen[domainNum[d]]);
    }

    bhf_int->CommunicateBoundaryData(domain2proc, matlist, false);
    bhf_float->CommunicateMixedBoundaryData(domain2proc, mixvf, mixmat, mixzone, mixlen);

    for (d = 0; d < mats.size(); d++)
    {
        avtMaterial *oldmat = mats[d];
        Boundary    &bi     = boundary[domainNum[d]];

        // Create the new material objects
        const int   *oldmatlist = oldmat->GetMatlist();
        int         *newmatlist = new int[bi.newncells];

        int oldmixlen = oldmat->GetMixlen();
        int newmixlen = oldmixlen;
        for (int n=0; n<bi.neighbors.size(); n++)
        {
            int mi = bi.neighbors[n].match;
            int d2 = bi.neighbors[n].domain;
            newmixlen += mixlen[d2][mi];
        }

        const float *oldmixvf   = oldmat->GetMixVF();
        float       *newmixvf   = new float[newmixlen];
        const int   *oldmixmat  = oldmat->GetMixMat();
        int         *newmixmat  = new int[newmixlen];
        const int   *oldmixzone = oldmat->GetMixZone();
        int         *newmixzone = new int[newmixlen];
        const int   *oldmixnext = oldmat->GetMixNext();
        int         *newmixnext = new int[newmixlen];

        // Set the known ones
        bhf_int->CopyOldValues(domainNum[d], oldmatlist, newmatlist, false);
        if (newmixlen > 0)
        {
            bhf_float->CopyOldMixedValues(mats[d], oldmixvf,   newmixvf);
            bhf_int->CopyOldMixedValues(mats[d], oldmixmat,  newmixmat);
            // BUG - can't just copy mixed zone nos: old zoneno != new zoneno
            bhf_int->CopyOldMixedValues(mats[d], oldmixzone, newmixzone);
            bhf_int->CopyOldMixedValues(mats[d], oldmixnext, newmixnext);
        }

        // Match the unknown ones
        bhf_int->SetNewBoundaryData(domainNum[d], matlist, newmatlist, false);
        if (newmixlen > 0)
        {
            bhf_float->SetNewMixedBoundaryData(domainNum[d], oldmat, mixlen,
                                   matlist, mixvf, mixmat, mixzone, newmatlist,
                                   newmixvf, newmixmat, newmixzone, newmixnext,
                                   newmixlen);
        }

        // Set the remaining unset ones (reduced connectivity, etc.)
        bhf_int->FakeNonexistentBoundaryData(domainNum[d], newmatlist, false);

        out[d] = new avtMaterial(oldmat->GetNMaterials(), 
                                 oldmat->GetMaterials(),
                                 boundary[domainNum[d]].newncells,
                                 newmatlist,
                                 newmixlen,
                                 newmixmat,
                                 newmixnext,
                                 newmixzone,
                                 newmixvf);

        delete[] newmatlist;
        delete[] newmixvf;
        delete[] newmixmat;
        delete[] newmixzone;
        delete[] newmixnext;
    }

    bhf_int->FreeBoundaryData(matlist);
    bhf_float->FreeBoundaryData(mixvf);
    bhf_int->FreeBoundaryData(mixmat);
    bhf_int->FreeBoundaryData(mixzone);

    return out;
}

// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::ExchangeMixVar
//
//  Purpose:
//    Exchange the ghost zone information for some mixvars,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    mixvar       an array of mixvars
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 13, 2001
//
//  Modifications:
//    Hank Childs, Thu Jul  4 16:51:37 PDT 2002
//    Add new variable name argument for mixvar constructor.
//
//    Jeremy Meredith, Mon Oct 28 19:25:34 PST 2002
//    Added newmixlen as an argument to SetNewMixedBoundaryData since
//    it may get shortened when some neighbors are missing.
//
//    Jeremy Meredith, Mon Dec  9 12:11:44 PST 2002
//    Changed the way the mixvar name was communicated in parallel.
//    IBMs don't seem to like MPI_MAX with a char (even unsigned) data type.
//
//    Hank Childs, Wed Jun 29 15:24:35 PDT 2005
//    Cache domain2proc.
//
//    Hank Childs, Wed Jul  6 06:50:55 PDT 2005
//    Instead of calculating index into domain's neighbor list in a non-robust
//    way, use the "match", which is already pre-computed by the client for
//    this purpose.
//
//    Hank Childs, Fri Jun  9 14:18:11 PDT 2006
//    Remove unused variable.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************
vector<avtMixedVariable*>
avtStructuredDomainBoundaries::ExchangeMixVar(vector<int>            domainNum,
                                          const vector<avtMaterial*> mats,
                                          vector<avtMixedVariable*>  mixvars)
{
    if (domain2proc.size() == 0)
    {
        domain2proc = CreateDomainToProcessorMap(domainNum);
        CreateCurrentDomainBoundaryInformation(domain2proc);
    }

    vector<avtMixedVariable*> out(mats.size(), NULL);

    //
    // Pretty ugly -- we need to come up with the mixed variable's name.  It
    // could be that not a single domain on the processor has a valid mixed
    // variable.  In that case we need to do global communication to find it.
    // (It really does happen that a domain with no mixed zones gets ghost
    // zones that are mixed.)
    //
    const char *mixvarname = NULL;
    for (int i = 0 ; i < mixvars.size() ; i++)
        if (mixvars[i] != NULL)
            mixvarname = mixvars[i]->GetVarname().c_str();

#ifdef PARALLEL
    int rank;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);

    int length = 0;
    if (mixvarname != NULL)
    {
        length = strlen(mixvarname)+1;
    }
    struct {int length; int rank;} len_rank_out, len_rank_in={length, rank};

    MPI_Allreduce(&len_rank_in, &len_rank_out, 1, MPI_2INT, MPI_MAXLOC,
                  VISIT_MPI_COMM);
    length = len_rank_out.length;

    char *mvname = new char[length];
    if (mixvarname != NULL)
    {
        strcpy(mvname, mixvarname);
    }

    MPI_Bcast(mvname, length, MPI_CHAR, len_rank_out.rank, VISIT_MPI_COMM);
    mixvarname = mvname;
#else
    char *mvname = NULL;
#endif

    //
    // Create the matching arrays for the given materials
    //
    int   ***matlist = bhf_int->InitializeBoundaryData();
    int   ***mixzone = bhf_int->InitializeBoundaryData();
    float ***mixvals = bhf_float->InitializeBoundaryData();
    vector<vector<int> > mixlen(boundary.size());
    for (int b = 0; b < boundary.size(); b++)
        mixlen[b] = vector<int>(boundary[b].neighbors.size(), 0);

    int d;
    for (d = 0; d < mats.size(); d++)
    {
        const int *oldmatlist = mats[d]->GetMatlist();
        bhf_int->FillBoundaryData(domainNum[d], oldmatlist, matlist, false);
    }

    for (d = 0; d < mats.size(); d++)
    {
        const float *oldmixvals = (mixvars[d] ? mixvars[d]->GetBuffer() : NULL);
        bhf_float->FillMixedBoundaryData(domainNum[d], mats[d], oldmixvals,
                              mixvals, NULL, mixzone, mixlen[domainNum[d]]);
    }

    bhf_int->CommunicateBoundaryData(domain2proc, matlist, false);
    bhf_float->CommunicateMixedBoundaryData(domain2proc, mixvals, NULL, mixzone, mixlen);

    for (d = 0; d < mats.size(); d++)
    {
        avtMaterial *oldmat    = mats[d];
        avtMixedVariable *oldmixvar = mixvars[d];
        Boundary    &bi     = boundary[domainNum[d]];

        // Create the new material objects
        const int   *oldmatlist = oldmat->GetMatlist();
        int         *newmatlist = new int[bi.newncells];

        int oldmixlen = oldmat->GetMixlen();
        int newmixlen = oldmixlen;
        for (int n=0; n<bi.neighbors.size(); n++)
        {
            int mi = bi.neighbors[n].match;
            int d2 = bi.neighbors[n].domain;
            newmixlen += mixlen[d2][mi];
        }

        const float *oldmixvals = oldmixvar ? oldmixvar->GetBuffer() : NULL;
        float       *newmixvals = new float[newmixlen];

        // Set the known ones
        bhf_int->CopyOldValues(domainNum[d], oldmatlist, newmatlist, false);
        if (newmixlen != 0)
            bhf_float->CopyOldMixedValues(mats[d], oldmixvals, newmixvals);

        // Match the unknown ones
        bhf_int->SetNewBoundaryData(domainNum[d], matlist, newmatlist, false);
        if (newmixlen != 0)
            bhf_float->SetNewMixedBoundaryData(domainNum[d], oldmat, mixlen,
                                    matlist, mixvals, NULL, mixzone,
                                    newmatlist, newmixvals, NULL, NULL, NULL,
                                    newmixlen);

        if (newmixlen == 0)
            out[d] = NULL;
        else
            out[d] = new avtMixedVariable(newmixvals, newmixlen, mixvarname);

        delete[] newmatlist;
        delete[] newmixvals;
    }

    bhf_int->FreeBoundaryData(matlist);
    bhf_float->FreeBoundaryData(mixvals);
    bhf_int->FreeBoundaryData(mixzone);

    if (mvname != NULL)
    {
        delete [] mvname;
    }
    return out;
}


// ****************************************************************************
//  Method: avtStructuredDomainBoundaries::RequiresCommunication
//
//  Purpose:
//      Determines if this domain boundaries object will need to perform
//      collective communication to create the type of ghost data requested.
//
//  Programmer: Hank Childs
//  Creation:   February 27, 2005
//
// ****************************************************************************

bool
avtStructuredDomainBoundaries::RequiresCommunication(avtGhostDataType gtype)
{
    if (gtype == NO_GHOST_DATA)
        return false;
    else if (gtype == GHOST_NODE_DATA)
        return false;

    // (gtype == GHOST_ZONE_DATA)
    return true;
}


// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::ConfirmMesh
//
//  Purpose:
//      If there is more than one mesh, the boundary information is likely for
//      only one of them.  Confirm that the mesh has the proper dimensions.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    mesh         an array of meshes
//
//  Programmer:  Hank Childs
//  Creation:    March 27, 2002
//
//  Modifications:
//    Mark C. Miller, Thu Mar  9 11:15:29 PST 2006
//    Protected deref of meshes[i] with test for non-0
//
// ****************************************************************************
bool
avtStructuredDomainBoundaries::ConfirmMesh(vector<int>         domainNum,
                                            vector<vtkDataSet*> meshes)
{
    for (int i = 0 ; i < domainNum.size() ; i++)
    {
        if (domainNum[i] < 0 || domainNum[i] >= wholeBoundary.size())
        {
            //
            // This mesh is referencing a domain that is not valid for the
            // boundary information we have ==> this cannot be a match.
            //
            return false;
        }
        Boundary &b = wholeBoundary[domainNum[i]];

        if (meshes[i] == 0)
            return false;

        //
        // Comparing points and cells is probably good enough.
        //
        if (meshes[i]->GetNumberOfPoints() != b.oldnpts)
        {
            return false;
        }
        if (meshes[i]->GetNumberOfCells() != b.oldncells)
        {
            return false;
        }
    }

    //
    // It passed all of our tests, so this must work.  However: if there were
    // no meshes on this processor, this may produce a false positive.
    //
    return true;
}


// ****************************************************************************
//  Method: avtStructuredDomainBoundaries::ResetCachedMembers
//
//  Purpose:
//      Resets cached data members, in this case domain2proc.
//
//  Programmer: Hank Childs
//  Creation:   June 29, 2005
//
// ****************************************************************************

void
avtStructuredDomainBoundaries::ResetCachedMembers(void)
{
    domain2proc.clear();
}


// ****************************************************************************
//  Method: avtStructuredDomainBoundaries::CreateGhostZones
//
//  Purpose:
//      Creates ghost zones for the output mesh.  This uses the boundary info,
//      as well as previous ghost zone information (if it exists -- this ghost
//      info comes about from nesting AMR patches), to creat the ghost zones
//      for the output.
//
//  Arguments:
//      outMesh    The output mesh we just made ghost zones for.
//      inMesh     The input mesh before it had ghost zones.
//      bi         The boundary information.
//
//  Programmer: Hank Childs
//  Creation:   November 12, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Aug 27 16:16:52 PDT 2004
//    Rename ghost data arrays.
//
// ****************************************************************************

void
avtStructuredDomainBoundaries::CreateGhostZones(vtkDataSet *outMesh,
                                              vtkDataSet *inMesh, Boundary *bi)
{
    vtkUnsignedCharArray *oldGhosts = (vtkUnsignedCharArray *)
                        inMesh->GetCellData()->GetArray("avtGhostZones");

    // Create the ghost zone array
    vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
    ghostCells->SetName("avtGhostZones");
    ghostCells->Allocate(bi->newncells);
    for (int k = bi->newzextents[4]; k <= bi->newzextents[5]; k++)
        for (int j = bi->newzextents[2]; j <= bi->newzextents[3]; j++)
            for (int i = bi->newzextents[0]; i <= bi->newzextents[1]; i++)
            {
                unsigned char gv = 0;
                if (oldGhosts != NULL)
                {
                    int index = bi->OldCellIndex(i, j, k);
                    if (index >= 0)
                        gv = oldGhosts->GetValue(index);
                }
                if (bi->IsGhostZone(i,j,k))
                    avtGhostData::AddGhostZoneType(gv,
                                          DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
                ghostCells->InsertNextValue(gv);
            }

    outMesh->GetCellData()->AddArray(ghostCells);
    ghostCells->Delete();
    outMesh->SetUpdateGhostLevel(0);

    //
    //  Create a field-data array indicating the extents of real zones.
    //  Used during ghostzone removal.
    //
    vtkIntArray *realDims = vtkIntArray::New();
    realDims->SetName("avtRealDims");
    realDims->SetNumberOfTuples(6);
    realDims->SetValue(0, bi->oldnextents[0] - bi->newnextents[0]);
    realDims->SetValue(1, bi->oldnextents[1] - bi->newnextents[0]);
    realDims->SetValue(2, bi->oldnextents[2] - bi->newnextents[2]);
    realDims->SetValue(3, bi->oldnextents[3] - bi->newnextents[2]);
    realDims->SetValue(4, bi->oldnextents[4] - bi->newnextents[4]);
    realDims->SetValue(5, bi->oldnextents[5] - bi->newnextents[4]);
    outMesh->GetFieldData()->AddArray(realDims);
    outMesh->GetFieldData()->CopyFieldOn("avtRealDims");
    realDims->Delete();
}

// ****************************************************************************
//  Method:  avtCurvilinearDomainBoundaries::ExchangeMesh
//
//  Purpose:
//    Exchange the ghost zone information for some meshes,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    mesh         an array of meshes
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Dec 13 12:06:54 PST 2001
//    Made use of templatized functions.  Added call to fake boundary
//    data when it is nonexistent.
//
//    Kathleen Bonnell, Wed Jul 10 16:02:56 PDT 2002 
//    Create a field-data array indicating the extents of real zones.
//    Used during ghostzone removal.
//
//    Hank Childs, Mon Nov 10 14:58:43 PST 2003
//    Made this routine be associated strictly with curvlinear meshes.
//
//    Hank Childs, Wed Nov 12 10:53:38 PST 2003
//    Allowed for pre-existing ghost zones as well.
//
//    Hank Childs, Thu Nov 13 08:56:18 PST 2003
//    Fixed stupid bug where arguments to CreateGhostZones were out of order.
//
//    Mark C. Miller, Mon Jan 12 19:21:22 PST 2004
//    Added check and exception for wrong VTK grid type
//
//    Hank Childs, Wed Jun 29 15:24:35 PDT 2005
//    Cache domain2proc.
//
// ****************************************************************************
vector<vtkDataSet*>
avtCurvilinearDomainBoundaries::ExchangeMesh(vector<int>         domainNum,
                                             vector<vtkDataSet*> meshes)
{
    if (domain2proc.size() == 0)
    {
        domain2proc = CreateDomainToProcessorMap(domainNum);
        CreateCurrentDomainBoundaryInformation(domain2proc);
    }

    vector<vtkDataSet*> out(meshes.size(), NULL);

    //
    // Create the matching arrays for the given meshes
    //
    float ***coord = bhf_float->InitializeBoundaryData();
    int d;
    for (d = 0; d < meshes.size(); d++)
    {
        vtkStructuredGrid *mesh = (vtkStructuredGrid*)(meshes[d]);
        float *oldcoord = (float*)mesh->GetPoints()->GetVoidPointer(0);
        bhf_float->FillBoundaryData(domainNum[d], oldcoord, coord, true, 3);
    }

    bhf_float->CommunicateBoundaryData(domain2proc, coord, true, 3);

    for (d = 0; d < meshes.size(); d++)
    {
        if (meshes[d]->GetDataObjectType() != VTK_STRUCTURED_GRID)
        {
            EXCEPTION1(VisItException,
                       "avtStructuredDomainBoundaries: "
                       "VTK data object type not VTK_STRUCTURED_GRID");
        }

        int d1 = domainNum[d];
        vtkStructuredGrid *mesh = (vtkStructuredGrid*)(meshes[d]);
        Boundary *bi = &boundary[d1];

        // Create the VTK objects
        vtkStructuredGrid    *outm  = vtkStructuredGrid::New(); 
        vtkPoints            *outp  = vtkPoints::New();
        outm->SetPoints(outp);
        outp->Delete();
        outm->SetDimensions(bi->newndims);
        outp->SetNumberOfPoints(bi->newnpts);

        float *oldcoord = (float *)mesh->GetPoints()->GetVoidPointer(0);
        float *newcoord = (float *)outp->GetVoidPointer(0);

        // Set the known ones
        bhf_float->CopyOldValues(d1, oldcoord, newcoord, true, 3);

        // Match the unknown ones
        bhf_float->SetNewBoundaryData(d1, coord, newcoord, true, 3);

        // Set the remaining unset ones (reduced connectivity, etc.)
        bhf_float->FakeNonexistentBoundaryData(d1, newcoord, true, 3);

        CreateGhostZones(outm, mesh, bi);

        out[d] = outm;
    }
 
    bhf_float->FreeBoundaryData(coord);

    return out;
}

// ****************************************************************************
//  Method:  avtRectilinearDomainBoundaries::ExchangeMesh
//
//  Purpose:
//    Exchange the ghost zone information for some meshes,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    mesh         an array of meshes
//
//  Programmer:  Hank Childs
//  Creation:    November 10, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 12 19:21:22 PST 2004
//    Added check and exception for wrong VTK grid type
//
//    Mark C. Miller, Thu Apr 21 09:37:41 PDT 2005
//    Fixed leak for coord arrays
//
//    Hank Childs, Tue Jun 21 14:50:55 PDT 2005
//    Just fake the new coordinates rather than undertaking a painful
//    communication phase.  This should only bite us if we are doing gradient
//    calculations, or something that depends on mesh position.
//
//    Hank Childs, Wed Jun 29 15:24:35 PDT 2005
//    Cache domain2proc.
//
// ****************************************************************************

vector<vtkDataSet*>
avtRectilinearDomainBoundaries::ExchangeMesh(vector<int>        domainNum,
                                            vector<vtkDataSet*> meshes)
{
    if (domain2proc.size() == 0)
    {
        domain2proc = CreateDomainToProcessorMap(domainNum);
        CreateCurrentDomainBoundaryInformation(domain2proc);
    }

    vector<vtkDataSet*> out(meshes.size(), NULL);

    //
    // Create the matching arrays for the given meshes
    //
    int d;
    for (d = 0; d < meshes.size(); d++)
    {
        if (meshes[d]->GetDataObjectType() != VTK_RECTILINEAR_GRID)
        {
            EXCEPTION1(VisItException,
                       "avtRectilinearDomainBoundaries: "
                       "VTK data object type not VTK_RECTILINEAR_GRID");
        }

        vtkRectilinearGrid *mesh = (vtkRectilinearGrid*)(meshes[d]);

        int d1 = domainNum[d];
        Boundary *bi = &boundary[d1];

        vtkRectilinearGrid    *outm  = vtkRectilinearGrid::New(); 
        vtkFloatArray         *x     = vtkFloatArray::New();
        vtkFloatArray         *y     = vtkFloatArray::New();
        vtkFloatArray         *z     = vtkFloatArray::New();
        outm->SetXCoordinates(x);
        outm->SetYCoordinates(y);
        outm->SetZCoordinates(z);
        x->Delete();
        y->Delete();
        z->Delete();
        outm->SetDimensions(bi->newndims);
        x->SetNumberOfTuples(bi->newndims[0]);
        y->SetNumberOfTuples(bi->newndims[1]);
        z->SetNumberOfTuples(bi->newndims[2]);

        float *oldx = (float *)mesh->GetXCoordinates()->GetVoidPointer(0);
        float *oldy = (float *)mesh->GetYCoordinates()->GetVoidPointer(0);
        float *oldz = (float *)mesh->GetZCoordinates()->GetVoidPointer(0);
        float *newx = (float *)x->GetVoidPointer(0);
        float *newy = (float *)y->GetVoidPointer(0);
        float *newz = (float *)z->GetVoidPointer(0);
     
        int  i;
        for (i = 0 ; i < bi->newndims[0] ; i++)
        {
            int id = i+bi->newnextents[0];
            if (id < bi->oldnextents[0])
            {
                float last_dist = (oldx[1] - oldx[0]);
                int   num_off = (bi->oldnextents[0]-id);
                newx[i] = oldx[0] - last_dist*num_off;
            }
            else if (id >= bi->oldnextents[1])
            {
                float last_dist = (oldx[bi->oldndims[0]-1] - 
                                   oldx[bi->oldndims[0]-2]);
                int   num_off = (id - bi->oldnextents[1]);
                newx[i] = oldx[bi->oldndims[0]-1] + last_dist*num_off;
            }
            else
            {
                int oldindex = bi->OldPointIndex(id, 0, 0);
                int newindex = bi->NewPointIndex(id, 0, 0);
                int oldI = oldindex % bi->oldndims[0];
                int newI = newindex % bi->newndims[0];
                newx[newI] = oldx[oldI];
            }
        }
        for (i = 0 ; i < bi->newndims[1] ; i++)
        {
            int id = i+bi->newnextents[2];
            if (id < bi->oldnextents[2])
            {
                float last_dist = (oldy[1] - oldy[0]);
                int   num_off = (bi->oldnextents[2]-id);
                newy[i] = oldy[0] - last_dist*num_off;
            }
            else if (id >= bi->oldnextents[3])
            {
                float last_dist = (oldy[bi->oldndims[1]-1] - 
                                   oldy[bi->oldndims[1]-2]);
                int   num_off = (id - bi->oldnextents[3]);
                newy[i] = oldy[bi->oldndims[1]-1] + last_dist*num_off;
            }
            else
            {
                int oldindex = bi->OldPointIndex(0, id, 0);
                int newindex = bi->NewPointIndex(0, id, 0);
                int oldJ = (oldindex/bi->oldndims[0]) % bi->oldndims[1];
                int newJ = (newindex/bi->newndims[0]) % bi->newndims[1];
                newy[newJ] = oldy[oldJ];
            }
        }
        for (i = 0 ; i < bi->newndims[2] ; i++)
        {
            int id = i+bi->newnextents[4];
            if (id < bi->oldnextents[4])
            {
                float last_dist = (oldz[1] - oldz[0]);
                int   num_off = (bi->oldnextents[4]-id);
                newz[i] = oldz[0] - last_dist*num_off;
            }
            else if (id >= bi->oldnextents[5])
            {
                float last_dist = (oldz[bi->oldndims[2]-1] - 
                                   oldz[bi->oldndims[2]-2]);
                int   num_off = (id - bi->oldnextents[5]);
                newz[i] = oldz[bi->oldndims[2]-1] + last_dist*num_off;
            }
            else
            {
                int oldindex = bi->OldPointIndex(0, 0, id);
                int newindex = bi->NewPointIndex(0, 0, id);
                int oldK = (oldindex/(bi->oldndims[0]*bi->oldndims[1])) 
                         % bi->oldndims[2];
                int newK = (newindex/(bi->newndims[0]*bi->newndims[1])) 
                         % bi->newndims[2];
                newz[newK] = oldz[oldK];
            }
        }

        CreateGhostZones(outm, mesh, bi);

        out[d] = outm;
    }

    return out;
}

// ****************************************************************************
//  Method: avtStructuredDomainBoundaries::CreateGhostNodes
//
//  Purpose:
//      Creates ghost nodes.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2004
//
//  Modifications:
//
//    Hank Childs, Tue Aug 24 09:25:39 PDT 2004
//    Create avtRealDims.
//
//    Hank Childs, Fri Aug 27 16:16:52 PDT 2004
//    Renamed ghost data array.
//
//    Hank Childs, Sun Feb 27 14:47:45 PST 2005
//    Added argument allDomains.
//
//    Hank Childs, Mon Mar 28 13:38:14 PST 2005
//    Do not create domain-processor map, since that requires communication
//    and cannot be used with dynamic load balancing.
//
//    Brad Whitlock, Tue May 10 15:08:21 PST 2005
//    Fixed for win32.
//
// ****************************************************************************

void
avtStructuredDomainBoundaries::CreateGhostNodes(vector<int>         domainNum,
                                                vector<vtkDataSet*> meshes,
                                                vector<int> &allDomains)
{
    //
    // If we are doing DLB, we want to mark nodes as ghost, even if their
    // neighboring domains are not being used on this iteration.  Do this by
    // consulting the "allDomains" list.  Note that we can only play this
    // trick because the rest of the routine does not care which domains 
    // are on which processors -- only that we are using them.
    //
    int i, ntotaldomains = wholeBoundary.size();
    vector<int> domain2proc(ntotaldomains, -1);
    for (i = 0 ; i < allDomains.size() ; i++)
    {
        if (domain2proc[allDomains[i]] < 0)
            domain2proc[allDomains[i]] = 0;
    }

    CreateCurrentDomainBoundaryInformation(domain2proc);

    for (i = 0 ; i < domainNum.size() ; i++)
    {
        int dom = domainNum[i];
        Boundary *bi = &boundary[dom];

        vtkDataSet *ds = meshes[i];
        int npts = ds->GetNumberOfPoints();

        vtkUnsignedCharArray *gn = vtkUnsignedCharArray::New();
        gn->SetNumberOfTuples(npts);
        gn->SetName("avtGhostNodes");
        unsigned char *gnp = gn->GetPointer(0);
   
        for (int j = 0 ; j < npts ; j++)
            gnp[j] = 0;

        int dims[3];
        dims[0] = bi->oldnextents[1] - bi->oldnextents[0] + 1;
        dims[1] = bi->oldnextents[3] - bi->oldnextents[2] + 1;
        dims[2] = bi->oldnextents[5] - bi->oldnextents[4] + 1;

        if (bi->newnextents[0] < bi->oldnextents[0])
        {
            for (int k = 0 ; k < dims[2] ; k++)
                for (int j = 0 ; j < dims[1] ; j++)
                {
                    int idx = 0 + j*dims[0] + k*dims[0]*dims[1];
                    avtGhostData::AddGhostNodeType(gnp[idx], DUPLICATED_NODE);
                }
        }
        if (bi->newnextents[1] > bi->oldnextents[1])
        {
            for (int k = 0 ; k < dims[2] ; k++)
                for (int j = 0 ; j < dims[1] ; j++)
                {
                    int idx = dims[0]-1 + j*dims[0] + k*dims[0]*dims[1];
                    avtGhostData::AddGhostNodeType(gnp[idx], DUPLICATED_NODE);
                }
        }
        if (bi->newnextents[2] < bi->oldnextents[2])
        {
            for (int k = 0 ; k < dims[2] ; k++)
                    for (int i = 0 ; i < dims[0] ; i++)
                {
                    int idx = i + 0*dims[0] + k*dims[0]*dims[1];
                    avtGhostData::AddGhostNodeType(gnp[idx], DUPLICATED_NODE);
                }
        }
        if (bi->newnextents[3] > bi->oldnextents[3])
        {
            for (int k = 0 ; k < dims[2] ; k++)
                for (int i = 0 ; i < dims[0] ; i++)
                {
                    int idx = i + (dims[1]-1)*dims[0] + k*dims[0]*dims[1];
                    avtGhostData::AddGhostNodeType(gnp[idx], DUPLICATED_NODE);
                }
        }
        if (bi->newnextents[4] < bi->oldnextents[4])
        {
            for (int j = 0 ; j < dims[1] ; j++)
                for (int i = 0 ; i < dims[0] ; i++)
                {
                        int idx = i + j*dims[0] + 0*dims[0]*dims[1];
                    avtGhostData::AddGhostNodeType(gnp[idx], DUPLICATED_NODE);
                }
        }
        if (bi->newnextents[5] > bi->oldnextents[5])
        {
                for (int j = 0 ; j < dims[1] ; j++)
                for (int i = 0 ; i < dims[0] ; i++)
                {
                    int idx = i + j*dims[0] + (dims[2]-1)*dims[0]*dims[1];
                    avtGhostData::AddGhostNodeType(gnp[idx], DUPLICATED_NODE);
                }
        }
    
        ds->GetPointData()->AddArray(gn);
        gn->Delete();

        //
        //  Create a field-data array indicating the extents of real zones.
        //  Used during ghostzone removal.
        //
        vtkIntArray *realDims = vtkIntArray::New();
        realDims->SetName("avtRealDims");
        realDims->SetNumberOfTuples(6);
        realDims->SetValue(0, 0);
        realDims->SetValue(1, dims[0]);
        realDims->SetValue(2, 0);
        realDims->SetValue(3, dims[1]);
        realDims->SetValue(4, 0);
        realDims->SetValue(5, dims[2]);
        ds->GetFieldData()->AddArray(realDims);
        ds->GetFieldData()->CopyFieldOn("avtRealDims");
        realDims->Delete();
    }
}

// ****************************************************************************
//  Method: avtStructuredDomainBoundaries::SetIndicesForRectGrid
//
//  Purpose:
//      Sets the indices for a rectilinear grid.  This just sets some state
//      information that will be used by 'CalculateBoundaries' later.
//
//  Programmer: Hank Childs
//  Creation:   November 11, 2003
//
// ****************************************************************************

void
avtStructuredDomainBoundaries::SetIndicesForRectGrid(int domain, int e[6])
{
    SetIndicesForAMRPatch(domain, 0, e);
}

// ****************************************************************************
//  Method: avtStructuredDomainBoundaries::SetIndicesForAMRPatch
//
//  Purpose:
//      Sets the indices for an AMR patch.  This just sets some state
//      information that will be used by 'CalculateBoundaries' later.
//
//  Programmer: Hank Childs
//  Creation:   November 11, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 12 17:29:19 PST 2004
//    Added code to disallow operation if shouldComputeNeighborsFromExtents is
//    not true.
//
//    Kathleen Bonnell, Tue Jan 20 17:26:40 PST 2004
//    Reversed order of Exceptions, per Mark Miller's request.
// 
// ****************************************************************************

void
avtStructuredDomainBoundaries::SetIndicesForAMRPatch(int domain, 
                                                     int level, int e[6])
{
    if (!shouldComputeNeighborsFromExtents)
    {
        EXCEPTION1(VisItException,
                   "avtStructuredDomainBoundaries: "
                   "passing indices for a mesh that does not support "
                   "computation of neighbors from index extents");
    }

    if (domain >= levels.size())
        EXCEPTION1(VisItException,
                   "avtStructuredDomainBoundaries: "
                   "targetted domain more than number of domains");


    levels[domain] = level;
    extents[6*domain+0] = e[0];
    extents[6*domain+1] = e[1];
    extents[6*domain+2] = e[2];
    extents[6*domain+3] = e[3];
    extents[6*domain+4] = e[4];
    extents[6*domain+5] = e[5];

    int tmp[6];
    tmp[0] = 1;
    tmp[1] = e[1] - e[0] + 1;
    tmp[2] = 1;
    tmp[3] = e[3] - e[2] + 1;
    tmp[4] = 1;
    tmp[5] = e[5] - e[4] + 1;
    SetExtents(domain, tmp);
}

// ****************************************************************************
//  Method: avtStructuredDomainBoundaries::CalculateBoundaries
//
//  Purpose:
//      Calculates the boundaries between rectilinear grids.
//
//  Programmer: Hank Childs
//  Creation:   November 11, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 12 17:29:19 PST 2004
//    Added code to disallow operation if shouldComputeNeighborsFromExtents is
//    not true.
//
//    Hank Childs, Mon Jun 27 10:48:41 PDT 2005
//    Re-wrote to use interval trees for more efficient overlap finding.
//
//    Hank Childs, Wed Jul  6 06:56:02 PDT 2005
//    Do some sorting so that we know the "match" entry for the neighbor
//    index will be correct.
//
//    Hank Childs, Fri Jun  9 14:18:11 PDT 2006
//    Remove unused variable.
//
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
// ****************************************************************************

void
avtStructuredDomainBoundaries::CalculateBoundaries(void)
{
    int i, j;

    if (!shouldComputeNeighborsFromExtents)
    {
        EXCEPTION1(VisItException,
                   "avtStructuredDomainBoundaries: "
                   "passing indices for a mesh that does not support "
                   "computation of neighbors from index extents");
    }

    int ndoms = levels.size();
    avtIntervalTree itree(ndoms, 3);
    double extf[6];
    for (i = 0 ; i < ndoms ; i++)
    {
        for (j = 0 ; j < 6 ; j++)
            extf[j] = (double) extents[6*i+j];
        itree.AddElement(i, extf);
    }
    itree.Calculate(true);

    vector<int> neighbors(ndoms, 0);
    for (i = 0 ; i < ndoms ; i++)
    {
        double min_vec[3], max_vec[3];
        min_vec[0] = (double) extents[6*i+0];
        min_vec[1] = (double) extents[6*i+2];
        min_vec[2] = (double) extents[6*i+4];
        max_vec[0] = (double) extents[6*i+1];
        max_vec[1] = (double) extents[6*i+3];
        max_vec[2] = (double) extents[6*i+5];
        vector<int> list;
        itree.GetElementsListFromRange(min_vec, max_vec, list);

        // To get the "match" entry correct, we have to sort the list.  This
        // will ensure that we can predict what a domain's match number will be
        // for its neighbor.
        sort(list.begin(), list.end());

        for (j = 0 ; j < list.size() ; j++)
        {
            if (i == list[j])
                continue; // Not interested in self-intersection.

            int orientation[3] = { 1, 2, 3 }; // this doesn't really
                                              // apply for rectilinear.
            int d1 = i;
            int d2 = list[j];
            if (levels[d1] != levels[d2])
                continue;
            int e[6];
            e[0] = (extents[6*d1+0] > extents[6*d2+0] ? extents[6*d1+0] 
                                                      : extents[6*d2+0]);
            e[0] -= extents[6*d1+0] - 1;
            e[1] = (extents[6*d1+1] < extents[6*d2+1] ? extents[6*d1+1] 
                                                      : extents[6*d2+1]);
            e[1] -= extents[6*d1+0] - 1;
            e[2] = (extents[6*d1+2] > extents[6*d2+2] ? extents[6*d1+2] 
                                                      : extents[6*d2+2]);
            e[2] -= extents[6*d1+2] - 1;
            e[3] = (extents[6*d1+3] < extents[6*d2+3] ? extents[6*d1+3] 
                                                      : extents[6*d2+3]);
            e[3] -= extents[6*d1+2] - 1;
            e[4] = (extents[6*d1+4] > extents[6*d2+4] ? extents[6*d1+4] 
                                                      : extents[6*d2+4]);
            e[4] -= extents[6*d1+4] - 1;
            e[5] = (extents[6*d1+5] < extents[6*d2+5] ? extents[6*d1+5] 
                                                      : extents[6*d2+5]);
            e[5] -= extents[6*d1+4] - 1;
            int index = neighbors[d2];
            neighbors[d2]++;
            AddNeighbor(d1, d2, index, orientation, e);
        }
    }

    //
    // This will perform some calculations that are necessary to do the actual
    // communication.
    //
    for (i = 0 ; i < ndoms ; i++)
    {
        Finish(i);
    }
}


// ****************************************************************************
//  Method:  avtStructuredDomainBoundaries::GetExtents
//
//  Purpose:
//    Get the extents of one domain.
//
//  Arguments:
//    domain     the domain to get the extents of
//    e          the extents
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    February 8, 2005 
//
// ****************************************************************************

void
avtStructuredDomainBoundaries::GetExtents(int domain, int e[6])
{
    if (domain >= wholeBoundary.size())
        EXCEPTION1(VisItException,
                   "avtStructuredDomainBoundaries: "
                   "targetted domain more than number of domains");

    e[0] = wholeBoundary[domain].oldnextents[0];
    e[1] = wholeBoundary[domain].oldnextents[1];
    e[2] = wholeBoundary[domain].oldnextents[2];
    e[3] = wholeBoundary[domain].oldnextents[3];
    e[4] = wholeBoundary[domain].oldnextents[4];
    e[5] = wholeBoundary[domain].oldnextents[5];
}

