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
 *  StreamlineAnalyzerLib.cc:
 *
 *  Written by:
 *   Allen Sanderson
 *   SCI Institute
 *   University of Utah
 *   September 2006
 *
 */

#include <StreamlineAnalyzerLib.h>

#include <algorithm>
#include <iostream>
#include <map>

#include <float.h>

#ifdef STRAIGHTLINE_SKELETON
#include "skelet.h"
#endif

using namespace std;

namespace FusionPSE {

#define SIGN(x) ((x) < 0.0 ? (int) -1 : (int) 1)



#ifdef COMMENT_OUT
/////////// Begin Guoning Code


/*
   This routine computes the mean and standard deviation
        for a sequence of winding pairs.
        Added by Guoning 10/05/2010
*/
void
standard_deviation_analysis( vector< int > t_windings,
                             vector< int > p_windings,
                             int truncate_size,
                             double &mean_saftyFactor,
                             double &max_saftyFactor,
                             double &min_saftyFactor,
                             vector< double > &deviations,
                             double &std_deviation)
{
  vector < double > vals;
  unsigned int i;
        
  /*cerr << " numbers of values in t_windings and p_windings are (" <<
    t_windings.size() << ", " << p_windings.size() << ")" << endl;
    cerr << " The input pairs are " << endl;

    for (i=0; i<t_windings.size(); i++)
    {
    cerr << "(" << t_windings[i] << ", "
    << p_windings[i] << ")" << endl;
    }
  */

  // we first compute the estimated slope (i.e. safety factor) for each pair
  for (i=0; i<t_windings.size(); i++)
  {
    if (p_windings[i] == 0)
      vals.push_back(0.);
    else
      vals.push_back((double)t_windings[i]/(double)p_windings[i]);
  }

  // we need to sort the obtaining safety factors in 'vals' and remove the front few ones
  
  //cerr << "start sorting the vals ..." << endl;
  double *temp_vals_array = new double[vals.size()];
  for (i=0; i<vals.size(); i++)
    temp_vals_array[i]=vals[i];
  quickSort <double> (temp_vals_array, 0, vals.size()-1);
  //cerr << "finish sorting. " << endl;
  for (i=0; i<vals.size(); i++)
    vals[i]=temp_vals_array[i];
        
  delete [] temp_vals_array;
        
  // Here we may remove the max and min or multiple max and min_dist
  //int truncate_size = 2;
  for (i=0; i<truncate_size; i++)
  {
    if (vals.size()>2)
      vals.erase(vals.begin());
  }
  
  for (i=0; i<truncate_size; i++)
  {
    if (vals.size()>2)
      vals.erase(vals.begin()+vals.size()-1);
  }
  
  // search for the max and min slope for the later envelope construction
  min_saftyFactor = 10000.;
  max_saftyFactor = 0.;
  for (i=0; i<vals.size(); i++)
  {
    if (vals[i]>max_saftyFactor) max_saftyFactor = vals[i];
    if (vals[i]<min_saftyFactor) min_saftyFactor = vals[i];
  }

  // second, we compute the mean
  mean_saftyFactor = 0.;
  for (i=0; i<vals.size(); i++)
    mean_saftyFactor += vals[i];

  mean_saftyFactor /= vals.size();
  //cerr << "current mean safetyFactor is " << mean_saftyFactor << endl;

  // third, we compute the deviation of each estimated slope to the mean
  deviations.clear();
  for (i=0; i<vals.size(); i++)
    deviations.push_back(vals[i]-mean_saftyFactor);
  //cerr << "current number of values in vals is " << vals.size() << endl;
                
  ////////////////////////////////////////////////////////////////////
  // additional step, we remove the maximal positive and negative deviation

/*      double max_positive, max_negative;
        max_positive = -10000.;
        max_negative = 10000.;
        for (i=0; i<vals.size(); i++)
        {
                if (max_positive<vals[i]) max_positive=vals[i];

                if (max_negative>vals[i]) max_negative=vals[i];
        }
        
        for (i=0; i<vals.size(); i++)
        {
                if (vals[i]>=max_positive) vals.erase(vals.begin()+i);

                if (vals[i]<=max_negative) vals.erase(vals.begin()+i);
        }

        min_saftyFactor = 10000.;
        max_saftyFactor = 0.;
        for (i=0; i<vals.size(); i++)
        {
                if (vals[i]>max_saftyFactor) max_saftyFactor = vals[i];
                if (vals[i]<min_saftyFactor) min_saftyFactor = vals[i];
        }

        // second, we compute the mean
        mean_saftyFactor = 0.;
        for (i=0; i<vals.size(); i++)
                mean_saftyFactor += vals[i];

        mean_saftyFactor /= vals.size();
        //cerr << "current mean safetyFactor is " << mean_saftyFactor << endl;
        
        // third, we compute the deviation of each estimated slope to the mean
        deviations.clear();
        for (i=0; i<vals.size(); i++)
                deviations.push_back(vals[i]-mean_saftyFactor);
        
        //cerr << "current number of values in vals is " << vals.size() << endl;
*/

  ////////////////////////////////////////////////////////////////////
  // compute the square of each deviation
  for (i=0; i<deviations.size(); i++)
    deviations[i] *= deviations[i];

  // fourth, we compute the standard deviation by summing all the
  // above deviations and taking the square root of the sum
  std_deviation = 0.;
  for (i=0; i<deviations.size(); i++)
    std_deviation += deviations[i];
  
  std_deviation = sqrt(std_deviation);
}


/*
  This routine is used to removed redundant poloidal entries
*/
void
windingPairs_cleanUp(vector <int> &toroidal, vector <int> &poloidal)
{
  unsigned int i;
  
  // we first remove all the zero poloidal winding
  for (i=0; i<poloidal.size(); i++)
  {
    if (poloidal[0]==0)
    {
      poloidal.erase(poloidal.begin());
      toroidal.erase(toroidal.begin());
    }
    else
      break;
  }
  
  int cur_poloidal = poloidal[0];
  
  for (i=1; i<toroidal.size(); i++)
  {
    if (poloidal[i]==cur_poloidal)
    {
      poloidal.erase(poloidal.begin()+i);
      toroidal.erase(toroidal.begin()+i);
      i--;
    }
    
    else
      cur_poloidal = poloidal[i];
  }
}

/*
  For better estimation of safety factor, we remove the first half of
  the pairs.  Note that this routine should be called before calling
  the windingPairs_cleanUp to remove the redundant pairs (with the
  same poloidal windings, i.e. flat).
*/
void
remove_first_half_pairs(vector <int> &toroidal, vector <int> &poloidal)
{
  vector <int> half_tor;
  vector <int> half_pol;
  int half_pos = toroidal.size()/2;
  unsigned int i;

  // Take the second half
  for (i=half_pos; i<toroidal.size(); i++)
  {
    half_tor.push_back (toroidal[i]);
    half_pol.push_back (poloidal[i]);
  }

  // Store the second half to the original arrays
  toroidal.clear();
  poloidal.clear();
  for (i=0; i<half_tor.size(); i++)
  {
    toroidal.push_back (half_tor[i]);
    poloidal.push_back (half_pol[i]);
  }

  half_tor.clear();
  half_pol.clear();
}

#endif

// A simple linear regression using linear lease square fitting
template< class TYPE >
void least_square_fit(vector< TYPE > p_windings,
                      double &bestfit_safetyFactor)
{
  unsigned int start = p_windings.size() *.75;
  unsigned int stop  = p_windings.size();

  unsigned int cc = 0;

  double t_mean = 0;
  for (unsigned int i=start; i<stop; i++, ++cc)
    t_mean += i;

  t_mean /= (double) cc;

  double t_square_sum = 0;
  for (unsigned int i=start; i<stop; i++)
    t_square_sum += (double) (i * i);

  double SS_xx = t_square_sum-cc*t_mean*t_mean;

  double p_mean = 0;
  for (unsigned int i=start; i<stop; i++)
    p_mean += p_windings[i];

  p_mean /= (double) cc;

  double p_square_sum = 0;
  for (unsigned int i=start; i<stop; i++)
    p_square_sum += p_windings[i]*p_windings[i];

  double SS_yy = p_square_sum-cc*p_mean*p_mean;
  double SS_xy = 0;

  for (int i=start; i<stop; i++)
    SS_xy += (i-t_mean)*(p_windings[i]-p_mean);

  bestfit_safetyFactor = SS_xx/SS_xy;
}


/////////// End Guoning Code







Point FieldlineLib::interpert( Point lastPt, Point currPt, double t ) {

  return Point( Vector( lastPt ) + Vector( currPt - lastPt ) * t );
}


int FieldlineLib::ccw( Vector v0, Vector v1 ) {
    
  if( v0.x * v1.z - v0.z * v1.x > FLT_MIN ) return  1;    // CCW
  if( v0.z * v1.x - v0.x * v1.z > FLT_MIN ) return -1;    // CW
  if( v0.x * v1.x < 0.0 || v0.z * v1.z < 0.0 ) return -1; // CW
    
  if( v0.x*v0.x+v0.z*v0.z >=
      v1.x*v1.x+v1.z*v1.z ) return 0;                     //  ON LINE
    
  return 1;                                               //  CCW
}


int FieldlineLib::intersect( Point l0_p0, Point l0_p1,
                             Point l1_p0, Point l1_p1 )
{
  //  See if the lines intersect.    
  if( ( ccw( Vector(l0_p1-l0_p0), Vector(l1_p0-l0_p0)) *
        ccw( Vector(l0_p1-l0_p0), Vector(l1_p1-l0_p0) ) <= 0 ) &&
      ( ccw( Vector(l1_p1-l1_p0), Vector(l0_p0-l1_p0)) *
        ccw( Vector(l1_p1-l1_p0), Vector(l0_p1-l1_p0) ) <= 0 ) ) {
        
    //  See if there is not a shared point.
    if( l0_p0 != l1_p0 && l0_p0 != l1_p1 &&
        l0_p1 != l1_p0 && l0_p1 != l1_p1 )
      return 1;
        
    //  See if there is a shared point.
    else if( l0_p0 == l1_p0 || l0_p0 == l1_p1 ||
             l0_p1 == l1_p0 || l0_p1 == l1_p1 )
      return 2;
        
    //  There must be a point that is on the other line.
    else
      return 3;
  }
    
  //  Lines do not intersect.
  return 0;
}


void FieldlineLib::convexHull( vector< pair< Point, unsigned int > > &hullPts,
                               unsigned int &m, // starting index
                               unsigned int npts, // number of points
                               int dir )
{
  // Three or less points so which is a hull
  if( npts - m < 3 ) {
    m = npts;
    return;
  }

  unsigned int min = m;

  // Find the point with the minimum z value.
  for( unsigned int i=m; i<npts; i++ ) {
    if( hullPts[ min ].first.z > hullPts[i].first.z )
      min = i;
  }

  // Store the minimum point so that we know when we are done.
  hullPts[npts] = hullPts[min];

  do {

    // Make the point with the smallest z value first.
    if( min != m ) {
      pair< Point, unsigned int > tmpPt = hullPts[m];
      hullPts[m] = hullPts[min];
      hullPts[min] = tmpPt;
    }

    m++;
    min = m;

    // Find the next point that is the farthest to the right of all others.
    for( unsigned int i=min+1; i<npts+1; i++ ) {

      Vector v0 = (Vector) hullPts[m-1].first - (Vector) hullPts[min].first;
      Vector v1 = (Vector) hullPts[i  ].first - (Vector) hullPts[min].first;

      int CCW = ccw( v0, v1 );

      // The next point that is the farthest to the right of all
      // others will be in a clockwise direction (i.e. the convex
      // hull is in the counter clockwise direction.
      if( CCW == dir )
        min = i;

      // In this case the points are co-linear so take the point
      // that is closest to the starting point.
      else if( hullPts[i].second != hullPts[m-1].second && CCW == 0 ) {
        v1 = (Vector) hullPts[m-1].first - (Vector) hullPts[i].first;

        if( v0.length2() > v1.length2() )
          min = i;
      }
    }

    // Stop when the first point is found again.
  } while( min != npts );

//   for( unsigned int i=0; i<m; ++i ) {
//     if( verboseFlag )
//       cerr << "convexHull " << hullPts[i].second << endl;
//   }

//   if( verboseFlag )
//     cerr << endl;
}


bool FieldlineLib::hullCheck( vector< Point > &points, int &direction )
{
  // If one, two, or three points the ordering makes no difference and
  // it is convex.
  if( points.size() <= 2 ) {
      return true;
  }
  else if( points.size() == 3 ) {
    return ccw( points[0] - points[1], points[2] - points[1] );
  }

  vector< pair < Point, unsigned int > > pts;
  vector< pair < Point, unsigned int > > hullPts;

  // Store the points and their original order in a temporary vector.
  for( unsigned int i=0; i<points.size(); i++ )
    pts.push_back( pair< Point, unsigned int >( points[i], i ) );

  unsigned int npts = chainHull_2D( pts, hullPts, 0 );

  unsigned int cc = 0;

  for( unsigned int i=0; i<npts-1; i++ )
  {
    // The hull points are returned in a clockwise order. So if the
    // point ordering is increasing then the point ordering is also in
    // clockwise.
    if( hullPts[i].second < hullPts[i+1].second+1)
      ++cc;
  }
   
  // The hull is found in a clockwise direction if the point count is
  // increasing.
  direction = ((cc > npts/2) ? 1 : -1);

  return (npts == points.size());
}


// Determine if a number is prime - return zero otherwise the GCD.
unsigned int FieldlineLib::isPrime( unsigned int a )
{
  unsigned int gcd = 0;

  for( unsigned int i=2; i<a/2; ++i )
  {
    if( a % i == 0 )
      gcd = i;
  }

  return gcd;
}


// Find the great comon denominator.
unsigned int FieldlineLib::GCD( unsigned int a, unsigned int b )
{
  if( a < b )
    { unsigned int tmp = a; a = b; b = tmp; }

  if( a % b )
    return GCD(b, a % b);
  else
    return b;
}


// Find the great comon denominator in a list of numbers.
unsigned int FieldlineLib::GCD( vector< unsigned int > values,
                                unsigned int &freq,
                                unsigned int minGCD )
{
  if( values.size() == 1 )
  {
    freq = 1;
    return values[0];
  }

  // Find the greatest common denominator between each value in the list.
  map< int, int > GCDCount;  
  map<int, int>::iterator ic;
  
  for( unsigned int i=0; i<values.size(); ++i )
  {
    for( unsigned int j=i+1; j<values.size(); ++j )
    {
      unsigned int gcd = GCD( values[i], values[j] );
      
      // Find the GCD excluding those smaller than the min.
      if( gcd >= minGCD )
      {
        ic = GCDCount.find( gcd );
        
        if( ic == GCDCount.end() )
          GCDCount.insert( pair<int, int>( gcd, 1) );
        else (*ic).second++;
      }
    }
  }
    
  // Find the most frequent greatest common denominator
  unsigned int gcd = 1;
  freq = 0;

  ic = GCDCount.begin();

  while( ic != GCDCount.end() )
  {
    // Two GCD have the same count so take the larger GCD.
    if( freq == (*ic).second && gcd < (*ic).first )
    {
      gcd = (*ic).first;
    }

    // GCD with a larger count
    else if( freq < (*ic).second )
    {
      gcd = (*ic).first;
      
      freq = (*ic).second;
    }

    ++ic;
  }
  
  return gcd;
}


// Find the resonance via the great common denominator in a list of
// samples.
unsigned int FieldlineLib::
ResonanceCheck( vector< pair< unsigned int, double > > &stats,
               unsigned int baseResonance,
               unsigned int max_samples )
{
  unsigned int freq;
  vector< unsigned int > values;

  unsigned int max_groups = stats.size() / max_samples;
  unsigned int mult = 0;

  map< int, int > GCDCount;  
  map<int, int>::iterator ic;

  // For a secondary resonance to exists the group times the
  // baseResonance should equal the resonance.
  for( unsigned int group=max_groups; group>1; --group )
  {
    unsigned int num_entries = stats.size() / group;

    values.resize( num_entries );

    for( unsigned int i=0; i<num_entries; ++i )
      values[i] = stats[i].first;

    unsigned int gcd = GCD( values, freq );

    // Store the GCD and counts just in case while in the chaotic
    // regime we are not able to get the proper GCD,
    // i.e. gcd  == group * baseResonance fails.
    ic = GCDCount.find( gcd );
        
    if( ic == GCDCount.end() )
      GCDCount.insert( pair<int, int>( gcd, 1) );
    else (*ic).second++;

    if( gcd  == group * baseResonance )
    {
      unsigned int cc = 0;

      // Make the majority of the integer mutliples can be divided by
      // the resonance. Sometimes odd gcd values can sneak in as well
      // as odd resonances when near a separtrice.
      for( unsigned int i=0; i<num_entries; ++i )
      {
        if( values[i] % gcd == 0 )
          ++cc;
      }

      // Set a 0.75 threshold for now.
      if( (double) cc / (double) num_entries > 0.75)
      {
        mult = group;
        break;
      }
    }
  }

  // Probably chaotic so just use the most frequent GCD. 
  if( mult == 0 )
  {
    // Find the most frequent greatest common denominator
    freq = 0;

    ic = GCDCount.begin();

    while( ic != GCDCount.end() )
    {
      // Two GCD have the same count so take the larger GCD.
      if( freq == (*ic).second && mult < (*ic).first )
      {
        mult = (*ic).first;
      }
      
      // GCD with a larger count
      else if( freq < (*ic).second )
      {
        mult = (*ic).first;
        
        freq = (*ic).second;
      }
      
      ++ic;
    }
  }
  
  // The multiplier is not greater than 1 then there is no second
  // order resonance present.
  if( mult > 1 )
  {
    unsigned int entries = stats.size() / mult;

    // Divide the stats up into groups and keep the first two groups.
    if( mult > 2 )
    {
      if( verboseFlag )
        cerr << "Splitting into " << mult << " groups " << endl;
      
      unsigned int num_entries = 2 * stats.size() / mult;
      
      if( num_entries && num_entries < stats.size() )
      {
        if( verboseFlag )
          cerr << "Erasing entries " << num_entries << " to "
               << stats.size() << endl;
        
        stats.resize( num_entries );
      }
    }
    else
    {
      if( verboseFlag )
        cerr << "No splitting needed for " << mult << " groups " << endl;
    }

    // If present, thresholding should leave only higher order
    // resonances.
    thresholdStats( stats, true, 2 );
        
//  values.resize( stats.size() );
    values.resize( entries );
        
//  for( unsigned int i=0; i<stats.size(); ++i )
    for( unsigned int i=0; i<entries; ++i )
      values[i] = stats[i].first;
    
    unsigned int resonance = GCD( values, freq );

    return mult * baseResonance;
  }
  else
    return 1;
}

void FieldlineLib::
thresholdStats( vector< pair< unsigned int, double > >& stats,
                bool erase,
                unsigned int checkType )
{
  Otsu otsu;
  double maxVet, cutoff;

  otsu.getOtsuThreshold2( stats, cutoff, maxVet );

//  unsigned int cutoffIndex = stats.size();
  unsigned int cutoffIndex;

  if( erase ) cutoffIndex = stats.size() / 2;
  else        cutoffIndex = stats.size();

  for( unsigned int i=0; i<stats.size(); ++i )
  {
//    if( stats[i].second <= cutoff ) 
    if( i < cutoffIndex ) 
    {
      if( verboseFlag )
        cerr << "period  " << stats[i].first << "  "
             <<  (checkType == 2 ? "distance  " :  "variance  " )
             << stats[i].second << "  "
             << endl;
    }
    else
    {
      cutoffIndex = i;
      break;
    }

    if( stats[i].second <= cutoff && stats[i+1].second > cutoff )
    {
      if( verboseFlag )
        cerr << "vet max  " << maxVet << "  " << "Threshold " << cutoff << endl;
      }
  }

  for( unsigned int i=cutoffIndex; i<stats.size(); ++i )
  {
    if( verboseFlag )
    {
      // Print the cutoff entries
      cerr << "period  " << stats[i].first << "  "
           <<  (checkType == 2 ? "distance  " :  "variance  " )
           << stats[i].second
           << (erase ? "  **" : "  *") << endl;
    }

    if( stats[i].second <= cutoff && stats[i+1].second > cutoff )
    {
      if( verboseFlag )
        cerr << "vet max  " << maxVet << "  " << "Threshold " << cutoff << endl;
    }
  }

  if( erase && cutoffIndex > 1 )
    stats.resize(cutoffIndex);
}


Point FieldlineLib::circle(Point &pt1, Point &pt2, Point &pt3)
{
  if (!IsPerpendicular(pt1, pt2, pt3) )
    return CalcCircle(pt1, pt2, pt3);   
  else if (!IsPerpendicular(pt1, pt3, pt2) )
    return CalcCircle(pt1, pt3, pt2);   
  else if (!IsPerpendicular(pt2, pt1, pt3) )
    return CalcCircle(pt2, pt1, pt3);   
  else if (!IsPerpendicular(pt2, pt3, pt1) )
    return CalcCircle(pt2, pt3, pt1);   
  else if (!IsPerpendicular(pt3, pt2, pt1) )
    return CalcCircle(pt3, pt2, pt1);   
  else if (!IsPerpendicular(pt3, pt1, pt2) )
    return CalcCircle(pt3, pt1, pt2);   
  else
    return Point(-1,-1,-1);
}


// Check the given point are perpendicular to x or y axis 
bool FieldlineLib::IsPerpendicular(Point &pt1, Point &pt2, Point &pt3)
{
  double d21z = pt2.z - pt1.z;
  double d21x = pt2.x - pt1.x;
  double d32z = pt3.z - pt2.z;
  double d32x = pt3.x - pt2.x;
        
  // checking whether the line of the two pts are vertical
  if (fabs(d21x) <= FLT_MIN && fabs(d32z) <= FLT_MIN)
    return false;
    
  else if (fabs(d21z) < FLT_MIN ||
           fabs(d32z) < FLT_MIN ||
           fabs(d21x) < FLT_MIN ||
           fabs(d32x)<= FLT_MIN )
    return true;

  else
    return false;
}

Point FieldlineLib::CalcCircle(Point &pt1, Point &pt2, Point &pt3)
{
  double d21z = pt2.z - pt1.z;
  double d21x = pt2.x - pt1.x;
  double d32z = pt3.z - pt2.z;
  double d32x = pt3.x - pt2.x;
        
  if (fabs(d21x) < FLT_MIN && fabs(d32z ) < FLT_MIN ) {

    return Point( ( 0.5*(pt2.x + pt3.x) ),
                  ( pt1.y ),
                  ( 0.5*(pt1.z + pt2.z) ) );
  }
        
  // IsPerpendicular() assure that xDelta(s) are not zero
  double aSlope = d21z / d21x;
  double bSlope = d32z / d32x;

  // checking whether the given points are colinear.    
  if (fabs(aSlope-bSlope) > FLT_MIN) {
    
    // calc center
    double x = (aSlope*bSlope*(pt1.z - pt3.z) +
                bSlope*(pt1.x + pt2.x) -
                aSlope*(pt2.x + pt3.x) ) / (2.0* (bSlope-aSlope) );

    return Point( x,
                  pt1.y,
                  -(x - (pt1.x+pt2.x)/2.0) / aSlope + (pt1.z+pt2.z)/2.0 );
  }

  return Point(0,0,0);
}


bool
FieldlineLib::IntersectCheck( vector< Point >& points,
                              unsigned int nbins,
                              unsigned int offset )
{
  Vector v0 = (Vector) points[ nbins] - (Vector) points[0];
  Vector v1 = (Vector) points[offset] - (Vector) points[0];

  // If the offset and point ordering have the same directions then
  // the next group is the offset. Otherwise if they are in the
  // opposite direction then nbins-offset is the next group.
  //int offsetDir = Dot( v0, v1 ) > 0.0 ? 1 : -1;
  for( unsigned int i=0, j=nbins; i<nbins && j<points.size(); ++i, ++j )
  {
    Point l0_p0 = points[i];
    Point l0_p1 = points[j];

    // The neighbor groups
    unsigned int skipNext     = (i + offset + nbins) % nbins;
    unsigned int skipPrevious = (i - offset + nbins) % nbins;

    for( unsigned int k=0, l=nbins; k<nbins && l<points.size(); ++k, ++l )
    {
      // Do not check the segment against itself or its immediate
      // neighbors.
      if( k == i || k == skipNext || k == skipPrevious )
        continue;

      Point l1_p0 = points[k];
      Point l1_p1 = points[l];

//       cerr << nbins << "  " << offsetDir << "    "
//         << i << "  " << j << "  " << k << "  " << l << endl;

      if( intersect( l0_p0, l0_p1, l1_p0, l1_p1 ) == 1)
        return false;
    }
  }

  return true;
}


unsigned int FieldlineLib::Blankinship( unsigned int toroidalWinding,
                                        unsigned int poloidalWinding,
                                        unsigned int offset /* = 1 */ )
{

  unsigned int skip;

  if( toroidalWinding > 1 && poloidalWinding != 0 ) {
    //  To find the skip find the mutual primes via the
    //  Blankinship Algorithm.
    for( skip=1; skip<toroidalWinding; skip++ )
      if( (skip * poloidalWinding) % toroidalWinding == offset )
        break;
    
    if( skip == toroidalWinding )
      skip = 1;
    
  } else {
    skip = 0;
  }

  return skip;
}


template< class TYPE >
void FieldlineLib::safetyFactorStats( vector< TYPE > &poloidalWindingCounts,
                                      double &averageSafetyFactor,
                                      double &stdDev )
{
  unsigned int start = poloidalWindingCounts.size() * .75;
  unsigned int stop  = poloidalWindingCounts.size();

  unsigned int cc = 0;

  averageSafetyFactor = 0;

  for( unsigned int i=start; i<stop; ++i, ++cc )
    averageSafetyFactor += (double) i / (double) poloidalWindingCounts[i];

  averageSafetyFactor /= (double) cc;

  stdDev = 0;

  for( unsigned int i=start; i<stop; ++i )
  {
    double diff =
      averageSafetyFactor - (double) i / (double) poloidalWindingCounts[i];

    stdDev += diff * diff;
  }

  stdDev /= (double) cc;

  stdDev = sqrt(stdDev);
}


int compareWindingPairs( const WindingPair s0, const WindingPair s1 )
{
  return (s0.stat > s1.stat);
}


void FieldlineLib::
SortWindingPairs( vector< WindingPair > &windingPairs, bool reverseOrder )
{
  // Now sort the results.
  sort( windingPairs.begin(), windingPairs.end(), compareWindingPairs );

  if( reverseOrder )
    reverse( windingPairs.begin(), windingPairs.end() );
}


void FieldlineLib::
RankWindingPairs( vector< WindingPair > &windingPairs, bool LT )
{
  if( windingPairs.size() < 2 )
    return;

  // Now rank the results.
  unsigned int rank = 0;

  windingPairs[0].ranking = rank;

  for( unsigned int i=1; i<windingPairs.size(); ++i )
  {
    if( ( LT && windingPairs[i].stat < windingPairs[i-1].stat) ||
        (!LT && windingPairs[i].stat > windingPairs[i-1].stat) )
      ++rank;

    windingPairs[i].ranking = rank;
  }
}


void FieldlineLib::
poloidalWindingCheck( vector< unsigned int > &poloidalWindingCounts,
                      vector< WindingPair > &windingPairs )
{
  windingPairs.clear();

  unsigned int nsets = poloidalWindingCounts.size();

  // The premise is that for a given toroidal winding the poloidal
  // winding should be consistent between each Nth punction point,
  // where N is the toroidal winding. For instance, if the toroidal
  // winding is 5 and the poloidal winding is 2. Then the pattern
  // could be:

  // 0 1 1 1 2 - 2 3 3 3 4 - 4 5 5 5 6

  // In this case the different between every 5th value (the toroidal
  // winding) should be 2 (the poloidal winding).

  unsigned int maxToroidalWinding = poloidalWindingCounts.size() / 2;

  for( unsigned int toroidalWinding=1;
       toroidalWinding<=maxToroidalWinding;
       ++toroidalWinding )
  {

    map< int, int > differenceCount;
    map< int, int >::iterator ic;

    // Find all the differences and count each one.
    for( unsigned int i=0; i<nsets-toroidalWinding; ++i)
    {
      // Get the poloidal winding between two counts.
      unsigned int poloidalWinding =
        poloidalWindingCounts[i+toroidalWinding] - poloidalWindingCounts[i];

      // Find this difference in the list.
      ic = differenceCount.find( poloidalWinding );

      // Not found, new difference.
      if( ic == differenceCount.end() )
        differenceCount.insert( pair<int, int>( poloidalWinding, 1) );
      // Found this difference, increment the count.
      else (*ic).second++;
    }

    // Find the difference that occurs most often.
    unsigned int nMatches = 0;
    unsigned int poloidalWinding = 0;
    
    ic = differenceCount.begin();
    
    while( ic != differenceCount.end() )
    {
      if( nMatches < (*ic).second )
      {
        poloidalWinding = (*ic).first;
        nMatches = (*ic).second;
      }

      ++ic;
    }

    double consistency = (double) nMatches / (double) (nsets-toroidalWinding);

    unsigned int t = toroidalWinding;
    unsigned int p = poloidalWinding;

    // If the toroidalWinding and poloidalWinding have a common
    // denominator find the greatest denominator and remove it.
//    if( t != p )
    if( 0 )
    {
      for( unsigned int d=p; d>1; --d) {
        if( t % d == 0 && p % d == 0 ) {
          t /= d;
          p /= d;
          
          d = p;
        }
      }
    }

    bool lowOrder = false;

    // Keep the low order toroidalWinding / poloidalWinding
    if( t != toroidalWinding && p != poloidalWinding )
    {
      for( unsigned int i=0; i<windingPairs.size(); ++i )
      {
        if( windingPairs[i].toroidal == t &&
            windingPairs[i].poloidal == p )
        {
          lowOrder = true;

          // If the consistency happens to be higher for the higher
          // order keep it instead. Typically the lower order math is
          // better.
          if( windingPairs[i].stat < consistency )
            windingPairs[i].stat = consistency;
          
          break;
        }
      }
    }

    // Did not fina a lower order match so record the set.
    if( ! lowOrder )
    {
      WindingPair windingPair;
      windingPair.toroidal = t;
      windingPair.poloidal = p;
      windingPair.stat = consistency;
      windingPair.ranking = 0;

      windingPairs.push_back( windingPair );
    }
  }

  // Now sort the results.
  SortWindingPairs( windingPairs );
  RankWindingPairs( windingPairs );
}


double FieldlineLib::
calculateSumOfSquares( vector< Point >& points,
                       unsigned int period,
                       unsigned int checkType )
{
  // Find the sum of squares for each of the periods. If the period is
  // correct the sum of squares should be a small compared to a wrong
  // value.

  double sum = 0;
  double tSamples = 0;
    
  for( unsigned int i=0; i<period; ++i )
  {
    Vector centroid(0,0,0);
    double length = 0;
    double nSamples = 0;
    
    if( checkType != 2 )
    {
      // Find the centroid of the points based on the period.
      for( unsigned int j=i; j<points.size(); j+=period )
      {
        // Centroid or Z difference.
        if( checkType == 0 || checkType == 1 )
        {
          centroid += points[j];
          ++nSamples;
        }

        // Find the average length of the distance between points
        // based on the period.
        else if( checkType == 3 && j>period )
        {
          length += (points[j]-points[j-period]).length();
          ++nSamples;
        }
      }
      
      if( checkType == 0 || checkType == 1 )
        centroid /= (double) nSamples;
      
      else if( checkType == 3 )
        length   /= (double) nSamples;
    }

    if( checkType == 2 || nSamples > 1 )
    {
      // Get the sum of squares for each bin.
      for( unsigned int j=i; j<points.size(); j+=period )
      {
        // Centroid difference
        if( checkType == 0 )
        {
          Vector diff = points[j] - centroid;
          sum += diff.length2();
          ++tSamples;
        }
        // Z difference
        else if( checkType == 1 )
        {
          double diff = points[j].z - centroid.z;
          sum += (diff * diff);
          ++tSamples;
        }

        // Length sum
        else if( checkType == 2 && j>period )
        {
          double len = (points[j]-points[j-period]).length();
          sum += len;
          ++tSamples;
        }

        // Length difference
        else if( checkType == 3 && j>period )
        {
          double diff = (points[j]-points[j-period]).length() - length;
          sum += (diff * diff);
          ++tSamples;
        }
      }
    }
  }

  if( checkType != 2 )
    return sum / tSamples;
  else
    return sum;
}

int
compareSecond( const pair< unsigned int, double > s0,
               const pair< unsigned int, double > s1 )
{
  return ( s0.second < s1.second );
}

void FieldlineLib::
periodicityStats( vector< Point >& points,
                  vector< pair< unsigned int, double > >& stats,
                  unsigned int max_period,
                  unsigned int checkType )
{
  stats.clear();

  // Find the base period variance.
  unsigned int best_period = points.size();
  double best_var = 1.0e9;

  double base_var = calculateSumOfSquares( points, 1, checkType );

  if( verboseFlag )
    cerr << "Base variance  " << base_var << endl;
  
  // Find the period with the best variance.
  if( max_period == 0 )
    max_period = points.size() / 2.0;

  for( unsigned int i=1; i<=max_period; ++i ) 
  {
    double var = calculateSumOfSquares( points, i, checkType );
    
    stats.push_back( pair< unsigned int, double > (i, var ) );

    if( best_var > var ) 
    {
      best_var = var;
      best_period = i;
    }
  }

  if( verboseFlag )
    cerr << "Best period " << best_period << "  "
         <<  (checkType == 2 ? "distance  " :  "variance  " )
         << calculateSumOfSquares( points, best_period, checkType )
         << endl;

  if( stats.size() == 0 )
    stats.push_back( pair< unsigned int, double > (best_period, best_var ) );

  // Now sort the results.
  sort( stats.begin(), stats.end(), compareSecond );

//   for( unsigned int i=0; i<stats.size(); ++i )
//   {
//     cerr << stats[i].first << "  " << stats[i].second << "  "
//       << 0 << "  " << stats[i].second << "  "
//       << (i<stats.size()/2 ? "  " : "**")
//       << endl;
//   }
}


bool FieldlineLib::
rationalCheck( vector< Point >& points,
               unsigned int toroidalWinding,
               unsigned int &nnodes,
               float delta)
{
  // Look at the distance between the centroid of each toroidal group
  // and the points that are in it. If it is smaller then the distance
  // between the points that make up the fieldine is assumed to be on
  // a rational surface.

  // Note: this distance, delta, is defined indirectly based on the
  // integration step size.

  for( unsigned int i=0; i<toroidalWinding; i++ ) {

    // Get the local centroid for the toroidal group.
    Vector localCentroid(0,0,0);

    unsigned int npts = 0;

    for( unsigned int j=i; j<points.size(); j+=toroidalWinding, ++npts )
      localCentroid += (Vector) points[j];

    localCentroid /= (float) npts;

    for( unsigned int j=i; j<points.size(); j+=toroidalWinding ) {

      Vector vec = (Vector) points[j] - localCentroid;

      if( vec.length() > delta ) {
        return false;
      }
    }
  }

  nnodes = 1;

  return true;
}


bool FieldlineLib::
islandChecks( vector< Point >& points,
              unsigned int toroidalWinding,
              unsigned int &islands,
              unsigned int &nnodes,
              bool &complete )
{
  islands = 0;
  nnodes = 0;
  complete = 0;

  unsigned int npts;

  // Get the centroid for all of the points.
  Vector globalCentroid(0,0,0);

  for( unsigned int i=0; i<points.size(); i++ )
    globalCentroid += (Vector) points[i];

  globalCentroid /= (float) points.size();

  if( toroidalWinding == 1 )
  {
    // Check to see if it overlaps itself - it should or more points
    // need to be obtained.
    npts = points.size() / toroidalWinding;

    for( unsigned int j=toroidalWinding, k=j+toroidalWinding;
         k<points.size();
         j+=toroidalWinding, k+=toroidalWinding )
    {     
      // See if the test point is between the first secton.
      Vector v0 = (Vector) points[0] - (Vector) points[k];
      Vector v1 = (Vector) points[1] - (Vector) points[k];
        
      if( Dot( v0, v1 ) < 0.0 )
      {
        npts = k / toroidalWinding;
        complete = true;
        break;
      }

      // See if the first point is between the test section.
      v0 = (Vector) points[0] - (Vector) points[j];
      v1 = (Vector) points[0] - (Vector) points[k];
      
      if( Dot( v0, v1 ) < 0.0)
      {
        npts = k / toroidalWinding;
        complete = true;
        break;
      }
    }
  }
  else
  {
    npts = toroidalWinding;
  }

  vector< Point > tmp_points;
  int direction;

  tmp_points.resize( npts );

  for( unsigned int j=toroidalWinding; j<npts; ++j )
    tmp_points[j] = points[j];

  bool convex = hullCheck( tmp_points, direction );

  // A convex hull for a toroidalWinding of 1 is defined as not
  // being an island.
  if( toroidalWinding == 1 && convex )
  {
//  cerr << "toroidalWinding of 1 and convex hull" << endl;
    return false;
  }

//  cerr << "Convex  " << convex << endl;

  // Check for islands. Islands will exists if there is a change in
  // direction of the connected points relative to a base point. If
  // the hull is convex the base point may the centroid of all of the
  // points or based upon a point that is perpendicular to the
  // principal axis of the group of points.
  vector< unsigned int > nodes(toroidalWinding);

  for( unsigned int i=0; i<toroidalWinding; i++ )
  {
    Vector baseCentroid;

    // If the hull is convex the global centroid can be used because
    // all of the islands will surround it in a radial manner.
    if( convex )
    {
      baseCentroid = globalCentroid;
    }
    // Otherwise use an offset from the local centroid of each group.
    else // !concave
    {
      // Get the local centroid for the group.
      Vector localCentroid(0,0,0);

      unsigned int npts = 0;

      for( unsigned int j=i; j<points.size(); j+=toroidalWinding, npts++ )
        localCentroid += (Vector) points[j];

      localCentroid /= (float) npts;

//      cerr << "localCentroid  " << localCentroid << endl;

      // Get the principal axes of the points.
      float Ixx = 0.0;
      float Ixz = 0.0;
      float Izz = 0.0;

      float maxDist = 0;

      // Get the moments of intertial for each point. It assumed that
      // everything is in the Y plane as such the moments of intertial
      // along the Y axis are zero.
      for( unsigned int j=i; j<points.size(); j+=toroidalWinding )
      {

        Vector vec = (Vector) points[j] - localCentroid;

        if( maxDist < vec.length() )
          maxDist = vec.length();

        Ixx += vec.z*vec.z;
        Ixz -= vec.x*vec.z;
        Izz += vec.x*vec.x;
      }

      // Short cut to the principal axes because the Y moments of
      // intertial are zero.
      float alpha = atan( 2.0 * Ixz / (Ixx - Izz) ) / 2.0;

      float Pxx = Ixx + Ixz * sin(alpha       )/cos(alpha       );
      float Pzz = Izz + Ixz * cos(alpha+M_PI/2)/sin(alpha+M_PI/2);

//     cerr << "Principal axes of intertial "
//       << Pxx << "  " << Pzz << "  " << Pxx/Pzz << "  " << Pzz/Pxx << endl;

      // For a toroidalWinding of 1 if the moment of interia of one axis is
      // similar to the other axis then use the centroid directly.
      if( toroidalWinding == 1 && Pxx / Pzz < 5.0 && Pzz / Pxx < 5.0 )
      {
//      cerr << "Using local centroid " << endl;
        baseCentroid = localCentroid;
      }
      else
      {
        // Find the approximate center if points were projected onto a
        // circle.
        unsigned int npts = 0;
        Vector center(0,0,0);
 
        for( unsigned int j=i+toroidalWinding;
             j<points.size()-toroidalWinding;
             j+=toroidalWinding ) {
          
          unsigned int j_1 = j - toroidalWinding;
          unsigned int j1  = j + toroidalWinding;


          center += (Vector) circle( points[j_1],
                                     points[j  ],
                                     points[j1 ] );

          ++npts;
        }

        center /= (float) npts;

//      cerr << "center  " << center << endl;

        // Use the principal axes to get an offset from the local
        // centroid which gives a point outside the island.

        // The direction along the axis is determined by where the
        // center is located.

//      cerr << "localCentroid  " << localCentroid << endl;

        if( Pxx > Pzz )
        {
          if( Dot( center - localCentroid, 
                   Vector( cos(alpha), 0, sin(alpha) ) ) )
            baseCentroid = localCentroid +
              Vector(  cos(alpha), 0, sin(alpha) ) * maxDist;
          else
            baseCentroid = localCentroid -
              Vector(  cos(alpha), 0, sin(alpha) ) * maxDist;
        }
        else
        {
          if( Dot( center - localCentroid, 
                   Vector( -sin(alpha), 0, cos(alpha) ) ) )
            baseCentroid = localCentroid +
              Vector( -sin(alpha), 0, cos(alpha) ) * maxDist;
          else
            baseCentroid = localCentroid -
              Vector( -sin(alpha), 0, cos(alpha) ) * maxDist;
        }
      }
    }

//    cerr << "baseCentroid  " << baseCentroid << endl;

    unsigned int turns = 0;
    unsigned int firstTurn = 0;
    unsigned int   midTurn = 0;
    unsigned int  lastTurn = 0;

    // Get the direction based on the first two points.
    Vector v0 = (Vector) points[i                ] - baseCentroid;
    Vector v1 = (Vector) points[i+toroidalWinding] - baseCentroid;

    bool localCCW, lastCCW = (ccw( v0, v1 ) == 1);

    // Get the direction based on the remaining points.
    for( unsigned int j=i+2*toroidalWinding;
         j<points.size();
         j+=toroidalWinding )
    {
      v0 = v1;
      v1 = (Vector) points[j] - baseCentroid;

      // Points are parallel - this result should not occur as the
      // base centroid point should be on the concave side of the
      // island.
      if (ccw( v0, v1 ) == 0 )
        localCCW = lastCCW;
      else
        localCCW = (ccw( v0, v1 ) == 1);

      // A change in direction indicates that an island is present.
      if( localCCW != lastCCW )
      {
        lastCCW = localCCW;

        // Record the number of turns. Ideally three turns will be
        // found which indicates that the island is complete. Although
        // the island could be complete with only two turns.
        ++turns;

        if( turns == 1 )
          ++islands;

        if( turns == 1 )     firstTurn = j - toroidalWinding;
        else if( turns == 2 )  midTurn = j - toroidalWinding;
        else if( turns == 3 ) lastTurn = j - toroidalWinding;

        if( turns == 3 )
          break;
      }
    }

    unsigned int overlap = 0;

    // Approximate number of points in the islands. 
    nodes[i] = points.size() / toroidalWinding;

    // Check to see if the island overlaps itself. Only use this if
    // there are two or more turns. Otherwise the it can appear that
    // it overlaps itself earlier.
    if( turns >= 2 )
    {
      // Okay initial approximatation. It really depending on where
      // the turns are relative to each other/
      if( turns == 3 )
        nodes[i] = (lastTurn - firstTurn) / toroidalWinding + 1;

      // Look for the overlap between the ends of the island which are
      // flatter.
      unsigned int base, start;      
      unsigned int quarter_dist =
        ((midTurn - firstTurn) / 4 / toroidalWinding) * toroidalWinding;

      // If the start is in the middle section use it.
      if( quarter_dist <= firstTurn && firstTurn <= quarter_dist * 3 )
      {
        base = i;
        start = midTurn;
      }

      // If the start is before the middle section use the first
      // middle section.
      else if( firstTurn >= quarter_dist * 2 )
      {
        base = firstTurn - quarter_dist * 2;
        start = midTurn;
      }

      // If the start is after the middle section use the next middle
      // secction.
      else
      {
        base = firstTurn + quarter_dist;

        if( lastTurn )
          start = lastTurn;
        else
          start = midTurn;
      }

      if( turns && verboseFlag )
        cerr << "Island " << i << " - "
             << " nodes " << nodes[i]
             << " turns " << turns
             << " first " << firstTurn
             << " mid " << midTurn
             << " last " << lastTurn
             << " base " << base
             << " start " << start
             << endl;

      for( unsigned int j=start, k=j+toroidalWinding;
           k<points.size();
           j+=toroidalWinding, k+=toroidalWinding ) {

        // See if the test point is between the first secton.
        v0 = ((Vector) points[base                ] -
              (Vector) points[k                   ]);
        v1 = ((Vector) points[base+toroidalWinding] -
              (Vector) points[k                   ]);
        
        if( Dot( v0, v1 ) < 0.0 ) {
          overlap = k;
          nodes[i] = (overlap-base) / toroidalWinding;
          turns = 3;
          complete = true;
          break;
        }

        // See if the first point is between the test section.
        v0 = (Vector) points[base] - (Vector) points[j];
        v1 = (Vector) points[base] - (Vector) points[k];
        
        if( Dot( v0, v1 ) < 0.0 ) {
          overlap = k;
          nodes[i] = (overlap-base) / toroidalWinding;
          turns = 3;
          complete = true;
          break;
        }
      }

      // Do a distance search based on the overlaping sections having
      // two points in close proximity while at the same time the
      // overlaping sections have approximately the same length.
      if( 1 || !complete )
      {
        unsigned int stop;

        // Because the curve may overlap the start multiple times
        if( lastTurn )
        {
          stop = lastTurn +
            ((midTurn - firstTurn) / 4 / toroidalWinding) * toroidalWinding;
        }
        else
        {
          stop = points.size()-toroidalWinding;
        }

        // Get the maximum distance from the first point so that the
        // search is not conducted near neighbor points.
        double max_gap = 0;
        
        for( unsigned int j=i+toroidalWinding;
             j<points.size()-toroidalWinding;
             j+=toroidalWinding )
        {
          double tmp_gap = (points[i] - points[j]).length();

          if( max_gap < tmp_gap )
            max_gap = tmp_gap;
        }

        max_gap *= 0.5;

        double min_gap = 1.0e9;  // Distance between points.
        unsigned int min_gap_index = 0;

        double min_len = 1.0e9;  // Difference in segments.
        unsigned int min_len_index = 0;

        double min_dist = 1.0e9; // Minimum distance.
        unsigned int min_dist_index = 0;

        double min_dist2 = 1.0e9;// Second minimum distance.
        unsigned int min_dist2_index = 0;

        bool past_max_gap = false;

        // Base length of the first segment.
        double len = (points[i] - points[i+toroidalWinding]).length();

        for( unsigned int j=i+toroidalWinding;
             j<points.size()-toroidalWinding && j<stop;
             j+=toroidalWinding )
        {
          double tmp_gap = (points[i] - points[j]).length();

          if( tmp_gap > max_gap )
            past_max_gap = true;

          double tmp_len = (points[j] - points[j+toroidalWinding]).length();
          double dist = sqrt(tmp_gap*tmp_gap + (tmp_len-len)*(tmp_len-len));

          if( past_max_gap )
          {
            if( min_gap > tmp_gap )
            {
              min_gap = tmp_gap;
              min_gap_index = j;
            }

            if( min_len > tmp_len )
            {
             min_len = tmp_len;
             min_len_index = j;
            }
          
            if( min_dist > dist )
            {
//          cerr << j << "  "
//               << tmp_gap << "  "
//               << fabs(tmp_len - len) << "  " 
//               << dist << "  "
//               << endl;

              min_dist2 = min_dist;
              min_dist2_index = min_dist_index;

              min_dist = dist;
              min_dist_index = j;
            }

            else if( min_dist2 > dist )
            {
//          cerr << j << "  "
//               << tmp_gap << "  "
//               << fabs(tmp_len - len) << "  " 
//               << dist << "  "
//               << endl;

              min_dist2 = dist;
              min_dist2_index = j;
            }
          }
        }

        if( !complete )
        {
          overlap = min_dist_index;
          nodes[i] = (min_dist_index-i)/toroidalWinding;
          turns = 3;
          complete = true;
        }

        cerr << "gap " << min_gap_index << "  "
             << "len " << min_len_index << "  "
             << "dist " << min_dist_index << "  "
             << "dist2 " << min_dist2_index << "  "
             << "nodes " << (min_dist_index-i)/toroidalWinding << endl;
      }
    }
    
    if( turns && verboseFlag )
      cerr << "Island " << i << " - "
           << " nodes " << nodes[i]
           << " turns " << turns 
           << " first " << firstTurn
           << " mid " << midTurn
           << " last " << lastTurn
           << " overlap " << overlap
           << " complete " << complete
           << endl;

    nnodes += nodes[i];

    // Started on a surface but jumpped to an island - i.e. a
    // separtrice
    if( !complete && lastTurn &&
        ((int) (firstTurn / toroidalWinding) - (int) (nodes[i]/2)) > 2 )
    {
      --islands;

      if( turns && verboseFlag )
        cerr << "  !!!!!!!!!!!!!!! Separatrice !!!!!!!!!!!!!!!" << endl;
    }
  }

  // Get the average number of nodes.
  nnodes /= (float) toroidalWinding;

  if( islands ) {

    unsigned int cc = 0;
    unsigned int index = 0;
    unsigned int new_nnodes = 0;

    for( unsigned int i=0; i<toroidalWinding; i++ )
    {
      if( nodes[i] < nnodes - 1 || nnodes + 1 < nodes[i] )
      {
        cc++;
        index = i;
      }
      else
        new_nnodes += nodes[i];
    }

    if( cc == 1 )
    {
      nnodes = new_nnodes / (float) (toroidalWinding-cc);
    }

    for( unsigned int i=0; i<toroidalWinding; i++ )
    {
      if( nodes[i] < nnodes - 1 || nnodes + 1 < nodes[i] )
      {
        if( verboseFlag )
          cerr << "Appears to be islands but not self consistent, "
               << "average number of nodes  " << nnodes << "   (  ";
          
        for( unsigned int i=0; i<toroidalWinding; i++ )
            if( verboseFlag )
              cerr << nodes[i] << "  ";

        cerr << ")" << endl;

        break;
      }
    }  
  }

  return (bool) islands;
}


void
FieldlineLib::getPunctures( vector< Point > &ptList,
                            Vector planeN,
                            vector< Point > &puncturePts )
{
  unsigned int startIndex = 0;

  // Set up the plane equation.
  Vector planePt(0,0,0);
  double plane[4];
            
  plane[0] = planeN.x;
  plane[1] = planeN.y;
  plane[2] = planeN.z;
  plane[3] = planePt.dot(planeN);
  
  int bin = 0;
  
  // So to get the winding groups consistant start examining
  // the streamline in the same place for each plane.
  Point lastPt, currPt = ptList[startIndex];
  double lastDist, currDist = planeN.dot( currPt ) - plane[3];
            
  for( unsigned int i=startIndex+1; i<ptList.size(); ++i )
  {
    lastPt = currPt;
    currPt = Vector(ptList[i]);
                
    lastDist = currDist;
    currDist = Dot( planeN, currPt ) - plane[3];
    
    // First look at only points that intersect the plane.
    if( SIGN(lastDist) != SIGN(currDist) ) 
    {
      Vector dir(currPt-lastPt);
                    
      double dot = Dot(planeN, dir);
                    
      // If the segment is in the same direction as the plane then
      // find where it intersects the plane.
      if( dot > 0.0 )
      {
        Vector w = lastPt - planePt;
        
        double t = -Dot(planeN, w ) / dot;
        
        Point point = Point(lastPt + dir * t);
        
        puncturePts.push_back( point );
      }
    }
  }
} 

void
FieldlineLib::getFieldlineBaseValues( vector< Point > &ptList,
                                      vector< Point > &poloidal_puncture_pts,
                                      vector< Point > &ridgeline_points,
                                      vector< double > &rotationalSums,
                                      vector< unsigned int > &poloidalWindingCounts,
                                      float &delta )
{
  poloidal_puncture_pts.clear();
  ridgeline_points.clear();

  rotationalSums.clear();
  poloidalWindingCounts.clear();

  delta = 0.0;

  Point currPt = ptList[0], nextPt = ptList[1];
  Vector planePt( 0, 0, 0 );

  if( verboseFlag )
  {
    cerr << "-----------------------------------------------------------------"
         << endl
         << "Analyzing  " << ptList[0] << "  "
         << "with  " << ptList.size() << " fieldline points"
         << endl;
  }

  // Set up the Y plane equation as the base poloidal puncture
  // analysis takes place in the X-Z plane.
  Vector planeNY( 0, 1, 0 );
  double planeY[4];

  planeY[0] = planeNY.x;
  planeY[1] = planeNY.y;
  planeY[2] = planeNY.z;
  planeY[3] = Dot( planePt, planeNY );

  double currDistY, nextDistY = Dot(planeNY, nextPt) - planeY[3];

  // Rotational sum
  double currR = sqrt(currPt.x*currPt.x+currPt.y*currPt.y);
  double currZ = currPt[2];

  double nextR = sqrt(nextPt.x*nextPt.x+nextPt.y*nextPt.y);
  double nextZ = nextPt[2];

  double dAngle = 0;
  double rotationalSum = 0;
  double ridgelineRotationalSum = 0;

  double deltaR = nextR - currR;
  double deltaZ = nextZ - currZ;

  double prevAngle=0, currAngle = atan2(deltaZ, deltaR);
  if (currAngle < 0) currAngle += (2*M_PI);
        
  bool zeroAngle = false;
  if( deltaR == 0 && deltaZ == 0 )
    zeroAngle = true;
  
  double maxZ = currPt.z;
  double maxZrotationalSum = 0;
  bool   maxZset = false;

  // Now collect the points.
  unsigned int npts = 0;

  for( unsigned int i=2; i<ptList.size(); ++i)
  {    
    currPt = nextPt;
    nextPt = ptList[i];

    if( maxZ < currPt.z )
    {
      maxZ = currPt.z;
      maxZrotationalSum = rotationalSum;
    }

    // Save the distance between points to use for finding periodic
    // fieldlines (i.e. rational surfaces and re-connection).
    Vector s = (Vector) nextPt - (Vector) currPt;

    double ds = s.length();

    delta += ds;
    ++npts;

    // Poloidal plane distances.
    currDistY = nextDistY;
    nextDistY = Dot( planeNY, nextPt ) - planeY[3];

    // First look at only points that intersect the poloidal plane.
    if( SIGN(currDistY) != SIGN(nextDistY) ) 
    {
      Vector dir(nextPt-currPt);
      
      double dot = Dot(planeNY, dir);
      
      // If the segment is in the same direction as the poloidal plane
      // then find where it intersects the plane.
      if( dot > 0.0 )
      {
        Vector w = (Vector) currPt - planePt;
        
        double t = -Dot(planeNY, w ) / dot;
        
        Point point = Point(currPt + dir * t);
        
        poloidal_puncture_pts.push_back( point );

        poloidalWindingCounts.push_back( fabs(rotationalSum) / (2.0*M_PI) );

//         cerr << poloidalWindingCounts.size() << "  " 
//              << fabs(rotationalSum) / (2.0*M_PI) << endl;

        rotationalSums.push_back( fabs(rotationalSum) );
      }
    }

    // Values for the rotational transform summation.
    currR = nextR;
    currZ = nextZ;
      
    nextR = sqrt(nextPt.x*nextPt.x+nextPt.y*nextPt.y);
    nextZ = nextPt.z;

    // Calculate the rotational transform summation.
    deltaR = nextR - currR;
    deltaZ = nextZ - currZ;

    // Check for a zero angle coming in on the right.
    if( !zeroAngle )
    {
      prevAngle = currAngle;

      if( deltaR == 0 && deltaZ == 0 )
        zeroAngle = true;
    }

    // In zero mode so look for a good angle coming in on the right.
    else if( zeroAngle && (deltaR != 0 || deltaZ != 0) )
    {
      zeroAngle = false;
    }

    if( !zeroAngle )
    {
      currAngle = atan2(deltaZ, deltaR);
      if (currAngle < 0) currAngle += (2*M_PI);
        
      dAngle = currAngle - prevAngle;

      if (dAngle > M_PI)
        dAngle -= (2*M_PI);
        
      else if (dAngle < -M_PI)
        dAngle += (2*M_PI);
    }
    else
      dAngle = 0;

    rotationalSum += dAngle;
    ridgelineRotationalSum += dAngle;

    if( fabs(ridgelineRotationalSum) >= 2.0 * M_PI )
    {
      ridgelineRotationalSum -= 2.0 * M_PI * SIGN(ridgelineRotationalSum);

      ridgeline_points.push_back( Point( (float) ridgeline_points.size(),
                                         0,
                                         maxZ) );
      maxZ = 0;
    }
  }

  // At this point all of the poloidal and toroidal puncture points
  // have been found.
  if( verboseFlag )
    cerr << poloidal_puncture_pts.size() << " poloidal puncture pts, "
         << ridgeline_points.size() << " ridgeline pts " << endl;

  // Get the average distance between puncture points for finding
  // periodic fieldlines (i.e. rational surfaces and re-connection).
  delta  /= (float) npts;
}

void
FieldlineLib::GetBaseWindingPairs( vector< unsigned int > &poloidalWindingCounts,
                                   vector< Point > &poloidal_puncture_pts,
                                   vector< WindingPair > &baseWindingPairs,
                                   double &windingPairConfidence,
                                   unsigned int &toroidalWindingMax,
                                   unsigned int &poloidalWindingMax,
                                   unsigned int &windingNumberMatchIndex )
{
  // Check the consistency of the poloidal winding counts. 
  poloidalWindingCheck( poloidalWindingCounts, baseWindingPairs );

  // Report the winding number pairs.
  vector< WindingPair >::iterator iter = baseWindingPairs.begin();
  
  // Get the first set that passes the intersection test and passes
  // the user setable match limit. Default is 0.90 (90%)
  for( unsigned int i=0; i<baseWindingPairs.size(); ++i, ++iter )
  {
    if( baseWindingPairs[i].stat < windingPairConfidence )
      break;

    double local_safetyFactor =
      (double) baseWindingPairs[i].toroidal /
      (double) baseWindingPairs[i].poloidal;

    if( toroidalWindingMax < baseWindingPairs[i].toroidal )
      toroidalWindingMax = baseWindingPairs[i].toroidal;

    if( poloidalWindingMax < baseWindingPairs[i].poloidal )
      poloidalWindingMax = baseWindingPairs[i].poloidal;

    unsigned int windingGroupOffset =
      Blankinship( baseWindingPairs[i].toroidal,
                   baseWindingPairs[i].poloidal );

    if( IntersectCheck( poloidal_puncture_pts,
                        baseWindingPairs[i].toroidal,
                        windingGroupOffset ) )
    {
      if( windingNumberMatchIndex == -1 )
        windingNumberMatchIndex = i;
        
      if( verboseFlag )
        cerr << "**Drawable winding pair "
             << baseWindingPairs[i].toroidal << ","
             << baseWindingPairs[i].poloidal << "  ("
             << local_safetyFactor << " - "
//             << fabs(safetyFactor - local_safetyFactor) << ")  "
             << "consistency "
             << 100.0 * baseWindingPairs[i].stat
             << "%" << endl;

      unsigned int cgd = GCD( baseWindingPairs[i].toroidal,
                              baseWindingPairs[i].poloidal );
    }
    else
    {
      // Debug info
      if( verboseFlag )
        cerr << "Undrawable winding pair "
             << baseWindingPairs[i].toroidal << ","
             << baseWindingPairs[i].poloidal << "  ("
             << local_safetyFactor << " - "
//             << fabs(safetyFactor - local_safetyFactor) << ")  "
             << "consistency " << 100.0 * baseWindingPairs[i].stat
             << "%"
             << endl;
    }
  }

  // Remove the winding number sets that are below the limit.
  if( iter != baseWindingPairs.end() )
    baseWindingPairs.erase( iter, baseWindingPairs.end() );
}


void FieldlineLib::
GetPeriodWindingPairs( vector< WindingPair > &baseWindingPairs,
                       vector< WindingPair > &periodWindingPairs,
                       vector< pair< unsigned int, double > > &toroidalStats,
                       vector< pair< unsigned int, double > > &poloidalStats )
{
  bool pairFound;

  for( unsigned int i=0; i<baseWindingPairs.size(); ++i )
  {
    pairFound = false;

    // Look for a toroidal winding
    for( unsigned int j=0; j<toroidalStats.size(); ++j )
    {
      if( toroidalStats[j].first == baseWindingPairs[i].toroidal )
      {
        // Look for a poloidal winding
        for( unsigned int k=0; k<poloidalStats.size(); ++k )
        {
          if( poloidalStats[k].first == baseWindingPairs[i].poloidal &&
           
              // Make sure the ratio of both periods is the same. This
              // ration is important for island chains.
              toroidalStats[j].first / baseWindingPairs[i].toroidal ==
              poloidalStats[k].first / baseWindingPairs[i].poloidal )
          {
            pairFound = true;

            WindingPair windingPair = baseWindingPairs[i];
            
            windingPair.stat = sqrt((double) (j*j+k*k));
            windingPair.ranking = 0;

            periodWindingPairs.push_back( windingPair );
            break;
          }     
        }
      }

      if( pairFound )
        break;
    }
  }

  // Now sort the results based on the index Euclidian distance.
  SortWindingPairs( periodWindingPairs, true );
  RankWindingPairs( periodWindingPairs, false );

  for( unsigned int i=0; i<periodWindingPairs.size(); ++i )
  {
    if( verboseFlag && i<10 )
      cerr << "Period based winding pair:  " 
           << periodWindingPairs[i].toroidal << ","
           << periodWindingPairs[i].poloidal << "  "
           << "Distance  " << periodWindingPairs[i].stat << "  "
           << "Rank  " << periodWindingPairs[i].ranking << "  "
           << endl;
  }
}


void
FieldlineLib::fieldlineProperties( vector< Point > &ptList,
                                   FieldlineProperties &fi,
                                   unsigned int overrideToroidalWinding,
                                   unsigned int overridePoloidalWinding,
                                   unsigned int maxToroidalWinding,
                                   double windingPairConfidence,
                                   bool detectIslandCenters )
{
  vector< Point > poloidal_puncture_pts;
  vector< Point > ridgeline_points;

  vector< double > rotationalSums;
  vector< unsigned int > poloidalWindingCounts;

  float delta = 0.0;

  getFieldlineBaseValues( ptList,
                          poloidal_puncture_pts,
                          ridgeline_points,
                          rotationalSums,
                          poloidalWindingCounts,
                          delta );

  if( ptList.empty() ||
      poloidal_puncture_pts.empty() ||
      ridgeline_points.empty() )
  {
    fi.type = FieldlineProperties::UNKNOWN_TYPE;
    fi.analysisState = FieldlineProperties::UNKNOWN_STATE;

    fi.safetyFactor = 0;
    fi.toroidalWinding = 0;
    fi.poloidalWinding = 0;
    fi.poloidalWindingP = 0;
    fi.toroidalResonance = 0;
    fi.poloidalResonance = 0;
    fi.windingPairs.clear();
    fi.windingGroupOffset = 0;
    fi.islands = 0;
    fi.islandGroups = 0;
    fi.nnodes  = 0;

    fi.nPuncturesNeeded  = 0;

    fi.OPoints.clear();

    return;
  }


  // Get the safety factor.
//   for( unsigned int i=0; i<poloidal_puncture_pts.size(); ++i )
//     cerr << i << "  " << (2.0 * M_PI * (i+1)) / fabs(rotationalSums[i]) << endl;

  // Last pair estimation for the safety factor.
  double LRS_SafetyFactor = (2.0 * M_PI * poloidal_puncture_pts.size()) /
    fabs(rotationalSums[poloidal_puncture_pts.size()-1]);

  // Average estimation for the safety factor.
//   double averageRotationalSum, stdDev;

//   safetyFactorStats( rotationalSums, averageRotationalSum, stdDev );
//   averageRotationalSum *= 2.0 * M_PI;

  // Least squares estimation for the safety factor.
  double LSRS_SafetyFactor;

  least_square_fit( rotationalSums, LSRS_SafetyFactor );
  LSRS_SafetyFactor *= 2.0 * M_PI;

  double safetyFactor = LSRS_SafetyFactor;

  if( verboseFlag )
  {
    cerr << "Limit Rotational Sum Safety Factor    "
         << LRS_SafetyFactor << endl
//          << "Average Rotational Sum Safety Factor         "
//          << averageRotationalSum << endl
         << "Least Square Rotational Sum Safety Factor    "
         << LSRS_SafetyFactor << endl

         << "Using safety factor " << safetyFactor << endl;
  }

  // Start the analysis.
  FieldlineProperties::FieldlineType type = FieldlineProperties::UNKNOWN_TYPE;
  FieldlineProperties::AnalysisState analysisState = FieldlineProperties::UNKNOWN_STATE;

  unsigned int toroidalWinding = 0, poloidalWinding = 0, poloidalWindingP = 0;
  unsigned int toroidalResonance = 1, poloidalResonance = 1;

  unsigned int windingGroupOffset = 0;
  unsigned int nnodes = 0;
  unsigned int islands = 0, islandGroups = 0;
  unsigned int nPuncturesNeeded = 0;

  unsigned int windingNumberMatchIndex = -1;
  unsigned int toroidalMatchIndex = -1, poloidalMatchIndex = -1;

  unsigned int toroidalWindingMax = 0, poloidalWindingMax = 0;

  vector< WindingPair > baseWindingPairs, //approximateWindingPairs,
    periodWindingPairs, mergedWindingPairs;

  vector< Point > islandCenters;

  GetBaseWindingPairs( poloidalWindingCounts, poloidal_puncture_pts,
                       baseWindingPairs,
                       windingPairConfidence,
                       toroidalWindingMax, poloidalWindingMax,
                       windingNumberMatchIndex );


  // Match consistency is less than the user set value. Run more
  // expensive tests to identify the fieldline.
  if( windingNumberMatchIndex == -1 )
  {
    if( verboseFlag )
      cerr << "Poor consistency - probably chaotic" << endl;

    if( overrideToroidalWinding && overridePoloidalWinding )
    {
      windingGroupOffset = Blankinship( overrideToroidalWinding,
                                        overridePoloidalWinding );

      fi.analysisState = FieldlineProperties::TERMINATED;
      fi.type = FieldlineProperties::CHAOTIC;
    }
    else
    {
      windingGroupOffset = 0;

      fi.analysisState = FieldlineProperties::TERMINATED;
      fi.type = FieldlineProperties::CHAOTIC;
    }
    
    fi.safetyFactor = safetyFactor;
    fi.toroidalWinding = overrideToroidalWinding;
    fi.poloidalWinding = overridePoloidalWinding;
    fi.poloidalWindingP = 0;
    fi.toroidalResonance = 1;
    fi.poloidalResonance = 1;
    fi.windingPairs.clear();
    fi.windingGroupOffset = windingGroupOffset;
    fi.islands = 0;
    fi.islandGroups = 0;
    fi.nnodes  = 0;

    fi.nPuncturesNeeded  = 0;

    fi.OPoints.clear();

    return;
  }

  vector< pair< unsigned int, double > > toroidalStats;
  vector< pair< unsigned int, double > > poloidalStats;

  // Find the best toroidal periodicity. For a flux surface the period
  // will be the toroidal winding number. For an island chain the
  // period will be the toroidal winding number times the number of
  // nodes.
  if( verboseFlag )
    cerr << "Toroidal Winding via "
         << poloidal_puncture_pts.size() << "  "
         << "poloidal punctures, "
         << "max period " << toroidalWindingMax
         << endl;

  periodicityStats( poloidal_puncture_pts, toroidalStats,
                    toroidalWindingMax, 2 );

  // Find the best poloidal periodicity. For a flux surface the period
  // will be the poloidal winding number. For an island chain the
  // period will be the poloidal winding number times the number of
  // nodes.
  if( verboseFlag )
    cerr << "Poloidal Winding via "
         << ridgeline_points.size() << "  "
         << "ridgeline points, "
         << "max period " << poloidalWindingMax
         << endl;
  
  periodicityStats( ridgeline_points, poloidalStats,
                    poloidalWindingMax, 1 );
  

  // Form a second winding number list that is ranked based on the
  // euclidian distance of each of the period lists.
  GetPeriodWindingPairs( baseWindingPairs, periodWindingPairs,
                         toroidalStats, poloidalStats );

  // Merge the base and period winding pairs together based on the
  // Euclidian distance via the index.
  for( unsigned int i=0; i<baseWindingPairs.size(); ++i )
  {
    int ii = baseWindingPairs[i].ranking;

    // Search for the same sibling pair in the best rational
    // approximation winding pair list.
    int jj = -1;

    for( unsigned int j=0; j<periodWindingPairs.size(); ++j )
    {
      if( baseWindingPairs[i].toroidal == periodWindingPairs[j].toroidal &&
          baseWindingPairs[i].poloidal == periodWindingPairs[j].poloidal )
      {
        jj = periodWindingPairs[j].ranking;
        break;
      }
    }

    // Found a matching pair so compute the index Euclidian distance.
    if( jj != -1 )
    {
      WindingPair windingPair = baseWindingPairs[i];

      windingPair.stat = sqrt((double)(ii*ii+jj*jj));
      windingPair.ranking = 0;
            
      mergedWindingPairs.push_back( windingPair );
    }
  }        

  // Now sort the results based on the Euclidian distance via the index.
  SortWindingPairs( mergedWindingPairs, true );
  RankWindingPairs( mergedWindingPairs, false );

  int drawableRank  = -1;
  int drawableIndex = -1;
  vector< unsigned int > drawableIndexs;

  vector< pair < unsigned int, unsigned int > > windingPairs;


  for( unsigned int i=0; i<mergedWindingPairs.size(); ++i )
  {
    windingPairs.push_back( pair< unsigned int,
                                  unsigned int > ( mergedWindingPairs[i].toroidal,
                                                   mergedWindingPairs[i].poloidal ) );

    windingGroupOffset = Blankinship( mergedWindingPairs[i].toroidal,
                                      mergedWindingPairs[i].poloidal );

    bool drawable = IntersectCheck( poloidal_puncture_pts,
                                    mergedWindingPairs[i].toroidal,
                                    windingGroupOffset );


    if( verboseFlag && (drawable || i < 10) )
      cerr << "Final "
           << (drawable ? "Drawable " : "Rejected ") 
           << "winding pair:  " 
           << mergedWindingPairs[i].toroidal << ","
           << mergedWindingPairs[i].poloidal << "  "
           << "Distance  " << mergedWindingPairs[i].stat << "  "
           << "Rank  " << mergedWindingPairs[i].ranking << "  ";

    if( (drawableRank == -1 ||
         drawableRank == mergedWindingPairs[i].ranking) &&

        // Ignore the user requested
        (maxToroidalWinding == 0 ||
         mergedWindingPairs[i].toroidal <= maxToroidalWinding) &&

        // Keep only those that are drawable.
        drawable )
    {
      drawableIndexs.push_back( i );

      // If a tie take the higher order winding.
      if( toroidalWinding < mergedWindingPairs[i].toroidal )
      {
        toroidalWinding = mergedWindingPairs[i].toroidal;

        drawableRank = mergedWindingPairs[i].ranking;
        drawableIndex = i;
      }
    }

    if( verboseFlag && (drawable || i<10) )
      cerr << endl;
  }


  if( drawableIndex == -1 )
  {
    if( overrideToroidalWinding && overridePoloidalWinding )
    {
      toroidalWinding = overrideToroidalWinding;
      poloidalWinding = overridePoloidalWinding;
    }

    else if( poloidal_puncture_pts.size() >= fi.maxPunctures )
    {
      toroidalWinding = baseWindingPairs[ windingNumberMatchIndex ].toroidal;
      poloidalWinding = baseWindingPairs[ windingNumberMatchIndex ].poloidal;

      analysisState = FieldlineProperties::TERMINATED;
    }

    else
    {
      toroidalWinding = 0;
      poloidalWinding = 0;
    }

    if( toroidalWinding && poloidalWinding )
    {
      windingGroupOffset = Blankinship( toroidalWinding, poloidalWinding );

      nnodes = poloidal_puncture_pts.size() / 2;

      // Check to see if the fieldline is periodic. I.e. on a rational
      // surface.  If within "delta" of the distance the fieldline is
      // probably on a rational surface.

      // NOTE: Define the rational based on the lowest order of teh
      // surface. Thus divide by the GCD.
      if( rationalCheck( poloidal_puncture_pts,
                         toroidalWinding / GCD(toroidalWinding, poloidalWinding ),
                         nnodes, delta*0.1 ) ) 
      {
        type = FieldlineProperties::RATIONAL;
        islands = 0;
        
        analysisState = FieldlineProperties::COMPLETED;
        
        if( verboseFlag )
          cerr << "Appears to be a rational surface " << delta*0.1 << endl;
      }
      else
      {
        fi.analysisState = FieldlineProperties::UNKNOWN_STATE;
        fi.type = FieldlineProperties::CHAOTIC;
      }

      fi.safetyFactor = safetyFactor;
      fi.toroidalWinding = toroidalWinding;
      fi.poloidalWinding = poloidalWinding;
      fi.poloidalWindingP = poloidalWindingP;
      fi.toroidalResonance = 1;
      fi.poloidalResonance = 1;
      fi.windingPairs = windingPairs;
      fi.windingGroupOffset = windingGroupOffset;
      fi.islands = 0;
      fi.islandGroups = 0;
      fi.nnodes = poloidal_puncture_pts.size() / 2;

      fi.nPuncturesNeeded  = 0;

      fi.OPoints.clear();

      return;
    }
  
    if( verboseFlag )
      cerr << "Garbage matches adding more points" << endl;

    fi.nPuncturesNeeded = poloidal_puncture_pts.size() * 1.25;

    return;
  }

  toroidalWinding  = mergedWindingPairs[drawableIndex].toroidal;
  poloidalWinding  = mergedWindingPairs[drawableIndex].poloidal;
  poloidalWindingP = mergedWindingPairs[drawableIndex].poloidal;

  float local_safetyFactor = (float) toroidalWinding / (float) poloidalWinding;

  // The windingGCD only has meaning if when islands are
  // present. Which in that case it is number of points per island.
  unsigned int windingGCD = GCD( mergedWindingPairs[drawableIndex].toroidal,
                                 mergedWindingPairs[drawableIndex].poloidal );

  if( mergedWindingPairs.size() == 1 )
  {
    toroidalResonance = 1;
    poloidalResonance = 1;
  }
  else
  {
    // The base values are the first order resonances if present.
    toroidalResonance = ResonanceCheck( toroidalStats, 1, 3 );
    poloidalResonance = ResonanceCheck( poloidalStats, 1, 3 );

    if( local_safetyFactor !=
        (float) toroidalResonance / (float) poloidalResonance )
    {
      cerr << toroidalResonance << "  " << poloidalResonance << endl;

      if ( toroidalWinding % toroidalResonance == 0 &&
           poloidalWinding != poloidalResonance )
      {
        // Get GCD from winding pairs ...
        unsigned int freq;
        vector< unsigned int > values;

        values.resize( mergedWindingPairs.size() );

        // Check the toroidal windings ...
        for( unsigned int i=0; i<mergedWindingPairs.size(); ++i )
          values[i] = mergedWindingPairs[i].toroidal;

        toroidalResonance = GCD( values, freq );

        // Check the poloidal windings ...
        for( unsigned int i=0; i<mergedWindingPairs.size(); ++i )
          values[i] = mergedWindingPairs[i].poloidal;

        poloidalResonance = GCD( values, freq );

        cerr << "winding pair resonance "
             << toroidalResonance << "  " << poloidalResonance << endl;

        // Still no match so give up.
        if( local_safetyFactor !=
            (float) toroidalResonance / (float) poloidalResonance )
        {
          toroidalResonance = 1;
          poloidalResonance = 1;
        }
        else
        {
          cerr << "Using resonance from the winding pairs." << endl;
        }
      }

      else
      {
        toroidalResonance = 1;
        poloidalResonance = 1;
      }

    }
  }

  unsigned int resonanceGCD = GCD( toroidalResonance, poloidalResonance );

  if( verboseFlag )
    cerr << "Winding Pair " << toroidalWinding << "," << poloidalWinding << "  "
         << "GCD = " << windingGCD << "   "
         << "Toroial, Poloidal resonances = "
         << toroidalResonance << "," << poloidalResonance << "  "
         << "GCD = " << resonanceGCD << "   "
         << endl;

  // Check for islands and islands around islands.  NOTE: Even with
  // islands the poloidal resonance can be one as such only check
  // the toroidal resonance
  if( (type == FieldlineProperties::UNKNOWN_TYPE ||
       type == FieldlineProperties::ISLAND_CHAIN ||
       type == FieldlineProperties::ISLANDS_WITHIN_ISLANDS ) &&

      toroidalResonance > 1 ) // && poloidalResonance >= 1 ) // Always true.
  {
    // Set the windings to reflect the resonances which is the number
    // of islands.
    toroidalWinding  = toroidalResonance;
    poloidalWinding  = poloidalResonance;
    poloidalWindingP = poloidalResonance;

    // The number of islands is always the toroidal resonance.
    islands = toroidalResonance;
    islandGroups = toroidalResonance / resonanceGCD;

    // If the resonance GCD is 1 then only one island per group thus a
    // simple island chain.
    if( resonanceGCD == 1 )
    {
      type = FieldlineProperties::ISLAND_CHAIN;

      if( verboseFlag )
        cerr << "Primary resonances = "
             << toroidalResonance << "," << poloidalResonance << " with "
             << islands << " islands "
             << endl;
    }
    // The resonance GCD (aka second order resonance) is the number of
    // smaller islands around an island.
    else
    {
      type = FieldlineProperties::ISLANDS_WITHIN_ISLANDS;

      if( verboseFlag )
        cerr << "Secondary resonances = "
             << toroidalResonance << "," << poloidalResonance << " with "
             << islands << " islands "
             << "(" << islands / resonanceGCD << " groups with "
             << resonanceGCD << " islands each)"
             << endl;
    }

    // When the drawable winding pair is the resonance then potentially
    // in a chaotic area as no other better solutions are drawable.
    if( toroidalResonance == mergedWindingPairs[drawableIndex].toroidal &&
        poloidalResonance == mergedWindingPairs[drawableIndex].poloidal )
    {
      // The best guestimate of number of nodes will be the GCD of the
      // best winding pair. If there is a tie it does not matter as it
      // is a guestimate.

      // Note: when the island is intering the chaotic zone the nnodes
      // will not be stable between two tracings of the boundary.
      nnodes = GCD( mergedWindingPairs[0].toroidal,
                    mergedWindingPairs[0].poloidal ) / resonanceGCD;

      // Less than the maximum number punctures allowed so add more
      // puncture points.
      if( poloidal_puncture_pts.size() < fi.maxPunctures )
      {
        analysisState = FieldlineProperties::ADDING_POINTS;

        // Get the number of nodes per island.
        unsigned int nodes = poloidal_puncture_pts.size() / islands / 2;
        
        // Add two more puncture points per island within and island.
        if( type == FieldlineProperties::ISLANDS_WITHIN_ISLANDS )
          nPuncturesNeeded = (nodes + 2) * islands * 2;
        // Add five more puncture points per island.
        else //if( type == FieldlineProperties::ISLAND_CHAIN )
          nPuncturesNeeded = (nodes + 5) * islands * 2;

        if( verboseFlag )
          cerr << endl
               << "Not enough puncture points yet; "
               << "need " << nPuncturesNeeded << " "
               << "have " << poloidal_puncture_pts.size() << " "
               << "asking for " << nPuncturesNeeded << " puncture points"
               << endl;
      }

      // Reached the user set maximum number of punctures allowed.
      else //if( poloidal_puncture_pts.size() >= fi.maxPunctures )
      {
        analysisState = FieldlineProperties::TERMINATED;

        if( verboseFlag )
          cerr << "Potentially within the chaotic regime." << endl;
      }
    }

    // When the correct number of points is found the winding pair
    // will be drawable and the winding GCD divided by the resonance GCD
    // will be the number of points in the cross section of each
    // island within an island.

    // Note: for a simple island chain the resonance GCD will 1.
    else // Possible solution.
    {
      nnodes = windingGCD / resonanceGCD;

      unsigned int nnodesPlus1 = nnodes + 1;

      // Try to get at least four points per island.
      if( nnodes < 4 )
      {
        analysisState = FieldlineProperties::ADDING_POINTS;

        nPuncturesNeeded = 4 * islands * 2;

        if( nPuncturesNeeded <= poloidal_puncture_pts.size() )
        {
          nPuncturesNeeded = 0;
          analysisState = FieldlineProperties::TERMINATED;
        }

        else
        {
          if( verboseFlag )
            cerr << "Adding puncture points for four points per island; "
                 << "have " << poloidal_puncture_pts.size() << " "
                 << "asking for " << nPuncturesNeeded << " puncture points"
                 << endl;
        }
      }

      // For a 1:1 island the nnodes will get stuck in a local minimum
      // of 1,2, or 3. At least that is our observation. As such, add
      // points to see if the analysis can get out of the local
      // minimum.
      else if( type == FieldlineProperties::ISLAND_CHAIN &&
               toroidalWinding == 1 && poloidalWinding == 1 && nnodes <= 5 )
      {
        nnodes = poloidal_puncture_pts.size() / toroidalWinding / 2;

        analysisState = FieldlineProperties::ADDING_POINTS;

        nPuncturesNeeded = poloidal_puncture_pts.size() + 4;

        if( verboseFlag )
          cerr << "Local minimum, not enough puncture points; "
               << "need " << nPuncturesNeeded << " "
               << "have " << poloidal_puncture_pts.size() << " "
               << "asking for " << nPuncturesNeeded << " puncture points"
               << endl;
      }

      // Get enough points so that the full toroidal and poloidal
      // periods can be analyzed.
      else if( poloidal_puncture_pts.size() < 2*(toroidalResonance*nnodesPlus1) ||
               ridgeline_points.size()      < 2*(poloidalResonance*nnodesPlus1) )
      {
        analysisState = FieldlineProperties::ADDING_POINTS;

        // For the toroidal period allow for one more possible period
        // to be exaimed.
        if( nPuncturesNeeded < 2.0 * (toroidalResonance*nnodesPlus1) )
        {
          nPuncturesNeeded = 2.0 * (toroidalResonance*nnodesPlus1);
        
          if( verboseFlag )
            cerr << "Not enough puncture points; "
                 << "need " << nPuncturesNeeded << " "
                 << "have " << poloidal_puncture_pts.size() << " "
                 << "asking for " << nPuncturesNeeded << " puncture points"
                 << endl;
        }

        // For the poloidal period allow for one more possible period
        // to be exaimed which requires two more additional toroidal
        // punctures.
        if( nPuncturesNeeded <
            2.0 * (poloidalResonance*(nnodes+2)) * local_safetyFactor + 0.5)
        {
          nPuncturesNeeded =
            2.0 * (poloidalResonance*(nnodes+2)) * local_safetyFactor + 0.5;

          if( verboseFlag )
            cerr << "Not enough ridgeline points; "
                 << "need " << 2*(poloidalResonance*nnodes+2) << " "
                 << "have " << ridgeline_points.size() << " "
                 << "asking for " << nPuncturesNeeded << " puncture points"
                 << endl;
        }
      }

      else if( nPuncturesNeeded == 0 )
      {
        if( detectIslandCenters )
        {
          findIslandCenters( poloidal_puncture_pts, toroidalWinding, nnodes,
                             islandCenters );
          
          if( islandCenters.empty() )
            analysisState = FieldlineProperties::COMPLETED;
          else
            analysisState = FieldlineProperties::ADD_O_POINTS;
        }
        else
          analysisState = FieldlineProperties::COMPLETED;
      }

      // Check to see if the fieldline is periodic. I.e. on a rational
      // surface.  If within "delta" of the distance the fieldline is
      // probably on a rational surface.

      // NOTE: Define the rational based on the lowest order of teh
      // surface. Thus divide by the GCD.
      if( rationalCheck( poloidal_puncture_pts,
                         toroidalWinding / GCD(toroidalWinding, poloidalWinding ),
                         nnodes, delta*0.1 ) ) 
      {
        type = FieldlineProperties::O_POINT;
        analysisState = FieldlineProperties::COMPLETED;
        
        if( verboseFlag )
          cerr << "Appears to be an O point " << delta*0.1 << endl;
      }
    }

    windingGroupOffset = Blankinship( toroidalWinding, poloidalWinding );
  }

  // Check to see if the fieldline is periodic. I.e. on a rational
  // surface.  If within "delta" of the distance the fieldline is
  // probably on a rational surface.
  else if( rationalCheck( poloidal_puncture_pts,
                          toroidalWinding / GCD(toroidalWinding, poloidalWinding ),
                          nnodes, delta*0.1 ) ) 
  {
    type = FieldlineProperties::RATIONAL;
    islands = 0;
    islandGroups = 0;

    analysisState = FieldlineProperties::COMPLETED;

    if( verboseFlag )
      cerr << "Appears to be a rational surface " << delta*0.1 << endl;

    windingGroupOffset = Blankinship( toroidalWinding, poloidalWinding );
  }

  // At this point assume the surface is irrational.
  else
  {
    type = FieldlineProperties::FLUX_SURFACE;
    islands = 0;
    islandGroups = 0;

    windingGroupOffset = Blankinship( toroidalWinding, poloidalWinding );

    // Get the direction based on the first two points in a group.
    Vector v0 = (Vector) poloidal_puncture_pts[toroidalWinding] -
      (Vector) poloidal_puncture_pts[0];

    // Get the direction based on the first points from adjacent groups.
    Vector v1 = (Vector) poloidal_puncture_pts[windingGroupOffset] -
      (Vector) poloidal_puncture_pts[0];
      
    // If the windingGroupOffset and point ordering are opposite in
    // directions then the previous group is the
    // windingGroupOffset. Otherwise is they have the same direction
    // then toroidalWinding-windingGroupOffset is the previous group.
    int offsetDir = (Dot( v0, v1 ) > 0.0 ? 1 : -1);
      
    vector< unsigned int > nodes(toroidalWinding);

    nnodes = 0;

    // Find the first point from another group that overlaps the first
    // group. This only works if there is an overlap between groups.
    for( unsigned int i=0; i<toroidalWinding; ++i ) 
    {
      nodes[i] = poloidal_puncture_pts.size() / toroidalWinding;

      // The next group
      unsigned int j =
        (i+offsetDir*windingGroupOffset + toroidalWinding) % toroidalWinding;
        
      Vector firstPoint = (Vector) poloidal_puncture_pts[j];
      Vector nextPoint  = (Vector) poloidal_puncture_pts[j+toroidalWinding];
        
      Vector v0 = nextPoint - firstPoint;

      for( unsigned int k=i; k<poloidal_puncture_pts.size(); k+=toroidalWinding ) 
      {
        Vector  testPoint = (Vector) poloidal_puncture_pts[k];
        
        Vector v1 = testPoint - firstPoint;
        
        if( (poloidalWinding > 2 || v1.length() / v0.length() < 5.0)
            && Dot( v0, v1 ) > 0.0 )
        {
          analysisState = FieldlineProperties::COMPLETED;
          
          nodes[i] = k / toroidalWinding;

          break;
        }
      }

      if( nodes[i] < 1 )
        nodes[i] = poloidal_puncture_pts.size() / toroidalWinding;

      nnodes += nodes[i];
    }

    // Get the average number of nodes.
    nnodes /= (float) toroidalWinding;

    unsigned int cc = 0;
    unsigned int index = 0;
    unsigned int new_nnodes = 0;
      
    for( unsigned int i=0; i<toroidalWinding; i++ )
    {
      if( nodes[i] < nnodes - 1 || nnodes + 1 < nodes[i] )
      {
        cc++;
        index = i;
      }
      else
        new_nnodes += nodes[i];
    }
    
    if( cc == 1 )
    {
      nnodes = new_nnodes / (float) (toroidalWinding-cc);
    }
      
    for( unsigned int i=0; i<toroidalWinding; i++ )
    {
      if( nodes[i] < nnodes - 1 || nnodes + 1 < nodes[i] )
      {
        if( verboseFlag )
          cerr << "Appears to be a flux surface but not self consistent, "
               << "average number of nodes  " << nnodes << "   (  ";
          
        for( unsigned int i=0; i<toroidalWinding; i++ )
          if( verboseFlag )
            cerr << nodes[i] << "  ";

        if( verboseFlag )
          cerr << ")" << endl;

        break;
      }
    }

    if( analysisState != FieldlineProperties::COMPLETED )
    {
      unsigned int additionalPts = 0;

      // Get the direction based on the first two points in a group.
      Vector v0 = (Vector) poloidal_puncture_pts[toroidalWinding] -
        (Vector) poloidal_puncture_pts[0];

      // Get the direction based on the first points from adjacent groups.
      Vector v1 = (Vector) poloidal_puncture_pts[windingGroupOffset] -
        (Vector) poloidal_puncture_pts[0];

      // If the windingGroupOffset and point ordering are opposite in
      // directions then the previous group is the
      // windingGroupOffset. Otherwise is they have the same direction
      // then toroidalWinding-windingGroupOffset is the previous
      // group.
      int offsetDir = (Dot( v0, v1 ) > 0.0 ? 1 : -1);

      for( unsigned int i=0; i<toroidalWinding; ++i ) 
      {
        // The next group
        unsigned int j =
          (i+offsetDir*windingGroupOffset + toroidalWinding) % toroidalWinding;

        Vector firstPoint = (Vector) poloidal_puncture_pts[j];
        Vector nextPoint  = (Vector) poloidal_puncture_pts[j+toroidalWinding];
          
        Vector  lastPoint =
          (Vector) poloidal_puncture_pts[i+(nnodes-1)*toroidalWinding];
        Vector  prevPoint =
          (Vector) poloidal_puncture_pts[i+(nnodes-2)*toroidalWinding];
          
        Vector v0 = nextPoint - firstPoint;
        Vector v1 = lastPoint - firstPoint;

        unsigned int needPts = ( (firstPoint-lastPoint).length() /
                                 (prevPoint-lastPoint).length() + 0.5 );
          
        if( additionalPts < needPts )
          additionalPts = needPts;
      }

      if( additionalPts )
      {
        if( nPuncturesNeeded == 0 )
          nPuncturesNeeded = (nnodes+additionalPts) * toroidalWinding + 1;
        
        if( verboseFlag )
          cerr << "Too few puncture points, at least "
               << (nnodes+additionalPts) * toroidalWinding
               << " are needed to complete the boundary."
               << endl;
      }
      else
        analysisState = FieldlineProperties::COMPLETED;
    }

    if( toroidalWinding == poloidalWinding )
    {
      vector< pair< Point, unsigned int > > hullPts;
      
      hullPts.resize( toroidalWinding+1 );
      
      for(unsigned int i=0; i<toroidalWinding; ++i )
      {
        hullPts[i] =
          pair< Point, unsigned int >( poloidal_puncture_pts[i], i );
      }
      
      unsigned int m = 0; // starting index
      
      convexHull( hullPts, m, toroidalWinding, 1 );

      if( m != toroidalWinding )
      {
        if( verboseFlag )
          cerr << "The surface does not have a convex hull, "
               << toroidalWinding-m << " point(s) are missing."
               << endl; 
      }
      
      map< unsigned int, unsigned int > offsets;
      map< unsigned int, unsigned int >::iterator ic;
      
      unsigned int offset;
      
      // Find all the differences and count each one.
      for(unsigned int i=0; i<toroidalWinding; ++i )
      {
        offset = ( (hullPts[i                    ].second -
                    hullPts[(i+1)%toroidalWinding].second) +
                   toroidalWinding ) % toroidalWinding;
        
        // Find this offset in the list.
        ic = offsets.find( offset );
        
        // Not found, new offset.
        if( ic == offsets.end() )
          offsets.insert( pair<int, int>( offset, 1) );
        // Found this difference, increment the count.
        else
          (*ic).second++;
      }

      if( offsets.size() != 1 )
        if( verboseFlag )
          cerr << "Multiple offsets  ";

      // Find the difference that occurs most often.
      unsigned int nMatches = 0;
      
      ic = offsets.begin();
      
      while( ic != offsets.end() )
      {
        if( offsets.size() != 1 )
          if( verboseFlag && (*ic).second > 1 )
            cerr << (*ic).first << " (" << (*ic).second << ")  ";

        if( nMatches < (*ic).second )
        {
          offset = (*ic).first;
          nMatches = (*ic).second;
        }

        ++ic;
      }

      if( offsets.size() != 1 )
        if( verboseFlag )
          cerr << endl;

      // Secondary angle around the nonaxisymmetric island.
      if( offset != 1 && offset != toroidalWinding-1 )
      {
        poloidalWindingP = Blankinship( toroidalWinding, offset );
        
        if( verboseFlag )
          cerr << "Secondary poloidal rotation  "
               << toroidalWinding << "," << poloidalWindingP << "  "
               << "with offset " << offset << endl;
      }
      else
        poloidalWindingP = poloidalWinding;
    }
    else
      poloidalWindingP = poloidalWinding;
  }

  // The user has set the toroidal winding get the poloidal winding
  // based on the data found.
  if( overrideToroidalWinding )
  {
    toroidalWinding = overrideToroidalWinding;

    unsigned int nsets = poloidalWindingCounts.size();

    map< unsigned int, unsigned int > differenceCount;
    map< unsigned int, unsigned int >::iterator ic;

    // Find all the differences and count each one.
    for( unsigned int i=0; i<nsets-toroidalWinding; ++i)
    {
      // Get the poloidal winding between two counts.
      poloidalWinding = poloidalWindingP =
        poloidalWindingCounts[i+toroidalWinding] - poloidalWindingCounts[i];

      // Find this difference in the list.
      ic = differenceCount.find( poloidalWinding );

      // Not found, new difference.
      if( ic == differenceCount.end() )
        differenceCount.insert( pair<int, int>( poloidalWinding, 1) );
      // Found this difference, increment the count.
      else (*ic).second++;
    }

    // Find the difference that occurs most often.
    unsigned int nMatches = 0;
    
    ic = differenceCount.begin();
    
    while( ic != differenceCount.end() )
    {
      if( nMatches < (*ic).second )
      {
        poloidalWinding  = (*ic).first;
        poloidalWindingP = (*ic).first;
        nMatches = (*ic).second;
      }

      ++ic;
    }


    double consistency = (double) nMatches / (double) (nsets-toroidalWinding);

    if( overridePoloidalWinding )
    {
      poloidalWinding  = overridePoloidalWinding;
      poloidalWindingP = overridePoloidalWinding;
    }

    double local_safetyFactor =
      (double) toroidalWinding / (double) poloidalWinding;

    windingGroupOffset = Blankinship( toroidalWinding, poloidalWinding );

    nnodes = poloidal_puncture_pts.size() / toroidalWinding;

    if( verboseFlag )
      cerr << "overriding" << endl
           << "**using**   winding pair "
           << toroidalWinding << ","
           << poloidalWinding << "  ("
           << local_safetyFactor << " - "
           << fabs(safetyFactor - local_safetyFactor) << ")  "
           << "consistency "
           << 100.0 * consistency
           << " %" << endl;
  }

  // Record the analysis.
  fi.analysisState = analysisState;

  fi.type = type;

  fi.safetyFactor = safetyFactor;
  fi.toroidalWinding = toroidalWinding;
  fi.poloidalWinding = poloidalWinding;
  fi.poloidalWindingP = poloidalWindingP;
  fi.toroidalResonance = toroidalResonance;
  fi.poloidalResonance = poloidalResonance;
  fi.windingPairs = windingPairs;
  fi.windingGroupOffset = windingGroupOffset;
  fi.islands = islands;
  fi.islandGroups = islandGroups;
  fi.nnodes  = nnodes;

  if( poloidal_puncture_pts.size() > fi.maxPunctures &&
      (analysisState != FieldlineProperties::COMPLETED ||
       analysisState != FieldlineProperties::TERMINATED) )
  {
    cerr << "0 TERMINATING  " << nPuncturesNeeded << endl;

    analysisState = FieldlineProperties::TERMINATED;
    fi.nPuncturesNeeded = 0;
  }
  else
    fi.nPuncturesNeeded = nPuncturesNeeded;

  fi.OPoints.clear();

  if( !(islandCenters.empty()) )
  {
    for( unsigned int i=0; i<islandCenters.size(); ++i )
    {
      cerr << islandCenters[i] << endl;
      fi.OPoints.push_back( Point( islandCenters[i].x, 
                                   islandCenters[i].y,
                                   islandCenters[i].z) );
    }
  }

//   FieldlineProperties tmp;

//   if( analysisState == FieldlineProperties::COMPLETED )
//     fieldlineProperties2( ptList, tmp );
}


void
FieldlineLib::fieldlineProperties2( vector< Point > &ptList,
                                    FieldlineProperties &fi )
{
  vector< Point > poloidal_puncture_pts;
  vector< Point > ridgeline_points;

  vector< double > rotationalSums;
  vector< unsigned int > poloidalWindingCounts;

  float delta = 0.0;

  getPunctures( ptList, Vector(0,1,0), poloidal_puncture_pts );

  if( ptList.empty() && poloidal_puncture_pts.empty() )
  {
    fi.type = FieldlineProperties::UNKNOWN_TYPE;
    fi.analysisState = FieldlineProperties::UNKNOWN_STATE;

    fi.safetyFactor = 0;
    fi.toroidalWinding = 0;
    fi.poloidalWinding = 0;
    fi.poloidalWindingP = 0;
    fi.toroidalResonance = 0;
    fi.poloidalResonance = 0;
    fi.windingPairs.clear();
    fi.windingGroupOffset = 0;
    fi.islands = 0;
    fi.islandGroups = 0;
    fi.nnodes  = 0;

    fi.nPuncturesNeeded  = 0;

    fi.OPoints.clear();

    return;
  }

  // Start the analysis.
  FieldlineProperties::FieldlineType type = FieldlineProperties::UNKNOWN_TYPE;
  FieldlineProperties::AnalysisState analysisState = FieldlineProperties::UNKNOWN_STATE;

  unsigned int toroidalWinding = 0;
  unsigned int toroidalResonance = 1;

  unsigned int nnodes = 0;
  unsigned int islands = 0, islandGroups = 0;
  unsigned int nPuncturesNeeded = 0;

  unsigned int toroidalWindingMax = poloidal_puncture_pts.size() / 2;

  // Find the best toroidal periodicity. For a flux surface the period
  // will be the toroidal winding number. For an island chain the
  // period will be the toroidal winding number times the number of
  // nodes.
  if( verboseFlag )
    cerr << "Toroidal Winding via "
         << poloidal_puncture_pts.size() << "  "
         << "poloidal punctures, "
         << "max period " << toroidalWindingMax
         << endl;

  vector< pair< unsigned int, double > > toroidalStats;

  periodicityStats( poloidal_puncture_pts, toroidalStats,
                    toroidalWindingMax, 2 );


  int drawableRank  = -1;
  int drawableIndex = -1;
  vector< unsigned int > drawableIndexs;

  for( unsigned int i=0; i<toroidalStats.size(); ++i )
  {
    bool drawable = false;

    for( unsigned int j=1; j<toroidalStats[i].first; ++j )
    {
      if( IntersectCheck( poloidal_puncture_pts, toroidalStats[i].first, j ) )
      {
        drawable = true;
        break;
      }
    }

    if( verboseFlag && (drawable || i < 10) )
      cerr << "Final "
           << (drawable ? "Drawable " : "Rejected ") 
           << "toroidal winding :  " 
           << toroidalStats[i].first << endl;

    if( (drawableRank == -1 ||
         drawableRank == toroidalStats[i].second) &&

        // Keep only those that are drawable.
        drawable )
    {
      drawableIndexs.push_back( i );

      // If a tie take the higher order winding.
      if( toroidalWinding < toroidalStats[i].first )
      {
        toroidalWinding = toroidalStats[i].first;

        drawableRank = toroidalStats[i].first;
        drawableIndex = i;
      }
    }
  }

  if( drawableIndex == -1 )
  {
    toroidalWinding = 0;

    if( verboseFlag )
      cerr << "Garbage matches adding more points" << endl;

    fi.nPuncturesNeeded = poloidal_puncture_pts.size() * 1.25;

    return;
  }

  if( toroidalStats.size() == 1 )
  {
    toroidalResonance = 1;
  }
  else
  {
    // The base values are the primary resonances if present.
    toroidalResonance = ResonanceCheck( toroidalStats, 1, 3 );
  }

  if( verboseFlag )
    cerr << "Toroidal Winding " << toroidalWinding << "  "
         << "Toroial Resonances = " << toroidalResonance << "  "
         << endl;

  // Check for islands and islands around islands.
  // NOTE: Even with islands the poloidalFirstResonance can be one as such
  // only check the toroidalFirstResonance
  if( (type == FieldlineProperties::UNKNOWN_TYPE ||
       type == FieldlineProperties::ISLAND_CHAIN ||
       type == FieldlineProperties::ISLANDS_WITHIN_ISLANDS ) &&

      toroidalResonance > 1 ) // && poloidalResonance >= 1 ) // Always true.
  {
    // Set the windings to reflect the resonances which is the number
    // of islands.
    toroidalWinding = toroidalResonance;

    // The number of islands is always the toroidal resonance.
    islands = toroidalResonance;

    // If the resonance GCD is 1 then only one island per group thus a
    // simple island chain.
    if( isPrime( toroidalResonance ) == 0 )
    {
      type = FieldlineProperties::ISLAND_CHAIN;

      if( verboseFlag )
        cerr << "Primary resonances = "
             << toroidalResonance << " with "
             << islands << " islands "
             << endl;
    }
    // The resonance GCD (aka secondary resonance) is the number of
    // smaller islands around an island.
    else
    {
      type = FieldlineProperties::ISLANDS_WITHIN_ISLANDS;

      if( verboseFlag )
        cerr << "Secondary resonances = "
             << toroidalResonance << " with "
             << islands << " islands "
             << endl;
    }

    // When the drawable winding pair is the resonance then potentially
    // in a chaotic area as no other better solutions are drawable.
    if( toroidalResonance == toroidalStats[drawableIndex].first )
    {
      // The best guestimate of number of nodes will be the GCD of the
      // best winding pair. If there is a tie it does not matter as it
      // is a guestimate.

      // Note: when the island is intering the chaotic zone the nnodes
      // will not be stable between two tracings of the boundary.
      nnodes = toroidalStats[0].first / toroidalResonance;

      // Less than the maximum number punctures allowed so add more
      // puncture points.
      if( poloidal_puncture_pts.size() < fi.maxPunctures )
      {
        analysisState = FieldlineProperties::ADDING_POINTS;

        // Get the number of nodes per island.
        unsigned int nodes = poloidal_puncture_pts.size() / islands / 2;
        
        // Add two more puncture points per island within and island.
        if( type == FieldlineProperties::ISLANDS_WITHIN_ISLANDS )
          nPuncturesNeeded = (nodes + 2) * islands * 2;
        // Add five more puncture points per island.
        else //if( type == FieldlineProperties::ISLAND_CHAIN )
          nPuncturesNeeded = (nodes + 5) * islands * 2;

        if( verboseFlag )
          cerr << endl
               << "Not enough puncture points yet; "
               << "need " << nPuncturesNeeded << " "
               << "have " << poloidal_puncture_pts.size() << " "
               << "asking for " << nPuncturesNeeded << " puncture points"
               << endl;
      }

      // Reached the user set maximum number of punctures allowed.
      else //if( poloidal_puncture_pts.size() >= fi.maxPunctures )
      {
        analysisState = FieldlineProperties::TERMINATED;

        if( verboseFlag )
          cerr << "Potentially within the chaotic regime." << endl;
      }
    }

    // When the correct number of points is found the winding pair
    // will be drawable and the winding GCD divided by the resonance GCD
    // will be the number of points in the cross section of each
    // island within an island.

    // Note: for a simple island chain the resonance GCD will 1.
    else // Possible solution.
    {
      nnodes = toroidalStats[0].first / toroidalResonance;

      unsigned int nnodesPlus1 = nnodes + 1;

      // Try to get at least four points per island.
      if( nnodes < 4 )
      {
        analysisState = FieldlineProperties::ADDING_POINTS;

        nPuncturesNeeded = 4 * islands * 2;

        if( nPuncturesNeeded <= poloidal_puncture_pts.size() )
        {
          nPuncturesNeeded = 0;
          analysisState = FieldlineProperties::TERMINATED;
        }

        else
        {
          if( verboseFlag )
            cerr << "Adding puncture points for four points per island; "
                 << "have " << poloidal_puncture_pts.size() << " "
                 << "asking for " << nPuncturesNeeded << " puncture points"
                 << endl;
        }
      }

      // For a 1:1 island the nnodes will get stuck in a local minimum
      // of 1,2, or 3. At least that is our observation. As such, add
      // points to see if the analysis can get out of the local
      // minimum.
      else if( type == FieldlineProperties::ISLAND_CHAIN &&
               toroidalWinding == 1 && nnodes <= 5 )
      {
        nnodes = poloidal_puncture_pts.size() / toroidalWinding / 2;

        analysisState = FieldlineProperties::ADDING_POINTS;

        nPuncturesNeeded = poloidal_puncture_pts.size() + 4;

        if( verboseFlag )
          cerr << "Local minimum, not enough puncture points; "
               << "need " << nPuncturesNeeded << " "
               << "have " << poloidal_puncture_pts.size() << " "
               << "asking for " << nPuncturesNeeded << " puncture points"
               << endl;
      }

      // Get enough points so that the full toroidal and poloidal
      // periods can be analyzed.
      else if( poloidal_puncture_pts.size() < 2*(toroidalResonance*nnodesPlus1) )
      {
        analysisState = FieldlineProperties::ADDING_POINTS;

        // For the toroidal period allow for one more possible period
        // to be exaimed.
        if( nPuncturesNeeded < 2.0 * (toroidalResonance*nnodesPlus1) )
        {
          nPuncturesNeeded = 2.0 * (toroidalResonance*nnodesPlus1);
        
          if( verboseFlag )
            cerr << "Not enough puncture points; "
                 << "need " << nPuncturesNeeded << " "
                 << "have " << poloidal_puncture_pts.size() << " "
                 << "asking for " << nPuncturesNeeded << " puncture points"
                 << endl;
        }
      }

      else if( nPuncturesNeeded == 0 )
      {
        analysisState = FieldlineProperties::COMPLETED;
      }

      // Check to see if the fieldline is periodic. I.e. on a rational
      // surface.  If within "delta" of the distance the fieldline is
      // probably on a rational surface.
      if( rationalCheck( poloidal_puncture_pts,
                         toroidalWinding,
                         nnodes, delta*0.1 ) ) 
      {
        type = FieldlineProperties::O_POINT;
        analysisState = FieldlineProperties::COMPLETED;
        
        if( verboseFlag )
          cerr << "Appears to be an O point " << delta*0.1 << endl;
      }
    }
  }

  // Check to see if the fieldline is periodic. I.e. on a rational
  // surface.  If within "delta" of the distance the fieldline is
  // probably on a rational surface.
  else if( rationalCheck( poloidal_puncture_pts,
                          toroidalWinding,
                          nnodes, delta*0.1 ) ) 
  {
    type = FieldlineProperties::RATIONAL;
    islands = 0;
    islandGroups = 0;

    analysisState = FieldlineProperties::COMPLETED;

    if( verboseFlag )
      cerr << "Appears to be a rational surface " << delta*0.1 << endl;
  }

  // At this point assume the surface is irrational.
  else
  {
    type = FieldlineProperties::FLUX_SURFACE;
    islands = 0;
    islandGroups = 0;

    // Get the average number of nodes.
    nnodes = poloidal_puncture_pts.size() / toroidalWinding;
  }

  // Record the analysis.
  fi.analysisState = analysisState;

  fi.type = type;

  fi.toroidalWinding = toroidalWinding;
  fi.toroidalResonance = toroidalResonance;
  fi.islands = islands;
  fi.nnodes  = nnodes;

  if( poloidal_puncture_pts.size() > fi.maxPunctures &&
      (analysisState != FieldlineProperties::COMPLETED ||
       analysisState != FieldlineProperties::TERMINATED) )
  {
    cerr << "1 TERMINATING  " << nPuncturesNeeded << endl;

    analysisState = FieldlineProperties::TERMINATED;
    fi.nPuncturesNeeded = 0;
  }
  else
    fi.nPuncturesNeeded = nPuncturesNeeded + (nPuncturesNeeded ? 1 : 0);

  fi.OPoints.clear();
}


unsigned int
FieldlineLib::
islandProperties( vector< Point > &points,
                  Vector &baseCentroid,
                  unsigned int &startIndex,
                  unsigned int &middleIndex,
                  unsigned int &stopIndex,
                  unsigned int &nodes )
{
  // Get the local centroid for the group.
  Vector localCentroid(0,0,0);

  for( unsigned int i=0; i<points.size(); i++ )
    localCentroid += (Vector) points[i];

  localCentroid /= (float) points.size();

  // Get the principal axes of the points.
  float Ixx = 0.0;
  float Ixz = 0.0;
  float Izz = 0.0;

  double maxDist = 0;

  // Get the moments of intertial for each point. It assumed that
  // everything is in the Y plane as such there the moments of
  // intertial along the Y axis are zero.
  for( unsigned int i=0; i<points.size(); i++ ) {

    Vector vec = (Vector) points[i] - localCentroid;
    
    if( maxDist < vec.length() )
      maxDist = vec.length();
    
    Ixx += vec.z*vec.z;
    Ixz -= vec.x*vec.z;
    Izz += vec.x*vec.x;
  }
  
  // Short cut to the principal axes because the Y moments of
  // intertial are zero.
  float alpha = atan( 2.0 * Ixz / (Ixx - Izz) ) / 2.0;

  //       cerr << "PRINCIPAL AXES " << alpha * 180.0 / M_PI << "    "
  //       << Ixx + Ixz * sin(alpha       )/cos(alpha       ) << "    "
  //       << Izz + Ixz * cos(alpha+M_PI/2)/sin(alpha+M_PI/2) << endl;

  // Use the principal axes to get an offset from the local
  // centroid which gives a point outside the island.
  baseCentroid = localCentroid;
  
  if( Ixx + Ixz * sin(alpha       )/cos(alpha       ) >
      Izz + Ixz * cos(alpha+M_PI/2)/sin(alpha+M_PI/2) )
    baseCentroid -= Vector(  cos(alpha), 0, sin(alpha) ) * maxDist;
  else
    baseCentroid -= Vector( -sin(alpha), 0, cos(alpha) ) * maxDist;

  // Determine if islands exists. If an island exists there will be
  // both clockwise and counterclockwise sections when compared to the
  // global centroid.
  unsigned int turns = 0;

  startIndex = middleIndex = stopIndex = 0;

  Vector v0 = (Vector) points[0] - baseCentroid;
  Vector v1 = (Vector) points[1] - baseCentroid;

  bool lastCCW = (ccw(v0, v1) == 1);
  v0 = v1;

  for( unsigned int j=2; j<points.size(); j++ ) {

    v1 = (Vector) points[j] - baseCentroid;

    bool CCW = (ccw(v0, v1) == 1);
    v0 = v1;

    if( CCW != lastCCW ) {
      turns++;

      if( turns == 1 )      startIndex  = j - 1;
      else if( turns == 2 ) middleIndex = j - 1;
      else if( turns == 3 ) stopIndex   = j - 1;

      if( turns == 3 )
        break;

      lastCCW = CCW;
    }
  }

  if( turns == 0 ) {
    stopIndex   = points.size() - 1;
    middleIndex = 0;
    startIndex  = 0;

    nodes = points.size();
  } else if( turns == 1 ) {
    stopIndex   = points.size() - 1;
    middleIndex = startIndex;
    startIndex  = 0;

    nodes = points.size();
  } else {

    if( turns == 3 ) {

      // Check for a negative epsilon.
      double length0 = 0;
      double length1 = 0;

      for( unsigned j=0, k=nodes, l=nodes+1; l<points.size(); j++, k++, l++ ) {
        length0 += ((Vector) points[j] - (Vector) points[k]).length();
        length1 += ((Vector) points[j] - (Vector) points[l]).length();
      }

      if( length0 < length1 )
        stopIndex--;

      if( 2*startIndex == middleIndex + 1 ) {
        // First point is actually the start point.
//      cerr << "First point is actually the start point.\n";

        stopIndex   = middleIndex;
        middleIndex = startIndex;
        startIndex  = 0;
      }
    } else if( turns == 2 ) {

      if( 2*startIndex == middleIndex + 1 ) {
        // First point is actually the start point.
//      cerr << "First point is actually the start point.\n";

        stopIndex   = middleIndex;
        middleIndex = startIndex;
        startIndex  = 0;
      
        turns = 3;

      } else if( points.size() < 2 * (middleIndex - startIndex) - 1 ) {
        // No possible over lap.
//      cerr <<  "islandProperties - No possible over lap.\n";

        stopIndex = startIndex + points.size() - 1;
      }
    }

    nodes = stopIndex - startIndex + 1;

    for( unsigned int j=middleIndex, k=middleIndex+1;
         j<points.size() && k<points.size();
         ++j, ++k ) {

      // See if the test point is between the first section.
      if( Dot( (Vector) points[0] - (Vector) points[j],
               (Vector) points[1] - (Vector) points[j] )
          < 0.0 ) {
        stopIndex = startIndex + j;
        nodes = j;
        turns = 3;
//      cerr << "islandProperties - A point overlaps the first section at " << j-1 << endl;
        break;
      }

      // See if the first point is between the test section.
      if( Dot( (Vector) points[j] - (Vector) points[0],
               (Vector) points[k] - (Vector) points[0] )
          < 0.0 ) {
        stopIndex = startIndex + j;
        nodes = j;
        turns = 3;
//      cerr <<  "islandProperties - First point overlaps another section after " << j-1 << endl;
        break;
      }
    }
      
    // No overlap found
    if( turns == 2 ) {
      stopIndex = startIndex + points.size() - 1;
      nodes = points.size();
//      cerr << "islandProperties - No overlap found\n";
    }
  }

  return turns;
}


unsigned int
FieldlineLib::
surfaceOverlapCheck( vector< vector< Point > > &bins,
                     unsigned int toroidalWinding,
                     unsigned int windingGroupOffset,
                     unsigned int &nnodes )
{
  nnodes = bins[0].size();

  // First make sure none of the groups overlap themselves.
  for( unsigned int i=0; i<toroidalWinding; i++ ) {

    if( nnodes > bins[i].size() )
      nnodes = bins[i].size();

    for( unsigned int j=2; j<nnodes; j++ ) {

      Vector v0 = (Vector) bins[i][0] - (Vector) bins[i][j];
      Vector v1 = (Vector) bins[i][1] - (Vector) bins[i][j];
      
      if( Dot( v0, v1 ) < 0.0 ) {
        nnodes = j;
        break;
      }

      Vector midPt = ((Vector) bins[i][j] + (Vector) bins[i][j-1]) / 2.0;

      v0 = (Vector) bins[i][0] - midPt;
      v1 = (Vector) bins[i][1] - midPt;
      
      if( Dot( v0, v1 ) < 0.0 ) {
        nnodes = j;
        break;
      }

    }
  }

  if( toroidalWinding == 1 || nnodes == 1 )
    return nnodes;

  // If the offset and point ordering are opposite in directions then
  // the previous group is the windingGroupOffset. Otherwise if they
  // have the same direction then -windingGroupOffset is the previous
  // group.
  Vector intra = (Vector) bins[                 0][1] - (Vector) bins[0][0];
  Vector inter = (Vector) bins[windingGroupOffset][0] - (Vector) bins[0][0];

  int offset = (Dot( intra, inter ) < 0.0) ?
    windingGroupOffset : toroidalWinding-windingGroupOffset;

  // Second make sure none of the groups overlap each other.
  for( unsigned int i=0; i<toroidalWinding; i++ )
  {
    // The previous group
    unsigned int j = (i + offset) % toroidalWinding;

    // Check for a point in the previous group being between the first
    // two points in the current group.
    for( unsigned int k=0; k<nnodes; k++ ) {

      Vector v0 = (Vector) bins[i][0] - (Vector) bins[j][k];
      Vector v1 = (Vector) bins[i][1] - (Vector) bins[j][k];
      
      if( Dot( v0, v1 ) < 0.0 ) {
        nnodes = k;
//        cerr << "adjacent overlap1 " << i << "  " << j << "  " << k << endl;
      }
    }

    // Check for a point in the current group being between two points
    // in the previous group.
    for( unsigned int k=1; k<nnodes; k++ ) {

      Vector v0 = (Vector) bins[j][k  ] - (Vector) bins[i][0];
      Vector v1 = (Vector) bins[j][k-1] - (Vector) bins[i][0];
      
      if( Dot( v0, v1 ) < 0.0 ) {
        nnodes = k;
//        cerr << "adjacent overlap2 " << i << "  " << j << "  " << k << endl;
        break;
      }
    }
  }

  return nnodes;
}


unsigned int
FieldlineLib::
surfaceGroupCheck( vector< vector< Point > > &bins,
                   unsigned int i,
                   unsigned int j,
                   unsigned int nnodes )
{
  unsigned int nodes = nnodes;

  while( nodes < bins[i].size() ) {
    // Check to see if the first overlapping point is really a
    // fill-in point. This happens because the spacing between
    // toroidalWinding groups varries between groups.
    Vector v0 = (Vector) bins[j][0      ] - (Vector) bins[i][nodes];
    Vector v1 = (Vector) bins[i][nodes-1] - (Vector) bins[i][nodes];
    
    if( Dot( v0, v1 ) < 0.0 )
      nodes++;
    else
      break;
  }

  return nodes;
}


unsigned int
FieldlineLib::
removeOverlap( vector< vector < Point > > &bins,
               unsigned int &nnodes,
               unsigned int toroidalWinding,
               unsigned int poloidalWinding,
               unsigned int windingGroupOffset,
               unsigned int island )
{
  Vector globalCentroid = Vector(0,0,0);;

  for( unsigned int i=0; i<toroidalWinding; i++ )
    for( unsigned int j=0; j<nnodes; j++ )
      globalCentroid += (Vector) bins[i][j];
  
  globalCentroid /= (toroidalWinding*nnodes);
    
  if( island && nnodes == 1 )
  {
  }
  else if( island )
  {
    // Loop through each island.
    for( unsigned int i=0; i<toroidalWinding; i++ )
    {
      unsigned int nodes = 0;
      bool completeIsland = false;

      // If just a single island search for an overlap immediately.
      if( toroidalWinding == 1 )
      {
        unsigned int i = 0;

        // See if the first point overlaps another section.
        for( unsigned int  j=nnodes/2; j<bins[i].size(); j++ )
        {
          if( Dot( (Vector) bins[i][j  ] - (Vector) bins[i][0],
                   (Vector) bins[i][j-1] - (Vector) bins[i][0] ) < 0.0 )
          {
            nodes = j;
        
            completeIsland = true;
            break;
          }
        }

        // See if a point overlaps the first section.
        if( nodes == 0 )
        {
          for( unsigned int j=nnodes/2; j<bins[i].size(); j++ )
          {
            if( Dot( (Vector) bins[i][0] - (Vector) bins[i][j],
                     (Vector) bins[i][1] - (Vector) bins[i][j] ) < 0.0 )
            {
              nodes = j;

              completeIsland = true;
              break;
            }
          }
        }
      }

      // Toroidal winding is greater than 1 as such get detailed data
      // for the removal of points so that each island has the same of
      // resulting nodes.
      else
      {
        /*
        unsigned int startIndex;
        unsigned int middleIndex;
        unsigned int stopIndex;
        
        vector< Point > points;
        points.resize( bins[i].size() );

        for( unsigned int j=0; j<bins[i].size(); j++ )
          points[j] = bins[i][j];
        
        if( islandProperties( points, globalCentroid,
                              startIndex, middleIndex, stopIndex, nodes ) == 3 )
        */
        completeIsland = true;

        nodes = nnodes;
      }

      if( nodes == 0 )
      {
        if( verboseFlag )
          cerr << "removeOverlap - "
               << "Island properties returned ZERO NODES for island "
               << i << endl;

        nodes = bins[i].size();
      }

      // No more than one point should be added.
      if( nodes > nnodes+1 )
      {
        if( verboseFlag )
          cerr << "removeOverlap - Island " << i
               << " nnodes mismatch " << nnodes << "  " << nodes << endl;
      }

      // Erase all of the overlapping points.
      bins[i].erase( bins[i].begin()+nnodes, bins[i].end() );
      
      // Close the island if it is complete
      if( completeIsland )
        bins[i].push_back( bins[i][0] );
    }

  }

  // Surface
  else // if( !islands )
  {
    // This gives the minimal number of nodes for each group.
    surfaceOverlapCheck( bins, toroidalWinding, windingGroupOffset, nnodes );
    
    if( nnodes == 0 )
    {
      if( verboseFlag )
          cerr << "removeOverlap - "
               << "Surface properties returned ZERO NODES for surface "
               << endl;

      nnodes = bins[0].size();

      for( unsigned int i=1; i<toroidalWinding; i++ )
      {
        if( nnodes > bins[i].size())
          nnodes = bins[i].size();
      }
    }

    // If the offset and point ordering are opposite in directions
    // then the next group is the -windingGroupOffset. Otherwise if
    // they have the same direction then windingGroupOffset is the
    // next group.
    Vector intra = (Vector) bins[                 0][1] - (Vector) bins[0][0];
    Vector inter = (Vector) bins[windingGroupOffset][0] - (Vector) bins[0][0];

    int offset = (Dot( intra, inter ) < 0.0) ?
      toroidalWinding-windingGroupOffset : windingGroupOffset;

    for( unsigned int i=0; i<toroidalWinding; i++ )
    {
      // The next group
      unsigned int j = (i + offset) % toroidalWinding;

      // Add back in any nodes that may not overlap.
      unsigned int nodes = surfaceGroupCheck( bins, i, j, nnodes );

      // No more than one point should be added.
      if( nodes > nnodes+1 )
      {
        if( verboseFlag )
          cerr << "removeOverlap - Surface " << i
             << " nnodes mismatch " << nnodes << "  " << nodes << endl;
      }

      // Erase all of the overlapping points.
      bins[i].erase( bins[i].begin()+nodes, bins[i].end() );
    }
  }

  return nnodes;
}


unsigned int
FieldlineLib::
smoothCurve( vector< vector < Point > > &bins,
             unsigned int &nnodes,
             unsigned int toroidalWinding,
             unsigned int poloidalWinding,
             unsigned int offset,
             unsigned int island )
{
  Vector globalCentroid = Vector(0,0,0);;

  for( unsigned int i=0; i<toroidalWinding; i++ )
    for( unsigned int j=0; j<nnodes; j++ )
      globalCentroid += (Vector) bins[i][j];
  
  globalCentroid /= (toroidalWinding*nnodes);

  unsigned int add = 2;

  if( island ) {

    for( unsigned int i=0; i<toroidalWinding; i++ ) {
//      for( unsigned int s=0; s<add; s++ )
      {
        vector< pair< Point, unsigned int > > newPts;

        newPts.resize( add*nnodes );

        for( unsigned int j=0; j<add*nnodes; j++ )
          newPts[j] = pair< Point, unsigned int > (Point(0,0,0), 0 );
        
        for( unsigned int j=1; j<nnodes-1; j++ ) {

          unsigned int j_1 = (j-1+nnodes) % nnodes;
          unsigned int j1  = (j+1+nnodes) % nnodes;

          Vector v0 = (Vector) bins[i][j1] - (Vector) bins[i][j  ];
          Vector v1 = (Vector) bins[i][j ] - (Vector) bins[i][j_1];

          if( verboseFlag )
            cerr << i << " smooth " << j_1 << " "  << j << " "  << j1 << "  "
                 << ( v0.length() > v1.length() ?
                      v0.length() / v1.length() :
                      v1.length() / v0.length() ) << endl;

          if( Dot( v0, v1 ) > 0 &&
              ( v0.length() > v1.length() ?
                v0.length() / v1.length() :
                v1.length() / v0.length() ) < 10.0 ) {

            Vector center = (Vector) circle( bins[i][j_1],
                                             bins[i][j  ],
                                             bins[i][j1 ] );

            double rad = ((Vector) bins[i][j] - center).length();


            for( unsigned int s=0; s<add; s++ ) {
              Vector midPt = (Vector) bins[i][j_1] +
                (double) (add-s) / (double) (add+1) *
                ((Vector) bins[i][j] - (Vector) bins[i][j_1] );
                

              Vector midVec = midPt - center;

              midVec.normalize();

              newPts[add*j+s].first += center + midVec * rad;
              newPts[add*j+s].second += 1;

              midPt = (Vector) bins[i][j] +
                (double) (add-s) / (double) (add+1) *
                ((Vector) bins[i][j1] - (Vector) bins[i][j] );

              midVec = midPt - center;

              midVec.normalize();

              newPts[add*j1+s].first += center + midVec * rad;
              newPts[add*j1+s].second += 1;
            }
          }
        }

        for( unsigned int j=nnodes-1; j>0; j-- ) {

          for( unsigned int s=0; s<add; s++ ) {

            unsigned int k = add * j + s;

            if( newPts[k].second > 0 ) {
              
              newPts[k].first /= newPts[k].second;
              
//            cerr << i << " insert " << j << "  " << newPts[k] << endl;
              
              bins[i].insert( bins[i].begin()+j, newPts[k].first );
            }
          }
        }

        for( unsigned int s=0; s<add; s++ ) {

          unsigned int k = add - 1 - s;

          if( newPts[k].second > 0 ) {
              
            newPts[k].first /= newPts[k].second;
              
//            cerr << i << " insert " << 0.0<< "  " << newPts[k] << endl;
              
            bins[i].push_back( newPts[k].first );
          }
        }
      }
    }

  } else {

    for( unsigned int i=0; i<toroidalWinding; i++ ) {

      if( bins[i].size() < 2 )
        continue;

      // Index of the next toroidalWinding group
      unsigned int j = (i+offset)%toroidalWinding;

      // Insert the first point from the next toroidalWinding so the curve
      // is contiguous.
      bins[i].push_back( bins[j][0] );

      //for( unsigned int s=0; s<add; s++ )
      {
        unsigned int nodes = bins[i].size();

        vector<pair< Point, unsigned int > > newPts(add*nodes);

        for( unsigned int j=0; j<add*nodes; j++ )
          newPts[j] = pair< Point, unsigned int > (Point(0,0,0), 0 );
        
        for( unsigned int j=1; j<nodes-1; j++ ) {

          unsigned int j_1 = j - 1;
          unsigned int j1  = j + 1;

          Vector v0 = (Vector) bins[i][j1] - (Vector) bins[i][j  ];
          Vector v1 = (Vector) bins[i][j ] - (Vector) bins[i][j_1];

          //      cerr << i << " smooth " << j_1 << " "  << j << " "  << j1 << "  "
          //           << ( v0.length() > v1.length() ?
          //                v0.length() / v1.length() :
          //                v1.length() / v0.length() ) << endl;

          if( Dot( v0, v1 ) > 0 &&
              ( v0.length() > v1.length() ?
                v0.length() / v1.length() :
                v1.length() / v0.length() ) < 10.0 ) {

            Vector center = (Vector) circle( bins[i][j_1],
                                             bins[i][j  ],
                                             bins[i][j1 ] );

            double rad = ((Vector) bins[i][j] - center).length();


            for( unsigned int s=0; s<add; s++ ) {
              Vector midPt = (Vector) bins[i][j_1] +
                (double) (add-s) / (double) (add+1) *
                ((Vector) bins[i][j] - (Vector) bins[i][j_1] );
                

              Vector midVec = midPt - center;

              midVec.normalize();

              newPts[add*j+s].first += center + midVec * rad;
              newPts[add*j+s].second += 1;

              midPt = (Vector) bins[i][j] +
                (double) (add-s) / (double) (add+1) *
                ((Vector) bins[i][j1] - (Vector) bins[i][j] );

              midVec = midPt - center;

              midVec.normalize();

              newPts[add*j1+s].first += center + midVec * rad;
              newPts[add*j1+s].second += 1;
            }
          }
        }

        for( int j=nodes-1; j>=0; j-- ) {

          for( unsigned int s=0; s<add; s++ ) {

            unsigned int k = add * j + s;

            if( newPts[k].second > 0 ) {
              
              newPts[k].first /= newPts[k].second;
              
              if( verboseFlag )
                cerr << i << " insert " << j << "  " << newPts[k].first << endl;
              
              bins[i].insert( bins[i].begin()+j, newPts[k].first );
            }
          }
        }
      }

      // Remove the last point so it is possilble to see the groups.
      bins[i].erase( bins[i].end() );
    }
  }

  return toroidalWinding*(add+1)*nnodes;
}


unsigned int
FieldlineLib::
mergeOverlap( vector< vector < Point > > &bins,
              unsigned int &nnodes,
              unsigned int toroidalWinding,
              unsigned int poloidalWinding,
              unsigned int windingGroupOffset,
              unsigned int island )
{
  Vector globalCentroid = Vector(0,0,0);;

  for( unsigned int i=0; i<toroidalWinding; i++ )
    for( unsigned int j=0; j<nnodes; j++ )
      globalCentroid += (Vector) bins[i][j];
  
  globalCentroid /= (toroidalWinding*nnodes);
    
  if( island ) {

    vector < vector < Point > > tmp_bins;

    tmp_bins.resize( toroidalWinding );

    for( unsigned int i=0; i<toroidalWinding; i++ ) {
      
      unsigned int startIndex;
      unsigned int middleIndex;
      unsigned int stopIndex;
      unsigned int nodes;
        
      vector< Point > points;
      points.resize( bins[i].size() );

      for( unsigned int j=0; j<bins[i].size(); j++ )
        points[j] = bins[i][j];

      unsigned int turns =
        islandProperties( points, globalCentroid,
                          startIndex, middleIndex, stopIndex, nodes );

      // Merge only if there are overlapping points.
      if( turns == 3) {

        if( nnodes == bins[i].size() )
          continue;

        // Store the overlapping points.
        for( unsigned int j=nnodes; j<bins[i].size(); j++ )
          tmp_bins[i].push_back( bins[i][j] );

        if( verboseFlag )
          cerr << i << " stored extra points " << tmp_bins[i].size() << endl;

        // Erase all of the overlapping points.
        bins[i].erase( bins[i].begin()+nnodes, bins[i].end() );
          
        // Insert the first point so the curve is contiguous.
        bins[i].insert( bins[i].begin()+nnodes, bins[i][0] );

        unsigned int index_prediction = 1;
        unsigned int prediction_true  = 0;
        unsigned int prediction_false = 0;

        unsigned int modulo = bins[i].size() - 1;
 
        // Insert the remaining points.
        for( unsigned int j=0; j<tmp_bins[i].size(); j++ ) {

          Vector v0 = (Vector) bins[i][0] -
            (Vector) tmp_bins[i][j];

          double angle = 0;
          double length = 99999;
          unsigned int angleIndex = 0;
          unsigned int lengthIndex = 0;

          for( unsigned int k=1; k<bins[i].size(); k++ ) {

            Vector v1 = (Vector) bins[i][k] -
              (Vector) tmp_bins[i][j];

            double ang = acos( Dot(v0, v1) / (v0.length() * v1.length()) );

            if( angle < ang ) {
              angle = ang;
              angleIndex = k;
            }

            if( length < v1.length() ) {
              length = v1.length();
              lengthIndex = k;
            }

            // Go on.
            v0 = v1;
          }

          // Insert it between the other two.
          if( angle > M_PI / 3.0 )
            bins[i].insert( bins[i].begin()+angleIndex, tmp_bins[i][j] );

          if( verboseFlag )
            cerr << i << "  " << modulo << "  " << j + nnodes
               << "  Prediction " << index_prediction
               << " actual " << angleIndex << "  "
               << (angleIndex == index_prediction) << endl;

          // Check to see if the prediction and the actual index are
          // the same.
          if( angleIndex == index_prediction )
            prediction_true++;
          else // if( angleIndex != index_prediction )
            prediction_false++;

          // Predict where the next insertion will take place.
          if( (j+1) % modulo == 0 )
            index_prediction = 1 + (j+1) / modulo;
          else
            index_prediction = angleIndex + (j+1) / modulo + 2;
        }

        if( verboseFlag )
          cerr << "ToroidalWinding " << i << " inserted "
               << prediction_true+prediction_false << " nodes "
               << " True " << prediction_true
               << " False " << prediction_false << endl;

        // If more of the predictions are incorrect than correct
        // insert based on the predictions.
        if( 0 && prediction_true < prediction_false ) {

          if( verboseFlag )
            cerr << "ToroidalWinding " << i << " bad predicted insertion ";

          unsigned int cc = 0;

          for( unsigned int j=0; j<tmp_bins[i].size(); j++ ) {

            vector< Point >::iterator inList =
              find( bins[i].begin(), bins[i].end(), tmp_bins[i][j] );
              
            if( inList != bins[i].end() ) {
              bins[i].erase( inList );

              cc++;
            }
          }

          if( verboseFlag )
            cerr << "removed " << cc << " points" << endl;

          unsigned int index = 1;
            
          for( unsigned int j=0; j<tmp_bins[i].size(); j++ ) {
            
            // Insert it between the other two.
            bins[i].insert( bins[i].begin()+index, tmp_bins[i][j] );

            if( verboseFlag )
              cerr << i << "  " << modulo << "  " << j + nnodes
                   << " actual " << index << endl;

            // Predict where the next insertion will take place.
            if( (j+1) % modulo == 0 )
              index = 1 + (j+1) / modulo;
            else
              index += (j+1) / modulo + 2;
          }
        }

        unsigned int start0  = 0;
        unsigned int end0    = 0;
        unsigned int start1  = 0;
        unsigned int end1    = 0;

        if( prediction_true > prediction_false ) {
          // See if any of the segments cross.
          for( unsigned int j=0; 0 && j<bins[i].size()-1; j++ ) {
              
            Point l0_p0 = bins[i][j];
            Point l0_p1 = bins[i][j+1];
              
            for( unsigned int k=j+2; k<bins[i].size()-1; k++ ) {
                
              Point l1_p0 = bins[i][k];
              Point l1_p1 = bins[i][k+1];
                
              if( intersect( l0_p0, l0_p1, l1_p0, l1_p1 ) == 1 ) {
                if( start0 == 0 ) {
                  start0  = j + 1;
                  end1    = k + 1;
                } else {
                  end0   = j + 1;
                  start1 = k + 1;

                  if( verboseFlag )
                    cerr << " merge self intersection " 
                         << start0 << "  " << end0 << "  "
                         << start1 << "  " << end1 << endl;

                  if( 0 ) {
                    vector < Point > tmp_bins[2];

                    for( unsigned int l=start0; l<end0; l++ )
                      tmp_bins[0].push_back( bins[i][l] );

                    for( unsigned int l=start1; l<end1; l++ )
                      tmp_bins[1].push_back( bins[i][l] );

                    bins[i].erase( bins[i].begin()+start1,
                                   bins[i].begin()+end1 );

                    bins[i].erase( bins[i].begin()+start0,
                                   bins[i].begin()+end0 );

                    for( unsigned int l=0; l<tmp_bins[1].size(); l++ )
                      bins[i].insert( bins[i].begin()+start0,
                                      tmp_bins[1][l] );

                    for( unsigned int l=0; l<tmp_bins[0].size(); l++ )
                      bins[i].insert( bins[i].begin() + start1 -
                                      tmp_bins[0].size() +
                                      tmp_bins[1].size(),
                                      tmp_bins[0][l] );
                  }

                  start0 = 0;
                }
              }
            }
          }
        }
      }
    }
  } else {

    vector<vector < Point > > tmp_bins(toroidalWinding);

    // This gives the minimal number of nodes for each group.
    surfaceOverlapCheck( bins, toroidalWinding, windingGroupOffset, nnodes );

    if( nnodes == 0 ) {

      for( unsigned int i=1; i<toroidalWinding; i++ ) {
        if( nnodes > bins[i].size())
          nnodes = bins[i].size();
      }
    }

    // If the offset and point ordering are opposite in directions
    // then the next group is the -windingGroupOffset. Otherwise if
    // they have the same direction then windingGroupOffset is the
    // next group.
    Vector intra = (Vector) bins[                 0][1] - (Vector) bins[0][0];
    Vector inter = (Vector) bins[windingGroupOffset][0] - (Vector) bins[0][0];

    int offset = (Dot( intra, inter ) < 0.0) ?
      toroidalWinding-windingGroupOffset : windingGroupOffset;

    for( unsigned int i=0; i<toroidalWinding; i++ )
    {
      // The next group
      unsigned int j = (i + offset) % toroidalWinding;

      // Add back in any nodes that may not overlap.
      unsigned int nodes = surfaceGroupCheck( bins, i, j, nnodes );

      // No more than one point should added.
      if( nodes > nnodes+1 )
        if( verboseFlag )
          cerr << "Surface fill " << i
               << " nnodes mismatch " << nodes << endl;

      // Store the overlapping points.
      for( unsigned int j=nodes; j<bins[i].size(); j++ )
        tmp_bins[i].push_back( bins[i][j] );

      if( verboseFlag )
        cerr << i << " stored extra points " << tmp_bins[i].size() << endl;

      // Erase all of the overlapping points.
      bins[i].erase( bins[i].begin()+nodes, bins[i].end() );

      // Insert the first point from the next toroidalWinding so the curve
      // is contiguous.
      bins[i].push_back( bins[(i+offset)%toroidalWinding][0] );
    }


    for( unsigned int i=0; i<toroidalWinding; i++ )
    {
      unsigned int toroidalWinding_prediction = (i+offset)%toroidalWinding;
      unsigned int index_prediction = 1;
      unsigned int prediction_true  = 0;
      unsigned int prediction_false = 0;

      for( unsigned int i0=0; i0<tmp_bins[i].size(); i0++ ) {

        double angle = 0;
        unsigned int index_wd = 0;
        unsigned int index_pt = 0;

        for( unsigned int j=0; j<toroidalWinding; j++ ) {

          Vector v0 = (Vector) bins[j][0] -
            (Vector) tmp_bins[i][i0];

          for( unsigned int j0=1; j0<bins[j].size(); j0++ ) {
            Vector v1 = (Vector) bins[j][j0] -
              (Vector) tmp_bins[i][i0];
        
            double ang = acos( Dot(v0, v1) / (v0.length() * v1.length()) );

            if( angle < ang ) {

              angle = ang;
              index_wd = j;
              index_pt = j0;
            }

            // Go on.
            v0 = v1;
          }
        }

        // Insert it between the other two.
        bins[index_wd].insert( bins[index_wd].begin()+index_pt,
                               tmp_bins[i][i0] );

        if( verboseFlag )
          cerr << "ToroidalWinding prediction " << toroidalWinding_prediction
               << " actual " << index_wd
               << "  Index prediction  " << index_prediction
               << " actual " << index_pt << "  "
               << (index_wd == toroidalWinding_prediction &&
                 index_pt == index_prediction) << endl;

        // Check to see if the prediction of where the point was inserted
        // is correct;
        if( index_wd == toroidalWinding_prediction && index_pt == index_prediction )
          prediction_true++;
        else 
          prediction_false++;

        // Prediction of where the next insertion will take place.
        index_prediction = index_pt + 2;
        toroidalWinding_prediction = index_wd;
      }

      if( verboseFlag )
        cerr << "ToroidalWinding " << i << " inserted "
             << prediction_true+prediction_false << " nodes "
             << " True " << prediction_true
             << " False " << prediction_false << endl;
    }

    // Remove the last point so it is possilble to see the groups.
    for( unsigned int i=0; i<toroidalWinding; i++ )
      bins[i].erase( bins[i].end() );
  }

  // Update the approximate node count.
  nnodes = 9999;

  for( unsigned int i=0; i<toroidalWinding; i++ )
    if( nnodes > bins[i].size() )
      nnodes = bins[i].size();

  return nnodes;
}

// ****************************************************************************
//  Method: FieldlineLib::findIslandCenters
//
//  Purpose: Finds the geometric centers of an island chain.
//
//  Arguments:
//
//  Returns:      void
//
//  Programmer: Allen Sanderson
//  Creation:   Wed Feb 25 09:52:11 EST 2009
//
//  Modifications:
//
// ****************************************************************************

void
FieldlineLib::findIslandCenters( vector < Point > &puncturePts,
                                 unsigned int toroidalWinding,
                                 unsigned int nnodes,
                                 vector< Point > &centers )
{
#ifdef STRAIGHTLINE_SKELETON

  // Loop through each toroidial group
  for( unsigned int i=0; i<toroidalWinding; ++i ) 
  {
    if( verboseFlag )
      cerr << "Island " << i << "  ";

    // temp storage incase the order needs to be reversed.
    vector< Point > tmp_points;

    bool selfIntersect = false;

    // Loop through each point in toroidial group
    for( unsigned int j=i, jc=0;
         j<puncturePts.size() && jc<nnodes;
         j+=toroidalWinding, ++jc )
    {
      tmp_points.push_back( puncturePts[j] );
    }

    // Points for the convex hull check
    for( unsigned int j=0; j<tmp_points.size()-1; ++j )
    {
      // Check for self intersections
      for( unsigned int k=j+2; k<tmp_points.size()-1; ++k )
      {
        int result;

        if( result = intersect( tmp_points[j], tmp_points[j+1],
                                tmp_points[k], tmp_points[k+1] ) )
        {
          if( verboseFlag )
            cerr << " self intersects  " << j << "  " << k << endl;
          
          // Self intersection found so skip this island.
          selfIntersect = true;
          break;
        }
      }

      if( selfIntersect )
        break;
    }
 
    // Self intersection found so skip this island as the skeleton
    // will not work.
    if( selfIntersect )
      continue;

    // Get the convex hull and the direction.
    int direction = 0;
    hullCheck( tmp_points, direction );
      
    // Store the points a 2D vector.
    Skeleton::PointVector pointVec;

    for( unsigned int j=0; j<tmp_points.size()-1; j+=3 )
      pointVec.push_back( Skeleton::Point( tmp_points[j].x,
                                           tmp_points[j].z ) );
      
    // If the points are clockwise reverse them as the skeleton needs
    // the points to be in a counter clockwise direction.
    if( direction == 1 )
      reverse( pointVec.begin(), pointVec.end() );

    if( verboseFlag )
      cerr << " Skeleton check ";
      
    Skeleton::Skeleton s (Skeleton::makeSkeleton (pointVec));
      
    // Delete all of the hull points.
    list<Skeleton::SkeletonLine>::iterator SL = s.begin();
    list<Skeleton::SkeletonLine>::iterator deleteSL;
      
    // Remove all of the points on the boundary while getting
    // the cord length of the remains interior segments.
      
    double cordLength = 0;
    
    map< int, int > indexCount;
    map< int, int >::iterator ic;

    if( verboseFlag )
      cerr << " cordLength ";

    while ( SL != s.end() )
    {
      if( (*SL).lower.vertex->ID < pointVec.size() ||
          (*SL).higher.vertex->ID < pointVec.size() ) 
      {
        // Boundary point so delete it.
        deleteSL = SL;
        ++SL;
        s.erase( deleteSL );
      }
      else
      {
        // Running cord length.
        cordLength += sqrt( ((*SL).higher.vertex->point.x-
                             (*SL).lower.vertex->point.x) *
                            ((*SL).higher.vertex->point.x-
                             (*SL).lower.vertex->point.x) +
                            
                            ((*SL).higher.vertex->point.y -
                             (*SL).lower.vertex->point.y) * 
                            ((*SL).higher.vertex->point.y -
                             (*SL).lower.vertex->point.y) );
        
        // Count the number of times each end point is used
        
        // Lower end point
        int index = (*SL).lower.vertex->ID;
        
        ic = indexCount.find( index );
        
        if( ic == indexCount.end() )
          indexCount.insert( pair<int, int>( index,1) );
        else (*ic).second++;
        
        // Higher end point
        index = (*SL).higher.vertex->ID;
        
        ic = indexCount.find( index );
        
        if( ic == indexCount.end() )
          indexCount.insert( pair<int, int>( index,1) );
        else (*ic).second++;
        
        ++SL;
      }
    }                   
        

    int startIndex, endIndex;
    int cc = 0;
    ic = indexCount.begin();
    
    while( ic != indexCount.end() )
    {
      // Find the end points that are used but once. 
      // There should only be two.
      if( (*ic).second == 1 )
      {
        if( cc == 1 )
          startIndex = (*ic).first;
        else if( cc == 0 )
          endIndex = (*ic).first;
        
        ++cc;
      }
      
      if( (*ic).second > 2 )
      {
        if( verboseFlag )
          cerr << "double segment ??? " << (*ic).first << endl;
        
        SL = s.begin();
        
        while( SL != s.end() &&
               (*SL).lower.vertex->ID != (*ic).first &&
               (*SL).higher.vertex->ID != (*ic).first )
          SL++;
        
        // Have an index so process
        if( SL != s.end() )
        {
          // Remove double segments;
          if( (*SL).lower.vertex->ID == (*ic).first &&
              (*indexCount.find( (*SL).higher.vertex->ID )).second > 2 )
          { 
            if( verboseFlag )
              cerr << "removing double segment ??? " << (*ic).first << "  "
                   << (*indexCount.find( (*SL).higher.vertex->ID )).first
                   << endl;
            
            (*ic).second--;
            (*indexCount.find( (*SL).higher.vertex->ID )).second--;
            
            s.erase( SL );
          }
          
          // Remove double segments;
          if( (*SL).higher.vertex->ID == (*ic).first &&
              
              (*indexCount.find( (*SL).lower.vertex->ID )).second > 2 )
          {
            if( verboseFlag )
              cerr << "removing double segment ??? " << (*ic).first << "  "
                   << (*indexCount.find( (*SL).lower.vertex->ID )).first
                   << endl;
            
            
            (*ic).second--;
            (*indexCount.find( (*SL).lower.vertex->ID )).second--;
            
            s.erase( SL );
          }
        }                    
      }
      
      ++ic;
    }
    
    if( cc < 2 )
    {
      if( verboseFlag )
        cerr << "Not enough start points " << cc << endl;
      continue;
    }
    
    else if( cc > 2 )
    {
      if( verboseFlag )
        cerr << "Too many start points " << cc << endl;
      continue;
    }

    if( verboseFlag )
      cerr << "Island " << i << " New skeleton "
           << "start index " << startIndex
           << "  end index " << endIndex
           << endl;
        
    double currentCord = 0;

    int nextIndex = startIndex;

    unsigned int nlines = 0;

    // Loop through all of the lines which are described
    // as set of paired points.
    while ( s.begin() != s.end() )
    {
      // Index of the first point.
      SL = s.begin();

      while( SL != s.end() &&
             (*SL).lower.vertex->ID != nextIndex &&
             (*SL).higher.vertex->ID != nextIndex )
        SL++;
                    
      // Have an index so process
      if( SL != s.end() )
      {
        //          cerr << "Island " << j
        //               << " Line segment " << nlines++
        //               << " index " << nextIndex;

        int lastIndex = nextIndex;

        // Index of the leading point.
        if( (*SL).lower.vertex->ID == nextIndex )
          nextIndex = (*SL).higher.vertex->ID;
        else // if( (*SL).higher.vertex->ID == startIndex )
          nextIndex = (*SL).lower.vertex->ID;
            
        //          cerr << " index " << nextIndex
        //               << endl;

        double localCord = sqrt( ((*SL).higher.vertex->point.x-
                                  (*SL).lower.vertex->point.x) *
                                 ((*SL).higher.vertex->point.x-
                                  (*SL).lower.vertex->point.x) +
                                     
                                 ((*SL).higher.vertex->point.y -
                                  (*SL).lower.vertex->point.y) * 
                                 ((*SL).higher.vertex->point.y -
                                  (*SL).lower.vertex->point.y) );

        // Check to see if the segment spans the mid cord.
        if( currentCord < cordLength/2 &&
            cordLength/2.0 <= currentCord + localCord )
        {
          double t = (cordLength/2.0 - currentCord) / localCord;

          Point center;

          // Already updated the nextIndex so use the lastIndex
          // for figuring out the closest to the cordlength center.
          if( (*SL).lower.vertex->ID == lastIndex )
          {
            center.x = (*SL).lower.vertex->point.x + t *
              ((*SL).higher.vertex->point.x-(*SL).lower.vertex->point.x);
            center.y = 0;
            center.z = (*SL).lower.vertex->point.y + t *
              ((*SL).higher.vertex->point.y-(*SL).lower.vertex->point.y);
          }

          else //if( (*SL).higher.vertex->ID == lastIndex )
          {
            center.x = (*SL).higher.vertex->point.x + t *
              ((*SL).lower.vertex->point.x-(*SL).higher.vertex->point.x);
            center.y = 0;
            center.z = (*SL).higher.vertex->point.y + t *
              ((*SL).lower.vertex->point.y-(*SL).higher.vertex->point.y);
          }
          
          cerr << "O Point " << center << endl;
          centers.push_back( center );
        }
        
        // Update the running cord length so that the mid cord is
        // not calculated again.
        currentCord += localCord;
        
        // Remove this point from the list so that it is
        // not touched again.
        deleteSL = SL;
        ++SL;
        s.erase( deleteSL );
      }
      else
      {
        if( nextIndex != endIndex )
        {
          if( verboseFlag )
            cerr << "Did not find end index  "
                 << nextIndex << "  " <<  endIndex
                 << endl;
          
          break;
        }
      }
    }
  }
#endif
}



//===================================================================
// Adapted from Tolga Birdal

// Compute the q values in the equation
double Otsu::Px( unsigned int init, unsigned int end,
                 vector< unsigned int > &histo)
{
  int sum = 0;

  for (unsigned int i=init; i<end; ++i)
    sum += histo[i];

  return (double) sum;
}

// Compute the mean values in the equation (mu)
double Otsu::Mx( unsigned int init, unsigned int end,
                 vector< unsigned int > &histo)
{
  int sum = 0;

  for (unsigned int i=init; i<end; ++i)
    sum += i * histo[i];
      
  return (double) sum;
}

// Find the maximum element in a vector
unsigned int Otsu::findMaxVet( vector< double > &vet, double &maxVet)
{
  maxVet = 0;
  unsigned int index = 0;
      
  for (unsigned int i=1; i<vet.size()-1; ++i)
  {
    if( maxVet < vet[i] )
    {
      maxVet = vet[i];
      index = i;
    }
  }

  return index;
}
    
// Compute the histogram
void Otsu::getHistogram( vector< pair< unsigned int, double > >& stats,
                         vector< unsigned int > &histo )
{
  unsigned int nbins = stats.size() / 2;

  if( nbins < 10 )
    nbins = stats.size();

  double min = 1.0e9, max = -1.0e9;
    
  for( unsigned int i=0; i<stats.size(); ++i )
  {
    if( min > stats[i].second )
      min = stats[i].second;
    
    if( max < stats[i].second )
      max = stats[i].second;
  }

  histo.resize( nbins );
    
  for( unsigned int i=0; i<nbins; ++i )
    histo[i] = 0;
    
  for( unsigned int i=0; i<stats.size(); ++i )
  {
    unsigned int index =
      ( ((double) nbins-1.0) * (stats[i].second-min) / (max-min) );
      
    histo[index]++;
  }
  
//   pair< unsigned int, unsigned int > zeropair[2];

//   zeropair[0] = pair< unsigned int, unsigned int >(0,0);
//   zeropair[1] = pair< unsigned int, unsigned int >(0,0);

//   unsigned int zerostart=nbins, zerostop=nbins;

//   for( unsigned int i=0; i<nbins; ++i )
//   {
//     if( histo[i] == 0 && zerostart == nbins )
//     {
//       zerostart = i;
//       zerostop  = i;
//     }
//     else if( histo[i] != 0 && zerostart != nbins )
//     {
//       zerostop = i-1;
      
//       if( zerostop-zerostart > zeropair[0].second-zeropair[0].first )
//       {
//         zeropair[1] = zeropair[0];
//         zeropair[0] = pair< unsigned int, unsigned int >(zerostart, zerostop);
//       }
//       else if( zerostop-zerostart > zeropair[1].second-zeropair[1].first )
//       {
//         zeropair[1] = pair< unsigned int, unsigned int >(zerostart, zerostop);
//       }
      
//       zerostart = nbins;
//     }

//     int diff;
    
//     if( i )
//       diff = abs((int)histo[i] - (int)histo[i-1]);
//     else
//       diff = 0;
    
//     cerr << i << "  " << histo[i] << "  " << diff << "  " << histo[i] << endl;
//   }

//   cerr << endl
//        << "zero pairs  "
//        << zeropair[0].first << "-" << zeropair[0].second << "  "
//        << zeropair[0].second-zeropair[0].first+1 << "     "
//        << zeropair[1].first << "-" << zeropair[1].second << "  "
//        << zeropair[1].second-zeropair[1].first+1 << endl;
}

// find otsu threshold
void Otsu::getOtsuThreshold2(vector< pair< unsigned int, double > >& stats,
                             double &threshold, double &maxVet )
{
  if( stats.size() == 1 )
  {
    threshold = stats[0].second;
    maxVet = 0;
    
    return;
  }
  
  else if( stats.size() == 2 )
  {
    threshold = (stats[0].second+stats[1].second)/2.0;
    maxVet = 0;
    
    return;
  }

  vector< unsigned int > histo;
  getHistogram( stats, histo );
  
  unsigned int nbins = histo.size();
  
  unsigned int index;
  maxVet = 0;
  
  // Loop through all possible t values and maximize between class variance.
  for( unsigned int i=1; i<nbins-1; ++i )
  {
    double p0 = Px(0,     i, histo);
    double p1 = Px(i, nbins, histo);
    
    double p01 = p0 * p1;
    
    if (p01 == 0) p01 = 1.0;
    
    double m0 = Mx(0,     i, histo);
    double m1 = Mx(i, nbins, histo);

    double diff = m0 * p1 - m1 * p0;
      
    double vet = diff * diff / p01;

    if( maxVet < vet )
    {
      maxVet = vet;
      index = i;
    }
  }

  // Calculate the min and max from the values to get the threshold.
  double min = 1.0e9, max = -1.0e9;
  
  for( unsigned int i=0; i<stats.size(); ++i )
  {
    if( min > stats[i].second )
      min = stats[i].second;
    
    if( max < stats[i].second )
      max = stats[i].second;
  }

  threshold = min + (double) index / ((double) nbins - 1.0) * (max-min);
}


// Find the Otsu threshold for three division
void Otsu::getOtsuThreshold3(vector< pair< unsigned int, double > >& stats,
                             double &threshold0, double &threshold1,
                             double &maxVet )
{
  vector< unsigned int > histo;
  getHistogram( stats, histo );

  unsigned int nbins = histo.size();

  unsigned int index0;
  unsigned int index1;

  maxVet = 0;

  // Loop through all possible t values and maximize between class variance
  for ( unsigned int i=1; i<nbins-2; ++i )
  {
    double p0 = Px(0, i, histo);
    double m0 = Mx(0, i, histo);

    for ( unsigned int j=i+1; j<nbins-1; ++j )
    {
      double p1 = Px(i,     j, histo);
      double p2 = Px(j, nbins, histo);

      double p01 = p0 * p1;
      double p12 = p1 * p2;

      if (p01 == 0) p01 = 1;
      if (p12 == 0) p12 = 1;

      double m1 = Mx(i,     j, histo);
      double m2 = Mx(j, nbins, histo);
        
      double diff0 = (m0 * p1 - m1 * p0);
      double vet0 = diff0 * diff0 / p01;

      double diff1 = (m1 * p2 - m2 * p1);
      double vet1 = diff1 * diff1 / p12;

      if( maxVet < vet0 + vet1 )
      {
        maxVet = vet0 + vet1;
        index0 = i;
        index1 = j;
      }
    }
  }

  // Calculate the min and max from the values to get the thresholds.
  double min = 1.0e9, max = -1.0e9;
    
  for( unsigned int i=0; i<stats.size(); ++i )
  {
    if( min > stats[i].second )
      min = stats[i].second;
      
    if( max < stats[i].second )
      max = stats[i].second;
  }

  threshold0 = min + (double) index0 / ((double) nbins-1.0) * (max-min);
  threshold1 = min + (double) index1 / ((double) nbins-1.0) * (max-min);
}



//===================================================================

//===================================================================
// Copyright 2001, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

//===================================================================
// isLeft(): tests if a point is Left|On|Right of an infinite line.
//    Input:  three points P0, P1, and P2
//    Return: >0 for P2 left of the line through P0 and P1
//            =0 for P2 on the line
//            <0 for P2 right of the line
//    See: the January 2001 Algorithm on Area of Triangles
inline float
isLeft( Point P0, Point P1, Point P2 )
{
    return (P1.x - P0.x)*(P2.z - P0.z) - (P2.x - P0.x)*(P1.z - P0.z);
}

//===================================================================
// ptcmp(): 
//     Input:  two points to compare
//
//     Return: 1 if point 0 is "greater than" point 1
//            -1 if point 0 is "less than" point 1
//             0 if point 0 is "equal to" point 1
inline int ptcmp( const void* v0, const void* v1 )
{
  Point *p0 = (Point *) v0;
  Point *p1 = (Point *) v1;

  if( p0->x > p1->x ) return 1;
  else if( p0->x < p1->x ) return -1;
  else if( p0->y > p1->y ) return 1;
  else if( p0->y < p1->y ) return -1;
  else return 0;
}

//===================================================================
// chainHull_2D(): Andrew's monotone chain 2D convex hull algorithm
//     Input:  pts = an array of 2D points 
//
//     Output: hullPts = an array of the convex hull vertices
//                       in a clockwise order
//     Return: the number of points in hullPts
int chainHull_2D( vector< pair< Point, unsigned int > > &pts,
                  vector< pair< Point, unsigned int > > &hullPts,
                  int direction  )
{
    //  Presorted by increasing x- and y-coordinates
    qsort( &(pts[0]), pts.size(),
           sizeof( pair< Point, unsigned int > ), ptcmp );

    int n = pts.size();

    hullPts.resize( n+1 );

    // the output array hullPts[] will be used as the stack
    int    bot=0, top=(-1);  // indices for bottom and top of the stack
    int    i;                // array scan index

    // Get the indices of points with min x-coord and min|max y-coord
    int minmin = 0, minmax;
    float xmin = pts[0].first.x;
    for (i=1; i<n; i++)
        if (pts[i].first.x != xmin) break;
    minmax = i-1;
    if (minmax == n-1) {       // degenerate case: all x-coords == xmin
        hullPts[++top] = pts[minmin];
        if (pts[minmax].first.z != pts[minmin].first.z) // a nontrivial segment
            hullPts[++top] = pts[minmax];
        hullPts[++top] = pts[minmin];           // add polygon endpoint
        return top+1;
    }

    // Get the indices of points with max x-coord and min|max y-coord
    int maxmin, maxmax = n-1;
    float xmax = pts[n-1].first.x;
    for (i=n-2; i>=0; i--)
        if (pts[i].first.x != xmax) break;
    maxmin = i+1;

    // Compute the lower hull on the stack H
    hullPts[++top] = pts[minmin];      // push minmin point onto stack
    i = minmax;
    while (++i <= maxmin)
    {
        // the lower line joins pts[minmin] with pts[maxmin]
        if (isLeft( pts[minmin].first,
                    pts[maxmin].first, pts[i].first) >= 0 && i < maxmin)
            continue;          // ignore pts[i] above or on the lower line

        while (top > 0)        // there are at least 2 points on the stack
        {
            // test if pts[i] is left of the line at the stack top
            if (isLeft( hullPts[top-1].first,
                        hullPts[top].first, pts[i].first) > 0)
                break;         // pts[i] is a new hull vertex
            else
                top--;         // pop top point off stack
        }
        hullPts[++top] = pts[i];       // push pts[i] onto stack
    }

    // Next, compute the upper hull on the stack H above the bottom hull
    if (maxmax != maxmin)      // if distinct xmax points
        hullPts[++top] = pts[maxmax];  // push maxmax point onto stack
    bot = top;                 // the bottom point of the upper hull stack
    i = maxmin;
    while (--i >= minmax)
    {
        // the upper line joins pts[maxmax] with pts[minmax]
        if (isLeft( pts[maxmax].first,
                    pts[minmax].first, pts[i].first) >= 0 && i > minmax)
            continue;          // ignore pts[i] below or on the upper line

        while (top > bot)    // at least 2 points on the upper stack
        {
            // test if pts[i] is left of the line at the stack top
            if (isLeft( hullPts[top-1].first,
                        hullPts[top].first, pts[i].first) > 0)
                break;         // pts[i] is a new hull vertex
            else
                top--;         // pop top point off stack
        }
        hullPts[++top] = pts[i];       // push pts[i] onto stack
    }

    if (minmax != minmin)
      hullPts[++top] = pts[minmin];  // push joining endpoint onto stack

    hullPts.resize( top+1 );

    return hullPts.size();
}

//===================================================================

}
