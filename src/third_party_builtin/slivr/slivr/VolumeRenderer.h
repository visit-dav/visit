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
//    File   : VolumeRenderer.h
//    Author : Milan Ikits
//    Date   : Sat Jul 10 11:26:26 2004

#ifndef SLIVR_VolumeRenderer_h
#define SLIVR_VolumeRenderer_h

#include <slivr/Plane.h>
#include <slivr/Texture.h>
#include <slivr/TextureRenderer.h>
#include <slivr/slivr_share.h>


namespace SLIVR {

class VolShaderFactory;

class SLIVRSHARE VolumeRenderer : public TextureRenderer
{
public:
  VolumeRenderer(Texture* tex, 
		 ColorMap* cmap1, 
		 const vector<ColorMap2*> &cmap2,
		 const vector<Plane*> &planes,
                 int tex_mem);
  VolumeRenderer(const VolumeRenderer&);
  virtual ~VolumeRenderer();

  void set_mode(RenderMode mode);
  void set_sampling_rate(double rate);
  void set_interactive_rate(double irate);
  void set_interactive_mode(bool mode);
  void set_adaptive(bool b);
  void set_gradient_range(double min, double max);
  inline void set_shading(bool shading) { shading_ = shading; }
  inline void set_material(double amb, double diff, double spec, double shine)
  { ambient_ = amb; diffuse_ = diff; specular_ = spec; shine_ = shine; }
  inline void set_light(int light) { light_ = light; }
  
  virtual void draw(bool draw_wireframe_p, 
		    bool interactive_mode_p, 
		    bool orthographic_p = false);

  void draw_wireframe(bool orthographic_p = false);
  void draw_volume(bool interactive_mode_p, bool orthographic_p = false);

  double num_slices_to_rate(int slices);
  
  void set_draw_level( int i, bool b) { draw_level_[i] = b; }
  void set_level_alpha(int i, double v) { level_alpha_[i] = v; }
  
  void set_planes(const vector<Plane*> &p);

  bool get_shading() const { return shading_; }

protected:
  double grange_, goffset_;
  bool shading_;
  double ambient_, diffuse_, specular_, shine_;
  int light_;
  bool adaptive_;
  vector< bool > draw_level_;
  vector< double > level_alpha_;
  vector<Plane *> planes_;
};

} // End namespace SLIVR

#endif 
