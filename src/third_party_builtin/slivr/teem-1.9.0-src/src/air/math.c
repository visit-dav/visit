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


#include "air.h"

/*
** by the way, the organization of functions into files is a little
** arbitrary around here
*/

/*
******** airNormalRand
**
** generates two numbers with normal distribution (mean 0, stdv 1)
** using the Box-Muller transform.
**
** on (seemingly sound) advice of
** <http://www.taygeta.com/random/gaussian.html>,
** I'm using the polar form of the Box-Muller transform, instead of the
** Cartesian one described at
** <http://mathworld.wolfram.com/Box-MullerTransformation.html>
**
** this is careful to not write into given NULL pointers
*/
void
airNormalRand(double *z1, double *z2) {
  double w, x1, x2;

  do {
    x1 = 2*airDrandMT() - 1;
    x2 = 2*airDrandMT() - 1;
    w = x1*x1 + x2*x2;
  } while ( w >= 1 );
  w = sqrt((-2*log(w))/w);
  if (z1) {
    *z1 = x1*w;
  }
  if (z2) {
    *z2 = x2*w;
  }
  return;
}

void
airNormalRand_r(double *z1, double *z2, airRandMTState *state) {
  double w, x1, x2;

  do {
    x1 = 2*airDrandMT_r(state) - 1;
    x2 = 2*airDrandMT_r(state) - 1;
    w = x1*x1 + x2*x2;
  } while ( w >= 1 );
  w = sqrt((-2*log(w))/w);
  if (z1) {
    *z1 = x1*w;
  }
  if (z2) {
    *z2 = x2*w;
  }
  return;
}

/*
******** airRandInt
**
** returns a random integer in range [0, N-1]
*/
unsigned int
airRandInt(unsigned int N) {
  
  return airIndex(0.0, airDrandMT(), 1.0, N);
}

unsigned int
airRandInt_r(airRandMTState *state, unsigned int N) {
  
  return airIndex(0.0, airDrandMT_r(state), 1.0, N);
}

/*
******** airShuffle()
**
** generates a random permutation of integers [0..N-1] if perm is non-zero,
** otherwise, just fills buff with [0..N-1] in order
*/
void
airShuffle(unsigned int *buff, unsigned int N, int perm) {
  unsigned i, swp, tmp;

  if (!(buff && N > 0)) {
    return;
  }
    
  for (i=0; i<N; i++) {
    buff[i] = i;
  }
  if (perm) {
    for (i=0; i<N; i++) {
      swp = i + airRandInt(N - i);
      tmp = buff[swp];
      buff[swp] = buff[i];
      buff[i] = tmp;
    }
  }
}

void
airShuffle_r(airRandMTState *state,
             unsigned int *buff, unsigned int N, int perm) {
  unsigned int i, swp, tmp;

  /* HEY !!! COPY AND PASTE !!!! */
  if (!(buff && N > 0)) {
    return;
  }

  for (i=0; i<N; i++) {
    buff[i] = i;
  }
  if (perm) {
    for (i=0; i<N; i++) {
      swp = i + airRandInt_r(state, N - i);
      tmp = buff[swp];
      buff[swp] = buff[i];
      buff[i] = tmp;
    }
  }
  /* HEY !!! COPY AND PASTE !!!! */
}

double
airSgnPow(double v, double p) {

  return (p == 1 
          ? v
          : (v >= 0
             ? pow(v, p)
             : -pow(-v, p)));
}

/*
******** airLog2()
**
** silly little function which returns log_2(n) if n is a power of 2,
** or -1 otherwise
*/
int 
airLog2(double n) {

  if (!AIR_EXISTS(n)) {
    return -1;  }
  if (n == 1.0) {
    return 0;  }
  if (n < 2) {
    return -1;  }
  return 1 + airLog2(n/2.0);
}

int
airSgn(double v) {
  return (v > 0
          ? 1
          : (v < 0
             ? -1
             : 0));
}

/*
******** airCbrt
**
** cbrt() isn't ANSI, so any hacks to create a stand-in for cbrt()
** are done here.
*/
double
airCbrt(double v) {
#ifdef _WIN32
  /* msvc does not know how to take powers of small negative numbers,
   * so we have to tell it to do it right */
  return (v < 0.0 ? -pow(-v,1.0/3.0) : pow(v,1.0/3.0));
#else
  return cbrt(v);
#endif
}

double
airGaussian(double x, double mean, double stdv) {
  
  x = x - mean;
  return exp(-(x*x)/(2*stdv*stdv))/(stdv*sqrt(2*AIR_PI));
}

/*
** The function approximations below were done by GLK in Mathematica,
** using its MiniMaxApproximation[] function.  The functional forms
** used for the Bessel functions were copied from Numerical Recipes,
** which were in turn copied from the "Handbook of Mathematical
** Functions with Formulas, Graphs, and Mathematical Tables" by
** Abramowitz and Stegun.
**
** The rational functions (crudely copy/paste from Mathematica into
** this file) upon which the approximations are based have a relative
** error of less than 10^-9, at least on the intervals on which they
** were created (in the case of the second branch of the
** approximation, the lower end of the interval was chosen as close to
** zero as possible). The relative error of the total approximation
** may be greater.
*/

double
airErfc(double x) {
  double ax, y, b;

  ax = AIR_ABS(x);
  if (ax < 0.9820789566638689) {
    b = (0.9999999999995380997 + ax*(-1.0198241793287349401 + 
        ax*(0.37030717279808919457 + ax*(-0.15987839763633308864 + 
        ax*(0.12416682580357861661 + (-0.04829622197742573233 +
        0.0066094852952188890901*ax)*ax)))))/
        (1 + ax*(0.1085549876246959456 + ax*(0.49279836663925410323 + 
        ax*(0.020058474597886988472 + ax*(0.10597158000597863862 + 
        (-0.0012466514192679810876 + 0.0099475501252703646772*ax)*ax)))));
  } else if (ax < 2.020104167011169) {
    y = ax - 1;
    b = (0.15729920705029613528 + y*(-0.37677358667097191131 + 
        y*(0.3881956571123873123 + y*(-0.22055886537359936478 + 
        y*(0.073002666454740425451 + (-0.013369369366972563804 +
        0.0010602024397541548717*y)*y)))))/
        (1 + y*(0.243700597525225235 + y*(0.47203101881562848941 + 
        y*(0.080051054975943863026 + y*(0.083879049958465759886 + 
        (0.0076905426306038205308 + 0.0058528196473365970129*y)*y)))));
  } else {
    y = 2/ax;
    b = (-2.7460876468061399519e-14 + y*(0.28209479188874503125 + 
        y*(0.54260398586720212019 + y*(0.68145162781305697748 + 
        (0.44324741856237800393 + 0.13869182273440856508*y)*y))))/
        (1 + y*(1.9234811027995435174 + y*(2.5406810534399069812 + 
        y*(1.8117409273494093139 + (0.76205066033991530997 +
        0.13794679143736608167*y)*y))));
    b *= exp(-x*x);
  }
  if (x < 0) {
    b = 2-b;
  }
  return b;
}

double
airErf(double x) {
  return 1.0 - airErfc(x);
}

/*
******** airBesselI0
**
** modified Bessel function of the first kind, order 0
*/
double
airBesselI0(double x) {
  double b, ax, y;

  ax = AIR_ABS(x);
  if (ax < 5.664804810929075) {
    y = x/5.7;
    y *= y;
    b = (0.9999999996966272 + y*(7.7095783675529646 + 
        y*(13.211021909077445 + y*(8.648398832703904 + 
        (2.5427099920536578 + 0.3103650754941674*y)*y))))/
        (1 + y*(-0.41292170755003793 + (0.07122966874756179 
        - 0.005182728492608365*y)*y));
  } else {
    y = 5.7/ax;
    b = (0.398942280546057 + y*(-0.749709626164583 + 
        y*(0.507462772839054 + y*(-0.0918770649691261 + 
        (-0.00135238228377743 - 0.0000897561853670307*y)*y))))/
        (1 + y*(-1.90117313211089 + (1.31154807540649 
        - 0.255339661975509*y)*y));
    b *= (exp(ax)/sqrt(ax));
  }
  return b;
}

/*
******** airBesselI1
**
** modified Bessel function of the first kind, order 1
*/
double
airBesselI1(double x) {
  double b, ax, y;

  ax = AIR_ABS(x);
  if (ax < 6.449305566387246) {
    y = x/6.45;
    y *= y;
    b = ax*(0.4999999998235554 + y*(2.370331499358438 + 
        y*(3.3554331305863787 + y*(2.0569974969268707 + 
        (0.6092719473097832 + 0.0792323006694466*y)*y))))/
        (1 + y*(-0.4596495788370524 + (0.08677361454866868 \
        - 0.006777712190188699*y)*y));
  } else {
    y = 6.45/ax;
    b = (0.398942280267484 + y*(-0.669339325353065 + 
        y*(0.40311772245257 + y*(-0.0766281832045885 + 
        (0.00248933264397244 + 0.0000703849046144657*y)*y))))/
        (1 + y*(-1.61964537617937 + (0.919118239717915 - 
        0.142824922601647*y)*y));
    b *= exp(ax)/sqrt(ax);
  }
  return x < 0.0 ? -b : b;
}

/*
******** airLogBesselI0
**
** natural logarithm of airBesselI0
*/
double
airLogBesselI0(double x) {
  double b, ax, y;

  ax = AIR_ABS(x);
  if (ax < 4.985769687853781) {
    y = x/5.0;
    y *= y;
    b = (5.86105592521167098e-27 + y*(6.2499999970669017 + 
        y*(41.1327842713925212 + y*(80.9030404787605539 + 
        y*(50.7576267390706893 + 6.88231907401413133*y)))))/
        (1 + y*(8.14374525361325784 + y*(21.3288286560361152 + 
        y*(20.0880670952325953 + (5.5138982784800139 + 
        0.186784275148079847*y)*y))));
  } else {
    y = 5.0/ax;
    b = (-0.91893853280169872884 + y*(2.7513907055333657679 + 
        y*(-3.369024122613176551 + y*(1.9164545708124343838 + 
        (-0.46136261965797010108 + 0.029092365715948197066*y)*y))))/
        (1 + y*(-2.9668913151685312745 + y*(3.5882191453626541066 + 
        y*(-1.9954040017063882247 + (0.45606687718126481603 - 
        0.0231678041994100784*y)*y))));
    b += ax - log(ax)/2;
  }
  return b;
}

/*
******** airBesselI1By0
**
** the quotient airBesselI1(x)/airBesselI0(x)
*/
double
airBesselI1By0(double x) {
  double q, ax, y;
  
  ax = AIR_ABS(x);
  if (ax < 2.2000207427754046) {
    y = ax/2.2;
    q = (1.109010375603908e-29 + y*(1.0999999994454934 + 
        y*(0.05256560007682146 + y*(0.3835178789165919 + 
        (0.011328636410807382 + 0.009066934622942833*y)*y))))/
        (1 + y*(0.047786822784523904 + y*(0.9536550439261017 + 
        (0.03918380275938573 + 0.09730715527121027*y)*y)));
  } else if (ax < 5.888258985638512) {
    y = (ax-2.2)/3.68;
    q = (0.7280299135046744 + y*(2.5697382341657002 + 
        y*(3.69819451510548 + y*(3.131374238190916 + 
        (1.2811958061688737 + 0.003601218043466571*y)*y))))/
        (1 + y*(2.8268553393021527 + y*(4.164742157157812 + 
        y*(3.2377768820326756 + 1.3051900460060342*y))));
  } else {
    y = 5.88/ax;
    q = (1.000000000646262020372530870790956088593 + 
         y*(-2.012513842496824157039372120680781513697 + 
         y*(1.511644590219033259220408231325838531123 + 
         (-0.3966391319921114140077576390415605232003 + 
         0.02651815520696779849352690755529178056941*y)*y)))/
         (1 + y*(-1.927479858946526082413004924812844224781 + 
         y*(1.351359456116228102988125069310621733956 + 
         (-0.288087717540546638165144937495654019162 + 
         0.005906535730887518966127383058238522133819*y)*y)));
  }
  return x < 0.0 ? -q : q;
}
