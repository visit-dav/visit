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
 *  Color.h: Simple RGB color model
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   June 1994
 *
 */

#if !defined(SLIVR_Color_h)
#define SLIVR_Color_h

#include <slivr/slivr_share.h>

namespace SLIVR {

class HSVColor;

class SLIVRSHARE Color {
protected:
  double r_, g_, b_;
public:
  Color();
  Color(double, double, double);
  Color(double[3]);
  Color(const Color&);
  Color& operator=(const Color&);
  Color(const HSVColor&);
  ~Color();

  Color operator*(const Color&) const;
  Color operator*(double) const;
  Color operator/(double) const;
  Color operator+(const Color&) const;
  Color operator-(const Color&) const;
  Color& operator+=(const Color&);

  inline int operator==(const Color& c) const {
    return ((r_==c.r_)&&(g_==c.g_)&&(b_==c.b_));
  }

  inline int operator!=(const Color& c) const {
    return ((r_ != c.r_)||(g_!=c.g_)||(b_!=c.b_));
  }

  void get_color(float color[4]);
  inline double r() const {return r_;}
  inline double g() const {return g_;}
  inline double b() const {return b_;}

  inline void r( const float v ) { r_ = v; }
  inline void g( const float v ) { g_ = v; }
  inline void b( const float v ) { b_ = v; }

  inline double& operator[](int i) {   
    switch (i) {
    case 0:
      return r_;
    case 1:
      return g_;
    default:
      return b_;
    }
  }
  inline const double& operator[](int i) const
  {
    switch (i) {
    case 0:
      return r_;
    case 1:
      return g_;
    default:
      return b_;
    }
  }

  friend class HSVColor;
};

class Colorub { // unsigned byte color
  unsigned char data[3]; // data...
public:
  Colorub() {};
  Colorub(Color& c) {
    data[0] = (unsigned char)(c.r()*255);
    data[1] = (unsigned char)(c.g()*255);
    data[2] = (unsigned char)(c.b()*255);
  }; // converts them...

  unsigned char* ptr() { return &data[0]; }; // grab pointer

  inline unsigned char r() const { return data[0]; };
  inline unsigned char g() const { return data[1]; };
  inline unsigned char b() const { return data[2]; };

  // should be enough for now - this is less bandwidth...
};

class SLIVRSHARE HSVColor {
  double hue_;
  double sat_;
  double val_;
public:
  HSVColor();
  HSVColor(double hue, double sat, double val);
  ~HSVColor();
  HSVColor(const HSVColor&);
  HSVColor(const Color&);
  HSVColor& operator=(const HSVColor&);

  // These only affect hue.
  HSVColor operator*(double);
  HSVColor operator+(const HSVColor&);
   
  inline double& operator[](const int i) {   
    switch (i) {
    case 0:
      return hue_;
    case 1:
      return sat_;
    default:
      return val_;
    }
  }

  inline double hue() const {return hue_;}
  inline double sat() const {return sat_;}
  inline double val() const {return val_;}

  friend class Color;
};
/*********************************************************
  This structure holds a simple RGB color in char format.
*********************************************************/

class CharColor {
public:
  char red;
  char green;
  char blue;
  // char alpha;

  CharColor ();
  CharColor ( char a, char b, char c );
  CharColor ( Color& c );
  
  inline double r() const {return red;}
  inline double g() const {return green;}
  inline double b() const {return blue;}
  
  CharColor operator= ( const Color& );
  CharColor operator= ( const CharColor& );

  int operator!= ( const CharColor& ) const;

};

} // End namespace SLIVR


#endif
