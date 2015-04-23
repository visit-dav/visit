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
//                        avtLimitCycleFilter.h                      //
// ************************************************************************* //

#ifndef AVT_LIMITCYCLE_FILTER_H
#define AVT_LIMITCYCLE_FILTER_H

/** header file for plugin development */
#include <avtPluginFilter.h>
/** header file for parallel integral curve system via the streamline filter */
#include <avtPICSFilter.h>

#include <vtkPlane.h>


/** included attributes for Limit Cycle */
#include <LimitCycleAttributes.h>

#include <vector>

class avtStateRecorderIntegralCurve;

// ****************************************************************************
// Class: avtLimitCycleFilter
//
// Purpose:
//     This class inherits from avtPICSFilter.
//
// Programmer: Hank Childs (refactoring) / Dave Pugmire (actual code)
// Creation:   December 2, 2008
//
// Modifications:
//
// ****************************************************************************

class avtLimitCycleFilter : public virtual avtPluginFilter,
                            public virtual avtPICSFilter
{
  public:
    // default constructor
                              avtLimitCycleFilter();
    // virtual destructor
    virtual                  ~avtLimitCycleFilter();
    // create filter
    static avtFilter   *Create();

    virtual const char       *GetType(void) { return "avtLimitCycleFilter"; };
    virtual const char       *GetDescription(void) {
      return "Performing Limit Cycle Search"; };

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
    
    void SetLineSource(const double *p0, const double *p1,
                       int den, bool rand, int seed, int numPts);
    
    void SetPlaneSource(double O[3], double N[3], double U[3], 
                        int den1, int den2, double dist1, double dist2,
                        bool fill, bool rand, int seed, int numPts);

    void SetDataValue(int, const std::string &var="");
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
    void IssueWarningForStepsize(bool v) 
                 { issueWarningForStepsize = v; };
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
    virtual bool ContinueExecute(void);

    virtual void  GetIntegralCurvePoints(std::vector<avtIntegralCurve *> &ics);
 
    virtual void UpdateDataObjectInfo(void);

    virtual void ReportWarnings(std::vector<avtIntegralCurve *> &ics);

    static std::string colorVarArrayName;
    static std::string paramArrayName;
    static std::string opacityArrayName;
    static std::string thetaArrayName;
    static std::string tangentsArrayName;
    static std::string scaleRadiusArrayName;

    void SetCoordinateSystem(int c) {coordinateSystem = c;}

    void GenerateSeedPointsFromLine(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromPlane(std::vector<avtVector> &pts);

    unsigned int GenerateAttributeFields() const;

    virtual std::vector<avtVector> GetInitialLocations(void);
    virtual std::vector<avtVector> GetInitialVelocities(void);
    virtual CommunicationPattern   GetCommunicationPattern(void);

    void  CreateIntegralCurveOutput(std::vector<avtIntegralCurve *> &ics);
    float ComputeCorrelationDistance(int idx,
                                     avtStateRecorderIntegralCurve *ic,
                                     double angTol,
                                     double minDist);
    
    void SetIntersectionCriteria();
    avtVector GetIntersectingPoint( avtVector pt0, avtVector pt1 );


    std::string outVarName;

    LimitCycleAttributes atts;
    bool               needsRecalculation;

    int    sourceType;   
    int    dataValue;

    std::string dataVariable;
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
    bool      issueWarningForStepsize;
    bool      issueWarningForStiffness;
    bool      issueWarningForCriticalPoints;
    double    criticalPointThreshold;

    avtVector seedVelocity;

    int    coordinateSystem;

    avtVector planePt, planeN;
    vtkPlane *intPlane; 
    int maxIntersections;

    std::string  SeedInfoString() const;
};

#endif
