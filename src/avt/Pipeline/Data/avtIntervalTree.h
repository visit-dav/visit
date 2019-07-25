// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtIntervalTree.h                              //
// ************************************************************************* //

#ifndef AVT_INTERVAL_TREE_H
#define AVT_INTERVAL_TREE_H

#include <pipeline_exports.h>

#include <vectortypes.h>


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
//    Hank Childs, Fri Jul 28 08:09:23 PDT 2006
//    Add axially-symmetric line intersection test.
//
//    Kathleen Bonnell, Mon Aug 21 13:34:18 PDT 2006 
//    Add default bool arg to constructor -- specifies whether or not this tree
//    will required collective communication. 
//
//    Hank Childs, Thu Mar  1 16:57:34 PST 2007
//    Add a method to get the dimension (for error checking).
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Hank Childs, Thu Jun 12 11:32:45 PDT 2008
//    Implement a copy constructor.
//
//    Kathleen Bonnell, Thu Jun 11 08:24:04 PDT 2009
//    Added optional tolerance argument to GetElementListsFromLine.
//
//    Hank Childs, Mon Sep 13 18:52:30 PDT 2010
//    Added options for accelerating size queries and whether or not to 
//    optimize for repeated queries.
//
// ****************************************************************************

class PIPELINE_API avtIntervalTree
{
  public:
                              avtIntervalTree(int, int, bool = true);
                              avtIntervalTree(const avtIntervalTree *);
    virtual                  ~avtIntervalTree();

    static void               Destruct(void *);

    void                      GetExtents(double *) const;
    int                       GetDimension(void) const { return nDims; };
    void                      GetElementsList(const double *, double,
                                             std::vector<int> &) const;
    void                      GetElementsList(double [3], double[3],
                                             std::vector<int> &) const;
    void                      GetElementsListFromRange(const double *,
                                                      const double *,
                                                      std::vector<int>&) const;
    int                       GetNumberOfElementsInRange(const double *,
                                                         const double *) const;
    void                      GetElementsFromAxiallySymmetricLineIntersection(
                                       const double *, const double *,
                                       std::vector<int>&) const;
    void                      GetElementsListFromRay(double [3], double[3],
                                          intVector &, doubleVector &) const; 
    void                      GetElementsListFromLine(double [3], double[3],
                                          intVector &, doubleVector &, 
                                          const double* = NULL) const; 

    void                      AccelerateSizeQueries(void);
    void                      OptimizeForRepeatedQueries(void) 
                                         { optimizeForRepeatedQueries = true; };

    void                      AddElement(int, const double *);
    void                      Calculate(bool = false);

    int                       GetNLeaves(void) const { return nElements; };
    int                       GetLeafExtents(int, double *) const;
    void                      GetElementExtents(int, double *) const;

  protected:
    int                       nElements;
    int                       nNodes;
    int                       nDims;
    int                       vectorSize;

    double                   *nodeExtents;
    int                      *nodeIDs;

    bool                      optimizeForRepeatedQueries;
    bool                      accelerateSizeQueries;
    int                      *numElementsBeneathThisNode;

    bool                      hasBeenCalculated;
    bool                      requiresCommunication;

    void                      CollectInformation(void);
    void                      ConstructTree(void);
    void                      SetIntervals(void);
    int                       SplitSize(int);

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtIntervalTree(const avtIntervalTree &) {;};
    avtIntervalTree     &operator=(const avtIntervalTree &) { return *this; };
};


#endif

  
