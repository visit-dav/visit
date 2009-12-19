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

#include <slivr/ColorMap.h>
#include <iostream>
#include <algorithm>

using std::cerr;
using std::endl;

namespace SLIVR {


// Used by maker function above
ColorMap::ColorMap() : 
  rawRampAlpha_(),
  rawRampAlphaT_(),
  rawRampColor_(),
  rawRampColorT_(),
  resolution_(0),
  units_(),
  min_(-1.0),
  max_(1.0),
  is_scaled_(false)
{
}


ColorMap::ColorMap(const ColorMap& copy) : 
  rawRampAlpha_(copy.rawRampAlpha_),
  rawRampAlphaT_(copy.rawRampAlphaT_),
  rawRampColor_(copy.rawRampColor_),
  rawRampColorT_(copy.rawRampColorT_),
  resolution_(copy.resolution_),
  min_(copy.min_),
  max_(copy.max_),
  is_scaled_(copy.is_scaled_)
{
  for (unsigned int i = 0; i < 256*4; ++i) {
    rawRGBA_[i] = copy.rawRGBA_[i];
  }
}


ColorMap::ColorMap(const vector<Color>& rgb,
		   const vector<float>& rgbT,
		   const vector<float>& ialpha,
		   const vector<float>& alphaT,
		   unsigned int res) : 
  rawRampAlpha_(ialpha),
  rawRampAlphaT_(alphaT),
  rawRampColor_(rgb),
  rawRampColorT_(rgbT),
  resolution_(res),
  min_(-1.0),
  max_(1.0),
  is_scaled_(false)
{
  build_rgba_from_ramp();
}


ColorMap::ColorMap(const float *rgba) : 
  rawRampAlpha_(0),
  rawRampAlphaT_(0),
  rawRampColor_(0),
  rawRampColorT_(0),
  resolution_(0),
  min_(-1.0),
  max_(1.0),
  is_scaled_(false)
{
  for (unsigned int i = 0; i < 256*4; ++i) {
    rawRGBA_[i] = rgba[i];
  }
  
  build_ramp_from_rgba();
}


ColorMap::~ColorMap()
{
}



// This function builds the raw rgba float array from the ramped
// colormap specified in the constructor
void
ColorMap::build_rgba_from_ramp()
{
  for (unsigned int i = 0; i < resolution_; i++)
  {
    const float t = i / (resolution_-1.0);
    vector<float>::iterator loc;
    
    Color diffuse;
    loc = std::lower_bound(rawRampColorT_.begin(), rawRampColorT_.end(), t);
    if (loc != rawRampColorT_.begin())
    {
      const unsigned int index = loc - rawRampColorT_.begin();
      const double d = (t - rawRampColorT_[index-1]) /
	(rawRampColorT_[index] - rawRampColorT_[index-1]);
      diffuse = rawRampColor_[index-1] * (1.0 - d) + rawRampColor_[index] * d;
    }
    else
    {
      diffuse = rawRampColor_.front();
    }

    float alpha = 0.0;
    sort(rawRampAlphaT_.begin(), rawRampAlphaT_.end());
    loc = std::lower_bound(rawRampAlphaT_.begin(), rawRampAlphaT_.end(), t);
    if (loc != rawRampAlphaT_.begin())
    {
      const unsigned int index = loc - rawRampAlphaT_.begin();
      const double d = (t - rawRampAlphaT_[index-1]) /
	(rawRampAlphaT_[index] - rawRampAlphaT_[index-1]);
      alpha = rawRampAlpha_[index-1] * (1.0 - d) + rawRampAlpha_[index] * d;
    }
    else
    {
      alpha = rawRampAlpha_.front();
    }

    rawRGBA_[i*4+0] = diffuse.r();
    rawRGBA_[i*4+1] = diffuse.g();
    rawRGBA_[i*4+2] = diffuse.b();
    rawRGBA_[i*4+3] = alpha;
  }

  // Pad out rawRGBA_ to texture size.
  for (unsigned int i = resolution_; i < 256; i++)
  {
    rawRGBA_[i*4+0] = rawRGBA_[(resolution_-1)*4+0];
    rawRGBA_[i*4+1] = rawRGBA_[(resolution_-1)*4+1];
    rawRGBA_[i*4+2] = rawRGBA_[(resolution_-1)*4+2];
    rawRGBA_[i*4+3] = rawRGBA_[(resolution_-1)*4+3];
  }  
}



// This builds a ramp with regular intervals from the rgba
// array
void
ColorMap::build_ramp_from_rgba()
{
  const int size = 256;
  resolution_ = size;
  rawRampAlpha_.resize(size);
  rawRampAlphaT_.resize(size);  
  rawRampColor_.resize(size);
  rawRampColorT_.resize(size);

  for (int i = 0; i < size; i++)
  {
    const float t = i / (size-1.0);
    rawRampAlphaT_[i] = t;
    rawRampColorT_[i] = t;
    rawRampColor_[i] = Color(rawRGBA_[i*4], rawRGBA_[i*4+1], rawRGBA_[i*4+2]);
    rawRampAlpha_[i] = rawRGBA_[i*4+3];
  }
}

bool
ColorMap::get_color(double t, float &r, float &g, float &b, float &a) const
{
  const unsigned int i = (unsigned int)(resolution_ * t);
  //t == 1.0 will index past the end of storage.
  if (i >= resolution_) return false; 

  r = rawRGBA_[i * 4];
  g = rawRGBA_[i * 4 + 1];
  b = rawRGBA_[i * 4 + 2];
  a = rawRGBA_[i * 4 + 3];
  return true;
}

ColorMap *
ColorMap::create_pseudo_random(int mult)
{
  float rgba[256*4];
  if (mult == 0) mult = 65537; 
  unsigned int seed = 1;
  for (int i = 0; i < 256*4; ++i) {
    seed = seed * mult;
    switch (i%4) {
    case 0: /* Red   */ rgba[i] = (seed % 7)  / 6.0; break;
    case 1: /* Green */ rgba[i] = (seed % 11) / 10.0; break;
    case 2: /* Blue  */ rgba[i] = (seed % 17) / 16.0; break;
    default:
    case 3: /* Alpha */ rgba[i] = 1.0; break;
    }
  }
  rgba[3] = 0.0;

  return new ColorMap(rgba);
}


ColorMap *
ColorMap::create_rainbow(double mult)
{
  float rgba[256*4];
  double spacing = 360.0/255.0;
  double hue = 90;
  double dhue = spacing * mult;
  for (int i = 0; i < 256; ++i) {
    
    Color col(HSVColor(hue, 1.0, 1.0));
    //                       (i % 3) / 3.0 + 1.0/3.0, 
    //(i % 4) / 4.0 + 1.0/4.0 ));
    hue += dhue + 180.0;
    
    rgba[i*4+0] = col.r();
    rgba[i*4+1] = col.g();
    rgba[i*4+2] = col.b();
    rgba[i*4+3] = 1.0;
  }
  rgba[3] = 0.0;

  return new ColorMap(rgba);
}


ColorMap *
ColorMap::create_greyscale()
{
  float rgba[256*4];
  for (int c = 0; c < 256*4; ++c) {
    rgba[c] = (c % 4 == 3) ? 1.0f : (c/4) / 255.0f;
  }

  // Sets the alpha of black to be 0
  // Thus making the minimum value in the colormap transparent
  rgba[3] = 0.0;

  return new ColorMap(rgba);
}

} // End namespace SLIVR


