// ************************************************************************* //
//                      avtActualDataNumNodesQuery.h                         //
// ************************************************************************* //

#ifndef AVT_ACTUALDATA_NUMNODES_QUERY_H
#define AVT_ACTUALDATA_NUMNODES_QUERY_H
#include <query_exports.h>

#include <avtNumNodesQuery.h>


// ****************************************************************************
//  Class: avtActualDataNumNodesQuery
//
//  Purpose:
//      This is a dataset query that returns the number of nodes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtActualDataNumNodesQuery : public avtNumNodesQuery
{
  public:
                              avtActualDataNumNodesQuery();
    virtual                  ~avtActualDataNumNodesQuery(); 

    virtual bool             OriginalData(void) { return false; };
};

#endif
