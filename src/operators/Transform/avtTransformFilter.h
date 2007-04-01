// ************************************************************************* //
//  File: avtTransformFilter.h
// ************************************************************************* //

#ifndef AVT_Transform_FILTER_H
#define AVT_Transform_FILTER_H


#include <avtPluginFilter.h>
#include <avtSingleFilterFacade.h>
#include <TransformAttributes.h>

class avtSimilarityTransformFilter;
class avtCoordSystemConvert;


// ****************************************************************************
//  Class: avtTransformFilter
//
//  Purpose:
//      A plugin operator for Transform.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 24, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Nov 28 16:59:53 PST 2001
//    Added RefashionDataObjectInfo, PerformRestriction.
//
//    Hank Childs, Tue Mar  5 16:05:25 PST 2002
//    Removed RefashionDataObjectInfo, since what is was doing is also
//    applicable to the base class, so I pushed the logic into it.
//
//    Kathleen Bonnell, Thu Apr 10 11:07:48 PDT 2003 
//    Store inverse matrix for possible use later in the pipeline. 
//    Added PostExecute method.
//    
//    Hank Childs, Tue Jul  1 08:59:08 PDT 2003
//    Moved original avtTransformFilter to libpipeline with the name
//    avtSimilarityTransformFilter.  Re-worked this filter to be a single
//    filter facade.
//
//    Hank Childs, Tue Feb  1 16:37:56 PST 2005
//    Allow for coordinate system transformations as well.
//
// ****************************************************************************

class avtTransformFilter : public virtual avtPluginFilter,
                           public virtual avtSingleFilterFacade
{
  public:
                         avtTransformFilter();
    virtual             ~avtTransformFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtTransformFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Transforming"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    TransformAttributes            atts;
    avtSimilarityTransformFilter  *stf;
    avtCoordSystemConvert         *csc;

    virtual avtFilter             *GetFacadedFilter(void);
};


#endif
