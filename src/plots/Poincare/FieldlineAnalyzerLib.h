/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


/*
 *  FieldlineAnalyzer.h:
 *
 *  Written by:
 *   Allen Sanderson
 *   SCI Institute
 *   University of Utah
 *   September 2006
 *
 */

#if !defined(FieldlineAnalyzerLib_h)
#define FieldlineAnalyzerLib_h

#define STRAIGHTLINE_SKELETON 1

#ifdef STRAIGHTLINE_SKELETON
#include "skelet.h"
#endif

#include <avtVector.h>
#include <DebugStream.h>

typedef avtVector Point ;
typedef avtVector Vector;

#include <vector>

struct WindingPairStat {
  unsigned int toroidal;
  unsigned int poloidal;
  double stat;
  int ranking;
};


class FieldlineProperties;

#include <avtPoincareIC.h>

#ifndef POINCARE_FIELDLINE_PROPERTIES_H
#define POINCARE_FIELDLINE_PROPERTIES_H

class WindingPair {

public:
  WindingPair( unsigned int t, unsigned int p )
  {
    toroidal = t;
    poloidal = p;
  };

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
                      RATIONAL_MINIMIZE = 11,
                      RATIONAL_BRACKET  = 12 }; //Remove a curve from continueExecute logic

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
  float nnodes;

  unsigned int maxPunctures;
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
  ////// Code for rational surface search
};
#endif


class FieldlineLib
{
public:
  Point interpert( Point lastPt, Point currPt, double t );

  int ccw( Vector v0, Vector v1 );

  int intersect( Point l0_p0, Point l0_p1,
                 Point l1_p0, Point l1_p1 );

  void convexHull( std::vector< std::pair< Point, unsigned int > > &hullPts,
                   unsigned int &m,
                   unsigned int npts,
                   int dir );

  bool hullCheck( std::vector< Point > &points,
                  int &direction);

  unsigned int isPrime( unsigned int a );

  unsigned int GCD( unsigned int a, unsigned int b );

  unsigned int GCD( std::vector< unsigned int > values,
                    unsigned int &freq,
                    unsigned int minGCD = 1 );

  unsigned int ResonanceCheck( std::vector< std::pair< unsigned int, double > > &stats,
                              unsigned int baseResonance,
                              unsigned int max_samples = 3 );

  Point circle(Point &pt1, Point &pt2, Point &pt3);

  bool IsPerpendicular(Point &pt1, Point &pt2, Point &pt3);

  Point CalcCircle(Point &pt1, Point &pt2, Point &pt3);

  bool
  IntersectCheck( std::vector< Point >& points,
                  unsigned int nbins,
                  unsigned int skip  );

  unsigned int Blankinship( unsigned int toroidalWinding,
                            unsigned int poloidalWinding,
                            unsigned int skip   = 1 );

  void rotationalSumStats( std::vector< double > &rotationalSums,
                           double &averageRotationalSum,
                           double &stdDev );
  template< class TYPE >
  void safetyFactorStats( std::vector< TYPE > &poloidalWindingCounts,
                          double &averageSafetyFactor,
                          double &stdDev );

  void SortWindingPairStats( std::vector< WindingPairStat > &windingPairStats,
                             bool reverse = false );

  void RankWindingPairStats( std::vector< WindingPairStat > &windingPairStats,
                             bool LT = true );

  void poloidalWindingCheck( std::vector< unsigned int > &poloidalWindingset,
                             std::vector< WindingPairStat > &windingPairStats );

  void
  periodicityStats( std::vector< Point >& points,
                    std::vector< std::pair< unsigned int, double > >& stats,
                    unsigned int max_period,
                    unsigned int checkType );


  void thresholdStats( std::vector< std::pair< unsigned int, double > >& stats,
                       bool erase,
                       unsigned int checkType );

  double
  calculateSumOfSquares( std::vector< Point >& poloidalWinding_points,
                         unsigned int poloidalWinding,
                         unsigned int checkType );

  bool
  rationalCheck( std::vector< Point >& points,
                 unsigned int toroidalWinding,
                 unsigned int &nnodes,
                 float delta=0.01 );

  bool
  islandChecks( std::vector< Point >& points,
                unsigned int toroidalWinding,
                unsigned int &islands,
                unsigned int &nnodes,
                bool &complete );

  void
  getPunctures( std::vector< Point > &ptList,
                Vector planeN,
                std::vector< Point > &puncturePts );

  void
  getFieldlineBaseValues( std::vector< Point > &ptList,
                          std::vector< Point > &poloidal_puncture_pts,
                          std::vector< Point > &ridgeline_points,
                          std::vector< double > &rotationalSums,
                          std::vector< unsigned int > &poloidalWindingCounts,
                          float &delta,
                          unsigned int OLineToroidalWinding );

  Point getAxisPt( Point pt, double phi, double toroidalBase );

  void
  GetBaseWindingPairs( std::vector< unsigned int > &poloidalWindingCounts,
                       std::vector< Point > &poloidal_puncture_pts,
                       std::vector< WindingPairStat > &baseWindingPairStats,
                       double &windingPairConfidence,
                       unsigned int &toroidalWindingMax,
                       unsigned int &poloidalWindingMax,
                       unsigned int &windingNumberMatchIndex );

  void
  GetPeriodWindingPairs( std::vector< WindingPairStat > &baseWindingPairStats,
                         std::vector< WindingPairStat > &periodWindingPairStats,
                         std::vector< std::pair< unsigned int, double > > &toroidalStats,
                         std::vector< std::pair< unsigned int, double > > &poloidalStats );

  void
  fieldlineProperties( std::vector< Point > &ptList,
                       FieldlineProperties &properties,
                       unsigned int overrideToroidalWinding,
                       unsigned int overridePoloidalWinding,
                       unsigned int maxToroidalWinding,
                       double windingPairConfidence,
                       double rationalSurfaceFactor,
                       bool detectIslandCenters,
                       unsigned int OLineToroidalWinding,
                       std::string OLineAxisFilename );

  void
  fieldlineProperties2( std::vector< Point > &ptList,
                        double rationalSurfaceFactor,
                        FieldlineProperties &fi );

  void findIslandCenters( std::vector< Point > &puncturePts,
                          unsigned int islands,
                          unsigned int offset,
                          unsigned int nnodes,
                          unsigned int moduloValue,
                          std::vector< Point > &centers,
                          std::vector< Vector > &axis );

  Point findSkeletonCenter( Skeleton::Skeleton &s,
                            unsigned int nHullPts );

  unsigned int
  islandProperties( std::vector< Point > &points,
                    Vector &baseCentroid,
                    unsigned int &startIndex,
                    unsigned int &middleIndex,
                    unsigned int &stopIndex,
                    unsigned int &nodes );

  unsigned int
  surfaceOverlapCheck( std::vector< std::vector< Point > > &bins,
                    unsigned int toroidalWinding,
                    unsigned int skip,
                    unsigned int &nnodes );

  unsigned int
  surfaceGroupCheck( std::vector< std::vector< Point > > &bins,
                     unsigned int i,
                     unsigned int j,
                     unsigned int nnodes );

  unsigned int
  removeOverlap( std::vector< std::vector < Point > > &bins,
                 unsigned int &nnodes,
                 unsigned int toroidalWinding,
                 unsigned int poloidalWinding,
                 unsigned int skip,
                 unsigned int island );

  unsigned int
  smoothCurve( std::vector< std::vector < Point > > &bins,
               unsigned int &nnodes,
               unsigned int toroidalWinding,
               unsigned int poloidalWinding,
               unsigned int skip,
               unsigned int island );

  unsigned int
  mergeOverlap( std::vector< std::vector < Point > > &bins,
                unsigned int &nnodes,
                unsigned int toroidalWinding,
                unsigned int poloidalWinding,
                unsigned int skip,
                unsigned int island );

  bool verboseFlag;

protected:
  std::vector< Point > OLineAxisPts;
  std::vector< double > OLineAxisPhiAngles;
  unsigned int OLineAxisIndex;
  std::string OLineAxisFileName;
};


class Otsu
{
  // Compute the q values in the equation
  double Px( unsigned int init, unsigned int end, std::vector< unsigned int > &histo);

  // Compute the mean values in the equation (mu)
  double Mx( unsigned int init, unsigned int end, std::vector< unsigned int > &histo);

  // Find the maximum element in a std::vector
  unsigned int findMaxVet( std::vector< double > &vet, double &maxVet);    
  // Compute the histogram
  void getHistogram( std::vector< std::pair< unsigned int, double > >& stats,
                     std::vector< unsigned int > &histo );
public:
  // find otsu threshold
  void getOtsuThreshold2(std::vector< std::pair< unsigned int, double > >& stats,
                         double &threshold, double &maxVet );


  // find otsu threshold
  void getOtsuThreshold3(std::vector< std::pair< unsigned int, double > >& stats,
                         double &threshold0, double &threshold1,
                         double &maxVet );
};

int chainHull_2D( std::vector< std::pair< Point, unsigned int > > &pts,
                  std::vector< std::pair< Point, unsigned int > > &hullPts,
                  int direction  );

#endif // FieldlineAnalyzerLib_h
