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
//    File   : CM2Shader.cc
//    Author : Milan Ikits, Josh Stratton (changed to GLSL)
//    Date   : Tue Jul 13 02:27:42 2004

#include <slivr/ShaderProgramARB.h>
#include <slivr/CM2Shader.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace SLIVR {

#define CM2_TRIANGLE_BASE \
"uniform vec4 loc0;\n" \
"uniform vec4 loc1;\n" \
"uniform vec4 loc2;\n" \
"uniform vec4 loc3;\n" \
"\n" \
"void main()\n" \
"{\n" \
"   vec4 color = loc0;\n" \
"   vec4 geom0 = loc1; // {base, top_x, top_y, bottom_y} \n" \
"   vec4 geom1 = loc2; // {width, bottom, 0.0, 0.0} \n" \
"   vec4 sz = loc3; // {1/sx, 1/sy, 0.0, 0.0} \n" \
"   vec4 c, p, t;\n" \
"   p.xy = gl_FragCoord.xy * sz.xy;\n" \
"   t.x = geom0.z;\n" \
"   t.x = t.x - geom1.z;\n" \
"   p.z = geom1.y * t.x; // p.z = bottom y cutoff \n" \
"   p.z = p.y - p.z;\n" \
"   p.z = p.z - geom1.z;\n" \
"   if (p.z < 0.0) \n" \
"      discard;\n" \
"   t.z = 1.0 / t.x;\n" \
"   p.y = p.y - geom1.z;\n" \
"   t.x = p.y * t.z; // t.x = p.y / top_y \n" \
"   c.x = mix(geom0.x, geom0.y, t.x); \n" \
"   c.y = t.x * geom1.x; \n" \
"   c.y = c.y * 0.5; \n" \
"   c.y = 1.0 / c.y; \n" \
"   c.z = p.x - c.x; \n" \
"   c.z = c.y * c.z; \n" \
"   c.z = abs(c.z); \n" \
"   t.w = 1.0 - c.z; \n"

#define CM2_RECTANGLE_1D_BASE \
"uniform vec4 loc0;\n" \
"uniform vec4 loc1;\n" \
"uniform vec4 loc2;\n" \
"uniform vec4 loc3;\n" \
"\n" \
"void main()\n" \
"{\n" \
"   vec4 color = loc0;\n" \
"   vec4 geom0 = loc1; // {left_x, left_y, width, height} \n" \
"   vec4 geom1 = loc2; // {offset, 1/offset, 1/(1-offset), 0.0} \n" \
"   vec4 sz = loc3; // {1/sx, 1/sy, 0.0, 0.0} \n" \
"   vec4 c, p, t;\n" \
"   p.xy = gl_FragCoord.xy * sz.xy;\n" \
"   p.xy = p.xy - geom0.xy;\n" \
"   p.z = 1.0 / geom0.z;\n" \
"   p.w = 1.0 / geom0.w;\n" \
"   p.xy = p.xy * p.zw;\n" \
"   t.x = p.x - geom1.x;\n" \
"   t.y = t.x * geom1.y;\n" \
"   t.z = t.x * geom1.z;\n" \
"   t.w = (t.y < 0.0) ? t.y : t.z;\n" \
"   t.w = abs(t.w);\n" \
"   t.w = 1.0 - t.w;\n"

#define CM2_RECTANGLE_ELLIPSOID_BASE \
"!!ARBfp1.0 \n" \
"PARAM color = program.local[0]; \n" \
"TEMP c, p, t;"

#define CM2_ELLIPSOID_BASE \
"uniform vec4 loc0;\n" \
"uniform vec4 loc1;\n" \
"uniform vec4 loc2;\n" \
"uniform vec4 loc3;\n" \
"\n" \
"void main()\n" \
"{\n" \
"   vec4 color = gl_Color;\n" \
"   vec4 c;\n" \
"   vec4 t = vec4(pow(clamp(distance(gl_Color.xyz, vec3(0.0,0.0,0.0)), 0.0, 0.9), 0.3)); \n" \

#define CM2_PARABOLOID_BASE \
"uniform vec4 loc0;\n" \
"uniform vec4 loc1;\n" \
"uniform vec4 loc2;\n" \
"uniform vec4 loc3;\n" \
"\n" \
"void main()\n" \
"{\n" \
"   vec4 color = loc0;\n" \
"   vec4 geom0 = loc1; // { top_x, top_y, bottom_x, bottom_y} \n" \
"   vec4 geom1 = loc2; // { left_x, left_y, right_x, right_y} \n" \
"   vec4 c, t; \n" \
"   t = vec4(pow(clamp(gl_Color.r, 0.0, 0.9), 0.3)); \n" \

#define CM2_PAINT_BASE \
"!!ARBfp1.0 \n" \
"PARAM color = program.local[0]; \n" \
"PARAM sz = program.local[3]; # {1/sx, 1/sy, 0.0, 0.0} \n" \
"TEMP c, p, t; \n" \
"MUL p.xy, fragment.position.xyyy, sz.xyyy; \n" \
"MOV t.w, fragment.color.a; \n" \

#define CM2_TEXTURE_BASE \
"!!ARBfp1.0 \n" \
"PARAM sz = program.local[3]; # {1/sx, 1/sy, 0.0, 0.0} \n" \
"ATTRIB  cm_tc = fragment.texcoord[1]; \n" \
"TEMP c, p, t, color; \n" \
"MUL p.xy, fragment.position.xyyy, sz.xyyy; \n" \
"TEX color, cm_tc, texture[1], 1D; \n" \
"MOV t, color; \n" \

#define CM2_REGULAR \
"   c.w = color.w * t.w;\n" \
"   c.xyz = color.xyz;\n"

#define CM2_FAUX \
"   c = color * t.w;\n"

#define CM2_FLAT \
"c = color;\n"

#define CM2_RASTER_BLEND \
"   gl_FragColor = c;\n" \
"}\n"

#define CM2_FRAGMENT_BLEND_ATI \
"MUL p.xy, fragment.position.xyyy, program.local[4].xyyy; \n" \
"TEX t, p.xyyy, texture[0], 2D; \n" \
"SUB p.w, 1.0, c.w; \n" \
"MAD_SAT result.color, t, p.w, c; \n" \
"END"

#define CM2_FRAGMENT_BLEND_NV \
"TEX t, fragment.position.xyyy, texture[0], RECT; \n" \
"SUB p.w, 1.0, c.w; \n" \
"MAD_SAT result.color, t, p.w, c; \n" \
"END"

CM2Shader::CM2Shader(CM2ShaderType type, bool faux, CM2BlendType blend)
  : type_(type), shadeType_(CM2_SHADE_REGULAR), faux_(faux), blend_(blend),
    program_(0)
{}

CM2Shader::CM2Shader(CM2ShaderType type, int shade, bool faux, CM2BlendType blend)
  : type_(type), shadeType_(shade), faux_(faux), blend_(blend),
    program_(0)
{}

CM2Shader::~CM2Shader()
{
  delete program_;
}

bool
CM2Shader::create()
{
  string s;
  if(emit(s)) return true;
  program_ = new FragmentProgramARB(s);
  return false;
}

bool
CM2Shader::emit(string& s)
{
  ostringstream z;
  switch(type_) {
  case CM2_SHADER_TRIANGLE:
    z << CM2_TRIANGLE_BASE;
    break;
  case CM2_SHADER_RECTANGLE_1D:
    z << CM2_RECTANGLE_1D_BASE;
    break;
  case CM2_SHADER_RECTANGLE_ELLIPSOID:
    z << CM2_RECTANGLE_ELLIPSOID_BASE;
    break;
  case CM2_SHADER_ELLIPSOID:
    z << CM2_ELLIPSOID_BASE;
    break;
  case CM2_SHADER_PARABOLOID:
    z << CM2_PARABOLOID_BASE;
    break;
  case CM2_SHADER_PAINT:
    z << CM2_PAINT_BASE;
    break;
  case CM2_SHADER_TEXTURE:
    z << CM2_TEXTURE_BASE;
    break;

  default:
    break;
  }
  if(faux_ || type_ == CM2_SHADER_PAINT) {
    z << CM2_FAUX;
  } else {
    z << CM2_REGULAR;
  }

  switch(shadeType_) {
    case CM2_SHADE_REGULAR:
      break;   // Regular shading shouldn't need any changes.
    case CM2_SHADE_FLAT:
      z << CM2_FLAT;
      break;
    default:
      break;   // Do nothing for undefined cases.
  }

  switch(blend_) {
  case CM2_BLEND_RASTER:
    z << CM2_RASTER_BLEND;
    break;
  case CM2_BLEND_FRAGMENT_ATI:
    z << CM2_FRAGMENT_BLEND_ATI;
    break;
  case CM2_BLEND_FRAGMENT_NV:
    z << CM2_FRAGMENT_BLEND_NV;
    break;
  default:
    break;
  }

  
  s = z.str();
  return false;
}

CM2ShaderFactory::CM2ShaderFactory()
  : prev_shader_(-1)
{}

CM2ShaderFactory::~CM2ShaderFactory()
{
  for(unsigned int i=0; i<shader_.size(); i++) {
    delete shader_[i];
  }
}

void
CM2ShaderFactory::destroy()
{
  for(unsigned int i=0; i<shader_.size(); i++) {
    if(shader_[i]->program() && shader_[i]->program()->valid())
      shader_[i]->program()->destroy();
  }
}

FragmentProgramARB*
CM2ShaderFactory::shader(CM2ShaderType type, int shading, 
			 bool faux, CM2BlendType blend)
{
  if(prev_shader_ >= 0) {
    if(shader_[prev_shader_]->match(type, shading, faux, blend)) {
      return shader_[prev_shader_]->program();
    }
  }
  for(unsigned int i=0; i<shader_.size(); i++) {
    if(shader_[i]->match(type, shading, faux, blend)) {
      prev_shader_ = i;
      return shader_[i]->program();
    }
  }
  CM2Shader* s = new CM2Shader(type, shading, faux, blend);
  if(s->create()) {
    delete s;
    return 0;
  }
  shader_.push_back(s);
  prev_shader_ = shader_.size()-1;
  return s->program();
}

} // namespace SLIVR
