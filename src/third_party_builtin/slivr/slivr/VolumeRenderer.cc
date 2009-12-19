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
//    File   : VolumeRenderer.cc
//    Author : Milan Ikits
//    Date   : Thu Jul  8 00:04:15 2004

#include <slivr/gldefs.h>
#include <slivr/VolumeRenderer.h>
#include <slivr/VolShader.h>
#include <slivr/ShaderProgramARB.h>
#include <slivr/TextureBrick.h>
#include <string>
#include <iostream>

using std::cerr;
using std::endl;
using std::string;

namespace SLIVR {

#ifdef _WIN32
#undef min
#undef max
#endif

struct cmap_data {
public:
  cmap_data() : tex_id_(0), dirty_(true), alpha_dirty_(true) {}
  vector<float>  data_;
  unsigned int tex_id_;
  bool dirty_;
  bool alpha_dirty_;
};

VolumeRenderer::VolumeRenderer(Texture* tex,
                               ColorMap* cmap1,
                               const vector<ColorMap2*> &cmap2,
                               const vector<Plane*> &planes,
                               int tex_mem) :
  TextureRenderer(tex, cmap1, cmap2, tex_mem),
  grange_(1.0),
  goffset_(0.0),
  shading_(false),
  ambient_(0.5),
  diffuse_(0.5),
  specular_(0.0),
  shine_(30.0),
  light_(0),
  adaptive_(true),
  draw_level_(20),
  level_alpha_(20),
  planes_(planes)
{
  mode_ = MODE_OVER;
  vector<bool>::iterator it = draw_level_.begin();
  for(;it != draw_level_.end(); ++it){
    (*it) = false;
  }
  vector< double >::iterator it2 = level_alpha_.begin();
  for(;it2 != level_alpha_.end(); ++it2){
    (*it2) = 0;
  }
}

VolumeRenderer::VolumeRenderer(const VolumeRenderer& copy):
  TextureRenderer(copy),
  shading_(copy.shading_),
  ambient_(copy.ambient_),
  diffuse_(copy.diffuse_),
  specular_(copy.specular_),
  shine_(copy.shine_),
  light_(copy.light_),
  adaptive_(copy.adaptive_),
  draw_level_(copy.draw_level_),
  level_alpha_(copy.level_alpha_),
  planes_(copy.planes_)
{
}


VolumeRenderer::~VolumeRenderer()
{
}


void
VolumeRenderer::set_mode(RenderMode mode)
{
  if(mode_ != mode) {
    mode_ = mode;
    alpha_dirty_ = true;
  }
}


void
VolumeRenderer::set_sampling_rate(double rate)
{
  if(sampling_rate_ != rate) {
    sampling_rate_ = rate;
    alpha_dirty_ = true;
  }
}


void
VolumeRenderer::set_interactive_rate(double rate)
{
  if(irate_ != rate) {
    irate_ = rate;
    alpha_dirty_ = true;
  }
}


void
VolumeRenderer::set_interactive_mode(bool mode)
{
  if(imode_ != mode) {
    imode_ = mode;
    alpha_dirty_ = true;
  }
}


void
VolumeRenderer::set_adaptive(bool b)
{
  adaptive_ = b;
}

void 
VolumeRenderer::set_planes(const vector<Plane*> &p)
{
  planes_ = p;
}

void
VolumeRenderer::draw(bool draw_wireframe_p, bool interactive_mode_p,
		     bool orthographic_p)
{
  if(draw_wireframe_p) {
    draw_wireframe(orthographic_p);
  } else  {
    draw_volume(interactive_mode_p, orthographic_p);
  }
}


void
VolumeRenderer::draw_volume(bool interactive_mode_p, bool orthographic_p)
{
  CHECK_OPENGL_ERROR();
  Ray view_ray = compute_view();
  double length = view_ray.direction().length2();
  if (!(length > 0)) return;

  const int levels = tex_->nlevels();

  vector<vector<TextureBrick*> > bricks(levels);
  int total_brick_size = 0;
  int firstlevel = -1;
  for (int i = 0; i < levels; i++)
  {
    tex_->get_sorted_bricks(bricks[i], view_ray, 
			    levels - i - 1, orthographic_p);
    total_brick_size += bricks[i].size();
    if (firstlevel < 0 && bricks[i].size()) { firstlevel = i; }
  }
  if (total_brick_size == 0) {
    //tex_->unlock_bricks();
    return;
  }

  bool cmap2_updating = false;
  for (unsigned int c = 0; c < cmap2_.size(); ++c)
    if (cmap2_[c]->updating()) {
      cmap2_updating = true;
      break;
    }

  set_interactive_mode(adaptive_ && (cmap2_updating || interactive_mode_p));
  CHECK_OPENGL_ERROR();
  // Set sampling rate based on interaction.
  const double rate = imode_ ? irate_ : sampling_rate_;
  const Vector diag = tex_->bbox().diagonal();
  const Vector cell_diag(diag.x() / (tex_->nx() * pow(2.0, levels-1)),
                         diag.y() / (tex_->ny() * pow(2.0, levels-1)),
                         diag.z() / (tex_->nz() * pow(2.0, levels-1)));
  const double dt = cell_diag.length()/rate;
  const int num_slices = (int)(diag.length()/dt);

  vector<float> vertex;
  vector<float> texcoord;
  vector<int> size;
  vector<int> mask;
  vertex.reserve(num_slices*6);
  texcoord.reserve(num_slices*6);
  size.reserve(num_slices*6);
  mask.reserve(num_slices*6);

  //--------------------------------------------------------------------------

  const int nb0 = bricks[firstlevel][0]->nb(0);
  const bool use_cmap1 = cmap1_;
  const bool use_cmap2 =
    cmap2_.size() && ShaderProgramARB::shaders_supported();
  if(!use_cmap1 && !use_cmap2) 
  {
    CHECK_OPENGL_ERROR();
    return;
  }

  CHECK_OPENGL_ERROR();
  const bool use_shading = shading_;
  const GLboolean use_fog = glIsEnabled(GL_FOG);
  GLfloat light_pos[4];
  glGetLightfv(GL_LIGHT0+light_, GL_POSITION, light_pos);
  GLfloat clear_color[4];
  glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_color);
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);

  //--------------------------------------------------------------------------
  // set up blending

  glEnable(GL_BLEND);
  switch(mode_) {
  case MODE_OVER:
#ifndef _WIN32
#if 1
    // VisIt Change
    if(GLEW_ARB_imaging)
#else
    if(gluCheckExtension((GLubyte*)"GL_ARB_imaging",
			 glGetString(GL_EXTENSIONS)))
#endif
#else
      if (glBlendEquation)
#endif
        glBlendEquation(GL_FUNC_ADD);
    
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    break;
  case MODE_MIP:
#ifndef _WIN32
#if 1
    // VisIt Change
    if(GLEW_ARB_imaging)
#else
    if(gluCheckExtension((GLubyte*)"GL_ARB_imaging",
			 glGetString(GL_EXTENSIONS)))
#endif
#else
      if (glBlendEquation)
#endif
        glBlendEquation(GL_MAX);
    glBlendFunc(GL_ONE, GL_ONE);
    break;
  default:
    break;
  }

  // Cache this value to reset, in case another framebuffer is active,
  // as it is in the case of saving an image from the viewer.
  GLint cur_framebuffer_id;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &cur_framebuffer_id);
  GLint cur_draw_buffer;
  glGetIntegerv(GL_DRAW_BUFFER, &cur_draw_buffer);
  GLint cur_read_buffer;
  glGetIntegerv(GL_READ_BUFFER, &cur_read_buffer);

  //   GLint max_attachments;
  //   glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &max_attachments);
  //   cerr << max_attachments << endl;

  CHECK_OPENGL_ERROR();
  if(blend_num_bits_ > 8) {  
    CHECK_OPENGL_ERROR();
    int w = vp[2];
    int h = vp[3];

    if (blend_framebuffer_ == 0) {
      glEnable(GL_TEXTURE_RECTANGLE_ARB);
      GLuint depth_rb;
      glGenFramebuffersEXT(1, &blend_framebuffer_);
      glGenTextures(1, &blend_tex_id_);
      glGenRenderbuffersEXT(1, &depth_rb);
      
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, blend_framebuffer_);

      // Initialize texture color renderbuffer
      glBindTexture(GL_TEXTURE_RECTANGLE_ARB, blend_tex_id_);
      glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
      CHECK_OPENGL_ERROR();

      // Ignoring the 32 bit float buffer for now, 
      // rendering to it is currently unusably slow. May, 2007
      // To test it just switch GL_RGBA16F_ARB to GL_RGBA32F_ARB.
      glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA16F_ARB, w, h, 0,
		     GL_RGBA, GL_FLOAT, NULL);

      CHECK_OPENGL_ERROR();
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT,
				GL_TEXTURE_RECTANGLE_ARB, blend_tex_id_, 0);
      CHECK_OPENGL_ERROR();

      // Initialize depth renderbuffer
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_rb);
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
			       GL_DEPTH_COMPONENT24, w, h);
      glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
				   GL_DEPTH_ATTACHMENT_EXT,
				   GL_RENDERBUFFER_EXT, depth_rb);
    }

    CHECK_OPENGL_ERROR();

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
    glDisable(GL_TEXTURE_RECTANGLE_ARB);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, blend_framebuffer_);


    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT); 

    // Should be complete.
    CHECK_FRAMEBUFFER_STATUS();

    // Copy the fog state to the new context.
    GLfloat fstart, fend, fcolor[4];
    if (use_fog)
    {
      glGetFloatv(GL_FOG_START, &fstart);
      glGetFloatv(GL_FOG_END, &fend);
      glGetFloatv(GL_FOG_COLOR, fcolor);
    }

    CHECK_OPENGL_ERROR();
    float* cc = clear_color;
    glClearColor(cc[0], cc[1], cc[2], cc[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    CHECK_OPENGL_ERROR();
    if (use_fog)
    {
      glFogi(GL_FOG_MODE, GL_LINEAR);
      glFogf(GL_FOG_START, fstart);
      glFogf(GL_FOG_END, fend);
      glFogfv(GL_FOG_COLOR, fcolor);
    }
    CHECK_OPENGL_ERROR();
  }
  CHECK_OPENGL_ERROR();
  glColor4f(1.0, 1.0, 1.0, 1.0);
  glDepthMask(GL_FALSE);

  //--------------------------------------------------------------------------
  // Load colormap texture into texture unit 2, rebuild if needed.
  vector< cmap_data * > cmaps;
  if (use_cmap2)
  {
    build_colormap2();
    bind_colormap2();
  }
  else
  {
    if (levels > 2 && mode_ == MODE_OVER)
    {
      int i;
      for (i = 0; i < levels; i++ ) {
        cmaps.push_back( new cmap_data );
      }
      for (int i = 0; i < levels; ++i )
      {
        cmaps.push_back( new cmap_data );
        build_colormap1( cmaps[levels - i - 1]->data_,
                         cmaps[levels - i - 1]->tex_id_,
                         cmaps[levels - i - 1]->dirty_,
                         cmaps[levels - i - 1]->alpha_dirty_,
                         double( invert_opacity_  ?
                                 tan(1.570796327 *
                                     (0.5 - level_alpha_[levels - i - 1])*
                                     0.49999) : i ));;
      }
    }
    else
    {
      build_colormap1(cmap1_array_, cmap1_tex_, cmap1_dirty_, alpha_dirty_);
      bind_colormap1(cmap1_array_, cmap1_tex_);
    }
  }
  CHECK_OPENGL_ERROR();
  //--------------------------------------------------------------------------
  // enable data texture unit 0
#if defined(GL_ARB_fragment_program) || defined(GL_ATI_fragment_shader)
#ifdef _WIN32
  if (glActiveTexture)
#endif
  glActiveTexture(GL_TEXTURE0_ARB);
#endif
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glEnable(GL_TEXTURE_3D);

  //--------------------------------------------------------------------------
  // Set up shaders
  FragmentProgramARB* shader = 0;
  int blend_mode = 0;
  shader = vol_shader_factory_->shader(use_cmap2 ? 2 : 1, nb0, tex_->nc(),
                                       use_shading, false,
                                       use_fog, blend_mode, cmap2_.size());
  if (shader)
  {
    if (!shader->valid())
    {
      shader->create();
    }
    shader->bind();
  }
  CHECK_OPENGL_ERROR();
  if (use_shading)
  {
    // set shader parameters
    double mat[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, mat);
    Transform mv;
    mv.set_trans(mat);
    const Transform &t = tex_->transform();
    Vector light(light_pos[0], light_pos[1], light_pos[2]);
    light = mv.unproject(light);
    light = t.unproject(light);
    light.safe_normalize();
    shader->setLocalParam(0, light.x(), light.y(), light.z(), 1.0);
    shader->setLocalParam(1, ambient_, diffuse_, specular_, shine_);
  }
  CHECK_OPENGL_ERROR();

  //-------------------------------------------------------------------------
  // Set up stenciling
  if (use_stencil_)
  {
    glStencilMask(1);
    glStencilFunc(GL_EQUAL, 0, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glEnable(GL_STENCIL_TEST);
  }
  CHECK_OPENGL_ERROR();
  //--------------------------------------------------------------------------
  // render bricks

  // Set up transform
  const Transform &tform = tex_->transform();
  double mvmat[16];
  tform.get_trans(mvmat);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glMultMatrixd(mvmat);

  if (use_cmap2)
  {
    const float cm2scale = 1.0 / Pow2(cmap2_.size());
    shader->setLocalParam(2, grange_, goffset_, cm2scale, cm2scale);
  }
  else
  {
    shader->setLocalParam(2, 1, 0, 0, 0);
  }
  CHECK_OPENGL_ERROR();
  if (levels == 1)
  {
#ifdef __APPLE__
    // Blend mode for no texture palette support.
    if (!ShaderProgramARB::shaders_supported() && mode_ == MODE_OVER)
    {
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // Scale slice opacity (from build_colormap1)
      const double level_exponent = 0.0;  // used for multi-layer code
      const double bp = tan(1.570796327 * (0.5 - slice_alpha_ * 0.49999));
      double alpha = pow(0.5, bp); // 0.5 as default global cmap alpha
      alpha = 1.0 - pow((1.0 - alpha), imode_ ?
                        1.0/irate_/pow(2.0, level_exponent) :
                        1.0/sampling_rate_/pow(2.0, level_exponent) );
      glColor4f(1.0, 1.0, 1.0, alpha);
    }
#endif

    vector<TextureBrick*>& bs  = bricks[0];
    for (unsigned int i=0; i < bs.size(); i++) {
      TextureBrick* b = bs[i];
      if (!test_against_view(b->bbox())) continue; // Clip against view.
      vertex.clear();
      texcoord.clear();
      mask.clear();
      size.clear();
      b->compute_polygons(view_ray, dt, vertex, texcoord, size);
      b->mask_polygons(size, vertex, texcoord, mask, planes_);
      if (vertex.size() == 0) { continue; }
      load_brick(bs, i, use_cmap2);
      shader->setLocalParam(4, 1.0/b->nx(), 1.0/b->ny(), 1.0/b->nz(), 0.0);
      draw_polygons(vertex, texcoord, size, false, use_fog,
                    &mask, shader);
    }
  }
  else
  {
    Point corner[8];
    BBox bbox = tex_->bbox();
    corner[0] = bbox.min();
    corner[1] = Point(bbox.min().x(), bbox.min().y(), bbox.max().z());
    corner[2] = Point(bbox.min().x(), bbox.max().y(), bbox.min().z());
    corner[3] = Point(bbox.min().x(), bbox.max().y(), bbox.max().z());
    corner[4] = Point(bbox.max().x(), bbox.min().y(), bbox.min().z());
    corner[5] = Point(bbox.max().x(), bbox.min().y(), bbox.max().z());
    corner[6] = Point(bbox.max().x(), bbox.max().y(), bbox.min().z());
    corner[7] = bbox.max();
    double ts[8];
    for(unsigned int i = 0; i < 8; i++) {
      ts[i] = Dot(corner[i]-view_ray.origin(), view_ray.direction());
    }
    Sort(ts, 8);
    double tmin = (floor(ts[0]/dt) + 1)*dt;
    double tmax = floor(ts[7]/dt)*dt;
    int count = 1;
    int reset_val = (int)(pow(2.0, levels - 1));

    for (double t = tmax ; t >= tmin; t -= dt)
    {
      if (use_stencil_)
      {
        glStencilMask(~0);
        glClear(GL_STENCIL_BUFFER_BIT);
        glStencilMask(1);
      }
    
      for (int j = 0; j < levels; ++j )
      {
        if ( !draw_level_[j] ) continue;
        if ( j > 0 ){
          bool go_on = false;
          int k = j;
          while( k < levels ){
            int draw_level = int(pow(2.0, k));
            if ( count < draw_level )
            {
              break;
            }
            else if( count == draw_level )
            {
              go_on = true;
              break;
            }
            else
            {
              ++k;
            }
          }

          if ( !go_on )
          {
            break;
          }
        }

        if (levels > 2 && mode_ == MODE_OVER)
        {
          bind_colormap1( cmaps[j]->data_, cmaps[j]->tex_id_ );
        }

#ifdef __APPLE__
        // Blend mode for no texture palette support.
        if (!ShaderProgramARB::shaders_supported() && mode_ == MODE_OVER)
        {
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

          // Scale slice opacity (from build_colormap1)
          const double level_exponent = invert_opacity_  ?
            tan(1.570796327 * (0.5 - level_alpha_[levels - j - 1]) * 0.49999) :
            (double)j;
          const double bp = tan(1.570796327 * (0.5 - slice_alpha_ * 0.49999));
          double alpha = pow(0.5, bp); // 0.5 as default global cmap alpha
          alpha = 1.0 - pow((1.0 - alpha), imode_ ?
                            1.0/irate_/pow(2.0, level_exponent) :
                            1.0/sampling_rate_/pow(2.0, level_exponent) );
          glColor4f(1.0, 1.0, 1.0, alpha);
        }
#endif

        vector<TextureBrick*>& bs  = bricks[j];
        for(unsigned int i = 0; i < bs.size(); i++) {
          TextureBrick* b = bs[i];
          vertex.clear();
          texcoord.clear();
          mask.clear();
          size.clear();
          b->compute_polygon( view_ray, t, vertex, texcoord, size);
          b->mask_polygons(size, vertex, texcoord, mask, planes_);
          if (vertex.size() == 0) { continue; }
          load_brick(bs, i, use_cmap2);
          shader->setLocalParam(4, 1.0/b->nx(), 1.0/b->ny(), 1.0/b->nz(), 0.0);
          draw_polygons(vertex, texcoord, size, false, use_fog,
                        &mask, shader);
        }

        if (levels > 2 && mode_ == MODE_OVER)
        {
          release_colormap1();
        }
      }
      if( count == reset_val ) count = 0;
      ++count;
    }
  }

  // Undo transform.
  glPopMatrix();
  CHECK_OPENGL_ERROR();

  // Turn off stenciling.
  if (use_stencil_)
  {
    glDisable(GL_STENCIL_TEST);
  }

  glDepthMask(GL_TRUE);

  // Release shader.
  if (shader && shader->valid())
  {
    shader->release();
  }

  // Release textures.
  if (use_cmap2)
  {
    release_colormap2();
  }
  else
  {
    if (!(levels > 2 && mode_ == MODE_OVER))
    {
      release_colormap1();
    }
  }

#if defined(GL_ARB_fragment_program) || defined(GL_ATI_fragment_shader)
#ifdef _WIN32
  if (glActiveTexture)
#endif
  glActiveTexture(GL_TEXTURE0_ARB);
#endif
  glDisable(GL_TEXTURE_3D);
  glBindTexture(GL_TEXTURE_3D, 0);
  //--------------------------------------------------------------------------

  // Reset the blend functions after MIP
#ifndef _WIN32
#if 1
  // VisIt Change
  if(GLEW_ARB_imaging)
#else
  if(gluCheckExtension((GLubyte*)"GL_ARB_imaging",glGetString(GL_EXTENSIONS)))
#endif
#else
  if (glBlendEquation)
#endif
    glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  glDisable(GL_BLEND);
  if (blend_num_bits_ > 8)
  {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, cur_framebuffer_id); 
    glDrawBuffer(cur_draw_buffer);
    glReadBuffer(cur_read_buffer);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(-1.0, -1.0, 0.0);
    glScalef(2.0, 2.0, 2.0);

    GLboolean depth_test = glIsEnabled(GL_DEPTH_TEST);
    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    GLboolean cull_face = glIsEnabled(GL_CULL_FACE);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
#if defined(GL_ARB_fragment_program) || defined(GL_ATI_fragment_shader)
#  ifdef _WIN32
    if (glActiveTexture)
#  endif
      glActiveTexture(GL_TEXTURE0_ARB);
#endif
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, blend_tex_id_);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glBegin(GL_QUADS);
    {
      glTexCoord2f(0.0,  0.0);
      glVertex2f( 0.0,  0.0);
      glTexCoord2f(vp[2],  0.0);
      glVertex2f( 1.0,  0.0);
      glTexCoord2f(vp[2],  vp[3]);
      glVertex2f( 1.0,  1.0);
      glTexCoord2f( 0.0,  vp[3]);
      glVertex2f( 0.0,  1.0);
    }
    glEnd();

    if (depth_test) glEnable(GL_DEPTH_TEST);
    if (lighting) glEnable(GL_LIGHTING);
    if (cull_face) glEnable(GL_CULL_FACE);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
    glDisable(GL_TEXTURE_RECTANGLE_ARB);
  }

  for (unsigned int i = 0; i < cmaps.size(); i++)
  {
    delete cmaps[i];
  }

  CHECK_OPENGL_ERROR();
}


void
VolumeRenderer::draw_wireframe(bool orthographic_p)
{
  Ray view_ray = compute_view();
  const Transform &tform = tex_->transform();
  double mvmat[16];
  tform.get_trans(mvmat);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glMultMatrixd(mvmat);
  glEnable(GL_DEPTH_TEST);
  GLboolean lighting = glIsEnabled(GL_LIGHTING);
  glDisable(GL_LIGHTING);
  vector<TextureBrick*> bricks;
  tex_->get_sorted_bricks(bricks, view_ray, 0, orthographic_p);

  const double rate = imode_ ? irate_ : sampling_rate_;
  const Vector diag = tex_->bbox().diagonal();
  const Vector cell_diag(diag.x()/tex_->nx(),
                         diag.y()/tex_->ny(),
                         diag.z()/tex_->nz());
  const double dt = cell_diag.length()/rate;
  const int num_slices = (int)(diag.length()/dt);

  vector<float> vertex;
  vector<float> texcoord;
  vector<int> size;
  vertex.reserve(num_slices*6);
  texcoord.reserve(num_slices*6);
  size.reserve(num_slices*6);

  for (unsigned int i=0; i<bricks.size(); i++)
  {
    glColor4f(0.8*(i+1.0)/bricks.size(), 0.8*(i+1.0)/bricks.size(), 0.8, 1.0);

    TextureBrick* b = bricks[i];
    if (!test_against_view(b->bbox())) continue;

    const Point &pmin(b->bbox().min());
    const Point &pmax(b->bbox().max());
    Point corner[8];
    corner[0] = pmin;
    corner[1] = Point(pmin.x(), pmin.y(), pmax.z());
    corner[2] = Point(pmin.x(), pmax.y(), pmin.z());
    corner[3] = Point(pmin.x(), pmax.y(), pmax.z());
    corner[4] = Point(pmax.x(), pmin.y(), pmin.z());
    corner[5] = Point(pmax.x(), pmin.y(), pmax.z());
    corner[6] = Point(pmax.x(), pmax.y(), pmin.z());
    corner[7] = pmax;

    glBegin(GL_LINES);
    {
      for(int i=0; i<4; i++) {
        glVertex3d(corner[i].x(), corner[i].y(), corner[i].z());
        glVertex3d(corner[i+4].x(), corner[i+4].y(), corner[i+4].z());
      }
    }
    glEnd();
    glBegin(GL_LINE_LOOP);
    {
      glVertex3d(corner[0].x(), corner[0].y(), corner[0].z());
      glVertex3d(corner[1].x(), corner[1].y(), corner[1].z());
      glVertex3d(corner[3].x(), corner[3].y(), corner[3].z());
      glVertex3d(corner[2].x(), corner[2].y(), corner[2].z());
    }
    glEnd();
    glBegin(GL_LINE_LOOP);
    {
      glVertex3d(corner[4].x(), corner[4].y(), corner[4].z());
      glVertex3d(corner[5].x(), corner[5].y(), corner[5].z());
      glVertex3d(corner[7].x(), corner[7].y(), corner[7].z());
      glVertex3d(corner[6].x(), corner[6].y(), corner[6].z());
    }
    glEnd();

    glColor4f(0.4, 0.4, 0.4, 1.0);

    vertex.clear();
    texcoord.clear();
    size.clear();

    // Scale out dt such that the slices are artificially further apart.
    b->compute_polygons(view_ray, dt * 10, vertex, texcoord, size);
    vector<int> mask;
    b->mask_polygons(size, vertex, texcoord, mask, planes_);

    draw_polygons_wireframe(vertex, texcoord, size, false, false, &mask);
  }
  if(lighting) glEnable(GL_LIGHTING);
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}


double
VolumeRenderer::num_slices_to_rate(int num_slices)
{
  const Vector diag = tex_->bbox().diagonal();
  const Vector cell_diag(diag.x()/tex_->nx(),
                         diag.y()/tex_->ny(),
                         diag.z()/tex_->nz());
  const double dt = diag.length() / num_slices;
  const double rate = cell_diag.length() / dt;

  return rate;
}


void
VolumeRenderer::set_gradient_range(double min, double max)
{
  double range = max-min;
  if (fabs(range) < 0.001) {
    grange_ = 1.0;
    goffset_ = 1.0;
  } else {
    grange_ = 1/(max-min);
    goffset_ = -min/(max-min);
  }
}



} // namespace SLIVR
