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

#include "ten.h"
#include "privateTen.h"

double
tenBVecNonLinearFit_error(double *bb, double *ss, double *ww, int len,
                          double amp, double dec) {
  int ii;
  double err, tmp;

  err = 0;
  for (ii=0; ii<len; ii++) {
    tmp = ww[ii]*(amp*exp(-dec*bb[ii]) - ss[ii]);
    err += tmp*tmp;
  }
  return err;
}

void
tenBVecNonLinearFit_linear(double *amp, double *dec,
                           double *bb, double *ss, double *ww, int len) {
  double x, y, wi=0, xi=0, yi=0, xiyi=0, xisq=0, det;
  int ii;

  for (ii=0; ii<len; ii++) {
    x = bb[ii];
    y = log(AIR_MAX(ss[ii], 0.01));
    xi += ww[ii]*x;
    yi += ww[ii]*y;
    xiyi += ww[ii]*x*y;
    xisq += ww[ii]*x*x;
    wi += ww[ii];
  }
  det = xisq*wi - xi*xi;
  *dec = -(wi*xiyi - xi*yi)/det;   /* negative sign assumed in model */
  *amp = exp((-xi*xiyi + xisq*yi)/det);
  return;
}

void
tenBVecNonLinearFit_GNstep(double *d_amp, double *d_dec,
                           double *bb, double *ss, double *ww, int len,
                           double amp, double dec) {
  double tmp, ff, dfdx1, dfdx2, AA=0, BB=0, CC=0, JTf[2], det;
  int ii;

  JTf[0] = JTf[1] = 0;
  for (ii=0; ii<len; ii++) {
    tmp = exp(-dec*bb[ii]);
    ff = ww[ii]*(amp*tmp - ss[ii]);
    dfdx1 = ww[ii]*tmp;
    dfdx2 = -ww[ii]*amp*bb[ii]*tmp;
    AA += dfdx1*dfdx1;
    BB += dfdx1*dfdx2;
    CC += dfdx2*dfdx2;
    JTf[0] += dfdx1*ff;
    JTf[1] += dfdx2*ff;
  }
  det = AA*CC - BB*BB;
  *d_amp = -(CC*JTf[0] - BB*JTf[1])/det;
  *d_dec = -(-BB*JTf[0] + AA*JTf[1])/det;
  return;
}


/*
******** tenBVecNonLinearFit
**
** Assuming that axis 0 represents a sequence of DWI measurements at a 
** range of b values (as described by bb[i]), do non-linear least-squares
** fitting of those measurements, governed by weights ww[i] (with at
** most iterMax interations, or terminated when L2 norm change < eps).
**
** Based on model fit amp*exp(-b*dec), output nrrd's axis 0 has three values:
** 0: amp
** 1: dec
** 2: error of fit
** and all other axes are unchanged from input.  Output type is always double.
*/
int
tenBVecNonLinearFit(Nrrd *nout, const Nrrd *nin, 
                    double *bb, double *ww, int iterMax, double eps) {
  char me[]="tenBVecNonLinearFit", err[BIFF_STRLEN];
  int map[NRRD_DIM_MAX], vecSize, iter;
  size_t ii, size[NRRD_DIM_MAX], vecI, vecNum;
  char *vec;
  double *out, ss[AIR_STRLEN_SMALL], amp, dec, d_amp, d_dec, error, diff,
    (*vecLup)(const void *v, size_t I);

  if (!( nout && nin && bb && ww )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  
  if (!( nin->dim >= 2 )) {
    sprintf(err, "%s: nin->dim (%d) not >= 2", me, nin->dim);
    biffAdd(TEN, err); return 1;
  }
  if (!( nin->axis[0].size < AIR_STRLEN_SMALL )) {
    sprintf(err, "%s: sorry need nin->axis[0].size (" 
            _AIR_SIZE_T_CNV ") < %d", 
            me, nin->axis[0].size, AIR_STRLEN_SMALL);
    biffAdd(TEN, err); return 1;
  }

  /* allocate/set-up output */
  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, size);
  size[0] = 3;
  if (nrrdMaybeAlloc_nva(nout, nrrdTypeDouble, nin->dim, size)) {
    sprintf(err, "%s: couldn't allocate output", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  for (ii=1; ii<nin->dim; ii++) {
    map[ii] = ii;
  }
  map[0] = -1;
  if (nrrdAxisInfoCopy(nout, nin, map, NRRD_AXIS_INFO_NONE)) {
    sprintf(err, "%s: couldn't copy axis info", me);
    biffMove(TEN, err, NRRD); return 1;
  }

  /* process all b vectors */
  vecSize = nin->axis[0].size*nrrdTypeSize[nin->type];
  vecNum = nrrdElementNumber(nin)/nin->axis[0].size;
  vecLup = nrrdDLookup[nin->type];
  vec = (char*)nin->data;
  out = (double*)nout->data;
  for (vecI=0; vecI<vecNum; vecI++) {
    /* copy DWI signal values */
    for (ii=0; ii<nin->axis[0].size; ii++) {
      ss[ii] = vecLup(vec, ii);
    }
    /* start with linear fit */
    tenBVecNonLinearFit_linear(&amp, &dec, bb, ss, ww, nin->axis[0].size);
    error = tenBVecNonLinearFit_error(bb, ss, ww, nin->axis[0].size, amp, dec);
    /* possibly refine with gauss-newton */
    if (iterMax > 0) {
      iter = 0;
      do {
        iter++;
        tenBVecNonLinearFit_GNstep(&d_amp, &d_dec,
                                   bb, ss, ww, nin->axis[0].size, amp, dec);
        amp += 0.3*d_amp;
        dec += 0.3*d_dec;
        diff = d_amp*d_amp + d_dec*d_dec;
      } while (iter < iterMax && diff > eps);
    }
    error = tenBVecNonLinearFit_error(bb, ss, ww, nin->axis[0].size, amp, dec);
    out[0] = amp;
    out[1] = dec;
    out[2] = error;
    vec += vecSize;
    out += 3;
  }
  
  return 0;
}

