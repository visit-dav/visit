// ************************************************************************* //
//                            avtIntervalTree.h                              //
// ************************************************************************* //

#ifndef AVT_INTERVAL_TREE_H
#define AVT_INTERVAL_TREE_H

#include <pipeline_exports.h>

#include <vector>


// ****************************************************************************
//  Class: avtIntervalTree
//
//  Purpose:
//      Keeps meta-data for fields.  The meta-data is stored as a tree and each
//      node covers the range of its children.  Each leaf node contains a range
//      that corresponds to the extents of a specific domain.  The range is of
//      spacial or data extents.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Aug 11 09:14:02 PDT 2000
//    Added conversion constructor from prototype's tree, IntervalTree.
//
//    Hank Childs, Tue Nov 19 12:26:34 PST 2002
//    Removed all knowledge of previous incarnation of 'IntervalTree'.
//
//    Mark C. Miller, Mon Oct 18 14:36:49 PDT 2004
//    Added hasBeenCalculated variable
//
//    Hank Childs, Mon Jun 27 09:01:01 PDT 2005
//    Remove unused methods for sorting.
//
// ****************************************************************************

class PIPELINE_API avtIntervalTree
{
  public:
                              avtIntervalTree(int, int);
                              //avtIntervalTree(const IntervalTree *);
    virtual                  ~avtIntervalTree();

    static void               Destruct(void *);

    void                      GetExtents(float *) const;
    void                      GetDomainsList(const float *, float,
                                             std::vector<int> &) const;
    void                      GetDomainsList(float [3], float[3],
                                             std::vector<int> &) const;
    void                      GetDomainsListFromRange(const float *,
                                                      const float *,
                                                      std::vector<int>&) const;

    void                      AddDomain(int, float *);
    void                      Calculate(bool = false);

    int                       GetNLeaves(void) const { return nDomains; };
    int                       GetLeafExtents(int, float *) const;
    void                      GetDomainExtents(int, float *) const;

  protected:
    int                       nDomains;
    int                       nNodes;
    int                       nDims;
    int                       vectorSize;

    float                    *nodeExtents;
    int                      *nodeIDs;

    bool                      hasBeenCalculated;

    void                      CollectInformation(void);
    void                      ConstructTree(void);
    void                      SetIntervals(void);
    int                       SplitSize(int);
};


#endif

  
