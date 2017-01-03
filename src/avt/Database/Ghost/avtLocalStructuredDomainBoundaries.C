/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
//                  avtLocalStructuredDomainBoundaries.C                     //
// ************************************************************************* //

#include <avtLocalStructuredDomainBoundaries.h>

#include <cstring>

#ifdef PARALLEL
#include <mpi.h>
#endif
#include <DebugStream.h>
#include <avtParallel.h>
#include <TimingsManager.h>

using   std::vector;

// ****************************************************************************
//  Constructor:  avtLocalStructuredDomainBoundaryList::
//                                 avtLocalStructuredDomainBoundaryList
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Apr 19 11:03:08 PDT 2012
//
//  Modifications:
//
// ****************************************************************************
avtLocalStructuredDomainBoundaryList::avtLocalStructuredDomainBoundaryList
(int domain_id, int extents[6])
: domainId(domain_id)
{
    memcpy(this->extents,extents,sizeof(int)*6);
}


// ****************************************************************************
//  Constructor:  avtLocalStructuredDomainBoundaryList::
//                                 avtLocalStructuredDomainBoundaryList
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Apr 19 11:03:08 PDT 2012
//
//  Modifications:
//
// ****************************************************************************
avtLocalStructuredDomainBoundaryList::avtLocalStructuredDomainBoundaryList
(int *ptr)
{
    int *msg_ptr = ptr;
    domainId = msg_ptr[0];
    memcpy(this->extents,&msg_ptr[1],sizeof(int)*6);
    int nnei = msg_ptr[7];
    // adv to start of neighbors
    msg_ptr += 8;
    for(int i=0;i<nnei;i++)
    {
        DomainNeighbor *nei = new DomainNeighbor(msg_ptr);
        msg_ptr += nei->PackedSize();
        neighbors.push_back(nei);
    }
}


// ****************************************************************************
//  Constructor:  avtLocalStructuredDomainBoundaryList::
//                                 avtLocalStructuredDomainBoundaryList
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Apr 19 11:03:08 PDT 2012
//
//  Modifications:
//
// ****************************************************************************
avtLocalStructuredDomainBoundaryList::avtLocalStructuredDomainBoundaryList
(avtLocalStructuredDomainBoundaryList *lst)
{
    domainId = lst->domainId;
    memcpy(this->extents,lst->extents,sizeof(int)*6);
    int nnei = (int)lst->neighbors.size();
    for(int i=0;i<nnei;i++)
    {
        DomainNeighbor *nei = new DomainNeighbor(lst->neighbors[i]);
        neighbors.push_back(nei);
    }
}



// ****************************************************************************
//  Destructor:  avtLocalStructuredDomainBoundaryList::
//                                 ~avtLocalStructuredDomainBoundaryList
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Apr 19 11:03:08 PDT 2012
//
//  Modifications:
//
// ****************************************************************************
avtLocalStructuredDomainBoundaryList::~avtLocalStructuredDomainBoundaryList()
{
    vector<DomainNeighbor *>::iterator itr;
    for (itr = neighbors.begin(); itr !=neighbors.end(); ++itr)
        delete *itr;
    neighbors.clear();
}

// ****************************************************************************
//  Destructor:  avtLocalStructuredDomainBoundaryList::Destruct
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Apr 19 11:03:08 PDT 2012
//
//  Modifications:
//
// ****************************************************************************
void
avtLocalStructuredDomainBoundaryList::Destruct(void *p)
{
    avtLocalStructuredDomainBoundaryList *lsdb =
                                     (avtLocalStructuredDomainBoundaryList *) p;
    delete lsdb;
}

// ****************************************************************************
//  Method:  avtLocalStructuredDomainBoundaryList::AddNeighbor
//
//  Purpose:
//    Adds a neighbor entry to the neighbor list.
//
//  Arguments:
//    neighbor_domain_id    the domain number of the new neigbor
//    match                 the current domain's index in the neighbor's neighbor list
//    orientation           the three orientation values
//    extents               the extents of the matching boundary in the current domain
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Apr 19 11:03:08 PDT 2012
//
//  Modifications:
//
// ****************************************************************************
void
avtLocalStructuredDomainBoundaryList::AddNeighbor(int neighbor_domain_id,
                                                  int match,
                                                  int orientation[3],
                                                  int extents[6])
{
    DomainNeighbor *nei = new DomainNeighbor(neighbor_domain_id,
                                             match,
                                             orientation,
                                             extents);
    neighbors.push_back(nei);
}


// ****************************************************************************
//  Method:  avtLocalStructuredDomainBoundaryList::Pack
//
//  Purpose:
//    Helper used to pack avtLocalStructuredDomainBoundaryList for MPI comm.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Apr 19 11:03:08 PDT 2012
//
//  Modifications:
//
// ****************************************************************************
int
avtLocalStructuredDomainBoundaryList::Pack(int *ptr) const
{
    int *msg_ptr = ptr;
    msg_ptr[0] = domainId;
    memcpy(&msg_ptr[1],this->extents,sizeof(int)*6);
    int nnei = NumberOfNeighbors();
    msg_ptr[7] = nnei;
    // adv to start of neighbors
    msg_ptr += 8;
    for(int i=0; i < nnei; i++)
    {
        int sz = neighbors[i]->Pack(msg_ptr);
        msg_ptr += sz;
    }

    return PackedSize();
}


// ****************************************************************************
//  Method: avtLocalStructuredDomainBoundaryList::Print
//
//  Purpose:
//    Pretty print helper for avtLocalStructuredDomainBoundaryList instances.
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Apr 19 11:03:08 PDT 2012
//
//  Modifications:
//
// ****************************************************************************
void
avtLocalStructuredDomainBoundaryList::Print(ostream &os) const
{
    os << "------------------------------------------" <<endl;
    os << "  avtLocalStructuredDomainBoundaryList"     <<endl;
    os << "------------------------------------------" <<endl;
    os << "domainId  = " << domainId << endl;
    os << "extents:\n ";
    for(int i=0;i<6;i++)
        os << " " << extents[i];
    os << endl;
    int nnei = NumberOfNeighbors();
    os << "# of neighbors = " << nnei << endl;
    for(int i=0;i<nnei;i++)
    {
        os << "neighbors[" << i<< "]:" << endl;
        neighbors[i]->Print(os);
    }
    os << "------------------------------------------" <<endl;
}


// ****************************************************************************
//  Method:  avtLocalStructuredDomainBoundaryList::GlobalGenerate
//
//  Purpose:
//     Creates a global domain boundaries object from local lists on all
//     MPI tasks.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Apr 19 11:03:08 PDT 2012
//
//  Modifications:
//
// ****************************************************************************
avtStructuredDomainBoundaries *
avtLocalStructuredDomainBoundaryList::GlobalGenerate
(const vector<avtLocalStructuredDomainBoundaryList*> &local_lists)
{
    int t_ggen = visitTimer->StartTimer();
    
    vector<avtLocalStructuredDomainBoundaryList*> all_lists;
    // get all domains
    GlobalGather(local_lists,all_lists);
    debug1 << "avtLocalStructuredDomainBoundaryList::GlobalGenerate: all lists size ="
           << all_lists.size() <<endl;

    // if we don't actually have any lists, don't attempt to create
    // a dbi object.
    if(all_lists.size() == 0)
    {
        visitTimer->StopTimer(t_ggen,
                              "avtLocalStructuredDomainBoundaryList Global Generate");
        return NULL;
    }

    avtStructuredDomainBoundaries *dbi = NULL;

    //
    // TODO: we need to know if we want to create curvi or rect
    // (ALSO, it shouldn't matter at this point ....
    //

    dbi = new avtCurvilinearDomainBoundaries();

    //
    // The current ghost zone logic for expanding struct boundaries only
    // works when you add all domains, and then remove those that are off.
    // 
    //
    // for the local dbi case, we don't have all domains, but we do have
    // all neighbors for the current domains. We can still make things
    // work status quo if we add all of the neighbors, and avoid
    // pitfalls related to the "match" logic
    //

    // now construct the global object, include slots of all neighbors
    int ndoms = 0;
    for(size_t i=0; i<all_lists.size(); i++)
    {
        avtLocalStructuredDomainBoundaryList *lst = all_lists[i];
        if( (lst->domainId+1) > ndoms)
        {
            ndoms = lst->domainId + 1;
        }

        int nnei = (int)lst->neighbors.size();
        for(int j=0; j < nnei; j++)
        {
            DomainNeighbor *n = lst->neighbors[j];
            if( (n->neighborId+1) > ndoms)
            {ndoms = n->neighborId + 1;}
        }
    }

    // we want to screen the list so that only
    // the enabled domains are actually considered
    vector<bool> domains_enabled(ndoms,false);

    for(size_t i=0; i<all_lists.size(); i++)
    {
        avtLocalStructuredDomainBoundaryList *lst = all_lists[i];
        debug5 << "avtLocalStructuredDomainBoundaryList::GlobalGenerate: "
               << "Domain " << lst->domainId << " is enabled" << endl;
        domains_enabled[lst->domainId] = true;
    }


    dbi->SetNumDomains(ndoms);
    debug5 << "avtLocalStructuredDomainBoundaryList::GlobalGenerate: "
           << "Number of known domains " << ndoms << endl;
    for(size_t i=0; i<all_lists.size(); i++)
    {
        avtLocalStructuredDomainBoundaryList *lst = all_lists[i];
        int domain_id = lst->domainId;
        //lst->Print(cout);
        dbi->SetExtents(domain_id,lst->extents);
        int nnei = lst->NumberOfNeighbors();

        for(int j=0; j < nnei; j++)
        {
            DomainNeighbor *n = lst->neighbors[j];
            dbi->AddNeighbor(domain_id,
                             n->neighborId,
                             -1, // this signals that we don't care about match
                             n->orientation,
                             n->extents);
        }

        dbi->Finish(domain_id);
        // delete the list after we have it's info .
        delete all_lists[i];
    }

    all_lists.clear();

    visitTimer->StopTimer(t_ggen,
                          "avtLocalStructuredDomainBoundaryList Global Generate");

    return dbi;
}

// ****************************************************************************
//  Method:  avtLocalStructuredDomainBoundaryList::GlobalGather
//
//  Purpose:
//    Helper that gathers local lists from all MPI tasks into a complete
//    global list.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Apr 19 11:03:08 PDT 2012
//
//  Modifications:
//
// ****************************************************************************
void
avtLocalStructuredDomainBoundaryList::GlobalGather
(const std::vector<avtLocalStructuredDomainBoundaryList*> &local_lists,
  std::vector<avtLocalStructuredDomainBoundaryList*> &out)
{

#ifdef PARALLEL
    int t_ggather_par = visitTimer->StartTimer();

    debug1 << "avtLocalStructuredDomainBoundaryList::GlobalGather" <<endl;
    out.clear();
    // get the number of processors and the current processor id
    int nprocs = PAR_Size();

    int num_local_domains = 0;
    int local_msg_size    = 0;

    int nnei = (int)local_lists.size();

    for(int i=0; i < nnei; i++)
    {
        num_local_domains++;
        local_msg_size += local_lists[i]->PackedSize();
    }

    int *local_msg_buffer = new int[local_msg_size];
    int *local_msg_ptr    = local_msg_buffer;
    // create mpi buffers
    for(int i=0; i < nnei; i++)
    {
        int sz = local_lists[i]->Pack(local_msg_ptr);
        local_msg_ptr += sz;
    }

    // gather the # of domains per processor
    // and the msg sizes
    int *rcv_num_domains = new int[nprocs];
    int *rcv_count       = new int[nprocs];
    int *rcv_disp        = new int[nprocs];


    MPI_Allgather(&num_local_domains,1,MPI_INT,
                  rcv_num_domains,1,MPI_INT,
                  VISIT_MPI_COMM);

    MPI_Allgather(&local_msg_size,1,MPI_INT,
                  rcv_count,1,MPI_INT,
                  VISIT_MPI_COMM);


    // do comm
    // calculate the size of the receive message, and the displacements
    // for each processor
    rcv_disp[0] = 0;
    int rcv_msg_size = rcv_count[0];
    for(int i = 1; i < nprocs; i++)
    {
        rcv_disp[i]   = rcv_count[i-1] + rcv_disp[i-1];
        rcv_msg_size += rcv_count[i];
    }

    // allocate memory for the receive message
    int  *rcv_msg = new int[rcv_msg_size];
    MPI_Allgatherv(local_msg_buffer, local_msg_size, MPI_INT,
                   rcv_msg, rcv_count, rcv_disp, MPI_INT,
                   VISIT_MPI_COMM);

    // cleanup the send message
    delete [] local_msg_buffer;

    // unpack results
    for(int n = 0; n < nprocs; n++)
    {
        int nd       =  rcv_num_domains[n];
        int *msg_ptr = &rcv_msg[rcv_disp[n]];
        for(int ld = 0; ld < nd; ld++)
        {
            avtLocalStructuredDomainBoundaryList *lst =
                            new avtLocalStructuredDomainBoundaryList(msg_ptr);
            msg_ptr += lst->PackedSize();
            out.push_back(lst);
        }
    }
//     cout << "GLOBAL # lists " << out.size() <<endl;

    delete [] rcv_num_domains;
    delete [] rcv_count;
    delete [] rcv_disp;
    delete [] rcv_msg;

    visitTimer->StopTimer(t_ggather_par,
                          "avtLocalStructuredDomainBoundaryList Global Gather [Parallel]");

#else
    int t_ggather_ser = visitTimer->StartTimer();
    // we need to copy the local list b/c they will be modified
    for(size_t i=0; i<local_lists.size(); i++)
    {
        avtLocalStructuredDomainBoundaryList *lst = local_lists[i];
        out.push_back(new avtLocalStructuredDomainBoundaryList(lst));
    }

    visitTimer->StopTimer(t_ggather_ser,
                          "avtLocalStructuredDomainBoundaryList Global Gather [Serial]");
#endif
}


// ****************************************************************************
//  Constructor:  DomainNeighbor::DomainNeighbor
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Apr 19 11:03:08 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

avtLocalStructuredDomainBoundaryList::DomainNeighbor::DomainNeighbor
(int neighbor_id, int match, int orientation[3], int extents[6]):
neighborId(neighbor_id), match(match)
{
    memcpy(this->orientation,orientation,sizeof(int)*3);
    memcpy(this->extents,extents,sizeof(int)*6);
}

// ****************************************************************************
//  Constructor: DomainNeighbor::DomainNeighbor
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Apr 19 11:03:08 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

avtLocalStructuredDomainBoundaryList::DomainNeighbor::DomainNeighbor(int *ptr)
{
    neighborId = ptr[0];
    match      = ptr[1];
    memcpy(orientation,&ptr[2],sizeof(int)*3);
    memcpy(extents,&ptr[5],sizeof(int)*6);
}

avtLocalStructuredDomainBoundaryList::DomainNeighbor::DomainNeighbor
(DomainNeighbor *nei)
{
    neighborId = nei->neighborId;
    match = nei->match;
    memcpy(orientation,nei->orientation,sizeof(int)*3);
    memcpy(extents,nei->extents,sizeof(int)*6);
}



// ****************************************************************************
//  Destructor: DomainNeighbor::~DomainNeighbor
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Apr 19 11:03:08 PDT 2012
//
//  Modifications:
//
// ****************************************************************************
avtLocalStructuredDomainBoundaryList::DomainNeighbor::~DomainNeighbor()
{}

// ****************************************************************************
//  Method: DomainNeighbor::Pack
//
//  Purpose:
//    Helper used to pack a DomainNeighbor instance for MPI comm.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Apr 19 11:03:08 PDT 2012
//
//  Modifications:
//
// ****************************************************************************
int
avtLocalStructuredDomainBoundaryList::DomainNeighbor::Pack(int *ptr) const
{
    ptr[0] = neighborId;
    ptr[1] = match;
    memcpy(&ptr[2],orientation,sizeof(int)*3);
    memcpy(&ptr[5],extents,sizeof(int)*6);

    return PackedSize();
}

// ****************************************************************************
//  Method: DomainNeighbor::Print
//
//  Purpose:
//    Pretty print helper for DomainNeighbor instances.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Apr 19 11:03:08 PDT 2012
//
//  Modifications:
//
// ****************************************************************************
void
avtLocalStructuredDomainBoundaryList::DomainNeighbor::Print(ostream &os) const
{
    os << " neighborId  = " << neighborId << endl;
    os << " match       = " << match      << endl;
    os << " orientation:\n ";
    for(int i=0;i<3;i++)
        os << " " << orientation[i];
    os << endl;
    os << " extents:\n ";
    for(int i=0;i<6;i++)
        os << " " << extents[i];
    os << endl;
}






