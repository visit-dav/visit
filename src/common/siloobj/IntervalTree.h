// ************************************************************************* //
//                             IntervalTree.h                                //
// ************************************************************************* //

#ifndef INTERVAL_TREE_H
#define INTERVAL_TREE_H
#include <siloobj_exports.h>

#include <iostream.h>
#include <silo.h>


// ****************************************************************************
//  Class: IntervalTree
// 
//  Purpose:
//      Keeps an interval tree.
//
//  Data Members:
//      nNodes         The number of nodes in the tree.
//      nDomains       The number of domains in the problem.  This is also the
//                     number of leaf nodes in the tree.
//      nDims          The dimension of the values of the variable. 
//      nodeExtents    The minimum and maximum bounds for all node that 
//                     are covered by this node.  An array of size nNodes by
//                     nDims*2.  Note the nodeExtents is used to store the 
//                     domain values until the tree is constructed.  The
//                     array is kept as 1-dimensional to prevent excessive 
//                     'new's and to allow for MPI calls.
//      nodeIDs        A list that will convert a node number for the interval
//                     tree back to its original domain number.  An array of 
//                     size nNodes.
//      vectorSize     The size of the vector associated with one domain.  This
//                     should be 2*nDims.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 1999
//
//  Modifications:
//
//    Hank Childs, Fri Aug 11 09:20:22 PDT 2000
//    Grant access to avtIntervalTree for its conversion constructor.
//
// ****************************************************************************

class SILOOBJ_API IntervalTree
{
    friend class   avtIntervalTree;

  public:
                   IntervalTree();
    virtual       ~IntervalTree();

    void           GetExtents(float *);
    char          *GetName() { return name; };
    void           PrintSelf(ostream &);

    void           Read(DBobject *, DBfile *);
    virtual void   Write(DBfile *);
      
  protected:
    int            nNodes;
    int            nDomains;
    int            nDims;
    float         *nodeExtents;
    int           *nodeIDs;

    char          *name;

  public:
    static char * const    NAME;
    static char * const    SILO_TYPE;
  protected:
    static int const       N_DIMS_LIMIT;
    static int const       SILO_NUM_COMPONENTS;
    static char * const    SILO_OBJ_NAME;

    static char * const    SILO_N_NODES_NAME;
    static char * const    SILO_NODE_EXTENTS_NAME;
    static char * const    SILO_NODE_IDS_NAME;
    static char * const    SILO_N_DIMS_NAME;
};


#endif


