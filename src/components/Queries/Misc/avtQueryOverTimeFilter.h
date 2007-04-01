// ************************************************************************* //
//                       avtQueryOverTimeFilter.h                            //
// ************************************************************************* //

#ifndef AVT_QUERYOVERTIME_FILTER_H
#define AVT_QUERYOVERTIME_FILTER_H

#include <filters_exports.h>

#include <avtDatasetToDatasetFilter.h>

#include <QueryOverTimeAttributes.h>


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
//
// ****************************************************************************

class AVTFILTERS_API avtQueryOverTimeFilter : public avtDatasetToDatasetFilter
{
  public:
                          avtQueryOverTimeFilter(const AttributeGroup*);
    virtual              ~avtQueryOverTimeFilter();

    static avtFilter     *Create(const AttributeGroup*);

    virtual const char   *GetType(void)  { return "avtQueryOverTimeFilter"; };
    virtual const char   *GetDescription(void) { return "Querying over Time"; };

    void                  SetSILUseSet(const unsignedCharVector &u)
                                        { silUseSet = u; };



  protected:
    QueryOverTimeAttributes   atts;
    unsignedCharVector    silUseSet;

    virtual void          Execute(void);
    virtual void          PostExecute(void);
    virtual void          RefashionDataObjectInfo(void);

    virtual int           AdditionalPipelineFilters(void) { return 1; };

#if 0
    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);
#endif
};


#endif


