// ************************************************************************* //
//                      avtActualDataNumZonesQuery.h                         //
// ************************************************************************* //

#ifndef AVT_ACTUALDATA_NUMZONES_QUERY_H
#define AVT_ACTUALDATA_NUMZONES_QUERY_H
#include <query_exports.h>

#include <avtNumZonesQuery.h>


// ****************************************************************************
//  Class: avtActualDataNumZonesQuery
//
//  Purpose:
//      This is a dataset query that returns the number of zones.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtActualDataNumZonesQuery : public avtNumZonesQuery
{
  public:
                              avtActualDataNumZonesQuery();
    virtual                  ~avtActualDataNumZonesQuery(); 

    virtual bool             OriginalData(void) { return false; };
};

#endif

