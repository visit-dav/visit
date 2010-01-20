/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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
//                           avtStreamlineFilter.h                           //
// ************************************************************************* //

#ifndef AVT_STREAMLINE_FILTER_H
#define AVT_STREAMLINE_FILTER_H

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
class vtkVisItStreamLine;
class vtkTubeFilter;
class vtkPolyData;
class vtkRibbonFilter;
class vtkAppendPolyData;
class avtStreamlineWrapper;
class DomainType;
class avtSLAlgorithm;

#define STREAMLINE_SOURCE_POINT      0
#define STREAMLINE_SOURCE_LINE       1
#define STREAMLINE_SOURCE_PLANE      2
#define STREAMLINE_SOURCE_SPHERE     3
#define STREAMLINE_SOURCE_BOX        4
#define STREAMLINE_SOURCE_CIRCLE     5
#define STREAMLINE_SOURCE_POINT_LIST 6

#define STREAMLINE_COLOR_SOLID       0
#define STREAMLINE_COLOR_SPEED       1
#define STREAMLINE_COLOR_VORTICITY   2
#define STREAMLINE_COLOR_ARCLENGTH   3
#define STREAMLINE_COLOR_TIME        4
#define STREAMLINE_COLOR_ID          5
#define STREAMLINE_COLOR_VARIABLE    6

#define STREAMLINE_DISPLAY_LINES     0
#define STREAMLINE_DISPLAY_TUBES     1
#define STREAMLINE_DISPLAY_RIBBONS   2

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
// Class: avtStreamlineFilter
//
// Purpose:
//    This operator is the implied operator associated with a Streamline plot.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 4 15:32:05 PST 2002
//
// Modifications:
//
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
//   Dave Pugmire, Thu Jan 15 12:01:30 EST 2009
//   Add case5 to MasterSlave algorithm.
//
//   Dave Pugmire, Tue Feb  3 10:58:56 EST 2009
//   Major refactor of the streamline code.  Moved all the streamline
//   algorithm code into different classes.
//
//   Dave Pugmire, Thu Feb  5 12:23:33 EST 2009
//   Add workGroupSize for masterSlave algorithm.
//
//   Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//   Reworked the termination code. Added a type enum and value.
//
//   Dave Pugmire, Mon Feb 23 13:40:28 EST 2009
//   Initial IO time and domain load counter.
//
//   Dave Pugmire (on behalf of Hank Childs), Tue Feb 24 09:39:17 EST 2009
//   Initial implemenation of pathlines.
//
//   Dave Pugmire, Fri Feb 27 15:13:55 EST 2009
//   Removed unused normalize expression string.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Mon Mar 16 15:05:14 EDT 2009
//   Make DomainType a const reference.
//
//   Hank Childs, Sun Mar 22 11:30:40 CDT 2009
//   Added specifyPoint data member.
//
//   Dave Pugmire, Tue Mar 31 17:01:17 EDT 2009
//   Added seedTime0 and seedTimeStep0 member data.
//
//   Gunther H. Weber, Thu Apr  2 10:47:01 PDT 2009
//   Added activeTimeStep and ExamineContract() to retrieve
//   currently active time step
//
//   Dave Pugmire, Fri Apr  3 09:18:03 EDT 2009
//   Add SeedInfoString method to report seed information.
//
//   Hank Childs, Mon Apr 20 08:40:07 PDT 2009
//   Change in inheritance to reflect changes in base class.
//
//   Hank Childs, Sun May  3 12:32:13 CDT 2009
//   Added point list source type.
// 
//   Dave Pugmire, Tue Aug 11 10:25:45 EDT 2009
//   Add new termination criterion: Number of intersections with an object.
//
//   Dave Pugmire, Tue Aug 18 09:10:49 EDT 2009
//   Add ability to restart integration of streamlines.
//
//   Dave Pugmire, Thu Dec  3 13:28:08 EST 2009
//   Added ID member data and new seedpoint generation methods.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Remove radius and showStart members, add circle seed source.
//
// ****************************************************************************

class AVTFILTERS_API avtStreamlineFilter : 
    virtual public avtDatasetOnDemandFilter,
    virtual public avtDatasetToDatasetFilter
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
    void                      SetIntersectionObject(vtkObject *obj);
    void                      SetPathlines(bool pathlines, double time0=0.0);
    void                      SetIntegrationType(int algo);
    void                      SetStreamlineAlgorithm(int algo, int maxCnt,
                                                     int domainCache,
                                                     int workGrpSz);
    void                      SetTolerances(double reltol, double abstol);

    void                      SetPointSource(double pt[3]);
    void                      SetLineSource(double pt[3], double pt2[3]);
    void                      SetPlaneSource(double O[3], double N[3],
                                             double U[3], double R);
    void                      SetSphereSource(double O[3], double R);
    void                      SetPointListSource(const std::vector<double> &);
    void                      SetBoxSource(double E[6]);
    void                      SetUseWholeBox(bool b) { useWholeBox = b; };

    void                      SetDisplayMethod(int d);
    void                      SetPointDensity(int den);
    void                      SetStreamlineDirection(int dir);
    void                      SetColoringMethod(int, const std::string &var="");
    void                      SetOpacityVariable(const std::string &var);

  protected:
    int    sourceType;   
    double maxStepLength;
    double relTol;
    double absTol;
    avtIVPSolver::TerminateType terminationType;
    int integrationType;
    double termination;
    int    displayMethod;
    int    pointDensity1, pointDensity2, pointDensity3;
    int    streamlineDirection;
    int    coloringMethod;
    std::string coloringVariable, opacityVariable;
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

    // Various starting locations for streamlines.
    std::string             SeedInfoString() const;
    double pointSource[3];
    double lineStart[3], lineEnd[3];
    double planeOrigin[3], planeNormal[3], planeUpAxis[3], planeRadius;
    double sphereOrigin[3], sphereRadius;
    double boxExtents[6];
    std::vector<double> pointList;
    bool   useWholeBox;

    vtkObject *intersectObj;

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
    virtual void              UpdateDataObjectInfo(void);
    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
    virtual avtContract_p     ModifyContract(avtContract_p);
    virtual void              ExamineContract(avtContract_p);
    virtual bool              CheckOnDemandViability(void);

    void                      IntegrateStreamline(avtStreamlineWrapper *slSeg, int maxSteps=-1);
    avtIVPSolver::Result      IntegrateDomain(avtStreamlineWrapper *slSeg, 
                                              vtkDataSet *ds,
                                              double *extents,
                                              int maxSteps=-1);
    virtual vtkDataSet        *GetDomain(const DomainType &, double = 0.0, double = 0.0, double = 0.0);
    virtual int               GetTimeStep(double &t) const;
    virtual bool              DomainLoaded(DomainType &) const;

    void                      SetZToZero(vtkPolyData *) const;

    void                      GenerateSeedPointsFromPoint(std::vector<avtVector> &pts);
    void                      GenerateSeedPointsFromLine(std::vector<avtVector> &pts);
    void                      GenerateSeedPointsFromPlane(std::vector<avtVector> &pts);
    void                      GenerateSeedPointsFromSphere(std::vector<avtVector> &pts);
    void                      GenerateSeedPointsFromBox(std::vector<avtVector> &pts);
    void                      GenerateSeedPointsFromCircle(std::vector<avtVector> &pts);
    void                      GenerateSeedPointsFromPointList(std::vector<avtVector> &pts);

    int                       GetNextStreamlineID(){ int id = MaxID; MaxID++; return id;}
    void                      CreateStreamlinesFromSeeds(std::vector<avtVector> &pts,
                                                         std::vector<avtStreamlineWrapper *> &streamlines,
                                                         std::vector<std::vector<int> > &ids);
    void                      GetStreamlinesFromInitialSeeds(std::vector<avtStreamlineWrapper *> &sls);
    void                      AddSeedpoints(std::vector<avtVector> &pts,
                                            std::vector<std::vector<int> > &ids);
    virtual void              CreateStreamlineOutput( 
                                                     vector<avtStreamlineWrapper *> &streamlines)
                                                    = 0;
    void                      GetTerminatedStreamlines(vector<avtStreamlineWrapper *> &sls);

    // Helper functions.
    bool                      PointInDomain(avtVector &pt, DomainType &domain);
    int                       DomainToRank(DomainType &domain);
    void                      ComputeDomainToRankMapping();
    bool                      OwnDomain(DomainType &domain);
    void                      SetDomain(avtStreamlineWrapper *slSeg);
    void                      Initialize();
    void                      ComputeRankList(const std::vector<int> &domList, 
                                              std::vector<int> &ranks, 
                                              std::vector<int> &doms );
    
    friend class avtSLAlgorithm;
};


#endif


