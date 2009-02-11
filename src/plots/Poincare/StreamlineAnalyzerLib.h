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

//#include <Core/Geometry/Point.h>
//#include <Core/Geometry/Vector.h>

#include <avtVector.h>

typedef avtVector Point ;
typedef avtVector Vector;

#include <vector>

enum FieldlineType { UNKNOWN  = 0,

                     PERIODIC = 10,
                     RATIONAL = 11,
                     O_POINT  = 12,
                     X_POINT  = 13,
                     
                     QUASI_PERIODIC = 20,
                     IRRATIONAL     = 21,
                     ISLAND_CHAIN   = 22,
                     
                     CHAOTIC = 30 };
  
struct FieldlineInfo {
  FieldlineType type;

  unsigned int toroidalWinding;
  unsigned int poloidalWinding;
  unsigned int skip;
  unsigned int islands;
  float nnodes;

  float confidence;
};

namespace FusionPSE {

using namespace std;
//using namespace SCIRun;


class FieldlineLib
{
public:

  Point interpert( Point lastPt, Point currPt, double t );

  int ccw( Vector v0, Vector v1 );

  int intersect( Point l0_p0, Point l0_p1,
                 Point l1_p0, Point l1_p1 );

  void convexHull( vector< Point > &hullPts,
                   vector< unsigned int > &ordering,
                   unsigned int &m,
                   unsigned int toroidalWinding,
                   int dir );

  bool hullCheck( vector< Point > &points,
                  unsigned int toroidalWinding );

  unsigned int factorial( unsigned int n0, unsigned int n1 );

  Point circle(Point &pt1, Point &pt2, Point &pt3);

  bool IsPerpendicular(Point &pt1, Point &pt2, Point &pt3);

  Point CalcCircle(Point &pt1, Point &pt2, Point &pt3);

  bool
  IntersectCheck( vector< Point >& points, unsigned int nbins );

  unsigned int Blankinship( unsigned int toroidalWinding,
                            unsigned int poloidalWinding,
                            unsigned int offset = 1 );

  void poloidalWindingCheck( vector< Point > &points,
                   vector< unsigned int > &poloidalWindingset,
                   unsigned int maxToroidalToroidalWinding,
                   unsigned int &bestToroidalWinding,
                   unsigned int &bestPoloidalWinding,
                   double &bestHitrate,
                   unsigned int &nextBestToroidalWinding,
                   unsigned int &nextBestPoloidalWinding,
                   double &nextBestHitrate,
                   unsigned int level );

  double
  poloidalWindingStats( vector< Point >& poloidalWinding_points,
              unsigned int poloidalWinding );

  bool
  rationalCheck( vector< Point >& points,
                 unsigned int toroidalWinding,
                 unsigned int &island,
                 float &avenode,
                 float delta=0.01);

  bool
  islandChecks( vector< Point >& points,
                unsigned int toroidalWinding,
                unsigned int &islands,
                float &avenode );

  bool
  basicChecks( vector< Point >& points,
               Vector & globalCentroid,
               unsigned int &toroidalWinding,
               unsigned int &poloidalWinding,
               unsigned int &skip,
               unsigned int &type,
               unsigned int &islands,
               float &avenode,
               bool &groupCCW,
               unsigned int &toroidalWindingNextBest );


  FieldlineInfo 
  fieldlineProperties( vector< Point > &ptList,
                       unsigned int override,
                       unsigned int maxToroidalWinding,
                       float hitrate );

  FieldlineInfo 
  fieldlineProperties( vector< Point > &points,
                       unsigned int maxToroidalToroidalWinding );


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
};

} // End namespace FusionPSE

#endif // StreamlineAnalyzerLib_h
