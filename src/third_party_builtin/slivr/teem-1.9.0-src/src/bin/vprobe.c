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


#include <stdio.h>

#include <teem/biff.h>
#include <teem/hest.h>
#include <teem/nrrd.h>
#include <teem/gage.h>
#include <teem/ten.h>

#define SPACING(spc) (AIR_EXISTS(spc) ? spc: nrrdDefaultSpacing)

/* copied this from ten.h; I don't want gage to depend on ten */
#define PROBE_MAT2LIST(l, m) ( \
   (l)[1] = (m)[0],          \
   (l)[2] = (m)[3],          \
   (l)[3] = (m)[6],          \
   (l)[4] = (m)[4],          \
   (l)[5] = (m)[7],          \
   (l)[6] = (m)[8] )

int
probeParseKind(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  char me[] = "probeParseKind";
  gageKind **kindP;
  
  if (!(ptr && str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  kindP = (gageKind **)ptr;
  airToLower(str);
  if (!strcmp("scalar", str)) {
    *kindP = gageKindScl;
  } else if (!strcmp("vector", str)) {
    *kindP = gageKindVec;
  } else if (!strcmp("tensor", str)) {
    *kindP = tenGageKind;
  } else if (!strcmp("dwi", str)) {
    *kindP = tenDwiGageKindNew();
  } else {
    sprintf(err, "%s: not \"scalar\", \"vector\", \"tensor\", or \"dwi\"", me);
    return 1;
  }

  return 0;
}

void *
probeParseKindDestroy(void *ptr) {
  gageKind *kind;
  
  if (ptr) {
    kind = AIR_CAST(gageKind *, ptr);
    if (!strcmp(TEN_DWI_GAGE_KIND_NAME, kind->name)) {
      tenDwiGageKindNix(kind);
    }
  }
  return NULL;
}

hestCB probeKindHestCB = {
  sizeof(gageKind *),
  "kind",
  probeParseKind,
  probeParseKindDestroy
}; 

char *probeInfo = ("Shows off the functionality of the gage library. "
                   "Uses gageProbe() to query various kinds of volumes "
                   "to learn various measured or derived quantities. ");

int
main(int argc, char *argv[]) {
  gageKind *kind;
  char *me, *outS, *whatS, *err;
  hestParm *hparm;
  hestOpt *hopt = NULL;
  NrrdKernelSpec *k00, *k11, *k22;
  float gmc;
  int what, E=0, otype, renorm;
  unsigned int iBaseDim, oBaseDim;
  const gage_t *answer;
  const char *key=NULL;
  Nrrd *nin, *nout, *_nmat, *nmat;
  Nrrd *ngrad=NULL, *nbmat=NULL, *ntocrop=NULL, *ntmp=NULL;
  size_t cropMin[2], cropMax[2], ai, ansLen,
    idx, xi, yi, zi, six, siy, siz, sox, soy, soz;
  double bval=0;
  gageContext *ctx;
  gagePerVolume *pvl;
  double t0, t1, mat[16], ipos[4], opos[4], spx, spy, spz, x, y, z, scale[3];
  airArray *mop;

  mop = airMopNew();
  me = argv[0];
  hparm = hestParmNew();
  airMopAdd(mop, hparm, AIR_CAST(airMopper, hestParmFree), airMopAlways);
  hparm->elideSingleOtherType = AIR_TRUE;
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "k", "kind", airTypeOther, 1, 1, &kind, NULL,
             "\"kind\" of volume (\"scalar\", \"vector\", "
             "\"tensor\", or \"dwi\")",
             NULL, NULL, &probeKindHestCB);
  hestOptAdd(&hopt, "q", "query", airTypeString, 1, 1, &whatS, NULL,
             "the quantity (scalar, vector, or matrix) to learn by probing");
  hestOptAdd(&hopt, "s", "sclX sclY sxlZ", airTypeDouble, 3, 3, scale,
             "1.0 1.0 1.0",
             "scaling factor for resampling on each axis "
             "(>1.0 : supersampling)");
  hestOptAdd(&hopt, "k00", "kern00", airTypeOther, 1, 1, &k00,
             "tent", "kernel for gageKernel00",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k11", "kern11", airTypeOther, 1, 1, &k11,
             "cubicd:1,0", "kernel for gageKernel11",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k22", "kern22", airTypeOther, 1, 1, &k22,
             "cubicdd:1,0", "kernel for gageKernel22",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "rn", NULL, airTypeInt, 0, 0, &renorm, NULL,
             "renormalize kernel weights at each new sample location. "
             "\"Accurate\" kernels don't need this; doing it always "
             "makes things go slower");
  hestOptAdd(&hopt, "gmc", "min gradmag", airTypeFloat, 1, 1, &gmc,
             "0.0", "For curvature-based queries, use zero when gradient "
             "magnitude is below this");
  hestOptAdd(&hopt, "m", "matrix", airTypeOther, 1, 1, &_nmat, "",
             "transform matrix to map volume through "
             "(actually the probe locations are sent through "
             "its inverse).  By default, there is no transform",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "t", "type", airTypeEnum, 1, 1, &otype, "float",
             "type of output volume", NULL, nrrdType);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output volume");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, probeInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, AIR_CAST(airMopper, hestOptFree), airMopAlways);
  airMopAdd(mop, hopt, AIR_CAST(airMopper, hestParseFree), airMopAlways);

  what = airEnumVal(kind->enm, whatS);
  if (-1 == what) {
    /* -1 indeed always means "unknown" for any gageKind */
    fprintf(stderr, "%s: couldn't parse \"%s\" as measure of \"%s\" volume\n",
            me, whatS, kind->name);
    hestUsage(stderr, hopt, me, hparm);
    hestGlossary(stderr, hopt, hparm);
    airMopError(mop);
    return 1;
  }

  /* special set-up required for DWI kind */
  if (!strcmp(TEN_DWI_GAGE_KIND_NAME, kind->name)) {
#if 0
    if (tenDWMRIKeyValueParse(&ngrad, &nbmat, &bval, nin)) {
      airMopAdd(mop, err = biffGetDone(TEN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble parsing DWI info:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
    ntocrop = ngrad ? ngrad : nbmat;
    cropMin[0] = 0;
    cropMin[1] = 1;
    cropMax[0] = ntocrop->axis[0].size-1;
    cropMax[1] = ntocrop->axis[1].size-1;
    airMopAdd(mop, ntmp = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    E = 0;
    key = NRRD;
    if (!E) E |= nrrdCrop(ntmp, ntocrop, cropMin, cropMax);
    key = TEN;
    if (ngrad) {
      if (!E) E |= tenDwiGageKindGradients(kind, bval, ntmp);
      if (!E) airMopAdd(mop, ngrad, (airMopper)nrrdNuke, airMopAlways);
    } else {
      if (!E) E |= tenDwiGageKindBMatrices(kind, bval, ntmp);
      if (!E) airMopAdd(mop, nbmat, (airMopper)nrrdNuke, airMopAlways);
    }
    if (!E) E |= tenDwiGageKindFitType(kind, tenDwiGageFitTypeLinear);
    if (!E) E |= tenDwiGageKindConfThreshold(kind, 100, 0);
    if (E) {
      airMopAdd(mop, err = biffGetDone(key), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble setting grad/bmat info:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
#else
    AIR_UNUSED(key);
    AIR_UNUSED(ngrad);
    AIR_UNUSED(nbmat);
    AIR_UNUSED(ntocrop);
    AIR_UNUSED(ntmp);
    AIR_UNUSED(cropMin);
    AIR_UNUSED(cropMax);
    AIR_UNUSED(bval);
    fprintf(stderr, "%s: sorry, tenDwiGageKind currently broken\n", me);
#endif
  }

  if (_nmat) {
    if (!( 2 == _nmat->dim 
           && 4 == _nmat->axis[0].size && 4 == _nmat->axis[1].size )) {
      fprintf(stderr, "%s: matrix needs to be a 2D 4x4 array\n", me);
      airMopError(mop);
      return 1;
    }
    nmat = nrrdNew();
    airMopAdd(mop, nmat, AIR_CAST(airMopper, nrrdNuke), airMopAlways);
    if (nrrdConvert(nmat, _nmat, nrrdTypeDouble)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble:\n%s\n", me, err);
      airMopError(mop);
      return 1;
    }
    ell_4m_inv_d(mat, (double*)(nmat->data));
    /* ell_4m_print_d(stderr, mat); */
  } else {
    ELL_4M_IDENTITY_SET(mat);
  }

  ansLen = kind->table[what].answerLength;
  iBaseDim = kind->baseDim;
  oBaseDim = 1 == ansLen ? 0 : 1;
  six = nin->axis[0+iBaseDim].size;
  siy = nin->axis[1+iBaseDim].size;
  siz = nin->axis[2+iBaseDim].size;
  spx = SPACING(nin->axis[0+iBaseDim].spacing);
  spy = SPACING(nin->axis[1+iBaseDim].spacing);
  spz = SPACING(nin->axis[2+iBaseDim].spacing);
  sox = AIR_CAST(size_t, scale[0]*six);
  soy = AIR_CAST(size_t, scale[1]*siy);
  soz = AIR_CAST(size_t, scale[2]*siz);
  nin->axis[0+iBaseDim].spacing = SPACING(nin->axis[0+iBaseDim].spacing);
  nin->axis[1+iBaseDim].spacing = SPACING(nin->axis[1+iBaseDim].spacing);
  nin->axis[2+iBaseDim].spacing = SPACING(nin->axis[2+iBaseDim].spacing);

  /***
  **** Except for the gageProbe() call in the inner loop below,
  **** and the gageContextNix() call at the very end, all the gage
  **** calls which set up (and take down) the context and state are here.
  ***/
  ctx = gageContextNew();
  airMopAdd(mop, ctx, AIR_CAST(airMopper, gageContextNix), airMopAlways);
  gageParmSet(ctx, gageParmGradMagCurvMin, gmc);
  gageParmSet(ctx, gageParmVerbose, 1);
  gageParmSet(ctx, gageParmRenormalize,
              AIR_CAST(gage_t, renorm ? AIR_TRUE : AIR_FALSE));
  gageParmSet(ctx, gageParmCheckIntegrals, AIR_TRUE);
  E = 0;
  if (!E) E |= !(pvl = gagePerVolumeNew(ctx, nin, kind));
  if (!E) E |= gagePerVolumeAttach(ctx, pvl);
  if (!E) E |= gageKernelSet(ctx, gageKernel00, k00->kernel, k00->parm);
  if (!E) E |= gageKernelSet(ctx, gageKernel11, k11->kernel, k11->parm); 
  if (!E) E |= gageKernelSet(ctx, gageKernel22, k22->kernel, k22->parm);
  if (!E) E |= gageQueryItemOn(ctx, pvl, what);
  if (!E) E |= gageUpdate(ctx);
  if (E) {
    airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }
  answer = gageAnswerPointer(ctx, pvl, what);
  gageParmSet(ctx, gageParmVerbose, 0);
  /***
  **** end gage setup.
  ***/

  fprintf(stderr, "%s: kernel support = %d^3 samples\n", me,
          2*ctx->radius);
  fprintf(stderr, "%s: effective scaling is %g %g %g\n", me,
          AIR_CAST(double, sox)/six,
          AIR_CAST(double, soy)/siy,
          AIR_CAST(double, soz)/siz);
  if (ansLen > 1) {
    fprintf(stderr, "%s: creating " _AIR_SIZE_T_CNV " x " _AIR_SIZE_T_CNV
            " x " _AIR_SIZE_T_CNV " x " _AIR_SIZE_T_CNV " output\n", 
            me, ansLen, sox, soy, soz);
    if (!E) E |= nrrdMaybeAlloc_va(nout=nrrdNew(), otype, 4,
                                   ansLen, sox, soy, soz);
  } else {
    fprintf(stderr, "%s: creating " _AIR_SIZE_T_CNV " x " _AIR_SIZE_T_CNV
            " x " _AIR_SIZE_T_CNV " output\n", me, sox, soy, soz);
    if (!E) E |= nrrdMaybeAlloc_va(nout=nrrdNew(), otype, 3,
                                   sox, soy, soz);
  }
  airMopAdd(mop, nout, AIR_CAST(airMopper, nrrdNuke), airMopAlways);
  if (E) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }
  t0 = airTime();
  for (zi=0; zi<=soz-1; zi++) {
    fprintf(stderr, " " _AIR_SIZE_T_CNV "/" _AIR_SIZE_T_CNV,
            zi, soz-1); fflush(stderr);
    z = AIR_AFFINE(0, zi, soz-1, 0, siz-1);
    for (yi=0; yi<=soy-1; yi++) {
      y = AIR_AFFINE(0, yi, soy-1, 0, siy-1);
      for (xi=0; xi<=sox-1; xi++) {
        x = AIR_AFFINE(0, xi, sox-1, 0, six-1);
        idx = xi + sox*(yi + soy*zi);
        ctx->verbose = 0*( (!xi && !yi && !zi) ||
                           /* ((100 == xi) && (8 == yi) && (8 == zi)) */
                           ((61 == xi) && (51 == yi) && (46 == zi))
                           /* ((40==xi) && (30==yi) && (62==zi)) || */
                           /* ((40==xi) && (30==yi) && (63==zi)) */ );
        
        ELL_4V_SET(opos, x*spx, y*spy, z*spz, 1);
        ELL_4MV_MUL(ipos, mat, opos);
        /*
        fprintf(stderr, "%s: (%g,%g,%g) --> (%g,%g,%g)\n", 
                me, opos[0], opos[1], opos[2], ipos[0], ipos[1], ipos[2]);
        */
        ELL_4V_HOMOG(ipos, ipos);
        ipos[0] = AIR_CLAMP(0, ipos[0]/spx, six-1);
        ipos[1] = AIR_CLAMP(0, ipos[1]/spy, siy-1);
        ipos[2] = AIR_CLAMP(0, ipos[2]/spz, siz-1);
        /*
        fprintf(stderr, "%s: (%g,%g,%g) --> (%g,%g,%g)\n", 
                me, x, y, z, ipos[0], ipos[1], ipos[2]);
        */
        
        if (gageProbe(ctx,
                      AIR_CAST(gage_t, ipos[0]),
                      AIR_CAST(gage_t, ipos[1]),
                      AIR_CAST(gage_t, ipos[2]))) {
          fprintf(stderr, 
                  "%s: trouble at i=(" _AIR_SIZE_T_CNV "," _AIR_SIZE_T_CNV
                  "," _AIR_SIZE_T_CNV ") -> f=(%g,%g,%g):\n%s\n(%d)\n",
                  me, xi, yi, zi, ipos[0], ipos[1], ipos[2],
                  ctx->errStr, ctx->errNum);
          airMopError(mop);
          return 1;
        }
        if (1 == ansLen) {
          nrrdFInsert[nout->type](nout->data, idx,
                                  nrrdFClamp[nout->type](*answer));
        } else {
          for (ai=0; ai<=ansLen-1; ai++) {
            nrrdFInsert[nout->type](nout->data, ai + ansLen*idx,
                                    nrrdFClamp[nout->type](answer[ai]));
          }
        }
      }
    }
  }

  /* HEY: this isn't actually correct in general, but is true
     for gageKindScl and gageKindVec */
  nrrdContentSet_va(nout, "probe", nin, "%s", airEnumStr(kind->enm, what));
  nout->axis[0+oBaseDim].spacing = 
    ((double)six/sox)*SPACING(nin->axis[0+iBaseDim].spacing);
  nout->axis[0+oBaseDim].label = airStrdup(nin->axis[0+iBaseDim].label);
  nout->axis[1+oBaseDim].spacing = 
    ((double)six/sox)*SPACING(nin->axis[1+iBaseDim].spacing);
  nout->axis[1+oBaseDim].label = airStrdup(nin->axis[1+iBaseDim].label);
  nout->axis[2+oBaseDim].spacing = 
    ((double)six/sox)*SPACING(nin->axis[2+iBaseDim].spacing);
  nout->axis[2+oBaseDim].label = airStrdup(nin->axis[2+iBaseDim].label);

  fprintf(stderr, "\n");
  t1 = airTime();
  fprintf(stderr, "probe rate = %g KHz\n", sox*soy*soz/(1000.0*(t1-t0)));
  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble saving output:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }

  airMopOkay(mop);
  return 0;
}
