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
  } else {
    sprintf(err, "%s: not \"scalar\", \"vector\", or \"tensor\"", me);
    return 1;
  }

  return 0;
}

hestCB probeKindHestCB = {
  sizeof(gageKind *),
  "kind",
  probeParseKind,
  NULL
}; 

void
printans(FILE *file, const gage_t *ans, int len) {
  int a;

  AIR_UNUSED(file);
  for (a=0; a<=len-1; a++) {
    if (a) {
      printf(", ");
    }
    printf("%g", ans[a]);
  }
}

char *probeInfo = ("Uses gageProbe() to query scalar or vector volumes "
                   "at a single probe location.");

int
main(int argc, char *argv[]) {
  gageKind *kind;
  char *me, *whatS, *err;
  hestParm *hparm;
  hestOpt *hopt = NULL;
  NrrdKernelSpec *k00, *k11, *k22;
  float pos[3], gmc;
  int what, ansLen, E=0, iBaseDim, renorm;
  const gage_t *answer, *answer2;
  Nrrd *nin;
  gageContext *ctx, *ctx2;
  gagePerVolume *pvl;
  airArray *mop;

  mop = airMopNew();
  me = argv[0];
  hparm = hestParmNew();
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hparm->elideSingleOtherType = AIR_TRUE;
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "k", "kind", airTypeOther, 1, 1, &kind, NULL,
             "\"kind\" of volume (\"scalar\", \"vector\", or \"tensor\")",
             NULL, NULL, &probeKindHestCB);
  hestOptAdd(&hopt, "p", "x y z", airTypeFloat, 3, 3, pos, NULL,
             "the position in index space at which to probe");
  hestOptAdd(&hopt, "q", "query", airTypeString, 1, 1, &whatS, NULL,
             "the quantity (scalar, vector, or matrix) to learn by probing");
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
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, probeInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

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

  ansLen = kind->table[what].answerLength;
  iBaseDim = kind->baseDim;
  nin->axis[0+iBaseDim].spacing = SPACING(nin->axis[0+iBaseDim].spacing);
  nin->axis[1+iBaseDim].spacing = SPACING(nin->axis[1+iBaseDim].spacing);
  nin->axis[2+iBaseDim].spacing = SPACING(nin->axis[2+iBaseDim].spacing);

  ctx = gageContextNew();
  airMopAdd(mop, ctx, (airMopper)gageContextNix, airMopAlways);
  gageParmSet(ctx, gageParmVerbose, 42);
  gageParmSet(ctx, gageParmGradMagMin, gmc);
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

  /* test with original context */
  answer = gageAnswerPointer(ctx, ctx->pvl[0], what);
  if (gageProbe(ctx, pos[0], pos[1], pos[2])) {
    fprintf(stderr, "%s: trouble:\n%s\n(%d)\n", me, ctx->errStr, ctx->errNum);
    airMopError(mop);
    return 1;
  }
  printf("%s: %s(%g,%g,%g) = ", me,
         airEnumStr(kind->enm, what), pos[0], pos[1], pos[2]);
  printans(stdout, answer, ansLen);
  printf("\n");


  if (0) {
    /* test with copied context */
    if (!(ctx2 = gageContextCopy(ctx))) {
      airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble:\n%s\n", me, err);
      airMopError(mop);
      return 1;
    }
    airMopAdd(mop, ctx2, (airMopper)gageContextNix, airMopAlways);
    answer2 = gageAnswerPointer(ctx, ctx2->pvl[0], what);
    if (gageProbe(ctx2, pos[0], pos[1], pos[2])) {
      fprintf(stderr, "%s: trouble:\n%s\n(%d)\n", me,
              ctx->errStr, ctx->errNum);
      airMopError(mop);
      return 1;
    }
    printf("====== B %s: %s(%g,%g,%g) = ", me,
           airEnumStr(kind->enm, what), pos[0], pos[1], pos[2]);
    printans(stdout, answer2, ansLen);
    printf("\n");
    
    /* random testing */
    ELL_3V_SET(pos, 1.2f, 2.3f, 3.4f);
    gageProbe(ctx2, pos[0], pos[1], pos[2]);
    printf("====== C %s: %s(%g,%g,%g) = ", me, airEnumStr(kind->enm, what),
           pos[0], pos[1], pos[2]);
    printans(stdout, answer2, ansLen);
    printf("\n");
    
    ELL_3V_SET(pos, 4.4f, 5.5f, 6.6f);
    gageProbe(ctx, pos[0], pos[1], pos[2]);
    printf("====== D %s: %s(%g,%g,%g) = ", me, airEnumStr(kind->enm, what),
           pos[0], pos[1], pos[2]);
    printans(stdout, answer, ansLen);
    printf("\n");
    
    ELL_3V_SET(pos, 1.2f, 2.3f, 3.4f);
    gageProbe(ctx, pos[0], pos[1], pos[2]);
    printf("====== E %s: %s(%g,%g,%g) = ", me, airEnumStr(kind->enm, what),
           pos[0], pos[1], pos[2]);
    printans(stdout, answer, ansLen);
    printf("\n");
    
    ELL_3V_SET(pos, 1.2f, 2.3f, 3.4f);
    gageProbe(ctx2, pos[0], pos[1], pos[2]);
    printf("====== F %s: %s(%g,%g,%g) = ", me, airEnumStr(kind->enm, what),
           pos[0], pos[1], pos[2]);
    printans(stdout, answer2, ansLen);
    printf("\n");
    
  }

  airMopOkay(mop);
  return 0;
}
