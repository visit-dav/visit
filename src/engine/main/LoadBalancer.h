// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                LoadBalancer.h                             //
// ************************************************************************* //

#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H


#include <string>
#include <vector>
#include <set>
#include <map>

#include <avtContract.h>
#include <avtIOInformation.h>
#include <engine_main_exports.h>

class     avtDatabase;


//
//  Callbacks
//
typedef   bool (*ParAbortCallback)(void *, bool);
typedef   void (*ProgressCallback)(void *, const char *, const char *,int,int);

// ****************************************************************************
//  Struct: LBInfo
//
//  Purpose:
//    Structure for load balancing information for each pipeline.
//
//  Modifications:
//
//    Hank Childs, Sun Feb 27 12:22:08 PST 2005
//    Added haveInitializedDLB and doDLB.
//
// ****************************************************************************
struct LBInfo
{
    std::string                       db;           // database name
    bool                              complete;     // true if complete
    int                               current;      // current domain (else -1)
    bool                              haveInitializedDLB; 
    bool                              doDLB; 

    LBInfo(const std::string &db_) : db(db_), complete(false), current(-1),
                                     haveInitializedDLB(false), doDLB(false){};
};

//
//  Structure containing info about which domains are on each processor/file
//
struct IOInfo
{
    avtIOInformation                  ioInfo;  // io hints, restrictions
    std::vector< int >                fileMap; // domain->file map
    std::vector< std::set<int> >      files;   // files open by each proc
    std::vector< std::set<int> >      domains; // domains on each proc
};

// ****************************************************************************
//  Class: LoadBalancer
//
//  Purpose:
//      Balances the load for multiple pipelines, whether it is dynamic or
//      static.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 12:30:43 PDT 2001
//    Added the ioMap and pipelineInfo.
//
//    Jeremy Meredith, Mon Sep 17 23:20:34 PDT 2001
//    Made the ioMap contain both the iohints and the cached domain lists.
//
//    Jeremy Meredith, Thu Sep 20 00:46:27 PDT 2001
//    Added a progress callback to match one in avtDataObjectSource so the
//    master process when dynamic load balancing can send progress updates.
//    Added CheckDynamicLoadBalancing so that clients can determine if
//    load balancing will be dynamic for a given pipeline specification.
//    Also, added file information to the IOInfo to support IO hints.
//
//    Jeremy Meredith, Fri Sep 21 14:42:42 PDT 2001
//    Added abort callbacks.
//    Added forceStatic/Dynamic.
//
//    Hank Childs, Wed Nov 21 11:36:35 PST 2001
//    Allow a pipeline to be reset.
//
//    Hank Childs, Mon May 12 19:34:54 PDT 2003
//    Allow for different load balancing schemes.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added avtDatabaseMetaData arg to AddDatabase
//
//    Hank Childs, Sun Feb 27 11:12:44 PST 2005
//    Added avtDatabase argument to AddDatabase.  Also added data member dbMap.
//
//    Hank Childs, Sun Mar  6 08:42:50 PST 2005
//    Renamed ForceDynamic to AllowDynamic.
//
//    Jeremy Meredith, Wed May 11 09:07:15 PDT 2005
//    Added "restricted" load balancing mode.  This is intended for
//    non-global filesystems and simulation-mode engines.
//
//    Mark C. Miller, Wed Sep 14 15:10:06 PDT 2005
//    Added "absolute" load balancing scheme. This assigns domains
//    to processors such that the same domains are always processed by the
//    same processors, regardless of which domains are involved in the
//    current pipeline.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added DetermineAppropriateLoadBalanceScheme 
//    Removed avtIOInformation and avtDatabaseMetaData args from AddDatabase
//
//    Hank Childs, Tue Feb 19 08:56:54 PST 2008
//    Add data member domainListForStreaming.
//
//    Hank Childs, Tue Feb 19 22:05:02 PST 2008
//    Remove unused method for checking whether we are doing dynamic load
//    balancing.
//
//    Brad Whitlock, Thu Jun 19 12:17:48 PDT 2014
//    Added GetIOInformation method.
//
// ****************************************************************************

class ENGINE_MAIN_API LoadBalancer
{
  public:
                                  LoadBalancer(int nProcs, int rank);
    virtual                      ~LoadBalancer() {;};

    avtDataRequest_p              Reduce(avtContract_p input);
    bool                          CheckDynamicLoadBalancing(
                                         avtContract_p input);

    LoadBalanceScheme             DetermineAppropriateScheme(
                                      avtContract_p input);

    void                          AddDatabase(const std::string &dbname,
                                              avtDatabase *, int time);

    int                           AddPipeline(const std::string &dbname);
    void                          ResetPipeline(int);

    bool                          ContinueExecute(int);

    static void                   RegisterProgressCallback(ProgressCallback,
                                                           void *);
    static void                   RegisterAbortCallback(ParAbortCallback,
                                                        void *);
    static void                   AllowDynamic();
    static bool                   GetAllowDynamic();
    static void                   SetScheme(LoadBalanceScheme);
    static LoadBalanceScheme      GetScheme();
    static std::string            GetSchemeAsString();
  protected:
    bool                          CheckAbort(bool);
    void                          UpdateProgress(int, int);
    bool                          GetIOInformation(const std::string &db,
                                                   int time,
                                                   const std::string &meshname);
    std::string                   GetMeshName(avtContract_p input, int stateIndex);

    static ParAbortCallback           abortCallback;
    static void                      *abortCallbackArgs;
    static ProgressCallback           progressCallback;
    static void                      *progressCallbackArgs;

    static bool                       allowDynamic;
    static LoadBalanceScheme          scheme;

    static int                        lastDomDoneMsg;
    static int                        newDomToDoMsg;

    int                               rank;
    int                               nProcs;

    std::map<std::string, IOInfo>     ioMap;
    std::vector<LBInfo>               pipelineInfo;
    std::map<std::string, avtDatabase *>  dbMap;
    std::map<std::string, int>            dbState;

    // Special data member for -lb-stream option.
    std::vector<int>                  domainListForStreaming;
};


#endif


