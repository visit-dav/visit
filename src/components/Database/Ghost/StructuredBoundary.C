#include <avtStructuredDomainBoundaries.h>
#include <VisItException.h>
#include <vtkStructuredGrid.h>

#define BNDMIN(A,B) (((A) < (B)) ? (A) : (B))
#define BNDMAX(A,B) (((A) > (B)) ? (A) : (B))

// ****************************************************************************
//  Function:  CalculateIndex
//
//  Purpose:
//    Calculate a linear index using an i,j,k index in global indexing space
//
//  Arguments:
//    e          the extents of the current domain
//    d          the dimensions of the current domain
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
inline static int
CalculateIndex(int *e, int *d, int i, int j, int k)
{
    i = BNDMIN(BNDMAX(i, e[0]), e[1]);
    j = BNDMIN(BNDMAX(j, e[2]), e[3]);
    k = BNDMIN(BNDMAX(k, e[4]), e[5]);
    if (d[2] == 1)
        return                  (j-e[2]) *d[0] + (i-e[0]);
    else
        return ((k-e[4])*d[1] + (j-e[2]))*d[0] + (i-e[0]);
}

// ****************************************************************************
//  Method:  Boundary::SetExtents
//
//  Purpose:
//    Set the old extents information.
//
//  Arguments:
//    e          the extents
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
void
Boundary::SetExtents(int e[6])
{
    oldndims[0]    = e[1] - e[0] + 1;
    oldndims[1]    = e[3] - e[2] + 1;
    oldndims[2]    = e[5] - e[4] + 1;
    oldzdims[0]    = BNDMAX(e[1] - e[0], 1);
    oldzdims[1]    = BNDMAX(e[3] - e[2], 1);
    oldzdims[2]    = BNDMAX(e[5] - e[4], 1);

    oldnpts        = oldndims[0] * oldndims[1] * oldndims[2];
    oldncells      = oldzdims[0] * oldzdims[1] * oldzdims[2];

    oldnextents[0] = e[0];
    oldnextents[1] = e[1];
    oldnextents[2] = e[2];
    oldnextents[3] = e[3];
    oldnextents[4] = e[4];
    oldnextents[5] = e[5];

    oldzextents[0] = e[0];
    oldzextents[1] = BNDMAX(e[1]-1, e[0]);
    oldzextents[2] = e[2];
    oldzextents[3] = BNDMAX(e[3]-1, e[2]);
    oldzextents[4] = e[4];
    oldzextents[5] = BNDMAX(e[5]-1, e[4]);

    expand[0]      = 0;
    expand[1]      = 0;
    expand[2]      = 0;
    expand[3]      = 0;
    expand[4]      = 0;
    expand[5]      = 0;
}

// ****************************************************************************
//  Method:  Boundary::AddNeighbor
//
//  Purpose:
//    Add a neighbor to our list, keeping track of how we must expand.
//
//  Arguments:
//    d          the matching domain number
//    mi         the index of the matching neighbor information in d's list
//    o          the "orientation" i,j,k values
//    e          the extents of the neighbor in global indexing
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
void
Boundary::AddNeighbor(int d, int mi, int o[3], int e[6])
{
    Neighbor n;

    n.domain        = d;
    n.match         = mi;

    n.orient[0]  = o[0];
    n.orient[1]  = o[1];
    n.orient[2]  = o[2];

    n.ndims[0]   = e[1] - e[0] + 1;
    n.ndims[1]   = e[3] - e[2] + 1;
    n.ndims[2]   = e[5] - e[4] + 1;
    n.zdims[0]   = BNDMAX(e[1] - e[0], 1);
    n.zdims[1]   = BNDMAX(e[3] - e[2], 1);
    n.zdims[2]   = BNDMAX(e[5] - e[4], 1);
    n.npts       = n.ndims[0] * n.ndims[1] * n.ndims[2];
    n.ncells     = n.zdims[0] * n.zdims[1] * n.zdims[2];

    n.nextents[0] = e[0];
    n.nextents[1] = e[1];
    n.nextents[2] = e[2];
    n.nextents[3] = e[3];
    n.nextents[4] = e[4];
    n.nextents[5] = e[5];

    n.zextents[0] = e[0];
    n.zextents[1] = BNDMAX(e[1]-1, e[0]);
    n.zextents[2] = e[2];
    n.zextents[3] = BNDMAX(e[3]-1, e[2]);
    n.zextents[4] = e[4];
    n.zextents[5] = BNDMAX(e[5]-1, e[4]);

    n.type = NONE;
    // ---- I ----
    if (e[0] == e[1]  &&  oldnextents[0] != oldnextents[1])
    {
        int ei = e[0];
        if (ei == oldnextents[0])
        {
            n.type |= IMIN;
            expand[0] = 1;
        }
        if (ei == oldnextents[1])
        {
            n.type |= IMAX;
            expand[1] = 1;
            n.zextents[0]--;
            n.zextents[1]--;
        }
    }
    // ---- J ----
    if (e[2] == e[3]  &&  oldnextents[2] != oldnextents[3])
    {
        int ej = e[2];
        if (ej == oldnextents[2])
        {
            n.type |= JMIN;
            expand[2] = 1;
        }
        if (ej == oldnextents[3])
        {
            n.type |= JMAX;
            expand[3] = 1;
            n.zextents[2]--;
            n.zextents[3]--;
        }
    }
    // ---- K ----
    if (e[4] == e[5]  &&  oldnextents[4] != oldnextents[5])
    {
        int ek = e[4];
        if (ek == oldnextents[4])
        {
            n.type |= KMIN;
            expand[4] = 1;
        }
        if (ek == oldnextents[5])
        {
            n.type |= KMAX;
            expand[5] = 1;
            n.zextents[4]--;
            n.zextents[5]--;
        }
    }

    // Add the neighbor to the list
    neighbors.push_back(n);
}

// ****************************************************************************
//  Method:  Boundary::DeleteNeighbor
//
//  Purpose:
//    Remove the neighbor corresponding to domain d in our list.
//    Shrink ourselves if we lost too much information to reconstruct
//    the boundary in that dimension.
//
//  Arguments:
//    d          the domain number to remove
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jul  5 14:05:09 PDT 2005
//    Do not call FindNeighborIndex, since it assumes that any pair of domains
//    can have at most one boundary.
//
// ****************************************************************************
void
Boundary::DeleteNeighbor(int d, vector<Boundary> &wholelist)
{
    int   i;

    vector<int> delete_list;
    for (i=0; i<neighbors.size(); i++)
        if (neighbors[i].domain == d)
            delete_list.push_back(i);

    if (delete_list.size() == 0)
    {
        // If we have two boundaries between two domains, both those boundaries
        // will be removed when this method is called.  But because of 
        // bookkeeping issues, this method may be called twice.  So just ignore
        // rather than throwing an exception.
        //
        // EXCEPTION1(VisItException,
        //                  "No match for domain in Boundary::DeleteNeighbor");
        return;
    }

    for (i = delete_list.size()-1 ; i >= 0 ; i--)
    {
        int n = delete_list[i];
        // Stop expansion of the boundary
        if (neighbors[n].type == IMIN)
            expand[0] = 0;
        if (neighbors[n].type == IMAX)
            expand[1] = 0;
        if (neighbors[n].type == JMIN)
            expand[2] = 0;
        if (neighbors[n].type == JMAX)
            expand[3] = 0;
        if (neighbors[n].type == KMIN)
            expand[4] = 0;
        if (neighbors[n].type == KMAX)
            expand[5] = 0;
    
        // Remove the neighbor from the list
        for (int i=n+1; i<neighbors.size(); i++)
        {
            int d = neighbors[i].domain;
            int entry = neighbors[i].match;
            wholelist[d].neighbors[entry].match--;
            neighbors[i-1] = neighbors[i];
        }
        neighbors.resize(neighbors.size()-1);
    }
    
    Finish();
}

// ****************************************************************************
//  Method:  Boundary::Finish
//
//  Purpose:
//    Create the new extents information from the old extents and the neighbors
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
void
Boundary::Finish()
{
    newndims[0]    = oldndims[0] + expand[0] + expand[1];
    newndims[1]    = oldndims[1] + expand[2] + expand[3];
    newndims[2]    = oldndims[2] + expand[4] + expand[5];

    newzdims[0]    = oldzdims[0] + expand[0] + expand[1];
    newzdims[1]    = oldzdims[1] + expand[2] + expand[3];
    newzdims[2]    = oldzdims[2] + expand[4] + expand[5];

    newnpts        = newndims[0] * newndims[1] * newndims[2];
    newncells      = newzdims[0] * newzdims[1] * newzdims[2];

    newnextents[0] = oldnextents[0] - expand[0];
    newnextents[1] = oldnextents[1] + expand[1];
    newnextents[2] = oldnextents[2] - expand[2];
    newnextents[3] = oldnextents[3] + expand[3];
    newnextents[4] = oldnextents[4] - expand[4];
    newnextents[5] = oldnextents[5] + expand[5];

    newzextents[0] = oldzextents[0] - expand[0];
    newzextents[1] = oldzextents[1] + expand[1];
    newzextents[2] = oldzextents[2] - expand[2];
    newzextents[3] = oldzextents[3] + expand[3];
    newzextents[4] = oldzextents[4] - expand[4];
    newzextents[5] = oldzextents[5] + expand[5];
}


// ****************************************************************************
//  Method:  Boundary::IsGhostZone
//
//  Purpose:
//    Return true if the given index is outside the old zonal extents.
//
//  Arguments:
//    i,j,k      index in global indexing
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
bool
Boundary::IsGhostZone(int i, int j, int k)
{
    return (i < oldzextents[0] ||
            i > oldzextents[1] ||
            j < oldzextents[2] ||
            j > oldzextents[3] ||
            k < oldzextents[4] ||
            k > oldzextents[5]);
}

// ****************************************************************************
//  Method:  Boundary::TranslatedPointIndex
//
//  Purpose:
//    Return the old point index, translated from one neighbor's domain to
//    the other's.
//
//  Arguments:
//    n1         the "from" domain
//    n2         the "to" domain
//    i,j,k      the index in global indexing
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
int 
Boundary::TranslatedPointIndex(Neighbor *n1,
                                                              Neighbor *n2,
                                                              int i,int j,int k)
{
    i -= n1->nextents[0];
    j -= n1->nextents[2];
    k -= n1->nextents[4];

    int lookup[7] = {n1->ndims[2]-k-1,
                     n1->ndims[1]-j-1,
                     n1->ndims[0]-i-1,
                     0,//error
                     i,
                     j,
                     k};
                            

    if      (n2->type & IMIN)
        i = n2->nextents[0] + 1;
    else if (n2->type & IMAX)
        i = n2->nextents[1] - 1;
    else
        i = lookup[n1->orient[0]+3] + n2->nextents[0];

    if      (n2->type & JMIN)
        j = n2->nextents[2] + 1;
    else if (n2->type & JMAX)
        j = n2->nextents[3] - 1;
    else
        j = lookup[n1->orient[1]+3] + n2->nextents[2];

    if      (n2->type & KMIN)
        k = n2->nextents[4] + 1;
    else if (n2->type & KMAX)
        k = n2->nextents[5] - 1;
    else
        k = lookup[n1->orient[2]+3] + n2->nextents[4];

    return OldPointIndex(i, j, k);
}

// ****************************************************************************
//  Method:  Boundary::TranslatedCellIndex
//
//  Purpose:
//    Return the old cell index, translated from one neighbor's domain to
//    the other's.
//
//  Arguments:
//    n1         the "from" domain
//    n2         the "to" domain
//    i,j,k      the index in global indexing
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Dec  4 12:39:39 PST 2001
//    Fixed a bug.  I forgot ghost cells behaved quite differently from
//    ghost nodes.
//
// ****************************************************************************
int
Boundary::TranslatedCellIndex(Neighbor *n1,
                                                             Neighbor *n2,
                                                             int i,int j,int k)
{
    i -= n1->zextents[0];
    j -= n1->zextents[2];
    k -= n1->zextents[4];

    int lookup[7] = {n1->zdims[2]-k-1,
                     n1->zdims[1]-j-1,
                     n1->zdims[0]-i-1,
                     0,//error
                     i,
                     j,
                     k};
                            

    if      (n2->type & IMIN)
        i = n2->zextents[0];
    else if (n2->type & IMAX)
        i = n2->zextents[1];
    else
        i = lookup[n1->orient[0]+3] + n2->zextents[0];

    if      (n2->type & JMIN)
        j = n2->zextents[2];
    else if (n2->type & JMAX)
        j = n2->zextents[3];
    else
        j = lookup[n1->orient[1]+3] + n2->zextents[2];

    if      (n2->type & KMIN)
        k = n2->zextents[4];
    else if (n2->type & KMAX)
        k = n2->zextents[5];
    else
        k = lookup[n1->orient[2]+3] + n2->zextents[4];

    return OldCellIndex(i, j, k);
}

// ****************************************************************************
//  Method:  Boundary::OldPointIndex
//
//  Purpose:
//    Return the old linear index of the given point given an index in 
//    global indexing space.
//
//  Arguments:
//    i,j,k      the index in global indexing
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
int
Boundary::OldPointIndex(int i, int j, int k)
{
    return CalculateIndex(oldnextents, oldndims, i, j, k);
}

// ****************************************************************************
//  Method:  Boundary::OldCellIndex
//
//  Purpose:
//    Return the old linear index of the given cell given an index in 
//    global indexing space.
//
//  Arguments:
//    i,j,k      the index in global indexing
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
int
Boundary::OldCellIndex(int i, int j, int k)
{
    return CalculateIndex(oldzextents, oldzdims, i, j, k);
}

// ****************************************************************************
//  Method:  Boundary::NewPointIndex
//
//  Purpose:
//    Return the new linear index of the given point given an index in 
//    global indexing space.
//
//  Arguments:
//    i,j,k      the index in global indexing
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
int
Boundary::NewPointIndex(int i, int j, int k)
{
    return CalculateIndex(newnextents, newndims, i, j, k);
}

// ****************************************************************************
//  Method:  Boundary::NewCellIndex
//
//  Purpose:
//    Return the new linear index of the given cell given an index in 
//    global indexing space.
//
//  Arguments:
//    i,j,k      the index in global indexing
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
int
Boundary::NewCellIndex(int i, int j, int k)
{
    return CalculateIndex(newzextents, newzdims, i, j, k);
}

// ****************************************************************************
//  Method:  Boundary::ClosestExistingNewPointIndex
//
//  Purpose:
//    Return the new linear index of the closest existing point to the given
//    point, given an index in global indexing space.
//
//  Arguments:
//    exists     the existence array
//    i,j,k      the index in global indexing
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
int
Boundary::ClosestExistingNewPointIndex(bool *exists, int i, int j, int k)
{
    int index = NewPointIndex(i,j,k);
    while (!exists[index])
    {
        if      (i < oldnextents[0])  i++;
        else if (i > oldnextents[1])  i--;
        else if (j < oldnextents[2])  j++;
        else if (j > oldnextents[3])  j--;
        else if (k < oldnextents[4])  k++;
        else if (k > oldnextents[5])  k--;
        index = NewPointIndex(i,j,k);
    }

    return index;
}

// ****************************************************************************
//  Method:  Boundary::ClosestExistingNewCellIndex
//
//  Purpose:
//    Return the new linear index of the closest existing cell to the given
//    cell, given an index in global indexing space.
//
//  Arguments:
//    exists     the existence array
//    i,j,k      the index in global indexing
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
int
Boundary::ClosestExistingNewCellIndex(bool *exists, int i, int j, int k)
{
    int index = NewCellIndex(i,j,k);
    while (!exists[index])
    {
        if      (i < oldzextents[0])  i++;
        else if (i > oldzextents[1])  i--;
        else if (j < oldzextents[2])  j++;
        else if (j > oldzextents[3])  j--;
        else if (k < oldzextents[4])  k++;
        else if (k > oldzextents[5])  k--;
        index = NewCellIndex(i,j,k);
    }

    return index;
}

// ****************************************************************************
//  Method:  Boundary::NewPointIndexFromNeighbor
//
//  Purpose:
//    Return the new linear index of the given point given an index in 
//    global indexing space, also given that this is from a neighbor
//    index and must be shifted to be a ghost value.
//
//  Arguments:
//    i,j,k      the index in global indexing
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
int
Boundary::NewPointIndexFromNeighbor(Neighbor *n, int i, int j, int k)
{
    if      (n->type & IMIN)  i--;
    else if (n->type & IMAX)  i++;
    if      (n->type & JMIN)  j--;
    else if (n->type & JMAX)  j++;
    if      (n->type & KMIN)  k--;
    else if (n->type & KMAX)  k++;

    if (i>=newnextents[0] && i<=newnextents[1] &&
        j>=newnextents[2] && j<=newnextents[3] &&
        k>=newnextents[4] && k<=newnextents[5])
    {
        return NewPointIndex(i,j,k);
    }

    return -1;
}

// ****************************************************************************
//  Method:  Boundary::NewCellIndexFromNeighbor
//
//  Purpose:
//    Return the new linear index of the given cell given an index in 
//    global indexing space, also given that this is from a neighbor
//    index and must be shifted to be a ghost value.
//
//  Arguments:
//    i,j,k      the index in global indexing
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
int
Boundary::NewCellIndexFromNeighbor(Neighbor *n, int i, int j, int k)
{
    if      (n->type & IMIN)  i--;
    else if (n->type & IMAX)  i++;
    if      (n->type & JMIN)  j--;
    else if (n->type & JMAX)  j++;
    if      (n->type & KMIN)  k--;
    else if (n->type & KMAX)  k++;

    if (i>=newzextents[0] && i<=newzextents[1] &&
        j>=newzextents[2] && j<=newzextents[3] &&
        k>=newzextents[4] && k<=newzextents[5])
    {
        return NewCellIndex(i,j,k);
    }

    return -1;
}

// ****************************************************************************
//  Method:  operator<<(Boundary&)
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2001
//
// ****************************************************************************
ostream&
operator<<(ostream &o, Boundary &b)
{
    if (b.domain == -1)
        return o;

    o << "---> domain "<<b.domain<<":"<<endl;
    o << "             : oldnextents = "
      << b.oldnextents[0] << ","
      << b.oldnextents[1] << "   "
      << b.oldnextents[2] << ","
      << b.oldnextents[3] << "   "
      << b.oldnextents[4] << ","
      << b.oldnextents[5] << "\t\t"
      << "oldzextents = "
      << b.oldzextents[0] << ","
      << b.oldzextents[1] << "   "
      << b.oldzextents[2] << ","
      << b.oldzextents[3] << "   "
      << b.oldzextents[4] << ","
      << b.oldzextents[5] << "\n";
    o << "             : newnextents = "
      << b.newnextents[0] << ","
      << b.newnextents[1] << "   "
      << b.newnextents[2] << ","
      << b.newnextents[3] << "   "
      << b.newnextents[4] << ","
      << b.newnextents[5] << "\t\t"
      << "newzextents = "
      << b.newzextents[0] << ","
      << b.newzextents[1] << "   "
      << b.newzextents[2] << ","
      << b.newzextents[3] << "   "
      << b.newzextents[4] << ","
      << b.newzextents[5] << "\n";
    o << "             : numneighbors = " 
      << b.neighbors.size() << endl;
    o << "             : expand = "
      << (b.expand[0] ? "-i" : "") << " "
      << (b.expand[1] ? "+i" : "") << " "
      << (b.expand[2] ? "-j" : "") << " "
      << (b.expand[3] ? "+j" : "") << " "
      << (b.expand[4] ? "-k" : "") << " "
      << (b.expand[5] ? "+k" : "") << "\n";
    for (int j=0; j<b.neighbors.size(); j++)
    {
        o << "                           >>>"
          << "\t d=" << b.neighbors[j].domain
          << "\t ncells="<<b.neighbors[j].ncells
          << "\t npts="<<b.neighbors[j].npts
          << "\t orient = " 
          << b.neighbors[j].orient[0] << ","
          << b.neighbors[j].orient[1] << ","
          << b.neighbors[j].orient[2] << "   "
          << "\t nextents = "
          << b.neighbors[j].nextents[0] << ","
          << b.neighbors[j].nextents[1] << "   "
          << b.neighbors[j].nextents[2] << ","
          << b.neighbors[j].nextents[3] << "   "
          << b.neighbors[j].nextents[4] << ","
          << b.neighbors[j].nextents[5] << "   "
          << "\t zextents = "
          << b.neighbors[j].zextents[0] << ","
          << b.neighbors[j].zextents[1] << "   "
          << b.neighbors[j].zextents[2] << ","
          << b.neighbors[j].zextents[3] << "   "
          << b.neighbors[j].zextents[4] << ","
          << b.neighbors[j].zextents[5] << "   "
          << "\t type = " 
          << ((b.neighbors[j].type&Boundary::IMIN) ? "-i " : "")
          << ((b.neighbors[j].type&Boundary::IMAX) ? "+i " : "")
          << ((b.neighbors[j].type&Boundary::JMIN) ? "-j " : "")
          << ((b.neighbors[j].type&Boundary::JMAX) ? "+j " : "")
          << ((b.neighbors[j].type&Boundary::KMIN) ? "-k " : "")
          << ((b.neighbors[j].type&Boundary::KMAX) ? "+k " : "") << "\n";
    }
    return o;
}
