/*
 * opts.h - set optional parameters
 */

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

/*  
 *  $Header: /n/picasso/project/mpeg/mpeg_dist/mpeg_encode/headers/RCS/opts.h,v 1.3 1995/08/15 23:43:43 smoot Exp $
 *  $Log: opts.h,v $
 *  Revision 1.3  1995/08/15 23:43:43  smoot
 *  *** empty log message ***
 *
 * Revision 1.2  1995/05/02  22:00:51  smoot
 * added TUNEing stuff
 *
 * Revision 1.1  1995/04/14  23:12:53  smoot
 * Initial revision
 *
 */

#include "general.h"
#include "ansi.h"
#include "mtypes.h"

/*
 TUNE b [limit] lower limit on how different a block must be to be DCT coded
 TUNE c [file [color-diff]] Collect statistics on Quantization
 TUNE d [RateScale DistortionScale] Do a DCT in the P search, not just DIFF
 TUNE k [breakpt end [slope]] Squash small lum values
 TUNE l Figure out Laplacian distrib and use them to dequantize and do snr calc
 TUNE n Dont consider DC differenece in DCT searches
 TUNE q Do MSE for distortion measure, not MAD
 TUNE s [Max] | [LumMax ChromMax] Squash small differences in successive frames
 TUNE u disallow skip blocks in B frames
 TUNE w filename [c]  Write I block distortion numbers to file [with bit-rates]
 TUNE z Zaps Intra blocks in P/B frames.

 [ Note k and s make -snr numbers a lie, by playing with input ]
 [ Note d n and q are contradictory (can only use one)         ]
 [ Note c will not work on parallel encodings                  ]
*/

extern boolean tuneingOn;

/* Smash to no-change a motion block DCT with MAD less than: */
/* DETAL b value               */
extern int block_bound;

/* Collect info on quantization */
extern boolean collect_quant;
extern int collect_quant_detailed;
extern FILE   *collect_quant_fp;

/* Nuke dim areas */
extern int kill_dim, kill_dim_break, kill_dim_end;
extern float kill_dim_slope;


/* Stuff to control MV search comparisons */
#define DEFAULT_SEARCH 0
#define LOCAL_DCT  1 /* Do DCT in search (SLOW!!!!) */
#define NO_DC_SEARCH  2  /* Dont consider DC component in motion searches */
#define DO_Mean_Squared_Distortion  3 /* Do Squared distortion, not ABS */

/* Parameters for special searches */
/* LOCAL_DCT */
extern float LocalDCTRateScale, LocalDCTDistortScale;

/* Search Type Variable */
extern int SearchCompareMode;

/* squash small differences */
extern boolean squash_small_differences;
extern int SquashMaxLum, SquashMaxChr;

/* Disallows Intra blocks in P/B code */
extern boolean IntraPBAllowed;

/* Write out distortion numbers */
extern boolean WriteDistortionNumbers;
extern int collect_distortion_detailed;
extern FILE *distortion_fp;
extern FILE *fp_table_rate[31], *fp_table_dist[31];

/* Laplacian Distrib */
extern boolean DoLaplace;
extern double **L1, **L2, **Lambdas;
extern int LaplaceNum, LaplaceCnum;

/* Turn on/off skipping in B frames */
extern boolean BSkipBlocks;

/* Procedures Prototypes */
int    GetIQScale _ANSI_ARGS_((void));
int    GetPQScale _ANSI_ARGS_((void));
int    GetBQScale _ANSI_ARGS_((void));
void    Tune_Init _ANSI_ARGS_((void));
char    *SkipSpacesTabs _ANSI_ARGS_((char *start));
int     CalcRLEHuffLength _ANSI_ARGS_((FlatBlock in));
void    ParseTuneParam _ANSI_ARGS_((char *charPtr));
int     mse _ANSI_ARGS_((Block blk1, Block blk2));




