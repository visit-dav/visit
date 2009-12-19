/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.

   
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
 *  ColorMap.h: ColorMap definitions
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   November 1994
 *
 */

#if !defined(SLIVR_ColorMap_h)
#define SLIVR_ColorMap_h

#include <slivr/Color.h>
#include <vector>
#include <string>
#include <slivr/share.h>

namespace SLIVR {

using std::vector;
using std::string;

class SLIVRSHARE ColorMap 
{
public:
  ColorMap(const ColorMap&);
  ColorMap(const float *);
  ColorMap(const vector<Color>& rgb,
	   const vector<float>& rgbT,
	   const vector<float>& alphas,
	   const vector<float>& alphaT,
	   unsigned int resolution = 256);
  virtual ~ColorMap();

  static ColorMap *             create_pseudo_random(int seed = 0);
  static ColorMap *             create_rainbow(double mult = 1.0);
  static ColorMap *             create_greyscale();

  const vector<Color> &         get_rgbs() { return rawRampColor_; }
  const vector<float> &         get_rgbT() { return rawRampColorT_; }
  const vector<float> &         get_alphas() { return rawRampAlpha_; }
  const vector<float> &         get_alphaT() { return rawRampAlphaT_; }

  // Functions for handling the color scale of the data.
  bool                          IsScaled() { return is_scaled_; }
  void                          Scale(double newmin, double newmax)
  { min_ = newmin; max_ = newmax; is_scaled_ = true; }
  void                          ResetScale() 
  { min_ = -1.0; max_ = 1.0; is_scaled_ = false; }
  virtual double                getMin() const { return min_; }
  virtual double                getMax() const { return max_; }

  void                          set_units(const string &u) { units_ = u; }
  string                        units() { return units_; }
  //! return the rgba value at parameter t, return false if t is out of range.
  bool                          get_color(double t, float &r, float &g, 
					  float &b, float &a) const;
  const float *                 get_rgba() { return rawRGBA_; }
  unsigned int                  resolution() { return resolution_; }

protected:
  ColorMap();

  void                          build_rgba_from_ramp();
  void                          build_ramp_from_rgba();
  

  vector<float>			rawRampAlpha_;
  vector<float>			rawRampAlphaT_;
  vector<Color>			rawRampColor_;
  vector<float>			rawRampColorT_;

  unsigned int                  resolution_;
  float         		rawRGBA_[256*4];
  string			units_;

  double			min_;
  double			max_;
  bool                          is_scaled_;
};


} // End namespace SLIVR


#endif
