// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtOriginatingSource.h                          //
// ************************************************************************* //

#ifndef AVT_ORIGINATING_SOURCE_H
#define AVT_ORIGINATING_SOURCE_H

#include <pipeline_exports.h>

#include <void_ref_ptr.h>

#include <avtQueryableSource.h>
#include <avtDataRequest.h>
#include <avtContract.h>

class     vtkObject;

class     avtInlinePipelineSource;
class     avtMetaData;


typedef avtDataRequest_p       (*LoadBalanceFunction)(void *, avtContract_p);
typedef bool                   (*StreamingCheckFunction)(void *,avtContract_p);
typedef void                   (*InitializeProgressCallback)(void *, int);


// ****************************************************************************
//  Class: avtOriginatingSource
//
//  Purpose:
//      This is the originator of a pipeline's update/execute cycle.  As such,
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
//    Kathleen Bonnell, Mon Dec 22 14:48:57 PST 2003 
//    Added virtual method GetDomainName. 
//
//    Kathleen Bonnell, Tue May 25 16:16:25 PDT 2004 
//    Add virtual method 'QueryZoneCenter'.
//
//    Jeremy Meredith, Wed Jun  9 09:13:39 PDT 2004
//    Added species aux data.
//
//    Kathleen Bonnell, Thu Jun 10 18:31:22 PDT 2004 
//    Renamed QueryZoneCenter to QueryCoords, added bool arg.
//
//    Kathleen Bonnell, Thu Dec 16 17:16:33 PST 2004 
//    Added another bool arg to QueryCoords. 
//
//    Kathleen Bonnell, Mon Jan  3 13:40:42 PST 2005 
//    Added GetSIL method. 
//
//    Kathleen Bonnell, Tue Jan 25 07:59:28 PST 2005 
//    Added const char *arg to QueryCoords. 
//
//    Hank Childs, Wed Feb  7 13:18:28 PST 2007
//    Add support for progress from time queries.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Hank Childs, Thu Jun 12 16:12:38 PDT 2008
//    Add method CanDoStreaming.
//
//    Hank Childs, Thu Aug 26 16:57:24 PDT 2010
//    Add data member for the last contract.
//
//    Hank Childs, Fri Nov 26 16:26:55 PST 2010
//    Add support for caching of arbitrary objects.
//
//    Hank Childs, Tue Dec 11 14:39:58 PST 2012
//    Add method for getting last data selections.
//
//    Kathleen Biagas, Mon Jun  5 16:41:17 PDT 2017
//    Added ResetAllExtents.
//
// ****************************************************************************

class PIPELINE_API avtOriginatingSource : virtual public avtQueryableSource
{
    friend class                   avtInlinePipelineSource;

  public:
                                   avtOriginatingSource();
    virtual                       ~avtOriginatingSource();

    virtual avtOriginatingSource  *GetOriginatingSource(void)  { return this;};

    avtMetaData                   *GetMetaData(void) { return metadata; };
    void                           GetMeshAuxiliaryData(const char *type,
                                       void *args, avtContract_p,
                                       VoidRefList &);
    void                           GetVariableAuxiliaryData(const char *type,
                                       void *args, avtContract_p,
                                       VoidRefList &);
    void                           GetMaterialAuxiliaryData(const char *type,
                                       void *args, avtContract_p,
                                       VoidRefList &);
    void                           GetSpeciesAuxiliaryData(const char *type,
                                       void *args, avtContract_p,
                                       VoidRefList &);

    virtual vtkObject             *FetchArbitraryVTKObject(const char *name,
                                                           int dom, int ts,
                                                           const char *type);
    virtual void                   StoreArbitraryVTKObject(const char *name,
                                                           int dom, int ts,
                                                           const char *type,
                                                           vtkObject *);
    virtual void_ref_ptr           FetchArbitraryRefPtr(const char *name,
                                                        int dom, int ts,
                                                        const char *type);
    virtual void                   StoreArbitraryRefPtr(const char *name,
                                                        int dom, int ts,
                                                        const char *type,
                                                        void_ref_ptr);

    virtual bool                   Update(avtContract_p);
    virtual void                   ResetAllExtents(void);
    virtual bool                   CanDoStreaming(avtContract_p) {return true;}

    static void                    SetLoadBalancer(LoadBalanceFunction,void *);
    static void                    SetStreamingChecker(StreamingCheckFunction,
                                                     void *);
    static void                    RegisterInitializeProgressCallback(
                                           InitializeProgressCallback, void *);
    void                           SetNumberOfExecutions(int numEx)
                                         { numberOfExecutions = numEx;
                                           haveIssuedProgress = false; };

    virtual avtDataRequest_p       GetFullDataRequest(void);
    avtContract_p                  GetGeneralContract(void);
    std::vector<avtDataSelection_p>  GetSelectionsForLastExecution(void);

    // Define this so derived types don't have to.
    virtual void                   Query(PickAttributes *){;};
    virtual avtSIL                *GetSIL(int){return NULL;};
    virtual bool                   FindElementForPoint(const char*, const int, 
                                       const int, const char*, double[3], int &)
                                       { return false;};
    virtual void                   GetDomainName(const std::string &, const int, 
                                       const int, std::string &) {;};
    virtual bool                   QueryCoords(const std::string &, const int, 
                                       const int, const int, double[3],
                                       const bool, const bool = false, const char *mn=NULL)
                                       { (void)mn; return false;};

  protected:
    avtMetaData                   *metadata;
    static InitializeProgressCallback
                                   initializeProgressCallback;
    static void                   *initializeProgressCallbackArgs;

    virtual bool                   FetchData(avtDataRequest_p) = 0;
    virtual void                   FetchMeshAuxiliaryData(const char *type,
                                       void *args, avtDataRequest_p,
                                       VoidRefList &);
    virtual void                   FetchVariableAuxiliaryData(const char *type,
                                       void *args, avtDataRequest_p,
                                       VoidRefList &);
    virtual void                   FetchMaterialAuxiliaryData(const char *type,
                                       void *args, avtDataRequest_p,
                                       VoidRefList &);
    virtual void                   FetchSpeciesAuxiliaryData(const char *type,
                                       void *args, avtDataRequest_p,
                                       VoidRefList &);

    avtDataRequest_p               BalanceLoad(avtContract_p);

    virtual bool                   UseLoadBalancer(void) { return true; };
    virtual bool                   ArtificialPipeline(void) { return false; };
    virtual int                    NumStagesForFetch(avtDataRequest_p);

 private:
    static LoadBalanceFunction     loadBalanceFunction;
    static void                   *loadBalanceFunctionArgs;
    static StreamingCheckFunction  streamingCheckFunction;
    static void                   *streamingCheckFunctionArgs;

    void                           InitPipeline(avtContract_p);
    virtual bool                   CanDoStreaming(void);

    // These data members are important when there are multiple executions, like
    // with the time loop filter.  This information is important to keep track
    // of progress.
    int                            numberOfExecutions;
    bool                           haveIssuedProgress;

  protected:
    avtContract_p                  lastContract;
};


#endif


