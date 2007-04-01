// ************************************************************************* //
//                     avtOriginalDataMinMaxQuery.h                          //
// ************************************************************************* //

#ifndef AVT_ORIGINAL_MINMAX_QUERY_H
#define AVT_ORIGINAL_MINMAX_QUERY_H
#include <query_exports.h>

#include <avtMinMaxQuery.h>

class avtCondenseDatasetFilter;
class avtExpressionEvaluatorFilter;


// ****************************************************************************
//  Class: avtOriginalDataMinMaxQuery
//
//  Purpose:
//    A query that retrieves the min/max of a var from the original data. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 10, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 16:07:50 PST 2004
//    Added optional constructor args.
//
//    Kathleen Bonnell, Wed Apr 14 18:05:08 PDT 2004 
//    Added condense filter. 
//
// ****************************************************************************

class QUERY_API avtOriginalDataMinMaxQuery : public avtMinMaxQuery
{
  public:
                                  avtOriginalDataMinMaxQuery(
                                      bool m = true, bool x = true);
    virtual                      ~avtOriginalDataMinMaxQuery();

    virtual bool                  OriginalData(void) { return true; };

  protected:
    virtual avtDataObject_p       ApplyFilters(avtDataObject_p);   
    virtual int                   GetNFilters() { return 1; };

  private:
    avtExpressionEvaluatorFilter *eef;
    avtCondenseDatasetFilter     *condense;
};

#endif
