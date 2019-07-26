// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtLocalStructuredDomainBoundaries.h                   //
// ************************************************************************* //

#ifndef AVT_LOCAL_STRUCTURED_DOMAIN_BOUNDARIES_H
#define AVT_LOCAL_STRUCTURED_DOMAIN_BOUNDARIES_H

#include <avtStructuredDomainBoundaries.h>
#include <visitstream.h>
#include <vector>

class DATABASE_API avtLocalStructuredDomainBoundaryList
{
public:
              avtLocalStructuredDomainBoundaryList(int domain_id,
                                                   int extents[6]);
              avtLocalStructuredDomainBoundaryList(int *ptr);
              avtLocalStructuredDomainBoundaryList(avtLocalStructuredDomainBoundaryList *lst);
    virtual  ~avtLocalStructuredDomainBoundaryList();

    static void Destruct(void *);

    void        AddNeighbor(int neighbor_id,
                           int match,
                           int orientation[3],
                           int extents[6]);

    int         NumberOfNeighbors() const {return (int)neighbors.size();}
    static avtStructuredDomainBoundaries  *GlobalGenerate(const std::vector<avtLocalStructuredDomainBoundaryList*> &lists);

    void        Print(ostream &os) const;

private:
    int domainId;
    int extents[6];

    int    Pack(int *ptr) const;

    int    PackedSize() const { return 8 + 11 * (int)neighbors.size();}

    static void GlobalGather(const std::vector<avtLocalStructuredDomainBoundaryList*> &local_lists,
                             std::vector<avtLocalStructuredDomainBoundaryList*> &out);

    class DomainNeighbor
    {
       public:
                 DomainNeighbor(int neighbor_id,
                                int match,
                                int orientation[3],
                                int extents[6]);
                 DomainNeighbor(int *ptr);
                 DomainNeighbor(DomainNeighbor *nei);
        virtual ~DomainNeighbor();

        int      PackedSize() const { return 11;}
        int      Pack(int *ptr) const;
        void     Print(ostream &os) const;
        int neighborId;
        int match;
        int orientation[3];
        int extents[6];
    };

    std::vector<DomainNeighbor *> neighbors;
};

#endif


