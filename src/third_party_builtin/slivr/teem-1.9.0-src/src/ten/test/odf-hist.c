/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005 Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998 University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
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


#include "../ten.h"

char *info = ("The histogram craziness continues.");

int
main(int argc, char *argv[]) {
  char *me;
  hestOpt *hopt=NULL;
  airArray *mop;

  char *errS, *outS, *covarS;
  Nrrd *_nodf, *nvec, *nhist, *ncovar;
  int bins;
  size_t size[NRRD_DIM_MAX];
  float min;
  
  mop = airMopNew();
  me = argv[0];

  hestOptAdd(&hopt, "i", "odf", airTypeOther, 1, 1, &_nodf, NULL,
             "ODF volume to analyze", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "v", "odf", airTypeOther, 1, 1, &nvec, NULL,
             "list of vectors by which odf is sampled",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "min", "min", airTypeFloat, 1, 1, &min, "0.0",
             "ODF values below this are ignored, and per-voxel ODF is "
             "normalized to have sum 1.0.  Use \"nan\" to subtract out "
             "the per-voxel min.");
  hestOptAdd(&hopt, "b", "bins", airTypeInt, 1, 1, &bins, "128",
             "number of bins in histograms");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output file");
  hestOptAdd(&hopt, "co", "covariance out", airTypeString, 1, 1,
             &covarS, "covar.nrrd", "covariance output file");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  if (!( nrrdTypeFloat == nvec->type )) {
    fprintf(stderr, "%s vector type (%s) not %s\n", me,
            airEnumStr(nrrdType, nvec->type),
            airEnumStr(nrrdType, nrrdTypeFloat));
    airMopError(mop); return 1;
  }
  if (!( 2 == nvec->dim && 3 == nvec->axis[0].size )) {
    fprintf(stderr, "%s: nvec not a 2-D 3-by-N array\n", me);
    airMopError(mop); return 1;
  }
  if (!( _nodf->axis[0].size == nvec->axis[1].size )) {
    fprintf(stderr, "%s mismatch of _nodf->axis[0].size (%d) vs. "
            "nvec->axis[1].size (%d)\n", me, 
            (int)_nodf->axis[0].size, (int)nvec->axis[1].size);
    airMopError(mop); return 1;
  }
  nrrdAxisInfoGet_nva(_nodf, nrrdAxisInfoSize, size);
  size[0] = bins;
  nhist = nrrdNew();
  airMopAdd(mop, nhist, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdMaybeAlloc_nva(nhist, nrrdTypeFloat, _nodf->dim, size)) {
    airMopAdd(mop, errS = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble allocating output:\n%s", me, errS);
    airMopError(mop); return 1;
  }
  ncovar = nrrdNew();
  airMopAdd(mop, ncovar, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdMaybeAlloc_va(ncovar, nrrdTypeFloat, 2,
                        AIR_CAST(size_t, bins),
                        AIR_CAST(size_t, bins))) {
    airMopAdd(mop, errS = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble allocating covariance output:\n%s", me, errS);
    airMopError(mop); return 1;
  }

  {
    /* we modify the lengths of the vectors here */
    int NN, VV, ii, jj=0, kk, *anglut;
    float *odf, *hist, *covar, *vec, *vi, *vj, tmp, pvmin;
    double *mean;
    Nrrd *nodf, *nanglut;

    VV = nvec->axis[1].size;
    NN = nrrdElementNumber(_nodf)/VV;

    nanglut = nrrdNew();
    airMopAdd(mop, nanglut, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdMaybeAlloc_va(nanglut, nrrdTypeInt, 2,
                          AIR_CAST(size_t, VV),
                          AIR_CAST(size_t, VV))) {
      airMopAdd(mop, errS = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble allocating lookup table:\n%s", me, errS);
      airMopError(mop); return 1;
    }
    if (nrrdTypeFloat == _nodf->type) {
      nodf = _nodf;
    } else {
      nodf = nrrdNew();
      airMopAdd(mop, nodf, (airMopper)nrrdNuke, airMopAlways);
      if (nrrdConvert(nodf, _nodf, nrrdTypeFloat)) {
        airMopAdd(mop, errS = biffGetDone(NRRD), airFree, airMopAlways);
        fprintf(stderr, "%s: trouble converting input:\n%s", me, errS);
        airMopError(mop); return 1;
      }
    }      
    
    /* normalize lengths (MODIFIES INPUT) */
    vec = (float*)nvec->data;
    for (ii=0; ii<=jj; ii++) {
      vi = vec + 3*ii;
      ELL_3V_NORM(vi, vi, tmp);
    }
    
    /* pre-compute pair-wise angles */
    anglut = (int*)nanglut->data;
    for (jj=0; jj<VV; jj++) {
      vj = vec + 3*jj;
      for (ii=0; ii<=jj; ii++) {
        vi = vec + 3*ii;
        tmp = ELL_3V_DOT(vi, vj);
        tmp = AIR_ABS(tmp);
        tmp = acos(tmp)/(AIR_PI/2.0);
        anglut[ii + VV*jj] = airIndex(0.0, tmp, 1.0, bins);
      }
    }

    /* process all samples (MODIFIES INPUT if input was already float) */
    odf = (float*)nodf->data;
    hist = (float*)nhist->data;
    for (kk=0; kk<NN; kk++) {
      if (!(kk % 100)) {
        fprintf(stderr, "%d/%d\n", kk, NN);
      }
      tmp = 0;
      if (AIR_EXISTS(min)) {
        for (ii=0; ii<VV; ii++) {
          odf[ii] = AIR_MAX(0.0, odf[ii]-min);
          tmp += odf[ii];
        }
      } else {
        /* we do the more sketchy per-voxel min subtraction */
        pvmin = airFPGen_f(airFP_POS_INF);
        for (ii=0; ii<VV; ii++) {
          pvmin = AIR_MIN(pvmin, odf[ii]);
        }
        for (ii=0; ii<VV; ii++) {
          odf[ii] -= pvmin;
          tmp += odf[ii];
        }
      }
      if (tmp) {
        /* something left after subtracting out baseline isotropic */
        for (ii=0; ii<VV; ii++) {
          odf[ii] /= tmp;
        }
        /* odf[] is normalized to 1.0 sum */
        for (jj=0; jj<VV; jj++) {
          for (ii=0; ii<=jj; ii++) {
            tmp = odf[ii]*odf[jj];
            hist[anglut[ii + VV*jj]] += tmp;
          }
        }
      }
      odf += VV;
      hist += bins;
    }
    odf = NULL;
    hist = NULL;

    /* find mean value of each bin (needed for covariance) */
    mean = (double*)calloc(bins, sizeof(double));
    if (!mean) {
      fprintf(stderr, "%s: couldn't allocate mean array", me);
      airMopError(mop); return 1;
    }
    hist = (float*)nhist->data;
    for (kk=0; kk<NN; kk++) {
      for (ii=0; ii<bins; ii++) {
        mean[ii] += hist[ii];
      }
      hist += bins;
    }
    hist = NULL;
    for (ii=0; ii<bins; ii++) {
      mean[ii] /= NN;
    }

    /* make covariance matrix of from all histograms */
    covar = (float*)ncovar->data;
    hist = (float*)nhist->data;
    for (kk=0; kk<NN; kk++) {
      for (jj=0; jj<bins; jj++) {
        for (ii=0; ii<jj; ii++) {
          tmp = (hist[ii] - mean[ii])*(hist[jj] - mean[jj]);
          covar[ii + bins*jj] += tmp;
          covar[jj + bins*ii] += tmp;
        }
        covar[jj + bins*jj] += (hist[jj] - mean[jj])*(hist[jj] - mean[jj]);
      }
      hist += bins;
    }
    hist = NULL;
  }

  if (nrrdSave(outS, nhist, NULL)) {
    airMopAdd(mop, errS = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't save output:\n%s\n", me, errS);
    airMopError(mop); return 1;
  }
  if (nrrdSave(covarS, ncovar, NULL)) {
    airMopAdd(mop, errS = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't save covariance output:\n%s\n", me, errS);
    airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}

