// ************************************************************************* //
//                               avtFilter.h                                 //
// ************************************************************************* //

#ifndef AVT_FILTER_H
#define AVT_FILTER_H
#include <pipeline_exports.h>


#include <vector>

#include <avtDataObjectSource.h>
#include <avtDataObjectSink.h>


class     AttributeGroup;

class     avtDynamicAttribute;
class     avtMetaData;


// ****************************************************************************
//  Method: avtFilter
//
//  Purpose:
//      A filter is an object that does not originate or terminate a pipeline.
//      It defines what it looks like to propagate an Update to an upstream
//      filter and also incorporates the idea of "Executes", which are
//      defined by all derived types of filters.
// 
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
//  Modifications:
//  
//    Kathleen Bonnell, Wed Oct  3 08:53:21 PDT 2001
//    Added TryCurrent and GetCurrent Data/Spatial Extents.
//
//    Hank Childs, Wed Oct 24 14:21:18 PDT 2001
//    Added PreExecute and PostExecute from avtDataTreeStreamer.
//
//    Hank Childs, Fri Mar 15 15:25:33 PST 2002 
//    Add support for attributes.
//
// ****************************************************************************

class PIPELINE_API avtFilter
    : virtual public avtDataObjectSource, virtual public avtDataObjectSink
{
  public:
                                        avtFilter();
    virtual                            ~avtFilter() {;};

    virtual const char                 *GetType(void) = 0;
    virtual const char                 *GetDescription(void) { return NULL; };

    virtual bool                        Equivalent(const AttributeGroup *)
                                            { return false; };

    virtual bool                        Update(avtPipelineSpecification_p);

    virtual avtTerminatingSource       *GetTerminatingSource(void);
    virtual avtQueryableSource         *GetQueryableSource(void);
    avtPipelineSpecification_p          GetGeneralPipelineSpecification(void);
    virtual void                        ReleaseData(void);

  protected:
    bool                                modified;
    bool                                inExecute;
    std::vector<avtDynamicAttribute *>  dynamicAttributes;

    virtual void                        Execute(void) = 0;
    avtPipelineSpecification_p          PerformRestrictionAndDoBookkeeping(
                                               avtPipelineSpecification_p);
    virtual avtPipelineSpecification_p  PerformRestriction(
                                               avtPipelineSpecification_p);

    virtual void                        ChangedInput(void);
    virtual void                        InitializeFilter(void);
    virtual void                        VerifyInput(void);
    virtual int                         AdditionalPipelineFilters(void);

    void                                PassOnDataObjectInfo(void);
    virtual void                        RefashionDataObjectInfo(void);

    virtual void                        PreExecute(void);
    virtual void                        PostExecute(void);
    virtual void                        ExamineSpecification(
                                                   avtPipelineSpecification_p);

    avtMetaData                        *GetMetaData(void);

    void                                UpdateProgress(int, int);

    bool                                TryDataExtents(double *,
                                                       const char * = NULL);
    void                                GetDataExtents(double *,
                                                       const char * = NULL);
    bool                                TrySpatialExtents(double *);
    void                                GetSpatialExtents(double *);
    bool                                TryCurrentDataExtents(double *);
    void                                GetCurrentDataExtents(double *);
    bool                                TryCurrentSpatialExtents(double *);
    void                                GetCurrentSpatialExtents(double *);
    virtual void                        SearchDataForDataExtents(double *);

    void                                RegisterDynamicAttribute(
                                                        avtDynamicAttribute *);
    void                                ResolveDynamicAttributes(void);
};


#endif


