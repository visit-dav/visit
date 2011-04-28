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
 *  StreamlineAnalyzer.h:
 *
 *  Written by:
 *   Allen Sanderson
 *   SCI Institute
 *   University of Utah
 *   September 2006
 *
 */

#if !defined(StreamlineAnalyzerLib_h)
#define StreamlineAnalyzerLib_h

//#define STRAIGHTLINE_SKELETON 1

#include <avtVector.h>
#include <DebugStream.h>

typedef avtVector Point ;
typedef avtVector Vector;

#include <vector>

using namespace std;

struct WindingPair {
  unsigned int toroidal;
  unsigned int poloidal;
  double stat;
  int ranking;
};


class FieldlineProperties;

#include <avtPoincareIC.h>

#ifndef POINCARE_FIELDLINE_PROPERTIES_H
#define POINCARE_FIELDLINE_PROPERTIES_H

class FieldlineProperties {

public:

  FieldlineProperties()
  {
    type = FieldlineProperties::UNKNOWN_TYPE;
    analysisState = FieldlineProperties::UNKNOWN_STATE;

    source = FieldlineProperties::UNKNOWN_TYPE;
    
    iteration = 0;

    toroidalWinding = 0;
    poloidalWinding = 0;

    toroidalHarmonic = 0;
    poloidalHarmonic = 0;

    windingGroupOffset = 0;
    islands = 0;
    islandGroups = 0;

    nnodes  = 0;
    
    maxPunctures      = 0;
    nPuncturesNeeded  = 0;
  };

enum FieldlineType { UNKNOWN_TYPE  = 0,

                     ORIGINAL_SEED = 1,

                     PERIODIC = 10,
                     RATIONAL = 11,
                     O_POINT  = 12,
                     X_POINT  = 13,
                     
                     QUASI_PERIODIC = 20,
                     IRRATIONAL     = 20,
                     FLUX_SURFACE   = 21,
                     ISLAND_CHAIN   = 22,
                     ISLANDS_WITHIN_ISLANDS = 23,
                     
                     CHAOTIC = 30 };
  
enum AnalysisState { UNKNOWN_STATE = 0,

                     ADDING_POINTS = 10,
                     RATIONAL_TEMPLATE_SEED = 11,
                     RATIONAL_SURFACE_SEED = 12,

                     O_POINT_SEED = 22,
                     X_POINT_SEED = 23,

                     COMPLETED  = 30,
                     TERMINATED = 40,
                     
                     DELETE     = 99,

                     ADD          = 50,
                     ADD_O_POINTS = 51,
                     ADD_X_POINTS = 52,

                     ADD_RATIONAL_SEED_POINT = 55 };

public:

  FieldlineType type;

  FieldlineType source;

  AnalysisState analysisState;

  unsigned int iteration;

  unsigned int toroidalWinding;
  unsigned int poloidalWinding;

  unsigned int toroidalPeriod;
  unsigned int poloidalPeriod;

  std::vector< std::pair< unsigned int, unsigned int > > windingPairs;

  unsigned int windingGroupOffset;
  unsigned int islands;
  unsigned int islandGroups;

  float nnodes;

  unsigned int maxPunctures;
  unsigned int nPuncturesNeeded;

  std::vector< Point > OPoints;
  bool seedOPoints;

  std::vector< int > parentIds;
  std::vector< int > childIds;
};
#endif

namespace FusionPSE {

//using namespace SCIRun;

class FieldlineLib
{
public:

  Point interpert( Point lastPt, Point currPt, double t );

  int ccw( Vector v0, Vector v1 );

  int intersect( Point l0_p0, Point l0_p1,
                 Point l1_p0, Point l1_p1 );

  void convexHull( vector< pair< Point, unsigned int > > &hullPts,
                   unsigned int &m,
                   unsigned int toroidalWinding,
                   int dir );

  bool hullCheck( vector< Point > &points,
                  int &direction);

  unsigned int isPrime( unsigned int a );

  unsigned int GCD( unsigned int a, unsigned int b );

  unsigned int GCD( vector< unsigned int > values,
                    unsigned int &freq,
                    unsigned int minGCD = 1 );

  unsigned int HarmonicCheck( vector< pair< unsigned int, double > > &stats,
                              unsigned int baseHarmonic,
                              unsigned int max_samples = 3 );

  Point circle(Point &pt1, Point &pt2, Point &pt3);

  bool IsPerpendicular(Point &pt1, Point &pt2, Point &pt3);

  Point CalcCircle(Point &pt1, Point &pt2, Point &pt3);

  bool
  IntersectCheck( vector< Point >& points,
                  unsigned int nbins,
                  unsigned int skip  );

  unsigned int Blankinship( unsigned int toroidalWinding,
                            unsigned int poloidalWinding,
                            unsigned int offset = 1 );

  
  void rotationalSumStats( vector< double > &rotationalSums,
                           double &averageRotationalSum,
                           double &stdDev );
  template< class TYPE >
  void safetyFactorStats( vector< TYPE > &poloidalWindingCounts,
                          double &averageSafetyFactor,
                          double &stdDev );

  void SortWindingPairs( vector< WindingPair > &windingPairs,
                         bool reverse = false );

  void RankWindingPairs( vector< WindingPair > &windingPairs,
                         bool LT = true );

  void poloidalWindingCheck( vector< unsigned int > &poloidalWindingset,
                             vector< WindingPair > &windingSetList );

  void
  periodicityStats( vector< Point >& points,
                    vector< pair< unsigned int, double > >& stats,
                    unsigned int max_period,
                    unsigned int checkType );


  void thresholdStats( vector< pair< unsigned int, double > >& stats,
                       bool erase,
                       unsigned int checkType );

  double
  calculateSumOfSquares( vector< Point >& poloidalWinding_points,
                         unsigned int poloidalWinding,
                         unsigned int checkType );

  bool
  rationalCheck( vector< Point >& points,
                 unsigned int toroidalWinding,
                 unsigned int &nnodes,
                 float delta=0.01 );

  bool
  islandChecks( vector< Point >& points,
                unsigned int toroidalWinding,
                unsigned int &islands,
                unsigned int &nnodes,
                bool &complete );

  void
  getPunctures( vector< Point > &ptList,
                Vector planeN,
                vector< Point > &puncturePts );

  void
  getFieldlineBaseValues( vector< Point > &ptList,
                          vector< Point > &poloidal_puncture_pts,
                          vector< Point > &ridgeline_points,
                          vector< double > &rotationalSums,
                          vector< unsigned int > &poloidalWindingCounts,
                          float &delta );

  void
  GetBaseWindingPairs( vector< unsigned int > &poloidalWindingCounts,
                       vector< Point > &poloidal_puncture_pts,
                       vector< WindingPair > &baseWindingPairs,
                       double &windingPairConfidence,
                       unsigned int &toroidalWindingMax,
                       unsigned int &poloidalWindingMax,
                       unsigned int &windingNumberMatchIndex );

  void
  GetPeriodWindingPairs( vector< WindingPair > &baseWindingPairs,
                         vector< WindingPair > &periodWindingPairs,
                         vector< pair< unsigned int, double > > &toroidalStats,
                         vector< pair< unsigned int, double > > &poloidalStats );

  void
  fieldlineProperties( vector< Point > &ptList,
                       FieldlineProperties &properties,
                       unsigned int overrideToroidalWinding,
                       unsigned int overridePoloidalWinding,
                       unsigned int maxToroidalWinding,
                       double windingPairConfidence,
                       bool detectIslandCenters );

  void
  fieldlineProperties2( vector< Point > &ptList,
                        FieldlineProperties &fi );

  void findIslandCenters( vector< Point > &puncturePts,
                          unsigned int toroialWinding,
                          unsigned int nnodes,
                          vector< Point > &centers );

  unsigned int
  islandProperties( vector< Point > &points,
                    Vector &baseCentroid,
                    unsigned int &startIndex,
                    unsigned int &middleIndex,
                    unsigned int &stopIndex,
                    unsigned int &nodes );

  unsigned int
  surfaceOverlapCheck( vector< vector< Point > > &bins,
                    unsigned int toroidalWinding,
                    unsigned int skip,
                    unsigned int &nnodes );

  unsigned int
  surfaceGroupCheck( vector< vector< Point > > &bins,
                     unsigned int i,
                     unsigned int j,
                     unsigned int nnodes );

  unsigned int
  removeOverlap( vector< vector < Point > > &bins,
                 unsigned int &nnodes,
                 unsigned int toroidalWinding,
                 unsigned int poloidalWinding,
                 unsigned int skip,
                 unsigned int island );

  unsigned int
  smoothCurve( vector< vector < Point > > &bins,
               unsigned int &nnodes,
               unsigned int toroidalWinding,
               unsigned int poloidalWinding,
               unsigned int skip,
               unsigned int island );

  unsigned int
  mergeOverlap( vector< vector < Point > > &bins,
                unsigned int &nnodes,
                unsigned int toroidalWinding,
                unsigned int poloidalWinding,
                unsigned int skip,
                unsigned int island );

  bool verboseFlag;
};


class Otsu
{
  // Compute the q values in the equation
  double Px( unsigned int init, unsigned int end, vector< unsigned int > &histo);

  // Compute the mean values in the equation (mu)
  double Mx( unsigned int init, unsigned int end, vector< unsigned int > &histo);

  // Find the maximum element in a vector
  unsigned int findMaxVet( vector< double > &vet, double &maxVet);    
  // Compute the histogram
  void getHistogram( vector< pair< unsigned int, double > >& stats,
                     vector< unsigned int > &histo );
public:
  // find otsu threshold
  void getOtsuThreshold2(vector< pair< unsigned int, double > >& stats,
                         double &threshold, double &maxVet );


  // find otsu threshold
  void getOtsuThreshold3(vector< pair< unsigned int, double > >& stats,
                         double &threshold0, double &threshold1,
                         double &maxVet );
};

int chainHull_2D( vector< pair< Point, unsigned int > > &pts,
                  vector< pair< Point, unsigned int > > &hullPts,
                  int direction  );

} // End namespace FusionPSE

#endif // StreamlineAnalyzerLib_h
