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

#include "nrrd.h"

#define _SINC(x) (sin(AIR_PI*x)/(AIR_PI*x))

double
_nrrdWindSincInt(const double *parm) {

  AIR_UNUSED(parm);
  /* This isn't true, but there aren't good accurate, closed-form
     approximations for these integrals ... */
  return 1.0;
}

double
_nrrdDWindSincInt(const double *parm) {

  AIR_UNUSED(parm);
  /* ... or their derivatives */
  return 0.0;
}

double
_nrrdWindSincSup(const double *parm) {
  double S;

  S = parm[0];
  return parm[1]*S;
}

#define POW1(S) (S)
#define POW2(S) ((S)*(S))
#define POW3(S) ((S)*(S)*(S))

#define WS_1_F(name, mac, spow)                               \
float                                                         \
_nrrd##name##_1_f(float x, const double *parm) {              \
  float R, S;                                                 \
                                                              \
  S = AIR_CAST(float, parm[0]); R = AIR_CAST(float, parm[1]); \
  x /= S;                                                     \
  return AIR_CAST(float, mac(x, R)/spow(S));                  \
}

#define WS_N_F(name, mac, spow)                                     \
void                                                                \
_nrrd##name##_N_f(float *f, const float *x, size_t len,             \
                  const double *parm) {                             \
  float S, R, t;                                                    \
  size_t i;                                                         \
                                                                    \
  S = AIR_CAST(float, parm[0]); R = AIR_CAST(float, parm[1]);       \
  for (i=0; i<len; i++) {                                           \
    t = x[i]/S;                                                     \
    f[i] = AIR_CAST(float, mac(t, R)/spow(S));                      \
  }                                                                 \
}

#define WS_1_D(name, mac, spow)                   \
double                                            \
_nrrd##name##_1_d(double x, const double *parm) { \
  double R, S;                                    \
                                                  \
  S = parm[0]; R = parm[1];                       \
  x /= S;                                         \
  return mac(x, R)/spow(S);                       \
}

#define WS_N_D(name, mac, spow)                                     \
void                                                                \
_nrrd##name##_N_d(double *f, const double *x, size_t len,           \
                  const double *parm) {                             \
  double S, R, t;                                                   \
  size_t i;                                                         \
                                                                    \
  S = parm[0]; R = parm[1];                                         \
  for (i=0; i<len; i++) {                                           \
    t = x[i]/S;                                                     \
    f[i] = mac(t, R)/spow(S);                                       \
  }                                                                 \
}

/* ------------------------------------------------------------ */

#define _HANN(x, R) \
   (x > R ? 0 : (x < -R ? 0 : (\
   (x < R/50000 && x > -R/50000) \
     ? 1.1 - x*x*(AIR_PI*AIR_PI*(3 + 2*R*R)/(12*R*R) \
                + AIR_PI*AIR_PI*AIR_PI*AIR_PI*(5 + 2*R*R*(5 + 2*R*R))*x*x/(240*R*R*R*R)) \
     : (1 + cos(AIR_PI*x/R))*_SINC(x)/2) \
    ))

WS_1_D(Hann, _HANN, POW1)
WS_1_F(Hann, _HANN, POW1)
WS_N_F(Hann, _HANN, POW1)
WS_N_D(Hann, _HANN, POW1)

NrrdKernel
_nrrdKernelHann = {
  "hann",
  2, _nrrdWindSincSup,  _nrrdWindSincInt,   
  _nrrdHann_1_f, _nrrdHann_N_f, _nrrdHann_1_d, _nrrdHann_N_d
};
NrrdKernel *const
nrrdKernelHann = &_nrrdKernelHann;

/* ------------------------------------------------------------ */

#define _DHANN(x, R)                                              \
   (x > R ? 0.0 : (x < -R ? 0.0 : (                               \
    (x < R/50000 && x > -R/50000)                                 \
     ? -x*AIR_PI*AIR_PI*(3 + 2*R*R)/(6*R*R)                           \
     : ((R*(1 + cos(AIR_PI*x/R))*(AIR_PI*x*cos(AIR_PI*x) - sin(AIR_PI*x)) \
       - AIR_PI*x*sin(AIR_PI*x)*sin(AIR_PI*x/R))/(2*R*AIR_PI*x*x))        \
   )))

WS_1_D(DHann, _DHANN, POW2)
WS_1_F(DHann, _DHANN, POW2)
WS_N_F(DHann, _DHANN, POW2)
WS_N_D(DHann, _DHANN, POW2)

NrrdKernel
_nrrdKernelDHann = {
  "hannD",
  2, _nrrdWindSincSup, _nrrdDWindSincInt,  
  _nrrdDHann_1_f,  _nrrdDHann_N_f,  _nrrdDHann_1_d,  _nrrdDHann_N_d
};
NrrdKernel *const
nrrdKernelHannD = &_nrrdKernelDHann;

/* ------------------------------------------------------------ */

#define _DDHANN_A(x, R) \
  (2*AIR_PI*R*cos(AIR_PI*x)*(R + R*cos(AIR_PI*x/R) + AIR_PI*x*sin(AIR_PI*x/R)))
#define _DDHANN_B(x, R)                                      \
  (cos(AIR_PI*x/R)*(AIR_PI*AIR_PI*x*x + R*R*(AIR_PI*AIR_PI*x*x - 2)) + \
   R*(R*(AIR_PI*AIR_PI*x*x - 2) - 2*AIR_PI*x*sin(AIR_PI*x/R)))
#define _DDHANN(x, R)                                                         \
   (x > R ? 0 : (x < -R ? 0 : (                                               \
     (x < R/50000 && x > -R/50000)                                            \
      ? (AIR_PI*AIR_PI/(2*R*R))*( -(3 + 2*R*R)/3                                  \
                             + AIR_PI*AIR_PI*(5 + 2*R*R*(5 + R*R))*x*x/(10*R*R))  \
      : -(_DDHANN_A(x,R) + sin(AIR_PI*x)*_DDHANN_B(x,R)/x)/(2*AIR_PI*R*R*x*x)     \
    )))

WS_1_D(DDHann, _DDHANN, POW3)
WS_1_F(DDHann, _DDHANN, POW3)
WS_N_F(DDHann, _DDHANN, POW3)
WS_N_D(DDHann, _DDHANN, POW3)

NrrdKernel
_nrrdKernelDDHann = {
  "hannDD",
  2, _nrrdWindSincSup, _nrrdDWindSincInt,  
  _nrrdDDHann_1_f, _nrrdDDHann_N_f, _nrrdDDHann_1_d, _nrrdDDHann_N_d
};
NrrdKernel *const
nrrdKernelHannDD = &_nrrdKernelDDHann;

/* ------------------------------------------------------------ */

#define _BLACK(x, R)                                             \
   (x > R ? 0 : (x < -R ? 0 : (                                  \
    (x < R/50000 && x > -R/50000)                                \
     ? 1.0 - x*x*(1.6449340668482264 + 4.046537804446637/(R*R))  \
     : (0.42 + cos(AIR_PI*x/R)/2 + 0.08*cos(2*AIR_PI*x/R))*_SINC(x)  \
   )))

WS_1_D(Black, _BLACK, POW1)
WS_1_F(Black, _BLACK, POW1)
WS_N_F(Black, _BLACK, POW1)
WS_N_D(Black, _BLACK, POW1)

NrrdKernel
_nrrdKernelBlackman = {
  "blackman",
  2, _nrrdWindSincSup,  _nrrdWindSincInt,   
  _nrrdBlack_1_f, _nrrdBlack_N_f, _nrrdBlack_1_d, _nrrdBlack_N_d
};
NrrdKernel *const
nrrdKernelBlackman = &_nrrdKernelBlackman;

/* ------------------------------------------------------------ */

#define _DBLACK_A(x, R)                                   \
  R*x*cos(AIR_PI*x)*(2.638937829015426 + AIR_PI*cos(AIR_PI*x/R) \
                   + 0.5026548245743669*cos(2*AIR_PI*x/R))
#define _DBLACK_B(x, R)                                                     \
  sin(AIR_PI*x)*(-0.84*R - R*cos(AIR_PI*x/R) - 0.16*R*cos(2*AIR_PI*x/R) -         \
               AIR_PI*x*sin(AIR_PI*x/R) - 1.0053096491487339*x*sin(2*AIR_PI*x/R))
#define _DBLACK(x, R)                                 \
  (x > R ? 0.0 : (x < -R ? 0.0 : (                    \
   (x < R/50000 && x > -R/50000)                      \
   ? -x*(3.289868133696453 + 8.093075608893272/(R*R)) \
   : (_DBLACK_A(x,R) + _DBLACK_B(x,R))/(2*AIR_PI*R*x*x) \
  )))

WS_1_D(DBlack, _DBLACK, POW2)
WS_1_F(DBlack, _DBLACK, POW2)
WS_N_F(DBlack, _DBLACK, POW2)
WS_N_D(DBlack, _DBLACK, POW2)

NrrdKernel
_nrrdKernelDBlack = {
  "blackmanD",
  2, _nrrdWindSincSup, _nrrdDWindSincInt,  
  _nrrdDBlack_1_f,  _nrrdDBlack_N_f,  _nrrdDBlack_1_d,  _nrrdDBlack_N_d
};
NrrdKernel *const
nrrdKernelBlackmanD = &_nrrdKernelDBlack;

/* ------------------------------------------------------------ */

#define _DDBLACK(x, R)                                                          \
  (x > R ? 0.0 : (x < -R ? 0.0 : (                                              \
   (x < R/30 && x > -R/30)                                                      \
   ? (-(3.289868133696453 + 8.093075608893272/(R*R))                            \
      + x*x*(9.7409091034 + 86.694091020262/(R*R*R*R) + 79.8754546479/(R*R)))   \
   : ((R*x*cos(AIR_PI*x)*(-2.638937829015426*R - AIR_PI*R*cos((AIR_PI*x)/R)           \
            - 0.5026548245743669*R*cos((2*AIR_PI*x)/R)                            \
            - AIR_PI*AIR_PI*x*sin((AIR_PI*x)/R)                                       \
            - 3.158273408348595*x*sin((2*AIR_PI*x)/R))                            \
  + sin(AIR_PI*x)*((-4.934802200544679*x*x                                        \
           + R*R*(1 - 4.934802200544679*x*x))*cos((AIR_PI*x)/R)                   \
          + (-3.158273408348595*x*x                                             \
             + R*R*(0.16 - 0.7895683520871487*x*x))*cos((2*AIR_PI*x)/R)           \
          + R*(0.84*R - 4.14523384845753*R*x*x                                  \
               + AIR_PI*x*sin((AIR_PI*x)/R)                                         \
               + 1.0053096491487339*x*sin((2*AIR_PI*x)/R))))/(AIR_PI*R*R*x*x*x))    \
   )))

WS_1_D(DDBlack, _DDBLACK, POW3)
WS_1_F(DDBlack, _DDBLACK, POW3)
WS_N_F(DDBlack, _DDBLACK, POW3)
WS_N_D(DDBlack, _DDBLACK, POW3)

NrrdKernel
_nrrdKernelDDBlack = {
  "blackDD",
  2, _nrrdWindSincSup, _nrrdDWindSincInt,  
  _nrrdDDBlack_1_f, _nrrdDDBlack_N_f, _nrrdDDBlack_1_d, _nrrdDDBlack_N_d
};
NrrdKernel *const
nrrdKernelBlackmanDD = &_nrrdKernelDDBlack;

