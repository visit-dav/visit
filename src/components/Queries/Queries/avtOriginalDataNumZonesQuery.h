// ************************************************************************* //
//                      avtOriginalDataNumZonesQuery.h                       //
// ************************************************************************* //

#ifndef AVT_ORIGINALDATA_NUMZONES_QUERY_H
#define AVT_ORIGINALDATA_NUMZONES_QUERY_H
#include <query_exports.h>

#include <avtNumZonesQuery.h>


// ****************************************************************************
//  Class: avtOriginalDataNumZonesQuery
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

class QUERY_API avtOriginalDataNumZonesQuery : public avtNumZonesQuery
{
  public:
                              avtOriginalDataNumZonesQuery();
    virtual                  ~avtOriginalDataNumZonesQuery(); 

    virtual bool             OriginalData(void) { return true; };

  protected:
    virtual avtDataObject_p  ApplyFilters(avtDataObject_p);
};

#endif

