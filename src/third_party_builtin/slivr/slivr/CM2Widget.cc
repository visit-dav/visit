//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2008 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  
//    File   : CM2Widget.cc
//    Author : Milan Ikits
//    Date   : Mon Jul  5 18:33:29 2004


#include <slivr/CM2Widget.h>
#include <slivr/ShaderProgramARB.h>
#include <slivr/Utils.h>
#include <slivr/gldefs.h>

// #ifdef _WIN32
// #include <Core/Util/Rand.h>
// #endif

#include <iostream>
#include <sstream>

#define SLIVR_FLOAT_EPSILON 0.000001


#include <math.h>
#include <string.h>
#include <stdlib.h>


#if defined(__sgi)
#    define glActiveTexture(x) 
#    define glMultiTexCoord1f(t,x) 
#else
#  ifndef GL_TEXTURE0
#    define GL_TEXTURE0 GL_TEXTURE0_ARB
#    define GL_TEXTURE1 GL_TEXTURE1_ARB
#    define glActiveTexture(x) glActiveTextureARB(x)
#    define glMultiTexCoord1f(t,x) glMultiTexCoord1fARB(t,x)
#  endif
#endif

#ifdef _WIN32
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
// no drand48 on windows - just use normal rand
static double drand48()
{
  static bool initialized = false;
  if (!initialized) {
    // srand with system time as random seed
    timeb curtime;
    ftime(&curtime);
    long ticks = (curtime.time*1000)+curtime.millitm;
    srand((int) ticks);
    initialized = true;
  }
  return ((double) rand())/ RAND_MAX;
}
#endif

namespace SLIVR {
using namespace std;

CM2Widget::CM2Widget() : 
  name_("default"),  
  color_(1.0, 1.0, 1.0),
  alpha_(0.8),
  selected_(0),
  shadeType_(CM2_SHADE_REGULAR),
  onState_(1),
  faux_(true),
  value_range_(0.0, -1.0)
{
  // Generates a bright random color
  while (fabs(color_[0] - color_[1]) + 
         fabs(color_[0] - color_[2]) + 
         fabs(color_[1] - color_[2]) < 1.0) {
    color_[0] = 1.0 - sqrt(1.0 - drand48());
    color_[1] = 1.0 - sqrt(1.0 - drand48());
    color_[2] = 1.0 - sqrt(1.0 - drand48());
  }
}

CM2Widget::~CM2Widget()
{
}

CM2Widget::CM2Widget(const CM2Widget& copy) : 
  name_(copy.name_),
  color_(copy.color_),
  alpha_(copy.alpha_),
  selected_(copy.selected_),
  shadeType_(copy.shadeType_),
  onState_(copy.onState_),
  faux_(copy.faux_),
  last_hsv_(copy.last_hsv_),
  value_range_(copy.value_range_)
{
}

void
CM2Widget::set_value_range(range_t range) {
  if (range.first > range.second) return;
  const bool recompute = (value_range_.first > value_range_.second);
  value_range_ = range;
  if (recompute) un_normalize();
}


void
CM2Widget::draw_thick_gl_line(double x1, double y1, double x2, double y2,
                              double r, double g, double b)
{
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glLineWidth(5.0);
  glColor4d(0.0, 0.0, 0.0, 1.0);
  glBegin(GL_LINES);
  glVertex2d(x1,y1);
  glVertex2d(x2,y2);
  glEnd();

  glLineWidth(3.0);
  glColor4d(r,g,b,1.0);
  glBegin(GL_LINES);
  glVertex2d(x1,y1);
  glVertex2d(x2,y2);
  glEnd();

  glLineWidth(1.0);
  glColor4d(1.0, 1.0, 1.0, 1.0);
  glBegin(GL_LINES);
  glVertex2d(x1,y1);
  glVertex2d(x2,y2);
  glEnd();

  glDisable(GL_BLEND);
  glDisable(GL_LINE_SMOOTH);
}


void
CM2Widget::draw_thick_gl_point(double x1, double y1,
                               double r, double g, double b)
{
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_POINT_SMOOTH);

  glPointSize(7.0);
  glColor4d(0.0, 0.0, 0.0, 1.0);
  glBegin(GL_POINTS);
    glVertex2f(x1, y1);
  glEnd();

  glPointSize(5.0);
  glColor4d(r,g,b,1.0);
  glBegin(GL_POINTS);
    glVertex2f(x1, y1);
  glEnd();

  glPointSize(3.0);
  glColor4d(1.0, 1.0, 1.0, 1.0);
  glBegin(GL_POINTS);
    glVertex2f(x1, y1);
  glEnd();

  glDisable(GL_BLEND);
  glDisable(GL_POINT_SMOOTH);
}

void
CM2Widget::draw_thin_gl_line(double x1, double y1, double x2, double y2,
                              double r, double g, double b)
{
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glLineWidth(3.0);
  glColor4d(0.0, 0.0, 0.0, 1.0);
  glBegin(GL_LINES);
  glVertex2d(x1,y1);
  glVertex2d(x2,y2);
  glEnd();
  
  glLineWidth(1.0);
  glColor4d(r,g,b,1.0);
  glBegin(GL_LINES);
  glVertex2d(x1,y1);
  glVertex2d(x2,y2);
  glEnd();
  
  glDisable(GL_BLEND);
  glDisable(GL_LINE_SMOOTH);
}


void
CM2Widget::draw_thin_gl_point(double x1, double y1,
                               double r, double g, double b)
{
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glEnable(GL_POINT_SMOOTH);
  
  glPointSize(5.0);
  glColor4d(0.0, 0.0, 0.0, 1.0);
  glBegin(GL_POINTS);
  glVertex2f(x1, y1);
  glEnd();
  
  glPointSize(3.0);
  glColor4d(r,g,b,1.0);
  glBegin(GL_POINTS);
  glVertex2f(x1, y1);
  glEnd();
  
  glDisable(GL_BLEND);
  glDisable(GL_POINT_SMOOTH);
}

/****************************
 *
 * Rectangle Widget Functions
 *
 ***************************/
RectangleCM2Widget::RectangleCM2Widget() : 
  CM2Widget(),
  type_(CM2_RECTANGLE_1D), 
  left_x_(0.25), 
  left_y_(0.5),
  width_(0.25), 
  height_(0.25),
  offset_(0.25),
  last_x_(0),
  last_y_(0),
  pick_ix_(0),
  pick_iy_(0)
{
  left_x_ = drand48()*0.9;
  left_y_ = drand48()*0.9;
  width_ = Clamp(0.1+(0.9-left_x_)*drand48(), 0.1, 0.5);
  height_ = Clamp(0.1+(0.9-left_y_)*drand48(), 0.5*width_, 1.5*width_);
  offset_ = 0.25+0.5*drand48();
  name_ = "Rectangle";
}

RectangleCM2Widget::RectangleCM2Widget(CM2RectangleType type, 
				       float left_x, float left_y, 
				       float width, float height,
                                       float offset) : 
  CM2Widget(),
  type_(type), 
  left_x_(left_x), 
  left_y_(left_y), 
  width_(width), 
  height_(height),
  offset_(offset),
  last_x_(0),
  last_y_(0),
  pick_ix_(0),
  pick_iy_(0)

{
  name_ = "Rectangle";
}

RectangleCM2Widget::~RectangleCM2Widget()
{
}

RectangleCM2Widget::RectangleCM2Widget(const RectangleCM2Widget& copy) :
  CM2Widget(copy)
{
  // The public virtual inheritance, forces the blind binary copy here.
  // basically you can't call CM2Widget's copy constructor during 
  // initialization, which is either a compiler bug, or a C++ oversight.
  *this = copy;
}

void
RectangleCM2Widget::rasterize(CM2ShaderFactory& factory)
{  
  CHECK_OPENGL_ERROR();
  if (!onState_) return;
  CM2BlendType blend = CM2_BLEND_RASTER;
  CM2ShaderType type = CM2_SHADER_RECTANGLE_1D;
  if (type_ == CM2_RECTANGLE_ELLIPSOID)
    type = CM2_SHADER_RECTANGLE_ELLIPSOID;
  FragmentProgramARB* shader = factory.shader(type, shadeType_, faux_, blend);

  if (!shader) return;
  CHECK_OPENGL_ERROR();
  if(!shader->valid()) {
    shader->create();
  }
 
  normalize();
    
  GLdouble modelview[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  double panx = (modelview[12]+1.0)/2.0;
  double pany = (modelview[13]+1.0)/2.0;
  double scalex = (modelview[0])/2.0;
  double scaley = (modelview[5])/2.0;
  double left_x = left_x_*scalex+panx;
  double left_y = left_y_*scaley+pany;
  double width = width_*scalex;
  double height = height_*scaley;
    
  shader->bind();
  shader->setLocalParam(0, color_.r(), color_.g(), color_.b(), alpha_);
  shader->setLocalParam(1, left_x, left_y, width, height);

  if(offset_ < SLIVR_FLOAT_EPSILON )
    shader->setLocalParam(2, offset_, 0.0, 1.0, 0.0);
  else if((1.0-offset_) < SLIVR_FLOAT_EPSILON )
    shader->setLocalParam(2, offset_, 1.0, 0.0, 0.0);
  else
    shader->setLocalParam(2, offset_, 1/offset_, 1/(1-offset_), 0.0);
  
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  shader->setLocalParam(3, 1.0/vp[2], 1.0/vp[3], 0.0, 0.0);
  
  glBegin(GL_QUADS);
  {
    glVertex2f(left_x_, left_y_);
    glVertex2f(left_x_+width_, left_y_);
    glVertex2f(left_x_+width_, left_y_+height_);
    glVertex2f(left_x_, left_y_+height_);
  }
  glEnd();
  un_normalize();
  shader->release();
  CHECK_OPENGL_ERROR();
}


void
RectangleCM2Widget::draw()
{
  if (!onState_) {
    double r = 0.5, g = 0.5, b = 0.5;
    if (selected_) {
      r = 0.9; g = 0.8; b = 0.1;
    }
    normalize();
    draw_thin_gl_line(left_x_, left_y_, left_x_+width_, left_y_, r,g,b);
    draw_thin_gl_line(left_x_, left_y_+height_, left_x_+width_, left_y_+height_,
                       r,g,b);
    
    draw_thin_gl_line(left_x_, left_y_, left_x_, left_y_+height_, r,g,b);
    draw_thin_gl_line(left_x_+width_, left_y_, left_x_+width_, left_y_+height_, 
                       r,g,b);
    
    if (selected_) {
      r = 0.9; g = 0.6; b = 0.4;
    }
    
    draw_thin_gl_point(left_x_, left_y_,r,g,b);
    draw_thin_gl_point(left_x_+width_, left_y_,r,g,b);
    draw_thin_gl_point(left_x_, left_y_+height_,r,g,b);
    draw_thin_gl_point(left_x_+width_, left_y_+height_,r,g,b);
    draw_thin_gl_point(left_x_+offset_*width_, left_y_+height_*0.5,r,g,b);
    un_normalize();
    
    return;
  }

  double r = 0.5, g = 0.5, b = 0.5;
  if (selected_) {
    r = 0.9; g = 0.8; b = 0.1;
  }
  normalize();
  draw_thick_gl_line(left_x_, left_y_, left_x_+width_, left_y_, r,g,b);
  draw_thick_gl_line(left_x_, left_y_+height_, left_x_+width_, left_y_+height_,
                     r,g,b);

  draw_thick_gl_line(left_x_, left_y_, left_x_, left_y_+height_, r,g,b);
  draw_thick_gl_line(left_x_+width_, left_y_, left_x_+width_, left_y_+height_, 
                     r,g,b);

  if (selected_) {
    r = 0.9; g = 0.6; b = 0.4;
  }

  draw_thick_gl_point(left_x_, left_y_,r,g,b);
  draw_thick_gl_point(left_x_+width_, left_y_,r,g,b);
  draw_thick_gl_point(left_x_, left_y_+height_,r,g,b);
  draw_thick_gl_point(left_x_+width_, left_y_+height_,r,g,b);
  draw_thick_gl_point(left_x_+offset_*width_, left_y_+height_*0.5,r,g,b);
  un_normalize();
}



string
RectangleCM2Widget::tk_cursorname(int obj)
{
  switch (obj) {
  case 0: return string("left_ptr"); break;
  case 1: return string("fleur"); break;
  case 2: return string("bottom_left_corner"); break;
  case 3: return string("bottom_right_corner"); break;
  case 4: return string("top_right_corner"); break;
  case 5: return string("top_left_corner"); break;
  case 6: return string("sb_h_double_arrow"); break;
  case 7: return string("fleur"); break;
  case 8: return string("sb_h_double_arrow"); break;
  case 9: return string("sb_h_double_arrow"); break;
  case 10: return string("sb_v_double_arrow"); break;
  case 11: return string("sb_v_double_arrow"); break;
  default: break;
  }
  return string("left_ptr");
}


int
RectangleCM2Widget::pick1 (int ix, int iy, int w, int h)
{
  //todo
  double point_size_ = 5.0;
  normalize();
  last_x_ = left_x_;
  last_y_ = left_y_;
  pick_ix_ = ix;
  pick_iy_ = iy;
  int ret_val = 0;
  const double x = ix / (double)w;
  const double y = iy / (double)h;
  const double xeps = point_size_ / w * 0.5;
  const double yeps = point_size_ / h * 0.5;

  double left = fabs(x - left_x_);
  double right = fabs(x - left_x_ - width_);
  double bottom = fabs(y - left_y_);
  double top = fabs(y - left_y_ - height_);

  if (left < xeps && bottom < yeps) ret_val = 2;
  else if (right < xeps && bottom < yeps) ret_val = 3;
  else if (right < xeps && top < yeps) ret_val = 4;
  else if (left < xeps && top < yeps) ret_val = 5;
  else if (fabs(x - left_x_ - offset_ * width_) < xeps &&
           fabs(y - left_y_ - height_ * 0.5) < yeps) ret_val = 6;
  else if (left < xeps && y > left_y_ && y < (left_y_+height_)) ret_val = 8;
  else if (right < xeps && y > left_y_ && y < (left_y_+height_)) ret_val = 9;
  else if (x > left_x_ && x < (left_x_+width_) && bottom < yeps) ret_val = 10;
  else if (x > left_x_ && x < (left_x_+width_) && top < yeps) ret_val = 11;

  un_normalize();
  return ret_val;
}


int
RectangleCM2Widget::pick2 (int ix, int iy, int w, int h, int m)
{
  const double x = ix / (double)w;
  const double y = iy / (double)h;
  int ret_val = 0;
  normalize();
  if (x > Min(left_x_, left_x_ + width_) &&
      x < Max(left_x_, left_x_ + width_) &&
      y > Min(left_y_, left_y_ + height_) &&
      y < Max(left_y_, left_y_ + height_))
  {
    last_x_ = left_x_;
    last_y_ = left_y_;
    pick_ix_ = ix;
    pick_iy_ = iy;
    last_hsv_ = HSVColor(color_);
    ret_val =  m?7:1;
  }
  un_normalize();

  return ret_val;
}


void
RectangleCM2Widget::move (int ix, int iy, int w, int h)
{
  const double x = ix / (double)w;
  const double y = iy / (double)h;
  normalize();
  switch (selected_)
  {
  case 1:
    left_x_ = last_x_ + x - pick_ix_ / (double)w;
    left_y_ = last_y_ + y - pick_iy_ / (double)h;
    break;
      
  case 2:
    width_ = width_ + left_x_ - x;
    left_x_ = x;
    height_ = height_ + left_y_ - y;
    left_y_ = y;
    break;

  case 3:
    width_ = x - left_x_;
    height_ = height_ + left_y_ - y;
    left_y_ = y;
    break;

  case 4:
    width_ = x - left_x_;
    height_ = y - left_y_;
    break;

  case 5:
    width_ = width_ + left_x_ - x;
    left_x_ = x;
    height_ = y - left_y_;
    break;

  case 6:
    offset_ = Clamp((x - left_x_) / width_, 0.0, 1.0);
    break;

  case 7:
    {
      // Hue controls on x axis
      const double hdelta = x - pick_ix_ / (double)w;
      double hue = last_hsv_[0] + hdelta * 360.0 * 2.0;
      while (hue < 0.0) hue += 360.0;
      while (hue > 360.0) hue -= 360;

      // Saturation controls on y axis
      const double sdelta = y - pick_iy_ / (double)h;
      double sat = Clamp(last_hsv_[1] - sdelta * 2.0, 0.0, 1.0);

      HSVColor hsv(hue, sat, last_hsv_.val());
      color_ = Color(hsv);
    }
    break;

  case 8:
    width_ = width_ + left_x_ - x;
    left_x_ = x;
    break;

  case 9:
    width_ = x - left_x_;
    break;

  case 10:
    height_ = height_ + left_y_ - y;
    left_y_ = y;
    break;

  case 11:
    height_ = y - left_y_;
    break;
  }

  if (width_ < 0.0) {
    left_x_ += width_;
    width_ *= -1;
    switch (selected_) {
    case 2: selected_ = 3; break;
    case 3: selected_ = 2; break;
    case 4: selected_ = 5; break;
    case 5: selected_ = 4; break;
    case 8: selected_ = 9; break;
    case 9: selected_ = 8; break;
    default: break;
    }
  }

  if (height_ < 0.0) {
    left_y_ += height_;
    height_ *= -1;
    switch (selected_) {
    case 2: selected_ = 5; break;
    case 3: selected_ = 4; break;
    case 4: selected_ = 3; break;
    case 5: selected_ = 2; break;
    case 10: selected_ = 11; break;
    case 11: selected_ = 10; break;
    default: break;
    }
  }  
  un_normalize();
}


void
RectangleCM2Widget::release (int /*x*/, int /*y*/,
                             int /*w*/, int /*h*/)
{
  normalize();
  if (width_ < 0.0) {
    left_x_ += width_;
    width_ *= -1.0;
  }

  if (height_ < 0.0) {
    left_y_ += height_;
    height_ *= -1.0;
  }
  un_normalize();
  // Don't need to do anything here.
}


string
RectangleCM2Widget::tcl_pickle()
{
  normalize();
  ostringstream s;
  s << "r ";
  s << (int)type_ << " ";
  s << left_x_ << " ";
  s << left_y_ << " ";
  s << width_ << " ";
  s << height_ << " ";
  s << offset_;
  un_normalize();
  return s.str();
}

void
RectangleCM2Widget::tcl_unpickle(const string &p)
{
  istringstream s(p);
  char c;
  s >> c;
  int t;
  s >> t;
  type_ = (CM2RectangleType)t;
  s >> left_x_;
  s >> left_y_;
  s >> width_;
  s >> height_;
  s >> offset_;
  value_range_.first = 0.0;
  value_range_.second = -1.0;
}


void
RectangleCM2Widget::normalize()
{
  if (value_range_.first > value_range_.second) return;
  const float offset = -value_range_.first;
  const float scale = 1.0/(value_range_.second-value_range_.first);

  left_x_ = (left_x_ + offset) * scale;
  width_ = width_ * scale;
}


void
RectangleCM2Widget::un_normalize()
{
  if (value_range_.first > value_range_.second) return;
  const float offset = -value_range_.first;
  const float scale = 1.0/(value_range_.second-value_range_.first);
  
  left_x_ = left_x_/scale - offset;
  width_ = width_/scale;
}

/****************************
 *
 * Ellipsoid Widget Functions
 *
 ***************************/
EllipsoidCM2Widget::EllipsoidCM2Widget() : 
  CM2Widget(),
  a_(.2),
  b_(.1),
  last_x_(0),
  last_y_(0),
  pick_ix_(0),
  pick_iy_(0)
{
  // Give ellipsoid default position.
  center_x_ = .5;
  center_y_ = .25;
  rot_ = 0.0;

  name_ = "Ellipsoid";
}

EllipsoidCM2Widget::EllipsoidCM2Widget(float x, float y, float a, float b, float rot) :
  CM2Widget(),
  center_x_(x),
  center_y_(y),
  a_(a),
  b_(b),
  rot_(rot),
  last_x_(0),
  last_y_(0),
  pick_ix_(0),
  pick_iy_(0)

{
  name_ = "Ellipsoid";
}

EllipsoidCM2Widget::~EllipsoidCM2Widget()
{
}

EllipsoidCM2Widget::EllipsoidCM2Widget(const EllipsoidCM2Widget& copy) :
  CM2Widget(copy)
{
  // The public virtual inheritance, forces the blind binary copy here.
  // basically you can't call CM2Widget's copy constructor during 
  // initialization, which is either a compiler bug, or a C++ oversight.
  *this = copy;
}

void
EllipsoidCM2Widget::rasterize(CM2ShaderFactory& factory)
{  
  CHECK_OPENGL_ERROR();
  if (!onState_) return;

  CM2BlendType blend = CM2_BLEND_RASTER;
  CM2ShaderType type = CM2_SHADER_ELLIPSOID;
  FragmentProgramARB* shader = factory.shader(type, shadeType_, faux_, blend);

  if (!shader) return;
  CHECK_OPENGL_ERROR();
  if (!shader->valid()) {
    shader->create();
  }

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  GLdouble aspect = (float)viewport[2] / (float)viewport[3];

  shader->bind();

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glScaled(1.0, aspect, 1.0);

  /* fill the ellipse */
  glColor4f(color_.r(), color_.g(), color_.b(), alpha_);
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(center_x_, center_y_);
  glColor4f(0.0f, 0.0f, 0.0f, alpha_);
  for (int i = 0; i <= 360; i += 2) {
    const float DEG2RAD = 3.14159/180;
    float degInRad = i * DEG2RAD;
    float x = cos(degInRad) * a_;
    float y = sin(degInRad) * b_;
    float x_rot = x * cos(rot_) + y * sin(rot_);
    float y_rot = (-1 * x) * sin(rot_) + y * cos(rot_);
    glVertex2f(center_x_ + x_rot, 
	       center_y_ + y_rot);
  }
  glEnd();

  glPopMatrix();

  shader->release();

  CHECK_OPENGL_ERROR();
}

void
EllipsoidCM2Widget::draw()
{
  // TODO:  Draw with thin lines if not on.
  if(!onState_) return;

  double r = 0.5, g = 0.5, b = 0.5;
  if (selected_) {
    r = 0.9; g = 0.8; b = 0.1;
  }

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  GLdouble aspect = (float)viewport[2] / (float)viewport[3];

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glScaled(1.0, aspect, 1.0);

  for (int i = 0; i < 3; i++) { /* for each line */
    switch(i) {
    case 0:
      glLineWidth(5.0);
      glColor4d(0.0, 0.0, 0.0, 1.0);
      break;
    case 1:
      glLineWidth(3.0);
      glColor4d(r,g,b,1.0);
      break;
    case 2:
      glLineWidth(1.0);
      glColor4d(1.0, 1.0, 1.0, 1.0);
      break;
    }

    /* draw the ellipse */
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i++) {
      const float DEG2RAD = 3.14159/180;
      float degInRad = (float)i * DEG2RAD;
      float x = cos(degInRad) * a_;
      float y = sin(degInRad) * b_;
      float x_rot = x * cos(rot_) + y * sin(rot_);
      float y_rot = (-1 * x) * sin(rot_) + y * cos(rot_);
      glVertex2f(center_x_ + x_rot, 
		 center_y_ + y_rot);
    }
    glEnd();
  }  

  /* draw rotation control */
  {
    float x_rot = (0.04 + a_) * cos(rot_);
    float y_rot = (-1 * (0.04 + a_)) * sin(rot_);
    draw_thick_gl_line(center_x_, center_y_, 
		       center_x_ + x_rot, center_y_ + y_rot, r,g,b);
  }
  for (int i = 0; i < 3; i++) { /* for each line */
    switch(i) {
    case 0:
      glLineWidth(5.0);
      glColor4d(0.0, 0.0, 0.0, 1.0);
      break;
    case 1:
      glLineWidth(3.0);
      glColor4d(r,g,b,1.0);    
      break;
    case 2:
      glLineWidth(1.0);
      glColor4d(1.0, 1.0, 1.0, 1.0);
      break;
    }
    /* draw the circle */
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i++) {
      const float DEG2RAD = 3.14159/180;
      float degInRad = i * DEG2RAD;
      float x = cos(degInRad) * .02 + a_ + 0.04;
      float y = sin(degInRad) * .02;
      float x_rot = x * cos(rot_) + y * sin(rot_);
      float y_rot = (-1 * x) * sin(rot_) + y * cos(rot_);
      glVertex2f(center_x_ + x_rot, 
		 center_y_ + y_rot);
    }
    glEnd();
  }

  /* draw control points */
  float x_array[] = { a_, 0.0, -a_, 0.0 };
  float y_array[] = { 0.0, b_, 0.0, -b_ };
  for (int i = 0; i < 4; i++) {
    float x_rot = x_array[i] * cos(rot_) + y_array[i] * sin(rot_);
    float y_rot = (-1 * x_array[i]) * sin(rot_) + y_array[i] * cos(rot_);

    x_controls_[i] = center_x_ + x_rot;
    y_controls_[i] = center_y_ + y_rot;

    draw_thick_gl_point(center_x_ + x_rot, center_y_ + y_rot, r, g, b);
  }

  glDisable(GL_BLEND);
  glDisable(GL_LINE_SMOOTH);
  glPopMatrix();
}

string
EllipsoidCM2Widget::tk_cursorname(int obj)
{
  switch (obj) {
  case 0: return string("fleur"); break;
  case 1: return string("left_ptr"); break;
  case 2: return string("right_ptr"); break;
  case 3: return string("right_ptr"); break;
  case 4: return string("right_ptr"); break;
  case 5: return string("right_ptr"); break;
  case 6: return string("left_ptr"); break;
  default: break;
  }
  return string("left_ptr");
}


int
EllipsoidCM2Widget::pick1 (int ix, int iy, int w, int h)
{
  int return_val = 0;

  float aspect = (float)w / (float)h;

  last_x_ = center_x_;
  last_y_ = center_y_;
  last_rot_ = rot_;
  pick_ix_ = ix;
  pick_iy_ = iy;

  /* move ellipse into screen coordinates (pixels) */
  float x_radius = a_ * w;
  float y_radius = b_ * h * aspect;

  /* translate mouse point to origin for simpler math */
  float attempt_x = (float)ix - ((float)w * center_x_);
  float attempt_y = (float)iy - ((float)h * center_y_ * aspect);

  /* rotate mouse based on rot_ */
  float x_rot = attempt_x * cos(-rot_) + attempt_y * sin(-rot_);
  float y_rot = (-1 * attempt_x) * sin(-rot_) + attempt_y * cos(-rot_);
  attempt_x = x_rot;
  attempt_y = y_rot;

  float rad = fabs(atan2(attempt_y * aspect, attempt_x));

  float x = cos(rad) * x_radius;
  float y = sin(rad) * y_radius;

  /* compute distances */
  float ellipse_d = sqrt((x * x) + (y * y));
  float pick_d = sqrt((attempt_x * attempt_x) + (attempt_y * attempt_y));

  const int p_size = 8;
  if (sqrt(pow(attempt_x - x_radius, 2) + pow(attempt_y, 2)) < p_size) {
    return_val = 2;
  }
  else if (sqrt(pow(attempt_x + x_radius, 2) + pow(attempt_y, 2)) < p_size) {
    return_val = 4;
  }
  else if (sqrt(pow(attempt_x, 2) + pow(attempt_y - y_radius, 2)) < p_size) {
    return_val = 3;
  }
  else if (sqrt(pow(attempt_x, 2) + pow(attempt_y + y_radius, 2)) < p_size) {
    return_val = 5;
  } 
  else if (sqrt(pow(attempt_x - (a_ + 0.04) * w, 2) + 
		pow(attempt_y, 2)) < 10) {
    return_val = 6;
  }
  else if (pick_d < ellipse_d) { /* inside ellipse */
    return_val = 1;
  }
  else {
    return_val = 0; /* translate */
  }

  return return_val;
}


int
EllipsoidCM2Widget::pick2 (int /*ix*/, int /*iy*/, int /*w*/, int /*h*/, int /*m*/)
{
  int return_val = 0;
#if 0
  float aspect = w / h;

  last_x_ = center_x_;
  last_y_ = center_y_;
  last_rot_ = rot_;
  pick_ix_ = ix;
  pick_iy_ = iy;

  /* move ellipse into screen coordinates (pixels) */
  float x_radius = a_ * w;
  float y_radius = b_ * h * aspect;

  /* translate mouse point to origin for simpler math */
  float attempt_x = ix - (w * center_x_);
  float attempt_y = iy - (h * center_y_ * aspect);

  /* rotate mouse based on rot_ */
  float x_rot = attempt_x * cos(-rot_) + attempt_y * sin(-rot_);
  float y_rot = (-1 * attempt_x) * sin(-rot_) + attempt_y * cos(-rot_);
  attempt_x = x_rot;
  attempt_y = y_rot;

  float rad = fabs(atan2(attempt_y, attempt_x));

  float x = cos(rad) * x_radius;
  float y = sin(rad) * y_radius;

  /* compute distances */
  float ellipse_d = sqrt((x * x) + (y * y));
  float pick_d = sqrt((attempt_x * attempt_x) + (attempt_y * attempt_y));

  //  cerr << pick_d << " " << ellipse_d << endl;

  const int p_size = 8;
  if (sqrt(pow(attempt_x - x_radius, 2) + pow(attempt_y, 2)) < p_size) {
    //    cerr << "Control 2" << endl;
    return_val = 2;
  }
  else if (sqrt(pow(attempt_x + x_radius, 2) + pow(attempt_y, 2)) < p_size) {
    //    cerr << "Control 4" << endl;
    return_val = 4;
  }
  else if (sqrt(pow(attempt_x, 2) + pow(attempt_y - y_radius, 2)) < p_size) {
    //    cerr << "Control 3" << endl;
    return_val = 4;
  }
  else if (sqrt(pow(attempt_x, 2) + pow(attempt_y + y_radius, 2)) < p_size) {
    //    cerr << "Control 5" << endl;
    return_val = 5;
  }
  else if (pick_d < Min(a_, b_) * .2 + 10) {
    return_val = 6;
  }
  else if (pick_d < ellipse_d - 20) { /* inside ellipse */
    //    cerr << "Grab" << endl;
    return_val = 1;
  }
  else {
    //    cerr << "Outside" << endl;
    return_val = 0; /* translate */
  }
#endif
  return return_val;
}


void
EllipsoidCM2Widget::move (int ix, int iy, int w, int h)
{
  double aspect = w / h;
  double x, y;
  double rad_diff;
  float x_diff, y_diff;

  switch(selected_) {
  case 0:
    break;
  case 1:
    x = (double)(pick_ix_ - ix) / (double)w;
    y = (double)(pick_iy_ - iy) / (double)h;
    y /= aspect;
    center_x_ = last_x_ - x; /* translate */
    center_y_ = last_y_ - y;
    break;
  case 2:
  case 4:
    x = (double)(ix) / (double)w;
    y = (double)(iy) / (double)h;
    y /= aspect;
    a_ = sqrt(pow(center_x_ - x, 2) + pow(center_y_ - y, 2));
    break;
  case 3:
  case 5:
    x = (double)(ix) / (double)w;
    y = (double)(iy) / (double)h;
    y /= aspect;
    b_ = sqrt(pow(center_x_ - x, 2) + pow(center_y_ - y, 2));
    break;
  case 6:
    x_diff = ix - center_x_ * w;
    y_diff = iy - center_y_ * h * aspect;
    
    rad_diff = atan2(y_diff, x_diff);

    rot_ = -1.0 * rad_diff;

    if (rot_ > M_PI * 2.0)
      rot_ -= M_PI * 2.0;
    else if (rot_ < 0.0)
      rot_ += M_PI * 2.0;

    break;
  default:
    break;
  }
}


void
EllipsoidCM2Widget::release (int /*x*/, int /*y*/,
                             int /*w*/, int /*h*/)
{
  // Don't need to do anything here.
}


string
EllipsoidCM2Widget::tcl_pickle()
{
  normalize();
  ostringstream s;

  s << "e ";
  s << center_x_ << " ";
  s << center_y_ << " ";
  s << a_ << " ";
  s << b_ << " ";
  s << rot_;

  return s.str();
}

void
EllipsoidCM2Widget::tcl_unpickle(const string &p)
{
  istringstream s(p);
  char c;
  s >> c;
  s >> center_x_;
  s >> center_y_;
  s >> a_;
  s >> b_;
  s >> rot_;
}


void
EllipsoidCM2Widget::normalize()
{
}


void
EllipsoidCM2Widget::un_normalize()
{
}

/****************************
 *
 * Paraboloid Widget Functions
 *
 ***************************/
ParaboloidCM2Widget::ParaboloidCM2Widget() : 
  CM2Widget(),
  top_x_(0.5),
  top_y_(0.7),
  bottom_x_(0.5),
  bottom_y_(0.3),
  left_x_(0.3),
  left_y_(0.5),
  right_x_(0.7),
  right_y_(0.5),
  pick_ix_(0),
  pick_iy_(0)
{
  name_ = "Paraboloid";
}

ParaboloidCM2Widget::ParaboloidCM2Widget(float top_x, float top_y,
					 float bottom_x, float bottom_y,
					 float left_x, float left_y,
					 float right_x, float right_y) :
  CM2Widget(),
  top_x_(top_x),
  top_y_(top_y),
  bottom_x_(bottom_x),
  bottom_y_(bottom_y),
  left_x_(left_x),
  left_y_(left_y),
  right_x_(right_x),
  right_y_(right_y),
  pick_ix_(0),
  pick_iy_(0)
{
  name_ = "Paraboloid";
}

ParaboloidCM2Widget::~ParaboloidCM2Widget()
{
}

ParaboloidCM2Widget::ParaboloidCM2Widget(const ParaboloidCM2Widget& copy) :
  CM2Widget(copy)
{
  // The public virtual inheritance, forces the blind binary copy here.
  // basically you can't call CM2Widget's copy constructor during 
  // initialization, which is either a compiler bug, or a C++ oversight.
  *this = copy;
}

float 
poly3(float x1, float y1, float x2, float y2, float x3, float y3, float x)
{ // Langrande polynomial interpolation
  float part1 = (x - x2) * (x - x3) * y1;
  part1 /= (x1 - x2) * (x1 - x3);
  float part2 = (x - x1) * (x - x3) * y2;
  part2 /= (x2 - x1) * (x2 - x3);
  float part3 = (x - x1) * (x - x2) * y3;
  part3 /= (x3 - x1) * (x3 - x2);

  return part1 + part2 + part3;
}

#define PARABOLA_SEGMENTS 16

void
ParaboloidCM2Widget::rasterize(CM2ShaderFactory& factory)
{
  CHECK_OPENGL_ERROR();
  if (!onState_) return;

  CM2BlendType blend = CM2_BLEND_RASTER;
  CM2ShaderType type = CM2_SHADER_PARABOLOID;
  FragmentProgramARB* shader = factory.shader(type, shadeType_, faux_, blend);

  if (!shader) return;
  CHECK_OPENGL_ERROR();
  if (!shader->valid()) {
    shader->create();
  }

  shader->bind();
  shader->setLocalParam(0, color_.r(), color_.g(), color_.b(), alpha_);

  glBegin(GL_QUADS);
  float segment_width = (top_x_ - left_x_) / PARABOLA_SEGMENTS;
  // draw left parabola
  for (int i = 0; i < PARABOLA_SEGMENTS; i++) {
    float start_x = left_x_ + i * segment_width;
    float end_x = start_x + segment_width;

    // draw bottom half of curve
    glColor3f(0.0, 0.0, 0.0);
    glVertex2f(start_x, poly3(left_x_, left_y_, bottom_x_, bottom_y_,
			      right_x_, right_y_, start_x));
    glVertex2f(end_x, poly3(left_x_, left_y_, bottom_x_, bottom_y_,
			    right_x_, right_y_, end_x));
    glColor3f(1.0, 0.0, 0.0);
    glVertex2f(end_x, poly3(left_x_, left_y_, top_x_, (top_y_ + bottom_y_) / 2.0, 
			    right_x_, right_y_, end_x));
    glVertex2f(start_x, poly3(left_x_, left_y_, top_x_, (top_y_ + bottom_y_) / 2.0, 
			      right_x_, right_y_, start_x));

    // draw top half of curve
    glColor3f(1.0, 0.0, 0.0);
    glVertex2f(start_x, poly3(left_x_, left_y_, top_x_, (top_y_ + bottom_y_) / 2.0, 
			      right_x_, right_y_, start_x));
    glVertex2f(end_x, poly3(left_x_, left_y_, top_x_, (top_y_ + bottom_y_) / 2.0, 
			    right_x_, right_y_, end_x));
    glColor3f(0.0, 0.0, 0.0);
    glVertex2f(end_x, poly3(left_x_, left_y_, top_x_, top_y_, 
			    right_x_, right_y_, end_x));
    glVertex2f(start_x, poly3(left_x_, left_y_, top_x_, top_y_, 
			      right_x_, right_y_, start_x));    
  }

  segment_width = (right_x_ - top_x_) / PARABOLA_SEGMENTS;
  // draw right parabola
  for (int i = 0; i < PARABOLA_SEGMENTS; i++) {
    float start_x = top_x_ + i * segment_width;
    float end_x = start_x + segment_width;

    // draw bottom half of curve
    glColor3f(0.0, 0.0, 0.0);
    glVertex2f(start_x, poly3(left_x_, left_y_, bottom_x_, bottom_y_, 
			      right_x_, right_y_, start_x));
    glVertex2f(end_x, poly3(left_x_, left_y_, bottom_x_, bottom_y_, 
			    right_x_, right_y_, end_x));
    glColor3f(1.0, 0.0, 0.0);
    glVertex2f(end_x, poly3(left_x_, left_y_, top_x_, (top_y_ + bottom_y_) / 2.0, 
			    right_x_, right_y_, end_x));
    glVertex2f(start_x, poly3(left_x_, left_y_, top_x_, (top_y_ + bottom_y_) / 2.0, 
			      right_x_, right_y_, start_x));

    // draw top half of curve
    glColor3f(1.0, 0.0, 0.0);
    glVertex2f(start_x, poly3(left_x_, left_y_, top_x_, (top_y_ + bottom_y_) / 2.0, 
			      right_x_, right_y_, start_x));
    glVertex2f(end_x, poly3(left_x_, left_y_, top_x_, (top_y_ + bottom_y_) / 2.0, 
			    right_x_, right_y_, end_x));
    glColor3f(0.0, 0.0, 0.0);
    glVertex2f(end_x, poly3(left_x_, left_y_, top_x_, top_y_, 
			    right_x_, right_y_, end_x));
    glVertex2f(start_x, poly3(left_x_, left_y_, top_x_, top_y_, 
			      right_x_, right_y_, start_x));
  }
  glEnd();

  shader->release();

  CHECK_OPENGL_ERROR();
}

void
ParaboloidCM2Widget::draw()
{
  double red = 0.5, green = 0.5, blue = 0.5;

  if (selected_) { // switch to (... && onState_) for constant grey
    red = 0.9; green = 0.8; blue = 0.1;
  }

  float segment_width = (top_x_ - left_x_) / PARABOLA_SEGMENTS;
  // draw left parabola
  for (int i = 0; i < PARABOLA_SEGMENTS; i++) {
    float start_x = left_x_ + i * segment_width;
    float end_x = start_x + segment_width;
    if (onState_) {
      draw_thick_gl_line(start_x, 
			 poly3(left_x_, left_y_, top_x_, top_y_, 
			       right_x_, right_y_, start_x),
			 end_x,
			 poly3(left_x_, left_y_, top_x_, top_y_, 
			       right_x_, right_y_, end_x),
			 red,green,blue);		       
      draw_thick_gl_line(start_x, 
			 poly3(left_x_, left_y_, bottom_x_, bottom_y_, 
			       right_x_, right_y_, start_x),
			 end_x,
			 poly3(left_x_, left_y_, bottom_x_, bottom_y_, 
			       right_x_, right_y_, end_x),
			 red,green,blue);
    } else {
      draw_thin_gl_line(start_x, 
			 poly3(left_x_, left_y_, top_x_, top_y_, 
			       right_x_, right_y_, start_x),
			 end_x,
			 poly3(left_x_, left_y_, top_x_, top_y_, 
			       right_x_, right_y_, end_x),
			 red,green,blue);		       
      draw_thin_gl_line(start_x, 
			 poly3(left_x_, left_y_, bottom_x_, bottom_y_, 
			       right_x_, right_y_, start_x),
			 end_x,
			 poly3(left_x_, left_y_, bottom_x_, bottom_y_, 
			       right_x_, right_y_, end_x),
			 red,green,blue);
    }
  }
  segment_width = (right_x_ - top_x_) / PARABOLA_SEGMENTS;
  // draw right parabola
  for (int i = 0; i < PARABOLA_SEGMENTS; i++) {
    float start_x = top_x_ + i * segment_width;
    float end_x = start_x + segment_width;
    if (onState_) {
      draw_thick_gl_line(start_x, 
			 poly3(left_x_, left_y_, top_x_, top_y_, 
			       right_x_, right_y_, start_x),
			 end_x,
			 poly3(left_x_, left_y_, top_x_, top_y_, 
			       right_x_, right_y_, end_x),
			 red,green,blue);
      draw_thick_gl_line(start_x, 
			 poly3(left_x_, left_y_, bottom_x_, bottom_y_, 
			       right_x_, right_y_, start_x),
			 end_x,
			 poly3(left_x_, left_y_, bottom_x_, bottom_y_, 
			       right_x_, right_y_, end_x),
			 red,green,blue);
    } else {
      draw_thin_gl_line(start_x, 
			poly3(left_x_, left_y_, top_x_, top_y_, 
			      right_x_, right_y_, start_x),
			end_x,
			poly3(left_x_, left_y_, top_x_, top_y_, 
			      right_x_, right_y_, end_x),
			red,green,blue);
      draw_thin_gl_line(start_x, 
			poly3(left_x_, left_y_, bottom_x_, bottom_y_, 
			      right_x_, right_y_, start_x),
			end_x,
			poly3(left_x_, left_y_, bottom_x_, bottom_y_, 
			      right_x_, right_y_, end_x),
			red,green,blue);
    }
  }

  // draw control points
  if (onState_) {
    draw_thick_gl_point(top_x_, top_y_, red,green,blue);  
    draw_thick_gl_point(bottom_x_, bottom_y_, red,green,blue);  
    draw_thick_gl_point(left_x_, left_y_, red,green,blue);  
    draw_thick_gl_point(right_x_, right_y_, red,green,blue);  
  }
}

string
ParaboloidCM2Widget::tk_cursorname(int obj)
{
  switch (obj) {
  case 0: return string("fleur"); break;
  case 1: return string("right_ptr"); break;
  case 2: return string("right_ptr"); break;
  case 3: return string("right_ptr"); break;
  case 4: return string("right_ptr"); break;
  case 5: return string("left_ptr"); break;
  default: break;
  }
  return string("left_ptr");
}


int
ParaboloidCM2Widget::pick1 (int ix, int iy, int w, int h)
{
  int return_val = 0;

  // Get mouse coordinate in [0,1]X[0,1] coordinates.
  float x = (float)ix / w;
  float y = (float)iy / h;

  bool inside_left_paraboloid = false;
  bool inside_right_paraboloid = false;

  if (y < poly3(left_x_, left_y_, top_x_, top_y_, right_x_, right_y_, x) &&
      y > poly3(left_x_, left_y_, bottom_x_, bottom_y_, right_x_, right_y_, x))
    inside_left_paraboloid = true;

  // draw control points
  float cntl_point = 0.001f;
  if ((top_x_ - x) * (top_x_ - x) + (top_y_ - y) * (top_y_ - y) < cntl_point)
    return_val = 1; // top control point
  else if ((bottom_x_ - x) * (bottom_x_ - x) + (bottom_y_ - y) * (bottom_y_ - y) < cntl_point)
    return_val = 2; // bottom control point
  else if ((left_x_ - x) * (left_x_ - x) + (left_y_ - y) * (left_y_ - y) < cntl_point)
    return_val = 3; // left control point
  else if ((right_x_ - x) * (right_x_ - x) + (right_y_ - y) * (right_y_ - y) < cntl_point)
    return_val = 4; // right control point
  else if (inside_left_paraboloid)
    return_val = 5; // inside widget
  else if (inside_right_paraboloid)
    return_val = 5; // inside widget

  // save off mouse position
  last_x_ = ix;
  last_y_ = iy;

  return return_val;
}


int
ParaboloidCM2Widget::pick2 (int /*ix*/, int /*iy*/, int /*w*/, int /*h*/, int /*m*/)
{
  int return_val = 0;

  return return_val;
}


void
ParaboloidCM2Widget::move (int ix, int iy, int w, int h)
{
  // get mouse coordinate in [0,1]X[0,1] coordinates
  float prev_x = (float)last_x_ / w;
  float prev_y = (float)last_y_ / h;
  float x = (float)ix / w;
  float y = (float)iy / h;
  float x_diff = x - prev_x;
  float y_diff = y - prev_y;

  float offset = 0.05;

  switch(selected_) {
  case 1: // top control point
    top_y_ += y_diff;
    if (top_y_ < bottom_y_ + offset)
      top_y_ = bottom_y_ + offset;
    break;
  case 2: // bottom control point
    bottom_y_ += y_diff;
    if (bottom_y_ > top_y_ - offset)
      bottom_y_ = top_y_ - offset;
    break;
  case 3: // left control point
    left_x_ += x_diff;
    left_y_ += y_diff;
    if (left_x_ > top_x_ - offset)
      left_x_ = top_x_ - offset;
    top_x_ = (left_x_ + right_x_) / 2.0f;
    bottom_x_ = (left_x_ + right_x_) / 2.0f;
    break;
  case 4: // right control point
    right_x_ += x_diff;
    right_y_ += y_diff;
    if (right_x_ < top_x_ + offset)
      right_x_ = top_x_ + offset;
    top_x_ = (left_x_ + right_x_) / 2.0f;
    bottom_x_ = (left_x_ + right_x_) / 2.0f;
    break;
  case 5: // grab widget
    top_x_ += x_diff;
    bottom_x_ += x_diff;
    left_x_ += x_diff;
    right_x_ += x_diff;
    top_y_ += y_diff;
    bottom_y_ += y_diff;
    left_y_ += y_diff;
    right_y_ += y_diff;
    
    break;
  }

  // save off mouse position
  last_x_ = ix;
  last_y_ = iy;
}


void
ParaboloidCM2Widget::release (int /*x*/, int /*y*/,
                             int /*w*/, int /*h*/)
{
}


string
ParaboloidCM2Widget::tcl_pickle()
{
  normalize();
  ostringstream s;

  s << "p ";
  s << top_x_ << " ";
  s << top_y_ << " ";
  s << bottom_x_ << " ";
  s << bottom_y_ << " ";
  s << left_x_ << " ";
  s << left_y_ << " ";
  s << right_x_ << " ";
  s << right_y_;

  return s.str();
}

void
ParaboloidCM2Widget::tcl_unpickle(const string &p)
{
  istringstream s(p);
  char c;
  s >> c;

  s >> top_x_;
  s >> top_y_;
  s >> bottom_x_;
  s >> bottom_y_;
  s >> left_x_;
  s >> left_y_;
  s >> right_x_;
  s >> right_y_;
}


void
ParaboloidCM2Widget::normalize()
{
}


void
ParaboloidCM2Widget::un_normalize()
{
}

/****************************
 *
 * Triangle Widget Functions
 *
 ***************************/
TriangleCM2Widget::TriangleCM2Widget() : 
  CM2Widget(),
  base_(0.5), 
  top_x_(0.15), 
  top_y_(0.5), 
  width_(0.25), 
  bottom_(0.5)
{
  base_ = 0.1+drand48()*0.8;
  top_x_ = -0.1 + drand48()*0.2;
  top_y_ = 0.2 + drand48()*0.8;
  width_ = 0.1 + drand48()*0.4;
  bottom_ = drand48()*0.4+0.2;
  name_ = "Triangle";
}

TriangleCM2Widget::TriangleCM2Widget(float base, float top_x, float top_y,
                                     float width, float bottom) : 
  CM2Widget(),
  base_(base), 
  top_x_(top_x), 
  top_y_(top_y), 
  width_(width), 
  bottom_(bottom)
{}

TriangleCM2Widget::~TriangleCM2Widget()
{
}

TriangleCM2Widget::TriangleCM2Widget(const TriangleCM2Widget& copy) :
  CM2Widget(copy)
{
  // The public virtual inheritance, forces the blind binary copy here.
  // basically you can't call CM2Widget's copy constructor during 
  // initialization, which is either a compiler bug, or a C++ oversight.
  *this = copy;
}

void
TriangleCM2Widget::rasterize(CM2ShaderFactory& factory)
{
  CHECK_OPENGL_ERROR();
  if(!onState_) return;

  CM2BlendType blend = CM2_BLEND_RASTER;
  CHECK_OPENGL_ERROR();
  FragmentProgramARB* shader = 
    factory.shader(CM2_SHADER_TRIANGLE, shadeType_, faux_, blend);

  if (!shader) return;
  CHECK_OPENGL_ERROR();
  if(!shader->valid()) shader->create();
    
  GLdouble modelview[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  double panx = (modelview[12]+1.0)/2.0;
  double pany = (modelview[13]+1.0)/2.0;
  double scalex = (modelview[0])/2.0;
  double scaley = (modelview[5])/2.0;
  normalize();  
  shader->bind();
  shader->setLocalParam(0, color_.r(), color_.g(), color_.b(), alpha_);
  shader->setLocalParam(1, base_*scalex+panx, 
                        scalex*(base_+top_x_)+panx,
                        top_y_*scaley+pany, pany);
  shader->setLocalParam(2, width_*scalex, bottom_, pany, 0.0);
  
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  shader->setLocalParam(3, 1.0 / vp[2], 1.0 / vp[3], pany, pany);
  
  glBegin(GL_TRIANGLES);
  {
    glVertex2f(base_, 0.0);
    glVertex2f(base_ + top_x_ + width_ / 2, top_y_);
    glVertex2f(base_ + top_x_ - width_ / 2, top_y_);
  }
  glEnd();
  shader->release();
  un_normalize();
  CHECK_OPENGL_ERROR();
}

void
TriangleCM2Widget::draw()
{
  if(!onState_){
    normalize();
    const float b_x = bottom_*top_x_ + base_;
    const float b_y = bottom_*top_y_;
    const float w = bottom_*width_;
    
    double r = 0.5, g = 0.5, b = 0.5;
    if (selected_) {
      r = 0.8; g = 0.7; b = 0.1;
    }
    draw_thin_gl_line(b_x-w/2, b_y, b_x+w/2, b_y, r,g,b);
    draw_thin_gl_line(base_+top_x_-width_/2, top_y_,
                       base_+top_x_+width_/2, top_y_,  r, g, b);
    draw_thin_gl_line(base_, 0.0, base_+top_x_-width_/2, top_y_, r,g,b);
    draw_thin_gl_line(base_, 0.0, base_+top_x_+width_/2, top_y_, r,g,b);
    
    if (selected_) {
      r = 0.9; g = 0.6; b = 0.4;
    }
    
    draw_thin_gl_point(base_, 0.0, r,g,b);
    draw_thin_gl_point(b_x-w/2, b_y,r,g,b);
    draw_thin_gl_point(b_x+w/2, b_y,r,g,b);
    draw_thin_gl_point(base_+top_x_-width_/2, top_y_,r,g,b);
    draw_thin_gl_point(base_+top_x_+width_/2, top_y_,r,g,b);
    un_normalize();
    
    return;
  }

  normalize();
  const float b_x = bottom_*top_x_ + base_;
  const float b_y = bottom_*top_y_;
  const float w = bottom_*width_;

  double r = 0.5, g = 0.5, b = 0.5;
  if (selected_) {
    r = 0.8; g = 0.7; b = 0.1;
  }
  draw_thick_gl_line(b_x-w/2, b_y, b_x+w/2, b_y, r,g,b);
  draw_thick_gl_line(base_+top_x_-width_/2, top_y_,
                     base_+top_x_+width_/2, top_y_,  r, g, b);
  draw_thick_gl_line(base_, 0.0, base_+top_x_-width_/2, top_y_, r,g,b);
  draw_thick_gl_line(base_, 0.0, base_+top_x_+width_/2, top_y_, r,g,b);

  if (selected_) {
    r = 0.9; g = 0.6; b = 0.4;
  }

  draw_thick_gl_point(base_, 0.0, r,g,b);
  draw_thick_gl_point(b_x-w/2, b_y,r,g,b);
  draw_thick_gl_point(b_x+w/2, b_y,r,g,b);
  draw_thick_gl_point(base_+top_x_-width_/2, top_y_,r,g,b);
  draw_thick_gl_point(base_+top_x_+width_/2, top_y_,r,g,b);
  un_normalize();
}


int
TriangleCM2Widget::pick1 (int ix, int iy, int sw, int sh)
{
  int ret_val = 0;
  normalize();
  //todo
  double point_size_ = 5.0;
  double thick_line_width_ = 5.0;
  last_x_ = top_x_;
  last_y_ = top_y_;
  pick_ix_ = ix;
  pick_iy_ = iy;
  last_width_ = width_;

  const double x = ix / (double)sw;
  const double y = iy / (double)sh;
  const double xeps = point_size_ / sw * 0.5;
  const double yeps = point_size_ / sh * 0.5;
  const double yleps = thick_line_width_ / sh;
  const float b_x = bottom_*top_x_ + base_;
  const float b_y = bottom_*top_y_;
  const float w = bottom_*width_;
  const double top_right_x = base_ + top_x_ + width_/2;
  const double top_left_x = base_ + top_x_ - width_/2;
  const double bot_left_x = b_x - w/2;
  const double bot_right_x = b_x + w/2;


  // upper right corner
  if (fabs(x - top_right_x) < xeps && fabs(y - top_y_) < yeps) ret_val = 2; 
  // upper left corner
  else if (fabs(x - top_left_x) < xeps && fabs(y - top_y_) < yeps) ret_val = 6;
  // middle left corner
  else if (fabs(x - bot_left_x) < xeps && fabs(y - b_y) < yeps) ret_val = 3;
  // middle right corner
  else if (fabs(x - bot_right_x) < xeps && fabs(y - b_y) < yeps) ret_val = 8; 
  // top bar
  else if (fabs(y - top_y_) < yleps && x > Min(top_right_x, top_left_x) && 
           x < Max(top_right_x, top_left_x)) ret_val = 4; 
  // bottom bar
  else if (fabs(y - b_y) < yleps && x > Min(bot_left_x, bot_right_x) && 
           x < Max(bot_left_x, bot_right_x)) ret_val = 7;
  un_normalize();
  return ret_val;
}


int
TriangleCM2Widget::pick2 (int ix, int iy, int sw, int sh, int m)
{
  normalize();
  const double x = ix / (double)sw;
  const double y = iy / (double)sh;
  const double x1top = top_x_ + width_ * 0.5;
  const double x2top = top_x_ - width_ * 0.5;
  const double x1 = base_ + x1top * (y / top_y_);
  const double x2 = base_ + x2top * (y / top_y_);
  int ret_val = 0;
  if (y < top_y_ && x > Min(x1, x2) && x < Max(x1, x2))
  {
    last_x_ = base_;
    pick_ix_ = ix;
    pick_iy_ = iy; // modified only.
    last_hsv_ = HSVColor(color_);  // modified only
    ret_val = m?5:1;
  }
  un_normalize();
  return ret_val;
}


string
TriangleCM2Widget::tk_cursorname(int obj)
{
  switch (obj) {
  case 0: return string("left_ptr"); break;
  case 1: return string("sb_h_double_arrow"); break;
  case 2: return string("fleur"); break;
  case 3: return string("sb_v_double_arrow"); break;
  case 4: return string("fleur"); break;
  case 5: return string("left_ptr"); break;
  case 6: return string("fleur"); break;
  case 7: return string("sb_v_double_arrow"); break;
  case 8: return string("sb_v_double_arrow"); break;
  default: break;
  }
  return string("left_ptr");
}
    
    


void
TriangleCM2Widget::move (int ix, int iy, int w, int h)
{
  const double x = ix / (double)w;
  const double y = iy / (double)h;
  normalize();
  switch (selected_)
  {
  case 1:
    base_ = last_x_ + x - pick_ix_ / (double)w;
    break;

  case 2:
    width_ = (x - top_x_ - base_) * 2.0;
    top_y_ = last_y_ + y - pick_iy_ / (double)h;
    break;

  case 3:
    bottom_ = Clamp(y / top_y_, 0.0, 1.0);
    break;

  case 4:
    top_x_ = last_x_ + x - pick_ix_ / (double)w;
    top_y_ = last_y_ + y - pick_iy_ / (double)h;
    width_ = last_width_ * top_y_ / last_y_;
    break;
    
  case 5:
    {
      // Hue controls on x axis
      const double hdelta = x - pick_ix_ / (double)w;
      double hue = last_hsv_[0] + hdelta * 360.0 * 2.0;
      while (hue < 0.0) hue += 360.0;
      while (hue > 360.0) hue -= 360;

      // Saturation controls on y axis
      const double sdelta = y - pick_iy_ / (double)h;
      double sat = Clamp(last_hsv_[1] - sdelta * 2.0, 0.0, 1.0);

      HSVColor hsv(hue, sat, last_hsv_.val());
      color_ = Color(hsv);
    }
    break;
  case 6:
    width_ = (x - top_x_ - base_) * 2.0;
    top_y_ = last_y_ + y - pick_iy_ / (double)h;
    break;

  case 7:
    bottom_ = Clamp(y / top_y_, 0.0, 1.0);
    break;

  case 8:
    bottom_ = Clamp(y / top_y_, 0.0, 1.0);
    break;

  default:
    break;
  }
  un_normalize();
}


void
TriangleCM2Widget::release (int /*x*/, int /*y*/, 
                            int /*w*/, int /*h*/)
{
  // Don't need to do anything here.
}


string
TriangleCM2Widget::tcl_pickle()
{
  normalize();
  ostringstream s;
  s << "t ";
  s << base_ << " ";
  s << top_x_ << " ";
  s << top_y_ << " ";
  s << width_ << " ";
  s << bottom_;
  un_normalize();
  return s.str();
}

void
TriangleCM2Widget::tcl_unpickle(const string &p)
{
  istringstream s(p);
  char c;
  s >> c;
  s >> base_;
  s >> top_x_;
  s >> top_y_;
  s >> width_;
  s >> bottom_;
  value_range_.first = 0.0;
  value_range_.second = -1.0;
}


void
TriangleCM2Widget::normalize()
{
  if (value_range_.first > value_range_.second) return;
  const float offset = -value_range_.first;
  const float scale = 1.0/(value_range_.second-value_range_.first);

  base_ = (base_ + offset) * scale;
  top_x_ = top_x_ * scale;
  width_ = width_ * scale;
}


void
TriangleCM2Widget::un_normalize()
{
  if (value_range_.first > value_range_.second) return;
  const float offset = -value_range_.first;
  const float scale = 1.0/(value_range_.second-value_range_.first);
  
  base_ = base_/scale - offset;
  top_x_ = top_x_/scale;
  width_ = width_/scale;
}


/****************************
 *
 * Image Widget Functions
 *
 ***************************/
ImageCM2Widget::ImageCM2Widget() : 
  pixels_(0)
{
}

ImageCM2Widget::ImageCM2Widget(Nrrd* p) : 
  pixels_(p)
{}

ImageCM2Widget::~ImageCM2Widget()
{}

ImageCM2Widget::ImageCM2Widget(const ImageCM2Widget& copy) :
  CM2Widget(copy)
{
  // The public virtual inheritance, forces the blind binary copy here.
  // basically you can't call CM2Widget's copy constructor during 
  // initialization, which is either a compiler bug, or a C++ oversight.
  *this = copy;
  // now the deep copy for the pixel data.
  nrrdCopy(pixels_, copy.pixels_);
}

const float trans = 1.0/255.0;

void
ImageCM2Widget::rasterize(CM2ShaderFactory& /*factory*/)
{
  CHECK_OPENGL_ERROR();
  //assume images draw first.
  
  if (! pixels_) return;
  
  glDisable(GL_BLEND);
  glRasterPos2i(0,0);

  // float data in the range 0 - 255 
  glPixelTransferf(GL_RED_SCALE, trans);
  glPixelTransferf(GL_GREEN_SCALE, trans);
  glPixelTransferf(GL_BLUE_SCALE, trans);
  glPixelTransferf(GL_ALPHA_SCALE, trans);


  Nrrd *nout = pixels_;
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);

  if ((unsigned int) vp[2] != nout->axis[1].size ||
      (unsigned int) vp[3] != nout->axis[2].size) 
  {
    nout = resize(vp[2], vp[3]);
  }
  if (!nout) return;

  glDrawPixels(vp[2], vp[3], GL_RGBA, GL_FLOAT, (float*)nout->data);

  if (nout != pixels_)
    nrrdNuke(nout);

  // restore default values
  glPixelTransferf(GL_RED_SCALE, 1.0);
  glPixelTransferf(GL_GREEN_SCALE, 1.0);
  glPixelTransferf(GL_BLUE_SCALE, 1.0);
  glPixelTransferf(GL_ALPHA_SCALE, 1.0);

  glEnable(GL_BLEND);
  CHECK_OPENGL_ERROR();
}

Nrrd*
ImageCM2Widget::resize(int width, int height) 
{
  Nrrd *nin   = pixels_;
  NrrdResampleInfo *info = nrrdResampleInfoNew();
  NrrdKernel *kern;
  double p[NRRD_KERNEL_PARMS_NUM];
  memset(p, 0, NRRD_KERNEL_PARMS_NUM * sizeof(double));
  p[0] = 1.0L;
  kern = nrrdKernelBCCubic; 
  p[1] = 1.0L; 
  p[2] = 0.0L; 
  info->kernel[0] = kern;
  info->kernel[1] = kern;
  info->kernel[2] = kern;
  info->samples[0]= 4;
  info->samples[1]= width;
  info->samples[2]= height;

  for (int a = 0; a < 3; a++)
  {
    if (nrrdKindSize(nin->axis[a].kind) > 1) {
      std::cerr << "Trying to resample along axis " << a 
                << " which is not of nrrdKindDomain or nrrdKindUnknown." 
                << std::endl;
    }

    memcpy(info->parm[a], p, NRRD_KERNEL_PARMS_NUM * sizeof(double));
    if (info->kernel[a] && 
        (!(airExists(nin->axis[a].min) && 
           airExists(nin->axis[a].max)))) {
      nrrdAxisInfoMinMaxSet(nin, a, nin->axis[a].center ? 
                            nin->axis[a].center : nrrdCenterCell);
    }
    info->min[a] = nin->axis[a].min;
    info->max[a] = nin->axis[a].max;
  } 

  info->boundary = nrrdBoundaryBleed;
  info->type = nin->type;
  info->renormalize = AIR_TRUE;

  bool fail = false;
  Nrrd *nrrd_resamp = nrrdNew();
  if (nrrdSpatialResample(nrrd_resamp, nin, info)) {
    char *err = biffGetDone(NRRD);
    std::cerr << "Resample Failed in Core/Volume/CM2Widget.cc: " 
              << err << std::endl;
    free(err);
    fail = true;
  }
  nrrdResampleInfoNix(info); 
  if (fail) {
    nrrdNuke(nrrd_resamp);
    return 0;
  } else {
    return nrrd_resamp;
  }
}

void
ImageCM2Widget::draw()
{
  // no widget controls to draw.
}


int
ImageCM2Widget::pick1(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
  printf("pick1: not implemented\n"); 
  return 0;
}


int
ImageCM2Widget::pick2(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*m*/)
{
  printf("pick2: not implemented\n");
  return 0;
}

void
ImageCM2Widget::move(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
  printf("move: not implemented\n");
}

void
ImageCM2Widget::release(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
  printf("release: not implemented\n");
}

std::string
ImageCM2Widget::tcl_pickle()
{ 
  printf("release: not implemented\n");
  return "";
}

void
ImageCM2Widget::tcl_unpickle(const std::string &/*p*/)
{
  printf("release: not implemented\n");
}



/****************************
 *
 * Paint Widget Functions
 *
 ***************************/
PaintCM2Widget::PaintCM2Widget() : 
  CM2Widget(),
  strokes_()
{
  name_ = "Paint";
}

PaintCM2Widget::~PaintCM2Widget()
{}

PaintCM2Widget::PaintCM2Widget(const PaintCM2Widget& copy) :
  CM2Widget(copy)
{
  // The public virtual inheritance, forces the blind binary copy here.
  // basically you can't call CM2Widget's copy constructor during 
  // initialization, which is either a compiler bug, or a C++ oversight.
  *this = copy;
}

void
PaintCM2Widget::rasterize(CM2ShaderFactory& factory)
{
  if(!onState_) return;
  normalize();
  CM2BlendType blend = CM2_BLEND_RASTER;

  FragmentProgramARB* shader = 
    factory.shader(CM2_SHADER_PAINT, shadeType_, faux_, blend);
  if (!shader) return;

  if(!shader->valid()) {
    shader->create();
  }
    
  GLdouble modelview[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  double pany = (modelview[13]+1.0)/2.0;
  
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  
  shader->bind();
  shader->setLocalParam(0, color_.r(), color_.g(), color_.b(), alpha_);
  shader->setLocalParam(3, 1.0/vp[2], 1.0/vp[3], pany, pany);
  
  GLdouble mid[4] = { color_.r(), color_.g(), color_.b(), alpha_ };
  GLdouble edg[4] = { color_.r(), color_.g(), color_.b(), 0 };
  glShadeModel(GL_SMOOTH);    
  double range = 1.0;
  if (value_range_.first < value_range_.second)
    range = value_range_.second - value_range_.first;
  for (unsigned int s = 0; s < strokes_.size(); ++s)
  {
    const double halfx = strokes_[s].first/range;
    Stroke &stroke = strokes_[s].second;
    const unsigned int coordinates = stroke.size();
    for (unsigned c = 1; c < coordinates; ++c)
    {
      glBegin(GL_QUADS);
      glColor4dv(edg);
      glVertex2d(stroke[c-1].first-halfx, stroke[c-1].second);
      
      glColor4dv(mid);
      glVertex2d(stroke[c-1].first, stroke[c-1].second);
      
      glColor4dv(mid);
      glVertex2d(stroke[c].first, stroke[c].second);
      
      glColor4dv(edg);
      glVertex2d(stroke[c].first-halfx, stroke[c].second);
      
      
      glColor4dv(mid);
      glVertex2d(stroke[c-1].first, stroke[c-1].second);
      
      glColor4dv(edg);
      glVertex2d(stroke[c-1].first+halfx, stroke[c-1].second);
      
      glColor4dv(edg);
      glVertex2d(stroke[c].first+halfx, stroke[c].second);
      
      glColor4dv(mid);
      glVertex2d(stroke[c].first, stroke[c].second);
      glEnd();
    }
  }
  glShadeModel(GL_FLAT);
  shader->release();
  un_normalize();
  CHECK_OPENGL_ERROR();
}

void
PaintCM2Widget::draw()
{
  // no widget controls to draw.
}

int
PaintCM2Widget::pick1(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
  printf("pick1: not implemented\n"); 
  return 0;
}

int
PaintCM2Widget::pick2(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*m*/)
{
  printf("pick2: not implemented\n");
  return 0;
}

void
PaintCM2Widget::move(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
  printf("move: not implemented\n");
}

void
PaintCM2Widget::release(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
  printf("release: not implemented\n");
}

std::string
PaintCM2Widget::tcl_pickle()
{
  printf("release: not implemented\n");
  return "";
}

void
PaintCM2Widget::tcl_unpickle(const std::string &/*p*/)
{
  printf("release: not implemented\n");
}

void
PaintCM2Widget::add_stroke(double width)
{
  double range = 1.0;
  if (value_range_.first < value_range_.second)
    range = value_range_.second - value_range_.first;

  if (width < 0.0)
    width = range/35.0;
  strokes_.push_back(make_pair(width,Stroke()));
}

void
PaintCM2Widget::add_coordinate(const Coordinate &coordinate)
{
  if (strokes_.empty()) return;

  Stroke &stroke = strokes_.back().second;
  // filter duplicate points
  if (!stroke.empty() && 
      coordinate.first == stroke.back().first &&
      coordinate.second == stroke.back().second) return;
      
  stroke.push_back(coordinate);
}

bool
PaintCM2Widget::pop_stroke()
{
  if (strokes_.empty()) return false;
  strokes_.pop_back();
  return true;
}

void
PaintCM2Widget::normalize()
{
  const float offset = -value_range_.first;
  const float scale = 1.0/(value_range_.second-value_range_.first);
  for (unsigned int s = 0; s < strokes_.size(); ++s)
    for (unsigned int c = 0; c < strokes_[s].second.size(); ++c)
      strokes_[s].second[c].first = 
        (strokes_[s].second[c].first + offset) * scale;
}

void
PaintCM2Widget::un_normalize()
{
  const float offset = -value_range_.first;
  const float scale = 1.0/(value_range_.second-value_range_.first);
  for (unsigned int s = 0; s < strokes_.size(); ++s)
    for (unsigned int c = 0; c < strokes_[s].second.size(); ++c)
      strokes_[s].second[c].first = strokes_[s].second[c].first/scale - offset;
}

ColorMapCM2Widget::ColorMapCM2Widget() : 
  RectangleCM2Widget(),
  colormap_(0)
{
  left_x_ = drand48()*0.9;
  left_y_ = drand48()*0.9;
  width_ = Clamp(0.1+(0.9-left_x_)*drand48(), 0.1, 0.5);
  height_ = Clamp(0.1+(0.9-left_y_)*drand48(), 0.5*width_, 1.5*width_);
  offset_ = 0.25+0.5*drand48();
  name_ = "ColorMap";
}

ColorMapCM2Widget::ColorMapCM2Widget(CM2RectangleType type, float left_x, 
                                     float left_y, float width, float height,
                                     float offset) : 
  RectangleCM2Widget(type, left_x, left_y, width, height, offset),
  colormap_(0)
{
  name_ = "ColorMap";
}

ColorMapCM2Widget::~ColorMapCM2Widget()
{}

ColorMapCM2Widget::ColorMapCM2Widget(const ColorMapCM2Widget& copy) :
  CM2Widget(copy), RectangleCM2Widget(copy)
{
  // The public virtual inheritance, forces the blind binary copy here.
  // basically you can't call CM2Widget's copy constructor during 
  // initialization, which is either a compiler bug, or a C++ oversight.
  *this = copy;
}

static GLuint cmap_tex_ = 0;

void
ColorMapCM2Widget::draw()
{
  if(!onState_) return;

  double r = 0.5, g = 0.5, b = 0.5;
  if (selected_) {
    r = 0.9; g = 0.8; b = 0.1;
  }
  normalize();
  draw_thick_gl_line(left_x_, left_y_, left_x_+width_, left_y_, r,g,b);
  draw_thick_gl_line(left_x_, left_y_+height_, left_x_+width_, left_y_+height_,
                     r,g,b);

  draw_thick_gl_line(left_x_, left_y_, left_x_, left_y_+height_, r,g,b);
  draw_thick_gl_line(left_x_+width_, left_y_, left_x_+width_, left_y_+height_, 
                     r,g,b);

  if (selected_) {
    r = 0.9; g = 0.6; b = 0.4;
  }

  draw_thick_gl_point(left_x_, left_y_,r,g,b);
  draw_thick_gl_point(left_x_+width_, left_y_,r,g,b);
  draw_thick_gl_point(left_x_, left_y_+height_,r,g,b);
  draw_thick_gl_point(left_x_+width_, left_y_+height_,r,g,b);
  draw_thick_gl_point(left_x_+offset_*width_, left_y_+height_*0.5,r,g,b);
  un_normalize();
}


void
ColorMapCM2Widget::set_colormap(ColorMap* cmap) {
  colormap_ = cmap;
}

ColorMap*
ColorMapCM2Widget::get_colormap() {
  return colormap_;
}


void
ColorMapCM2Widget::rasterize(CM2ShaderFactory& factory)
{
  if (!onState_) return;
  CM2BlendType blend = CM2_BLEND_RASTER;

  FragmentProgramARB* shader =
    factory.shader(CM2_SHADER_TEXTURE, shadeType_, true, blend);

  if (!shader) return;
  
  if(!shader->valid()) {
    shader->create();
  }
 
  normalize();
    
  GLdouble modelview[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  double panx = (modelview[12]+1.0)/2.0;
  double pany = (modelview[13]+1.0)/2.0;
  double scalex = (modelview[0])/2.0;
  double scaley = (modelview[5])/2.0;
  double left_x = left_x_*scalex+panx;
  double left_y = left_y_*scaley+pany;
  double width = width_*scalex;
  double height = height_*scaley;
  
  shader->bind();
  shader->setLocalParam(1, left_x, left_y, width, height);

  if(offset_ < SLIVR_FLOAT_EPSILON )
    shader->setLocalParam(2, offset_, 0.0, 1.0, 0.0);
  else if((1.0-offset_) < SLIVR_FLOAT_EPSILON )
    shader->setLocalParam(2, offset_, 1.0, 0.0, 0.0);
  else
    shader->setLocalParam(2, offset_, 1/offset_, 1/(1-offset_), 0.0);
  
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  shader->setLocalParam(3, 1.0/vp[2], 1.0/vp[3], 0.0, 0.0);

  glColor4d(1.0, 1.0, 1.0, 1.0);
  glActiveTexture(GL_TEXTURE1);
  glEnable(GL_TEXTURE_1D);
  glDeleteTextures(1, &cmap_tex_);
  glGenTextures(1, &cmap_tex_);
  glBindTexture(GL_TEXTURE_1D, cmap_tex_);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, colormap_->resolution(), 0,
               GL_RGBA, GL_FLOAT, colormap_->get_rgba());
  glBegin(GL_QUADS);
  {
    glMultiTexCoord1f(GL_TEXTURE1, 0.0);
    glVertex2f(left_x_, left_y_);
    glMultiTexCoord1f(GL_TEXTURE1, 0.5);
    glVertex2f(left_x_+width_*offset_, left_y_);
    glMultiTexCoord1f(GL_TEXTURE1, 0.5);
    glVertex2f(left_x_+width_*offset_, left_y_+height_);
    glMultiTexCoord1f(GL_TEXTURE1, 0.0);
    glVertex2f(left_x_, left_y_+height_);

    glMultiTexCoord1f(GL_TEXTURE1, 0.5);
    glVertex2f(left_x_+width*offset_, left_y_);
    glMultiTexCoord1f(GL_TEXTURE1, 1.0);
    glVertex2f(left_x_+width_, left_y_);
    glMultiTexCoord1f(GL_TEXTURE1, 1.0);
    glVertex2f(left_x_+width_, left_y_+height_);
    glMultiTexCoord1f(GL_TEXTURE1, 0.5);
    glVertex2f(left_x_+width_*offset_, left_y_+height_);


  }
  glEnd();
  glDisable(GL_TEXTURE_1D);
  glActiveTexture(GL_TEXTURE0);
  un_normalize();
  shader->release();
}

string
ColorMapCM2Widget::tcl_pickle()
{
  normalize();
  ostringstream s;
  s << "r ";
  s << (int)type_ << " ";
  s << left_x_ << " ";
  s << left_y_ << " ";
  s << width_ << " ";
  s << height_ << " ";
  s << offset_;
  un_normalize();
  return s.str();
}

void
ColorMapCM2Widget::tcl_unpickle(const string &p)
{
  istringstream s(p);
  char c;
  s >> c;
  int t;
  s >> t;
  type_ = (CM2RectangleType)t;
  s >> left_x_;
  s >> left_y_;
  s >> width_;
  s >> height_;
  s >> offset_;
  value_range_.first = 0.0;
  value_range_.second = -1.0;
}

ClippingCM2Widget::ClippingCM2Widget() : 
  CM2Widget(),
  plane_()
{
  name_ = "Clipping";
}

ClippingCM2Widget::ClippingCM2Widget(const Plane &plane)
  : CM2Widget(),
    plane_(plane)
{
  name_ = "Clipping P,ane";
}

ClippingCM2Widget::~ClippingCM2Widget()
{}

ClippingCM2Widget::ClippingCM2Widget(const ClippingCM2Widget& copy) :
  CM2Widget(copy)
{
  // The public virtual inheritance, forces the blind binary copy here.
  // basically you can't call CM2Widget's copy constructor during 
  // initialization, which is either a compiler bug, or a C++ oversight.
  *this = copy;
}

void
ClippingCM2Widget::draw()
{
}

void
ClippingCM2Widget::rasterize(CM2ShaderFactory&)
{
}

int
ClippingCM2Widget::pick1(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
  return 0;
}

int
ClippingCM2Widget::pick2(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*m*/)
{
  return 0;
}

void
ClippingCM2Widget::move(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
}

void
ClippingCM2Widget::release(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
}

std::string
ClippingCM2Widget::tcl_pickle()
{
  return "";
}

void
ClippingCM2Widget::tcl_unpickle(const std::string &/*p*/)
{
}


} //namespace SLIVR
