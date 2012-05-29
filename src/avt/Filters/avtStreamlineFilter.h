/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//                           avtStreamlineFilter.h                           //
// ************************************************************************* //

#ifndef AVT_STREAMLINE_FILTER_H
#define AVT_STREAMLINE_FILTER_H

#include <avtPICSFilter.h>

class vtkTubeFilter;
class vtkPolyData;
class vtkRibbonFilter;
class vtkAppendPolyData;

#define STREAMLINE_SOURCE_POINT      0
#define STREAMLINE_SOURCE_POINT_LIST 1
#define STREAMLINE_SOURCE_LINE       2
#define STREAMLINE_SOURCE_CIRCLE     3
#define STREAMLINE_SOURCE_PLANE      4
#define STREAMLINE_SOURCE_SPHERE     5
#define STREAMLINE_SOURCE_BOX        6
#define STREAMLINE_SOURCE_SELECTION  7

#define STREAMLINE_COLOR_SOLID       0
#define STREAMLINE_COLOR_SPEED       1
#define STREAMLINE_COLOR_VORTICITY   2
#define STREAMLINE_COLOR_ARCLENGTH   3
#define STREAMLINE_COLOR_TIME        4
#define STREAMLINE_COLOR_ID          5
#define STREAMLINE_COLOR_VARIABLE    6
#define STREAMLINE_CORRELATION_DISTANCE 7

#define STREAMLINE_DISPLAY_LINES     0
#define STREAMLINE_DISPLAY_TUBES     1
#define STREAMLINE_DISPLAY_RIBBONS   2


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
//   Make BlockIDType a const reference.
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
//   Hank Childs, Sat Feb 20 05:12:45 PST 2010
//   Add methods for initializing locators.
//
//   Allen Sanderson, Sun Mar  7 16:18:46 PST 2010
//   Re-order define's for source type to match recent changes in the 
//   streamline attributes.
//
//   Dave Pugmire, Tue May 25 10:15:35 EDT 2010
//   Added DeleteStreamlines method.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sat Jun  5 16:21:27 CDT 2010
//   Add virtual method CreateIntegralCurve.
//
//   Hank Childs, Sat Jun  5 15:02:43 PDT 2010
//   Separated out much of the base infrastructure into the avtPICSFilter.
//
//   Hank Childs, Tue Jun  8 09:30:45 CDT 2010
//   Add method GetCommunicationPattern.
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
//   Dave Pugmire, Fri Jun 11 15:12:04 EDT 2010
//   Remove seed densities.
//
//   Hank Childs, Mon Oct  4 14:50:01 PDT 2010
//   Specify termination type at avtStreamlineFilter, not avtPICSFilter.
//   Add data member for reference type for display.
//
//   Dave Pugmire, Fri Nov  5 15:38:33 EDT 2010
//   Add GenerateAttributeFields method.
//
//   Hank Childs, Sun Dec  5 10:43:57 PST 2010
//   Add new data members for warnings.
//
//   Dave Pugmire, Fri Jan 14 11:10:44 EST 2011
//   Set default communication pattern to RestoreSequenceAssembleUniformly.
//
// ****************************************************************************

class AVTFILTERS_API avtStreamlineFilter : virtual public avtPICSFilter
{
  public:
                              avtStreamlineFilter();
    virtual                  ~avtStreamlineFilter();

    virtual const char       *GetType(void)  { return "avtStreamlineFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Creating streamlines"; };

    virtual avtIntegralCurve    *CreateIntegralCurve();
    virtual avtIntegralCurve    *CreateIntegralCurve(
                                        const avtIVPSolver* model,
                                        avtIntegralCurve::Direction dir,
                                        const double& t_start,
                                        const avtVector &p_start,
                                        const avtVector &v_start,
                                        long ID);

    void                      SetTermination(int maxSteps, 
                                             bool doDistance, double maxDistance, 
                                             bool doTime, double maxTime);

    void                      SetVelocitySource(const double *v);

    void                      SetPointSource(const double *p);
    void                      SetLineSource(const double *p0, const double *p1,
                                            int den, bool rand, int seed, int numPts);
    
    void                      SetPlaneSource(double O[3], double N[3], double U[3], 
                                             int den1, int den2, double dist1, double dist2,
                                             bool fill, bool rand, int seed, int numPts);
    void                      SetCircleSource(double O[3], double N[3], double U[3], double r,
                                              int den1, int den2,
                                              bool fill, bool rand, int seed, int numPts);
    void                      SetSphereSource(double O[3], double R,
                                              int den1, int den2, int den3,
                                              bool fill, bool rand, int seed, int numPts);
    void                      SetBoxSource(double E[6], bool wholeBox,
                                           int den1, int den2, int den3,
                                           bool fill, bool rand, int seed, int numPts);
    void                      SetPointListSource(const std::vector<double> &);
    void                      SetSelectionSource(std::string selectionName,
                                                 int stride,
                                                 bool random, int seed, int numPts);

    void                      SetDisplayMethod(int d);
    void                      SetColoringMethod(int, const std::string &var="");
    void                      SetColorByCorrelationDistanceTol(double angTol, double minDist, bool doBBox)
                              {
                                  correlationDistanceAngTol = angTol;
                                  correlationDistanceMinDist = minDist;
                                  correlationDistanceDoBBox = doBBox;
                              }

    void                      SetVelocitiesForLighting(bool v) { storeVelocitiesForLighting = v; };
    void                      SetOpacityVariable(const std::string &var);
    void                      SetScaleTubeRadiusVariable(const std::string &var);

    void                      SetReferenceTypeForDisplay(int d) 
                                               { referenceTypeForDisplay = d; };
    void                      IssueWarningForMaxStepsTermination(bool v) 
                                      { issueWarningForMaxStepsTermination = v; };
    void                      IssueWarningForStiffness(bool v) 
                                      { issueWarningForStiffness = v; };
    void                      IssueWarningForCriticalPoints(bool v, double speed) 
                                      { issueWarningForCriticalPoints = v;
                                        criticalPointThreshold = speed; };

    virtual avtIVPField      *GetFieldForDomain(const BlockIDType&, vtkDataSet*);

    virtual void              PostExecute(void);
    virtual void              UpdateDataObjectInfo(void);
    virtual avtContract_p     ModifyContract(avtContract_p);

  protected:
    int    sourceType;   
    int    displayMethod;
    int    coloringMethod;
    int    referenceTypeForDisplay;
    std::string coloringVariable, opacityVariable, scaleTubeRadiusVariable;
    double correlationDistanceAngTol, correlationDistanceMinDist;
    bool correlationDistanceDoBBox;

    int      maxSteps;
    bool     doDistance;
    double   maxDistance;
    bool     doTime;
    double   maxTime;

    // Various starting locations for streamlines.
    std::vector<avtVector> pointList;
    avtVector points[2];
    avtVector vectors[2];
    std::vector<double> listOfPoints;
    int       numSamplePoints;
    int       sampleDensity[3];
    double    sampleDistance[3];
    bool      randomSamples;
    int       randomSeed;
    bool      fill, useBBox;
    std::string sourceSelection;
    bool      storeVelocitiesForLighting;

    bool      issueWarningForMaxStepsTermination;
    bool      issueWarningForStiffness;
    bool      issueWarningForCriticalPoints;
    double    criticalPointThreshold;

    avtVector seedVelocity;

    std::string             SeedInfoString() const;

    void                      GenerateSeedPointsFromPoint(std::vector<avtVector> &pts);
    void                      GenerateSeedPointsFromLine(std::vector<avtVector> &pts);
    void                      GenerateSeedPointsFromPlane(std::vector<avtVector> &pts);
    void                      GenerateSeedPointsFromSphere(std::vector<avtVector> &pts);
    void                      GenerateSeedPointsFromBox(std::vector<avtVector> &pts);
    void                      GenerateSeedPointsFromCircle(std::vector<avtVector> &pts);
    void                      GenerateSeedPointsFromPointList(std::vector<avtVector> &pts);
    void                      GenerateSeedPointsFromSelection(std::vector<avtVector> &pts);

    unsigned char             GenerateAttributeFields() const;

    virtual std::vector<avtVector> GetInitialLocations(void);
    virtual std::vector<avtVector> GetInitialVelocities(void);
    virtual CommunicationPattern   GetCommunicationPattern(void);
};

#endif

