/*===========================================================================*
 * iframe.c                                     *
 *                                         *
 *    Procedures concerned with the I-frame encoding                 *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    GenIFrame                                 *
 *    SetSlicesPerFrame                             *
 *    SetBlocksPerSlice                             *
 *    SetIQScale                                 *
 *    GetIQScale                                 *
 *    ResetIFrameStats                             *
 *    ShowIFrameSummary                             *
 *    EstimateSecondsPerIFrame                         *
 *    EncodeYDC                                 *
 *    EncodeCDC                                 *
 *      time_elapsed                                                         *
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
 *  $Header: /n/picasso/project/mpeg/mpeg_dist/mpeg_encode/RCS/iframe.c,v 1.23 1995/08/14 22:29:49 smoot Exp $
 *  $Log: iframe.c,v $
 *  Revision 1.23  1995/08/14 22:29:49  smoot
 *  changed inits in BlockComputeSNR so sgi compiler would be happy
 *
 *  Revision 1.22  1995/08/07 21:50:51  smoot
 *  spacing
 *  simplified some code
 *  added Laplace stuff
 *  minor error check bug in alloc
 *
 * Revision 1.21  1995/06/21  22:24:25  smoot
 * added CalcDistortion (TUNEing)
 * fixed timeing stuff for ANSI
 * fixed specifics bug
 *
 * Revision 1.20  1995/05/02  21:59:43  smoot
 * fixed BlockComputeSNR bugs
 *
 * Revision 1.19  1995/04/24  23:02:50  smoot
 * Fixed BlockComputeSNR for Linux and others
 *
 * Revision 1.18  1995/04/14  23:08:02  smoot
 * reorganized to ease rate control experimentation
 *
 * Revision 1.17  1995/02/24  23:49:38  smoot
 * added support for Specifics file version 2
 *
 * Revision 1.16  1995/01/30  20:02:34  smoot
 * cleanup, killed a couple warnings
 *
 * Revision 1.15  1995/01/30  19:49:17  smoot
 * cosmetic
 *
 * Revision 1.14  1995/01/23  02:49:34  darryl
 * initialized variable
 *
 * Revision 1.13  1995/01/19  23:08:30  eyhung
 * Changed copyrights
 *
 * Revision 1.12  1995/01/16  08:01:34  eyhung
 * Added realQuiet
 *
 * Revision 1.11  1994/12/07  00:40:36  smoot
 * Added seperate P and B search ranges
 *
 * Revision 1.10  1994/11/14  22:30:30  smoot
 * Merged specifics and rate control
 *
 * Revision 1.9  1994/11/01  05:00:48  darryl
 * with rate control changes added
 *
 * Revision 2.2  1994/10/31  00:06:07  darryl
 * version before, hopefully, final changes
 *
 * Revision 2.1  1994/10/24  22:03:01  darryl
 * put in preliminary experiments code
 *
 * Revision 2.0  1994/10/24  02:38:04  darryl
 * will be adding the experiment stuff.
 *
 * Revision 1.1  1994/09/27  00:15:24  darryl
 * Initial revision
 *
 * Revision 1.8  1994/03/15  00:27:11  keving
 * nothing
 *
 * Revision 1.7  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.6  1993/07/22  22:23:43  keving
 * nothing
 *
 * Revision 1.5  1993/06/30  20:06:09  keving
 * nothing
 *
 * Revision 1.4  1993/06/03  21:08:08  keving
 * nothing
 *
 * Revision 1.3  1993/03/04  22:24:06  keving
 * nothing
 *
 * Revision 1.2  1993/02/19  18:10:02  keving
 * nothing
 *
 * Revision 1.1  1993/02/18  22:56:39  keving
 * nothing
 *
 *
 */


/*==============*
 * HEADER FILES *
 *==============*/


#ifdef CLOCKS_PER_SEC
#include <times.h>
#else
#include <sys/times.h>
#endif

#include <sys/param.h>
#include "all.h"
#include "mtypes.h"
#include "frames.h"
#include "prototypes.h"
#include "mpeg.h"
#include "param.h"
#include "mheaders.h"
#include "fsize.h"
#include "parallel.h"
#include "postdct.h"
#include "rate.h"
#include "opts.h"

/*==================*
 * STATIC VARIABLES *
 *==================*/

static  int    lastNumBits = 0;
static  int    lastIFrame = 0;
static int numBlocks = 0;
static int numBits;
static int numFrames = 0;
static int numFrameBits = 0;
static int32_mpeg_t totalTime = 0;
static float    totalSNR = 0.0;
static float    totalPSNR = 0.0;

static int lengths[256] = {
    0, 1, 2, 2, 3, 3, 3, 3,        /* 0 - 7 */
    4, 4, 4, 4, 4, 4, 4, 4,        /* 8 - 15 */
    5, 5, 5, 5, 5, 5, 5, 5,        /* 16 - 31 */
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6,        /* 32 - 63 */
    6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7,        /* 64 - 127 */
    7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8
};


/*==================*
 * GLOBAL VARIABLES *
 *==================*/

int    qscaleI;
int    slicesPerFrame;
int    blocksPerSlice;
int    fCodeI, fCodeP, fCodeB;
boolean    printSNR = FALSE;
boolean    printMSE = FALSE;
boolean    decodeRefFrames = FALSE;
Block **dct=NULL, **dctr=NULL, **dctb=NULL;
dct_data_type   **dct_data; /* used in p/bframe.c */
int  TIME_RATE;


/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/
extern void    PrintItoIBitRate _ANSI_ARGS_((int numBits, int frameNum));

/*===============================*
 * INTERNAL PROCEDURE prototypes *
 *===============================*/
void AllocDctBlocks _ANSI_ARGS_((void ));
int SetFCodeHelper _ANSI_ARGS_((int sr));
void CalcDistortion _ANSI_ARGS_((MpegFrame *current, int y, int x));

int
  SetFCodeHelper(SR)
int SR;
{
    int        range,fCode;

    if ( pixelFullSearch ) {
    range = SR;
    } else {
    range = SR*2;
    }

    if ( range < 256 ) {
    if ( range < 64 ) {
        if ( range < 32 ) {
        fCode = 1;
        } else {
        fCode = 2;
        }
    } else {
        if ( range < 128 ) {
        fCode = 3;
        } else {
        fCode = 4;
        }
    }
    } else {
    if ( range < 1024 ) {
        if ( range < 512 ) {
        fCode = 5;
        } else {
        fCode = 6;
        }
    } else {
        if ( range < 2048 ) {
        fCode = 7;
        } else {
        fprintf(stderr, "ERROR:  INVALID SEARCH RANGE!!!\n");
        exit(1);
        }
    }
      }
    return fCode;
}

/*===========================================================================*
 *
 * SetFCode
 *
 *    set the forward_f_code and backward_f_code according to the search
 *    range.  Must be called AFTER pixelFullSearch and searchRange have
 *    been initialized.  Irrelevant for I-frames, but computation is
 *    negligible (done only once, as well)
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    fCodeI,fCodeP,fCodeB
 *
 *===========================================================================*/
void
SetFCode()
{
  fCodeI = SetFCodeHelper(1); /* GenIFrame ignores value */
  fCodeP = SetFCodeHelper(searchRangeP);
  fCodeB = SetFCodeHelper(searchRangeB);
}

/*===========================================================================*
 *
 * SetSlicesPerFrame
 *
 *    set the number of slices per frame
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    slicesPerFrame
 *
 *===========================================================================*/
void
SetSlicesPerFrame(number)
    int number;
{
    slicesPerFrame = number;
}


/*===========================================================================*
 *
 * SetBlocksPerSlice
 *
 *    set the number of blocks per slice, based on slicesPerFrame
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    blocksPerSlice
 *
 *===========================================================================*/
void
SetBlocksPerSlice()
{
    int        totalBlocks;

    totalBlocks = (Fsize_y>>4)*(Fsize_x>>4);

    if ( slicesPerFrame > totalBlocks ) {
    blocksPerSlice = 1;
    } else {
    blocksPerSlice = totalBlocks/slicesPerFrame;
    }
}


/*===========================================================================*
 *
 * SetIQScale
 *
 *    set the I-frame Q-scale
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    qscaleI
 *
 *===========================================================================*/
void
SetIQScale(qI)
int qI;
{
    qscaleI = qI;
}

/*===========================================================================*
 *
 * GetIQScale
 *
 *    Get the I-frame Q-scale
 *
 * RETURNS:    the Iframe Qscale
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
int
GetIQScale()
{
    return qscaleI;
}

/*===========================================================================*
 *
 * GenIFrame
 *
 *    generate an I-frame; appends result to bb
 *
 * RETURNS:    I-frame appended to bb
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
GenIFrame(bb, current)
     BitBucket *bb;
     MpegFrame *current;
{
    register int x, y;
    register int index;
    FlatBlock     fb[6];
    Block      dec[6];
    int32_mpeg_t y_dc_pred, cr_dc_pred, cb_dc_pred;
    int          totalBits;
    int             totalFrameBits;
    int32_mpeg_t        startTime, endTime;
    float        snr[3], psnr[3];
    int             mbAddress;
    int          QScale;
    BlockMV      *info; /* Not used in Iframes, but nice to pass in anyway */
    int          bitstreamMode, newQScale;
    int          rc_blockStart=0;

    if (dct==NULL) AllocDctBlocks();
    if (collect_quant) {fprintf(collect_quant_fp, "# I\n");}

    /* set-up for statistics */
    numFrames++;
    totalFrameBits = bb->cumulativeBits;
    if ( ( ! childProcess) && showBitRatePerFrame ) {
      if ( lastNumBits == 0 ) {
    lastNumBits = bb->cumulativeBits;
    lastIFrame = current->id;
      } else {
    /* ASSUMES 30 FRAMES PER SECOND */
    
    if (! realQuiet) {
      fprintf(stdout, "I-to-I (frames %5d to %5d) bitrate:  %8d\n",
          lastIFrame, current->id-1,
          ((bb->cumulativeBits-lastNumBits)*30)/
          (current->id-lastIFrame));
    }
    
    fprintf(bitRateFile, "I-to-I (frames %5d to %5d) bitrate:  %8d\n",
        lastIFrame, current->id-1,
        ((bb->cumulativeBits-lastNumBits)*30)/
        (current->id-lastIFrame));
    lastNumBits = bb->cumulativeBits;        
    lastIFrame = current->id;
      }
    }
    
    startTime = time_elapsed();
    
    Frame_AllocBlocks(current);
    BlockifyFrame(current);

    DBG_PRINT(("Generating iframe\n"));
    QScale = GetIQScale();
    /*   Allocate bits for this frame for rate control purposes */
    bitstreamMode = getRateMode();
    if (bitstreamMode == FIXED_RATE) {
      targetRateControl(current);
    }

    Mhead_GenPictureHeader(bb, I_FRAME, current->id, fCodeI);
    /* Check for Qscale change */
    if (specificsOn) {
      newQScale = SpecLookup(current->id, 0, 0 /* junk */, &info, QScale);
      if (newQScale != -1) {
    QScale = newQScale;
      }
      /* check for slice */
      newQScale = SpecLookup(current->id, 1, 1, &info, QScale);
      if (newQScale != -1) {
    QScale = newQScale;
      }
    }
    Mhead_GenSliceHeader(bb, 1, QScale, NULL, 0);
    
    if ( referenceFrame == DECODED_FRAME ) {
      Frame_AllocDecoded(current, TRUE);
    } else if ( printSNR ) {
      Frame_AllocDecoded(current, FALSE);
    }
    
    y_dc_pred = cr_dc_pred = cb_dc_pred = 128;
    totalBits = bb->cumulativeBits;
    mbAddress = 0;

    /* DCT the macroblocks */
    for (y = 0;  y < (Fsize_y >> 3);  y += 2) {
      for (x = 0;  x < (Fsize_x >> 3);  x += 2) {
    if (collect_quant && (collect_quant_detailed & 1)) fprintf(collect_quant_fp, "l\n");
    if (DoLaplace) {LaplaceCnum = 0;}
    mp_fwd_dct_block2(current->y_blocks[y][x], dct[y][x]);
    mp_fwd_dct_block2(current->y_blocks[y][x+1], dct[y][x+1]);
    mp_fwd_dct_block2(current->y_blocks[y+1][x], dct[y+1][x]);
    mp_fwd_dct_block2(current->y_blocks[y+1][x+1], dct[y+1][x+1]);
    if (collect_quant && (collect_quant_detailed & 1)) fprintf(collect_quant_fp, "c\n");
    if (DoLaplace) {LaplaceCnum = 1;}
    mp_fwd_dct_block2(current->cb_blocks[y>>1][x>>1], dctb[y>>1][x>>1]);
    if (DoLaplace) {LaplaceCnum = 2;}
    mp_fwd_dct_block2(current->cr_blocks[y>>1][x>>1], dctr[y>>1][x>>1]);
      }}
    
    if (DoLaplace) {
      extern void CalcLambdas();
      CalcLambdas();
    }

    for (y = 0;  y < (Fsize_y >> 3);  y += 2) {
      for (x = 0;  x < (Fsize_x >> 3);  x += 2) {
    /* Check for Qscale change */
    if (specificsOn) {
      newQScale = SpecLookup(current->id, 2, mbAddress, &info, QScale);
      if (newQScale != -1) {
        QScale = newQScale;
      }
    }
    
    /*  Determine if new Qscale needed for Rate Control purposes  */
    if (bitstreamMode == FIXED_RATE) {
      rc_blockStart = bb->cumulativeBits;
      newQScale = needQScaleChange(qscaleI,
                       current->y_blocks[y][x],
                       current->y_blocks[y][x+1],
                       current->y_blocks[y+1][x],
                       current->y_blocks[y+1][x+1]);
      if (newQScale > 0) {
        QScale = newQScale;
      }
    }
    
    if ( (mbAddress % blocksPerSlice == 0) && (mbAddress != 0) ) {
      /* create a new slice */
      if (specificsOn) {
        /* Make sure no slice Qscale change */
        newQScale = SpecLookup(current->id,1,mbAddress/blocksPerSlice, &info, QScale);
        if (newQScale != -1) QScale = newQScale;
      }
      Mhead_GenSliceEnder(bb);
      Mhead_GenSliceHeader(bb, 1+(y>>1), QScale, NULL, 0);
      y_dc_pred = cr_dc_pred = cb_dc_pred = 128;
      
      GEN_I_BLOCK(I_FRAME, current, bb, 1+(x>>1), QScale);
    } else {
      GEN_I_BLOCK(I_FRAME, current, bb, 1, QScale);
    }

    if (WriteDistortionNumbers) {
      CalcDistortion(current, y, x);
    }
    
    if ( decodeRefFrames ) {
      /* now, reverse the DCT transform */
      LaplaceCnum = 0;
      for ( index = 0; index < 6; index++ ) {
        if (!DoLaplace) {
          Mpost_UnQuantZigBlock(fb[index], dec[index], QScale, TRUE);
        } else {
          if (index == 4) {LaplaceCnum = 1;}
          if (index == 5) {LaplaceCnum = 2;}
          Mpost_UnQuantZigBlockLaplace(fb[index], dec[index], QScale, TRUE);
        }
        mpeg_jrevdct((int16_mpeg_t *)dec[index]);        
        }
      
      /* now, unblockify */
      BlockToData(current->decoded_y, dec[0], y, x);
      BlockToData(current->decoded_y, dec[1], y, x+1);
      BlockToData(current->decoded_y, dec[2], y+1, x);
      BlockToData(current->decoded_y, dec[3], y+1, x+1);
      BlockToData(current->decoded_cb, dec[4], y>>1, x>>1);
      BlockToData(current->decoded_cr, dec[5], y>>1, x>>1);
    }
    
    numBlocks++;
    mbAddress++;
    /*   Rate Control */
    if (bitstreamMode == FIXED_RATE) {
      incMacroBlockBits(bb->cumulativeBits - rc_blockStart);
      rc_blockStart = bb->cumulativeBits;
      MB_RateOut(TYPE_IFRAME);
    }
      }
    }
    
    if ( printSNR ) {
      BlockComputeSNR(current,snr,psnr);
      totalSNR += snr[0];
      totalPSNR += psnr[0];
    }
    
    if ( (referenceFrame == DECODED_FRAME) && NonLocalRefFrame(current->id) ) {
      if ( remoteIO ) {
    SendDecodedFrame(current);
      } else {
    WriteDecodedFrame(current);
      }
      
      /* now, tell decode server it is ready */
      NotifyDecodeServerReady(current->id);
    }
    
    numBits += (bb->cumulativeBits-totalBits);
    
    DBG_PRINT(("End of frame\n"));
    
    Mhead_GenSliceEnder(bb);
    /*   Rate Control  */
    if (bitstreamMode == FIXED_RATE) {
      updateRateControl(TYPE_IFRAME);
    }
    
    endTime = time_elapsed();
    totalTime += (endTime-startTime);
    
    numFrameBits += (bb->cumulativeBits-totalFrameBits);
    
    if ( ( ! childProcess) && showBitRatePerFrame ) {
      /* ASSUMES 30 FRAMES PER SECOND */
      fprintf(bitRateFile, "%5d\t%8d\n", current->id,
          30*(bb->cumulativeBits-totalFrameBits));
    }
    
    if ( (! childProcess) && frameSummary && (! realQuiet) ) {
      
      /* ASSUMES 30 FRAMES PER SECOND */
      fprintf(stdout, "FRAME %d (I):  %ld seconds  (%d bits/s output)\n", 
          current->id, (long)((endTime-startTime)/TIME_RATE),
          30*(bb->cumulativeBits-totalFrameBits));
      if ( printSNR ) {
    fprintf(stdout, "FRAME %d:  SNR:  %.1f\t%.1f\t%.1f\tPSNR:  %.1f\t%.1f\t%.1f\n",
        current->id, snr[0], snr[1], snr[2],
        psnr[0], psnr[1], psnr[2]);
      }
    }
}


/*===========================================================================*
 *
 * ResetIFrameStats
 *
 *    reset the I-frame statistics
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
ResetIFrameStats()
{
    numBlocks = 0;
    numBits = 0;
    numFrames = 0;
    numFrameBits = 0;
    totalTime = 0;
}


/*===========================================================================*
 *
 * ShowIFrameSummary
 *
 *    prints out statistics on all I-frames
 *
 * RETURNS:    time taken for I-frames (in seconds)
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
float
ShowIFrameSummary(inputFrameBits, totalBits, fpointer)
    int inputFrameBits;
    int32_mpeg_t totalBits;
    FILE *fpointer;
{
    if ( numFrames == 0 ) {
    return 0.0;
    }

    fprintf(fpointer, "-------------------------\n");
    fprintf(fpointer, "*****I FRAME SUMMARY*****\n");
    fprintf(fpointer, "-------------------------\n");

    fprintf(fpointer, "  Blocks:    %5d     (%6d bits)     (%5d bpb)\n",
        numBlocks, numBits, numBits/numBlocks);
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

    return (float)totalTime/(float)TIME_RATE;
}


/*===========================================================================*
 *
 * EstimateSecondsPerIFrame
 *
 *    estimates the number of seconds required per I-frame
 *
 * RETURNS:    seconds (floating point value)
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
float
EstimateSecondsPerIFrame()
{
    return (float)totalTime/((float)TIME_RATE*(float)numFrames);
}


/*===========================================================================*
 *
 * EncodeYDC
 *
 *    Encode the DC portion of a DCT of a luminance block
 *
 * RETURNS:    result appended to bb
 *
 * SIDE EFFECTS:    updates pred_term
 *
 *===========================================================================*/
void
EncodeYDC(dc_term, pred_term, bb)
    int32_mpeg_t dc_term;
    int32_mpeg_t *pred_term;
    BitBucket *bb;
{
    /* see Table B.5a -- MPEG-I doc */
    static int codes[9] = {
    0x4, 0x0, 0x1, 0x5, 0x6, 0xe, 0x1e, 0x3e, 0x7e
    };
    static int codeLengths[9] = {
    3,   2,   2,   3,   3,   4,   5,    6,    7
    };
    int ydiff, ydiff_abs;
    int    length;

    ydiff = (dc_term - (*pred_term));
    if (ydiff > 255) {
#ifdef BLEAH 
      fprintf(stdout, "TRUNCATED\n");
#endif
    ydiff = 255;
    } else if (ydiff < -255) {
#ifdef BLEAH 
      fprintf(stdout, "TRUNCATED\n");
#endif
    ydiff = -255;
    }

    ydiff_abs = ABS(ydiff);
    length = lengths[ydiff_abs];
    Bitio_Write(bb, codes[length], codeLengths[length]);
    if ( length != 0 ) {
    if ( ydiff > 0 ) {
        Bitio_Write(bb, ydiff_abs, length);
    } else {
        Bitio_Write(bb, ~ydiff_abs, length);
    }
    }

    (*pred_term) += ydiff;
}


/*===========================================================================*
 *
 * EncodeCDC
 *
 *    Encode the DC portion of a DCT of a chrominance block
 *
 * RETURNS:    result appended to bb
 *
 * SIDE EFFECTS:    updates pred_term
 *
 *===========================================================================*/
void
EncodeCDC(dc_term, pred_term, bb)
    int32_mpeg_t dc_term;
    int32_mpeg_t *pred_term;
    BitBucket *bb;
{
    /* see Table B.5b -- MPEG-I doc */
    static int codes[9] = {
    0x0, 0x1, 0x2, 0x6, 0xe, 0x1e, 0x3e, 0x7e, 0xfe
    };
    static int codeLengths[9] = {
    2,   2,   2,   3,   4,   5,    6,    7,    8
    };
    int cdiff, cdiff_abs;
    int    length;

    cdiff = (dc_term - (*pred_term));
    if (cdiff > 255) {
#ifdef BLEAH
fprintf(stdout, "TRUNCATED\n");    
#endif
    cdiff = 255;
    } else if (cdiff < -255) {
#ifdef BLEAH
fprintf(stdout, "TRUNCATED\n");    
#endif
    cdiff = -255;
    }

    cdiff_abs = ABS(cdiff);
    length = lengths[cdiff_abs];
    Bitio_Write(bb, codes[length], codeLengths[length]);
    if ( length != 0 ) {
    if ( cdiff > 0 ) {
        Bitio_Write(bb, cdiff_abs, length);
    } else {
        Bitio_Write(bb, ~cdiff_abs, length);
    }
    }

    (*pred_term) += cdiff;
}


void
BlockComputeSNR(current, snr, psnr)
     MpegFrame *current;
     float snr[];
     float psnr[];
{
  register int32_mpeg_t    tempInt;
  register int y, x;
  int32_mpeg_t    varDiff[3];
  double    ratio[3];
  double    total[3];
  register uint8_mpeg_t **origY=current->orig_y, **origCr=current->orig_cr, 
  **origCb=current->orig_cb;
  register uint8_mpeg_t **newY=current->decoded_y, **newCr=current->decoded_cr, 
  **newCb=current->decoded_cb;
  static int32_mpeg_t       **SignalY,  **NoiseY;
  static int32_mpeg_t       **SignalCb, **NoiseCb;
  static int32_mpeg_t       **SignalCr, **NoiseCr;
  static short   ySize[3], xSize[3];
  static boolean needs_init=TRUE;
  
  /* Init */
  if (needs_init) {
    int ysz = (Fsize_y>>3) * sizeof(int32_mpeg_t *);
    int xsz = (Fsize_x>>3);
    
    needs_init = FALSE;
    for (y=0; y<3; y++) {
      varDiff[y] = ratio[y] = total[y] = 0.0;
    }
    ySize[0]=Fsize_y;     xSize[0]=Fsize_x;
    ySize[1]=Fsize_y>>1;  xSize[1]=Fsize_x>>1;
    ySize[2]=Fsize_y>>1;  xSize[2]=Fsize_x>>1;
    SignalY  = (int32_mpeg_t **) malloc(ysz);
    NoiseY   = (int32_mpeg_t **) malloc(ysz);
    SignalCb = (int32_mpeg_t **) malloc(ysz);
    NoiseCb  = (int32_mpeg_t **) malloc(ysz);
    SignalCr = (int32_mpeg_t **) malloc(ysz);
    NoiseCr  = (int32_mpeg_t **) malloc(ysz);
    if (SignalY == NULL || NoiseY == NULL || SignalCr == NULL || 
    NoiseCb == NULL || SignalCb == NULL || NoiseCr == NULL) {
      fprintf(stderr, "Out of memory in BlockComputeSNR\n");
      exit(-1);
    }
    for (y = 0; y < ySize[0]>>3; y++) {
      SignalY[y]  = (int32_mpeg_t *) calloc(xsz,4);
      SignalCr[y]  = (int32_mpeg_t *) calloc(xsz,4);
      SignalCb[y]  = (int32_mpeg_t *) calloc(xsz,4);
      NoiseY[y]  = (int32_mpeg_t *) calloc(xsz,4);
      NoiseCr[y]  = (int32_mpeg_t *) calloc(xsz,4);
      NoiseCb[y]  = (int32_mpeg_t *) calloc(xsz,4);
    }
  } else {
    for (y = 0; y < ySize[0]>>3; y++) {
      memset((char *) &NoiseY[y][0], 0, (xSize[0]>>3) * 4);
      memset((char *) &SignalY[y][0], 0, (xSize[0]>>3) * 4);
      memset((char *) &NoiseCb[y][0], 0, (xSize[0]>>3) * 4);
      memset((char *) &NoiseCr[y][0], 0, (xSize[0]>>3) * 4);
      memset((char *) &SignalCb[y][0], 0, (xSize[0]>>3) * 4);
      memset((char *) &SignalCr[y][0], 0, (xSize[0]>>3) * 4);
    }
  }
  
  /* find all the signal and noise */
  for (y = 0; y < ySize[0]; y++) {
    for (x = 0; x < xSize[0]; x++) {
      tempInt = (origY[y][x] - newY[y][x]);
      NoiseY[y>>4][x>>4] += tempInt*tempInt;
      total[0] += (double)abs(tempInt);
      tempInt = origY[y][x];
      SignalY[y>>4][x>>4] += tempInt*tempInt;
    }}
  for (y = 0; y < ySize[1]; y++) {
    for (x = 0; x < xSize[1]; x ++) {
      tempInt = (origCb[y][x] - newCb[y][x]);
      NoiseCb[y>>3][x>>3] += tempInt*tempInt;
      total[1] += (double)abs(tempInt);
      tempInt = origCb[y][x];
      SignalCb[y>>3][x>>3] += tempInt*tempInt;
      tempInt = (origCr[y][x]-newCr[y][x]);
      NoiseCr[y>>3][x>>3] += tempInt*tempInt;
      total[2] += (double)abs(tempInt);
      tempInt = origCr[y][x];
      SignalCr[y>>3][x>>3] += tempInt*tempInt;
    }}
  
  /* Now sum up that noise */
  for(y=0; y<Fsize_y>>4; y++){
    for(x=0; x<Fsize_x>>4; x++){
      varDiff[0] += NoiseY[y][x];
      varDiff[1] += NoiseCb[y][x];
      varDiff[2] += NoiseCr[y][x];
      if (printMSE) printf("%4d ",(int)(NoiseY[y][x]/256.0));
    }
    if (printMSE) puts("");
  }
  
  /* Now look at those ratios! */
  for(y=0; y<Fsize_y>>4; y++){
    for(x=0; x<Fsize_x>>4; x++){
      ratio[0] += (double)SignalY[y][x]/(double)varDiff[0];
      ratio[1] += (double)SignalCb[y][x]/(double)varDiff[1];
      ratio[2] += (double)SignalCr[y][x]/(double)varDiff[2];
    }}
  
  for (x=0; x<3; x++) {
    snr[x] = 10.0*log10(ratio[x]);
    psnr[x] = 20.0*log10(255.0/sqrt((double)varDiff[x]/(double)(ySize[x]*xSize[x])));

    if (! realQuiet) {
      fprintf(stdout, "Mean error[%1d]:  %f\n", x, total[x]/(double)(xSize[x]*ySize[x]));
    }

  }
}

void
WriteDecodedFrame(frame)
    MpegFrame *frame;
{
    FILE    *fpointer;
    char    fileName[256];
    int    width, height;
    register int y;

    /* need to save decoded frame to disk because it might be accessed
       by another process */

    width = Fsize_x;
    height = Fsize_y;

    sprintf(fileName, "%s.decoded.%d", outputFileName, frame->id);

    if (!realQuiet) {
    fprintf(stdout, "Outputting to %s\n", fileName);
    fflush(stdout);
    }

    fpointer = fopen(fileName, "wb");

    for ( y = 0; y < height; y++ ) {
        fwrite(frame->decoded_y[y], 1, width, fpointer);
    }

    for (y = 0; y < (height >> 1); y++) {            /* U */
        fwrite(frame->decoded_cb[y], 1, width >> 1, fpointer);
    }

    for (y = 0; y < (height >> 1); y++) {            /* V */
        fwrite(frame->decoded_cr[y], 1, width >> 1, fpointer);
    }
    fflush(fpointer);
    fclose(fpointer);
}


void
PrintItoIBitRate(numBits, frameNum)
    int        numBits;
    int        frameNum;
{
    if ( ( ! childProcess) && showBitRatePerFrame ) {
    /* ASSUMES 30 FRAMES PER SECOND */

    if (! realQuiet) {
    fprintf(stdout, "I-to-I (frames %5d to %5d) bitrate:  %8d\n",
        lastIFrame, frameNum-1,
        ((numBits-lastNumBits)*30)/
        (frameNum-lastIFrame));
        }

    fprintf(bitRateFile, "I-to-I (frames %5d to %5d) bitrate:  %8d\n",
        lastIFrame, frameNum-1,
        ((numBits-lastNumBits)*30)/
        (frameNum-lastIFrame));
    }
}


/*===========================================================================*
 *
 * AllocDctBlocks
 *
 *    allocate memory for dct blocks
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    creates dct, dctr, dctb
 *
 *===========================================================================*/
void
AllocDctBlocks()
{
    int dctx, dcty;
    int i;

    dctx = Fsize_x / DCTSIZE;
    dcty = Fsize_y / DCTSIZE;

    dct = (Block **) malloc(sizeof(Block *) * dcty);
    ERRCHK(dct, "malloc");
    for (i = 0; i < dcty; i++) {
    dct[i] = (Block *) malloc(sizeof(Block) * dctx);
    ERRCHK(dct[i], "malloc");
    }

    dct_data = (dct_data_type **) malloc(sizeof(dct_data_type *) * dcty);
    ERRCHK(dct_data, "malloc");
    for (i = 0; i < dcty; i++) {
    dct_data[i] = (dct_data_type *) malloc(sizeof(dct_data_type) * dctx);
    ERRCHK(dct[i], "malloc");
    }

    dctr = (Block **) malloc(sizeof(Block *) * (dcty >> 1));
    dctb = (Block **) malloc(sizeof(Block *) * (dcty >> 1));
    ERRCHK(dctr, "malloc");
    ERRCHK(dctb, "malloc");
    for (i = 0; i < (dcty >> 1); i++) {
    dctr[i] = (Block *) malloc(sizeof(Block) * (dctx >> 1));
    dctb[i] = (Block *) malloc(sizeof(Block) * (dctx >> 1));
    ERRCHK(dctr[i], "malloc");
    ERRCHK(dctb[i], "malloc");
    }
}


/*======================================================================*
 *
 * time_elapsed
 *
 *     Handle different time systems on different machines
 *
 *  RETURNS number of seconds process time used
 *
 *======================================================================*/
int32_mpeg_t time_elapsed()
{
#ifdef CLOCKS_PER_SEC
 /* ANSI C */
  TIME_RATE = CLOCKS_PER_SEC;
  return (int32_mpeg_t) clock();
#else
  struct tms   timeBuffer;
  TIME_RATE = 60;
  times(&timeBuffer);
  return timeBuffer.tms_utime + timeBuffer.tms_stime;
#endif
}


void
CalcDistortion(current, y, x)
MpegFrame *current;
int y,x;
{

  int qscale, distort=0;
  Block decblk;
  FlatBlock fblk;
  int datarate = 0;
  
  for (qscale = 1; qscale < 32; qscale ++) {
    distort = 0;
    datarate = 0;
    Mpost_QuantZigBlock(dct[y][x], fblk, qscale, TRUE);
    Mpost_UnQuantZigBlock(fblk, decblk, qscale, TRUE);
    if (collect_distortion_detailed) datarate += CalcRLEHuffLength(fblk);
    mpeg_jrevdct((int16_mpeg_t *)decblk);
    distort += mse(current->y_blocks[y][x], decblk);

    Mpost_QuantZigBlock(dct[y][x+1], fblk, qscale, TRUE);
    Mpost_UnQuantZigBlock(fblk, decblk, qscale, TRUE);
    if (collect_distortion_detailed) datarate += CalcRLEHuffLength(fblk);
    mpeg_jrevdct((int16_mpeg_t *)decblk);
    distort += mse(current->y_blocks[y][x+1], decblk);

    Mpost_QuantZigBlock(dct[y+1][x], fblk, qscale, TRUE);
    Mpost_UnQuantZigBlock(fblk, decblk, qscale, TRUE);
    if (collect_distortion_detailed) datarate += CalcRLEHuffLength(fblk);
    mpeg_jrevdct((int16_mpeg_t *)decblk);
    distort += mse(current->y_blocks[y+1][x], decblk);

    Mpost_QuantZigBlock(dct[y+1][x+1], fblk, qscale, TRUE);
    Mpost_UnQuantZigBlock(fblk, decblk, qscale, TRUE);
    if (collect_distortion_detailed) datarate += CalcRLEHuffLength(fblk);
    mpeg_jrevdct((int16_mpeg_t *)decblk);
    distort += mse(current->y_blocks[y+1][x+1], decblk);

    Mpost_QuantZigBlock(dctb[y >> 1][x >> 1], fblk, qscale, TRUE);
    Mpost_UnQuantZigBlock(fblk, decblk, qscale, TRUE);
    if (collect_distortion_detailed) datarate += CalcRLEHuffLength(fblk);
    mpeg_jrevdct((int16_mpeg_t *)decblk);
    distort += mse(current->cb_blocks[y>>1][x>>1], decblk);

    Mpost_QuantZigBlock(dctr[y >> 1][x >> 1], fblk, qscale, TRUE);
    Mpost_UnQuantZigBlock(fblk, decblk, qscale, TRUE);
    if (collect_distortion_detailed) datarate += CalcRLEHuffLength(fblk);
    mpeg_jrevdct((int16_mpeg_t *)decblk);
    distort += mse(current->cr_blocks[y >> 1][x >> 1], decblk);

    if (!collect_distortion_detailed) {
      fprintf(distortion_fp, "\t%d\n", distort);
    } else if (collect_distortion_detailed == 1) {
      fprintf(distortion_fp, "\t%d\t%d\n", distort, datarate);
    } else {
      fprintf(fp_table_rate[qscale-1], "%d\n", datarate);
      fprintf(fp_table_dist[qscale-1], "%d\n", distort);
    }
  }
}




