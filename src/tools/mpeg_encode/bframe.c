/*===========================================================================*
 * bframe.c                                     *
 *                                         *
 *    Procedures concerned with the B-frame encoding                 *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    GenBFrame                                 *
 *    ResetBFrameStats                             *
 *    ShowBFrameSummary                             *
 *    EstimateSecondsPerBFrame                         *
 *    ComputeBMotionLumBlock                             *
 *    SetBQScale                                 *
 *    GetBQScale                                 *
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

/*  
 *  $Header: /n/picasso/project/mpeg/mpeg_dist/mpeg_encode/RCS/bframe.c,v 1.20 1995/08/14 22:28:11 smoot Exp $
 *  $Log: bframe.c,v $
 *  Revision 1.20  1995/08/14 22:28:11  smoot
 *  renamed index to idx
 *  added option to not skip in B frames
 *
 *  Revision 1.19  1995/08/07 21:52:11  smoot
 *  added Color to skip routine
 *  fixed full/half bug in intial loop
 *  added comments
 *  removed buggy "extra skips" code
 *
 *  Revision 1.18  1995/06/21 22:22:24  smoot
 *  generalized time checking, fixed bug in specifics filesm
 *  and added TUNEing stuff
 *
 * Revision 1.17  1995/04/14  23:08:02  smoot
 * reorganized to ease rate control experimentation
 *
 * Revision 1.16  1995/02/24  23:49:10  smoot
 * added Spec version 2
 *
 * Revision 1.15  1995/01/30  19:45:45  smoot
 * Fixed a cr/cb screwup
 *
 * Revision 1.14  1995/01/23  02:46:43  darryl
 * initialized variable
 *
 * Revision 1.13  1995/01/19  23:07:12  eyhung
 * Changed copyrights
 *
 * Revision 1.12  1995/01/16  07:44:11  eyhung
 * Added realQuiet
 *
 * Revision 1.11  1994/12/07  00:40:36  smoot
 * Added seperate P and B search ranges
 *
 * Revision 1.10  1994/11/24  00:35:47  smoot
 * fixed bug (divide by 0) in B fram statsitics
 *
 * Revision 1.9  1994/11/14  22:26:48  smoot
 * Merged specifics and rate control.
 *
 * Revision 1.8  1994/11/01  05:01:16  darryl
 *  with rate control changes added
 *
 * Revision 2.0  1994/10/24  02:38:51  darryl
 * will be adding the experiment code
 *
 * Revision 1.1  1994/09/27  00:16:04  darryl
 * Initial revision
 *
 * Revision 1.7  1994/03/15  00:27:11  keving
 * nothing
 *
 * Revision 1.6  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.5  1993/07/30  19:24:04  keving
 * nothing
 *
 * Revision 1.4  1993/07/22  22:23:43  keving
 * nothing
 *
 * Revision 1.3  1993/06/30  20:06:09  keving
 * nothing
 *
 * Revision 1.2  1993/06/03  21:08:08  keving
 * nothing
 *
 * Revision 1.1  1993/02/19  19:14:28  keving
 * nothing
 *
 */


/*==============*
 * HEADER FILES *
 *==============*/

#include "all.h"
#include <sys/param.h>
#include <assert.h>
#include "mtypes.h"
#include "bitio.h"
#include "frames.h"
#include "prototypes.h"
#include "fsize.h"
#include "param.h"
#include "mheaders.h"
#include "postdct.h"
#include "rate.h"
#include "opts.h"

/*==================*
 * STATIC VARIABLES *
 *==================*/

static int numBIBlocks = 0;
static int numBBBlocks = 0;
static int numBSkipped = 0;
static int numBIBits = 0;
static int numBBBits = 0;
static int numFrames = 0;
static int numFrameBits = 0;
static int32_mpeg_t totalTime = 0;
static int qscaleB;
static float    totalSNR = 0.0;
static float    totalPSNR = 0.0;

static int numBFOBlocks = 0;    /* forward only */
static int numBBABlocks = 0;    /* backward only */
static int numBINBlocks = 0;    /* interpolate */
static int numBFOBits = 0;
static int numBBABits = 0;
static int numBINBits = 0;

/*====================*
 * EXTERNAL VARIABLES *
 *====================*/

extern Block **dct, **dctr, **dctb;
extern dct_data_type **dct_data;
#define NO_MOTION 0
#define MOTION 1
#define SKIP 2  /* used in useMotion in dct_data */

/*===============================*
 * INTERNAL PROCEDURE prototypes *
 *===============================*/

static boolean    MotionSufficient _ANSI_ARGS_((MpegFrame *curr, LumBlock currBlock, MpegFrame *prev, MpegFrame *next,
             int by, int bx, int mode, int fmy, int fmx,
             int bmy, int bmx));
static void    ComputeBMotionBlock _ANSI_ARGS_((MpegFrame *prev, MpegFrame *next,
                   int by, int bx, int mode, int fmy, int fmx,
                   int bmy, int bmx, Block motionBlock, int type));
static void    ComputeBDiffDCTs _ANSI_ARGS_((MpegFrame *current, MpegFrame *prev, MpegFrame *next,
             int by, int bx, int mode, int fmy, int fmx, 
             int bmy, int bmx, int *pattern));
static boolean    DoBIntraCode _ANSI_ARGS_((MpegFrame *current, MpegFrame *prev, MpegFrame *next,
             int by, int bx, int mode, int fmy, int fmx, int bmy,
             int bmx));

static int ComputeBlockColorDiff _ANSI_ARGS_((Block current, Block motionBlock));

/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/

/*===========================================================================*
 *
 * GenBFrame
 *
 *    generate a B-frame from previous and next frames, adding the result
 *    to the given bit bucket
 *
 * RETURNS:    frame appended to bb
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
GenBFrame(bb, curr, prev, next)
    BitBucket *bb;
    MpegFrame *curr;
    MpegFrame *prev;
    MpegFrame *next;
{
    extern int **bfmvHistogram;
    extern int **bbmvHistogram;
    FlatBlock fba[6], fb[6];
    Block     dec[6];
    int32_mpeg_t y_dc_pred, cr_dc_pred, cb_dc_pred;
    int x, y;
    int    fMotionX = 0, fMotionY = 0;
    int bMotionX = 0, bMotionY = 0;
    int    oldFMotionX = 0, oldFMotionY = 0;
    int oldBMotionX = 0, oldBMotionY = 0;
    int    oldMode = MOTION_FORWARD;
    int    mode = MOTION_FORWARD;
    int    offsetX, offsetY;
    int    tempX, tempY;
    int    fMotionXrem = 0, fMotionXquot = 0;
    int    fMotionYrem = 0, fMotionYquot = 0;
    int    bMotionXrem = 0, bMotionXquot = 0;
    int    bMotionYrem = 0, bMotionYquot = 0;
    int    pattern;
    int    numIBlocks = 0, numBBlocks = 0;
    int numSkipped = 0, totalBits;
    int    numIBits = 0,   numBBits = 0;
    boolean    lastIntra = TRUE;
    boolean    motionForward, motionBackward;
    int        totalFrameBits;
    int32_mpeg_t    startTime, endTime;
    int lastX, lastY;
    int lastBlockX, lastBlockY;
    register int ix, iy;
    LumBlock currentBlock;
    int         fy, fx;
    boolean    make_skip_block;
    int    mbAddrInc = 1;
    int    mbAddress;
    int        slicePos;
    float   snr[3], psnr[3];
    int        idx;
    int     QScale;
    BlockMV *info;
    int     bitstreamMode, newQScale;
    int     rc_blockStart=0;
    boolean overflowChange=FALSE;
    int overflowValue = 0;

    if (collect_quant) {fprintf(collect_quant_fp, "# B\n");}
    if (dct == NULL) AllocDctBlocks();
    numFrames++;
    totalFrameBits = bb->cumulativeBits;
    startTime = time_elapsed();

    /*   Rate Control */
    bitstreamMode = getRateMode();
    if (bitstreamMode == FIXED_RATE) {
      targetRateControl(curr);
    }
 
    QScale = GetBQScale();
    Mhead_GenPictureHeader(bb, B_FRAME, curr->id, fCodeB);
    /* Check for Qscale change */
    if (specificsOn) {
      newQScale = SpecLookup(curr->id, 0, 0 /* junk */, &info, QScale);
      if (newQScale != -1) {
    QScale = newQScale;
      }
      /* check for slice */
      newQScale = SpecLookup(curr->id, 1, 1, &info, QScale);
      if (newQScale != -1) {
    QScale = newQScale;
      }
    }

    Mhead_GenSliceHeader(bb, 1, QScale, NULL, 0);

    Frame_AllocBlocks(curr);
    BlockifyFrame(curr);

    if ( printSNR ) {
    Frame_AllocDecoded(curr, FALSE);
    }

    /* for I-blocks */
    y_dc_pred = cr_dc_pred = cb_dc_pred = 128;

    totalBits = bb->cumulativeBits;

    if ( ! pixelFullSearch ) {
    if ( ! prev->halfComputed && (prev != NULL)) {
        ComputeHalfPixelData(prev);
    }

    if ( ! next->halfComputed ) {
        ComputeHalfPixelData(next);
    }
    }

    lastBlockX = Fsize_x>>3;
    lastBlockY = Fsize_y>>3;
    lastX = lastBlockX-2;
    lastY = lastBlockY-2;
    mbAddress = 0;

    /* find motion vectors and do dcts */
    /* In this first loop, all MVs are in half-pixel scope, (if FULL is set
       then they will be multiples of 2).  This is not true in the second loop. */
    for (y = 0;  y < lastBlockY;  y += 2) {
      for (x = 0;  x < lastBlockX;  x += 2) {
    slicePos = (mbAddress % blocksPerSlice);

    /* compute currentBlock */
    BLOCK_TO_FRAME_COORD(y, x, fy, fx);
    for ( iy = 0; iy < 16; iy++ ) {
      for ( ix = 0; ix < 16; ix++ ) {
        currentBlock[iy][ix] = (int16_mpeg_t)curr->orig_y[fy+iy][fx+ix];
      }
    }
        
    if (slicePos == 0) {
      oldFMotionX = 0;    oldFMotionY = 0;
      oldBMotionX = 0;    oldBMotionY = 0;
      oldMode = MOTION_FORWARD;
      lastIntra = TRUE;
    }

    /* STEP 1:  Select Forward, Backward, or Interpolated motion vectors */
    /* see if old motion is good enough */
    /* but force last block to be non-skipped */
    /* can only skip if:
     *     1)  not the last block in frame
     *     2)  not the last block in slice
     *     3)  not the first block in slice
     *     4)  previous block was not intra-coded
     */
    if ( ((y < lastY) || (x < lastX)) &&
        (slicePos+1 != blocksPerSlice) &&
        (slicePos != 0) &&
        (! lastIntra) &&
        (BSkipBlocks) ) {
      make_skip_block = MotionSufficient(curr, currentBlock, prev, next, y, x, oldMode,
                         oldFMotionY, oldFMotionX,
                         oldBMotionY, oldBMotionX);
    } else {
      make_skip_block = FALSE;
    }

    if ( make_skip_block ) {
    skip_it:
      /* skipped macro block */
      dct_data[y][x].useMotion = SKIP;
    } else {
      if (specificsOn) {
        (void) SpecLookup(curr->id, 2, mbAddress, &info, QScale);
        if (info == (BlockMV*)NULL) goto gosearch;
        else {
          switch (info->typ) {
          case TYP_SKIP:
        goto skip_it;
          case TYP_FORW:
        fMotionX = info->fx;
        fMotionY = info->fy;
        mode = MOTION_FORWARD;
        break;
          case TYP_BACK:
        bMotionX = info->bx;
        bMotionY = info->by;
        mode = MOTION_BACKWARD;
        break;
          case TYP_BOTH:
        fMotionX = info->fx;
        fMotionY = info->fy;
        bMotionX = info->bx;
        bMotionY = info->by;
        mode = MOTION_INTERPOLATE;
        break;
          default:
        fprintf(stderr,"Unreachable code in GenBFrame!\n");
        goto gosearch;
          }
          goto skipsearch;
        }}
    gosearch:        /* do bsearch */
      mode = BMotionSearch(currentBlock, prev, next, y, x, &fMotionY,
                   &fMotionX, &bMotionY, &bMotionX, mode);
    skipsearch:          
          
      /* STEP 2:  INTRA OR NON-INTRA CODING */
      if ( IntraPBAllowed && DoBIntraCode(curr, prev, next, y, x, mode, fMotionY,
                fMotionX, bMotionY, bMotionX) ) {
        /* output I-block inside a B-frame */
        numIBlocks++;
        oldFMotionX = 0;    oldFMotionY = 0;
        oldBMotionX = 0;    oldBMotionY = 0;
        lastIntra = TRUE;
        dct_data[y][x].useMotion = NO_MOTION;
        oldMode = MOTION_FORWARD;
        /* calculate forward dct's */
        if (collect_quant && (collect_quant_detailed & 1)) fprintf(collect_quant_fp, "l\n");
        mp_fwd_dct_block2(curr->y_blocks[y][x], dct[y][x]);
        mp_fwd_dct_block2(curr->y_blocks[y][x+1], dct[y][x+1]);
        mp_fwd_dct_block2(curr->y_blocks[y+1][x], dct[y+1][x]);
        mp_fwd_dct_block2(curr->y_blocks[y+1][x+1], dct[y+1][x+1]);
        if (collect_quant && (collect_quant_detailed & 1)) {fprintf(collect_quant_fp, "c\n");}
        mp_fwd_dct_block2(curr->cb_blocks[y>>1][x>>1], dctb[y>>1][x>>1]);
        mp_fwd_dct_block2(curr->cr_blocks[y>>1][x>>1], dctr[y>>1][x>>1]);

      } else { /* dct P/Bi/B block */

        pattern = 63;
        lastIntra = FALSE;
        numBBlocks++;
        dct_data[y][x].mode = mode;
        oldMode = mode;
        dct_data[y][x].fmotionX = fMotionX;
        dct_data[y][x].fmotionY = fMotionY;
        dct_data[y][x].bmotionX = bMotionX;
        dct_data[y][x].bmotionY = bMotionY;
        switch (mode) {
        case MOTION_FORWARD:
          numBFOBlocks++;
          oldFMotionX = fMotionX;        oldFMotionY = fMotionY;
          break;
        case MOTION_BACKWARD:
          numBBABlocks++;
          oldBMotionX = bMotionX;        oldBMotionY = bMotionY;
          break;
        case MOTION_INTERPOLATE:
          numBINBlocks++;
          oldFMotionX = fMotionX;        oldFMotionY = fMotionY;
          oldBMotionX = bMotionX;        oldBMotionY = bMotionY;
          break;
        default:
          fprintf(stderr, "PROGRAMMER ERROR:  Illegal mode: %d\n", mode);
          exit(1);
        }
        
        ComputeBDiffDCTs(curr, prev, next, y, x, mode, fMotionY,
                 fMotionX, bMotionY, bMotionX, &pattern);
        
        dct_data[y][x].pattern = pattern;
        dct_data[y][x].useMotion = MOTION;
        if ( computeMVHist ) {
          assert(fMotionX+searchRangeB+1 >= 0);
          assert(fMotionY+searchRangeB+1 >= 0);
          assert(fMotionX+searchRangeB+1 <= 2*searchRangeB+2);
          assert(fMotionY+searchRangeB+1 <= 2*searchRangeB+2);
          assert(bMotionX+searchRangeB+1 >= 0);
          assert(bMotionY+searchRangeB+1 >= 0);
          assert(bMotionX+searchRangeB+1 <= 2*searchRangeB+2);
          assert(bMotionY+searchRangeB+1 <= 2*searchRangeB+2);

          bfmvHistogram[fMotionX+searchRangeB+1][fMotionY+searchRangeB+1]++;
          bbmvHistogram[bMotionX+searchRangeB+1][bMotionY+searchRangeB+1]++;
        }
      } /* motion-block */
    } /* not skipped */
    mbAddress++;
      }}

    /* reset everything */
    oldFMotionX = 0;    oldFMotionY = 0;
    oldBMotionX = 0;    oldBMotionY = 0;
    oldMode = MOTION_FORWARD;
    lastIntra = TRUE;
    y_dc_pred = cr_dc_pred = cb_dc_pred = 128;
    mbAddress = 0;

    /* Now generate the frame */
    for (y = 0; y < lastBlockY; y += 2) {
      for (x = 0; x < lastBlockX; x += 2) {
    slicePos = (mbAddress % blocksPerSlice);

    if ( (slicePos == 0) && (mbAddress != 0) ) {
      if (specificsOn) {
        /* Make sure no slice Qscale change */
        newQScale = SpecLookup(curr->id,1,mbAddress/blocksPerSlice, &info, QScale);
        if (newQScale != -1) QScale = newQScale;
      }
      Mhead_GenSliceEnder(bb);
      Mhead_GenSliceHeader(bb, 1+(y>>1), QScale, NULL, 0);

      /* reset everything */
      oldFMotionX = 0;    oldFMotionY = 0;
      oldBMotionX = 0;    oldBMotionY = 0;
      oldMode = MOTION_FORWARD;
      lastIntra = TRUE;
      y_dc_pred = cr_dc_pred = cb_dc_pred = 128;

      mbAddrInc = 1+(x>>1);
    }

    /*  Determine if new Qscale needed for Rate Control purposes */
    if (bitstreamMode == FIXED_RATE) {
      rc_blockStart =  bb->cumulativeBits;
      newQScale = needQScaleChange(QScale,
                       curr->y_blocks[y][x],
                       curr->y_blocks[y][x+1],
                       curr->y_blocks[y+1][x],
                       curr->y_blocks[y+1][x+1]);
      if (newQScale > 0) {
        QScale = newQScale;
      }
    }
 
    if (specificsOn) {
      newQScale = SpecLookup(curr->id, 2, mbAddress, &info, QScale);
      if (newQScale != -1) {
        QScale = newQScale;
      }}

    if (dct_data[y][x].useMotion == NO_MOTION) {

      GEN_I_BLOCK(B_FRAME, curr, bb, mbAddrInc, QScale);
      mbAddrInc = 1;
      numIBits += (bb->cumulativeBits-totalBits);
      totalBits = bb->cumulativeBits;
          
      /* reset because intra-coded */
      oldFMotionX = 0;        oldFMotionY = 0;
      oldBMotionX = 0;        oldBMotionY = 0;
      oldMode = MOTION_FORWARD;
      lastIntra = TRUE;
          
      if ( printSNR ) {
        /* need to decode block we just encoded */
        /* and reverse the DCT transform */
        for ( idx = 0; idx < 6; idx++ ) {
          Mpost_UnQuantZigBlock(fb[idx], dec[idx], QScale, TRUE);
          mpeg_jrevdct((int16_mpeg_t *)dec[idx]);
        }

        /* now, unblockify */
        BlockToData(curr->decoded_y, dec[0], y, x);
        BlockToData(curr->decoded_y, dec[1], y, x+1);
        BlockToData(curr->decoded_y, dec[2], y+1, x);
        BlockToData(curr->decoded_y, dec[3], y+1, x+1);
        BlockToData(curr->decoded_cb, dec[4], y>>1, x>>1);
        BlockToData(curr->decoded_cr, dec[5], y>>1, x>>1);
      }
    } else if (dct_data[y][x].useMotion == SKIP) {
    skip_block:
      numSkipped++;
      mbAddrInc++;
          
      /* decode skipped block */
      if ( printSNR ) {
        int    fmy, fmx, bmy, bmx;
        
        for ( idx = 0; idx < 6; idx++ ) {
          memset((char *)dec[idx], 0, sizeof(Block)); 
        }
        
        if ( pixelFullSearch ) {
          fmy = 2*oldFMotionY;
          fmx = 2*oldFMotionX;
          bmy = 2*oldBMotionY;
          bmx = 2*oldBMotionX;
        } else {
          fmy = oldFMotionY;
          fmx = oldFMotionX;
          bmy = oldBMotionY;
          bmx = oldBMotionX;
        }
        
        /* now add the motion block */
        AddBMotionBlock(dec[0], prev->decoded_y,
                next->decoded_y, y, x, mode,
                fmy, fmx, bmy, bmx);
        AddBMotionBlock(dec[1], prev->decoded_y,
                next->decoded_y, y, x+1, mode,
                fmy, fmx, bmy, bmx);
        AddBMotionBlock(dec[2], prev->decoded_y,
                next->decoded_y, y+1, x, mode,
                fmy, fmx, bmy, bmx);
        AddBMotionBlock(dec[3], prev->decoded_y,
                next->decoded_y, y+1, x+1, mode,
                fmy, fmx, bmy, bmx);
        AddBMotionBlock(dec[4], prev->decoded_cb,
                next->decoded_cb, y>>1, x>>1, mode,
                fmy/2, fmx/2,
                bmy/2, bmx/2);
        AddBMotionBlock(dec[5], prev->decoded_cr,
                next->decoded_cr, y>>1, x>>1, mode,
                fmy/2, fmx/2,
                bmy/2, bmx/2);
        
        /* now, unblockify */
        BlockToData(curr->decoded_y, dec[0], y, x);
        BlockToData(curr->decoded_y, dec[1], y, x+1);
        BlockToData(curr->decoded_y, dec[2], y+1, x);
        BlockToData(curr->decoded_y, dec[3], y+1, x+1);
        BlockToData(curr->decoded_cb, dec[4], y>>1, x>>1);
        BlockToData(curr->decoded_cr, dec[5], y>>1, x>>1);
      }
    } else   /* B block */ {
      int fCode = fCodeB;    

      pattern = dct_data[y][x].pattern;
      fMotionX = dct_data[y][x].fmotionX;
      fMotionY = dct_data[y][x].fmotionY;
      bMotionX = dct_data[y][x].bmotionX;
      bMotionY = dct_data[y][x].bmotionY;

      if ( pixelFullSearch ) {
        fMotionX /= 2;        fMotionY /= 2;
        bMotionX /= 2;        bMotionY /= 2;
      }
          
      /* create flat blocks and update pattern if necessary */
    calc_blocks:
    /* Note DoQuant references QScale, overflowChange, overflowValue,
           pattern, and the calc_blocks label                 */
      DoQuant(0x20, dct[y][x], fba[0]);
      DoQuant(0x10, dct[y][x+1], fba[1]);
      DoQuant(0x08, dct[y+1][x], fba[2]);
      DoQuant(0x04, dct[y+1][x+1], fba[3]);
      DoQuant(0x02, dctb[y>>1][x>>1], fba[4]);
      DoQuant(0x01, dctr[y>>1][x>>1], fba[5]);

      motionForward  = (dct_data[y][x].mode != MOTION_BACKWARD);
      motionBackward = (dct_data[y][x].mode != MOTION_FORWARD);

#ifdef BUGGY_CODE
      /*
      send us mail if you can tell me why this code
          doesnt work.  Generates some bad vectors.
          I suspect 'cuz oldMode/motions aren't being set right,
          but am unsure.
      */
      /* Check to see if we should have skipped */
      if ((pattern == 0) &&
          ((y < lastY) || (x < lastX)) &&
          (slicePos+1 != blocksPerSlice) &&
          (slicePos != 0) &&
          (!lastIntra) &&
          ( (!motionForward) || 
           (motionForward && 
        fMotionX == oldFMotionX && fMotionY == oldFMotionY)) &&
          ( (!motionBackward) || 
           (motionBackward && 
        bMotionX == oldBMotionX && bMotionY == oldBMotionY))
          ) {
        /* Now *thats* an if statement! */
        goto skip_block; 
      }
#endif
      /* Encode Vectors */
      if ( motionForward ) {
        /* transform the fMotion vector into the appropriate values */
        offsetX = fMotionX - oldFMotionX;
        offsetY = fMotionY - oldFMotionY;

        ENCODE_MOTION_VECTOR(offsetX, offsetY, fMotionXquot,
                 fMotionYquot, fMotionXrem, fMotionYrem,
                 FORW_F);
        oldFMotionX = fMotionX;        oldFMotionY = fMotionY;
      }
          
      if ( motionBackward ) {
        /* transform the bMotion vector into the appropriate values */
        offsetX = bMotionX - oldBMotionX;
        offsetY = bMotionY - oldBMotionY;
        ENCODE_MOTION_VECTOR(offsetX, offsetY, bMotionXquot,
                 bMotionYquot, bMotionXrem, bMotionYrem,
                 BACK_F);
        oldBMotionX = bMotionX;        oldBMotionY = bMotionY;
      }
          
      oldMode = dct_data[y][x].mode;
          
      if ( printSNR ) { /* Need to decode */
        if ( pixelFullSearch ) {
          fMotionX *= 2;    fMotionY *= 2;
          bMotionX *= 2;    bMotionY *= 2;
        }
        for ( idx = 0; idx < 6; idx++ ) {
          if ( pattern & (1 << (5-idx)) ) {
        Mpost_UnQuantZigBlock(fba[idx], dec[idx], QScale, FALSE);
        mpeg_jrevdct((int16_mpeg_t *)dec[idx]);
          } else {
        memset((char *)dec[idx], 0, sizeof(Block));
          }
        }

        /* now add the motion block */
        AddBMotionBlock(dec[0], prev->decoded_y,
                next->decoded_y, y, x, mode,
                fMotionY, fMotionX, bMotionY, bMotionX);
        AddBMotionBlock(dec[1], prev->decoded_y,
                next->decoded_y, y, x+1, mode,
                fMotionY, fMotionX, bMotionY, bMotionX);
        AddBMotionBlock(dec[2], prev->decoded_y,
                next->decoded_y, y+1, x, mode,
                fMotionY, fMotionX, bMotionY, bMotionX);
        AddBMotionBlock(dec[3], prev->decoded_y,
                next->decoded_y, y+1, x+1, mode,
                fMotionY, fMotionX, bMotionY, bMotionX);
        AddBMotionBlock(dec[4], prev->decoded_cb,
                next->decoded_cb, y>>1, x>>1, mode,
                fMotionY/2, fMotionX/2,
                bMotionY/2, bMotionX/2);
        AddBMotionBlock(dec[5], prev->decoded_cr,
                next->decoded_cr, y>>1, x>>1, mode,
                fMotionY/2, fMotionX/2,
                bMotionY/2, bMotionX/2);

        /* now, unblockify */
        BlockToData(curr->decoded_y,  dec[0], y,    x);
        BlockToData(curr->decoded_y,  dec[1], y,    x+1);
        BlockToData(curr->decoded_y,  dec[2], y+1,  x);
        BlockToData(curr->decoded_y,  dec[3], y+1,  x+1);
        BlockToData(curr->decoded_cb, dec[4], y>>1, x>>1);
        BlockToData(curr->decoded_cr, dec[5], y>>1, x>>1);
      }

      /* reset because non-intra-coded */
      y_dc_pred = cr_dc_pred = cb_dc_pred = 128;
      lastIntra = FALSE;
      mode = dct_data[y][x].mode;

      /*      DBG_PRINT(("MB Header(%d,%d)\n", x, y));  */
      Mhead_GenMBHeader(bb, 3 /* pict_code_type */, mbAddrInc /* addr_incr */,
        QScale /* q_scale */,
        fCodeB /* forw_f_code */, fCodeB /* back_f_code */,
        fMotionXrem /* horiz_forw_r */, fMotionYrem /* vert_forw_r */,
        bMotionXrem /* horiz_back_r */, bMotionYrem /* vert_back_r */,
        motionForward /* motion_forw */, fMotionXquot /* m_horiz_forw */,
        fMotionYquot /* m_vert_forw */, motionBackward /* motion_back */,
        bMotionXquot /* m_horiz_back */, bMotionYquot /* m_vert_back */,
        pattern /* mb_pattern */, FALSE /* mb_intra */);
      mbAddrInc = 1;
          
      /* now output the difference */
      for ( tempX = 0; tempX < 6; tempX++ ) {
        if ( GET_ITH_BIT(pattern, 5-tempX) ) {
          Mpost_RLEHuffPBlock(fba[tempX], bb);
        }
      }
          
      
      switch (mode) {
      case MOTION_FORWARD:
        numBFOBits += (bb->cumulativeBits-totalBits);
        break;
      case MOTION_BACKWARD:
        numBBABits += (bb->cumulativeBits-totalBits);
        break;
      case MOTION_INTERPOLATE:
        numBINBits += (bb->cumulativeBits-totalBits);
        break;
      default:
        fprintf(stderr, "PROGRAMMER ERROR:  Illegal mode: %d\n",
            mode);
        exit(1);
      }
      
      numBBits += (bb->cumulativeBits-totalBits);
      totalBits = bb->cumulativeBits;
    
      if (overflowChange) {
        /* undo an overflow-caused Qscale change */
        overflowChange = FALSE;
        QScale -= overflowValue;
        overflowValue = 0;
      }
    } /* if I-block, skip, or B */

    mbAddress++;
    /*   Rate Control  */
    if (bitstreamMode == FIXED_RATE) {
      incMacroBlockBits( bb->cumulativeBits - rc_blockStart);
      rc_blockStart = bb->cumulativeBits;
      MB_RateOut(TYPE_BFRAME);
    }
    
      }
    }

    if ( printSNR ) {
      BlockComputeSNR(curr,snr,psnr);
      totalSNR += snr[0];
      totalPSNR += psnr[0];
    }
    
    Mhead_GenSliceEnder(bb);
    /*   Rate Control  */
    if (bitstreamMode == FIXED_RATE) {
      updateRateControl(TYPE_BFRAME);
    }
    
    endTime = time_elapsed();
    totalTime += (endTime-startTime);
    
    if ( ( ! childProcess) && showBitRatePerFrame ) {
      /* ASSUMES 30 FRAMES PER SECOND */
      fprintf(bitRateFile, "%5d\t%8d\n", curr->id,
          30*(bb->cumulativeBits-totalFrameBits));
    }
    
    if ( (! childProcess) && frameSummary && !realQuiet) {
      fprintf(stdout, "FRAME %d (B):  I BLOCKS:  %d;  B BLOCKS:  %d   SKIPPED:  %d (%ld seconds)\n",
          curr->id, numIBlocks, numBBlocks, numSkipped, (long)((endTime-startTime)/TIME_RATE));
      if ( printSNR )
    fprintf(stdout, "FRAME %d:  SNR:  %.1f\t%.1f\t%.1f\tPSNR:  %.1f\t%.1f\t%.1f\n",
        curr->id, snr[0], snr[1], snr[2],
        psnr[0], psnr[1], psnr[2]);
    }
    
    numFrameBits += (bb->cumulativeBits-totalFrameBits);
    numBIBlocks += numIBlocks;
    numBBBlocks += numBBlocks;
    numBSkipped += numSkipped;
    numBIBits += numIBits;
    numBBBits += numBBits;
  }


/*===========================================================================*
 *
 * SetBQScale
 *
 *    set the B-frame Q-scale
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    qscaleB
 *
 *===========================================================================*/
void
SetBQScale(qB)
    int qB;
{
    qscaleB = qB;
}


/*===========================================================================*
 *
 * GetBQScale
 *
 *    get the B-frame Q-scale
 *
 * RETURNS:    the Q-scale
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
int
GetBQScale()
{
    return qscaleB;
}


/*===========================================================================*
 *
 * ResetBFrameStats
 *
 *    reset the B-frame stats
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
ResetBFrameStats()
{
    numBIBlocks = 0;
    numBBBlocks = 0;
    numBSkipped = 0;
    numBIBits = 0;
    numBBBits = 0;
    numFrames = 0;
    numFrameBits = 0;
    totalTime = 0;
}


/*===========================================================================*
 *
 * ShowBFrameSummary
 *
 *    print out statistics on all B-frames
 *
 * RETURNS:    time taken for B-frames (in seconds)
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
float
ShowBFrameSummary(inputFrameBits, totalBits, fpointer)
    int inputFrameBits;
    int32_mpeg_t totalBits;
    FILE *fpointer;
{
    if ( numFrames == 0 ) {
    return 0.0;
    }

    fprintf(fpointer, "-------------------------\n");
    fprintf(fpointer, "*****B FRAME SUMMARY*****\n");
    fprintf(fpointer, "-------------------------\n");

    if ( numBIBlocks != 0 ) {
    fprintf(fpointer, "  I Blocks:  %5d     (%6d bits)     (%5d bpb)\n",
        numBIBlocks, numBIBits, numBIBits/numBIBlocks);
    } else {
    fprintf(fpointer, "  I Blocks:  %5d\n", 0);
    }

    if ( numBBBlocks != 0 ) {
    fprintf(fpointer, "  B Blocks:  %5d     (%6d bits)     (%5d bpb)\n",
        numBBBlocks, numBBBits, numBBBits/numBBBlocks);
    fprintf(fpointer, "  B types:   %5d     (%4d bpb) forw  %5d (%4d bpb) back   %5d (%4d bpb) bi\n",
        numBFOBlocks, (numBFOBlocks==0)?0:numBFOBits/numBFOBlocks,
        numBBABlocks, (numBBABlocks==0)?0:numBBABits/numBBABlocks,
        numBINBlocks, (numBINBlocks==0)?0:numBINBits/numBINBlocks);
    } else {
    fprintf(fpointer, "  B Blocks:  %5d\n", 0);
    }

    fprintf(fpointer, "  Skipped:   %5d\n", numBSkipped);

    fprintf(fpointer, "  Frames:    %5d     (%6d bits)     (%5d bpf)     (%2.1f%% of total)\n",
        numFrames, numFrameBits, numFrameBits/numFrames,
        100.0*(float)numFrameBits/(float)totalBits);        
    fprintf(fpointer, "  Compression:  %3d:1     (%9.4f bpp)\n",
        numFrames*inputFrameBits/numFrameBits,
        24.0*(float)numFrameBits/(float)(numFrames*inputFrameBits));
    if ( printSNR )
    fprintf(fpointer, "  Avg Y SNR/PSNR:  %.1f     %.1f\n",
        totalSNR/(float)numFrames, totalPSNR/(float)numFrames);
    if ( totalTime == 0 ) {
    fprintf(fpointer, "  Seconds:  NONE\n");
    } else {
    fprintf(fpointer, "  Seconds:  %9ld     (%9.4f fps)  (%9ld pps)  (%9ld mps)\n",
        (long)(totalTime/TIME_RATE),
        (float)((float)(TIME_RATE*numFrames)/(float)totalTime),
        (long)((float)TIME_RATE*(float)numFrames*(float)inputFrameBits/(24.0*(float)totalTime)),
        (long)((float)TIME_RATE*(float)numFrames*(float)inputFrameBits/(256.0*24.0*(float)totalTime)));
    }

    return (float)totalTime/TIME_RATE;
}


/*===========================================================================*
 *
 * ComputeBMotionLumBlock
 *
 *    compute the luminance block resulting from motion compensation
 *
 * RETURNS:    motionBlock modified
 *
 * SIDE EFFECTS:    none
 *
 * PRECONDITION:    the motion vectors must be valid!
 *
 *===========================================================================*/
void
ComputeBMotionLumBlock(prev, next, by, bx, mode, fmy, fmx, bmy, bmx, motionBlock)
    MpegFrame *prev;
    MpegFrame *next;
    int by;
    int bx;
    int mode;
    int fmy;
    int fmx;
    int bmy;
    int bmx;
    LumBlock motionBlock;
{
    LumBlock    prevBlock, nextBlock;
    register int    y, x;

    switch(mode) {
    case MOTION_FORWARD:
      ComputeMotionLumBlock(prev, by, bx, fmy, fmx, motionBlock);
      break;
    case MOTION_BACKWARD:
      ComputeMotionLumBlock(next, by, bx, bmy, bmx, motionBlock);
      break;
    case MOTION_INTERPOLATE:
      ComputeMotionLumBlock(prev, by, bx, fmy, fmx, prevBlock);
      ComputeMotionLumBlock(next, by, bx, bmy, bmx, nextBlock);
      
      for ( y = 0; y < 16; y++ ) {
    for ( x = 0; x < 16; x++ ) {
      motionBlock[y][x] = (prevBlock[y][x]+nextBlock[y][x]+1)/2;
    }
      }
      break;
    default:
      fprintf(stderr, "Bad mode!\nProgrammer error!\n");
      break;
      
    }
}


/*===========================================================================*
 *
 * EstimateSecondsPerBFrame
 *
 *    estimate the seconds to compute a B-frame
 *
 * RETURNS:    the time, in seconds
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
float
EstimateSecondsPerBFrame()
{
    if ( numFrames == 0 ) {
    return 20.0;
    } else {
    return (float)totalTime/((float)TIME_RATE*(float)numFrames);
    }
}


/*=====================*
 * INTERNAL PROCEDURES *
 *=====================*/

/*===========================================================================*
 *
 * ComputeBMotionBlock
 *
 *    compute the block resulting from motion compensation
 *
 * RETURNS:    motionBlock is modified
 *
 * SIDE EFFECTS:    none
 *
 * PRECONDITION:    the motion vectors must be valid!
 *
 *===========================================================================*/
static void
ComputeBMotionBlock(prev, next, by, bx, mode, fmy, fmx, bmy, bmx, motionBlock, type)
    MpegFrame *prev;
    MpegFrame *next;
    int by;
    int bx;
    int mode;
    int fmy;
    int fmx;
    int bmy;
    int bmx;
    Block motionBlock;
    int type;
{
    Block    prevBlock, nextBlock;
    register int    y, x;

    switch(mode) {
    case MOTION_FORWARD:
        if ( type == LUM_BLOCK ) {
        ComputeMotionBlock(prev->ref_y, by, bx, fmy, fmx, motionBlock);
        } else if ( type == CB_BLOCK ) {
        ComputeMotionBlock(prev->ref_cb, by, bx, fmy, fmx, motionBlock);
        } else if ( type == CR_BLOCK ) {
        ComputeMotionBlock(prev->ref_cr, by, bx, fmy, fmx, motionBlock);
        }
        break;
    case MOTION_BACKWARD:
        if ( type == LUM_BLOCK ) {
        ComputeMotionBlock(next->ref_y, by, bx, bmy, bmx, motionBlock);
        } else if ( type == CB_BLOCK ) {
        ComputeMotionBlock(next->ref_cb, by, bx, bmy, bmx, motionBlock);
        } else if ( type == CR_BLOCK ) {
        ComputeMotionBlock(next->ref_cr, by, bx, bmy, bmx, motionBlock);
        }
        break;
    case MOTION_INTERPOLATE:
        if ( type == LUM_BLOCK ) {
        ComputeMotionBlock(prev->ref_y, by, bx, fmy, fmx, prevBlock);
        ComputeMotionBlock(next->ref_y, by, bx, bmy, bmx, nextBlock);
        } else if ( type == CB_BLOCK ) {
        ComputeMotionBlock(prev->ref_cb, by, bx, fmy, fmx, prevBlock);
        ComputeMotionBlock(next->ref_cb, by, bx, bmy, bmx, nextBlock);
        } else if ( type == CR_BLOCK ) {
        ComputeMotionBlock(prev->ref_cr, by, bx, fmy, fmx, prevBlock);
        ComputeMotionBlock(next->ref_cr, by, bx, bmy, bmx, nextBlock);
        }

        for ( y = 0; y < 8; y++ ) {
        for ( x = 0; x < 8; x++ ) {
            motionBlock[y][x] = (prevBlock[y][x]+nextBlock[y][x]+1)/2;
        }
        }
        break;
    }
}


/*===========================================================================*
 *
 * ComputeBDiffDCTs
 *
 *    compute the DCT of the error term
 *
 * RETURNS:    appropriate blocks of current will contain the DCTs
 *
 * SIDE EFFECTS:    none
 *
 * PRECONDITION:    the motion vectors must be valid!
 *
 *===========================================================================*/
static void
ComputeBDiffDCTs(current, prev, next, by, bx, mode, fmy, fmx, bmy, bmx, pattern)
    MpegFrame *current;
    MpegFrame *prev;
    MpegFrame *next;
    int by;
    int bx;
    int mode;
    int fmy;
    int fmx;
    int bmy;
    int bmx;
    int *pattern;
{
    Block   motionBlock;

    if ( *pattern & 0x20 ) {
    ComputeBMotionBlock(prev, next, by, bx, mode, fmy, fmx,
                bmy, bmx, motionBlock, LUM_BLOCK);
    if (! ComputeDiffDCTBlock(current->y_blocks[by][bx], dct[by][bx], motionBlock)) {
      *pattern ^=  0x20;
    }
    }

    if ( *pattern & 0x10 ) {
    ComputeBMotionBlock(prev, next, by, bx+1, mode, fmy, fmx,
                bmy, bmx, motionBlock, LUM_BLOCK);
    if (! ComputeDiffDCTBlock(current->y_blocks[by][bx+1], dct[by][bx+1], motionBlock)) {
      *pattern ^=  0x10;
    }
    }

    if ( *pattern & 0x8 ) {
    ComputeBMotionBlock(prev, next, by+1, bx, mode, fmy, fmx,
                bmy, bmx, motionBlock, LUM_BLOCK);
    if (! ComputeDiffDCTBlock(current->y_blocks[by+1][bx], dct[by+1][bx], motionBlock)) {
      *pattern ^= 0x8;
    }
    }

    if ( *pattern & 0x4 ) {
    ComputeBMotionBlock(prev, next, by+1, bx+1, mode, fmy, fmx,
                bmy, bmx, motionBlock, LUM_BLOCK);
    if (! ComputeDiffDCTBlock(current->y_blocks[by+1][bx+1], dct[by+1][bx+1], motionBlock)) {
      *pattern ^= 0x4;
    }
    }

    if ( *pattern & 0x2 ) {
    ComputeBMotionBlock(prev, next, by>>1, bx>>1, mode, fmy/2, fmx/2,
                bmy/2, bmx/2, motionBlock, CB_BLOCK);
    if (! ComputeDiffDCTBlock(current->cb_blocks[by >> 1][bx >> 1], dctb[by >> 1][bx >> 1], motionBlock)) {
      *pattern ^= 0x2;
    }
    }

    if ( *pattern & 0x1 ) {
    ComputeBMotionBlock(prev, next, by>>1, bx>>1, mode, fmy/2, fmx/2,
                bmy/2, bmx/2, motionBlock, CR_BLOCK);
    if (! ComputeDiffDCTBlock(current->cr_blocks[by >> 1][bx >> 1], dctr[by >> 1][bx >> 1], motionBlock)) {
      *pattern ^= 0x1;
    }
    }
}


/*===========================================================================*
 *
 *                USER-MODIFIABLE
 *
 * DoBIntraCode
 *
 *    decides if this block should be coded as intra-block
 *
 * RETURNS:    TRUE if intra-coding should be used; FALSE otherwise
 *
 * SIDE EFFECTS:    none
 *
 * PRECONDITION:    the motion vectors must be valid!
 *
 *===========================================================================*/
static boolean
DoBIntraCode(current, prev, next, by, bx, mode, fmy, fmx, bmy, bmx)
    MpegFrame *current;
    MpegFrame *prev;
    MpegFrame *next;
    int by;
    int bx;
    int mode;
    int fmy;
    int fmx;
    int bmy;
    int bmx;
{
    int        x, y;
    int32_mpeg_t sum = 0, vard = 0, varc = 0, dif;
    int32_mpeg_t currPixel, prevPixel;
    LumBlock    motionBlock;
    int        fy, fx;

    ComputeBMotionLumBlock(prev, next, by, bx, mode, fmy, fmx,
               bmy, bmx, motionBlock);

    MOTION_TO_FRAME_COORD(by, bx, 0, 0, fy, fx);

    for ( y = 0; y < 16; y++ ) {
    for ( x = 0; x < 16; x++ ) {
        currPixel = current->orig_y[fy+y][fx+x];
        prevPixel = motionBlock[y][x];

        sum += currPixel;
        varc += currPixel*currPixel;

        dif = currPixel - prevPixel;
        vard += dif*dif;
    }
    }

    vard >>= 8;        /* divide by 256; assumes mean is close to zero */
    varc = (varc>>8) - (sum>>8)*(sum>>8);

    if ( vard <= 64 ) {
    return FALSE;
    } else if ( vard < varc ) {
    return FALSE;
    } else {
    return TRUE;
    }
}

static int
ComputeBlockColorDiff(current, motionBlock)
    Block current, motionBlock;
{
  register int x, y, diff_total = 0, diff_tmp;
  
  for ( y = 0; y < 8; y++ ) {
    for ( x = 0; x < 8; x++ ) {
      diff_tmp = current[y][x] - motionBlock[y][x];
      diff_total += ABS(diff_tmp);
    }
  }
  return diff_total;
}

/*===========================================================================*
 *
 *                USER-MODIFIABLE
 *
 * MotionSufficient
 *
 *    decides if this motion vector is sufficient without DCT coding
 *
 * RETURNS:    TRUE if no DCT is needed; FALSE otherwise
 *
 * SIDE EFFECTS:    none
 *
 * PRECONDITION:    the motion vectors must be valid!
 *
 *===========================================================================*/
static boolean
MotionSufficient(curr, currBlock, prev, next, by, bx, mode, fmy, fmx, bmy, bmx)
    MpegFrame *curr;
    LumBlock currBlock;
    MpegFrame *prev;
    MpegFrame *next;
    int by, bx;
    int mode;
    int fmy, fmx;
    int bmy, bmx;
{
    LumBlock   mLumBlock;
    Block mColorBlock;
    int lumErr, colorErr;

    /* check bounds */
    if ( mode != MOTION_BACKWARD ) {
    if ( (by*DCTSIZE+(fmy-1)/2 < 0) || ((by+2)*DCTSIZE+(fmy+1)/2-1 >= Fsize_y) ) {
        return FALSE;
    }
    if ( (bx*DCTSIZE+(fmx-1)/2 < 0) || ((bx+2)*DCTSIZE+(fmx+1)/2-1 >= Fsize_x) ) {
        return FALSE;
    }
    }

    if ( mode != MOTION_FORWARD ) {
    if ( (by*DCTSIZE+(bmy-1)/2 < 0) || ((by+2)*DCTSIZE+(bmy+1)/2-1 >= Fsize_y) ) {
        return FALSE;
    }
    if ( (bx*DCTSIZE+(bmx-1)/2 < 0) || ((bx+2)*DCTSIZE+(bmx+1)/2-1 >= Fsize_x) ) {
        return FALSE;
    }
    }

    /* check Lum */
    ComputeBMotionLumBlock(prev, next, by, bx, mode, fmy, fmx,
               bmy, bmx, mLumBlock);
    lumErr =  LumBlockMAD(currBlock, mLumBlock, 0x7fffffff);
    if (lumErr > 512) {
      return FALSE;
    }

    /* check color */
    ComputeBMotionBlock(prev, next, by>>1, bx>>1, mode, fmy/2, fmx/2, 
            bmy/2, bmx/2, mColorBlock, CR_BLOCK);
    colorErr = ComputeBlockColorDiff(curr->cr_blocks[by >> 1][bx >> 1], mColorBlock);
    ComputeBMotionBlock(prev, next, by>>1, bx>>1, mode, fmy/2, fmx/2, 
            bmy/2, bmx/2, mColorBlock, CB_BLOCK);
    colorErr += ComputeBlockColorDiff(curr->cr_blocks[by >> 1][bx >> 1], mColorBlock);
    
    return (colorErr < 256); /* lumErr checked above */
}


