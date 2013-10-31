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
#include <avtPoincareIC.h>
#include <DebugStream.h>
#include <vector>

typedef avtVector Point ;
typedef avtVector Vector;

struct WindingPairStat {
  unsigned int toroidal;
  unsigned int poloidal;
  double stat;
  int ranking;

  bool operator < (const WindingPairStat &i) const {return stat < i.stat;}
  bool operator > (const WindingPairStat &i) const {return stat > i.stat;}
};


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
