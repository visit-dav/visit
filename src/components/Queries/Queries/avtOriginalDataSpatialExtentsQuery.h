// ************************************************************************* //
//                   avtOriginalDataSpatialExtentsQuery.h                    //
// ************************************************************************* //

#ifndef AVT_ORIGINALDATA_SPATIALEXTENTS_QUERY_H
#define AVT_ORIGINALDATA_SPATIALEXTENTS_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>

// ****************************************************************************
//  Class: avtOriginalDataSpatialExtentsQuery
//
//  Purpose:
//    This is a dataset query that returns the spatial extents.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 31, 2006 
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtOriginalDataSpatialExtentsQuery : public avtDatasetQuery
{
  public:
                              avtOriginalDataSpatialExtentsQuery();
    virtual                  ~avtOriginalDataSpatialExtentsQuery(); 

    virtual const char       *GetType(void) 
                               { return "avtOriginalDataSpatialExtentsQuery"; };
    virtual const char       *GetDescription(void) 
                               { return "Getting the spatial extents"; };


    virtual void             PerformQuery(QueryAttributes *);

    virtual bool             OriginalData(void) { return true; };

  protected:
    virtual avtDataObject_p  ApplyFilters(avtDataObject_p);
    virtual void             Execute(vtkDataSet*, const int) {;};
};

#endif

