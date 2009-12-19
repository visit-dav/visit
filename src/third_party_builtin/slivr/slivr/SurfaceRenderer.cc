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
//    File   : SurfaceRenderer.cc
//    Author : Michael Callahan
//    Date   : April 2008


#include <slivr/gldefs.h>
#include <slivr/SurfaceRenderer.h>
#include <slivr/VolShader.h>
#include <slivr/TextureBrick.h>
#include <slivr/ShaderProgramARB.h>

#include <iostream>
#include <string>

using std::cerr;
using std::endl;
using std::string;

namespace SLIVR {

#ifndef GL_TEXTURE_3D
#define GL_TEXTURE_3D 0x806F
#endif

#ifdef _WIN32
#undef min
#undef max
#endif

SurfaceRenderer::SurfaceRenderer(Texture* tex,
                                 ColorMap* cmap1, 
                                 vector<ColorMap2*> &cmap2,
                                 int tex_mem) : 
  TextureRenderer(tex, cmap1, cmap2, tex_mem)
{
  lighting_ = true;
  mode_ = MODE_SLICE;
}


SurfaceRenderer::SurfaceRenderer(const SurfaceRenderer& copy) : 
  TextureRenderer(copy),
  points_(copy.points_)
{
  lighting_ = true;
}


SurfaceRenderer::~SurfaceRenderer()
{
}

void
SurfaceRenderer::add_triangle(const Point &p0, const Point &p1,
                              const Point &p2)
{
  points_.push_back(p0.x());
  points_.push_back(p0.y());
  points_.push_back(p0.z());
  points_.push_back(p1.x());
  points_.push_back(p1.y());
  points_.push_back(p1.z());
  points_.push_back(p2.x());
  points_.push_back(p2.y());
  points_.push_back(p2.z());
  
  Vector dv1 = p1 - p0;
  Vector dv2 = p2 - p0;
  Vector n = Cross(dv1, dv2);
  n.normalize();
  normals_.push_back(n.x());
  normals_.push_back(n.y());
  normals_.push_back(n.z());
  normals_.push_back(n.x());
  normals_.push_back(n.y());
  normals_.push_back(n.z());
  normals_.push_back(n.x());
  normals_.push_back(n.y());
  normals_.push_back(n.z());
}

void
SurfaceRenderer::draw(double)
{
  draw_surface();
}


void
SurfaceRenderer::draw_surface()
{
  Ray view_ray = compute_view();
  vector<TextureBrick*> bricks;
  tex_->get_sorted_bricks(bricks, view_ray);
  if(bricks.size() == 0) return;

  //--------------------------------------------------------------------------

  const int nc = tex_->nc();
  const int nb0 = tex_->nb(0);
  const bool use_cmap1 = cmap1_ != 0;
  const bool use_cmap2 =
    cmap2_.size() && nc == 2 && ShaderProgramARB::shaders_supported();
  if (!use_cmap1 && !use_cmap2)
  {
    return;
  }
  GLboolean use_fog = glIsEnabled(GL_FOG);
  
  //--------------------------------------------------------------------------
  // load colormap texture
  // rebuild if needed
  if(use_cmap2) {
    build_colormap2();
    bind_colormap2();
  } else {
    build_colormap1(cmap1_array_, cmap1_tex_, cmap1_dirty_, alpha_dirty_);
    bind_colormap1(cmap1_array_, cmap1_tex_);
  }
  
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
  // enable alpha test
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.0);
  glColor4f(1.0, 1.0, 1.0, 1.0);
  glDepthMask(GL_TRUE);

  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  //--------------------------------------------------------------------------
  // set up shaders
  FragmentProgramARB* shader = 0;
  int blend_mode = 0;
  shader = vol_shader_factory_->shader(use_cmap2 ? 2 : 1, nb0, tex_->nc(),
                                       false, true,
                                       use_fog, blend_mode, cmap2_.size());

  if(shader) {
    if(!shader->valid()) {
      shader->create();
    }
    shader->bind();
  }

  //--------------------------------------------------------------------------
  // render bricks
  for (unsigned int i=0; i < bricks.size(); i++)
  {
    TextureBrick* b = bricks[i];
    if (!test_against_view(b->bbox())) continue; // Clip brick against view.

    load_brick(bricks, i, use_cmap2);

#if 0
    glVertexPointer(3, GL_FLOAT, 0, &(points_.front()));
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLES, 0, points_.size()/3);
#else
    const Transform &tform = tex_->transform();
    
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < points_.size(); i+=3)
    {
      const Point p(points_[i+0], points_[i+1], points_[i+2]);
      const Point q = tform.unproject(p);
      glMultiTexCoord3f(GL_TEXTURE0, q.x(), q.y(), q.z());
      //glTexCoord3f(q.x(), q.y(), q.z());
      glNormal3f(normals_[i+0], normals_[i+1], normals_[i+2]);
      glVertex3f(points_[i+0], points_[i+1], points_[i+2]);
    }
    glEnd();
#endif
  }
  
  //--------------------------------------------------------------------------
  // release shaders
  if (shader && shader->valid())
  {
    shader->release();
  }

  //--------------------------------------------------------------------------

  glDisable(GL_BLEND);
  
  glDisable(GL_ALPHA_TEST);
  glDepthMask(GL_TRUE);

  if(use_cmap2) {
    release_colormap2();
  } else {
    release_colormap1();
  }
#if defined(GL_ARB_fragment_program) || defined(GL_ATI_fragment_shader)
#ifdef _WIN32
  if (glActiveTexture)
#endif
    glActiveTexture(GL_TEXTURE0_ARB);
#endif
  glDisable(GL_TEXTURE_3D);
  glBindTexture(GL_TEXTURE_3D, 0);
}


void 
SurfaceRenderer::draw_wireframe()
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
  tex_->get_sorted_bricks(bricks, view_ray);

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

  }
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glColor4f(0.4, 0.4, 0.4, 1.0);
  for (size_t i = 0; i < points_.size(); i+=9)
  {
    glBegin(GL_LINE_LOOP);
    glVertex3f(points_[i+0], points_[i+1], points_[i+2]);
    glVertex3f(points_[i+3], points_[i+4], points_[i+5]);
    glVertex3f(points_[i+6], points_[i+7], points_[i+8]);
    glEnd();
  }

  if(lighting) glEnable(GL_LIGHTING);
}


} // namespace SLIVR
