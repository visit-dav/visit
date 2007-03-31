/*===========================================================================*
 * block.c                                     *
 *                                         *
 *    Block routines                                 *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    ComputeDiffDCTBlock                             *
 *    ComputeDiffDCTs                                 *
 *    ComputeMotionBlock                             *
 *    ComputeMotionLumBlock                             *
 *    LumBlockMAD                                 *
 *    LumMotionError                                 *
 *    LumMotionErrorSubSampled                         *
 *    LumAddMotionError                             *
 *    AddMotionBlock                                 *
 *    BlockToData                                 *
 *    BlockifyFrame                                 *
 *                                         *
 * NOTES:   MAD    =   Mean Absolute Difference                     *
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
 *  $Header: /n/picasso/project/mpeg/mpeg_dist/mpeg_encode/RCS/block.c,v 1.16 1995/08/07 21:43:29 smoot Exp $
 *  $Log: block.c,v $
 *  Revision 1.16  1995/08/07 21:43:29  smoot
 *  restructured lumdiff so it read better and used a switch instead of ifs
 *
 *  Revision 1.15  1995/06/21 22:21:16  smoot
 *  added TUNEing options
 *
 * Revision 1.14  1995/05/08  22:47:45  smoot
 * typechecking better
 *
 * Revision 1.13  1995/05/08  22:44:14  smoot
 * added prototypes (postdct.h)
 *
 * Revision 1.12  1995/05/02  21:44:07  smoot
 * added tuneing parameters
 *
 * Revision 1.11  1995/03/31  23:50:45  smoot
 * removed block bound (moved to opts.c)
 *
 * Revision 1.10  1995/03/29  20:12:39  smoot
 * added block_bound for TUNEing
 *
 * Revision 1.9  1995/02/01  21:43:55  smoot
 * cleanup
 *
 * Revision 1.8  1995/01/19  23:52:43  smoot
 * Made computeDiffDCTs able to rule out changes to the pattern (diff too small)
 *
 * Revision 1.7  1995/01/19  23:07:17  eyhung
 * Changed copyrights
 *
 * Revision 1.6  1994/11/12  02:11:44  keving
 * nothing
 *
 * Revision 1.5  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.5  1993/12/22  19:19:01  keving
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
 * Revision 1.1  1993/04/08  21:31:59  keving
 * nothing
 *
 */


/*==============*
 * HEADER FILES *
 *==============*/

#include "all.h"
#include "mtypes.h"
#include "frames.h"
#include "bitio.h"
#include "prototypes.h"
#include "fsize.h"
#include "opts.h"
#include "postdct.h"

#undef ABS
#define ABS(x)    ((x < 0) ? (-x) : x)

#define TRUNCATE_UINT8(x)    ((x < 0) ? 0 : ((x > 255) ? 255 : x))

/*==================*
 * GLOBAL VARIABLES *
 *==================*/


extern Block **dct, **dctb, **dctr;

/*===========================*
 * COMPUTE DCT OF DIFFERENCE *
 *===========================*/

/*===========================================================================*
 *
 * ComputeDiffDCTBlock
 *
 *    compute current-motionBlock, take the DCT, and put the difference
 *    back into current
 *
 * RETURNS:    current block modified
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
boolean
ComputeDiffDCTBlock(current, dest, motionBlock)
    Block current, dest, motionBlock;
{
    register int x, y, diff = 0;

    for ( y = 0; y < 8; y++ ) {
    for ( x = 0; x < 8; x++ ) {
      current[y][x] -= motionBlock[y][x];
      diff += ABS(current[y][x]);
    }
    }
    /* Kill the block if change is too small     */
    /* (block_bound defaults to 128, see opts.c) */
    if (diff < block_bound) return FALSE;

    mp_fwd_dct_block2(current, dest);

    return TRUE;
}

/*===========================================================================*
 *
 * ComputeDiffDCTs
 *
 *    appropriate (according to pattern, the coded block pattern) blocks
 *    of 'current' are diff'ed and DCT'd.
 *
 * RETURNS:    current blocks modified
 *
 * SIDE EFFECTS:    Can remove too-small difference blocks from pattern
 *
 * PRECONDITIONS:    appropriate blocks of 'current' have not yet been
 *            modified
 *
 *===========================================================================*/
void
ComputeDiffDCTs(current, prev, by, bx, my, mx, pattern)
    MpegFrame *current;
    MpegFrame *prev;
    int by;
    int bx;
    int my;
    int mx;
    int *pattern;
{
    Block   motionBlock;

    if (collect_quant && (collect_quant_detailed & 1)) fprintf(collect_quant_fp, "l\n");
    if ( *pattern & 0x20 ) {
    ComputeMotionBlock(prev->ref_y, by, bx, my, mx, motionBlock);
    if (!ComputeDiffDCTBlock(current->y_blocks[by][bx], dct[by][bx], motionBlock))
      *pattern^=0x20;
    }

    if ( *pattern & 0x10 ) {
    ComputeMotionBlock(prev->ref_y, by, bx+1, my, mx, motionBlock);
    if (!ComputeDiffDCTBlock(current->y_blocks[by][bx+1], dct[by][bx+1], motionBlock))
      *pattern^=0x10;
    }

    if ( *pattern & 0x8 ) {
    ComputeMotionBlock(prev->ref_y, by+1, bx, my, mx, motionBlock);
    if (!ComputeDiffDCTBlock(current->y_blocks[by+1][bx], dct[by+1][bx], motionBlock))
      *pattern^=0x8;
    }

    if ( *pattern & 0x4 ) {
    ComputeMotionBlock(prev->ref_y, by+1, bx+1, my, mx, motionBlock);
    if (!ComputeDiffDCTBlock(current->y_blocks[by+1][bx+1], dct[by+1][bx+1], motionBlock))
      *pattern^=0x4;
    }

    if (collect_quant && (collect_quant_detailed & 1)) fprintf(collect_quant_fp, "c\n");
    if ( *pattern & 0x2 ) {
    ComputeMotionBlock(prev->ref_cb, by >> 1, bx >> 1, my/2, mx/2, motionBlock);
    if (!ComputeDiffDCTBlock(current->cb_blocks[by >> 1][bx >> 1], dctb[by >> 1][bx >> 1], motionBlock))
      *pattern^=0x2;
    }

    if ( *pattern & 0x1 ) {
    ComputeMotionBlock(prev->ref_cr, by >> 1, bx >> 1, my/2, mx/2, motionBlock);
    if (!ComputeDiffDCTBlock(current->cr_blocks[by >> 1][bx >> 1], dctr[by >> 1][bx >> 1], motionBlock))
      *pattern^=0x1;
    }
}


    /*======================*
     * COMPUTE MOTION BLOCK *
     *======================*/

/*===========================================================================*
 *
 * ComputeMotionBlock
 *
 *    compute the motion-compensated block
 *
 * RETURNS:    motionBlock
 *
 * SIDE EFFECTS:    none
 *
 * PRECONDITIONS:    motion vector MUST be valid
 *
 * NOTE:  could try to speed this up using halfX, halfY, halfBoth,
 *      but then would have to compute for chrominance, and it's just
 *      not worth the trouble (this procedure is not called relatively
 *      often -- a constant number of times per macroblock)
 *
 *===========================================================================*/
void
ComputeMotionBlock(prev, by, bx, my, mx, motionBlock)
    uint8_mpeg_t **prev;
    int by;    
    int bx;
    int my;
    int mx;
    Block motionBlock;
{
    register int   fy, fx;
    register int   y;
    register int16_mpeg_t *destPtr;
    register uint8_mpeg_t *srcPtr;
    register uint8_mpeg_t *srcPtr2;
    boolean xHalf, yHalf;

    xHalf = (ABS(mx) % 2 == 1);
    yHalf = (ABS(my) % 2 == 1);

    MOTION_TO_FRAME_COORD(by, bx, (my/2), (mx/2), fy, fx);

    if ( xHalf && yHalf ) {
    /* really should be fy+y-1 and fy+y so do (fy-1)+y = fy+y-1 and
       (fy-1)+y+1 = fy+y
     */
    if ( my < 0 ) {
        fy--;
    }
    if ( mx < 0 ) {
        fx--;
    }

    for ( y = 0; y < 8; y++ ) {
        destPtr = motionBlock[y];
        srcPtr = &(prev[fy+y][fx]);
        srcPtr2 = &(prev[fy+y+1][fx]);

        destPtr[0] = (srcPtr[0]+srcPtr[1]+srcPtr2[0]+srcPtr2[1]+2)>>2;
        destPtr[1] = (srcPtr[1]+srcPtr[2]+srcPtr2[1]+srcPtr2[2]+2)>>2;
        destPtr[2] = (srcPtr[2]+srcPtr[3]+srcPtr2[2]+srcPtr2[3]+2)>>2;
        destPtr[3] = (srcPtr[3]+srcPtr[4]+srcPtr2[3]+srcPtr2[4]+2)>>2;
        destPtr[4] = (srcPtr[4]+srcPtr[5]+srcPtr2[4]+srcPtr2[5]+2)>>2;
        destPtr[5] = (srcPtr[5]+srcPtr[6]+srcPtr2[5]+srcPtr2[6]+2)>>2;
        destPtr[6] = (srcPtr[6]+srcPtr[7]+srcPtr2[6]+srcPtr2[7]+2)>>2;
        destPtr[7] = (srcPtr[7]+srcPtr[8]+srcPtr2[7]+srcPtr2[8]+2)>>2;
    }
    } else if ( xHalf ) {
    if ( mx < 0 ) {
        fx--;
    }

    for ( y = 0; y < 8; y++ ) {
        destPtr = motionBlock[y];
        srcPtr = &(prev[fy+y][fx]);

        destPtr[0] = (srcPtr[0]+srcPtr[1]+1)>>1;
        destPtr[1] = (srcPtr[1]+srcPtr[2]+1)>>1;
        destPtr[2] = (srcPtr[2]+srcPtr[3]+1)>>1;
        destPtr[3] = (srcPtr[3]+srcPtr[4]+1)>>1;
        destPtr[4] = (srcPtr[4]+srcPtr[5]+1)>>1;
        destPtr[5] = (srcPtr[5]+srcPtr[6]+1)>>1;
        destPtr[6] = (srcPtr[6]+srcPtr[7]+1)>>1;
        destPtr[7] = (srcPtr[7]+srcPtr[8]+1)>>1;
    }
    } else if ( yHalf ) {
    if ( my < 0 ) {
        fy--;
    }

    for ( y = 0; y < 8; y++ ) {
        destPtr = motionBlock[y];
        srcPtr = &(prev[fy+y][fx]);
        srcPtr2 = &(prev[fy+y+1][fx]);

        destPtr[0] = (srcPtr[0]+srcPtr2[0]+1)>>1;
        destPtr[1] = (srcPtr[1]+srcPtr2[1]+1)>>1;
        destPtr[2] = (srcPtr[2]+srcPtr2[2]+1)>>1;
        destPtr[3] = (srcPtr[3]+srcPtr2[3]+1)>>1;
        destPtr[4] = (srcPtr[4]+srcPtr2[4]+1)>>1;
        destPtr[5] = (srcPtr[5]+srcPtr2[5]+1)>>1;
        destPtr[6] = (srcPtr[6]+srcPtr2[6]+1)>>1;
        destPtr[7] = (srcPtr[7]+srcPtr2[7]+1)>>1;
    }
    } else {
    for ( y = 0; y < 8; y++ ) {
        destPtr = motionBlock[y];
        srcPtr = &(prev[fy+y][fx]);

        destPtr[0] = (uint8_mpeg_t) srcPtr[0];
        destPtr[1] = (uint8_mpeg_t) srcPtr[1];
        destPtr[2] = (uint8_mpeg_t) srcPtr[2];
        destPtr[3] = (uint8_mpeg_t) srcPtr[3];
        destPtr[4] = (uint8_mpeg_t) srcPtr[4];
        destPtr[5] = (uint8_mpeg_t) srcPtr[5];
        destPtr[6] = (uint8_mpeg_t) srcPtr[6];
        destPtr[7] = (uint8_mpeg_t) srcPtr[7];
    }
    }
}


/*===========================================================================*
 *
 * ComputeMotionLumBlock
 *
 *    compute the motion-compensated luminance block
 *
 * RETURNS:    motionBlock
 *
 * SIDE EFFECTS:    none
 *
 * PRECONDITIONS:    motion vector MUST be valid
 *
 * NOTE:  see ComputeMotionBlock
 *
 *===========================================================================*/
void
ComputeMotionLumBlock(prevFrame, by, bx, my, mx, motionBlock)
    MpegFrame *prevFrame;
    int by;
    int bx;
    int my;
    int mx;
    LumBlock motionBlock;
{
    register uint8_mpeg_t *across;
    register int32_mpeg_t *macross;
    register int y;
    uint8_mpeg_t **prev;
    int        fy, fx;
    boolean xHalf, yHalf;

    xHalf = (ABS(mx) % 2 == 1);
    yHalf = (ABS(my) % 2 == 1);

    MOTION_TO_FRAME_COORD(by, bx, my/2, mx/2, fy, fx);

    if ( xHalf ) {
    if ( mx < 0 ) {
        fx--;
    }

    if ( yHalf ) {
        if ( my < 0 ) {
        fy--;
        }
        
        prev = prevFrame->halfBoth;
    } else {
        prev = prevFrame->halfX;
    }
    } else if ( yHalf ) {
    if ( my < 0 ) {
        fy--;
    }

    prev = prevFrame->halfY;
    } else {
    prev = prevFrame->ref_y;
    }

    for ( y = 0; y < 16; y++ ) {
    across = &(prev[fy+y][fx]);
    macross = motionBlock[y];

    macross[0] = across[0];
    macross[1] = across[1];
    macross[2] = across[2];
    macross[3] = across[3];
    macross[4] = across[4];
    macross[5] = across[5];
    macross[6] = across[6];
    macross[7] = across[7];
    macross[8] = across[8];
    macross[9] = across[9];
    macross[10] = across[10];
    macross[11] = across[11];
    macross[12] = across[12];
    macross[13]= across[13];
    macross[14] = across[14];
    macross[15] = across[15];
    }

    /* this is what's really happening, in slow motion:
     *
     *    for ( y = 0; y < 16; y++, py++ )
     *      for ( x = 0; x < 16; x++, px++ )
     *        motionBlock[y][x] = prev[fy+y][fx+x];
     *
     */
}


/*=======================*
 * BASIC ERROR FUNCTIONS *
 *=======================*/


/*===========================================================================*
 *
 * LumBlockMAD
 *
 *    return the MAD of two luminance blocks
 *
 * RETURNS:    the MAD, if less than bestSoFar, or
 *        some number bigger if not
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
int32_mpeg_t
LumBlockMAD(currentBlock, motionBlock, bestSoFar)
    LumBlock currentBlock;
    LumBlock motionBlock;
    int32_mpeg_t bestSoFar;
{
    register int32_mpeg_t   diff = 0;    /* max value of diff is 255*256 = 65280 */
    register int32_mpeg_t localDiff;
    register int y, x;

    for ( y = 0; y < 16; y++ ) {
    for ( x = 0; x < 16; x++ ) {
        localDiff = currentBlock[y][x] - motionBlock[y][x];
        diff += ABS(localDiff);
    }

    if ( diff > bestSoFar ) {
        return diff;
    }
    }

    return (int32_mpeg_t)diff;
}


/*===========================================================================*
 *
 * LumMotionError
 *
 *    return the MAD of the currentBlock and the motion-compensated block
 *      (without TUNEing)
 *
 * RETURNS:    the MAD, if less than bestSoFar, or
 *        some number bigger if not
 *
 * SIDE EFFECTS:    none
 *
 * PRECONDITIONS:  motion vector MUST be valid
 *
 * NOTES:  this is the procedure that is called the most, and should therefore
 *         be the most optimized!!!
 *
 *===========================================================================*/
int32_mpeg_t
LumMotionError(currentBlock, prevFrame, by, bx, my, mx, bestSoFar)
    LumBlock currentBlock;
    MpegFrame *prevFrame;
    int by;
    int bx;
    int my;    
    int mx;        
    int32_mpeg_t bestSoFar;
{
    register int32_mpeg_t adiff = 0,  diff = 0;    /* max value of diff is 255*256 = 65280 */
    register int32_mpeg_t localDiff;
    register uint8_mpeg_t *across;
    register int32_mpeg_t *cacross;
    register int y;
    uint8_mpeg_t **prev;
    int        fy, fx;
    boolean xHalf, yHalf;

    xHalf = (ABS(mx) % 2 == 1);
    yHalf = (ABS(my) % 2 == 1);

    MOTION_TO_FRAME_COORD(by, bx, my/2, mx/2, fy, fx);

    if ( xHalf ) {
    if ( mx < 0 ) {
        fx--;
    }

    if ( yHalf ) {
        if ( my < 0 ) {
        fy--;
        }
        
        prev = prevFrame->halfBoth;
    } else {
        prev = prevFrame->halfX;
    }
    } else if ( yHalf ) {
    if ( my < 0 ) {
        fy--;
    }

    prev = prevFrame->halfY;
    } else {
    prev = prevFrame->ref_y;
    }

    switch (SearchCompareMode) {
    case DEFAULT_SEARCH: /* Default. */
      /* this is what's happening:
       *    ComputeMotionLumBlock(prevFrame, by, bx, my, mx, lumMotionBlock);
       *    for ( y = 0; y < 16; y++ )
       *        for ( x = 0; x < 16; x++ )
       *        {
       *        localDiff = currentBlock[y][x] - lumMotionBlock[y][x];
       *        diff += ABS(localDiff);
       *        }
       */
      for ( y = 0; y < 16; y++ ) {
    across = &(prev[fy+y][fx]);
    cacross = currentBlock[y];
    
    localDiff = across[0]-cacross[0];     diff += ABS(localDiff);
    localDiff = across[1]-cacross[1];     diff += ABS(localDiff);
    localDiff = across[2]-cacross[2];     diff += ABS(localDiff);
    localDiff = across[3]-cacross[3];     diff += ABS(localDiff);
    localDiff = across[4]-cacross[4];     diff += ABS(localDiff);
    localDiff = across[5]-cacross[5];     diff += ABS(localDiff);
    localDiff = across[6]-cacross[6];     diff += ABS(localDiff);
    localDiff = across[7]-cacross[7];     diff += ABS(localDiff);
    localDiff = across[8]-cacross[8];     diff += ABS(localDiff);
    localDiff = across[9]-cacross[9];     diff += ABS(localDiff);
    localDiff = across[10]-cacross[10];     diff += ABS(localDiff);
    localDiff = across[11]-cacross[11];     diff += ABS(localDiff);
    localDiff = across[12]-cacross[12];     diff += ABS(localDiff);
    localDiff = across[13]-cacross[13];     diff += ABS(localDiff);
    localDiff = across[14]-cacross[14];     diff += ABS(localDiff);
    localDiff = across[15]-cacross[15];     diff += ABS(localDiff);
    
    if ( diff > bestSoFar ) {
      return diff;
    }
      }
      break;
      
    case LOCAL_DCT: {
      Block     dctdiff[4], dctquant[4];
      FlatBlock quant;
      int x, i, tmp;
      int distortion=0, datarate=0;
      int pq = GetPQScale();
      
      for (y = 0;  y < 16;  y++) {
    across = &(prev[fy+y][fx]);
    cacross = currentBlock[y];
    for (x = 0;  x < 16;  x++) {
      dctdiff[(x>7)+2*(y>7)][y%8][x%8] = cacross[x]-across[x];
    }}

      /* Calculate rate */
      for (i = 0;  i < 4;  i++) {
    mp_fwd_dct_block2(dctdiff[i], dctdiff[i]);
    if (Mpost_QuantZigBlock(dctdiff[i], quant, pq, FALSE) == MPOST_ZERO) {
      /* no sense in continuing */
      memset((char *)dctquant[i], 0, sizeof(Block));
    } else {
      Mpost_UnQuantZigBlock(quant, dctquant[i], pq, FALSE);
      mpeg_jrevdct((int16_mpeg_t *)dctquant[i]);
      datarate += CalcRLEHuffLength(quant);
    }
      }
      
      /* Calculate distortion */
      for (y = 0;  y < 16;  y++) {
    across = &(prev[fy+y][fx]);
    cacross = currentBlock[y];
    for (x = 0;  x < 16;  x++) {
      tmp = across[x] - cacross[x] + dctquant[(x>7)+2*(y>7)][y%8][x%8];
      distortion += tmp*tmp;
    }}
      distortion /= 256;
      distortion *= LocalDCTDistortScale;
      datarate *= LocalDCTRateScale;
      diff = (int) sqrt(distortion*distortion + datarate*datarate);
      break;
    }

    case NO_DC_SEARCH: {
      extern int32_mpeg_t niqtable[];
      int pq = niqtable[0]*GetPQScale();
      
      for ( y = 0; y < 16; y++ ) {
    across = &(prev[fy+y][fx]);
    cacross = currentBlock[y];

    localDiff = across[0]-cacross[0];  diff += localDiff; adiff += ABS(localDiff);
    localDiff = across[1]-cacross[1];  diff += localDiff; adiff += ABS(localDiff);
    localDiff = across[2]-cacross[2];  diff += localDiff; adiff += ABS(localDiff);
    localDiff = across[3]-cacross[3];  diff += localDiff; adiff += ABS(localDiff);
    localDiff = across[4]-cacross[4];  diff += localDiff; adiff += ABS(localDiff);
    localDiff = across[5]-cacross[5];  diff += localDiff; adiff += ABS(localDiff);
    localDiff = across[6]-cacross[6];  diff += localDiff; adiff += ABS(localDiff);
    localDiff = across[7]-cacross[7];  diff += localDiff; adiff += ABS(localDiff);
    localDiff = across[8]-cacross[8];  diff += localDiff; adiff += ABS(localDiff);
    localDiff = across[9]-cacross[9];  diff += localDiff; adiff += ABS(localDiff);
    localDiff = across[10]-cacross[10];  diff += localDiff; adiff += ABS(localDiff);
    localDiff = across[11]-cacross[11];  diff += localDiff; adiff += ABS(localDiff);
    localDiff = across[12]-cacross[12];  diff += localDiff; adiff += ABS(localDiff);
    localDiff = across[13]-cacross[13];  diff += localDiff; adiff += ABS(localDiff);
    localDiff = across[14]-cacross[14];  diff += localDiff; adiff += ABS(localDiff);
    localDiff = across[15]-cacross[15];  diff += localDiff; adiff += ABS(localDiff);

      }

      diff /= 64*pq;  /* diff is now the DC difference (with QSCALE 1) */
      adiff -= 64*pq*ABS(diff);
      diff = adiff;
    }
      break;

    case DO_Mean_Squared_Distortion:
      for ( y = 0; y < 16; y++ ) {
    across = &(prev[fy+y][fx]);
    cacross = currentBlock[y];

    localDiff = across[0]-cacross[0];     diff += localDiff*localDiff;
    localDiff = across[1]-cacross[1];     diff += localDiff*localDiff;
    localDiff = across[2]-cacross[2];     diff += localDiff*localDiff;
    localDiff = across[3]-cacross[3];     diff += localDiff*localDiff;
    localDiff = across[4]-cacross[4];     diff += localDiff*localDiff;
    localDiff = across[5]-cacross[5];     diff += localDiff*localDiff;
    localDiff = across[6]-cacross[6];     diff += localDiff*localDiff;
    localDiff = across[7]-cacross[7];     diff += localDiff*localDiff;
    localDiff = across[8]-cacross[8];     diff += localDiff*localDiff;
    localDiff = across[9]-cacross[9];     diff += localDiff*localDiff;
    localDiff = across[10]-cacross[10];     diff += localDiff*localDiff;
    localDiff = across[11]-cacross[11];     diff += localDiff*localDiff;
    localDiff = across[12]-cacross[12];     diff += localDiff*localDiff;
    localDiff = across[13]-cacross[13];     diff += localDiff*localDiff;
    localDiff = across[14]-cacross[14];     diff += localDiff*localDiff;
    localDiff = across[15]-cacross[15];     diff += localDiff*localDiff;

    if ( diff > bestSoFar ) {
      return diff;
    }
      }
      break;
    } /* End of Switch */

    return diff;
}


/*===========================================================================*
 *
 * LumAddMotionError
 *
 *    return the MAD of the currentBlock and the average of the blockSoFar
 *    and the motion-compensated block (this is used for B-frame searches)
 *
 * RETURNS:    the MAD, if less than bestSoFar, or
 *        some number bigger if not
 *
 * SIDE EFFECTS:    none
 *
 * PRECONDITIONS:  motion vector MUST be valid
 *
 *===========================================================================*/
int32_mpeg_t
LumAddMotionError(currentBlock, blockSoFar, prevFrame, by, bx, my, mx,
          bestSoFar)
    LumBlock currentBlock;
    LumBlock blockSoFar;
    MpegFrame *prevFrame;
    int by;
    int bx;
    int my;
    int mx;
    int32_mpeg_t bestSoFar;
{
    register int32_mpeg_t   diff = 0;    /* max value of diff is 255*256 = 65280 */
    register int32_mpeg_t localDiff;
    register uint8_mpeg_t *across;
    register int32_mpeg_t *bacross;
    register int32_mpeg_t *cacross;
    register int y;
    uint8_mpeg_t **prev;
    int        fy, fx;
    boolean xHalf, yHalf;

    xHalf = (ABS(mx) % 2 == 1);
    yHalf = (ABS(my) % 2 == 1);

    MOTION_TO_FRAME_COORD(by, bx, my/2, mx/2, fy, fx);

    if ( xHalf ) {
    if ( mx < 0 ) {
        fx--;
    }

    if ( yHalf ) {
        if ( my < 0 ) {
        fy--;
        }
        
        prev = prevFrame->halfBoth;
    } else {
        prev = prevFrame->halfX;
    }
    } else if ( yHalf ) {
    if ( my < 0 ) {
        fy--;
    }

    prev = prevFrame->halfY;
    } else {
    prev = prevFrame->ref_y;
    }

/* do we add 1 before dividing by two?  Yes -- see MPEG-1 doc page 46 */

#define ADD_ADD_DIFF(d,l,a,b,c,i)       \
    l = ((a[i]+b[i]+1)>>1)-c[i];        \
    d += ABS(l)

    for ( y = 0; y < 16; y++ ) {
    across = &(prev[fy+y][fx]);
    bacross = blockSoFar[y];
    cacross = currentBlock[y];

    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,0);
    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,1);
    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,2);
    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,3);
    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,4);
    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,5);
    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,6);
    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,7);
    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,8);
    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,9);
    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,10);
    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,11);
    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,12);
    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,13);
    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,14);
    ADD_ADD_DIFF(diff,localDiff,across,bacross,cacross,15);

    if ( diff > bestSoFar ) {
        return diff;
    }
    }

    /* this is what's happening:
     *
     *    ComputeMotionLumBlock(prevFrame, by, bx, my, mx, lumMotionBlock);
     *
     *    for ( y = 0; y < 16; y++ )
     *        for ( x = 0; x < 16; x++ )
     *        {
     *        localDiff = currentBlock[y][x] - lumMotionBlock[y][x];
     *        diff += ABS(localDiff);
     *        }
     *
     */

    return diff;
}


/*===========================================================================*
 *
 * AddMotionBlock
 *
 *    adds the motion-compensated block to the given block
 *
 * RETURNS:    block modified
 *
 * SIDE EFFECTS:    none
 *
 * PRECONDITIONS:  motion vector MUST be valid
 *
 *===========================================================================*/
void
AddMotionBlock(block, prev, by, bx, my, mx)
    Block block;
    uint8_mpeg_t **prev;
    int by;
    int bx;
    int my;
    int mx;
{
    int        fy, fx;
    int        x, y;
    boolean xHalf, yHalf;

    xHalf = (ABS(mx) % 2 == 1);
    yHalf = (ABS(my) % 2 == 1);

    MOTION_TO_FRAME_COORD(by, bx, (my/2), (mx/2), fy, fx);

    if ( xHalf && yHalf ) {
    /* really should be fy+y-1 and fy+y so do (fy-1)+y = fy+y-1 and
       (fy-1)+y+1 = fy+y
     */
    if ( my < 0 ) {
        fy--;
    }
    if ( mx < 0 ) {
        fx--;
    }

    for ( y = 0; y < 8; y++ ) {
        for ( x = 0; x < 8; x++ ) {
        block[y][x] += (prev[fy+y][fx+x]+prev[fy+y][fx+x+1]+
                    prev[fy+y+1][fx+x]+prev[fy+y+1][fx+x+1]+2)>>2;
        }
    }
    } else if ( xHalf ) {
    if ( mx < 0 ) {
        fx--;
    }

    for ( y = 0; y < 8; y++ ) {
        for ( x = 0; x < 8; x++ ) {
        block[y][x] += (prev[fy+y][fx+x]+prev[fy+y][fx+x+1]+1)>>1;
        }
    }
    } else if ( yHalf ) {
    if ( my < 0 ) {
        fy--;
    }

    for ( y = 0; y < 8; y++ ) {
        for ( x = 0; x < 8; x++ ) {
        block[y][x] += (prev[fy+y][fx+x]+prev[fy+y+1][fx+x]+1)>>1;
        }
    }
    } else {
    for ( y = 0; y < 8; y++ ) {
        for ( x = 0; x < 8; x++ ) {
        block[y][x] += (int16_mpeg_t)prev[fy+y][fx+x];
        }
    }
    }
}


/*===========================================================================*
 *
 * AddBMotionBlock
 *
 *    adds the motion-compensated B-frame block to the given block
 *
 * RETURNS:    block modified
 *
 * SIDE EFFECTS:    none
 *
 * PRECONDITIONS:  motion vectors MUST be valid
 *
 *===========================================================================*/
void
AddBMotionBlock(block, prev, next, by, bx, mode, fmy, fmx, bmy, bmx)
    Block block;
    uint8_mpeg_t **prev;
    uint8_mpeg_t **next;
    int by;
    int bx;
    int    mode;
    int fmy;
    int fmx;
    int bmy;
    int bmx;
{
    int        x, y;
    Block   prevBlock, nextBlock;

    if ( mode == MOTION_FORWARD ) {
    AddMotionBlock(block, prev, by, bx, fmy, fmx);
    } else if ( mode == MOTION_BACKWARD ) {
    AddMotionBlock(block, next, by, bx, bmy, bmx);
    } else {
    ComputeMotionBlock(prev, by, bx, fmy, fmx, prevBlock);
    ComputeMotionBlock(next, by, bx, bmy, bmx, nextBlock);

    for ( y = 0; y < 8; y++ ) {
        for ( x = 0; x < 8; x++ ) {
        block[y][x] += (prevBlock[y][x]+nextBlock[y][x]+1)/2;
        }
    }
    }
}


/*===========================================================================*
 *
 * BlockToData
 *
 *    copies the given block into the appropriate data area
 *
 * RETURNS:    data modified
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
BlockToData(data, block, by, bx)
    uint8_mpeg_t **data;
    Block block;
    int by;
    int bx;
{
    register int x, y;
    register int fy, fx;
    register int16_mpeg_t    blockItem;

    BLOCK_TO_FRAME_COORD(by, bx, fy, fx);

    for ( y = 0; y < 8; y++ ) {
    for ( x = 0; x < 8; x++ ) {
        blockItem = block[y][x];
        data[fy+y][fx+x] = TRUNCATE_UINT8(blockItem);
    }
    }
}


/*===========================================================================*
 *
 * BlockifyFrame
 *
 *    copies data into appropriate blocks
 *
 * RETURNS:    mf modified
 *
 * SIDE EFFECTS:    none
 *
 * NOTES:  probably shouldn't be in this file
 *
 *===========================================================================*/
void
BlockifyFrame(framePtr)
    MpegFrame *framePtr;
{
    register int dctx, dcty;
    register int x, y;
    register int bx, by;
    register int fy, fx;
    register int16_mpeg_t  *destPtr;
    register uint8_mpeg_t  *srcPtr;
    register int16_mpeg_t  *destPtr2;
    register uint8_mpeg_t  *srcPtr2;
    Block   *blockPtr;
    Block   *blockPtr2;

    dctx = Fsize_x / DCTSIZE;
    dcty = Fsize_y / DCTSIZE;

    /*
     * copy y data into y_blocks
     */
    for (by = 0; by < dcty; by++) {
    fy = by*DCTSIZE;
    for (bx = 0; bx < dctx; bx++) {
        fx = bx*DCTSIZE;
        blockPtr = (Block *) &(framePtr->y_blocks[by][bx][0][0]);
        for (y = 0; y < DCTSIZE; y++) {
        destPtr = &((*blockPtr)[y][0]);
        srcPtr = &(framePtr->orig_y[fy+y][fx]);
        for (x = 0; x < DCTSIZE; x++) {
            destPtr[x] = srcPtr[x];
        }
        }
    }
    }

    /*
     * copy cr/cb data into cr/cb_blocks
     */
    for (by = 0; by < (dcty >> 1); by++) {
    fy = by*DCTSIZE;
    for (bx = 0; bx < (dctx >> 1); bx++) {
        fx = bx*DCTSIZE;
        blockPtr = (Block *) &(framePtr->cr_blocks[by][bx][0][0]);
        blockPtr2 = (Block *) &(framePtr->cb_blocks[by][bx][0][0]);
        for (y = 0; y < DCTSIZE; y++) {
        destPtr = &((*blockPtr)[y][0]);
        srcPtr = &(framePtr->orig_cr[fy+y][fx]);
        destPtr2 = &((*blockPtr2)[y][0]);
        srcPtr2 = &(framePtr->orig_cb[fy+y][fx]);
        for (x = 0; x < DCTSIZE; x++) {
            destPtr[x] = srcPtr[x];
            destPtr2[x] = srcPtr2[x];
        }
        }
    }
    }
}


/*===========================================================================*
 *                                         *
 * UNUSED PROCEDURES                                 *
 *                                         *
 *    The following procedures are all unused by the encoder             *
 *                                         *
 *    They are listed here for your convenience.  You might want to use    *
 *    them if you experiment with different search techniques             *
 *                                         *
 *===========================================================================*/

#ifdef UNUSED_PROCEDURES

/* this procedure calculates the subsampled motion block (obviously)
 *
 * for speed, this procedure is probably not called anywhere (it is
 * incorporated directly into LumDiffA, LumDiffB, etc.
 *
 * but leave it here anyway for clarity
 *
 * (startY, startX) = (0,0) for A....(0,1) for B...(1,0) for C...(1,1) for D
 *  
 */
void
ComputeSubSampledMotionLumBlock(prevFrame, by, bx, my, mx, motionBlock,
                startY, startX)
    MpegFrame *prevFrame;
    int by;
    int bx;
    int my;
    int mx;
    LumBlock motionBlock;
    int startY;
    int startX;
{
    register uint8_mpeg_t *across;
    register int32_mpeg_t *macross;
    register int32_mpeg_t *lastx;
    register int y;
    uint8_mpeg_t **prev;
    int    fy, fx;
    boolean xHalf, yHalf;

    xHalf = (ABS(mx) % 2 == 1);
    yHalf = (ABS(my) % 2 == 1);

    MOTION_TO_FRAME_COORD(by, bx, my/2, mx/2, fy, fx);

    if ( xHalf ) {
    if ( mx < 0 ) {
        fx--;
    }

    if ( yHalf ) {
        if ( my < 0 ) {
        fy--;
        }
        
        prev = prevFrame->halfBoth;
    } else {
        prev = prevFrame->halfX;
    }
    } else if ( yHalf ) {
    if ( my < 0 ) {
        fy--;
    }

    prev = prevFrame->halfY;
    } else {
    prev = prevFrame->ref_y;
    }

    for ( y = startY; y < 16; y += 2 ) {
    across = &(prev[fy+y][fx+startX]);
    macross = &(motionBlock[y][startX]);
    lastx = &(motionBlock[y][16]);
    while ( macross < lastx ) {
        (*macross) = (*across);
        across += 2;
        macross += 2;
    }
    }

    /* this is what's really going on in slow motion:
     *
     *    for ( y = startY; y < 16; y += 2 )
     *        for ( x = startX; x < 16; x += 2 )
     *        motionBlock[y][x] = prev[fy+y][fx+x];
     *
     */
}


/*===========================================================================*
 *
 * LumMotionErrorSubSampled
 *
 *    return the MAD of the currentBlock and the motion-compensated block,
 *    subsampled 4:1 with given starting coordinates (startY, startX)
 *
 * RETURNS:    the MAD
 *
 * SIDE EFFECTS:    none
 *
 * PRECONDITIONS:  motion vector MUST be valid
 *
 * NOTES:  this procedure is never called.  Instead, see subsample.c.  This
 *         procedure is provided only for possible use in extensions
 *
 *===========================================================================*/
int32_mpeg_t
LumMotionErrorSubSampled(currentBlock, prevFrame, by, bx, my, mx, startY,
             startX)
    LumBlock currentBlock;
    MpegFrame *prevFrame;
    int by;
    int bx;
    int my;
    int mx;
    int startY;
    int startX;
{
    register int32_mpeg_t    diff = 0;        /* max value of diff is 255*256 = 65280 */
    register int32_mpeg_t localDiff;
    register int32_mpeg_t *cacross;
    register uint8_mpeg_t *macross;
    register int32_mpeg_t *lastx;
    register int y;
    uint8_mpeg_t **prev;
    int    fy, fx;
    boolean xHalf, yHalf;

    xHalf = (ABS(mx) % 2 == 1);
    yHalf = (ABS(my) % 2 == 1);

    MOTION_TO_FRAME_COORD(by, bx, my/2, mx/2, fy, fx);

    if ( xHalf ) {
    if ( mx < 0 ) {
        fx--;
    }

    if ( yHalf ) {
        if ( my < 0 ) {
        fy--;
        }
        
        prev = prevFrame->halfBoth;
    } else {
        prev = prevFrame->halfX;
    }
    } else if ( yHalf ) {
    if ( my < 0 ) {
        fy--;
    }

    prev = prevFrame->halfY;
    } else {
    prev = prevFrame->ref_y;
    }

    for ( y = startY; y < 16; y += 2 ) {
    macross = &(prev[fy+y][fx+startX]);
    cacross = &(currentBlock[y][startX]);
    lastx = &(currentBlock[y][16]);
    while ( cacross < lastx ) {
        localDiff = (*cacross)-(*macross);
        diff += ABS(localDiff);
        macross += 2;
        cacross += 2;
    }
    }

    /* this is what's really happening:
     *
     *    ComputeSubSampledMotionLumBlock(prevFrame, by, bx, my, mx,
     *                    lumMotionBlock, startY, startX);
     *
     *    for ( y = startY; y < 16; y += 2 )
     *        for ( x = startX; x < 16; x += 2 )
     *        {
     *             localDiff = currentBlock[y][x] - lumMotionBlock[y][x];
     *        diff += ABS(localDiff);
     *        }
     *
     */

    return (int32_mpeg_t)diff;
}


#endif /* UNUSED_PROCEDURES */
