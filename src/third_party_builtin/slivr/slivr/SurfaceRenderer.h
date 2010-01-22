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
//    File   : SurfaceRenderer.h
//    Author : Michael Callahan
//    Date   : April 2008

#ifndef SLIVR_SurfaceRenderer_h
#define SLIVR_SurfaceRenderer_h

//#include <Core/Thread/Mutex.h>
//#include <Core/Geometry/Point.h>
//#include <Core/Geometry/Ray.h>
//#include <Core/Geometry/Vector.h>
//#include <Core/Geometry/Transform.h>
//#include <Core/Geom/ColorMap.h>
//#include <Core/Geom/GeomObj.h>

#include <slivr/Color.h>
#include <slivr/Texture.h>
#include <slivr/TextureRenderer.h>
#include <slivr/slivr_share.h>

namespace SLIVR {


class SLIVRSHARE SurfaceRenderer : public TextureRenderer
{
public:
  SurfaceRenderer(Texture* tex, 
                  ColorMap* cmap1, 
                  vector<ColorMap2*> &cmap2,
                  int tex_mem);
  SurfaceRenderer(const SurfaceRenderer&);
  virtual ~SurfaceRenderer();

  virtual void draw(double time);
  virtual void draw_surface();
  virtual void draw_wireframe();

  void clear_triangles() { points_.clear(); }
  void add_triangle(const Point &p0, const Point &p1, const Point &p2);

protected:
  vector<float> points_;
  vector<float> normals_;
};

} // end namespace SLIVR

#endif // SurfaceRenderer_h
