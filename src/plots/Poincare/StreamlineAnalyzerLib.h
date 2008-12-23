/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2004 Scientific Computing and Imaging Institute,
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
 *  Copyright (C) 2006 SCI Group
 */

#if !defined(StreamlineAnalyzerLib_h)
#define StreamlineAnalyzerLib_h

#include "Point.h"
#include "Vector.h"

#include <vector>
#include <avtVector.h>

// Basic interface between the outside world and the base libs.
void getFieldlineProperties( const std::vector<avtVector> &pts,
                             unsigned int maxWindings,
                             unsigned int *winding,
                             unsigned int *twist,
                             unsigned int *islands );


namespace FusionPSE {

using namespace std;
using namespace SLIVR;
    //using namespace SCIRun;


class FieldlineLib
{
public:

  Point interpert( Point lastPt, Point currPt, double t );

  int ccw( Vector v0, Vector v1 );

  int intersect( Point l0_p0, Point l0_p1,
                 Point l1_p0, Point l1_p1 );

  unsigned int convexHull( vector< Point > &hullPts,
                           vector< unsigned int > &ordering,
                           unsigned int &m,
                           unsigned int winding,
                           int dir );

  unsigned int hull( vector< Point > &points,
                     unsigned int winding,
                     bool &convex );

  unsigned int factorial( unsigned int n0, unsigned int n1 );

  Point circle(Point &pt1, Point &pt2, Point &pt3);

  bool IsPerpendicular(Point &pt1, Point &pt2, Point &pt3);

  Point CalcCircle(Point &pt1, Point &pt2, Point &pt3);

  bool
  IntersectCheck( vector< Point >& points, unsigned int nbins );

  bool
  basicChecks( vector< Point >& points,
               Vector & globalCentroid,
               unsigned int &winding,
               unsigned int &twist,
               unsigned int &skip,
               unsigned int &type,
               unsigned int &island,
               float &avenode,
               bool &groupCCW,
               unsigned int &windingNextBest );

  unsigned int
  fieldlineProperties( vector< Point > &points,
                       unsigned int maxWindings,
                       unsigned int &winding,
                       unsigned int &twist,
                       unsigned int &skip,
                       unsigned int &type,
                       unsigned int &island,
                       float &avenode,
                       unsigned int &windingNextBest );


  unsigned int
  islandProperties( vector< Point > &points,
                    Vector globalCentroid,
                    unsigned int &startIndex,
                    unsigned int &middleIndex,
                    unsigned int &stopIndex,
                    unsigned int &nodes );

  unsigned int
  surfaceOverlapCheck( vector< vector< pair< Point, double > > > &bins,
                    unsigned int winding,
                    unsigned int skip,
                    unsigned int &nnodes );

  unsigned int
  surfaceGroupCheck( vector< vector< pair< Point, double > > > &bins,
                     unsigned int i,
                     unsigned int j,
                     unsigned int nnodes );

  unsigned int
  removeOverlap( vector< vector < pair< Point, double > > > &bins,
                 unsigned int &nnodes,
                 unsigned int winding,
                 unsigned int twist,
                 unsigned int skip,
                 unsigned int island );

  unsigned int
  smoothCurve( vector< vector < pair< Point, double > > > &bins,
               unsigned int &nnodes,
               unsigned int winding,
               unsigned int twist,
               unsigned int skip,
               unsigned int island );

  unsigned int
  mergeOverlap( vector< vector < pair< Point, double > > > &bins,
                unsigned int &nnodes,
                unsigned int winding,
                unsigned int twist,
                unsigned int skip,
                unsigned int island );
};

} // End namespace FusionPSE

#endif // StreamlineAnalyzerLib_h
