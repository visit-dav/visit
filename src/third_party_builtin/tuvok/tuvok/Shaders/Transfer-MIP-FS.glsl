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
  \file     Transfer-MIP-FS.glsl
  \author   Jens Krueger
            SCI Institute
            University of Utah
  \version  1.0
  \date     December 2008
*/

uniform sampler2D texLast;    ///< the maximum value from the previous passes
uniform sampler1D texTrans1D; ///< the 1D Transfer function
uniform float fTransScale;    ///< scale for 1D Transfer function lookup

void main(void){
  // fetch MIP value
  vec2 fVLastVal = texture2D(texLast,  gl_TexCoord[0].xy).xw;

  // apply 1D transfer function but ignore opacity
	if (fVLastVal.y > 0.5)  // this is a "very robust" float test for fVLastVal.y == 1 && fVLastVal.y != 0
    gl_FragColor = vec4(texture1D(texTrans1D, fVLastVal.x*fTransScale).rgb,1.0);
  else
    gl_FragColor = vec4(0.0,0.0,0.0,1.0);
}