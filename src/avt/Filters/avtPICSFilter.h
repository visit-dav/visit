/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

class DomainType;
class avtICAlgorithm;

#define STREAMLINE_TERMINATE_DISTANCE 0
#define STREAMLINE_TERMINATE_TIME 1
#define STREAMLINE_TERMINATE_STEPS 2
#define STREAMLINE_TERMINATE_INTERSECTIONS 3

#define STREAMLINE_INTEGRATE_DORMAND_PRINCE 0
#define STREAMLINE_INTEGRATE_ADAMS_BASHFORTH 1
#define STREAMLINE_INTEGRATE_M3D_C1_INTEGRATOR 2
#define STREAMLINE_INTEGRATE_NIMROD_INTEGRATOR 3

#define STREAMLINE_SERIAL                0
#define STREAMLINE_PARALLEL_OVER_DOMAINS 1
#define STREAMLINE_PARALLEL_MASTER_SLAVE 2
#define STREAMLINE_VISIT_SELECTS         3


#define STREAMLINE_PARALLEL_COMM_DOMAINS 4


// ****************************************************************************
// Class: avtPICSFilter
//
// Purpose:
//    PICS = Parallel integral curve system.  An abstract type.  The purpose
//    of this filter is to define an AVT filter that can do parallel
//    particle advection.  How to analyze those particles, termination 
//    criteria, etc, are left to the derived types.  Examples of derived
//    types are for streamlines and poincare analysis.
//
// Programmer: Hank Childs (extracted base class from avtStreamlineFilter)
// Creation:   June 5, 2010
//
// Modifications:
//
//   Hank Childs, Sun Jun  6 15:26:15 CDT 2010
//   Rename all methods and data members to reflect integral curves, not 
//   streamlines.
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
// ****************************************************************************

class AVTFILTERS_API avtPICSFilter : 
    virtual public avtDatasetOnDemandFilter,
    virtual public avtDatasetToDatasetFilter
{
  public:
    enum CommunicationPattern
    {
        RestoreSequence = 0,
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
                                        const avtVector &p_start, long ID) = 0;

    virtual std::vector<avtVector>  GetInitialLocations() = 0;
    virtual CommunicationPattern    GetCommunicationPattern() = 0;

    // Methods to set the filter's attributes.
    void                      SetMaxStepLength(double len);
    void                      SetPathlines(bool pathlines, bool overrideTime, double time0, int _pathlineCMFE);
    void                      SetIntegrationType(int algo);
    void                      SetStreamlineAlgorithm(int algo, int maxCnt,
                                                     int domainCache,
                                                     int workGrpSz);
    void                      SetTolerances(double reltol, double abstol, bool isFraction);

    void                      SetIntegrationDirection(int dir);

    void                      InitializeLocators(void);
    void                      UpdateProgress(int amt, int total)
                                 { avtFilter::UpdateProgress(amt, total); };

    virtual void              ReleaseData(void);
    virtual void              UpdateDataObjectInfo(void);

    void                      ConvertToCartesian(bool val) { convertToCartesian = val; };


  protected:
    bool   emptyDataset;
    double maxStepLength;
    double relTol;
    double absTol;
    bool   absTolIsFraction;
    int    integrationType;
    int    integrationDirection;
    int    dataSpatialDimension;
    bool   convertToCartesian;

    avtICAlgorithm *icAlgo;

    avtContract_p lastContract;

    std::vector<std::vector<double> > domainTimeIntervals;
    bool   doPathlines;
    bool   pathlineOverrideTime;
    int    pathlineCMFE;
    double seedTime0;
    int    seedTimeStep0;

    avtIntervalTree *intervalTree;
    bool             specifyPoint;
    avtIVPSolver *solver;

    int numDomains, numTimeSteps, cacheQLen;
    std::vector<int> domainToRank;
    std::vector<vtkDataSet*>dataSets;
    std::map<DomainType, avtCellLocator_p> domainToCellLocatorMap;

    std::vector<double> pointList;

    // Data retrieved from contract
    int activeTimeStep;

    // Time slice currently read and being processed
    int curTimeSlice;

    // Timings helpers.
    int                       numSeedPts, MaxID;
    int                       method;
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

    void                      AdvectParticle(avtIntegralCurve *ic, int maxSteps=-1);
    void                      AdvectParticle(avtIntegralCurve *ic, vtkDataSet *ds, int maxSteps=-1);
    void                      IntegrateDomain(avtIntegralCurve *ic, 
                                              vtkDataSet *ds,
                                              double *extents,
                                              int maxSteps=-1);
    virtual vtkDataSet        *GetDomain(const DomainType &, double = 0.0, double = 0.0, double = 0.0);
    bool                      LoadNextTimeSlice();
    virtual int               GetTimeStep(double t) const;
    virtual bool              DomainLoaded(DomainType &) const;

    int                       GetNextCurveID(){ int id = MaxID; MaxID++; return id;}
    void                      CreateIntegralCurvesFromSeeds(std::vector<avtVector> &pts,
                                                         std::vector<avtIntegralCurve *> &ics,
                                                         std::vector<std::vector<int> > &ids);
    void                      GetIntegralCurvesFromInitialSeeds(std::vector<avtIntegralCurve *> &ics);
    void                      AddSeedpoints(std::vector<avtVector> &pts,
                                            std::vector<std::vector<int> > &ids);
    void                      DeleteIntegralCurves(std::vector<int> &icIDs);
    virtual void              CreateIntegralCurveOutput(vector<avtIntegralCurve *> &ics) = 0;
    void                      GetTerminatedIntegralCurves(vector<avtIntegralCurve *> &ics);

    virtual void              GetPathlineVelocityMeshVariables(avtDataRequest_p &dataRequest, std::string &velocity, std::string &mesh);

    // Helper functions.
    bool                      PointInDomain(avtVector &pt, DomainType &domain);
    int                       DomainToRank(DomainType &domain);
    void                      ComputeDomainToRankMapping();
    bool                      OwnDomain(DomainType &domain);
    void                      SetDomain(avtIntegralCurve *ic);
    void                      Initialize();
    void                      ComputeRankList(const std::vector<int> &domList,
                                              std::vector<int> &ranks,
                                              std::vector<int> &doms );
    double                    GetLengthScale(void);
    bool                      CacheLocators(void);

    avtCellLocator_p          SetupLocator(const DomainType &, vtkDataSet *);
    virtual avtIVPField      *GetFieldForDomain(const DomainType&, vtkDataSet*);

    friend class avtICAlgorithm;
};

#endif

