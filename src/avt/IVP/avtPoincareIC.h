// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtPoincareIC.h                              //
// ************************************************************************* //

#ifndef AVT_POINCARE_IC_H
#define AVT_POINCARE_IC_H

#include <avtStateRecorderIntegralCurve.h>
#include <map>

class vtkObject;

// ****************************************************************************
//  Class: avtPoincareIC
//
//  Purpose:
//      A derived type of avtStateRecorderIntegralCurve.  This class 
//      decides how to terminate a Poncare integral curve.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2010
//
//  Modifications:
//
//    Hank Childs, Sun Dec  5 10:18:13 PST 2010
//    Add avtIVPField to CheckForTermination.
//
//   David Camp, Wed Mar  7 10:43:07 PST 2012
//   Added a Serialize flag to the arguments. This is to support the restore
//   ICs code.
//
// ****************************************************************************

#ifndef POINCARE_FIELDLINE_PROPERTIES_H
#define POINCARE_FIELDLINE_PROPERTIES_H

class avtPoincareIC;

class WindingPair {

public:
  WindingPair( unsigned int t, unsigned int p )
  {
    toroidal = t;
    poloidal = p;
  };

  bool operator < (const WindingPair &i) const {return toroidal < i.toroidal;}
  bool operator > (const WindingPair &i) const {return toroidal > i.toroidal;}

  unsigned int toroidal;
  unsigned int poloidal;
};


class FieldlineProperties {

public:

  FieldlineProperties()
  {
    type = FieldlineProperties::UNKNOWN_TYPE;

    analysisMethod = FieldlineProperties::DEFAULT_METHOD;
    searchState    = FieldlineProperties::NO_SEARCH;
    analysisState  = FieldlineProperties::UNKNOWN_ANALYSIS;

    source = FieldlineProperties::UNKNOWN_TYPE;
    
    iteration = 0;

    safetyFactor = 0;

    toroidalWinding = 0;
    poloidalWinding = 0;

    toroidalWindingP = 0;
    poloidalWindingP = 0;

    toroidalResonance = 0;
    poloidalResonance = 0;

    windingGroupOffset = 0;
    islands = 0;
    islandGroups = 0;

    nnodes  = 0;
    
    rationalSurfaceTolerance = 0;

    maxPunctures      = 0;
    numPunctures      = 0;
    nPuncturesNeeded  = 0;

    parentOPointIC = 0;
    childOPointIC = 0;
  };

enum FieldlineType { UNKNOWN_TYPE  = 0,

                     PERIODIC = 0x0010,
                     RATIONAL = 0x0010,

                     O_POINT  = 0x0011,
                     X_POINT  = 0x0012,

                     
                     QUASI_PERIODIC = 0x0060,
                     IRRATIONAL     = 0x0060,

                     FLUX_SURFACE   = 0x0020,

                     ISLAND_CHAIN                    = 0x0040,

                     ISLAND_PRIMARY_CHAIN            = 0x0040,
                     ISLAND_SECONDARY_CHAIN          = 0x0042,

                     ISLAND_PRIMARY_SECONDARY_AXIS   = 0x0041,
                     ISLAND_SECONDARY_SECONDARY_AXIS = 0x0043,
                     
                     CHAOTIC = 30 };
  
enum AnalysisMethod { UNKNOWN_METHOD = 0,
                      DEFAULT_METHOD = 1,

                      RATIONAL_SEARCH   = 10,
                      RATIONAL_MINIMIZE };

enum AnalysisState { UNKNOWN_ANALYSIS = 0,

                     COMPLETED  = 2,
                     TERMINATED = 3,
                     
                     OVERRIDE = 5,

                     DELETE   = 7,

                     ADDING_POINTS = 10,

                     ADD_O_POINTS = 15,
                     ADD_X_POINTS = 16,

                     ADD_BOUNDARY_POINT = 17 };

enum SearchState { UNKNOWN_SEARCH = 0,

                   NO_SEARCH = 1,

                   ////// Code for island width search
                   ISLAND_O_POINT,
                   ISLAND_BOUNDARY_SEARCH,

                   ////// Code for island width search
                      
                   ////// Code for rational surface search
                   ORIGINAL_RATIONAL = 100,
                   SEARCHING_SEED,
                   WAITING_SEED,
                   FINISHED_SEED,
                   DEAD_SEED,
                   MINIMIZING_A      = 105,  // Used to bracket the minimum
                   MINIMIZING_B,
                   MINIMIZING_C,
                   MINIMIZING_X0     = 110, // Used for Golden search routine
                   MINIMIZING_X1,
                   MINIMIZING_X2,
                   MINIMIZING_X3
                      ////// Code for rational surface search
};


public:

  FieldlineType type;

  FieldlineType source;

  ////// Code for rational surface search
  AnalysisMethod analysisMethod;

  AnalysisState analysisState;

  ////// Code for rational surface search
  SearchState searchState;

  unsigned int iteration;

  double safetyFactor;

  // Base number of transits
  unsigned int toroidalWinding;
  unsigned int poloidalWinding;

  // Secondary axis number of transits
  unsigned int toroidalWindingP;
  unsigned int poloidalWindingP;

  // Number of transists when an island to get back to the initial puncture.
  unsigned int toroidalPeriod;
  unsigned int poloidalPeriod;

  // Resonance periods 

  // When a surface resonances equal 1

  // When a primary island the resonances equal the base number of
  // windings and the toroidal resonance and the base toroidal winding both
  // equal the number of islands.

  // When secondary islands the toroial resonance is the total number
  // of islands and the toroidal resonance divided by the base
  // toroidal winding equals the number of island on each group.

  unsigned int toroidalResonance;
  unsigned int poloidalResonance;

  std::vector< WindingPair > windingPairs;

  std::map< WindingPair, unsigned int > topWindingPairs;

  unsigned int windingGroupOffset;
  unsigned int islands;
  unsigned int islandGroups;

  // If a surface it's overlap is found geometrically
  // If an island (primary or secondary) toroidalPeriod / toroidalResonance
  double nnodes;

  unsigned int maxPunctures;
  unsigned int numPunctures;
  unsigned int nPuncturesNeeded;

  // Rational Surface periodicity measures
  double rationalSurfaceTolerance;

  // Seeds for islands
  avtVector lastSeedPoint;
  std::vector< avtVector > seedPoints;

  ////// Code for island width search
  bool pastFirstSearchFailure;
  double islandWidth;
  double searchBaseDelta;
  double searchDelta;
  double searchIncrement;
  double searchMagnitude;
  avtVector searchNormal;
  avtPoincareIC* parentOPointIC;
  avtPoincareIC* childOPointIC;

  unsigned int baseToroidalWinding;
  unsigned int basePoloidalWinding;

  ////// Code for island width search

  ////// Code for rational surface search
  // The rational points bounding the location of the minimization action
  avtVector rationalPt1;
  avtVector rationalPt2;

  std::vector< avtPoincareIC *> *children;

  avtVector srcPt;
  ////// Code for rational surface search
};
#endif

class IVP_API avtPoincareIC : public avtStateRecorderIntegralCurve
{
public:
    avtPoincareIC(int maxSteps, bool doTime, double maxTime,
                  unsigned int mask, const avtIVPSolver* model, 
                  Direction dir, const double& t_start, 
                  const avtVector &p_start,
                  const avtVector &v_start,
                  int ID);

    void          SetIntersectionCriteria(vtkObject *obj, int);
    void          SetPuncturePeriodCriteria(double, double);

    avtPoincareIC();
    virtual ~avtPoincareIC();

    virtual void  Serialize(MemStream::Mode mode,
                            MemStream &buff, 
                            avtIVPSolver *solver,
                            SerializeFlags serializeFlags);

    virtual void  MergeIntegralCurve(avtIntegralCurve *);

  protected:
    avtPoincareIC( const avtPoincareIC& );
    avtPoincareIC& operator=( const avtPoincareIC& );
    
    bool         IntersectPlane(const avtVector &p0, const avtVector &p1,
                                const double    &t0, const double    &t1);

  public:
    virtual bool CheckForTermination(avtIVPStep& step, avtIVPField *);

    virtual void SetMaxSteps( int ms ) { maxSteps = ms; }
    virtual int  GetNumSteps() { return numSteps; }
    virtual bool TerminatedBecauseOfMaxSteps(void)
                                 { return terminatedBecauseOfMaxSteps; };

    virtual void SetMaxIntersections( int mi ) { maxIntersections = mi; };
    virtual int  GetNumIntersections( ) { return numIntersections; };
    virtual bool TerminatedBecauseOfMaxIntersections(void) 
                            { return terminatedBecauseOfMaxIntersections; };

  protected:
    bool             doTime;
    double           maxTime;

    unsigned int     numSteps;
    unsigned int     maxSteps;
    bool             terminatedBecauseOfMaxSteps;

    unsigned int     numIntersections;
    unsigned int     maxIntersections;
    bool             terminatedBecauseOfMaxIntersections;

    // Intersection plane definition.
    avtVector intersectPlanePt;
    avtVector intersectPlaneNorm;

    double intersectPlaneEq[4]; // Typically the Y=0 plane i.e. 0, 1, 0

  public:
    // Intersection period for the double Poncare section
    double puncturePeriod;
    double puncturePeriodTolerance;

    // These are the fieldline points as stripped out of the IC
    // proper.  They are stored here for convience so the analysis can
    // be done without schleping the whole integral curve around.
    std::vector<avtVector> points;
    std::vector<double> times;

    // The fieldline properties as returned from the analysis library.
    FieldlineProperties properties;

    ////// Code for rational surface search
    avtPoincareIC *src_seed_ic;
    avtPoincareIC *src_rational_ic;

    // If this curve is minimizing, keep track of 'a' and 'c' (this is 'b')
    avtPoincareIC *a_IC;
    avtPoincareIC *b_IC;
    avtPoincareIC *c_IC;
    // Golden Search catches X0. X1, X2 and X3 must all have had integration done
    avtPoincareIC *GS_x1;
    avtPoincareIC *GS_x2;
    avtPoincareIC *GS_x3;
    ////// Code for rational surface search
};

// ostream operators for FieldlineProperties::AnalysisState's enum types
inline std::ostream& operator<<( std::ostream& out, 
                                 FieldlineProperties::AnalysisState state )
{
    switch( state )
    {
    case FieldlineProperties::UNKNOWN_ANALYSIS:
        return out << "UNKNOWN_ANALYSIS";
    case FieldlineProperties::OVERRIDE:
        return out << "OVERRIDE";
    case FieldlineProperties::ADDING_POINTS:
        return out << "ADDING_POINTS";
    case FieldlineProperties::ADD_O_POINTS:
        return out << "ADD_O_POINTS";
    case FieldlineProperties::ADD_BOUNDARY_POINT:
        return out << "ADD_BOUNDARY_POINT";
    case FieldlineProperties::COMPLETED:
        return out << "COMPLETED";
    case FieldlineProperties::TERMINATED:
        return out << "TERMINATED";
    default:
        return out << "???????";
    }
}

// ostream operators for FieldlineProperties::AnalysisState's enum types
inline std::ostream& operator<<( std::ostream& out, 
                                 FieldlineProperties::SearchState state )
{
    switch( state )
    {
    case FieldlineProperties::UNKNOWN_SEARCH:
        return out << "UNKNOWN_SEARCH";
    case FieldlineProperties::NO_SEARCH:
        return out << "NO_SEARCH";
    case FieldlineProperties::ISLAND_O_POINT:
        return out << "ISLAND_O_POINT";
    case FieldlineProperties::ISLAND_BOUNDARY_SEARCH:
        return out << "ISLAND_BOUNDARY_SEARCH";
    default:
        return out << "???????";
    }
}

inline std::ostream& operator<<( std::ostream& out, 
                                  FieldlineProperties::FieldlineType type )
{
    switch( type )
    {
    case FieldlineProperties::UNKNOWN_TYPE:
        return out << "UNKNOWN_TYPE";
    case FieldlineProperties::RATIONAL:
        return out << "RATIONAL";
    case FieldlineProperties::O_POINT:
        return out << "O_POINT";
    case FieldlineProperties::X_POINT:
        return out << "X_POINT";
    case FieldlineProperties::FLUX_SURFACE:
        return out << "FLUX_SURFACE";
    case FieldlineProperties::ISLAND_PRIMARY_CHAIN:
        return out << "ISLAND_PRIMARY_CHAIN";
    case FieldlineProperties::ISLAND_SECONDARY_CHAIN:
        return out << "ISLAND_SECONDARY_CHAIN";
    case FieldlineProperties::ISLAND_PRIMARY_SECONDARY_AXIS:
        return out << "ISLAND_PRIMARY_SECONDARY_AXIS";
    case FieldlineProperties::ISLAND_SECONDARY_SECONDARY_AXIS:
        return out << "ISLAND_SECONDARY_SECONDARY_AXIS";
    case FieldlineProperties::CHAOTIC:
        return out << "CHAOTIC";
    default:
        return out << "???????";
    }
}
#endif 
