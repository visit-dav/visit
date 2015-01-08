/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
//                        avtIntegralCurveFilter.h                      //
// ************************************************************************* //

#ifndef AVT_INTEGRALCURVE_FILTER_H
#define AVT_INTEGRALCURVE_FILTER_H

/** header file for plugin development */
#include <avtPluginFilter.h>
/** header file for parallel integral curve system via the streamline filter */
#include <avtPICSFilter.h>

/** included attributes for Poincare */
#include <IntegralCurveAttributes.h>

#include <vector>

class avtStateRecorderIntegralCurve;

// ****************************************************************************
// Class: avtIntegralCurveFilter
//
// Purpose:
//     This class inherits from avtStreamlineFilter and its sole job is to
//     implement CreateStreamlineOutput, which it does by creating vtkPolyData.
//
// Notes:  The original implementation of CreateStreamlineOutput was in
//         avtStreamlineFilter and was by Dave Pugmire.  That code was moved to
//         this module by Hank Childs during a later refactoring that allowed
//         the avtStreamlineFilter to be used in more places.
//
// Programmer: Hank Childs (refactoring) / Dave Pugmire (actual code)
// Creation:   December 2, 2008
//
// Modifications:
//
//   Dave Pugmire, Mon Feb  2 14:39:35 EST 2009
//   Moved GetVTKPolyData from avtStreamline to here.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Add tangentsArrayName (for Christoph Garth).
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sun Jun  6 12:21:30 CDT 2010
//   Rename CreateIntegralCurveOutput to reflect the new emphasis in particle 
//   advection, as opposed to streamlines.
//
//   Hank Childs, Sun Jun  6 14:54:08 CDT 2010
//   Convert references from avtStreamline to avtIntegralCurve, the new name
//   for the abstract base type.
//
//   Dave Pugmire, Mon Feb 21 08:22:30 EST 2011
//   Color by correlation distance.
//
// ****************************************************************************

class avtIntegralCurveFilter : public virtual avtPluginFilter,
                               public avtPICSFilter
{
  public:
    // default constructor
                              avtIntegralCurveFilter();
    // virtual destructor
    virtual                  ~avtIntegralCurveFilter();
    // create filter
    static avtFilter   *Create();

    virtual const char       *GetType(void) { return "avtIntegralCurveFilter"; };
    virtual const char       *GetDescription(void) {
      return "Performing Integral Curve"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

    virtual avtIntegralCurve    *CreateIntegralCurve();
    virtual avtIntegralCurve    *CreateIntegralCurve(
                                        const avtIVPSolver* model,
                                        avtIntegralCurve::Direction dir,
                                        const double& t_start,
                                        const avtVector &p_start,
                                        const avtVector &v_start,
                                        long ID);

    void SetTermination(int maxSteps, 
                        bool doDistance, double maxDistance, 
                        bool doTime, double maxTime);

    void SetVelocitySource(const double *v);

    void SetPointSource(const double *p);
    void SetLineSource(const double *p0, const double *p1,
                       int den, bool rand, int seed, int numPts);
    
    void SetPlaneSource(double O[3], double N[3], double U[3], 
                        int den1, int den2, double dist1, double dist2,
                        bool fill, bool rand, int seed, int numPts);
    void SetCircleSource(double O[3], double N[3], double U[3], double r,
                         int den1, int den2,
                         bool fill, bool rand, int seed, int numPts);
    void SetSphereSource(double O[3], double R,
                         int den1, int den2, int den3,
                         bool fill, bool rand, int seed, int numPts);
    void SetBoxSource(double E[6], bool wholeBox,
                      int den1, int den2, int den3,
                      bool fill, bool rand, int seed, int numPts);
    void SetPointListSource(const std::vector<double> &);
    void SetSelectionSource(std::string selectionName,
                            int stride,
                            bool random, int seed, int numPts);
    
    void SetDisplayGeometry(int d);
    void SetDataValue(int, const std::string &var="");
    void SetCropValue(int);
    void SetCorrelationDistanceTol(double angTol,
                                   double minDist, bool doBBox)
    {
        correlationDistanceAngTol = angTol;
        correlationDistanceMinDist = minDist;
        correlationDistanceDoBBox = doBBox;
    }

    void SetVelocitiesForLighting(bool v) { storeVelocitiesForLighting = v; };

    void IssueWarningForMaxStepsTermination(bool v) 
                 { issueWarningForMaxStepsTermination = v; };
    void IssueWarningForStiffness(bool v) 
                 { issueWarningForStiffness = v; };
    void IssueWarningForCriticalPoints(bool v, double speed) 
                 { issueWarningForCriticalPoints = v;
                   criticalPointThreshold = speed; };

    virtual avtIVPField  *GetFieldForDomain(const BlockIDType&, vtkDataSet*);

  protected:
    // Fieldline overides.
    virtual void ExamineContract(avtContract_p in_contract);
    virtual avtContract_p ModifyContract(avtContract_p in_spec);

    virtual void PreExecute(void);
    virtual void PostExecute(void);
    virtual void Execute(void);
 
    virtual void UpdateDataObjectInfo(void);

    virtual void ReportWarnings(std::vector<avtIntegralCurve *> &ics);

    static std::string colorVarArrayName;
    static std::string paramArrayName;
    static std::string opacityArrayName;
    static std::string thetaArrayName;
    static std::string tangentsArrayName;
    static std::string scaleRadiusArrayName;

    void SetCoordinateSystem(int c) {coordinateSystem = c;}

    void GenerateSeedPointsFromPoint(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromLine(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromPlane(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromSphere(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromBox(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromCircle(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromPointList(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromSelection(std::vector<avtVector> &pts);

    unsigned int GenerateAttributeFields() const;

    virtual std::vector<avtVector> GetInitialLocations(void);
    virtual std::vector<avtVector> GetInitialVelocities(void);
    virtual CommunicationPattern   GetCommunicationPattern(void);

    void  CreateIntegralCurveOutput(std::vector<avtIntegralCurve *> &ics);
    float ComputeCorrelationDistance(int idx,
                                     avtStateRecorderIntegralCurve *ic,
                                     double angTol,
                                     double minDist);
    
    void  ProcessVaryTubeRadiusByScalar(std::vector<avtIntegralCurve *> &ics);

    std::string outVarName;

    IntegralCurveAttributes atts;
    bool               needsRecalculation;

    int    sourceType;   
    int    displayGeometry;
    int    dataValue;
    int    cropValue;

    std::string dataVariable;
    int    tubeVariableIndex;
    double correlationDistanceAngTol, correlationDistanceMinDist;
    bool correlationDistanceDoBBox;

    std::vector< std::string > secondaryVariables;

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

    int    coordinateSystem;

    std::string  SeedInfoString() const;
};
#endif
