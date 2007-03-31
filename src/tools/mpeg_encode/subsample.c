/*===========================================================================*
 * subsample.c                                     *
 *                                         *
 *    Procedures concerned with subsampling                     *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    LumMotionErrorA                                 *
 *    LumMotionErrorB                                 *
 *    LumMotionErrorC                                 *
 *    LumMotionErrorD                                 *
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
 *  $Header: /n/charlie-brown/project/mm/mpeg/mpeg_dist/mpeg_encode/RCS/subsample.c,v 1.6 1995/01/19 23:09:28 eyhung Exp $
 *  $Log: subsample.c,v $
 * Revision 1.6  1995/01/19  23:09:28  eyhung
 * Changed copyrights
 *
 * Revision 1.5  1994/11/12  02:12:01  keving
 * nothing
 *
 * Revision 1.4  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.4  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.3  1993/07/22  22:23:43  keving
 * nothing
 *
 * Revision 1.2  1993/06/30  20:06:09  keving
 * nothing
 *
 * Revision 1.1  1993/06/22  21:56:05  keving
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

#undef ABS
#define ABS(x)    ((x < 0) ? (-x) : x)


/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/


/*===========================================================================*
 *
 * LumMotionErrorA
 *
 *    compute the motion error for the A subsampling pattern
 *
 * RETURNS:    the error, or some number greater if it is worse
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
int32_mpeg_t
LumMotionErrorA(currentBlock, prevFrame, by, bx, my, mx, bestSoFar)
    LumBlock currentBlock;
    MpegFrame *prevFrame;
    int by;
    int bx;
    int my;
    int mx;
    int32_mpeg_t bestSoFar;
{
    register int32_mpeg_t    diff = 0;        /* max value of diff is 255*256 = 65280 */
    register int32_mpeg_t localDiff;
    register uint8_mpeg_t *macross;
    register uint8_mpeg_t **prev;
    register int    fy, fx;
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

    macross = &(prev[fy][fx]);

    localDiff = macross[0]-currentBlock[0][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[0][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[0][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[0][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[0][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[0][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[0][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[0][14];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+2][fx]);

    localDiff = macross[0]-currentBlock[2][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[2][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[2][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[2][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[2][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[2][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[2][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[2][14];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+4][fx]);

    localDiff = macross[0]-currentBlock[4][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[4][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[4][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[4][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[4][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[4][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[4][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[4][14];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+6][fx]);

    localDiff = macross[0]-currentBlock[6][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[6][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[6][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[6][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[6][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[6][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[6][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[6][14];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+8][fx]);

    localDiff = macross[0]-currentBlock[8][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[8][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[8][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[8][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[8][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[8][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[8][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[8][14];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+10][fx]);

    localDiff = macross[0]-currentBlock[10][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[10][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[10][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[10][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[10][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[10][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[10][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[10][14];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+12][fx]);

    localDiff = macross[0]-currentBlock[12][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[12][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[12][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[12][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[12][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[12][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[12][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[12][14];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+14][fx]);

    localDiff = macross[0]-currentBlock[14][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[14][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[14][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[14][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[14][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[14][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[14][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[14][14];     diff += ABS(localDiff);

    return diff;
}


/*===========================================================================*
 *
 * LumMotionErrorB
 *
 *    compute the motion error for the B subsampling pattern
 *
 * RETURNS:    the error, or some number greater if it is worse
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
int32_mpeg_t
LumMotionErrorB(currentBlock, prevFrame, by, bx, my, mx, bestSoFar)
    LumBlock currentBlock;
    MpegFrame *prevFrame;
    int by;
    int bx;
    int my;
    int mx;
    int32_mpeg_t bestSoFar;
{
    register int32_mpeg_t    diff = 0;        /* max value of diff is 255*256 = 65280 */
    register int32_mpeg_t localDiff;
    register uint8_mpeg_t *macross;
    register uint8_mpeg_t **prev;
    register int    fy, fx;
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

    macross = &(prev[fy+0][fx]);

    localDiff = macross[1]-currentBlock[0][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[0][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[0][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[0][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[0][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[0][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[0][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[0][15];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+2][fx]);

    localDiff = macross[1]-currentBlock[2][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[2][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[2][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[2][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[2][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[2][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[2][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[2][15];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+4][fx]);

    localDiff = macross[1]-currentBlock[4][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[4][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[4][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[4][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[4][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[4][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[4][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[4][15];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+6][fx]);

    localDiff = macross[1]-currentBlock[6][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[6][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[6][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[6][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[6][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[6][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[6][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[6][15];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+8][fx]);

    localDiff = macross[1]-currentBlock[8][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[8][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[8][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[8][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[8][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[8][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[8][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[8][15];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+10][fx]);

    localDiff = macross[1]-currentBlock[10][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[10][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[10][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[10][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[10][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[10][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[10][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[10][15];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+12][fx]);

    localDiff = macross[1]-currentBlock[12][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[12][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[12][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[12][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[12][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[12][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[12][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[12][15];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+14][fx]);

    localDiff = macross[1]-currentBlock[14][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[14][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[14][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[14][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[14][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[14][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[14][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[14][15];     diff += ABS(localDiff);

    return diff;
}


/*===========================================================================*
 *
 * LumMotionErrorC
 *
 *    compute the motion error for the C subsampling pattern
 *
 * RETURNS:    the error, or some number greater if it is worse
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
int32_mpeg_t
LumMotionErrorC(currentBlock, prevFrame, by, bx, my, mx, bestSoFar)
    LumBlock currentBlock;
    MpegFrame *prevFrame;
    int by;
    int bx;
    int my;
    int mx;
    int32_mpeg_t bestSoFar;
{
    register int32_mpeg_t    diff = 0;        /* max value of diff is 255*256 = 65280 */
    register int32_mpeg_t localDiff;
    register uint8_mpeg_t *macross;
    register uint8_mpeg_t **prev;
    register int    fy, fx;
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

    macross = &(prev[fy+1][fx]);

    localDiff = macross[0]-currentBlock[1][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[1][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[1][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[1][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[1][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[1][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[1][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[1][14];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+3][fx]);

    localDiff = macross[0]-currentBlock[3][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[3][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[3][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[3][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[3][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[3][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[3][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[3][14];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+5][fx]);

    localDiff = macross[0]-currentBlock[5][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[5][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[5][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[5][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[5][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[5][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[5][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[5][14];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+7][fx]);

    localDiff = macross[0]-currentBlock[7][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[7][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[7][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[7][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[7][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[7][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[7][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[7][14];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+9][fx]);

    localDiff = macross[0]-currentBlock[9][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[9][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[9][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[9][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[9][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[9][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[9][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[9][14];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+11][fx]);

    localDiff = macross[0]-currentBlock[11][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[11][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[11][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[11][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[11][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[11][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[11][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[11][14];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+13][fx]);

    localDiff = macross[0]-currentBlock[13][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[13][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[13][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[13][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[13][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[13][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[13][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[13][14];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+15][fx]);

    localDiff = macross[0]-currentBlock[15][0];     diff += ABS(localDiff);
    localDiff = macross[2]-currentBlock[15][2];     diff += ABS(localDiff);
    localDiff = macross[4]-currentBlock[15][4];     diff += ABS(localDiff);
    localDiff = macross[6]-currentBlock[15][6];     diff += ABS(localDiff);
    localDiff = macross[8]-currentBlock[15][8];     diff += ABS(localDiff);
    localDiff = macross[10]-currentBlock[15][10];     diff += ABS(localDiff);
    localDiff = macross[12]-currentBlock[15][12];     diff += ABS(localDiff);
    localDiff = macross[14]-currentBlock[15][14];     diff += ABS(localDiff);

    return diff;
}


/*===========================================================================*
 *
 * LumMotionErrorD
 *
 *    compute the motion error for the D subsampling pattern
 *
 * RETURNS:    the error, or some number greater if it is worse
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
int32_mpeg_t
LumMotionErrorD(currentBlock, prevFrame, by, bx, my, mx, bestSoFar)
    LumBlock currentBlock;
    MpegFrame *prevFrame;
    int by;
    int bx;
    int my;
    int mx;
    int32_mpeg_t bestSoFar;
{
    register int32_mpeg_t    diff = 0;        /* max value of diff is 255*256 = 65280 */
    register int32_mpeg_t localDiff;
    register uint8_mpeg_t *macross;
    register uint8_mpeg_t **prev;
    register int    fy, fx;
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

    macross = &(prev[fy+1][fx]);

    localDiff = macross[1]-currentBlock[1][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[1][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[1][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[1][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[1][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[1][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[1][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[1][15];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+3][fx]);

    localDiff = macross[1]-currentBlock[3][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[3][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[3][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[3][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[3][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[3][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[3][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[3][15];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+5][fx]);

    localDiff = macross[1]-currentBlock[5][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[5][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[5][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[5][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[5][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[5][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[5][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[5][15];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+7][fx]);

    localDiff = macross[1]-currentBlock[7][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[7][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[7][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[7][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[7][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[7][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[7][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[7][15];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+9][fx]);

    localDiff = macross[1]-currentBlock[9][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[9][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[9][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[9][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[9][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[9][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[9][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[9][15];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+11][fx]);

    localDiff = macross[1]-currentBlock[11][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[11][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[11][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[11][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[11][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[11][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[11][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[11][15];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+13][fx]);

    localDiff = macross[1]-currentBlock[13][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[13][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[13][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[13][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[13][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[13][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[13][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[13][15];     diff += ABS(localDiff);

    if ( diff > bestSoFar ) {
    return diff;
    }

    macross = &(prev[fy+15][fx]);

    localDiff = macross[1]-currentBlock[15][1];     diff += ABS(localDiff);
    localDiff = macross[3]-currentBlock[15][3];     diff += ABS(localDiff);
    localDiff = macross[5]-currentBlock[15][5];     diff += ABS(localDiff);
    localDiff = macross[7]-currentBlock[15][7];     diff += ABS(localDiff);
    localDiff = macross[9]-currentBlock[15][9];     diff += ABS(localDiff);
    localDiff = macross[11]-currentBlock[15][11];     diff += ABS(localDiff);
    localDiff = macross[13]-currentBlock[15][13];     diff += ABS(localDiff);
    localDiff = macross[15]-currentBlock[15][15];     diff += ABS(localDiff);

    return diff;
}
