// ************************************************************************* //
//                             avtOriginatingSink.h                          //
// ************************************************************************* //

#ifndef AVT_ORIGINATING_SINK_H
#define AVT_ORIGINATING_SINK_H

#include <pipeline_exports.h>

#include <avtDataObjectSink.h>
#include <avtPipelineSpecification.h>


typedef  bool (*GuideFunction)(void *, int);


// ****************************************************************************
//  Class: avtOriginatingSink
//
//  Purpose:
//      This sink object serves as the originator of a pipeline.  It 
//      understands that there are many pipelines and what its pipeline index
//      is.  It also understands that dynamic load balancing may occur and
//      that it may have to execute a pipeline multiple times.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Sep 28 13:18:47 PDT 2001
//    Added DynamicLoadBalanceCleanUp.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers 
//    have problems with them.
//
//    Hank Childs, Wed Mar  2 11:16:01 PST 2005
//    Take a full-blown pipeline specification rather than a data spec and a
//    pipeline index.
//
// ****************************************************************************

class PIPELINE_API avtOriginatingSink : virtual public avtDataObjectSink
{
  public:
                              avtOriginatingSink();
    virtual                  ~avtOriginatingSink();

    void                      Execute(avtPipelineSpecification_p);

    static void               SetGuideFunction(GuideFunction, void *);
    static void               GetGuideFunction(GuideFunction &, void *&);

  protected:
    virtual void              InputIsReady(void);
    virtual void              DynamicLoadBalanceCleanUp(void);

  private:
    static GuideFunction      guideFunction;
    static void              *guideFunctionArgs;
};


#endif


