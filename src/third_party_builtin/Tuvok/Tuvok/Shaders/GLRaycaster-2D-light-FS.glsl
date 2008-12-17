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
  \file    GLRaycaster-2D-light-FS.glsl
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.0
  \date    October 2008
*/

uniform sampler3D texVolume;  ///< the data volume
uniform sampler2D texTrans2D; ///< the 2D Transfer function
uniform sampler2D texRayExitPos; ///< the backface (or ray exit point) texture in eyecoords
uniform float fTransScale;    ///< value scale for 2D Transfer function lookup
uniform float fGradientScale; ///< gradient scale for 2D Transfer function lookup
uniform float fStepScale;     ///< opacity correction quotient
uniform vec3 vVoxelStepsize;  ///< Stepsize (in texcoord) to get to the next voxel
uniform vec2 vScreensize;      ///< the size of the screen in pixels
uniform float fRayStepsize;     ///< stepsize along the ray


uniform vec3 vLightAmbient;
uniform vec3 vLightSpecular;
uniform vec3 vLightDiffuse;
uniform vec3 vLightDir;

varying vec3 vEyePos;

vec4 ColorBlend(vec4 src, vec4 dst) {
	vec4 result = dst;
	result.rgb   += src.rgb*(1.0-dst.a)*src.a;
	result.a     += (1.0-dst.a)*src.a;
	return result;
}

void main(void)
{
  // compute the coordinates to look up the previous pass
  vec2 vFragCoords = vec2(gl_FragCoord.x / vScreensize.x , gl_FragCoord.y / vScreensize.y);

  // compute the ray parameters
  vec3  vRayEntry    = vEyePos;  
  vec3  vRayExit     = texture2D(texRayExitPos, vFragCoords).xyz;  
  vec3  vRayEntryTex = (gl_TextureMatrix[0] * vec4(vRayEntry,1.0)).xyz;
  vec3  vRayExitTex  = (gl_TextureMatrix[0] * vec4(vRayExit,1.0)).xyz;
  vec3  vRayDir      = vRayExit - vRayEntry;
  
  float fRayLength = length(vRayDir);
  vRayDir /= fRayLength;

  // compute the maximum number of steps before the domain is left
  int iStepCount = int(fRayLength/fRayStepsize)+1;
  
  vec3  fRayInc    = vRayDir*fRayStepsize;
  vec3  vRayIncTex = (vRayExitTex-vRayEntryTex)/(fRayLength/fRayStepsize);

  // do the actual raycasting
  vec4  vColor = vec4(0.0,0.0,0.0,0.0);
  vec3  vCurrentPosTex = vRayEntryTex;
  vec3  vCurrentPos    = vRayEntry;
  for (int i = 0;i<iStepCount;i++) {
    float fVolumVal = texture3D(texVolume, vCurrentPosTex).x;	

    // compute the gradient/normal
    float fVolumValXp = texture3D(texVolume, vCurrentPosTex+vec3(+vVoxelStepsize.x,0,0)).x;
    float fVolumValXm = texture3D(texVolume, vCurrentPosTex+vec3(-vVoxelStepsize.x,0,0)).x;
    float fVolumValYp = texture3D(texVolume, vCurrentPosTex+vec3(0,-vVoxelStepsize.y,0)).x;
    float fVolumValYm = texture3D(texVolume, vCurrentPosTex+vec3(0,+vVoxelStepsize.y,0)).x;
    float fVolumValZp = texture3D(texVolume, vCurrentPosTex+vec3(0,0,+vVoxelStepsize.z)).x;
    float fVolumValZm = texture3D(texVolume, vCurrentPosTex+vec3(0,0,-vVoxelStepsize.z)).x;
    vec3  vGradient = vec3(fVolumValXm-fVolumValXp, fVolumValYp-fVolumValYm, fVolumValZm-fVolumValZp); 
    float fGradientMag = length(vGradient); 

    /// apply 2D transfer function
	  vec4  vTransVal = texture2D(texTrans2D, vec2(fVolumVal*fTransScale, 1.0-fGradientMag*fGradientScale));

    /// compute lighting
    vec3 vNormal     = gl_NormalMatrix * vGradient;
    float l = length(vNormal); if (l>0.0) vNormal /= l; // secure normalization
    vec3 vViewDir    = normalize(vec3(0,0,0)-vCurrentPos);
    vec3 vReflection = normalize(reflect(vViewDir, vNormal));
    vec3 vLightColor = vLightAmbient+
                       vLightDiffuse*max(dot(vNormal, -vLightDir),0.0)*vTransVal.xyz+
                       vLightSpecular*pow(max(dot(vReflection, vLightDir),0.0),8.0);

    /// apply opacity correction
    vTransVal.a = 1.0 - pow(1.0 - vTransVal.a, fStepScale);
    
    vTransVal = clamp(vec4(vLightColor.x, vLightColor.y, vLightColor.z, vTransVal.a),0.0,1.0);
    vColor = ColorBlend(vTransVal,vColor);

    vCurrentPos    += fRayInc;
    vCurrentPosTex += vRayIncTex;

    if (vColor.a >= 0.99) break;
  }
  
  gl_FragColor  = vColor;
}
