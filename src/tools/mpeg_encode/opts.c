/*===========================================================================*
 * opts.c                                     *
 *                                         *
 *      Special C code to handle TUNEing options                             *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *      Tune_Init                                                            *
 *      CollectQuantStats                                                    *
 *                                         *
 *===========================================================================*/


/*
 * Copyright (c) 1995 The Regents of the University of California.
 * All rights reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*==============*
 * HEADER FILES *
 *==============*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "opts.h"

/*==============*
 * EXTERNALS    *
 *==============*/

extern char    outputFileName[];
extern boolean pureDCT;
extern int32_mpeg_t   qtable[], niqtable[];
extern int     ZAG[];
extern boolean printSNR, decodeRefFrames;

void init_idctref _ANSI_ARGS_((void));
void init_fdct _ANSI_ARGS_((void));


/*===================*
 * GLOBALS MADE HERE *
 *===================*/

boolean tuneingOn = FALSE;
int block_bound = 128;
boolean collect_quant = FALSE;
int collect_quant_detailed = 0;
FILE *collect_quant_fp;
int kill_dim = FALSE;
int kill_dim_break, kill_dim_end;
float kill_dim_slope;
int SearchCompareMode = DEFAULT_SEARCH;
boolean squash_small_differences = FALSE;
int SquashMaxLum, SquashMaxChr;
float LocalDCTRateScale = 1.0, LocalDCTDistortScale = 1.0;
boolean IntraPBAllowed = TRUE;
boolean WriteDistortionNumbers = FALSE;
int collect_distortion_detailed = 0;
FILE *distortion_fp;
FILE *fp_table_rate[31], *fp_table_dist[31];
boolean DoLaplace = FALSE;
double **L1, **L2, **Lambdas;
int LaplaceNum, LaplaceCnum;
boolean BSkipBlocks = TRUE;

/*====================*
 * Internal Prototypes*
 *====================*/
void    SetupCollectQuantStats _ANSI_ARGS_((char *charPtr));
void    SetupSquashSmall _ANSI_ARGS_ ((char *charPtr));
void    SetupKillDimAreas _ANSI_ARGS_((char *charPtr));
void    SetupLocalDCT _ANSI_ARGS_((char *charPtr));
void    SetupWriteDistortions _ANSI_ARGS_((char *charPtr));
void    SetupLaplace _ANSI_ARGS_((void));
void    CalcLambdas  _ANSI_ARGS_((void));
void    Mpost_UnQuantZigBlockLaplace _ANSI_ARGS_((FlatBlock in, Block out, int qscale, boolean iblock));

/* define this as it too much of a pain to find toupper on different arch'es */
#define ASCII_TOUPPER(c) ((c>='a') && (c<='z')) ? c-'a'+'A' : c

/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/

/*===========================================================================*
 *
 * Tune_Init
 *
 *     Do any setup needed before coding stream
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:  varies
 *
 *===========================================================================*/
void Tune_Init()
{
  int i;

  /* Just check for each, and do whats needed */
  if (collect_quant) {
    if (!pureDCT) {
      pureDCT = TRUE;
      init_idctref();
      init_fdct();
    }
    fprintf(collect_quant_fp, "# %s\n", outputFileName);
    fprintf(collect_quant_fp, "#");
    for (i=0; i<64; i++) 
      fprintf(collect_quant_fp, " %d", qtable[i]);
    fprintf(collect_quant_fp, "\n#");
    for (i=0; i<64; i++) 
      fprintf(collect_quant_fp, " %d", niqtable[i]);
    fprintf(collect_quant_fp, "\n# %d %d %d\n\n", 
        GetIQScale(), GetPQScale(), GetBQScale());
    
  }

  if (DoLaplace) {
    if (!pureDCT) {
      pureDCT = TRUE;
      init_idctref();
      init_fdct();
    }
    decodeRefFrames = TRUE;
    printSNR = TRUE;
  }
    
}

/*===========================================================================*
 *
 * ParseTuneParam
 *
 *     Handle the strings following TUNE
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:  varies
 *
 *===========================================================================*/
void ParseTuneParam(charPtr)
char *charPtr;
{
  switch (ASCII_TOUPPER(*charPtr)) {
  case 'B': 
    if (1 != sscanf(charPtr+2, "%d", &block_bound)) {
      fprintf(stderr, "Invalid tuning parameter (b) in parameter file.\n");
    }
    break;
  case 'C':
    SetupCollectQuantStats(charPtr+2);
    break;
  case 'D':
    SetupLocalDCT(SkipSpacesTabs(charPtr+1));
    break;
  case 'K':
    SetupKillDimAreas(SkipSpacesTabs(charPtr+1));
    break;
  case 'L':
    SetupLaplace();
    break;
  case 'N':
    SearchCompareMode = NO_DC_SEARCH;
    break;
  case 'Q':
    SearchCompareMode = DO_Mean_Squared_Distortion;
    break;
  case 'S':
    SetupSquashSmall(SkipSpacesTabs(charPtr+1));
    break;
  case 'W':
    SetupWriteDistortions(SkipSpacesTabs(charPtr+1));
    break;
  case 'U':
    BSkipBlocks = FALSE;
    break;
  case 'Z':
     IntraPBAllowed = FALSE;
    break;
  default:
    fprintf(stderr, "Unknown tuning (%s) in parameter file.\n",charPtr);
    break;
  }
}


/*===============*
 * Internals     *
 *===============*/

/*===========================================================================*
 *
 * SetupCollectQuantStats
 *
 *     Setup variables to collect statistics on quantization values
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    sets collect_quant and collect_quant_fp
 *
 *===========================================================================*/
void SetupCollectQuantStats(charPtr)
char *charPtr;
{
  char fname[256], *cp;

  cp = charPtr;
  while ( (*cp != ' ') && (*cp != '\t') && (*cp != '\n')) {
    cp++;
  }

  strncpy(fname, charPtr, cp-charPtr);
  fname[cp-charPtr] = '\0';
  collect_quant = TRUE;
  if ((collect_quant_fp = fopen(fname,"w")) == NULL) {
    fprintf(stderr, "Error opening %s for quant statistics\n", fname);
    fprintf(stderr, "Using stdout (ick!)\n");
    collect_quant_fp = stdout;
  }

  cp = SkipSpacesTabs(cp);
  if (*cp != '\n') {
    switch (*cp) {
    case 'c':
      collect_quant_detailed = 1;
      break;
    default:
      fprintf(stderr, "Unknown TUNE parameter setting format %s\n", cp);
    }}
}




/*===========================================================================*
 *
 * SetupKillDimAreas
 *
 *     Do a transform on small lum values
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    sets kill_dim, kill_dim_break, kill_dim_end
 *
 *===========================================================================*/
void SetupKillDimAreas(charPtr)
char *charPtr;
{
  int items_scanned;

  kill_dim = TRUE;
  items_scanned = sscanf(charPtr, "%d %d %f", 
             &kill_dim_break, &kill_dim_end, &kill_dim_slope);
  if (items_scanned != 3) {
    kill_dim_slope = 0.25;
    items_scanned = sscanf(charPtr, "%d %d", 
               &kill_dim_break, &kill_dim_end);
    if (items_scanned != 2) {
      /* Use defaults */
      kill_dim_break = 20;
      kill_dim_end   = 25;
    }
  }
  /* check values */
  if (kill_dim_break > kill_dim_end) {
    fprintf(stderr, "TUNE parameter k: break > end is illegal.\n");
    exit(-1);
  }
  if (kill_dim_slope < 0) {
    fprintf(stderr, "TUNE parameter k: slope < 0 is illegal.\n");
    exit(-1);
  }
}



/*===========================================================================*
 *
 * SetupSquashSmall
 *
 *     Setup encoder to squash small changes in Y or Cr/Cb values
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    sets squash_max_differences SquashMaxLum SquashMaxChr 
 *
 *===========================================================================*/
void SetupSquashSmall(charPtr)
char *charPtr;
{
  squash_small_differences = TRUE;

  if (sscanf(charPtr, "%d %d", &SquashMaxLum, &SquashMaxChr) == 1) {
    /* Only set one, do both */
    SquashMaxChr = SquashMaxLum;
  }
}


/*===========================================================================*
 *
 * SetupLocalDCT
 *
 *     Setup encoder to use DCT for rate-distortion estimat ein Psearches
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    sets SearchCompareMode and
 *                        can change LocalDCTRateScale, LocalDCTDistortScale
 *
 *===========================================================================*/
void SetupLocalDCT(charPtr)
char *charPtr;
{
  int num_scales=0;

  SearchCompareMode = LOCAL_DCT;

  /* Set scaling factors if present */
  num_scales = sscanf(charPtr, "%f %f", &LocalDCTRateScale, &LocalDCTDistortScale);
  if (num_scales == 1) {
    fprintf(stderr, "Invalid number of scaling factors for local DCT\n");
    fprintf(stderr, "Must specify Rate Scale and Distorion scale (both floats)\n");
    fprintf(stderr, "Continuing with 1.0 1.0\n");
    LocalDCTRateScale = 1.0;
    LocalDCTDistortScale = 1.0;
  }
}


/*===========================================================================*
 *
 * SetupLaplace
 *
 *     Setup encoder to find distrubution for I-frames, and use for -snr
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    sets DoLaplace, L1, L2, and Lambdas
 *
 *===========================================================================*/
void SetupLaplace()
{
  int i;

  DoLaplace = TRUE;
  LaplaceNum = 0;
  L1 = (double **)malloc(sizeof(double *)*3);
  L2 = (double **)malloc(sizeof(double *)*3);
  Lambdas = (double **)malloc(sizeof(double *)*3);
  if (L1 == NULL || L2 == NULL || Lambdas == NULL) {
    fprintf(stderr,"Out of memory!!!\n");
    exit(1);
  }
  for (i = 0; i < 3; i++) {
    L1[i] = (double *)calloc(64, sizeof(double));
    L2[i] = (double *)calloc(64, sizeof(double));
    Lambdas[i] = (double *)malloc(sizeof(double) * 64);
    if (L1[i] == NULL || L2[i] == NULL || Lambdas[i] == NULL) {
      fprintf(stderr,"Out of memory!!!\n");
      exit(1);
    }
  }
}

void CalcLambdas()
{
  int i,j,n;
  double var;
  
  n = LaplaceNum;
  for (i = 0;   i < 3;  i++) {
    for (j = 0;  j < 64;  j++) {
      var = (n*L1[i][j] + L2[i][j]*L2[i][j]) / (n*(n-1));
      Lambdas[i][j] = sqrt(2.0) / sqrt(var);
    }
  }
}


/*===========================================================================*
 *
 * Mpost_UnQuantZigBlockLaplace
 *
 *    unquantize and zig-zag (decode) a single block, using the distrib to get vals
 *      Iblocks only now
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Mpost_UnQuantZigBlockLaplace(in, out, qscale, iblock)
    FlatBlock in;
    Block out;
    int qscale;
    boolean iblock;
{
    register int index;
    int        position;
    register int        qentry;
    int        level, coeff;
    double low, high;
    double mid,lam;

    /* qtable[0] must be 8 */
    out[0][0] = (int16_mpeg_t)(in[0] * 8);
    
    for ( index = 1;  index < DCTSIZE_SQ;  index++ ) {
      position = ZAG[index];
      level = in[index];
      
      if (level == 0) {
    ((int16_mpeg_t *)out)[position] = 0;
    continue;
      }
      qentry = qtable[position] * qscale;
      coeff = (level*qentry)/8;
      low = ((ABS(level)-.5)*qentry)/8;
      high = ((ABS(level)+.5)*qentry)/8;
      lam = Lambdas[LaplaceCnum][position];
      mid = (1.0/lam) * log(0.5*(exp(-lam*low)+exp(-lam*high)));
      mid = ABS(mid);
      if (mid - floor(mid) > .4999) {
    mid = ceil(mid);
      } else {
    mid = floor(mid);
      }
      if (level<0) {mid = -mid;}
/*printf("(%2.1lf-%2.1lf): old: %d vs %d\n",low,high,coeff,(int) mid);*/
      coeff = mid;
      if ( (coeff & 1) == 0 ) {
    if ( coeff < 0 ) {
      coeff++;
    } else if ( coeff > 0 ) {
      coeff--;
    }
      }
      ((int16_mpeg_t *)out)[position] = coeff;
    }
}

void
SetupWriteDistortions(charPtr)
char *charPtr;
{
  char fname[256], *cp;
  int i;

  WriteDistortionNumbers = TRUE;
  cp = charPtr;
  while ( (*cp != ' ') && (*cp != '\t') && (*cp != '\n')) {
    cp++;
  }

  strncpy(fname, charPtr, cp-charPtr);
  fname[cp-charPtr] = '\0';
  collect_quant = TRUE;
  if ((distortion_fp = fopen(fname,"w")) == NULL) {
    fprintf(stderr, "Error opening %s for quant statistics\n", fname);
    fprintf(stderr, "Using stdout (ick!)\n");
    distortion_fp = stdout;
  }

  cp = SkipSpacesTabs(cp);
  if (*cp != '\n') {
    switch (*cp) {
    case 'c':
      collect_distortion_detailed = TRUE;
      break;
    case 't': {
      char scratch[256];
      collect_distortion_detailed = 2;
      for (i = 1;  i < 32;  i++) {
    sprintf(scratch, "%srate%d", fname, i);
    fp_table_rate[i-1] = fopen(scratch, "w");
    sprintf(scratch, "%sdist%d", fname, i);
    fp_table_dist[i-1] = fopen(scratch, "w");
    }}
      break;
    default:
      fprintf(stderr, "Unknown TUNE parameter setting format %s\n", cp);
    }}
}  

int mse(blk1, blk2)
Block blk1, blk2;
{
  register int index, error, tmp;
  int16_mpeg_t *bp1, *bp2;

  bp1 = (int16_mpeg_t *)blk1;
  bp2 = (int16_mpeg_t *)blk2;
  error = 0;
  for ( index = 0;  index < DCTSIZE_SQ;  index++ ) {
    tmp = *bp1++ - *bp2++;
    error += tmp*tmp;
  }
  return error;
}
