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
//    File   : SliceRenderer.h
//    Author : Milan Ikits
//    Date   : Wed Jul  7 23:36:05 2004

#ifndef SLIVR_SliceRenderer_h
#define SLIVR_SliceRenderer_h

//#include <Core/Thread/Mutex.h>
//#include <Core/Geometry/Point.h>
//#include <Core/Geometry/Ray.h>
//#include <Core/Geometry/Vector.h>
//#include <Core/Geometry/Transform.h>
//#include <Core/Geom/ColorMap.h>
//#include <Core/Geom/GeomObj.h>

//#include <Core/Containers/BinaryTree.h>
#include <slivr/Color.h>
#include <slivr/Texture.h>
#include <slivr/TextureRenderer.h>
#include <slivr/slivr_share.h>

namespace SLIVR {


class SLIVRSHARE SliceRenderer : public TextureRenderer
{
public:
  SliceRenderer(Texture* tex, 
		ColorMap* cmap1, 
		vector<ColorMap2*> &cmap2,
                int tex_mem);
  SliceRenderer(const SliceRenderer&);
  virtual ~SliceRenderer();

  inline void set_control_point(const Point& point) { control_point_ = point; }

  inline void set_x(bool b) { if(b) draw_view_ = false; draw_x_ = b; }
  inline void set_y(bool b) { if(b) draw_view_ = false; draw_y_ = b; }
  inline void set_z(bool b) { if(b) draw_view_ = false; draw_z_ = b; }
  inline void set_view(bool b) {
    if(b) {
      draw_x_=false; draw_y_=false; draw_z_=false;
    }
    draw_view_ = b;
  }
  inline void set_level_outline( bool b ) { draw_level_outline_ = b;}
  void set_outline_colors( vector< Color >& colors );

  bool draw_x() const { return draw_x_; }
  bool draw_y() const { return draw_y_; }
  bool draw_z() const { return draw_z_; }
  bool draw_view() const { return draw_view_; }
  bool draw_phi_0() const { return draw_phi0_; }
  bool draw_phi_1() const { return draw_phi1_; }
  double phi0() const { return phi0_; }
  double phi1() const { return phi1_; }
  bool draw_cyl() const { return draw_cyl_; }
  bool draw_level_outline () const { return draw_level_outline_; }
  void set_cylindrical(bool cyl_active, bool draw_phi0, double phi0, 
		       bool draw_phi1, double phi1) {
    draw_cyl_ = cyl_active;
    draw_phi0_ = draw_phi0;
    phi0_ = phi0;
    draw_phi1_ = draw_phi1;
    phi1_ = phi1;
  }

  virtual void draw(double time);
  virtual void draw_slice();
  virtual void draw_wireframe();
  void multi_level_draw();

  void set_draw_level( int i, bool b) { draw_level_[i] = b; }

protected:

  void draw_level_outline(vector<float>& vertex, vector<int>& poly,
                          bool use_fog, int color_index, 
                          FragmentProgramARB* shader);
  
  Point control_point_;
  bool draw_x_;
  bool draw_y_;
  bool draw_z_;
  bool draw_view_;
  bool draw_phi0_;
  double phi0_;
  bool draw_phi1_;
  double phi1_;
  bool draw_cyl_;
  bool draw_level_outline_;
  vector< bool > draw_level_;
  vector< Color > outline_colors_;
};

} // end namespace SLIVR

#endif // SliceRenderer_h
