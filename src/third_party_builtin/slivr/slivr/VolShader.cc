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
//    File   : VolShader.cc
//    Author : Milan Ikits, Josh Stratton (changed to GLSL)
//    Date   : Tue Jul 13 02:28:09 2004

#include <string>
#include <sstream>
#include <iostream>
#include <slivr/gldefs.h>
#include <slivr/VolShader.h>
#include <slivr/ShaderProgramARB.h>

using std::string;
using std::vector;
using std::ostringstream;

namespace SLIVR {

#define VOL_UNIFORMS_1 \
"// VOL_UNIFORMS_1\n" \
"uniform vec4 loc0, loc1, loc2, loc3, loc4;\n" \
"uniform sampler3D tex0;\n" \
"uniform sampler3D tex1;\n" \
"uniform sampler1D tex2;\n" \
"\n"

#define VOL_UNIFORMS_2 \
"// VOL_UNIFORMS_2\n" \
"uniform vec4 loc0, loc1, loc2, loc3, loc4;\n" \
"uniform sampler3D tex0;\n" \
"uniform sampler3D tex1;\n" \
"uniform sampler2D tex2;\n" \
"\n"

#define VOL_HEAD \
"// VOL_HEAD\n" \
"void main()\n" \
"{\n" \
"   vec4 t = gl_TexCoord[0];\n" \
"   vec4 v;\n" \
"   vec4 c;\n"

#define VOL_TAIL \
"}\n"

#define VOL_VLUP_1_1 \
"   v = texture3D(tex0, t.stp); // VOL_VLUP_1_1\n"
#define VOL_VLUP_1_4 \
"   v.w = texture3D(tex0, t.stp).w; // VOL_VLUP_1_4\n"
#define VOL_VLUP_2_1 \
"   v = texture3D(tex0, t.stp); // VOL_VLUP_2_1\n"
#define VOL_VLUP_2_4 \
"   v.w = texture3D(tex0, t.stp).w; // VOL_VLUP_2_4\n"
#define VOL_GLUP_2_1 \
"   v.y = texture3D(tex1, t.stp).x; // VOL_GLUP_2_1\n"
#define VOL_GLUP_2_4 \
"   v.x = texture3D(tex1, t.stp).x; // VOL_GLUP_2_4\n"

#define VOL_TFLUP_1_1 \
"   c = texture1D(tex2, v.x); // VOL_TFLUP_1_1\n"
#define VOL_TFLUP_1_4 \
"   c = texture1D(tex2, v.w); // VOL_TFLUP_1_4\n"
#define VOL_TFLUP_2_1 \
"   c = texture2D(tex2, v.xy); // VOL_TFLUP_2_1\n"
#define VOL_TFLUP_2_4 \
"   c = texture2D(tex2, v.wx); // VOL_TFLUP_2_4\n"

#define VOL_TFLUP_MASK_HEAD \
"   vec4 mask = loc3; // VOL_TFLUP_MASK_HEAD\n" \
"   vec4 f, m, b;\n" \
"   c = vec4(0.0);\n" \
"   v.w = v.w * g.w;\n" \
"   m = mask;\n"

#define VOL_TFLUP_2_1_MASK \
"   b = texture2D(tex2, v.wx); // VOL_TFLUP_2_1_MASK\n" \
"   m = m * 0.5;\n" \
"   f = fract(m);\n" \
"   f = vec4(greaterThanEqual(f, vec4(0.5)));\n" \
"   c = clamp((b * f) + c,0.0,1.0);\n" \
"   v.w = v.w + g.w;\n"

#define VOL_FOG_HEAD \
"   vec4 fc = gl_Fog.color; // VOL_FOG_HEAD\n" \
"   vec4 fp;\n" \
"   fp.x = gl_Fog.density;\n" \
"   fp.y = gl_Fog.start;\n" \
"   fp.z = gl_Fog.end;\n" \
"   fp.w = gl_Fog.scale;\n" \
"   vec4 tf = gl_TexCoord[1];\n" \
"   vec4 fctmp;\n"

#define VOL_FOG_BODY \
"   v.x = fp.z - tf.x; // VOL_FOG_BODY\n" \
"   v.x = clamp(v.x * fp.w, 0.0, 1.0);\n" \
"   fctmp = c.w * fc;\n" \
"   c.xyz = mix(fctmp.xyzz, c.xyzz, v.x).xyz; \n"

#define VOL_FRAG_HEAD \
"   vec4 cf = gl_Color; // VOL_FRAG_HEAD\n"
#define VOL_FRAG_BODY \
"   c = c * cf; // VOL_FRAG_BODY\n"

#define VOL_LIT_HEAD \
"   vec4 l = loc0; // {lx, ly, lz, alpha} VOL_LIT_HEAD\n" \
"   vec4 k = loc1; // {ka, kd, ks, ns}\n" \
"   vec4 g = loc2; // {1/gradrange, -gradmin/gradrange, 0, 0}\n" \
"   vec4 n, w;\n"

#define VOL_LIT_BODY_NOGRAD \
"   n = (v * 2.0) - 1.0; // rescale from [0,1] to [-1, 1] VOL_LIT_BODY_NOGRAD \n" \
"   n.w = dot(n.xyz, n.xyz); // n.w = x*x + y*y + z*z \n" \
"   n.w = inversesqrt(n.w); // n.w = 1 / sqrt(x*x+y*y+z*z) \n" \
"   n = n * n.w; // n = n / length(normal)\n" \
"   n.w = dot(l.xyz, n.xyz); // calculate angle between light and normal. \n" \
"   n.w = clamp(abs(n.w), 0.0, 1.0); // two-sided lighting, n.w = abs(cos(angle))  \n" \
"   w = k; // w.x = weight*ka, w.y = weight*kd, w.z = weight*ks \n" \
"   w.x = k.x - w.y; // w.x = ka - kd*weight \n" \
"   w.x = w.x + k.y; // w.x = ka + kd - kd*weight \n" \
"   n.z = pow(n.w, k.w); // n.z = abs(cos(angle))^ns \n" \
"   n.w = (n.w * w.y) + w.x; // n.w = abs(cos(angle))*kd+ka\n" \
"   n.z = w.z * n.z; // n.z = weight*ks*abs(cos(angle))^ns \n"

#define VOL_LIT_BODY \
"   n = (v * 2.0) - 1.0; // rescale from [0,1] to [-1,1] VOL_LIT_BODY\n" \
"   n.w = dot(n.xyz, n.xyz); \n" \
"   n.w = inversesqrt(n.w); \n" \
"   n = n * n.w; // n = n / length(normal)\n" \
"   n.w = dot(l.xyz, n.xyz); // calculate angle between light and normal \n" \
"   n.w = clamp(abs(n.w), 0.0, 1.0); // two-sided lighting, n.w = abs(cos(angle))  \n" \
"   w.x = texture3D(tex1, t.stp); // get the gradient magnitude \n" \
"   w.xyzw = clamp((w.x * g.x) + g.y, 0.0, 1.0); // compute saturated weight based on current gradient \n" \
"   w = w * k; // w.x = weight*ka, w.y = weight*kd, w.z = weight*ks \n" \
"   w.x = k.x - w.y; // w.x = ka - kd*weight \n" \
"   w.x = w.x + k.y; // w.x = ka + kd - kd*weight \n" \
"   n.z = pow(n.w, k.w); // n.z = abs(cos(angle))^ns \n" \
"   n.w = (n.w * w.y) + w.x; // n.w = abs(cos(angle))*kd+ka\n" \
"   n.z = w.z * n.z; // n.z = weight*ks*abs(cos(angle))^ns \n"

#define VOL_LIT_END \
"   n.z = n.z * c.w; // VOL_LIT_END\n" \
"   c.xyz = (c.xyz * n.w) + n.z;\n" \

#define VOL_GRAD_COMPUTE_2_1 \
"   vec4 dir = loc4; // VOL_GRAD_COMPUTE_2_1\n" \
"   vec4 r, p; \n" \
"   mat4 tmat = gl_TextureMatrixInverseTranspose[0]; \n" \
"   v = vec4(v.x); \n" \
"   n = vec4(0.0); \n" \
"   w.x = dir.x; \n" \
"   p = clamp(gl_TexCoord[0] + w, 0.0, 1.0); \n" \
"   r = texture3D(tex0, p.stp); \n" \
"   n.x = r.x + n.x; \n" \
"   p = clamp(gl_TexCoord[0] - w, 0.0, 1.0); \n" \
"   r = texture3D(tex0, p.stp); \n" \
"   n.x = r.x - n.x; \n" \
"   w = vec4(0.0); \n" \
"   w.y = dir.y; \n" \
"   p = clamp(gl_TexCoord[0] + w, 0.0, 1.0); \n" \
"   r = texture3D(tex0, p.stp); \n" \
"   n.y = r.x + n.y; \n" \
"   p = clamp(gl_TexCoord[0] - w, 0.0, 1.0); \n" \
"   r = texture3D(tex0, p.stp); \n" \
"   n.y = r.x - n.y; \n" \
"   w = vec4(0.0); \n" \
"   w.z = dir.z; \n" \
"   p = clamp(gl_TexCoord[0] + w, 0.0, 1.0); \n" \
"   r = texture3D(tex0, p.stp); \n" \
"   n.z = r.x + n.z; \n" \
"   p = clamp(gl_TexCoord[0] - w, 0.0, 1.0); \n" \
"   r = texture3D(tex0, p.stp); \n" \
"   n.z = r.x - n.z; \n" \
"   w.x = dot(n.xxx, vec3(tmat[0].x, tmat[1].x, tmat[2].x)); \n" \
"   w.y = dot(n.yyy, vec3(tmat[0].y, tmat[1].y, tmat[2].y)); \n" \
"   w.z = dot(n.zzz, vec3(tmat[0].z, tmat[1].z, tmat[2].z)); \n" \
"   r = vec4(dot(w.xyz, w.xyz)); \n" \
"   p = vec4(inversesqrt(r.x)); \n" \
"   // the code below looks messy since the distance function \n" \
"   // in assembly and GLSL are quite different \n" \
"   vec4 tmp0 = r; \n" \
"   vec4 tmp1 = p; \n" \
"   p.x = 1.0; \n" \
"   p.y = tmp0.y * tmp1.y; \n" \
"   p.z = tmp0.z; \n" \
"   p.w = tmp1.w; \n" \
"   p.y = p.y * 1.75; \n" \
"   n.xyz = w.xyz * 1.0; \n //"

#define VOL_GRAD_COMPUTE_2_4 \
"   vec4 dir = loc4; \n" \
"   vec4 r, p; \n" \
"   mat4 tmat = gl_TextureMatrixInverseTranspose[0]; \n" \
"   v = vec4(v.w); \n" \
"   n = vec4(0.0); \n" \
"   w.x = dir.x; \n" \
"   p = clamp(gl_TexCoord[0] + w, 0.0, 1.0); \n" \
"   r = texture3D(tex0, p.stp); \n" \
"   n.x = r.w + n.x; \n" \
"   p = clamp(gl_TexCoord[0] - w, 0.0, 1.0); \n" \
"   r = texture3D(tex0, p.stp); \n" \
"   n.x = r.w - n.x; \n" \
"   w = vec4(0.0); \n" \
"   w.y = dir.y; \n" \
"   p = clamp(gl_TexCoord[0] + w, 0.0, 1.0); \n" \
"   r = texture3D(tex0, p.stp); \n" \
"   n.y = r.w + n.y; \n" \
"   p = clamp(gl_TexCoord[0] - w, 0.0, 1.0); \n" \
"   r = texture3D(tex0, p.stp); \n" \
"   n.y = r.w - n.y; \n" \
"   w = vec4(0.0); \n" \
"   w.z = dir.z; \n" \
"   p = clamp(gl_TexCoord[0] + w, 0.0, 1.0); \n" \
"   r = texture3D(tex0, p.stp); \n" \
"   n.z = r.w + n.z; \n" \
"   p = clamp(gl_TexCoord[0] - w, 0.0, 1.0); \n" \
"   r = texture3D(tex0, p.stp); \n" \
"   n.z = r.w - n.z; \n" \
"   w.x = dot(n.xxx, vec3(tmat[0].x, tmat[1].x, tmat[2].x)); \n" \
"   w.y = dot(n.yyy, vec3(tmat[0].y, tmat[1].y, tmat[2].y)); \n" \
"   w.z = dot(n.zzz, vec3(tmat[0].z, tmat[1].z, tmat[2].z)); \n" \
"   r = vec4(dot(w.xyz, w.xyz)); \n" \
"   p = vec4(inversesqrt(r.x)); \n" \
"   // the code below looks messy since the distance function \n" \
"   // in assembly and GLSL are quite different \n" \
"   vec4 tmp0 = r; \n" \
"   vec4 tmp1 = p; \n" \
"   p.x = 1.0; \n" \
"   p.y = tmp0.y * tmp1.y; \n" \
"   p.z = tmp0.z; \n" \
"   p.w = tmp1.w; \n" \
"   p.y = p.y * 1.75; \n" \
"   n.xyz = w.xyz * 1.0; \n //"

#define VOL_COMPUTED_GRADIENT_LOOKUP \
"   v.y = p.y; // VOL_COMPUTED_GRADIENT_LOOKUP\n"

#define VOL_FRAGMENT_BLEND_HEAD \
"   vec4 n; // VOL_FRAGMENT_BLEND_HEAD\n"

#define VOL_GRAD_COMPUTE_NOLIGHT_HEAD \
"   vec4 w; // VOL_GRAD_COMPUTE_NOLIGHT_HEAD\n"

#define VOL_FRAGMENT_BLEND_OVER_NV \
"TEX v, fragment.position.xyyy, texture[3], RECT; \n" \
"SUB n.w, 1.0, c.w; \n" \
"MAD result.color, v, n.w, c; \n"
#define VOL_FRAGMENT_BLEND_MIP_NV \
"TEX v, fragment.position.xyyy, texture[3], RECT; \n" \
"MAX result.color, v, c; \n"

#define VOL_FRAGMENT_BLEND_OVER_ATI \
"MUL n.xy, fragment.position.xyyy, program.local[2].xyyy;\n" \
"TEX v, n.xyyy, texture[3], 2D; \n" \
"SUB n.w, 1.0, c.w; \n" \
"MAD result.color, v, n.w, c; \n"
#define VOL_FRAGMENT_BLEND_MIP_ATI \
"MUL n.xy, fragment.position.xyyy, program.local[2].xyyy;\n" \
"TEX v, n.xyyy, texture[3], 2D; \n" \
"MAX result.color, v, c; \n"

#define VOL_RASTER_BLEND \
"   gl_FragColor = c; // VOL_RASTER_BLEND\n"

VolShader::VolShader(int dim, int vsize, int channels, bool shading, 
		     bool frag, bool fog, int blend, int cmaps)
  : dim_(dim), 
    vsize_(vsize),
    channels_(channels),
    shading_(shading),
    fog_(fog),
    blend_(blend),
    frag_(frag),
    num_cmaps_(cmaps),
    program_(0)
{}

VolShader::~VolShader()
{
  delete program_;
}

bool
VolShader::create()
{
  string s;
  if (emit(s)) return true;
  program_ = new FragmentProgramARB(s);
  return false;
}

bool
VolShader::emit(string& s)
{
  if (dim_!=1 && dim_!=2) return true;
  if (vsize_!=1 && vsize_!=4) return true;
  if (blend_!=0 && blend_!=1 && blend_!=2) return true;
  ostringstream z;

  // determine whether GL_TEXTURE2 is 1D or 2D and use appropriate uniforms
  if (dim_ == 1) {
    z << VOL_UNIFORMS_1;
  }
  else {
    z << VOL_UNIFORMS_2;
  }


  z << VOL_HEAD;

  // Set up light/blend variables and input parameters.
  if (shading_)
  {
    z << VOL_LIT_HEAD;
  }
  else if (blend_)
  {
    z << VOL_FRAGMENT_BLEND_HEAD;
  }

  if (frag_)
  {
    z << VOL_FRAG_HEAD;
  }
  
  // Set up fog variables and input parameters.
  if (fog_)
  {
    z << VOL_FOG_HEAD;
  }

  if (dim_ == 1)  // 1D colormap
  {
    // Get value
    z << VOL_VLUP_1_1;
    
    if (shading_)
    {
      if (vsize_ == 1)
      {
        // Compute the normal if needed and not there.
        z << VOL_GRAD_COMPUTE_2_1;
      }
      // Add the lighting.
      z << VOL_LIT_BODY_NOGRAD;
    }
    
    // Lookup the colormap entry for this value.
    if (vsize_ == 1)
    {
      z << VOL_TFLUP_1_1;
    }
    else
    {
      z << VOL_TFLUP_1_4;
    }

    // Apply the lighting.
    if (shading_)
    {
      z << VOL_LIT_END;
    }
  }
  else // dim_ == 2, 2D colormap
  {
    if (shading_)
    {
      if (vsize_ == 1)
      {
        z << VOL_VLUP_2_1;
        z << VOL_GRAD_COMPUTE_2_1;
        z << VOL_LIT_BODY_NOGRAD;
        if (channels_ == 1)
        {
          z << VOL_COMPUTED_GRADIENT_LOOKUP;
        }
        else
        {
          z << VOL_GLUP_2_1;
        }
        
        if (num_cmaps_ > 1)
        {
          z << VOL_TFLUP_MASK_HEAD;
          for (int n = 0; n < num_cmaps_; ++n)
          {
            z << VOL_TFLUP_2_1_MASK;
          }
        }
        else
        {
	  z << VOL_TFLUP_2_1;
        }
        z << VOL_LIT_END;
      }
      else // vsize_ == 4
      {
        z << VOL_VLUP_2_1;
        if (channels_ == 1)
        {
          z << VOL_GRAD_COMPUTE_2_4;
        }

        z << VOL_LIT_BODY_NOGRAD;

        if (channels_ == 1)
        {
          z << VOL_COMPUTED_GRADIENT_LOOKUP;
        }
        else
        {
          z << VOL_GLUP_2_4;
        }

        if (num_cmaps_ > 1)
        {
          z << VOL_TFLUP_MASK_HEAD;
          for (int n = 0; n < num_cmaps_; ++n)
          {
            z << VOL_TFLUP_2_1_MASK;
          }
        }
        else
        {
          if (channels_ == 1)
          {
            z << VOL_TFLUP_2_1;
          }
          else
          {
            z << VOL_TFLUP_2_4;
          }
        }
        z << VOL_LIT_END;
      }
    }
    else // No shading, 2D colormap.
    {
      if (vsize_ == 1)
      {
        z << VOL_VLUP_2_1;
        if (channels_ == 1)
        {
          // Compute Gradient magnitude and use it.
          if (!blend_) z << VOL_FRAGMENT_BLEND_HEAD;
          z << VOL_GRAD_COMPUTE_NOLIGHT_HEAD;
          z << VOL_GRAD_COMPUTE_2_1;
	  z << "\n";
          z << VOL_COMPUTED_GRADIENT_LOOKUP;
        }
        else
        {
          z << VOL_GLUP_2_1;
        }
        z << VOL_TFLUP_2_1;
      }
      else // vsize_ == 4
      {
        z << VOL_VLUP_2_4;
        if (channels_ == 1)
        {
          if (!blend_) z << VOL_FRAGMENT_BLEND_HEAD;
          z << VOL_GRAD_COMPUTE_NOLIGHT_HEAD;
          z << VOL_GRAD_COMPUTE_2_4;
	  z << "\n";
          z << VOL_COMPUTED_GRADIENT_LOOKUP;
          z << VOL_TFLUP_2_1; // look it up as if 2_1
        }
        else
        {
          z << VOL_GLUP_2_4;
          z << VOL_TFLUP_2_4;
        }
      }
    }
  }

  // frag
  if (frag_)
  {
    z << VOL_FRAG_BODY;
  }

  // fog
  if (fog_)
  {
    z << VOL_FOG_BODY;
  }

  // blend
  if (blend_ == 0)
  {
    z << VOL_RASTER_BLEND;
  }
  else if (blend_ == 1)
  {
    z << VOL_FRAGMENT_BLEND_OVER_NV;
  }
  else if (blend_ == 2)
  {
    z << VOL_FRAGMENT_BLEND_MIP_NV;
  }
  else if (blend_ == 3)
  {
    z << VOL_FRAGMENT_BLEND_OVER_ATI;
  }
  else if (blend_ == 4)
  {
    z << VOL_FRAGMENT_BLEND_MIP_ATI;
  }

  z << VOL_TAIL;

  s = z.str();
  //std::cerr << s << std::endl;
  return false;
}


VolShaderFactory::VolShaderFactory()
  : prev_shader_(-1)
{}

VolShaderFactory::~VolShaderFactory()
{
  for(unsigned int i=0; i<shader_.size(); i++) {
    delete shader_[i];
  }
}

FragmentProgramARB*
VolShaderFactory::shader(int dim, int vsize, int channels, bool shading, 
			 bool frag, bool fog, int blend, int cmaps)
{
  CHECK_OPENGL_ERROR();
  if(prev_shader_ >= 0) {
    if(shader_[prev_shader_]->match(dim, vsize, channels, shading, 
				    frag, fog, blend, cmaps)) 
    {
      CHECK_OPENGL_ERROR();
      return shader_[prev_shader_]->program();
    }
  }
  for(unsigned int i=0; i<shader_.size(); i++) {
    if(shader_[i]->match(dim, vsize, channels, shading, 
			 frag, fog, blend, cmaps)) 
    {
      prev_shader_ = i;
      CHECK_OPENGL_ERROR();
      return shader_[i]->program();
    }
  }

  VolShader* s = new VolShader(dim, vsize, channels, shading, 
			       frag, fog, blend, cmaps);
  if(s->create()) {
    delete s;
    CHECK_OPENGL_ERROR();
    return 0;
  }
  shader_.push_back(s);
  prev_shader_ = shader_.size()-1;
  CHECK_OPENGL_ERROR();
  return s->program();
}

} // end namespace SLIVR

