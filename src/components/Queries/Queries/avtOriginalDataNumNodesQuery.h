// ************************************************************************* //
//                      avtOriginalDataNumNodesQuery.h                       //
// ************************************************************************* //

#ifndef AVT_ORIGINALDATA_NUMNODES_QUERY_H
#define AVT_ORIGINALDATA_NUMNODES_QUERY_H
#include <query_exports.h>

#include <avtNumNodesQuery.h>


// ****************************************************************************
//  Class: avtOriginalDataNumNodesQuery
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

class QUERY_API avtOriginalDataNumNodesQuery : public avtNumNodesQuery
{
  public:
                              avtOriginalDataNumNodesQuery();
    virtual                  ~avtOriginalDataNumNodesQuery(); 

    virtual bool             OriginalData(void) { return true; };

  private:
    virtual avtDataObject_p  ApplyFilters(avtDataObject_p);
};

#endif


