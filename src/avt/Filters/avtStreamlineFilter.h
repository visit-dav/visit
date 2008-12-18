/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                              avtStreamlineFilter.h                        //
// ************************************************************************* //

#ifndef AVT_STREAMLINE_FILTER_H
#define AVT_STREAMLINE_FILTER_H

#include <avtStreamline.h>
#include <avtDatasetOnDemandFilter.h>
#include <avtIVPDopri5.h>
#include <avtVec.h>
#include <avtIntervalTree.h>
#include <MemStream.h>
#include <filters_exports.h>
#ifdef PARALLEL
#include <avtParallel.h>
#include <mpi.h>
#endif

class vtkVisItCellLocator;
class vtkVisItStreamLine;
class vtkTubeFilter;
class vtkPolyData;
class vtkRibbonFilter;
class vtkAppendPolyData;

class SlaveInfo;

#define STREAMLINE_SOURCE_POINT      0
#define STREAMLINE_SOURCE_LINE       1
#define STREAMLINE_SOURCE_PLANE      2
#define STREAMLINE_SOURCE_SPHERE     3
#define STREAMLINE_SOURCE_BOX        4

#define STREAMLINE_COLOR_SOLID       0
#define STREAMLINE_COLOR_SPEED       1
#define STREAMLINE_COLOR_VORTICITY   2
#define STREAMLINE_COLOR_ARCLENGTH   3
#define STREAMLINE_COLOR_TIME        4
#define STREAMLINE_COLOR_ID          5

#define STREAMLINE_DISPLAY_LINES     0
#define STREAMLINE_DISPLAY_TUBES     1
#define STREAMLINE_DISPLAY_RIBBONS   2

#define STREAMLINE_TERMINATE_DISTANCE 0
#define STREAMLINE_TERMINATE_TIME 1

#define STREAMLINE_INTEGRATE_DORLAND_PRINCE 0
#define STREAMLINE_INTEGRATE_ADAMS_BASHFORTH 1

#define STREAMLINE_STAGED_LOAD_ONDEMAND 0
#define STREAMLINE_PARALLEL_STATIC_DOMAINS 1
#define STREAMLINE_MASTER_SLAVE 2

class pt3d
{
  public:
    pt3d( const pt3d &p ) {xyz[0]=p.xyz[0]; xyz[1]=p.xyz[1]; xyz[2]=p.xyz[2]; }
    pt3d() { xyz[0]=xyz[1]=xyz[2]=0.0;}
    pt3d( double _x, double _y, double _z=0.0 ) {xyz[0]=_x; xyz[1]=_y; xyz[2]=_z;}

    const double *values() const { return xyz; }

    //double& operator[]( const size_type& n ) const { return xyz[n]; }
    //double& operator[]( const size_type& n ) { return xyz[n]; }


    double xyz[3];
};

typedef enum
{
    NO_BALANCE,
    MAX_CNT,
    MAX_PCT
    
} BalanceType;

/*
typedef enum
{
    STAGED_LOAD_ONDEMAND,
    STATIC_DOMAINS,
    BALANCED_STATIC_DOMAINS,
    ASYNC_BALANCED_STATIC_DOMAINS,
    BALANCED_LOAD_ONDEMAND,
    ASYNC_BALANCED_LOAD_ONDEMAND,
} SLMethod;
*/

class avtStreamlineWrapper
{
  public:
    enum Status
    {
        UNSET=-1,
        TERMINATE,
        OUTOFBOUNDS
    };

    enum Dir
    {
        FWD=0,
        BWD
    };

    avtStreamlineWrapper();
    avtStreamlineWrapper( avtStreamline *s, Dir slDir=FWD, int ID=-1 );
    ~avtStreamlineWrapper();

    void UpdateDomainCount( int dom );
    void ComputeStatistics();

    vtkPolyData * GetVTKPolyData( int spatialDim, int coloringMethod, int displayMethod, vector<float> &thetas );
    void GetStartPoint( pt3d &pt );
    void GetEndPoint( pt3d &pt );

    void Debug();
    void Serialize( MemStream::Mode mode, MemStream &buff, avtIVPSolver *solver );


    avtStreamline *sl;

    // Helpers needed for computing streamlines
    std::vector<int> seedPtDomainList;
    int domain;
    Status status;
    Dir dir;
    
    // statistical bookeeping.
    vector<int> domainVisitCnts;
    int maxCnt, sum, numDomainsVisited;
    int numTimesCommunicated;
    int id;
};

// ****************************************************************************
// Class: avtStreamlineFilter
//
// Purpose:
//    This operator is the implied operator associated with a Streamline plot.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 4 15:32:05 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Dec 22 12:45:29 PDT 2004
//   Added ability to color by vorticity and the ability to display as ribbons.
//
//   Brad Whitlock, Mon Jan 3 10:56:47 PDT 2005
//   Added SetZToZero to ensure that we can set restrict streamline sources
//   to 2D when the input data is 2D.
//
//   Brad Whitlock, Tue Jan 4 10:56:47 PDT 2005
//   Removed the integrator member since it was not being used.
//
//   Hank Childs, Sat Mar  3 09:43:57 PST 2007
//   Add new data member: useWholeBox.  Also added method PreExecute to 
//   support it.  Added PostExecute method to get the extents right.
//
//   Dave Pugmire, Thu Nov 15 12:09:08 EST 2007
//   Add support for streamline direction option.
//
//   Hank Childs, Tue Mar  4 08:54:27 PST 2008
//   Refactored to inherit from avtDatasetOnDemandFilter.
//
//   Hank Childs, Thu Jun 12 16:52:35 PDT 2008
//   Define method ModifyContract.
//
//   Dave Pugmire, Wed Aug 6 15:16:23 EST 2008
//   Add accurate distance calculate option.
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Add dataSpatialDimension
//
//   Dave Pugmire, Tue Aug 19 17:13:04EST 2008
//   Remove accurate distance calculate option.
//
//   Hank Childs, Tue Dec  2 13:53:49 PST 2008
//   Made CreateStreamlineOutput be a pure virtual function.
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add 3 point density vars. Add masterSlave algorithm.
//
// ****************************************************************************

class AVTFILTERS_API avtStreamlineFilter : public avtDatasetOnDemandFilter
{
  public:
                              avtStreamlineFilter();
    virtual                  ~avtStreamlineFilter();

    virtual const char       *GetType(void)  { return "avtStreamlineFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Creating streamlines"; };

    // Methods to set the filter's attributes.
    void                      SetSourceType(int sourceType);
    void                      SetMaxStepLength(double len);
    void                      SetTermination(int type, double term);
    void                      SetIntegrationType(int algo);
    void                      SetStreamlineAlgorithm(int algo, int maxCnt, int domainCache);
    void                      SetTolerances(double reltol, double abstol);

    void                      SetPointSource(double pt[3]);
    void                      SetLineSource(double pt[3], double pt2[3]);
    void                      SetPlaneSource(double O[3], double N[3],
                                             double U[3], double R);
    void                      SetSphereSource(double O[3], double R);
    void                      SetBoxSource(double E[6]);
    void                      SetUseWholeBox(bool b) { useWholeBox = b; };

    void                      SetDisplayMethod(int d);
    void                      SetShowStart(bool);
    void                      SetRadius(double rad);
    void                      SetPointDensity(int den);
    void                      SetStreamlineDirection(int dir);
    void                      SetColoringMethod(int);

  protected:
    int    sourceType;   
    double maxStepLength;
    double relTol;
    double absTol;
    int terminationType;
    int integrationType;
    double termination;
    double radius;
    int    displayMethod;
    bool   showStart;
    int    pointDensity1, pointDensity2, pointDensity3;
    int    streamlineDirection;
    int    coloringMethod;
    int    dataSpatialDimension;
    std::string normalizedVecExprName;

    avtIntervalTree *intervalTree;
    avtIVPSolver *solver;

    int numDomains, cacheQLen;
    std::vector<int> domainToRank;
    std::vector<vtkDataSet*>dataSets;
    std::map<int, vtkVisItCellLocator*> domainToCellLocatorMap;

    int numDomainsLoaded, numSLCommunicated, numStatusCommunicated, 
        numIntegrationSteps, numIterations, numBytesSent, numBusyLoopIterations;
    int maxSLCommunications, totalMaxSLCommunications;
    int totalNumDomainsLoaded, totalNumSLCommunicated, 
        totalNumStatusCommunicated, totalNumIntegrationSteps, 
        totalNumIterations, totalNumBytesSent, totalNumBusyLoopIterations;
    bool haveGhostZones;
#ifdef PARALLEL
    int rank, nProcs;
    std::map<MPI_Request, unsigned char*> sendSLBufferMap, recvSLBufferMap;
    std::map<MPI_Request, int *> sendIntBufferMap, recvIntBufferMap;
    std::vector<int> terminationSends;
    std::vector< std::vector<int> > domainsLoaded;
    std::vector<int> allSLCounts;

    std::vector<MPI_Request>  statusRecvRequests, slRecvRequests;
    int                       statusMsgSz;
    std::vector<SlaveInfo>    slaveInfo;
    vector<int>               slDomCnts, domLoaded;
#endif

    // Various starting locations for streamlines.
    double pointSource[3];
    double lineStart[3], lineEnd[3];
    double planeOrigin[3], planeNormal[3], planeUpAxis[3], planeRadius;
    double sphereOrigin[3], sphereRadius;
    double boxExtents[6];
    bool   useWholeBox;

    //Timings helpers.
    int                       numSeedPts;
    double                    totalTime, wallTime;
    double                    integrationTime, communicationTime, 
                              IOTime, sortTime, sleepTime;
    float                     totalIOTime, totalIntegrationTime, totalCommTime, 
                              totalTotalTime, totalSortTime, totalSleepTime;
    //Statistics.
    typedef struct SLStatistics
    {
        float min, max, mean, sigma, total;
        float value;
    } SLStatistics;

    SLStatistics              IOTimeStats, integrationTimeStats, commTimeStats,
                              totalTimeStats, sleepTimeStats, sortTimeStats,
                              loadDomStats, statusCommStats, slCommStats, bytesSentStats,
                              iterationStats, intStepStats, busyLoopStats;

    /*
    float                     minMaxIOTime[2], meanIOTime, stdDevIOTime,
                              minMaxIntegrationTime[2], meanIntTime, stdDevIntTime,
                              minMaxCommTime[2], meanCommTime, stdDevCommTime,
                              minMaxTotalTime[2], meanTotalTime, stdDevTotalTime,
                              minMaxSortTime[2], meanSortTime, stdDevSortTime,
                              minMaxSleepTime[2],meanSleepTime, stdDevSleepTime;
    
    int                       minMaxNumDomains[2],minMaxNumSLComm[2], minMaxNumStatusComm[2],
                              minMaxNumIntSteps[2], minMaxNumIterations[2],minMaxNumBytesSent[2],
                              minMaxNumBusyLoopIterations[2];
    float                     meanNumDomains, stdDevNumDomains, meanNumSLComm, stdDevSLComm,
    meanStatusComm, stdDevStatusComm;
    */

    double                    gatherTime1, gatherTime2, asyncSLTime, 
                              asyncTermTime, asyncSendCleanupTime;
    void                      ReportTimings();
    void                      ReportTimings(ostream &os);
    int                       method;
    int                       maxCount, balanceNumToSend;
    double                    loadFactor, underWorkedFactor;
    

    virtual void              Execute(void);
    virtual void              UpdateDataObjectInfo(void);
    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
    virtual avtContract_p     ModifyContract(avtContract_p);
    virtual bool              CheckOnDemandViability(void);

    virtual vtkDataSet        *GetDomain(int);
    virtual bool              DomainLoaded(int) const;

    void                      SetZToZero(vtkPolyData *) const;

    void                      GetSeedPoints(
                                   std::vector<avtStreamlineWrapper *> &pts);
    void                      IntegrateStreamline(avtStreamlineWrapper *slSeg, int maxSteps=-1);
    avtIVPSolver::Result      IntegrateDomain(avtStreamlineWrapper *slSeg, 
                                              vtkDataSet *ds,
                                              double *extents,
                                              int maxSteps=-1);
    virtual void              CreateStreamlineOutput( 
                                                     vector<avtStreamlineWrapper *> &streamlines)
                                                    = 0;

    void                      ReportStatistics(
                                  vector<avtStreamlineWrapper *> &streamlines);

    //Streamline techniques.

    virtual void              LoadOnDemand(
                                   std::vector<avtStreamlineWrapper *> &sdpts);
    virtual void              StagedLoadOnDemand(
                                   std::vector<avtStreamlineWrapper *> &sdpts);
    virtual void              ParallelBalancedStaticDomains(
                                   std::vector<avtStreamlineWrapper *> &sdpts,
                                   bool asynchronous = false,
                                   BalanceType balance=NO_BALANCE,
                                   bool loadOnDemand = false );
    virtual void              AsyncStaticDomains( 
                                   std::vector<avtStreamlineWrapper *> &sdpts );
    virtual void              MasterSlave(
                                    std::vector<avtStreamlineWrapper *> &sdpts);
    virtual void             UpdateStatus(vector<avtStreamlineWrapper *> &streamlines,
                                          vector<vector<int> > &status);
    virtual void             PrintStatus();
    virtual void             Case1(vector<avtStreamlineWrapper *> &streamlines );
    virtual void             Case2(vector<avtStreamlineWrapper *> &streamlines );
    virtual void             Case3(vector<avtStreamlineWrapper *> &streamlines,
                                   int overloadFactor,
                                   int NDomainFactor);
    virtual void             Case4(vector<avtStreamlineWrapper *> &streamlines,
                                   int oobThreshold);
    virtual void              FindSlackers(std::vector<int> &slackers,
                                           int oobFactor=-1,
                                           int sortMethod=0,
                                           bool checkJustUpdated=false);
    /*
    virtual void              AsynchronousParallelStaticDomains(
                                   std::vector<pt3d> &seedpoints);
    virtual void              OLD_ParallelBalancedStaticDomains( 
                                   std::vector<pt3d> &seedpoints,
                                   BalanceType balance=NO_BALANCE,
                                   int maxPts=-1,
                                   int divFactor=1);
    virtual void              ParallelBalancedLoadOnDemand(
                                   std::vector<pt3d> &seedpoints,
                                   float loadFactor = 1.20,
                                   BalanceType balance=NO_BALANCE,
                                   int maxPts=-1,
                                   int divFactor=1);
    */

    virtual void              SortStreamlines( 
                                   std::vector<avtStreamlineWrapper *> &);
    virtual bool              StaticDomainExchangeStreamlines( 
                               std::vector<avtStreamlineWrapper *> &slines,
                               std::vector< 
                                    std::vector< avtStreamlineWrapper *> > &);
    virtual bool              AsyncExchangeStreamlines( 
                               std::vector<avtStreamlineWrapper *> &streamlines,
                               std::vector< 
                               std::vector< avtStreamlineWrapper *> > &,
                               int &earlyTerminations);

    virtual void              AsyncExchangeStatus(int numTerminated,
                                                  int &otherTerminates,
                                                  bool recvBalanceInfo,
                                                  bool sendBalanceInfo=false,
                                                  int slCount=0);

    virtual void              AsyncSendStatus2(int master,
                                               int numTerminated,
                                               std::vector<int> &status);
    virtual bool              AsyncRecvStatus2(int &numTerminated,
                                               std::vector< std::vector<int> > &slaveStatus);
    virtual bool              AsyncRecvMasterMsg(std::vector< std::vector<int> > &balanceInfo);
    virtual void              AsyncSendSlaveMsgs(int dst, int msg, std::vector<int> &info);
    virtual void              AsyncSendSlaveMsgs(int msg, std::vector<int> &info);
    
    virtual void              FigureOutBalancing( 
                                 std::vector<avtStreamlineWrapper *> &slines, 
                                 float loadFactor );
    virtual void              AsyncFigureOutBalancing( 
                                 std::vector<avtStreamlineWrapper *> &slines );

    virtual void              SendStreamlinesTo(int num, int sender, 
                                   int receiver, 
                                   std::vector<avtStreamlineWrapper *> &);

    virtual void              InitRequests();
    virtual void              CheckPendingSendRequests();
    virtual void              CleanupAsynchronous();
    virtual void              PostRecvStatusReq( int proc );
    virtual void              PostRecvSLReq( int proc );

    virtual void              AsyncSendSL(int receiver,
                                          avtStreamlineWrapper *slSeg);
    virtual void              AsyncSendSLs(int receiver, 
                                 const std::vector<avtStreamlineWrapper*> 
                                                                      &slSegs);

    virtual void              AsyncSendStatus(int numTerminated, 
                                    bool sendBalanceInfo=false, int slCount=0);
    virtual void              AsyncRecvStatus(int &numTerminated, 
                                              bool balanceInfo=false );

    virtual void              PrintLoadBalanceInfo();
    virtual int               AsyncRecvStreamlines(
                                    std::vector<avtStreamlineWrapper *> &slines,
                                    int &earlyTerminations );
    virtual void              AsyncRecvs(
                                std::vector<avtStreamlineWrapper *> &slines, 
                                bool blockingWait, int *numSLs, int *numTerm );

    // Helper functions.
    vtkPolyData *             StartSphere( float val, double pt[3] );
    bool                      PointInDomain( pt3d &pt, int domain );
    int                       DomainToRank( int domain );
    void                      ComputeDomainToRankMapping();
    bool                      OwnDomain( int domain );
    void                      SetDomain( avtStreamlineWrapper *slSeg );
    void                      Initialize();
    void                      ComputeRankList(const std::vector<int> &domList, 
                                              std::vector<int> &ranks, 
                                              std::vector<int> &doms );

    void                      InitStatistics();
    void                      FinalizeStatistics();
    void                      ComputeStatistics( float val,
                                                 SLStatistics &stats,
                                                 bool skipMaster=false);
};


#endif


