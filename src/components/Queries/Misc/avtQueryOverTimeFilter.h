// ************************************************************************* //
//                       avtQueryOverTimeFilter.h                            //
// ************************************************************************* //

#ifndef AVT_QUERYOVERTIME_FILTER_H
#define AVT_QUERYOVERTIME_FILTER_H

#include <query_exports.h>

#include <avtDatasetToDatasetFilter.h>

#include <QueryOverTimeAttributes.h>
#include <SILRestrictionAttributes.h>


// ****************************************************************************
//  Class: avtQueryOverTimeFilter
//
//  Purpose:
//    Performs a query over time. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 19, 2004
//
//  Modifications:
//    Brad Whitlock, Wed Apr 14 14:56:45 PST 2004
//    Fixed for Windows.
//
//    Kathleen Bonnell, Tue May  4 14:21:37 PDT 2004
//    Removed SilUseSet in favor of SILRestrictionAttributes. 
//
// ****************************************************************************

class QUERY_API avtQueryOverTimeFilter : public avtDatasetToDatasetFilter
{
  public:
                          avtQueryOverTimeFilter(const AttributeGroup*);
    virtual              ~avtQueryOverTimeFilter();

    static avtFilter     *Create(const AttributeGroup*);

    virtual const char   *GetType(void)  { return "avtQueryOverTimeFilter"; };
    virtual const char   *GetDescription(void) { return "Querying over Time"; };

    void                  SetSILAtts(const SILRestrictionAttributes *silAtts);



  protected:
    QueryOverTimeAttributes   atts;
    SILRestrictionAttributes  querySILAtts;

    virtual void          Execute(void);
    virtual void          PostExecute(void);
    virtual void          RefashionDataObjectInfo(void);

    virtual int           AdditionalPipelineFilters(void) { return 1; };

};


#endif


