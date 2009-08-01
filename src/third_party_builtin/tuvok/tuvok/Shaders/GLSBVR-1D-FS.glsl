/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/**
 * \file    GLSBVR-1D-FS.glsl
 * \author  Jens Krueger
 *          SCI Institute
 *          University of Utah
 */
#define BIAS_SCALE 1

uniform sampler3D texVolume;  ///< the data volume
uniform sampler1D texTrans1D; ///< the 1D Transfer function
uniform float fTransScale;    ///< scale for 1D Transfer function lookup
uniform float fStepScale;     ///< opacity correction quotient
#ifdef BIAS_SCALE
  uniform float TFuncBias;    ///< bias amount for transfer func
#endif

/* bias and scale method for mapping a TF to a value. */
vec4 bias_scale(const float bias, const float scale)
{
  float vol_val = texture3D(texVolume, gl_TexCoord[0].xyz).x;
  vol_val = (vol_val + bias) / scale;

  return texture1D(texTrans1D, vol_val);
}

vec4 bit_width(const float tf_scale)
{
  float fVolumVal = texture3D(texVolume, gl_TexCoord[0].xyz).x;
  return texture1D(texTrans1D, fVolumVal * tf_scale);
}

void main(void)
{
#if defined(BIAS_SCALE)
  vec4 vTransVal = bias_scale(TFuncBias, fTransScale);
#else
  vec4 vTransVal = bit_width(fTransScale);
#endif

  // opacity correction
  vTransVal.a = 1.0 - pow(1.0 - vTransVal.a, fStepScale);

  // premultiply color with alpha
  vTransVal.xyz *= vTransVal.a;

	gl_FragColor = vTransVal;
}
