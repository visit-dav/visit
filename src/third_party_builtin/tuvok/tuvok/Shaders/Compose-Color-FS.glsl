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
  \file    GLRaycaster-compose-FS.glsl
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.0
  \date    October 2008
*/

uniform sampler2D texRayHitPos;    ///< the hitposition of the ray (alpha flags hit yes/no)
uniform sampler2D texRayHitNormal; ///< the surface normal at the hit position

uniform vec3 vLightAmbient;
uniform vec3 vLightDir;

uniform vec2 vScreensize;      ///< the size of the screen in pixels
uniform vec2 vProjParam;       ///< X = far / (far - near)  / Y = (far * near / (near - far))

void main(void){
  // compute the coordinates to look up the previous pass
  vec2 vFragCoords = vec2(gl_FragCoord.x / vScreensize.x , gl_FragCoord.y / vScreensize.y);

  // get hitposition and check if a isosurface hit for this ray was found
  vec4  vPosition = texture2D(texRayHitPos, vFragCoords);
  
  if (vPosition.a == 0.0) discard;
  
  // get hit normal
  vec4  vNormalFetch = texture2D(texRayHitNormal, vFragCoords);
  vec3  vNormal  = abs(vNormalFetch.xyz);

  // recover color from the two alpha channels
  vec3 vColor = vec3(vPosition.a-1.0, 
                     floor(vNormalFetch.a/2.0)/256.0, 
                     vNormalFetch.a-floor(vNormalFetch.a));

	// compute lighting
	vec3 vViewDir    = normalize(vec3(0.0,0.0,0.0)-vPosition.xyz);
  float l = length(vNormal);
  vec3 vLightColor = vLightAmbient * l + (1.0-l) * vColor.rgb +
                     vColor.rgb*clamp(dot(vNormal, -vLightDir),0.0,1.0);

	/// write result to fragment color
	gl_FragColor    = vec4(vLightColor.x, vLightColor.y, vLightColor.z, 1.0);

  // compute non linear depth from linear eye depth
  gl_FragDepth = vProjParam.x + (vProjParam.y / -vPosition.z);
}