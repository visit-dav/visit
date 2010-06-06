/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <avtStreamline.h>
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

class vtkVisItCellLocator;
class DomainType;
class avtSLAlgorithm;

#define STREAMLINE_TERMINATE_DISTANCE 0
#define STREAMLINE_TERMINATE_TIME 1
#define STREAMLINE_TERMINATE_STEPS 2
#define STREAMLINE_TERMINATE_INTERSECTIONS 3

#define STREAMLINE_INTEGRATE_DORMAND_PRINCE 0
#define STREAMLINE_INTEGRATE_ADAMS_BASHFORTH 1
#define STREAMLINE_INTEGRATE_M3D_C1_INTEGRATOR 2

#define STREAMLINE_STAGED_LOAD_ONDEMAND 0
#define STREAMLINE_PARALLEL_STATIC_DOMAINS 1
#define STREAMLINE_MASTER_SLAVE 2

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
// ****************************************************************************

class AVTFILTERS_API avtPICSFilter : 
    virtual public avtDatasetOnDemandFilter,
    virtual public avtDatasetToDatasetFilter
{
  public:
                              avtPICSFilter();
    virtual                  ~avtPICSFilter();

    virtual const char       *GetType(void)  { return "avtPICSFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Advecting particles"; };

    virtual avtStreamline          *CreateIntegralCurve() = 0;
    virtual avtStreamline          *CreateIntegralCurve(
                                        const avtIVPSolver* model, 
                                        const double& t_start,
                                        const avtVector &p_start, int ID) = 0;
    virtual std::vector<avtVector>  GetInitialLocations() = 0;

    // Methods to set the filter's attributes.
    void                      SetMaxStepLength(double len);
    void                      SetTermination(int type, double term);
    void                      SetPathlines(bool pathlines, double time0=0.0);
    void                      SetIntegrationType(int algo);
    void                      SetStreamlineAlgorithm(int algo, int maxCnt,
                                                     int domainCache,
                                                     int workGrpSz);
    void                      SetTolerances(double reltol, double abstol);

    void                      SetIntegrationDirection(int dir);

    void                      InitializeLocators(void);

  protected:
    double maxStepLength;
    double relTol;
    double absTol;
    avtIVPSolver::TerminateType terminationType;
    int integrationType;
    double termination;
    int    integrationDirection;
    int    dataSpatialDimension;
    avtSLAlgorithm *slAlgo;

    avtContract_p lastContract;

    std::vector<std::vector<double> > domainTimeIntervals;
    std::string pathlineVar, pathlineNextTimeVar;
    bool   doPathlines;
    double seedTime0;
    int    seedTimeStep0;

    avtIntervalTree *intervalTree;
    bool             specifyPoint;
    avtIVPSolver *solver;

    int numDomains, numTimeSteps, cacheQLen;
    std::vector<int> domainToRank;
    std::vector<vtkDataSet*>dataSets;
    std::map<DomainType, vtkVisItCellLocator*> domainToCellLocatorMap;

    std::vector<double> pointList;

    // Data retrieved from contract
    int activeTimeStep;

    //Timings helpers.
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

    void                      AdvectParticle(avtStreamline *sl, int maxSteps=-1);
    void                      IntegrateDomain(avtStreamline *sl, 
                                              vtkDataSet *ds,
                                              double *extents,
                                              int maxSteps=-1);
    virtual vtkDataSet        *GetDomain(const DomainType &, double = 0.0, double = 0.0, double = 0.0);
    virtual int               GetTimeStep(double &t) const;
    virtual bool              DomainLoaded(DomainType &) const;

    void                      SetZToZero(vtkPolyData *) const;

    int                       GetNextCurveID(){ int id = MaxID; MaxID++; return id;}
    void                      CreateIntegralCurvesFromSeeds(std::vector<avtVector> &pts,
                                                         std::vector<avtStreamline *> &streamlines,
                                                         std::vector<std::vector<int> > &ids);
    void                      GetIntegralCurvesFromInitialSeeds(std::vector<avtStreamline *> &sls);
    void                      AddSeedpoints(std::vector<avtVector> &pts,
                                            std::vector<std::vector<int> > &ids);
    void                      DeleteIntegralCurves(std::vector<int> &slIDs);
    virtual void              CreateIntegralCurveOutput(vector<avtStreamline *> &streamlines)
                                                    = 0;
    void                      GetTerminatedIntegralCurves(vector<avtStreamline *> &sls);

    // Helper functions.
    bool                      PointInDomain(avtVector &pt, DomainType &domain);
    int                       DomainToRank(DomainType &domain);
    void                      ComputeDomainToRankMapping();
    bool                      OwnDomain(DomainType &domain);
    void                      SetDomain(avtStreamline *sl);
    void                      Initialize();
    void                      ComputeRankList(const std::vector<int> &domList, 
                                              std::vector<int> &ranks, 
                                              std::vector<int> &doms );
    vtkVisItCellLocator      *SetupLocator(const DomainType &, vtkDataSet *);
    
    friend class avtSLAlgorithm;
};


#endif


