// ************************************************************************* //
//                           avtTerminatingSource.h                          //
// ************************************************************************* //

#ifndef AVT_TERMINATING_SOURCE_H
#define AVT_TERMINATING_SOURCE_H

#include <pipeline_exports.h>

#include <void_ref_ptr.h>

#include <avtQueryableSource.h>
#include <avtDataSpecification.h>
#include <avtPipelineSpecification.h>


class     avtInlinePipelineSource;
class     avtMetaData;


typedef avtDataSpecification_p (*LoadBalanceFunction)(void *,
                                                   avtPipelineSpecification_p);
typedef bool                   (*DynamicCheckFunction)(void *,
                                                   avtPipelineSpecification_p);
typedef void                   (*InitializeProgressCallback)(void *, int);


// ****************************************************************************
//  Class: avtTerminatingSource
//
//  Purpose:
//      This is the terminator of a pipeline's update/execute cycle.  As such,
//      it owns the actual pipeline object.
//
//  Programmer: Hank Childs
//  Creation:   May 23, 2001
//
//  Modifications:
//
//    Jeremy Meredith, Thu Sep 20 01:01:49 PDT 2001
//    Added dynamic checker callback function so it can make determinations
//    about the network it will execute.
//
//    Hank Childs, Thu Oct 25 16:44:24 PDT 2001
//    Add a virtual function for whether or not this source permits dynamic
//    load balancing.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002
//    Added virtual method Query.
//
//    Hank Childs, Mon Jul 28 16:33:34 PDT 2003
//    Derived from avtQueryableSource instead of avtDataObjectSource.
//
//    Kathleen Bonnell, Wed Nov 12 18:26:21 PST 2003 
//    Added virtual method FindElementForPoint. 
//
// ****************************************************************************

class PIPELINE_API avtTerminatingSource : virtual public avtQueryableSource
{
    friend class                   avtInlinePipelineSource;

  public:
                                   avtTerminatingSource();
    virtual                       ~avtTerminatingSource();

    virtual avtTerminatingSource  *GetTerminatingSource(void)  { return this;};

    avtMetaData                   *GetMetaData(void) { return metadata; };
    void                           GetMeshAuxiliaryData(const char *type,
                                       void *args, avtPipelineSpecification_p,
                                       VoidRefList &);
    void                           GetVariableAuxiliaryData(const char *type,
                                       void *args, avtPipelineSpecification_p,
                                       VoidRefList &);
    void                           GetMaterialAuxiliaryData(const char *type,
                                       void *args, avtPipelineSpecification_p,
                                       VoidRefList &);

    virtual bool                   Update(avtPipelineSpecification_p);

    static void                    SetLoadBalancer(LoadBalanceFunction,void *);
    static void                    SetDynamicChecker(DynamicCheckFunction,
                                                     void *);
    static void                    RegisterInitializeProgressCallback(
                                           InitializeProgressCallback, void *);

    virtual avtDataSpecification_p GetFullDataSpecification(void);
    avtPipelineSpecification_p     GetGeneralPipelineSpecification(void);

    // Define this so derived types don't have to.
    virtual void                   Query(PickAttributes *){;};
    virtual bool                   FindElementForPoint(const char*, const int, 
                                       const int, const char*, float[3], int &)
                                       { return false;};

  protected:
    avtMetaData                   *metadata;
    static InitializeProgressCallback
                                   initializeProgressCallback;
    static void                   *initializeProgressCallbackArgs;

    virtual bool                   FetchData(avtDataSpecification_p) = 0;
    virtual void                   FetchMeshAuxiliaryData(const char *type,
                                       void *args, avtDataSpecification_p,
                                       VoidRefList &);
    virtual void                   FetchVariableAuxiliaryData(const char *type,
                                       void *args, avtDataSpecification_p,
                                       VoidRefList &);
    virtual void                   FetchMaterialAuxiliaryData(const char *type,
                                       void *args, avtDataSpecification_p,
                                       VoidRefList &);

    avtDataSpecification_p         BalanceLoad(avtPipelineSpecification_p);

    virtual bool                   UseLoadBalancer(void) { return true; };
    virtual bool                   ArtificialPipeline(void) { return false; };
    virtual int                    NumStagesForFetch(avtDataSpecification_p);

 private:
    static LoadBalanceFunction     loadBalanceFunction;
    static void                   *loadBalanceFunctionArgs;
    static DynamicCheckFunction    dynamicCheckFunction;
    static void                   *dynamicCheckFunctionArgs;

    void                           InitPipeline(avtPipelineSpecification_p);
    virtual bool                   CanDoDynamicLoadBalancing(void);

};


#endif


