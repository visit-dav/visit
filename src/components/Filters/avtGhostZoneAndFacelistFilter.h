// ************************************************************************* //
//                        avtGhostZoneAndFacelistFilter.h                    //
// ************************************************************************* //

#ifndef AVT_GHOSTZONE_AND_FACELIST_FILTER_H
#define AVT_GHOSTZONE_AND_FACELIST_FILTER_H

#include <filters_exports.h>

#include <avtDatasetToDatasetFilter.h>

class   avtGhostZoneFilter;
class   avtFacelistFilter;


// ****************************************************************************
//  Class: avtGhostZoneAndFacelistFilter
//
//  Purpose:
//    Applies the ghostZone and facelist filters, after
//    determining which should come first. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 20, 2001 
//
//  Modifications:
//
//    Hank Childs, Fri Jul 27 14:54:13 PDT 2001
//    Added RefashionDataObjectInfo.
//
//    Hank Childs, Thu Sep  6 11:14:38 PDT 2001
//    Allowed for dynamic load balancing.
//
//    Hank Childs, Wed Aug 11 09:46:53 PDT 2004
//    Added PerformRestriction.
//
//    Hank Childs, Fri Sep 23 10:38:31 PDT 2005
//    Add support for edge lists.
//
// ****************************************************************************

class AVTFILTERS_API avtGhostZoneAndFacelistFilter :
                                               public avtDatasetToDatasetFilter
{
  public:
                          avtGhostZoneAndFacelistFilter();
    virtual              ~avtGhostZoneAndFacelistFilter(); 

    virtual const char   *GetType(void)
                                   { return "avtGhostZoneAndFacelistFilter"; };
    virtual const char   *GetDescription(void)
                                   { return "Finding visible triangles"; };

    void                  SetUseFaceFilter(bool val) { useFaceFilter = val; };
    void                  SetCreate3DCellNumbers(bool val);
    void                  SetForceFaceConsolidation(bool);
    void                  SetCreateEdgeListFor2DDatasets(bool val);

    virtual void          ReleaseData(void);

  protected:
    avtGhostZoneFilter   *ghostFilter;
    avtFacelistFilter    *faceFilter;
    bool                  useFaceFilter;
    bool                  useGhostFilter;

    virtual void          Execute(void);
    virtual void          RefashionDataObjectInfo(void);
    virtual int           AdditionalPipelineFilters(void);
    virtual void          ChangedInput(void);
    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);
};


#endif


