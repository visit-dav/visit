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
//    File   : TextureRenderer.h
//    Author : Milan Ikits
//    Date   : Wed Jul  7 23:34:33 2004

#ifndef TextureRenderer_h
#define TextureRenderer_h

#include <teem/nrrd.h>
#include <slivr/ColorMap.h>
#include <slivr/ColorMap2.h>
#include <slivr/CM2Shader.h>
#include <slivr/TextureBrick.h>
#include <slivr/Texture.h>
#include <slivr/share.h>

namespace SLIVR {

class FragmentProgramARB;
class VolShaderFactory;

typedef void (*PendingDeleteTextureCallback_type)(unsigned int);

class SLIVRSHARE TextureRenderer 
{
public:
  enum RenderMode { MODE_NONE, 
		    MODE_OVER, 
		    MODE_MIP, 
		    MODE_SLICE };

  TextureRenderer(Texture* tex, 
		  ColorMap* cmap1,
		  const vector<ColorMap2*> & cmap2,
                  int tex_mem);
  TextureRenderer(const TextureRenderer&);
  virtual ~TextureRenderer();

  void set_texture(Texture* tex);

  void set_colormap1(ColorMap* cmap1);

  void set_colormap2(const vector<ColorMap2*> &cmap2);
  void set_colormap2_dirty() { cmap2_dirty_ = true; }
  void set_colormap2_width(int size);
  void set_slice_alpha(double alpha);
  void set_blend_num_bits(int b);
  bool use_blend_buffer();
  void set_stencil(bool use){ use_stencil_ = use; }
  void invert_opacity(bool invert){ invert_opacity_ = invert; }
  inline void set_interp(bool i) { interp_ = i; }
 
  void draw(double time);
  void get_bounds(BBox& bb) const { tex_->get_bounds(bb); }

  bool get_lighting() const { return lighting_; }

  void set_pending_delete_texture_callback(PendingDeleteTextureCallback_type callback) { pending_delete_texture_callback_ = callback; }

protected:
  Texture *tex_;
  ColorMap *cmap1_;
  vector<ColorMap2*> cmap2_;
  bool cmap1_dirty_;
  bool cmap2_dirty_;
  bool alpha_dirty_;
  RenderMode mode_;
  bool interp_;
  bool lighting_;
  double sampling_rate_;
  double irate_;
  bool imode_;
  double slice_alpha_;

  int cmap2_width_;
  vector<float> cmap1_array_;
  unsigned int  cmap1_tex_;
  Nrrd* raster_array_;
  Nrrd* cmap2_array_;
  unsigned int cmap2_tex_;
  unsigned int tex_width_;
  GLuint            blend_framebuffer_;
  GLuint            blend_tex_id_;
  CM2ShaderFactory* shader_factory_;
  GLuint            cmap2_widget_framebuffer_;
  GLuint            cmap2_widget_tex_id_; // associated texture id
  GLuint            cmap2_framebuffer_;
  GLuint            cmap2_tex_id_; // associated texture id
  FragmentProgramARB* cmap2_shader_glsl_;
  VolShaderFactory* vol_shader_factory_;

  int blend_num_bits_;
  bool use_blend_buffer_;
  int free_tex_mem_;
  bool use_stencil_;
  bool invert_opacity_;
  bool clear_pool_;
  
  struct TexParam
  {
    int nx, ny, nz, nb;
    unsigned int id;
    TextureBrick *brick;
    int comp;
    GLenum textype;
    TexParam() : 
      nx(0), ny(0), nz(0), nb(0), 
      id(0), brick(0), comp(0), textype(GL_UNSIGNED_BYTE) {}
    TexParam(int x, int y, int z, int b, GLenum f, unsigned int i) : 
      nx(x), ny(y), nz(z), nb(b), id(i), brick(0), comp(0), textype(f) {}
  };
  vector<TexParam> tex_pool_;
  
  PendingDeleteTextureCallback_type pending_delete_texture_callback_;
  
  // Tests the bounding box against the current MODELVIEW and
  // PROJECTION matrices to determine if it is within the viewport.
  // Returns true if it is visible.
  bool test_against_view(const BBox &bbox);

  Ray compute_view();
  void load_brick(vector<TextureBrick*> &b, int i, bool use_cmap2);
  void draw_polygons(vector<float>& vertex, vector<float>& texcoord,
		     vector<int>& poly,
                     bool normal, bool fog, vector<int> *mask = 0, 
		     FragmentProgramARB *shader = 0);
  void draw_polygons_wireframe(vector<float>& vertex, vector<float>& texcoord,
			       vector<int>& poly,
			       bool normal, bool fog,
			       vector<int> *mask=0);

  void build_colormap1(vector<float> cmap_array,
		       unsigned int& cmap_tex, bool& cmap_dirty,
		       bool& alpha_dirty,  double level_exponent = 0.0);

  void build_colormap2();
  void colormap2_hardware_rasterize_setup();
  void colormap2_hardware_destroy_buffers();
  void colormap2_hardware_rasterize();


  void bind_colormap1(vector<float> cmap_array, unsigned int cmap_tex);
  void bind_colormap2();

  void release_colormap1();
  void release_colormap2();
  
  void clear_tex_pool();
};



} // end namespace SLIVR

#endif // TextureRenderer_h
