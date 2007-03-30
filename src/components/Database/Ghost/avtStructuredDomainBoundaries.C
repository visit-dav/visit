#include <avtStructuredDomainBoundaries.h>
#include <VisItException.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkStructuredGrid.h>
#include <avtMaterial.h>
#include <avtMixedVariable.h>


#ifdef PARALLEL
#include <mpi.h>
#endif


using   std::string;


// ----------------------------------------------------------------------------
//                            private helper methods
// ----------------------------------------------------------------------------

#ifdef PARALLEL
template <class T> int GetMPIDataType();
template <>        int GetMPIDataType<int>()    { return MPI_INT;  }
template <>        int GetMPIDataType<float>()  { return MPI_FLOAT;}
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
// ****************************************************************************
vector<int>
avtStructuredDomainBoundaries::CreateDomainToProcessorMap(const vector<int> &domainNum)
{
    // get the processor rank
    int rank = 0;
#ifdef PARALLEL
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
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
                  MPI_MAX, MPI_COMM_WORLD);
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
// ****************************************************************************
void
avtStructuredDomainBoundaries::CreateCurrentDomainBoundaryInformation(
                                                const vector<int> &domain2proc)
{
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
                boundary[i].DeleteNeighbor(wbi.neighbors[j].domain);
        }
    }
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
        int mi = sdb->boundary[d2].FindNeighborIndex(d1);
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
        int mi = sdb->boundary[d2].FindNeighborIndex(d1);
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
// ****************************************************************************
template <class T>
void
BoundaryHelperFunctions<T>::CommunicateBoundaryData(const vector<int> &domain2proc,
                                                       T     ***bnddata,
                                                       bool     isPointData,
                                                       int      ncomp)
{
#ifdef PARALLEL
    int mpi_datatype = GetMPIDataType<T>();

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    for (int d1 = 0; d1 < sdb->boundary.size(); d1++)
    {
        Boundary *bi = &sdb->boundary[d1];

        // find matching neighbors
        for (int n=0; n<bi->neighbors.size(); n++)
        {
            Neighbor *n1 = &bi->neighbors[n];
            int d2 = n1->domain;
            int mi = sdb->boundary[d2].FindNeighborIndex(bi->domain);
            int size = ncomp * (isPointData ? n1->npts : n1->ncells);

            if (domain2proc[d1] != domain2proc[d2])
            {
                if (domain2proc[d1] == rank)
                {
                    MPI_Send(bnddata[d1][n], size, mpi_datatype,
                             domain2proc[d2], rank, MPI_COMM_WORLD);
                }
                else if (domain2proc[d2] == rank)
                {
                    bnddata[d1][n] = new T[size];

                    MPI_Status stat;
                    MPI_Recv(bnddata[d1][n], size, mpi_datatype,
                             domain2proc[d1], MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
                }
            }
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
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

    int mpi_datatype = GetMPIDataType<T>();
    MPI_Status stat;

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    for (int d1 = 0; d1 < sdb->boundary.size(); d1++)
    {
        Boundary *bi = &sdb->boundary[d1];

        // find matching neighbors
        for (int n=0; n<bi->neighbors.size(); n++)
        {
            Neighbor *n1 = &bi->neighbors[n];
            int d2 = n1->domain;
            int mi = sdb->boundary[d2].FindNeighborIndex(bi->domain);

            if (domain2proc[d1] != domain2proc[d2])
            {
                if (domain2proc[d1] == rank)
                {
                    MPI_Send(&(bndmixlen[d1][n]), 1, MPI_INT,
                             domain2proc[d2], rank, MPI_COMM_WORLD);
                }
                else if (domain2proc[d2] == rank)
                {
                    MPI_Recv(&(bndmixlen[d1][n]), 1, MPI_INT,
                             domain2proc[d1], MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
                }
            }
        }
    }

    for (int d1 = 0; d1 < sdb->boundary.size(); d1++)
    {
        Boundary *bi = &sdb->boundary[d1];

        // find matching neighbors
        for (int n=0; n<bi->neighbors.size(); n++)
        {
            Neighbor *n1 = &bi->neighbors[n];
            int d2 = n1->domain;
            int mi = sdb->boundary[d2].FindNeighborIndex(bi->domain);
            int size = bndmixlen[d1][n];

            if (domain2proc[d1] != domain2proc[d2])
            {
            
                if (domain2proc[d1] == rank)
                {
                    if (bnddata)
                        MPI_Send(bnddata[d1][n], size, mpi_datatype,
                                 domain2proc[d2], rank, MPI_COMM_WORLD);
                    if (bndmixmat)
                        MPI_Send(bndmixmat[d1][n], size, MPI_INT,
                                 domain2proc[d2], rank, MPI_COMM_WORLD);
                    if (bndmixzone)
                        MPI_Send(bndmixzone[d1][n], size, MPI_INT,
                                 domain2proc[d2], rank, MPI_COMM_WORLD);
                }
                else if (domain2proc[d2] == rank)
                {
                    if (bnddata)
                    {
                        bnddata[d1][n] = new T[size];
                        MPI_Recv(bnddata[d1][n], size, mpi_datatype,
                                 domain2proc[d1], MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
                    }
                    if (bndmixmat)
                    {
                        bndmixmat[d1][n] = new int[size];
                        MPI_Recv(bndmixmat[d1][n], size, MPI_INT,
                                 domain2proc[d1], MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
                    }
                    if (bndmixzone)
                    {
                        bndmixzone[d1][n] = new int[size];
                        MPI_Recv(bndmixzone[d1][n], size, MPI_INT,
                                 domain2proc[d1], MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
                    }
                }
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
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

        int mi = sdb->boundary[d2].FindNeighborIndex(bi->domain);
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
        int mi = sdb->boundary[d2].FindNeighborIndex(bi->domain);

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
// ****************************************************************************
avtStructuredDomainBoundaries::avtStructuredDomainBoundaries()
{
    bhf_int   = new BoundaryHelperFunctions<int>(this);
    bhf_float = new BoundaryHelperFunctions<float>(this);
}

// ****************************************************************************
//  Destructor:  avtStructuredDomainBoundaries::~avtStructuredDomainBoundaries
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 25, 2001
//
// ****************************************************************************
avtStructuredDomainBoundaries::~avtStructuredDomainBoundaries()
{
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
// ****************************************************************************
void
avtStructuredDomainBoundaries::SetNumDomains(int nd)
{
    wholeBoundary.resize(nd);
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
//  Method:  avtStructuredDomainBoundaries::ExchangeMesh
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
// ****************************************************************************
vector<vtkDataSet*>
avtStructuredDomainBoundaries::ExchangeMesh(vector<int>         domainNum,
                                            vector<vtkDataSet*> meshes)
{
    vector<int> domain2proc = CreateDomainToProcessorMap(domainNum);
    CreateCurrentDomainBoundaryInformation(domain2proc);

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

        // Create the ghost zone array
        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName("vtkGhostLevels");
        ghostCells->Allocate(bi->newncells);
        for (int k = bi->newzextents[4]; k <= bi->newzextents[5]; k++)
            for (int j = bi->newzextents[2]; j <= bi->newzextents[3]; j++)
                for (int i = bi->newzextents[0]; i <= bi->newzextents[1]; i++)
                    ghostCells->InsertNextValue(bi->IsGhostZone(i,j,k) ? 1:0);
        outm->GetCellData()->AddArray(ghostCells);
        ghostCells->Delete();
        outm->SetUpdateGhostLevel(0);

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
        outm->GetFieldData()->AddArray(realDims);
        outm->GetFieldData()->CopyFieldOn("avtRealDims");
        realDims->Delete();

        out[d] = outm;
    }
 
    bhf_float->FreeBoundaryData(coord);

    return out;
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
// ****************************************************************************
vector<vtkDataArray*>
avtStructuredDomainBoundaries::ExchangeScalar(vector<int>         domainNum,
                                              bool                isPointData,
                                              vector<vtkDataArray*> scalars)
{
    vector<int> domain2proc = CreateDomainToProcessorMap(domainNum);
    CreateCurrentDomainBoundaryInformation(domain2proc);

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
// ****************************************************************************
vector<vtkDataArray*>
avtStructuredDomainBoundaries::ExchangeFloatVector(vector<int>      domainNum,
                                              bool                  isPointData,
                                              vector<vtkDataArray*> vectors)
{
    vector<int> domain2proc = CreateDomainToProcessorMap(domainNum);
    CreateCurrentDomainBoundaryInformation(domain2proc);

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
// ****************************************************************************
vector<vtkDataArray*>
avtStructuredDomainBoundaries::ExchangeIntVector(vector<int>        domainNum,
                                                 bool               isPointData,
                                                 vector<vtkDataArray*> vectors)
{
    vector<int> domain2proc = CreateDomainToProcessorMap(domainNum);
    CreateCurrentDomainBoundaryInformation(domain2proc);

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
// ****************************************************************************
vector<avtMaterial*>
avtStructuredDomainBoundaries::ExchangeMaterial(vector<int>          domainNum,
                                                vector<avtMaterial*> mats)
{
    vector<int> domain2proc = CreateDomainToProcessorMap(domainNum);
    CreateCurrentDomainBoundaryInformation(domain2proc);

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
            int d1 = bi.domain;
            int d2 = bi.neighbors[n].domain;
            int mi = boundary[d2].FindNeighborIndex(d1);
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
// ****************************************************************************
vector<avtMixedVariable*>
avtStructuredDomainBoundaries::ExchangeMixVar(vector<int>            domainNum,
                                          const vector<avtMaterial*> mats,
                                          vector<avtMixedVariable*>  mixvars)
{
    vector<int> domain2proc = CreateDomainToProcessorMap(domainNum);
    CreateCurrentDomainBoundaryInformation(domain2proc);

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
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int length = 0;
    if (mixvarname != NULL)
    {
        length = strlen(mixvarname)+1;
    }
    struct {int length; int rank;} len_rank_out, len_rank_in={length, rank};

    MPI_Allreduce(&len_rank_in, &len_rank_out, 1, MPI_2INT, MPI_MAXLOC,
                  MPI_COMM_WORLD);
    length = len_rank_out.length;

    char *mvname = new char[length];
    if (mixvarname != NULL)
    {
        strcpy(mvname, mixvarname);
    }

    MPI_Bcast(mvname, length, MPI_CHAR, len_rank_out.rank, MPI_COMM_WORLD);
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
            int d1 = bi.domain;
            int d2 = bi.neighbors[n].domain;
            int mi = boundary[d2].FindNeighborIndex(d1);
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
