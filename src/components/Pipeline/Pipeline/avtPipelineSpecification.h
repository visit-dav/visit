// ************************************************************************* //
//                         avtPipelineSpecification.h                        //
// ************************************************************************* //

#ifndef AVT_PIPELINE_SPECIFICATION_H
#define AVT_PIPELINE_SPECIFICATION_H
#include <pipeline_exports.h>


#include <ref_ptr.h>

#include <avtDataSpecification.h>

class avtPipelineSpecification;
typedef ref_ptr<avtPipelineSpecification> avtPipelineSpecification_p;


// ****************************************************************************
//  Class: avtPipelineSpecification
//
//  Purpose:
//      This is the specification of a pipeline.  This includes the
//      specification of which data you want, as well as a pipeline index to
//      be used later for load balancing and a boolean value indicating if
//      dynamic load balancing is possible.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 12:35:52 PDT 2001
//    Added ShouldUseDynamicLoadBalancing.
//
// ****************************************************************************

class PIPELINE_API avtPipelineSpecification
{
  public:
                        avtPipelineSpecification(avtDataSpecification_p, int);
                        avtPipelineSpecification(avtPipelineSpecification_p);
                        avtPipelineSpecification(avtPipelineSpecification_p,
                                                 avtDataSpecification_p);
    virtual            ~avtPipelineSpecification() {;};

    bool                ShouldUseDynamicLoadBalancing(void)
                               { return canDoDynamic; };
    void                NoDynamicLoadBalancing(void)
                               { canDoDynamic = false; };
    void                SetDataSpecification(avtDataSpecification_p ds)
                               { data = ds; };

    bool                ShouldUseLoadBalancing(void)  
                               { return useLoadBalancing; };
    void                UseLoadBalancing(bool);

    avtDataSpecification_p    GetDataSpecification(void)
                               { return data; };
    int                       GetPipelineIndex(void) 
                               { return pipelineIndex; };

    void                      AddFilter(void)  { nFilters++; };
    int                       GetNFilters(void)  { return nFilters; };

    avtPipelineSpecification &operator=(avtPipelineSpecification &);

  protected:
    avtDataSpecification_p    data;
    int                       pipelineIndex;
    bool                      canDoDynamic;
    bool                      useLoadBalancing;
    int                       nFilters;
};


#endif


