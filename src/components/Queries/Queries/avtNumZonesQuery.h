// ************************************************************************* //
//                            avtNumZonesQuery.h                             //
// ************************************************************************* //

#ifndef AVT_NUMZONES_QUERY_H
#define AVT_NUMZONES_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>


// ****************************************************************************
//  Class: avtNumZonesQuery
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

class QUERY_API avtNumZonesQuery : public avtDatasetQuery
{
  public:
                              avtNumZonesQuery();
    virtual                  ~avtNumZonesQuery(); 

    virtual const char        *GetType(void) { return "avtNumZonesQuery"; };
    virtual const char        *GetDescription(void) 
                                  { return "Getting the number of zones"; };

    virtual void             PerformQuery(QueryAttributes *);

  protected:

    virtual void             Execute(vtkDataSet*, const int){;};
};

#endif

