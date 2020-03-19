// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtIntegralCurveFilter.h                      //
// ************************************************************************* //

#ifndef AVT_INTEGRALCURVE_FILTER_H
#define AVT_INTEGRALCURVE_FILTER_H

/** header file for plugin development */
#include <avtPluginFilter.h>
/** header file for parallel integral curve system via the PICS filter */
#include <avtPICSFilter.h>

/** included attributes for Poincare */
#include <IntegralCurveAttributes.h>

#include <vector>

class avtStateRecorderIntegralCurve;

// ****************************************************************************
// Class: avtIntegralCurveFilter
//
// Purpose:
//     This class inherits from avtPICSFilter and its sole job is to
//     implement CreateIntegralCurveOutput, which it does by creating
//     vtkPolyData.
//
// Programmer: Hank Childs (refactoring) / Dave Pugmire (actual code)
// Creation:   December 2, 2008
//
// Modifications:
//
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Add tangentsArrayName (for Christoph Garth).
//
//   Dave Pugmire, Mon Feb 21 08:22:30 EST 2011
//   Color by correlation distance.
//
// ****************************************************************************

class avtIntegralCurveFilter : public virtual avtPluginFilter,
                               public virtual avtPICSFilter
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

    //turn off on demand processing, I cannot handle this functionality yet..
    virtual bool OperatingOnDemand(void) const { return false; }
    virtual bool CheckOnDemandViability(void) { return false; }

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

    virtual bool             GetAllSeedsSentToAllProcs() { return true; };

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
    void SetFieldDataSource();
    
    void SetDisplayGeometry(int d);
    void SetDataValue(int, const std::string &var="");
    void SetCleanupMethod(int, double);
    void SetCropValue(int);
    void SetCorrelationDistanceTol(double angTol,
                                   double minDist, bool doBBox)
    {
        correlationDistanceAngTol = angTol;
        correlationDistanceMinDist = minDist;
        correlationDistanceDoBBox = doBBox;
    }

    void SetVelocitiesForLighting(bool v) { storeVelocitiesForLighting = v; };

    void IssueWarningForAdvection(bool v) 
                 { issueWarningForAdvection = v; };
    void IssueWarningForBoundary(bool v) 
                 { issueWarningForBoundary = v; };
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
 
    virtual void UpdateDataObjectInfo(void);

    virtual void ReportWarnings(std::vector<avtIntegralCurve *> &ics);

    static std::string colorVarArrayName;
    static std::string thetaArrayName;
    static std::string tangentsArrayName;
    static std::string normalsArrayName;

    void SetCoordinateSystem(int c) {coordinateSystem = c;}

    void GenerateSeedPointsFromPoint(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromLine(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromPlane(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromSphere(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromBox(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromCircle(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromPointList(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromSelection(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromFieldData(std::vector<avtVector> &pts);
    void GenerateSeedPointsFromFieldData(avtDataTree_p inDT);
    void GenerateSeedPointsFromFieldData(vtkDataSet *in_ds);

    unsigned int GenerateAttributeFields() const;

    virtual std::vector<avtVector> GetInitialLocations(void);
    virtual std::vector<avtVector> GetInitialVelocities(void);
    virtual CommunicationPattern   GetCommunicationPattern(void);

    void  CreateIntegralCurveOutput(std::vector<avtIntegralCurve *> &ics);
    float ComputeCorrelationDistance(int idx,
                                     avtStateRecorderIntegralCurve *ic,
                                     double angTol,
                                     double minDist);
    
    unsigned int CheckForClosedCurve(avtStateRecorderIntegralCurve *ic);

    void  ProcessVaryTubeRadiusByScalar(std::vector<avtIntegralCurve *> &ics);

    std::string outVarName;

    IntegralCurveAttributes atts;
    bool               needsRecalculation;

    int    sourceType;   
    int    displayGeometry;
    int    dataValue;
    int    cleanupMethod;
    double cleanupThreshold;
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

    double   absMaxTime;

    // Various starting locations for integral curves.
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

    bool      issueWarningForAdvection;
    bool      issueWarningForBoundary;
    bool      issueWarningForMaxStepsTermination;
    bool      issueWarningForStepsize;
    bool      issueWarningForStiffness;
    bool      issueWarningForCriticalPoints;
    double    criticalPointThreshold;

    avtVector seedVelocity;

    int    coordinateSystem;

    std::string  SeedInfoString() const;
};

#endif
