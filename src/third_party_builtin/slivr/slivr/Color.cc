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
 *  Color.cc: Simple RGB color model
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   June 1994
 *
 */

#include <slivr/Color.h>
#include <slivr/Utils.h>

namespace SLIVR {

Color::Color()
  : r_(0), g_(0), b_(0)
{
}

Color::Color(double r, double g, double b)
  : r_(r), g_(g), b_(b)
{
}

Color::Color(const Color& c)
  : r_(c.r_), g_(c.g_), b_(c.b_)
{
}

Color::Color(double c[3])
  : r_(c[0]), g_(c[1]), b_(c[2])
{
}

Color& Color::operator=(const Color& c)
{
  r_=c.r_;
  g_=c.g_;
  b_=c.b_;
  return *this;
}

Color::~Color()
{
}

void Color::get_color(float color[4])
{
  color[0]=r_;
  color[1]=g_;
  color[2]=b_;
  color[3]=1.0;
}

Color Color::operator*(const Color& c) const
{
  return Color(r_*c.r_, g_*c.g_, b_*c.b_);
}

Color Color::operator*(double w) const
{
  return Color(r_*w, g_*w, b_*w);
}

Color Color::operator/(double w) const
{
  return Color(r_/w, g_/w, b_/w);
}

Color Color::operator+(const Color& c) const
{
  return Color(r_+c.r_, g_+c.g_, b_+c.b_);
}

Color Color::operator-(const Color& c) const
{
  return Color(r_-c.r_, g_-c.g_, b_-c.b_);
}

Color& Color::operator+=(const Color& c)
{
  r_+=c.r_;
  g_+=c.g_;
  b_+=c.b_;
  return *this;
}

Color::Color(const HSVColor& hsv)
{
  int hh((int)(hsv.hue_/360.0));
  double hue(hsv.hue_-hh*360.0);
   
  double h6(hue/60.0);
  int i((int)h6);
  double f(h6-i);
  double p1(hsv.val_*(1.0-hsv.sat_));
  double p2(hsv.val_*(1.0-(hsv.sat_*f)));
  double p3(hsv.val_*(1.0-(hsv.sat_*(1-f))));
  switch(i){
  case 0:
    r_=hsv.val_; g_=p3;       b_=p1;   break;
  case 1:
    r_=p2;       g_=hsv.val_; b_=p1;   break;
  case 2:
    r_=p1;       g_=hsv.val_; b_=p3;   break;
  case 3:
    r_=p1;       g_=p2;       b_=hsv.val_; break;
  case 4:
    r_=p3;       g_=p1;       b_=hsv.val_; break;
  case 5:
    r_=hsv.val_; g_=p1;       b_=p2;   break;
  default:
    r_=g_=b_=0;
  }
}

HSVColor::HSVColor()
{
}

HSVColor::HSVColor(double hue, double sat, double val)
  : hue_(hue), sat_(sat), val_(val)
{
}

HSVColor::~HSVColor()
{
}

HSVColor::HSVColor(const HSVColor& copy)
  : hue_(copy.hue_), sat_(copy.sat_), val_(copy.val_)
{
}

HSVColor::HSVColor(const Color& rgb)
{

  double max(Max(rgb.r_,rgb.g_,rgb.b_));
  double min(Min(rgb.r_,rgb.g_,rgb.b_));
  sat_ = ((max == 0.0) ? 0.0 : ((max-min)/max));
  if (sat_ != 0.0) {
    double rl((max-rgb.r_)/(max-min));
    double gl((max-rgb.g_)/(max-min));
    double bl((max-rgb.b_)/(max-min));
    if (max == rgb.r_) {
      if (min == rgb.g_) hue_ = 60.0*(5.0+bl);
      else hue_ = 60.0*(1.0-gl);
    } else if (max == rgb.g_) {
      if (min == rgb.b_) hue_ = 60.0*(1.0+rl);
      else hue_ = 60.0*(3.0-bl);
    } else {
      if (min == rgb.r_)	hue_ = 60.0*(3.0+gl);
      else hue_ = 60.0*(5.0-rl);
    }
  } else {
    hue_ = 0.0;
  }
  val_ = max;
}

HSVColor& HSVColor::operator=(const HSVColor& copy)
{
  hue_=copy.hue_; sat_=copy.sat_; val_=copy.val_;
  return *this;
}

HSVColor HSVColor::operator*(double w)
{
  return HSVColor(hue_*w,val_*w,sat_*w);
}

HSVColor HSVColor::operator+(const HSVColor& c)
{
  return HSVColor(hue_+c.hue_, sat_+c.sat_, val_+c.val_);
}

/***************************************************
 ***************************************************/

CharColor::CharColor ()
{
  red = green = blue = 0;
}

CharColor::CharColor ( char a, char b, char c )
{
  red = a;
  green = b;
  blue = c;
}

CharColor::CharColor ( Color& c )
{
  red   = (char)(c.r()*255);
  green = (char)(c.g()*255);
  blue  = (char)(c.b()*255);
}


CharColor
CharColor::operator= ( const Color& c )
{
  red = (char)(c.r()*255);
  green = (char)(c.g()*255);
  blue = (char)(c.b()*255);

  return *this;
}


CharColor
CharColor::operator= ( const CharColor& c )
{
  red = c.red;
  green = c.green;
  blue = c.blue;
  return *this;
}


int
CharColor::operator!= ( const CharColor& c ) const
{
  if ( ( red == c.r() ) && ( green == c.g() ) &&
       ( blue == c.b() ) )
    return 1;
  else
    return 0;
}

} // End namespace SLIVR




