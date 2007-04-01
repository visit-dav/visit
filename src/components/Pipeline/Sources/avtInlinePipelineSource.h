// ************************************************************************* //
//                         avtInlinePipelineSource.h                         //
// ************************************************************************* //

#ifndef AVT_INLINE_PIPELINE_SOURCE_H
#define AVT_INLINE_PIPELINE_SOURCE_H

#include <pipeline_exports.h>

#include <avtTerminatingSource.h>


// ****************************************************************************
//  Class: avtInlinePipelineSource
//
//  Purpose:
//      There are occasions where a pipeline is "inlined" inside another
//      pipeline.  When that occurs, the there needs to be a new terminating
//      source -- avtInlinePipelineSource.  That source should be smart enough
//      to act like its own terminating source in some instances and to bypass
//      to the pipeline's real terminating source in others.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2001
//
//  Modifications:
//    Jeremy Meredith, Wed Jun  9 09:14:57 PDT 2004
//    Added species aux data.
//
//    Hank Childs, Sat Mar  5 18:36:02 PST 2005
//    Turn off load balancing for inline pipeline sources.
//
// ****************************************************************************

class PIPELINE_API avtInlinePipelineSource : virtual public avtTerminatingSource
{
  public:
                           avtInlinePipelineSource(avtDataObject_p);
    virtual               ~avtInlinePipelineSource();

    virtual avtDataSpecification_p 
                           GetFullDataSpecification(void);

  protected:
    avtTerminatingSource  *realPipelineSource;

    virtual void           FetchMeshAuxiliaryData(const char *type, void *args,
                               avtDataSpecification_p, VoidRefList &);
    virtual void           FetchVariableAuxiliaryData(const char *type,
                               void *args, avtDataSpecification_p,
                               VoidRefList &);
    virtual void           FetchMaterialAuxiliaryData(const char *type,
                               void *args, avtDataSpecification_p,
                               VoidRefList &);
    virtual void           FetchSpeciesAuxiliaryData(const char *type,
                               void *args, avtDataSpecification_p,
                               VoidRefList &);

    virtual bool           ArtificialPipeline(void)  { return true; };
    virtual bool           UseLoadBalancer(void) { return false; };
};


#endif


