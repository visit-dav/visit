/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "dye.h"

/* 
** from http://www.cs.rit.edu/~ncs/color/t_convert.html 
** each row below is really one column of the matrix 
*/
float dyeRGBtoXYZMatx[9] = {
  0.412453f, 0.212671f, 0.019334f,
  0.357580f, 0.715160f, 0.119193f,
  0.180423f, 0.072169f, 0.950227f};
float dyeXYZtoRGBMatx[9] = {
  3.240479f,-0.969256f, 0.055648f,
  -1.537150f, 1.875992f,-0.204043f,
  -0.498535f, 0.041556f, 1.057311f};

/* summing the rows of the RGBtoXYZ matrix to get X_n, Y_n, Z_n */
float dyeWhiteXYZ_n[3] = {0.950456f, 1.0f, 1.088754f};

/* the u'_n and v'_n which appear in the XYZ -> LUV conversion;
   u'_n = 4X_n / (X_n + 15Y_n + 3Z_n)
   v'_n = 9Y_n / (X_n + 15Y_n + 3Z_n)
*/ 
float dyeWhiteuvp_n[2] = {0.197839f, 0.468342f};

void
dyeRGBtoHSV(float *H, float *S, float *V,
            float  R, float  G, float  B) {
  float max, min, delta;
  
  max = AIR_MAX(R,G);
  max = AIR_MAX(B,max);
  min = AIR_MIN(R,G);
  min = AIR_MIN(B,min);

  *V = max;
  if (max != 0)
    *S = (max - min)/max;
  else
    *S = 0;
  if (0 == *S) {
    *H = 0;
    return;
  }
  /* else there is hue */
  delta = max - min;
  if (R == max)
    *H = (G - B)/delta;
  else if (G == max)
    *H = 2 + (B - R)/delta;
  else
    *H = 4 + (R - G)/delta;
  *H /= 6;
  if (*H < 0)
    *H += 1;
  return;
}

/*
******** dyeHSVtoRGB()
**
** conversion from HSV single hexcone to RGB
**
** input and ouput are all floats in interval [0,1]
** DOES NO RANGE CHECKING WHATSOEVER
**
** From Foley + vanDam, 2nd Ed., p. 593
*/
void
dyeHSVtoRGB(float *R, float *G, float *B,
            float  H, float  S, float  V) {
  float min, fract, vsf, mid1, mid2;
  int sextant;
  
  if (0 == S) {
    *R = *G = *B = V;
    return;
  }
  /* else there is hue */
  if (1 == H)
    H = 0;
  H *= 6;
  sextant = (int) floor(H);
  fract = H - sextant;
  vsf = V*S*fract;
  min = V*(1 - S);
  mid1 = min + vsf;
  mid2 = V - vsf;
  switch (sextant) {
  case 0: { *R = V;    *G = mid1; *B = min;  break; }
  case 1: { *R = mid2; *G = V;    *B = min;  break; }
  case 2: { *R = min;  *G = V;    *B = mid1; break; }
  case 3: { *R = min;  *G = mid2; *B = V;    break; }
  case 4: { *R = mid1; *G = min;  *B = V;    break; }
  case 5: { *R = V;    *G = min;  *B = mid2; break; }
  }
}

/*
******** dyeRGBtoHSL()
**
** converts from RGB to HSL double hexcone
** L: "lightness" = (max(R,G,B)+min(R,G,B))/2
** note that saturation (S) is different than the saturation in HSV
** hue (H) is the same in HSL and HSV
**
** r,g,b input and *h,*s,*l output are all floats in [0, 1]
** DOES NO RANGE CHECKING WHATSOEVER
**
** From Foley + vanDam, 2nd Ed., p. 595
*/
void
dyeRGBtoHSL(float *H, float *S, float *L,
            float  R, float  G, float  B) {
  float min, max, lev, delta;

  max = AIR_MAX(R,G);
  max = AIR_MAX(max,B);
  min = AIR_MIN(R,G);
  min = AIR_MIN(min,B);
  
  *L = lev = (max + min)/2.0f;
  if (max == min) {
    *S = 0;
    *H = 0;  /* actually, undefined */
    return;
  }
  /* else there is hue */
  delta = max - min;
  if (lev <= 0.5)
    *S = delta/(max + min);
  else
    *S = delta/(2-(max + min));
  if (R == max)
    *H = (G - B)/delta;
  else if (G == max)
    *H = 2 + (B - R)/delta;
  else
    *H = 4 + (R - G)/delta;
  *H /= 6;
  if (*H < 0)
    *H += 1;
  return;
}

/*
******** dyeHSLtoRGB()
**
** converts from HSL double hexcone back to RGB
**
** input and ouput are all floats in interval [0,1]
** DOES NO RANGE CHECKING WHATSOEVER
**
** From Foley + vanDam, 2nd Ed., p. 596
*/
void
dyeHSLtoRGB(float *R, float *G, float *B,
            float  H, float  S, float  L) {
  float m1, m2, fract, mid1, mid2;
  int sextant;
  
  if (S == 0) {
    *R = *G = *B = L;
    return;
  }
  /* else there is hue */
  if (L <= 0.5)
    m2 = L*(1+S);  /* the book says L*(L+S) which is ?? wrong ?? */
  else
    m2 = L + S - L*S;
  m1 = 2*L - m2;
  if (1 == H)
    H = 0;
  H *= 6;
  sextant = (int) floor(H);
  fract = H - sextant;
  mid1 = m1 + fract*(m2 - m1);
  mid2 = m2 + fract*(m1 - m2);
  /* compared to HSVtoRGB: V -> m2, min -> m1 */
  switch (sextant) {
  case 0: { *R = m2;   *G = mid1; *B = m1;   break; }
  case 1: { *R = mid2; *G = m2;   *B = m1;   break; }
  case 2: { *R = m1;   *G = m2;   *B = mid1; break; }
  case 3: { *R = m1;   *G = mid2; *B = m2;   break; }
  case 4: { *R = mid1; *G = m1;   *B = m2;   break; }
  case 5: { *R = m2;   *G = m1;   *B = mid2; break; }
  }
}

void
dyeRGBtoXYZ(float *X, float *Y, float *Z,
            float  R, float  G, float  B) {
  float in[3], out[3];
  
  ELL_3V_SET(in, R, G, B);
  ELL_3MV_MUL(out, dyeRGBtoXYZMatx, in);
  ELL_3V_GET(*X, *Y, *Z, out);
  return;
}

void
dyeXYZtoRGB(float *R, float *G, float *B,
            float  X, float  Y, float  Z) {
  float in[3], out[3];
  
  ELL_3V_SET(in, X, Y, Z);
  ELL_3MV_MUL(out, dyeXYZtoRGBMatx, in);
  ELL_3V_GET(*R, *G, *B, out);
  return;
}

float
dyeLcbrt(float t) {
  return AIR_CAST(float, (t > 0.008856 
                          ? airCbrt(t)
                          : 7.787*t + 16.0/116.0));
}

float
dyeLcubed(float t) {
  return(t > 0.206893 ? t*t*t : (t - 16.0f/116.0f)/7.787f);
}

void
dyeXYZtoLAB(float *L, float *A, float *B,
            float  X, float  Y, float  Z) {
  float Xnorm, Ynorm, Znorm;
  
  Xnorm = X/dyeWhiteXYZ_n[0];
  Ynorm = Y/dyeWhiteXYZ_n[1];
  Znorm = Z/dyeWhiteXYZ_n[2];
  *L = 116.0f*dyeLcbrt(Ynorm) - 16.0f;
  *A = 500.0f*(dyeLcbrt(Xnorm) - dyeLcbrt(Ynorm));
  *B = 200.0f*(dyeLcbrt(Ynorm) - dyeLcbrt(Znorm));
}

void 
dyeXYZtoLUV(float *L, float *U, float *V,
            float  X, float  Y, float  Z) {
  float Ynorm, up, vp;

  Ynorm = Y/dyeWhiteXYZ_n[1];
  *L = 116.0f*dyeLcbrt(Ynorm) - 16.0f;
  up = 4.0f*X/(X + 15.0f*Y + 3.0f*Z);
  vp = 9.0f*Y/(X + 15.0f*Y + 3.0f*Z);
  *U = 13.0f*(*L)*(up - dyeWhiteuvp_n[0]);
  *V = 13.0f*(*L)*(vp - dyeWhiteuvp_n[1]);
}

void 
dyeLABtoXYZ(float *X, float *Y, float *Z,
            float  L, float  A, float  B) {
  float YnormCbrt;

  YnormCbrt = (16 + L)/116;
  *X = dyeWhiteXYZ_n[0]*dyeLcubed(A/500 + YnormCbrt);
  *Y = dyeWhiteXYZ_n[1]*dyeLcubed(YnormCbrt);
  *Z = dyeWhiteXYZ_n[2]*dyeLcubed(YnormCbrt - B/200);
  return;
}

void 
dyeLUVtoXYZ(float *X, float *Y, float *Z,
            float  L, float  U, float  V) {
  float up, vp, YnormCbrt;

  YnormCbrt = (16 + L)/116;
  up = U/(13*L) + dyeWhiteuvp_n[0];
  vp = V/(13*L) + dyeWhiteuvp_n[1];
  *Y = dyeWhiteXYZ_n[1]*dyeLcubed(YnormCbrt);
  *X = -9*(*Y)*up/((up - 4)*vp - up*vp);
  *Z = (9*(*Y) - 15*vp*(*Y) - vp*(*X))/(3*vp);
  return;
}

void 
dyeIdentity(float *A, float *B, float *C,
            float  a, float  b, float  c) {
  *A = a;
  *B = b;
  *C = c;
  return;
}

dyeConverter dyeSimpleConvert[DYE_MAX_SPACE+1][DYE_MAX_SPACE+1] = 
{
  {NULL,          NULL,          NULL,          NULL,          NULL,          NULL,          NULL},
  {NULL,          dyeIdentity,   NULL,          dyeHSVtoRGB,   NULL,          NULL,          NULL},
  {NULL,          NULL,          dyeIdentity,   dyeHSLtoRGB,   NULL,          NULL,          NULL},
  {NULL,          dyeRGBtoHSV,   dyeRGBtoHSL,   dyeIdentity,   dyeRGBtoXYZ,   NULL,          NULL},
  {NULL,          NULL,          NULL,          dyeXYZtoRGB,   dyeIdentity,   dyeXYZtoLAB,   dyeXYZtoLUV},  
  {NULL,          NULL,          NULL,          NULL,          dyeLABtoXYZ,   dyeIdentity,   NULL},
  {NULL,          NULL,          NULL,          NULL,          dyeLUVtoXYZ,   NULL,          dyeIdentity}
  };

/*
******** dyeConvert()
**
** master color conversion function.  Can convert between any two
** types by recursive calls and calls to the simple converters above.
*/
int
dyeConvert(dyeColor *col, int outSpace) {
  char me[] = "dyeConvert", err[128];
  float i0, i1, i2, o0, o1, o2;
  dyeConverter simple;
  int inSpace, E;

  E = 0;
  if (!col) {
    sprintf(err, "%s: got NULL pointer", me); biffAdd(DYE, err); return 1;
  }
  inSpace = dyeColorGet(&i0, &i1, &i2, col);
  if (!DYE_VALID_SPACE(inSpace)) {
    sprintf(err, "%s: invalid input space #%d\n", me, inSpace);
    biffAdd(DYE, err); return 1;
  }
  if (!DYE_VALID_SPACE(outSpace)) {
    sprintf(err, "%s: invalid output space #%d\n", me, outSpace);
    biffAdd(DYE, err); return 1;
  }

  if ( (simple = dyeSimpleConvert[inSpace][outSpace]) ) {
    (*simple)(&o0, &o1, &o2, i0, i1, i2);
    dyeColorSet(col, outSpace, o0, o1, o2);
  }
  else {
    /* we have some work to do ... */
    if (inSpace < dyeSpaceRGB && outSpace < dyeSpaceRGB) {
      /* its an easy HSV <-- RGB --> HSL conversion */
      if (!E) E |= dyeConvert(col, dyeSpaceRGB);
      if (!E) E |= dyeConvert(col, outSpace);
    }
    else if (inSpace > dyeSpaceXYZ && outSpace > dyeSpaceXYZ) {
      /* its an easy LAB <-- XYZ --> LUV conversion */
      if (!E) E |= dyeConvert(col, dyeSpaceXYZ);
      if (!E) E |= dyeConvert(col, outSpace);
    }
    else {
      /* the start and end spaces are at different stages */
      if (inSpace < outSpace) {
        /* we are going towards higher stages */
        if (inSpace < dyeSpaceRGB) {
          if (!E) E |= dyeConvert(col, dyeSpaceRGB);
          if (!E) E |= dyeConvert(col, outSpace);
        }
        else if (inSpace == dyeSpaceRGB) {
          if (!E) E |= dyeConvert(col, dyeSpaceXYZ);
          if (!E) E |= dyeConvert(col, outSpace);
        }
        else {
          sprintf(err, "%s: CONFUSED! can't go %s -> %s\n",
                  me, dyeSpaceToStr[inSpace], dyeSpaceToStr[outSpace]);
          biffAdd(DYE, err);
          E = 1;
        }
      }
      else {
        /* we are going towards lower stages */
        if (outSpace < dyeSpaceRGB) {
          if (!E) E |= dyeConvert(col, dyeSpaceRGB);
          if (!E) E |= dyeConvert(col, outSpace);
        }
        else if (outSpace == dyeSpaceRGB) {
          if (!E) E |= dyeConvert(col, dyeSpaceXYZ);
          if (!E) E |= dyeConvert(col, dyeSpaceRGB);
        }
        else {
          sprintf(err, "%s: CONFUSED! can't go %s -> %s\n",
                  me, dyeSpaceToStr[inSpace], dyeSpaceToStr[outSpace]);
          biffAdd(DYE, err);
          E = 1;
        }
      }
    }
  }
  
  return E;
}
