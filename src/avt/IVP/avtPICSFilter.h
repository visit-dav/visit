// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtPICSFilter.h                               //
// ************************************************************************* //

#ifndef AVT_PICS_FILTER_H
#define AVT_PICS_FILTER_H

#include <avtCellLocator.h>
#include <avtIntegralCurve.h>
#include <avtDatasetOnDemandFilter.h>
#include <avtDatasetToDatasetFilter.h>
#include <avtIVPDopri5.h>
#include <avtVec.h>
#include <avtIntervalTree.h>
#include <MemStream.h>
#include <filters_exports.h>
#ifdef PARALLEL
#include <avtParallel.h>
#include <mpi.h>
#endif

#include <vector>

class BlockIDType;
class avtICAlgorithm;

// Must match the integral curve (poincare) attributes!!
#define PICS_FIELD_DEFAULT   0
#define PICS_FIELD_FLASH     1
#define PICS_FIELD_M3D_C1_2D 2
#define PICS_FIELD_M3D_C1_3D 3
#define PICS_FIELD_NEK5000   4
#define PICS_FIELD_NEKTARPP  5

#define PICS_INTEGRATE_EULER 0
#define PICS_INTEGRATE_LEAPFROG 1
#define PICS_INTEGRATE_DORMAND_PRINCE 2
#define PICS_INTEGRATE_ADAMS_BASHFORTH 3
#define PICS_INTEGRATE_RK4 4
#define PICS_INTEGRATE_M3D_C1_2D 5

#define PICS_TERMINATE_DISTANCE 0
#define PICS_TERMINATE_TIME 1
#define PICS_TERMINATE_STEPS 2
#define PICS_TERMINATE_INTERSECTIONS 3

#define PICS_SERIAL                0
#define PICS_PARALLEL_OVER_DOMAINS 1
#define PICS_PARALLEL_MASTER_SLAVE 2
#define PICS_VISIT_SELECTS         3

#define PICS_CONN_CMFE 0
#define PICS_POS_CMFE  1

#define PICS_PARALLEL_COMM_DOMAINS 4

#define PICS_INTEGRATE_FORWARD         0
#define PICS_INTEGRATE_BACKWARD        1
#define PICS_INTEGRATE_BOTH_DIRECTIONS 2
#define PICS_INTEGRATE_FORWARD_DIRECTIONLESS  3
#define PICS_INTEGRATE_BACKWARD_DIRECTIONLESS 4
#define PICS_INTEGRATE_BOTH_DIRECTIONLESS     5



// ****************************************************************************
// Class: avtPICSFilter
//
// Purpose:
//    PICS = Parallel integral curve system.  An abstract type.  The purpose
//    of this filter is to define an AVT filter that can do parallel
//    particle advection.  How to analyze those particles, termination 
//    criteria, etc, are left to the derived types.  Examples of derived
//    types are for integral curves and poincare analysis.
//
// Programmer: Hank Childs
// Creation:   June 5, 2010
//
// Modifications:
//
//   Hank Childs, Tue Jun  8 09:11:36 CDT 2010
//   Added communication pattern enum and virtual method.
//
//   Christoph Garth, Fri Jul 9 12:53:11 PDF 2010
//   Replace vtkVisItCellLocator by avtCellLocator
// 
//   Hank Childs, Sun Sep 19 11:04:32 PDT 2010
//   Added UpdateDataObjectInfo.
//
//   Hank Childs, Fri Oct  1 20:35:21 PDT 2010
//   Add an argument for absTol that is a fraction of the bounding box.
//
//   Hank Childs, Fri Oct  8 23:30:27 PDT 2010
//   Allow for multiple termination criterias.
//
//   Hank Childs, Thu Oct 21 08:54:51 PDT 2010
//   Add support for cases where we have an empty data set.
//
//   Hank Childs, Sat Nov 27 16:52:12 PST 2010
//   Add method for updating progress.  This method exists in the base class, 
//   but is protected.  The purpose of this method is to make it public to the
//   IC algorithms.
//
//   Hank Childs, Sun Nov 28 12:20:12 PST 2010
//   Add support for caching locators in the database.
//
//   Dave Pugmire, Fri Jan 14 11:09:59 EST 2011
//   Add new communication pattern: RestoreSequenceAssembleUniformly, and
//   PostStepCallback()
//
//   David Camp, Tue May  3 06:51:37 PDT 2011
//   Added the PurgeDomain() to be able to remove the cell locator when the
//   avtDatasetOnDemandFilter purges a data set from it's cache.
//
//   Dave Pugmire, Wed Jun  5 16:43:36 EDT 2013
//   Code hardening. Better handling for rectilinear grid corner cases.
//
//   Dave Pugmire, Mon Dec 15 11:00:23 EST 2014
//   Return number of steps taken.
//
// ****************************************************************************

class IVP_API avtPICSFilter : 
    virtual public avtDatasetOnDemandFilter,
    virtual public avtDatasetToDatasetFilter
{
    using avtDatasetOnDemandFilter::GetDomain;
  public:
    enum CommunicationPattern
    {
        RestoreSequenceAssembleOnCurrentProcessor = 0,
        RestoreSequenceAssembleUniformly,
        ReturnToOriginatingProcessor,
        LeaveOnCurrentProcessor,
        UndefinedCommunicationPattern
    };

                              avtPICSFilter();
    virtual                  ~avtPICSFilter();

    virtual const char       *GetType(void)  { return "avtPICSFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Advecting particles"; };

    virtual avtIntegralCurve          *CreateIntegralCurve() = 0;
    virtual avtIntegralCurve          *CreateIntegralCurve(
                                        const avtIVPSolver* model,
                                        avtIntegralCurve::Direction dir,
                                        const double& t_start,
                                        const avtVector &p_start,
                                        const avtVector &v_start,
                                        long ID) = 0;

    virtual bool                    GetAllSeedsSentToAllProcs() = 0;

    virtual std::vector<avtVector>  GetInitialLocations() = 0;
    virtual std::vector<avtVector>  GetInitialVelocities() = 0;
    virtual CommunicationPattern    GetCommunicationPattern() = 0;
    std::vector<std::pair<int,int> > GetICPairs() { return ICPairs; }
    
    // Methods to set the filter's attributes.
    void SetFieldType(int val);
    void SetFieldConstant(double val);
    void SetMaxStepLength(double len);
    void SetPathlines(bool pathlines, bool overrideTime,
                      double time0, double period, int _pathlineCMFE);
    void SetIntegrationType(int algo);
    void SetParallelizationAlgorithm(int algo, int maxCnt,
                                     int domainCache,
                                     int workGrpSz);
    void SetTolerances(double reltol, double abstol, bool isFraction);

    void SetIntegrationDirection(int dir);
    int  GetIntegrationDirection() { return integrationDirection; };

    void InitializeLocators(void);
    void UpdateProgress(int amt, int total)
    {
        avtFilter::UpdateProgress(amt, total);
    };

    virtual void ReleaseData(void);
    virtual void UpdateDataObjectInfo(void);

    void         ConvertToCartesian(bool val) { convertToCartesian = val; };
    bool         PostStepCallback();

    void         FindCandidateBlocks(avtIntegralCurve *ic, BlockIDType *skipBlk=NULL);
    bool         ICInBlock(const avtIntegralCurve *ic, const BlockIDType &block);
    
  protected:
    bool   emptyDataset;
    double maxStepLength;
    double relTol;
    double absTol;
    bool   absTolIsFraction;
    int    integrationType;
    int    integrationDirection;
    bool   directionlessField;
    int    dataSpatialDimension;
    bool   convertToCartesian;

    int    fieldType;
    double fieldConstant;

    avtICAlgorithm *icAlgo;

    avtContract_p lastContract;

    std::vector<std::vector<double> > domainTimeIntervals;
    bool   doPathlines;
    bool   pathlineOverrideTime;
    int    pathlineCMFE;
    double seedTime0;
    int    seedTimeStep0;

    double period, baseTime, timeSliceInterval;
    bool   rollover;

    avtIntervalTree *intervalTree;
    bool             specifyPoint;
    avtIVPSolver *solver;

    int numDomains, cacheQLen;
    std::vector<int> domainToRank;
    std::vector<vtkDataSet*>dataSets;
    std::map<BlockIDType, avtCellLocator_p> domainToCellLocatorMap;
    std::vector<std::pair<int,int> > ICPairs;

    std::vector<double> pointList;

    // Data retrieved from contract
    int activeTimeStep;

    // Time slice currently read and being processed
    int curTimeSlice;

    // Timings helpers.
    int                       MaxID;
    int                       selectedAlgo;
    int                       maxCount, workGroupSz;
    double                    InitialIOTime;
    int                       InitialDomLoads;

    virtual void              Execute(void);
    virtual bool              ContinueExecute() {return false;}
    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
    virtual avtContract_p     ModifyContract(avtContract_p);
    virtual void              ExamineContract(avtContract_p);
    virtual bool              CheckOnDemandViability(void);

    int                       AdvectParticle(avtIntegralCurve *ic);
    virtual vtkDataSet        *GetDomain(const BlockIDType &, const avtVector &);
    bool                      LoadNextTimeSlice();
    virtual int               GetTimeStep(double t) const;
    virtual bool              BlockLoaded(BlockIDType &) const;
    bool                      ICInRectilinearBlock(const avtIntegralCurve *ic, 
                                                   const BlockIDType &block,
                                                   vtkDataSet *ds);
    int                       OnFace(const avtIntegralCurve *ic, double *bbox);
    int                       GetNextCurveID(){ int id = MaxID; MaxID++; return id;}
    void                      CreateIntegralCurvesFromSeeds(std::vector<avtVector> &pts,
                                                            std::vector<avtVector> &vels,
                                                            std::vector<avtIntegralCurve *> &ics,
                                                            std::vector<std::vector<int> > &ids);
    void                      GetIntegralCurvesFromInitialSeeds(std::vector<avtIntegralCurve *> &ics);
    void                      AddSeedPoint(avtVector &pt,
                                           avtVector &vel,
                                           std::vector<avtIntegralCurve *> &ics);
    void                      AddSeedPoints(std::vector<avtVector> &pts,
                                            std::vector<avtVector> &vels,
                                            std::vector<std::vector<avtIntegralCurve *> > &ics);
    void                      DeleteIntegralCurves(std::vector<int> &icIDs);
    virtual void              CreateIntegralCurveOutput(std::vector<avtIntegralCurve *> &ics) = 0;
    void                      GetTerminatedIntegralCurves(std::vector<avtIntegralCurve *> &ics);

    virtual void              GetPathlineVelocityMeshVariables(avtDataRequest_p &dataRequest, std::string &velocity, std::string &mesh);

    virtual void              PurgeDomain( const int domain, const int timeStep );

    // Helper functions.
    void                      CheckStagger(vtkDataSet *ds, bool &isEdge, bool &isFace);
    int                       DomainToRank(BlockIDType &domain);
    void                      ComputeDomainToRankMapping();
    bool                      OwnDomain(BlockIDType &domain);
    void                      InitializeTimeInformation(int);
    void                      ComputeRankList(const std::vector<int> &domList,
                                              std::vector<int> &ranks,
                                              std::vector<int> &doms );
    double                    GetLengthScale(void);
    bool                      CacheLocators(void);

    avtCellLocator_p          SetupLocator(const BlockIDType &, vtkDataSet *);
    void                      ClearDomainToCellLocatorMap();
    virtual avtIVPField      *GetFieldForDomain(const BlockIDType&, vtkDataSet*);

    void                      SetICAlgorithm();

    void                      InitializeIntervalTree();
    void                      UpdateIntervalTree(int timeSlice);

    // Use this to be able to save the ICs for restart.
    std::vector<avtIntegralCurve *> _ics;
    int                             restart;

    void                      SaveICs( std::vector<avtIntegralCurve *> &ics, int timeStep );
    void                      RestoreICs( std::vector<avtIntegralCurve *> &ics, int timeStep );
    bool                      CheckIfRestart( int &timeStep );
    void                      RestoreICsFilename( int timeStep, char *filename, size_t filenameSize );

    friend class avtICAlgorithm;
};

#endif
