// ************************************************************************* //
//                     avtActualDataMinMaxQuery.h                            //
// ************************************************************************* //

#ifndef AVT_ACTUALDATA_MINMAX_QUERY_H
#define AVT_ACTUALDATA_MINMAX_QUERY_H
#include <query_exports.h>

#include <avtMinMaxQuery.h>

class avtCondenseDatasetFilter;


// ****************************************************************************
//  Class: avtActualDataMinMaxQuery
//
//  Purpose:
//    A query that retrieves var information about a mesh given a 
//    particular domain and zone number.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 23, 2003
//
//  Modifications:
//    Kathleen Bonnell, Tue Feb  3 17:54:19 PST 2004
//    Renamed from avtPlotMinMaxQuery. Moved definition of GetNFilters to
//    source file.
//
// ****************************************************************************

class QUERY_API avtActualDataMinMaxQuery : public avtMinMaxQuery
{
  public:
                              avtActualDataMinMaxQuery();
    virtual                  ~avtActualDataMinMaxQuery();

    virtual bool              OriginalData(void) { return false; };

  protected:

    virtual avtDataObject_p   ApplyFilters(avtDataObject_p);   
    virtual int               GetNFilters() { return 1; };

  private:

    avtCondenseDatasetFilter *condense;
};

#endif
