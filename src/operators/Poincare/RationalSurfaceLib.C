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
//                            RationaSurfaceLib.C                            //
// ************************************************************************* //

#include "RationalSurfaceLib.h"
#include <algorithm>
#include <limits>

bool MinimumIsBracketed(avtPoincareIC *poincare_ic)
{
  Vector xzplane(0,1,0);
  FieldlineLib fieldlib;
  avtPoincareIC *_a = poincare_ic->a_IC;
  avtPoincareIC *_b = poincare_ic;
  avtPoincareIC *_c = poincare_ic->c_IC;        
  std::vector<avtVector> a_puncturePoints;
  std::vector<avtVector> b_puncturePoints;
  std::vector<avtVector> c_puncturePoints;
  fieldlib.getPunctures(_a->points,xzplane,a_puncturePoints);
  fieldlib.getPunctures(_b->points,xzplane,b_puncturePoints);
  fieldlib.getPunctures(_c->points,xzplane,c_puncturePoints);
                
  // Need to get distances for each a, b & c
  double a_dist = FindMinimizationDistance(_a);
  double b_dist = FindMinimizationDistance(_b);
  double c_dist = FindMinimizationDistance(_c);

  if (3 <= RATIONAL_DEBUG)
    std::cerr << "LINE " << __LINE__ << " Checking if min is bracketed:\ta_dist, b_dist, c_dist:\n "
              << a_dist<<", "<<b_dist<<", "<<c_dist  << std::endl;

  return (b_dist <= a_dist && b_dist <= c_dist);
}
bool BracketIsValid(avtPoincareIC *poincare_ic)
{
  Vector xzplane(0,1,0);
  FieldlineLib fieldlib;
  avtPoincareIC *_a = poincare_ic->a_IC;
  avtPoincareIC *_b = poincare_ic;
  avtPoincareIC *_c = poincare_ic->c_IC;        
  std::vector<avtVector> a_puncturePoints;
  std::vector<avtVector> b_puncturePoints;
  std::vector<avtVector> c_puncturePoints;
  fieldlib.getPunctures(_a->points,xzplane,a_puncturePoints);
  fieldlib.getPunctures(_b->points,xzplane,b_puncturePoints);
  fieldlib.getPunctures(_c->points,xzplane,c_puncturePoints);
                
  // Need to get distances for each a, b & c
  int a_i = FindMinimizationIndex(_a);
  int b_i = FindMinimizationIndex(_b);
  int c_i = FindMinimizationIndex(_c);

  if (3 <= RATIONAL_DEBUG)
    {
      std::cerr << "LINE " << __LINE__ << "  " << "a_i, b_i, c_i: "             <<a_i;
      if (a_i > -1)
        std::cerr <<"\n"<< VectorToString(a_puncturePoints[a_i])<<",\n"<<b_i;
      if (b_i > -1) 
        std::cerr << VectorToString(b_puncturePoints[b_i])<<",\n"<<c_i;
      if (c_i > -1)     
        std::cerr << VectorToString(c_puncturePoints[c_i]);
      std::cerr << std::endl;
    }
      
  return !(a_i == -1 || b_i == -1 || c_i == -1);
}
bool NeedToMinimize(avtPoincareIC *poincare_ic)
{
  Vector xzplane(0,1,0);
  FieldlineLib fieldlib;

  std::vector<avtVector> seed_puncture_points;
  fieldlib.getPunctures( poincare_ic->points, xzplane,
                         seed_puncture_points);

  std::vector<avtVector> orig_puncture_points;
  fieldlib.getPunctures( poincare_ic->src_rational_ic->points, xzplane,
                         orig_puncture_points);

  // Need to get distances for each a, b & c
  int _i = FindMinimizationIndex(poincare_ic);
  // Find distance between puncture points
  double _dist = PythDist(seed_puncture_points[_i],seed_puncture_points[_i+poincare_ic->properties.toroidalWinding]);

  return (_dist >  MAX_SPACING);
}
bool SetupNewBracketA(avtPoincareIC *bracketA, avtPoincareIC *parent_seed, avtVector aPt)
{
  FieldlineProperties &properties = parent_seed->properties;
  // Transfer and update properties.
  bracketA->properties = properties;
  bracketA->src_seed_ic = parent_seed;
  bracketA->src_rational_ic = parent_seed->src_rational_ic;
  bracketA->properties.iteration = properties.iteration + 1;
  bracketA->maxIntersections = 8 * (properties.toroidalWinding + 2);
  bracketA->properties.type = FieldlineProperties::IRRATIONAL;
  bracketA->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
  bracketA->properties.searchState = FieldlineProperties::MINIMIZING_A;
  bracketA->properties.srcPt = aPt;

  if (3 <= RATIONAL_DEBUG)
    std::cerr << "LINE " << __LINE__ << "  "
              << "Setup Bracket A ID: "
              << bracketA->id << std::endl;

  bracketA->properties.rationalPt1 = properties.rationalPt1;
  bracketA->properties.rationalPt2 = properties.rationalPt2;

  return true;
}
bool SetupNewBracketB(avtPoincareIC *bracketB, avtPoincareIC *parent_seed, avtVector bPt)
{
  FieldlineProperties &properties = parent_seed->properties;
  // Transfer and update properties.
  bracketB->properties = properties;
  bracketB->src_seed_ic = parent_seed;
  bracketB->src_rational_ic = parent_seed->src_rational_ic;
  bracketB->properties.iteration = properties.iteration + 1;
  bracketB->maxIntersections = 8 * (properties.toroidalWinding + 2);
  bracketB->properties.type = FieldlineProperties::IRRATIONAL;
  bracketB->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
  bracketB->properties.searchState = FieldlineProperties::MINIMIZING_B;
  bracketB->properties.srcPt = bPt;

  if (3 <= RATIONAL_DEBUG)
    std::cerr << "LINE " << __LINE__ << "  "
              << "Setup Bracket B ID: "
              << bracketB->id << std::endl;

  bracketB->properties.rationalPt1 = properties.rationalPt1;
  bracketB->properties.rationalPt2 = properties.rationalPt2;

  return true;
}
bool SetupNewBracketC(avtPoincareIC *bracketC, avtPoincareIC *parent_seed, avtVector cPt)
{
  FieldlineProperties &properties = parent_seed->properties;
  // Transfer and update properties.
  bracketC->properties = properties;
  bracketC->src_seed_ic = parent_seed;
  bracketC->src_rational_ic = parent_seed->src_rational_ic;
  bracketC->properties.iteration = properties.iteration + 1;
  bracketC->maxIntersections = 8 * (properties.toroidalWinding + 2);
  bracketC->properties.type = FieldlineProperties::IRRATIONAL;
  bracketC->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
  bracketC->properties.searchState = FieldlineProperties::MINIMIZING_C;
  bracketC->properties.srcPt = cPt;

  if (3 <= RATIONAL_DEBUG)
    std::cerr << "LINE " << __LINE__ << "  "
              << "Setup Bracket C ID: "
              << bracketC->id << std::endl;
  
  bracketC->properties.rationalPt1 = properties.rationalPt1;
  bracketC->properties.rationalPt2 = properties.rationalPt2;
  
  return true;
}


//*******************
// To start the minimization, we need to define three points
// If the middle point ('b) is already lower than the
// other two we can go straight to
// minimization. Otherwise, we have to bracket the
// minimum.  First things first, setup the two new
// points (b & c) and send them off
bool PrepareToBracket(avtPoincareIC *seed,
                      avtVector &newA,
                      avtVector &newB,
                      avtVector &newC)
{
  if (5 <= RATIONAL_DEBUG)
    std::cerr<< "Line: " << __LINE__ <<" Preparing to bracket the minimum"<<std::endl;
  
  Vector xzplane(0,1,0);
  FieldlineLib fieldlib;
  
  std::vector<avtVector> seed_puncture_points;
  fieldlib.getPunctures( seed->points, xzplane,
                         seed_puncture_points);
  avtVector maxPuncture;
  avtVector origPt1 = seed->properties.rationalPt1;
  avtVector origPt2 = seed->properties.rationalPt2;
  int ix = FindMinimizationIndex(seed);
  if (1 <= RATIONAL_DEBUG)
    cerr <<"Line: "<<__LINE__<< " Bracketing inside Original Rational Pts:\n"<<VectorToString(origPt1)<<"\n"<<VectorToString(origPt2)<<std::endl;
  if (ix < 0)
    return false;  

  maxPuncture = seed_puncture_points[ix];
  
  avtVector origChord = origPt2 - origPt1;
  avtVector perpendicular = -origChord.cross(avtVector(0,1,0));

  double xa = (origPt1 - maxPuncture).dot(perpendicular);
  perpendicular.normalize();
  
  double minDist = 2 * MAX_SPACING;
  if (xa < minDist)
    xa = minDist;

  avtVector intersection = maxPuncture + xa * perpendicular;

  cerr << "Max Puncture: "<<VectorToString(maxPuncture)<<"\nxa: "<<xa<<"\nintersection: "<<VectorToString(intersection)<<"\n";
  
  avtVector newPt1 = intersection;
  avtVector newPt2 = maxPuncture + GOLD * (newPt1 - maxPuncture);

  newPt1[1] =  Z_OFFSET;
  newPt2[1] =  Z_OFFSET;
  
  newA = maxPuncture;
  newB = newPt1;
  newC = newPt2;

  if (1 <= RATIONAL_DEBUG)
    cerr<<"LINE "<<__LINE__<<"New A,B,C:\n"<< VectorToString(newA) <<"\n"<< VectorToString(newB) <<"\n"<< VectorToString(newC) <<"\n";

  seed->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
  seed->properties.searchState = FieldlineProperties::MINIMIZING_A;
  
  return true;
}
bool UpdateBracket(avtPoincareIC *poincare_ic, bool &swapped, avtVector &C)
{
  if (4 <= RATIONAL_DEBUG)
    std::cerr << "LINE " << __LINE__ << "  " << "Minimum needs to be bracketed" << std::endl;                          

  // Have bracketed the minimum
  Vector xzplane(0,1,0);
  FieldlineLib fieldlib;
  avtPoincareIC *_a = poincare_ic->a_IC;
  avtPoincareIC *_b = poincare_ic;
  avtPoincareIC *_c = poincare_ic->c_IC;        
  std::vector<avtVector> a_puncturePoints;
  std::vector<avtVector> b_puncturePoints;
  std::vector<avtVector> c_puncturePoints;
  fieldlib.getPunctures(_a->points,xzplane,a_puncturePoints);
  fieldlib.getPunctures(_b->points,xzplane,b_puncturePoints);
  fieldlib.getPunctures(_c->points,xzplane,c_puncturePoints);
                
  // Need to get distances for each a, b & c
  double a_dist = FindMinimizationDistance(_a);
  double b_dist = FindMinimizationDistance(_b);
  double c_dist = FindMinimizationDistance(_c);
                
  // Need to swap so a > b
  if ( b_dist > a_dist )
    {
      swapped = true;

      avtPoincareIC *temp = _a;
      _a = _b;
      _b = temp;

      std::vector<avtVector> vtemp = a_puncturePoints;
      a_puncturePoints = b_puncturePoints;
      b_puncturePoints = vtemp;

      int itemp = a_dist;
      a_dist = b_dist;
      b_dist = itemp;

      _a->properties.searchState =
        FieldlineProperties::MINIMIZING_A;
      _b->properties.searchState =
        FieldlineProperties::MINIMIZING_B;

      _b->a_IC = _a;
   }

  if (c_dist >= b_dist)
    return false;

  int a_i = FindMinimizationIndex(_a);
  int b_i = FindMinimizationIndex(_b);
  int c_i = FindMinimizationIndex(_c);

  // For Parabolic Bracketing, not actually implemented, too difficult
  //          double rr = PythDist( _a, _b ) * (b_dist - c_dist);
  //          double qq = PythDist( _c, _b ) * (b_dist - a_dist);

  avtVector va = a_puncturePoints[a_i];
  avtVector vb = b_puncturePoints[b_i];              
  avtVector vc = c_puncturePoints[c_i];
  if (swapped == true)
    {
      vc = vb + GOLD * (vb - va);
    }
  else    
    {
      vc = vb + GOLD * (vc - vb);
    }

  C = vc;
  C[1] = Z_OFFSET;

  return true;
}
bool PrepareToMinimize(avtPoincareIC *poincare_ic, avtVector &newPt, bool &cbGTba)
{
  // Have bracketed the minimum
  Vector xzplane(0,1,0);
  FieldlineLib fieldlib;
  avtPoincareIC *_a = poincare_ic->a_IC;
  avtPoincareIC *_b = poincare_ic;
  avtPoincareIC *_c = poincare_ic->c_IC;        
  std::vector<avtVector> a_puncturePoints;
  std::vector<avtVector> b_puncturePoints;
  std::vector<avtVector> c_puncturePoints;
  fieldlib.getPunctures(_a->points,xzplane,a_puncturePoints);
  fieldlib.getPunctures(_b->points,xzplane,b_puncturePoints);
  fieldlib.getPunctures(_c->points,xzplane,c_puncturePoints);
                
  // Need to get distances for each a, b & c
  int a_i = FindMinimizationIndex(_a);
  int b_i = FindMinimizationIndex(_b);
  int c_i = FindMinimizationIndex(_c);

  _a->properties.searchState = FieldlineProperties::MINIMIZING_X0;
  _c->properties.searchState = FieldlineProperties::MINIMIZING_X3;
  _a->properties.analysisMethod = FieldlineProperties::RATIONAL_MINIMIZE;
  _c->properties.analysisMethod = FieldlineProperties::RATIONAL_MINIMIZE;
  _a->properties.rationalPt1 =_b->properties.rationalPt1;
  _a->properties.rationalPt2 =_b->properties.rationalPt2;
              
  double bx_ax,cx_bx;
  bx_ax = PythDist(a_puncturePoints[a_i],b_puncturePoints[b_i]);
  cx_bx = PythDist(b_puncturePoints[b_i],c_puncturePoints[c_i]);
                
  avtVector va = a_puncturePoints[a_i];
  avtVector vb = b_puncturePoints[b_i];
  avtVector vc = c_puncturePoints[c_i];

  if (3 <= RATIONAL_DEBUG)
    {
      std::cerr<<"Line: "<<__LINE__<<"Prepare to minimize: "<<"\n\t"<<VectorToString(a_puncturePoints[a_i])<<"\n\t"<<VectorToString(b_puncturePoints[b_i])<<"\n\t"<<VectorToString(c_puncturePoints[c_i])<<"\n";
    }
  if (cx_bx > bx_ax)
    {
      cbGTba = true;
      _b->properties.searchState = FieldlineProperties::MINIMIZING_X1;
      _b->properties.analysisMethod = FieldlineProperties::RATIONAL_MINIMIZE;
      
      newPt = vb + golden_C * (vc-vb);

      if (2 <= RATIONAL_DEBUG)
        std::cerr << "LINE " << __LINE__ << " New Minimization pt:\t"<<VectorToString(newPt)<< std::endl; 
    }
  else
    {
      cbGTba = false;
      _b->properties.searchState = FieldlineProperties::MINIMIZING_X2;
      _b->properties.analysisMethod = FieldlineProperties::RATIONAL_MINIMIZE;
      
      newPt = vb + golden_C * (va-vb);
      
      if (2 <= RATIONAL_DEBUG)   
        std::cerr << "LINE " << __LINE__  << " New Minimization pt:\t"<< VectorToString(newPt) << std::endl; 
    }

  newPt[1] = Z_OFFSET;

    return true;
}
bool SetupNewSeed(avtPoincareIC *seed,
                  avtPoincareIC *rational,
                  avtVector seedPt,
                  avtVector point1,
                  avtVector point2)
{
  FieldlineProperties &properties = rational->properties;
  // Transfer and update properties.
  seed->properties = properties;
  seed->src_rational_ic = rational;
  seed->src_seed_ic = seed;
  seed->properties.iteration = properties.iteration + 1;
  seed->maxIntersections = 8 * (properties.toroidalWinding + 2);
  seed->properties.type = FieldlineProperties::IRRATIONAL;
  seed->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
  seed->properties.searchState = FieldlineProperties::SEARCHING_SEED;
  seed->a_IC = NULL;
  seed->c_IC = NULL;
  seed->properties.rationalPt1 = point1;
  seed->properties.rationalPt2 = point2;
  seed->properties.srcPt = seedPt;

  rational->properties.children->push_back(seed);
  
  return true;
}
bool SetupRational(avtPoincareIC *rational)
{
  if (2 <= RATIONAL_DEBUG)
    {
      std::vector<avtVector> puncturePoints;
      FieldlineLib fieldlib;
      fieldlib.getPunctures(rational->points, avtVector(0, 1, 0), puncturePoints);
      std::cerr << "LINE " << __LINE__ << "  " 
                << "Found an unsearched rational, ID: " 
                << rational->id << ", " << puncturePoints[0] << std::endl;
    }
  // Update rational's properties
  // The analysis method is Rational_Search for most of the process.
  
  // The Original_Rational is kept around mainly to help with
  // organization.
  rational->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
  rational->properties.searchState  = FieldlineProperties::ORIGINAL_RATIONAL;
  
  // The Original_Rational has a list of each of the
  // seeds. These get swapped out with better curves over the
  // course of the minimization and this list is used to draw
  // the final curves
  rational->properties.children = new std::vector< avtPoincareIC* >();

  return true;
}

/**     
 *
 *      pythDist
 *      Get the pythagorean distance between two points
 *
 **/
double PythDist( avtVector p1, avtVector p2 )
{
  return (p1 - p2).length();
}


/**     
 *
 *      getAngle
 *      Get the angle between three points, let b be the center.
 *
 */
double GetAngle( avtVector a, avtVector b, avtVector c )
{
  return acos( (PythDist(b,a)*PythDist(b,a) + 
                PythDist(b,c)*PythDist(b,c) -
                PythDist(c,a)*PythDist(c,a)) / 
               (2.0f * PythDist(b,a) * PythDist(b,c)) );
}
        

/**
 *
 * return a vector of seed points for the given rational
 *
 **/
std::vector<avtVector> GetSeeds( avtPoincareIC *poincare_ic,
                                 avtVector &point1,
                                 avtVector &point2,
                                 double maxDistance )
{
  std::vector<avtVector> puncturePoints;

  FieldlineLib fieldlib;
  fieldlib.getPunctures(poincare_ic->points, avtVector(0, 1, 0), puncturePoints);

  unsigned int toroidalWinding = poincare_ic->properties.toroidalWinding;
  unsigned int windingGroupOffset = poincare_ic->properties.windingGroupOffset;
  
  // Calculate angle size for each puncture point and find the one
  // that forms the largest angle (i.e. flattest portion of the
  // surface).
  int nSeeds = 0;
  double maxAngle = 0;
  unsigned int best_index = 0;

  unsigned int best_one_less = 0;
  unsigned int best_one_more = 0;

  bool twoPts = false;
  for( unsigned int i=0; i<toroidalWinding; ++i )
  {
    unsigned int one_less = (i-windingGroupOffset+toroidalWinding) % toroidalWinding;
    unsigned int one_more = (i+windingGroupOffset+toroidalWinding) % toroidalWinding;
    
    if (one_less == one_more)
      {
        best_index = i;
        best_one_more = one_more;
        twoPts = true;
        break;
      }
    
    if (3 <= RATIONAL_DEBUG)
      cerr << "Line: "<<__LINE__<<" wgo: "<<windingGroupOffset<<", ix-1: "<<one_less<<", ix: "<<i<<", ix+1: " <<one_more<<std::endl;

    avtVector pt0 = puncturePoints[one_less];
    avtVector pt1 = puncturePoints[i];
    avtVector pt2 = puncturePoints[one_more];

    // Save the maximum angle angle and the index of the puncture point.
    double angle = GetAngle( pt0, pt1, pt2 );
    if (maxAngle < angle)
    {     
      maxAngle = angle;
      best_index = i;

      best_one_less = one_less;
      best_one_more = one_more;
    }
  } 

 // Get circle equation 
  avtVector pt0,pt1,pt2;
  if (twoPts)
    {
       pt0 = puncturePoints[best_index];
       pt2 = puncturePoints[best_one_more];
       if (1 <= RATIONAL_DEBUG)
        cerr <<"Line: "<<__LINE__<< " 2 Rational Pts:\n"
             <<VectorToString(pt0)<<"\n"
             <<VectorToString(pt2)<<"\n";
       avtVector midpt = pt0 + 0.5 * (pt2-pt0);
       avtVector cx = (pt2-pt0).cross(avtVector(0,1,0));
       avtVector newpt = midpt + .5*cx;
       if (1 <= RATIONAL_DEBUG)
        cerr <<"Line: "<<__LINE__<< " 2 New Pts:\n"
             <<VectorToString(midpt)<<"\n"
             <<VectorToString(newpt)<<"\n";

       pt1 = newpt;
    }
  else
    {

      // Find the circle that intersects the three punctures points which
      // approximates the cross section of the surface.
      
      // Get circle equation 
      pt0 = puncturePoints[best_one_less];
      pt1 = puncturePoints[best_index];
      pt2 = puncturePoints[best_one_more];
      if (1 <= RATIONAL_DEBUG)
        cerr <<"Line: "<<__LINE__<< " Rational Pts:\n"
             <<VectorToString(pt0)<<"\n"
             <<VectorToString(pt1)<<"\n"
             <<VectorToString(pt2)<<"\n";
    }

  point1 = pt1; //for future reference
  point2 = pt2;
  
  if (1 <= RATIONAL_DEBUG)
    cerr <<"Line: "<<__LINE__<< " New seeds between:\n"
         <<VectorToString(point1)<<"\n"
         <<VectorToString(point2)<<"\n";
  
  // Center of the circle
  /////*********** Find The Circle Using Three Points******************///      
  double ax,ay,bx,by,cx,cy,x1,y11,dx1,dy1,x2,y2,dx2,dy2,ox,oy,dx,dy,radius;
  ax = pt0[0]; ay = pt0[2];
  bx = pt1[0]; by = pt1[2];
  cx = pt2[0]; cy = pt2[2];      
  x1 = (bx + ax) / 2;
  y11 = (by + ay) / 2;
  dy1 = bx - ax;
  dx1 = -(by - ay);      
  x2 = (cx + bx) / 2;
  y2 = (cy + by) / 2;
  dy2 = cx - bx;
  dx2 = -(cy - by);      
  ox = (y11 * dx1 * dx2 + x2 * dx1 * dy2 - x1 * dy1 * dx2 - y2 * dx1 * dx2)/ (dx1 * dy2 - dy1 * dx2);
  oy = (ox - x1) * dy1 / dx1 + y11;      
  dx = ox - ax;
  dy = oy - ay;
  radius = sqrt(dx * dx + dy * dy);     
  
  avtVector center(ox,0,oy);
  
  // Get the number of points needed to cover the range between the two
  // the puncture points.
  double dist = (pt2 - pt1).length();
  nSeeds = dist / maxDistance;
  if( dist > (double) nSeeds * maxDistance )
    ++nSeeds;
  double angle =GetAngle(point1, center, point2);
  
  // Add seeds stretching between two of the puncture points.
  std::vector<avtVector> seedPts;
  seedPts.resize( nSeeds );
  if (1 <= RATIONAL_DEBUG)
    std::cerr <<"Line: "<<__LINE__
              << " \ncenter: " << center 
              << " \nradius: " << radius
              << " \nnSeeds: " << nSeeds
              << " \nAngle: " << angle
              << std::endl;
  for( double i=0; i<nSeeds; ++i)
    {
      double t = i / nSeeds;
      avtVector X = center + (std::sin((1-t) * angle ) * (point1-center) + std::sin(t * angle) * (point2-center)) / std::sin(angle);
      X[1] = Z_OFFSET;
      
      seedPts[i] = X;
      
      if (3 <= RATIONAL_DEBUG)
        cerr << "New Seed: "<<VectorToString(X) << std::endl;
    }
  
  return seedPts;  
}


double MaxRationalDistance( avtPoincareIC *ic,
                        unsigned int toroidalWinding,
                        unsigned int &index )
{  
  double delta = 0.0;

  std::vector<avtVector> puncturePoints;
  FieldlineLib fieldlib;
  fieldlib.getPunctures(ic->points, avtVector(0, 1, 0), puncturePoints);

  for (unsigned int i=0; i < toroidalWinding; ++i)
    {      
      avtVector vec = puncturePoints[i] - puncturePoints[i+toroidalWinding];
      if( delta < vec.length() )
      {
        delta = vec.length();
        index = i;
      }
    }

  return delta;
  
}
double MinRationalDistance( avtPoincareIC *ic,
                        unsigned int toroidalWinding,
                        unsigned int &index )
{
  
  double delta = 9999999999999.0;


  std::vector<avtVector> puncturePoints;
  FieldlineLib fieldlib;
  fieldlib.getPunctures(ic->points, avtVector(0, 1, 0), puncturePoints);

  for (unsigned int i=0; i+toroidalWinding < puncturePoints.size() && i < toroidalWinding; ++i)
    {      
      avtVector vec = puncturePoints[i] - puncturePoints[i+toroidalWinding];
      if( delta > vec.length() )
      {
        delta = vec.length();
        index = i;
      }
    }
  return delta;
}

/**
 *
 * Takes in a curve, returns the index of puncture point contained
 * between pt0 and pt1.
 *
 **/
int FindMinimizationIndex( avtPoincareIC *ic )
{
  if (4 <= RATIONAL_DEBUG)
    cerr << "Finding Minimization Index...\n";

  std::vector<avtVector> puncturePoints;
  FieldlineLib fieldlib;
  fieldlib.getPunctures(ic->points, avtVector(0, 1, 0), puncturePoints);

  if (puncturePoints.size() < 2*ic->properties.toroidalWinding)
    {
      cerr << "Not enough puncture points to find correct Rational Surface Minimization index. Failing.\n";
      cerr << "need "<<2*ic->properties.toroidalWinding<<" puncture pts at least.\n";
      return -1;
    }

  avtVector seedPt = ic->properties.srcPt;
  double min = 99999999;
  int minix = -1;
  for (unsigned int i =0; i < ic->properties.toroidalWinding && i < puncturePoints.size(); i++)
    {
      avtVector puncture = puncturePoints[i];
      double dist = (seedPt - puncture).length();
      if (min > dist)
        {
          min = dist;
          minix = i;
        }
    }
  if (minix > -1 && 3 <= RATIONAL_DEBUG)
    cerr <<"LINE: "<<__LINE__<<"found " << minix <<", "<< min <<"\n";
  return minix;
  
  if (puncturePoints.size() > 0)
    return 0;
  else 
    return -1;
}
double FindMinimizationDistance( avtPoincareIC* ic)
{
  if (4 <= RATIONAL_DEBUG)
    cerr << "Finding Minimization Distance...\n";

  std::vector<avtVector> puncturePoints;
  FieldlineLib fieldlib;
  fieldlib.getPunctures(ic->points, avtVector(0, 1, 0), puncturePoints);

  if (puncturePoints.size() < 2*ic->properties.toroidalWinding)
    {
      cerr << "Not enough puncture points to find correct Rational Surface Minimization distance. Failing.\n";
      cerr << "need "<<2*ic->properties.toroidalWinding<<" puncture pts at least.\n";
      return -1;
    }

  int ix = FindMinimizationIndex(ic);

  avtVector puncture1 = puncturePoints[ix];
  avtVector puncture2 = puncturePoints[ix + ic->properties.toroidalWinding];

  double dist = (puncture1 - puncture2).length();

  return dist;
}

// can use a,b, c too
void
PickBestAndSetupToDraw(avtPoincareIC *x0, avtPoincareIC *x1, avtPoincareIC *x2, avtPoincareIC *x3, std::vector<int> &ids_to_delete)
{
  avtPoincareIC *seed = x0->src_seed_ic;
  std::vector<avtPoincareIC *> *children = x0->src_rational_ic->properties.children;
  
  // Find distance between puncture points
  double x0_dist = FindMinimizationDistance(x0);
  double x1_dist = FindMinimizationDistance(x1);
  double x2_dist = FindMinimizationDistance(x2);
  double x3_dist = 9999999999999999.9999999999;
  bool bracketing = false;
  if (x3 != NULL)
    x3_dist = FindMinimizationDistance(x3);
  else
    bracketing = true;
 
  bool use0 = x0_dist <= x1_dist &&
    x0_dist <= x2_dist &&
    x0_dist <= x3_dist;
  bool use1 = !use0 &&
    x1_dist <= x0_dist &&
    x1_dist <= x2_dist &&
    x1_dist <= x3_dist;
  bool use2 = !use0 && !use1 && 
    x2_dist <= x1_dist &&
    x2_dist <= x0_dist &&
    x2_dist <= x3_dist;
  bool use3 = x3!= NULL && !use0 && !use1 && !use2 && 
    x3_dist <= x1_dist &&
    x3_dist <= x2_dist &&
    x3_dist <= x0_dist;
  
  // should only be one
  int ct = 0;
  if (use0)ct++;
  if (use1)ct++;
  if (use2)ct++;
  if (use3)ct++;
  
  if (ct > 1)
    std::cerr << "Line: " << __LINE__ << "Too many points going to be used here!!\n";
  
  if (!use0)
    {
      if (2 <= RATIONAL_DEBUG)
        std::cerr << "Line: " << __LINE__ << "Deleting x0\n";
      ids_to_delete.push_back(x0->id);
      if(std::find(children->begin(), children->end(), x0) != children->end())
        children->erase(std::remove(children->begin(), children->end(), x0), children->end());
    }
  else
    {
      x0->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
      x0->properties.searchState = FieldlineProperties::WAITING_SEED;
      
      if (std::find(children->begin(), children->end(), seed) != children->end())
        {
          if (seed->id != x0->id)
            {
              std::replace(children->begin(),children->end(),seed,x0);
              if (2 <= RATIONAL_DEBUG)
                std::cerr << "LINE " << __LINE__ << "  " << "x0 ID: "<<x0->id<<" swapped with seed ID: " <<seed->id << std::endl;
              ids_to_delete.push_back(seed->id);
            }
          else if (2 <= RATIONAL_DEBUG)
            std::cerr << "LINE " << __LINE__ << "  " << "original seed was already minimum" << std::endl;
        }
      else if (2 <= RATIONAL_DEBUG)
        std::cerr << "LINE " << __LINE__ << "Where's the seed...?\n";
      
      
    }
  if (!use1)
    {
      if (2 <= RATIONAL_DEBUG)
        std::cerr << "Line: " << __LINE__ << "Deleting x1\n";
      ids_to_delete.push_back(x1->id);
      if(std::find(children->begin(), children->end(), x1) != children->end())
        children->erase(std::remove(children->begin(), children->end(), x1), children->end());
    }
  else
    {
      x1->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
      x1->properties.searchState = FieldlineProperties::WAITING_SEED;
      
      if (std::find(children->begin(), children->end(), seed) != children->end())
        {
          if (seed->id != x1->id)
            {
              std::replace(children->begin(),children->end(),seed,x1);
              if (2 <= RATIONAL_DEBUG)
                std::cerr << "LINE " << __LINE__ << "  " << "x1 ID: "<<x1->id<<" swapped with seed ID: " <<seed->id << std::endl;
              ids_to_delete.push_back(seed->id);
            }
          else if (2 <= RATIONAL_DEBUG)
            std::cerr << "LINE " << __LINE__ << "  " << "original seed was already minimum" << std::endl;
        }
      else if (2 <= RATIONAL_DEBUG)
        std::cerr << "LINE " << __LINE__ << "Where's the seed...?\n";
      
    }
  if (!use2)
    {
      if (2 <= RATIONAL_DEBUG)
        std::cerr << "Line: " << __LINE__ << "Deleting x2\n";
      ids_to_delete.push_back(x2->id);
      if(std::find(children->begin(), children->end(), x2) != children->end())
        children->erase(std::remove(children->begin(), children->end(), x2), children->end());                
    }
  else
    {
      x2->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
      x2->properties.searchState = FieldlineProperties::WAITING_SEED;
      
      if (std::find(children->begin(), children->end(), seed) != children->end())
        {
          if (seed->id != x2->id)
            {
              std::replace(children->begin(),children->end(),seed,x2);
              if (2 <= RATIONAL_DEBUG)
                std::cerr << "LINE " << __LINE__ << "  " << "x2 ID: "<<x2->id<<" swapped with seed ID: " <<seed->id << std::endl;
              ids_to_delete.push_back(seed->id);
            }
          else if (2 <= RATIONAL_DEBUG)
            std::cerr << "LINE " << __LINE__ << "  " << "original seed was already minimum" << std::endl;
        }
      else if (2 <= RATIONAL_DEBUG)
        std::cerr << "LINE " << __LINE__ << "Where's the seed...?\n";
    }
  if (!bracketing && !use3)
    {
      if (2 <= RATIONAL_DEBUG)
        std::cerr << "Line: " << __LINE__ << "Deleting x3\n";
      ids_to_delete.push_back(x3->id);
      if(std::find(children->begin(), children->end(), x3) != children->end())
        children->erase(std::remove(children->begin(), children->end(), x3), children->end());
    }
  else if (!bracketing)
    {
      x3->properties.analysisMethod = FieldlineProperties::RATIONAL_SEARCH;
      x3->properties.searchState = FieldlineProperties::WAITING_SEED;
      
      if (std::find(children->begin(), children->end(), seed) != children->end())
        {
          if (seed->id != x3->id)
            {
              std::replace(children->begin(),children->end(),seed,x3);
              if (2 <= RATIONAL_DEBUG)
                std::cerr << "LINE " << __LINE__ << "  " << "x3 ID: "<<x3->id<<" swapped with seed ID: " <<seed->id << std::endl;
              ids_to_delete.push_back(seed->id);
            }
          else if (2 <= RATIONAL_DEBUG)
            std::cerr << "LINE " << __LINE__ << "  " << "original seed was already minimum" << std::endl;
        }
      else if (2 <= RATIONAL_DEBUG)
        std::cerr << "LINE " << __LINE__ << "Where's the seed...?\n";
    }
}

std::string VectorToString(avtVector &vec)
{
  std::ostringstream ss;
  ss << std::setprecision(std::numeric_limits<double>::digits10+6);

  ss << vec[0] <<" ";
  ss << vec[1] <<" ";
  ss << vec[2];

  std::string s(ss.str());
  return s;
}
