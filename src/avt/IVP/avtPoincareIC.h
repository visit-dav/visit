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
//                            avtPoincareIC.h                              //
// ************************************************************************* //

#ifndef AVT_POINCARE_IC_H
#define AVT_POINCARE_IC_H

#include <avtStateRecorderIntegralCurve.h>

// ****************************************************************************
//  Class: avtPoincareIC
//
//  Purpose:
//      A derived type of avtStateRecorderIntegralCurve.  This class 
//      decides how to terminate a streamline.
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

class FieldlineProperties {

public:

  FieldlineProperties()
  {
    type = FieldlineProperties::UNKNOWN_TYPE;
    analysisState = FieldlineProperties::UNKNOWN_STATE;
    analysisMethod = FieldlineProperties::DEFAULT_METHOD;

    searchState = FieldlineProperties::UNKNOWN_SEARCH;

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
    
    maxPunctures      = 0;
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

                     FLUX_SURFACE   = 0x0021,

                     ISLAND_CHAIN                    = 0x0040,

                     ISLAND_PRIMARY_CHAIN            = 0x0040,
                     ISLAND_SECONDARY_CHAIN          = 0x0042,

                     ISLAND_PRIMARY_SECONDARY_AXIS   = 0x0041,
                     ISLAND_SECONDARY_SECONDARY_AXIS = 0x0043,
                     
                     CHAOTIC = 30 };
  
enum AnalysisMethod { DEFAULT_METHOD,
                      RATIONAL_SEARCH,
                      RATIONAL_MINIMIZE,
                      RATIONAL_BRACKET }; //Remove a curve from continueExecute logic

enum AnalysisState { UNKNOWN_STATE = 0,

                     COMPLETED  = 2,
                     TERMINATED = 3,
                     
                     OVERRIDE = 5,

                     DELETE   = 7,

                     ADDING_POINTS = 10,

                     ADD_O_POINTS = 15,
                     ADD_X_POINTS = 16,

                     ADD_WIDTH_POINT };

enum SearchingState { UNKNOWN_SEARCH = 0,

                      ////// Code for island width search
                      ISLAND_O_POINT,

                      ISLAND_PCA_SEARCH,
                      ISLAND_WIDTH_SEARCH,
                      ISLAND_WIDTH_COMPLETED,
                      ////// Code for island width search
                      
                      ////// Code for rational surface search
                      ORIGINAL_RATIONAL = 100,
                      SEARCHING_SEED,
                      WAITING_SEED,
                      FINISHED_SEED,
                      MINIMIZING_A      = 105,  // Used to bracket the minimum
                      MINIMIZING_B,
                      MINIMIZING_C,
                      MINIMIZING_X0     = 110, // Used for Golden search routine
                      MINIMIZING_X1,
                      MINIMIZING_X2,
                      MINIMIZING_X3,
                      BRACKETING_A      = 120, //Used to bracket the minimum
                      BRACKETING_B,
                      BRACKETING_C
                      ////// Code for rational surface search
};



public:

  FieldlineType type;

  FieldlineType source;

  ////// Code for rational surface search
  AnalysisMethod analysisMethod;

  AnalysisState analysisState;

  ////// Code for rational surface search
  SearchingState searchState;

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

  std::vector< std::pair< unsigned int, unsigned int > > windingPairs;

  unsigned int windingGroupOffset;
  unsigned int islands;
  unsigned int islandGroups;

  // If a surface it is overlap found geometrically
  // If an island (primary or secondary) toroidalPeriod / toroidalResonance
  float nnodes;

  unsigned int maxPunctures;
  unsigned int nPuncturesNeeded;

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
  ////// Code for rational surface search
};
#endif

class IVP_API avtPoincareIC : public avtStateRecorderIntegralCurve
{
public:
    avtPoincareIC(unsigned char mask, const avtIVPSolver* model, 
                  Direction dir, const double& t_start, 
                  const avtVector &p_start,
                  const avtVector &v_start,
                  int ID);

    void          SetIntersectionCriteria(vtkObject *obj, int);

    avtPoincareIC();
    virtual ~avtPoincareIC();

    virtual void  Serialize(MemStream::Mode mode, MemStream &buff, 
                            avtIVPSolver *solver, SerializeFlags serializeFlags);

  protected:
    avtPoincareIC( const avtPoincareIC& );
    avtPoincareIC& operator=( const avtPoincareIC& );
    
    bool      IntersectPlane(const avtVector &p0, const avtVector &p1);

  public:
    virtual bool CheckForTermination(avtIVPStep& step, avtIVPField *);

    // Intersection points.
    bool   intersectionsSet;
    int    maxIntersections;
    int    numIntersections;
    double intersectPlaneEq[4]; // Typically the Y=0 plane i.e. 0, 1, 0

    // These are the fieldline points as stripped out of the IC
    // proper.  They are stored here for convience so the analysis can
    // be done without schleping the whole integral curve around.
    std::vector<avtVector> points;

    // The fieldline properties as returned from the analysis library.
    FieldlineProperties properties;


    ////// Code for rational surface search
    avtPoincareIC *source_ic;

    // If this curve is minimizing, keep track of 'a' and 'c' (this is 'b')
    float a_bound_dist;
    avtPoincareIC *a_IC;
    avtPoincareIC *b_IC;
    float c_bound_dist;
    avtPoincareIC *c_IC;
    // Golden Search catches X0. X1, X2 and X3 must all have had integration done
    avtPoincareIC *GS_x1;
    avtPoincareIC *GS_x2;
    avtPoincareIC *GS_x3;
    ////// Code for rational surface search
};

// ostream operators for FieldlineProperties::AnalysisState's enum types
inline std::ostream& operator<<( std::ostream& out, 
                                 FieldlineProperties::AnalysisState status )
{
    switch( status )
    {
    case FieldlineProperties::UNKNOWN_STATE:
        return out << "UNKNOWN_STATE";
    case FieldlineProperties::OVERRIDE:
        return out << "OVERRIDE";
    case FieldlineProperties::ADDING_POINTS:
        return out << "ADDING_POINTS";
    case FieldlineProperties::ADD_O_POINTS:
        return out << "ADD_O_POINTS";
    case FieldlineProperties::COMPLETED:
        return out << "COMPLETED";
    case FieldlineProperties::TERMINATED:
        return out << "TERMINATED";
    default:
        return out << "UNKNOWN";
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
        return out << "UNKNOWN";
    }
}
#endif 
