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
//    File   : CM2Widget.h
//    Author : Milan Ikits
//    Date   : Mon Jul  5 18:33:12 2004

#ifndef SLIVR_CM2Widget_h
#define SLIVR_CM2Widget_h

#include <string>
#include <teem/nrrd.h>
#include <slivr/CM2Shader.h>
#include <slivr/Color.h>
#include <slivr/ColorMap.h>
#include <slivr/BBox.h>
#include <slivr/Plane.h>
#include <slivr/share.h>

#include <utility>

namespace SLIVR {

class CM2ShaderFactory;
using std::pair;

typedef std::pair<float, float> range_t;

class SLIVRSHARE CM2Widget
{
public:
  CM2Widget();
  CM2Widget(const CM2Widget& copy);
  virtual ~CM2Widget();  

  virtual CM2Widget* duplicate() const = 0;
  
  // Draw widget frame/controls
  virtual void          draw() = 0;

  // Draw widget to hardware GL buffer
  virtual void          rasterize(CM2ShaderFactory& factory) = 0;

  // Getters/Setters
  virtual range_t       get_value_range() { return value_range_; }
  virtual void          set_value_range(range_t range);
  virtual std::string   get_name() { return name_; }
  virtual void          set_name(std::string name) { name_ = name; }
  virtual int           get_shadeType() { return shadeType_; }
  virtual void          set_shadeType(int type) { shadeType_ = type; }
  virtual int           get_onState() { return onState_; }
  virtual void          set_onState(int state) { onState_ = state; }
  virtual bool          get_faux() { return faux_; }
  virtual void          set_faux(bool faux) { faux_ = faux; }
  virtual Color         get_color() const { return color_; }
  virtual void          set_color(const Color &c) { color_ = c; }
  virtual float         get_alpha() const { return alpha_; }
  virtual void          set_alpha(float alpha) { alpha_ = alpha; }

  // Selection
  virtual void          select(int obj)  { selected_ = obj; }
  virtual int           selected() const { return selected_; }
  virtual void          unselect_all()   { selected_ = 0; }
  virtual std::string   tk_cursorname(int) { return "left_ptr"; };

  // Pure Virtual Methods below

  // Picking/movement
  virtual int           pick1(int x, int y, int w, int h) = 0;
  virtual int           pick2(int x, int y, int w, int h, int m) = 0;
  virtual void          move(int x, int y, int w, int h) = 0;
  virtual void          release(int x, int y, int w, int h) = 0;


  // State management to/from a string
  virtual std::string   tcl_pickle() = 0;
  virtual void          tcl_unpickle(const std::string &pickled) = 0;

protected:
  virtual void          normalize() {}
  virtual void          un_normalize() {}
  virtual void          draw_thick_gl_line(double x1, double y1, 
                                           double x2, double y2,
                                           double r, double g, double b);
  virtual void          draw_thick_gl_point(double x1, double y1,
                                            double r, double g, double b);
  virtual void          draw_thin_gl_line(double x1, double y1, 
                                           double x2, double y2,
                                           double r, double g, double b);
  virtual void          draw_thin_gl_point(double x1, double y1,
                                            double r, double g, double b);

  std::string           name_;
  Color                 color_;
  float                 alpha_;
  int                   selected_;
  int                   shadeType_;
  int                   onState_;
  bool                  faux_;
  HSVColor              last_hsv_;
  range_t               value_range_;
};



class SLIVRSHARE ClippingCM2Widget : public virtual CM2Widget
{
public:
  ClippingCM2Widget();
  ClippingCM2Widget(const ClippingCM2Widget& copy);
  ClippingCM2Widget(const Plane &);
  virtual ~ClippingCM2Widget();
  virtual CM2Widget* duplicate() const {return new ClippingCM2Widget(*this);} 

  Plane &                plane() { return plane_; }
  
  virtual void          draw();
  virtual void          rasterize(CM2ShaderFactory&);

  virtual int           pick1(int x, int y, int w, int h);
  virtual int           pick2(int x, int y, int w, int h, int m);
  virtual void          move(int x, int y, int w, int h);
  virtual void          release(int x, int y, int w, int h);
  virtual std::string   tcl_pickle();
  virtual void          tcl_unpickle(const std::string &pickled);;

protected:
  Plane                 plane_;
};



class SLIVRSHARE TriangleCM2Widget : public virtual CM2Widget
{
public:
  TriangleCM2Widget();
  TriangleCM2Widget(const TriangleCM2Widget& copy);
  TriangleCM2Widget(float base, float top_x, float top_y,
                    float width, float bottom);
  virtual ~TriangleCM2Widget();
  virtual CM2Widget* duplicate() const {return new TriangleCM2Widget(*this);} 

  virtual void          draw();
  virtual void          rasterize(CM2ShaderFactory& factory);
  virtual int           pick1(int x, int y, int w, int h);
  virtual int           pick2(int x, int y, int w, int h, int m);
  virtual void          move(int x, int y, int w, int h);
  virtual void          release(int x, int y, int w, int h);
  virtual std::string   tk_cursorname(int obj);
  virtual std::string   tcl_pickle();
  virtual void          tcl_unpickle(const std::string &p);

protected:
  void                  normalize();
  void                  un_normalize();  

  float                 base_;
  float                 top_x_;
  float                 top_y_;
  float                 width_;
  float                 bottom_;

  // Used by picking.
  float                 last_x_;
  float                 last_y_;
  float                 last_width_;
  int                   pick_ix_;
  int                   pick_iy_;
};



enum CM2RectangleType
{
  CM2_RECTANGLE_1D = 0,
  CM2_RECTANGLE_ELLIPSOID = 1
};

class SLIVRSHARE RectangleCM2Widget : public virtual CM2Widget
{
public:
  RectangleCM2Widget();
  RectangleCM2Widget(const RectangleCM2Widget& copy);
  RectangleCM2Widget(CM2RectangleType type, float left_x, float left_y,
                     float width, float height, float offset);
  virtual ~RectangleCM2Widget();

  virtual CM2Widget* duplicate() const 
  { return new RectangleCM2Widget(*this); } 

  virtual void          draw();
  virtual void          rasterize(CM2ShaderFactory& factory);
  virtual int           pick1(int x, int y, int w, int h);
  virtual int           pick2(int x, int y, int w, int h, int m);
  virtual void          move(int x, int y, int w, int h);
  virtual void          release(int x, int y, int w, int h);
  virtual std::string   tk_cursorname(int obj);
  virtual std::string   tcl_pickle();
  virtual void          tcl_unpickle(const std::string &p);

protected:
  void                  normalize();
  void                  un_normalize();
  
  CM2RectangleType      type_;
  float                 left_x_;
  float                 left_y_;
  float                 width_;
  float                 height_;
  float                 offset_;

  // Used by picking.
  float                 last_x_;
  float                 last_y_;
  int                   pick_ix_;
  int                   pick_iy_;
};



class SLIVRSHARE EllipsoidCM2Widget : public virtual CM2Widget
{
public:
  EllipsoidCM2Widget();
  EllipsoidCM2Widget(const EllipsoidCM2Widget& copy);
  EllipsoidCM2Widget(float x, float y, float a, float b, float rot);
  virtual ~EllipsoidCM2Widget();

  virtual CM2Widget* duplicate() const 
  { return new EllipsoidCM2Widget(*this); } 

  virtual void          draw();
  virtual void          rasterize(CM2ShaderFactory& factory);
  virtual int           pick1(int x, int y, int w, int h);
  virtual int           pick2(int x, int y, int w, int h, int m);
  virtual void          move(int x, int y, int w, int h);
  virtual void          release(int x, int y, int w, int h);
  virtual std::string   tk_cursorname(int obj);
  virtual std::string   tcl_pickle();
  virtual void          tcl_unpickle(const std::string &p);

protected:
  void                  normalize();
  void                  un_normalize();

  float                 center_x_;
  float                 center_y_;
  float                 a_;
  float                 b_;
  float                 rot_; /* counter-clockwise rotation in radians */
                              /* (1, 0) would have rot_ = 0.0 */

  // Used by picking.
  float                 last_x_;
  float                 last_y_;
  int                   pick_ix_;
  int                   pick_iy_;
  // holds last position of control points
  float                 x_controls_[4]; 
  float                 y_controls_[4];
  float                 last_rot_;
};



class SLIVRSHARE ParaboloidCM2Widget : public virtual CM2Widget
{
public:
  ParaboloidCM2Widget();
  ParaboloidCM2Widget(const ParaboloidCM2Widget& copy);
  ParaboloidCM2Widget(float top_x, float top_y,
		      float bottom_x, float bottom_y,
		      float left_x, float left_y,
		      float right_x, float right_y);
  virtual ~ParaboloidCM2Widget();

  virtual CM2Widget* duplicate() const 
  { return new ParaboloidCM2Widget(*this); } 

  virtual void          draw();
  virtual void          rasterize(CM2ShaderFactory& factory);
  virtual int           pick1(int x, int y, int w, int h);
  virtual int           pick2(int x, int y, int w, int h, int m);
  virtual void          move(int x, int y, int w, int h);
  virtual void          release(int x, int y, int w, int h);
  virtual std::string   tk_cursorname(int obj);
  virtual std::string   tcl_pickle();
  virtual void          tcl_unpickle(const std::string &p);

protected:
  void                  normalize();
  void                  un_normalize();

  float top_x_;
  float top_y_;
  float bottom_x_;
  float bottom_y_;
  float left_x_;
  float left_y_;
  float right_x_; 
  float right_y_;

  // Used by picking.
  float                 last_x_;
  float                 last_y_;
  int                   pick_ix_;
  int                   pick_iy_;
  // holds last position of control points
  float                 x_controls_[4]; // top bottom left right
  float                 y_controls_[4];
};



class SLIVRSHARE ColorMapCM2Widget : public virtual RectangleCM2Widget
{
public:
  ColorMapCM2Widget();
  ColorMapCM2Widget(const ColorMapCM2Widget& copy);
  ColorMapCM2Widget(CM2RectangleType type, float left_x, float left_y,
                     float width, float height, float offset);
  virtual ~ColorMapCM2Widget();

  virtual CM2Widget* duplicate() const 
  { return new ColorMapCM2Widget(*this); } 

  virtual void          draw();
  virtual void          rasterize(CM2ShaderFactory& factory);
  virtual std::string   tcl_pickle();
  virtual void          tcl_unpickle(const std::string &p);

  ColorMap*             get_colormap();
  void                  set_colormap(ColorMap* cmap);
protected:
  ColorMap*             colormap_;
};



// The image widget cannot be manipulated, only drawn.
class SLIVRSHARE ImageCM2Widget : public virtual CM2Widget
{
public:
  ImageCM2Widget();
  ImageCM2Widget(const ImageCM2Widget& copy);
  ImageCM2Widget(Nrrd* p);
  virtual ~ImageCM2Widget();

  virtual CM2Widget* duplicate() const 
  { return new ImageCM2Widget(*this); } 

  virtual void          draw();
  virtual void          rasterize(CM2ShaderFactory& factory);
  virtual int           pick1(int x, int y, int w, int h);
  virtual int           pick2(int x, int y, int w, int h, int m);
  virtual void          move(int x, int y, int w, int h);
  virtual void          release(int x, int y, int w, int h);
  virtual std::string   tcl_pickle();
  virtual void          tcl_unpickle(const std::string &p);

protected:
  Nrrd*                 resize(int w, int h);  // nrrdSpatialResample ...
  Nrrd*                 pixels_;
};



class SLIVRSHARE PaintCM2Widget : public virtual CM2Widget
{
public:
  typedef pair<double, double>          Coordinate;
  typedef vector<Coordinate>            Stroke;
  typedef vector<pair<double, Stroke> > Strokes;

  PaintCM2Widget();
  virtual ~PaintCM2Widget();
  PaintCM2Widget(const PaintCM2Widget& copy);
  
  virtual CM2Widget* duplicate() const { return new PaintCM2Widget(*this); } 

  virtual void          draw();
  virtual void          rasterize(CM2ShaderFactory& factory);
  virtual int           pick1(int x, int y, int w, int h);
  virtual int           pick2(int x, int y, int w, int h, int m);
  virtual void          move(int x, int y, int w, int h);
  virtual void          release(int x, int y, int w, int h);
  virtual std::string   tcl_pickle();
  virtual void          tcl_unpickle(const std::string &p);

  void                  add_stroke(double width = -1.0);
  bool                  pop_stroke();
  void                  add_coordinate(const Coordinate &);
protected:
  void                  normalize();
  void                  un_normalize();  

  Strokes               strokes_;
};

} // End namespace SLIVR

#endif //  SLIVR_CM2Widget_h
