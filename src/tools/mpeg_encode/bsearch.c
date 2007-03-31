/*===========================================================================*
 * bsearch.c                                     *
 *                                         *
 *    Procedures concerned with the B-frame motion search             *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    SetBSearchAlg                                 *
 *    BMotionSearch                                 *
 *    BSearchName                                 *
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
 *  $Header: /n/picasso/project/mpeg/mpeg_dist/mpeg_encode/RCS/bsearch.c,v 1.10 1995/08/07 21:49:01 smoot Exp $
 *  $Log: bsearch.c,v $
 *  Revision 1.10  1995/08/07 21:49:01  smoot
 *  fixed bug in initial-B-frame searches
 *
 *  Revision 1.9  1995/06/26 21:36:07  smoot
 *  added new ordering constraints
 *  (B frames which are backward P's at the start of a sequence)
 *
 *  Revision 1.8  1995/03/27 19:17:43  smoot
 *  killed useless type error messge (int32_mpeg_t defiend as int)
 *
 * Revision 1.7  1995/01/19  23:07:20  eyhung
 * Changed copyrights
 *
 * Revision 1.6  1994/12/07  00:40:36  smoot
 * Added seperate P and B search ranges
 *
 * Revision 1.5  1994/03/15  00:27:11  keving
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
 * Revision 1.1  1993/06/03  21:08:08  keving
 * nothing
 *
 * Revision 1.1  1993/03/02  18:27:05  keving
 * nothing
 *
 */


/*==============*
 * HEADER FILES *
 *==============*/

#include "all.h"
#include "mtypes.h"
#include "frames.h"
#include "search.h"
#include "fsize.h"


/*==================*
 * STATIC VARIABLES *
 *==================*/

static int    bsearchAlg;


/*===============================*
 * INTERNAL PROCEDURE prototypes *
 *===============================*/

static int32_mpeg_t    FindBestMatch _ANSI_ARGS_((LumBlock block, LumBlock currentBlock, MpegFrame *prev,
              int by, int bx, int *motionY, int *motionX, int32_mpeg_t bestSoFar, int searchRange));
static int BMotionSearchSimple _ANSI_ARGS_((LumBlock currentBlock, MpegFrame *prev,
            MpegFrame *next, int by, int bx, int *fmy, int *fmx,
            int *bmy, int *bmx, int oldMode));
static int BMotionSearchCross2 _ANSI_ARGS_((LumBlock currentBlock, MpegFrame *prev,
            MpegFrame *next, int by, int bx, int *fmy, int *fmx,
            int *bmy, int *bmx, int oldMode));
static int BMotionSearchExhaust _ANSI_ARGS_((LumBlock currentBlock, MpegFrame *prev,
            MpegFrame *next, int by, int bx, int *fmy, int *fmx,
            int *bmy, int *bmx, int oldMode));
static void BMotionSearchNoInterp _ANSI_ARGS_((LumBlock currentBlock, MpegFrame *prev,
                  MpegFrame *next, int by, int bx,
                  int *fmy, int *fmx, int32_mpeg_t *forwardErr,
                  int *bmy, int *bmx, int32_mpeg_t *backErr,
                           boolean backNeeded));
static int32_mpeg_t    FindBestMatchExhaust _ANSI_ARGS_((LumBlock block, LumBlock currentBlock, MpegFrame *prev,
              int by, int bx, int *motionY, int *motionX,
              int32_mpeg_t bestSoFar, int searchRange));
static int32_mpeg_t    FindBestMatchTwoLevel _ANSI_ARGS_((LumBlock block, LumBlock currentBlock, MpegFrame *prev,
              int by, int bx, int *motionY, int *motionX,
              int32_mpeg_t bestSoFar, int searchRange));
static int32_mpeg_t    FindBestMatchLogarithmic _ANSI_ARGS_((LumBlock block, LumBlock currentBlock, MpegFrame *prev,
              int by, int bx, int *motionY, int *motionX,
              int32_mpeg_t bestSoFar, int searchRange));
static int32_mpeg_t    FindBestMatchSubSample _ANSI_ARGS_((LumBlock block, LumBlock currentBlock, MpegFrame *prev,
              int by, int bx, int *motionY, int *motionX,
              int32_mpeg_t bestSoFar, int searchRange));


/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/

/*===========================*
 * INITIALIZATION PROCEDURES *
 *===========================*/


/*===========================================================================*
 *
 * SetBSearchAlg
 *
 *    set the B-search algorithm
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    bsearchAlg modified
 *
 *===========================================================================*/
void
SetBSearchAlg(alg)
    char *alg;
{
    if ( strcmp(alg, "SIMPLE") == 0 ) {
    bsearchAlg = BSEARCH_SIMPLE;
    } else if ( strcmp(alg, "CROSS2") == 0 ) {
    bsearchAlg = BSEARCH_CROSS2;
    } else if ( strcmp(alg, "EXHAUSTIVE") == 0 ) {
    bsearchAlg = BSEARCH_EXHAUSTIVE;
    } else {
    fprintf(stderr, "ERROR:  Illegal bsearch alg:  %s\n", alg);
    exit(1);
    }
}


/*===========================================================================*
 *
 * BSearchName
 *
 *    return the text of the B-search algorithm
 *
 * RETURNS:    a pointer to the string
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
char *
BSearchName()
{
    switch(bsearchAlg) {
    case BSEARCH_SIMPLE:
        return "SIMPLE";
    case BSEARCH_CROSS2:
        return "CROSS2";
    case BSEARCH_EXHAUSTIVE:
        return "EXHAUSTIVE";
    default:
        exit(1);
        break;
    }
}


/*===========================================================================*
 *
 * BMotionSearch
 *
 *    search for the best B-frame motion vectors
 *
 * RETURNS:    MOTION_FORWARD        forward motion should be used
 *        MOTION_BACKWARD     backward motion should be used
 *        MOTION_INTERPOLATE  both should be used and interpolated
 *
 * OUTPUTS:    *fmx, *fmy  =    TWICE the forward motion vector
 *        *bmx, *bmy  =    TWICE the backward motion vector
 *
 * SIDE EFFECTS:    none
 *
 * PRECONDITIONS:    The relevant block in 'current' is valid (it has not
 *            been dct'd).  Thus, the data in 'current' can be
 *            accesed through y_blocks, cr_blocks, and cb_blocks.
 *            This is not the case for the blocks in 'prev' and
 *            'next.'  Therefore, references into 'prev' and 'next'
 *            should be done
 *            through the struct items ref_y, ref_cr, ref_cb
 *
 * POSTCONDITIONS:    current, prev, next should be unchanged.
 *            Some computation could be saved by requiring
 *            the dct'd difference to be put into current's block
 *            elements here, depending on the search technique.
 *            However, it was decided that it mucks up the code
 *            organization a little, and the saving in computation
 *            would be relatively little (if any).
 *
 * NOTES:    the search procedure MAY return (0,0) motion vectors
 *
 *===========================================================================*/
int
BMotionSearch(currentBlock, prev, next, by, bx, fmy, fmx, bmy, bmx, oldMode)
    LumBlock currentBlock;
    MpegFrame *prev;
    MpegFrame *next;
    int by;
    int bx;
    int *fmy;
    int *fmx;
    int *bmy;
    int *bmx;
    int oldMode;
{
  /* If we are an initial B frame, no possibility of forward motion */
  if (prev == (MpegFrame *) NULL) {
    PMotionSearch(currentBlock, next, by, bx, bmy, bmx);
    return MOTION_BACKWARD;
  }
  
  /* otherwise simply call the appropriate algorithm, based on user preference */
  
    switch(bsearchAlg) {
    case BSEARCH_SIMPLE:
        return BMotionSearchSimple(currentBlock, prev, next, by, bx, fmy,
                       fmx, bmy, bmx, oldMode);
        break;
    case BSEARCH_CROSS2:
        return BMotionSearchCross2(currentBlock, prev, next, by, bx, fmy,
                       fmx, bmy, bmx, oldMode);
        break;
    case BSEARCH_EXHAUSTIVE:
        return BMotionSearchExhaust(currentBlock, prev, next, by, bx, fmy,
                       fmx, bmy, bmx, oldMode);
        break;
    default:
        fprintf(stderr, "Illegal B-frame motion search algorithm:  %d\n",
            bsearchAlg);
        exit(1);
    }
}


/*===========================================================================*
 *
 * BMotionSearchSimple
 *
 *    does a simple search for B-frame motion vectors
 *    see BMotionSearch for generic description
 *
 * DESCRIPTION:
 *    1)  find best backward and forward vectors
 *    2)  compute interpolated error using those two vectors
 *    3)  return the best of the three choices
 *
 *===========================================================================*/
static int
BMotionSearchSimple(currentBlock, prev, next, by, bx, fmy, fmx, bmy, bmx,
            oldMode)
    LumBlock currentBlock;
    MpegFrame *prev;
    MpegFrame *next;
    int by;
    int bx;
    int *fmy;
    int *fmx;
    int *bmy;
    int *bmx;
    int oldMode;
{
    int32_mpeg_t    forwardErr, backErr, interpErr;
    LumBlock    interpBlock;
    int32_mpeg_t    bestSoFar;

                    /* STEP 1 */
    BMotionSearchNoInterp(currentBlock, prev, next, by, bx, fmy, fmx,
              &forwardErr, bmy, bmx, &backErr, TRUE);
              
                    /* STEP 2 */

    ComputeBMotionLumBlock(prev, next, by, bx, MOTION_INTERPOLATE,
               *fmy, *fmx, *bmy, *bmx, interpBlock);
    bestSoFar = min(backErr, forwardErr);
    interpErr = LumBlockMAD(currentBlock, interpBlock, bestSoFar);

                /* STEP 3 */

    if ( interpErr <= forwardErr ) {
    if ( interpErr <= backErr ) {
        return MOTION_INTERPOLATE;
    }
    else
        return MOTION_BACKWARD;
    } else if ( forwardErr <= backErr ) {
    return MOTION_FORWARD;
    } else {
    return MOTION_BACKWARD;
    }
}


/*===========================================================================*
 *
 * BMotionSearchCross2
 *
 *    does a cross-2 search for B-frame motion vectors
 *    see BMotionSearch for generic description
 *
 * DESCRIPTION:
 *    1)  find best backward and forward vectors
 *    2)  find best matching interpolating vectors
 *    3)  return the best of the 4 choices
 *
 *===========================================================================*/
static int
BMotionSearchCross2(currentBlock, prev, next, by, bx, fmy, fmx, bmy, bmx,
            oldMode)
    LumBlock currentBlock;
    MpegFrame *prev;
    MpegFrame *next;
    int by;
    int bx;
    int *fmy;
    int *fmx;
    int *bmy;
    int *bmx;
    int oldMode;
{
    LumBlock    forwardBlock, backBlock;
    int32_mpeg_t    forwardErr, backErr, interpErr;
    int        newfmy, newfmx, newbmy, newbmx;
    int32_mpeg_t    interpErr2;
    int32_mpeg_t    bestErr;

                /* STEP 1 */

    BMotionSearchNoInterp(currentBlock, prev, next, by, bx, fmy, fmx,
              &forwardErr, bmy, bmx, &backErr, TRUE);

    bestErr = min(forwardErr, backErr);

                /* STEP 2 */
    ComputeBMotionLumBlock(prev, next, by, bx, MOTION_FORWARD,
               *fmy, *fmx, 0, 0, forwardBlock);
    ComputeBMotionLumBlock(prev, next, by, bx, MOTION_BACKWARD,
               0, 0, *bmy, *bmx, backBlock);

    /* try a cross-search; total of 4 local searches */    
    newbmy = *bmy;    newbmx = *bmx;
    newfmy = *fmy;    newfmx = *fmx;

    interpErr = FindBestMatch(forwardBlock, currentBlock, next, by, bx,
                  &newbmy, &newbmx, bestErr, searchRangeB);
    bestErr = min(bestErr, interpErr);
    interpErr2 = FindBestMatch(backBlock, currentBlock, prev, by, bx,
                   &newfmy, &newfmx, bestErr, searchRangeB);

                /* STEP 3 */

    if ( interpErr <= interpErr2 ) {
    newfmy = *fmy;
    newfmx = *fmx;
    }
    else
    {
    newbmy = *bmy;
    newbmx = *bmx;
    interpErr = interpErr2;
    }

    if ( interpErr <= forwardErr ) {
    if ( interpErr <= backErr ) {
        *fmy = newfmy;
        *fmx = newfmx;
        *bmy = newbmy;
        *bmx = newbmx;

        return MOTION_INTERPOLATE;
    }
    else
        return MOTION_BACKWARD;
    } else if ( forwardErr <= backErr ) {
    return MOTION_FORWARD;
    } else {
    return MOTION_BACKWARD;
    }
}


/*===========================================================================*
 *
 * BMotionSearchExhaust
 *
 *    does an exhaustive search for B-frame motion vectors
 *    see BMotionSearch for generic description
 *
 * DESCRIPTION:
 *    1)  find best backward and forward vectors
 *    2)  use exhaustive search to find best interpolating vectors
 *    3)  return the best of the 3 choices
 *
 *===========================================================================*/
static int
BMotionSearchExhaust(currentBlock, prev, next, by, bx, fmy, fmx, bmy, bmx,
            oldMode)
    LumBlock currentBlock;
    MpegFrame *prev;
    MpegFrame *next;
    int by;
    int bx;
    int *fmy;
    int *fmx;
    int *bmy;
    int *bmx;
    int oldMode;
{
    register int mx, my;
    int32_mpeg_t diff, bestDiff;
    int        stepSize;
    LumBlock    forwardBlock;
    int32_mpeg_t    forwardErr, backErr;
    int        newbmy, newbmx;
    int        leftMY, leftMX;
    int        rightMY, rightMX;
    boolean result;

                /* STEP 1 */

    BMotionSearchNoInterp(currentBlock, prev, next, by, bx, fmy, fmx,
              &forwardErr, bmy, bmx, &backErr, FALSE);

    if ( forwardErr <= backErr ) {
        bestDiff = forwardErr;
    result = MOTION_FORWARD;
    }
    else
    {
        bestDiff = backErr;
    result = MOTION_BACKWARD;
    }

                /* STEP 2 */

    stepSize = (pixelFullSearch ? 2 : 1);

    COMPUTE_MOTION_BOUNDARY(by,bx,stepSize,leftMY,leftMX,rightMY,rightMX);

    if ( searchRangeB < rightMY ) {
    rightMY = searchRangeB;
    }
    if ( searchRangeB < rightMX ) {
    rightMX = searchRangeB;
    }

    for ( my = -searchRangeB; my < rightMY; my += stepSize ) {
    if ( my < leftMY ) {
        continue;
    }

    for ( mx = -searchRangeB; mx < rightMX; mx += stepSize ) {
        if ( mx < leftMX ) {
        continue;
        }

        ComputeBMotionLumBlock(prev, next, by, bx, MOTION_FORWARD,
               my, mx, 0, 0, forwardBlock);

        newbmy = my;    newbmx = mx;

        diff = FindBestMatch(forwardBlock, currentBlock, next, by, bx,
                 &newbmy, &newbmx, bestDiff, searchRangeB);

        if ( diff < bestDiff ) {
        *fmy = my;
        *fmx = mx;
        *bmy = newbmy;
        *bmx = newbmx;
        bestDiff = diff;
        result = MOTION_INTERPOLATE;
        }
    }
    }

    return result;
}


/*===========================================================================*
 *
 * FindBestMatch
 *
 *    given a motion-compensated block in one direction, tries to find
 *    the best motion vector in the opposite direction to match it
 *
 * RETURNS:    the best vector (*motionY, *motionX), and the corresponding
 *        error is returned if it is better than bestSoFar.  If not,
 *        then a number greater than bestSoFar is returned and
 *        (*motionY, *motionX) has no meaning.
 *
 * SIDE EFFECTS:  none
 *
 *===========================================================================*/
static int32_mpeg_t
FindBestMatch(block, currentBlock, prev, by, bx, motionY, motionX, bestSoFar, searchRange)
    LumBlock block;
    LumBlock currentBlock;
    MpegFrame *prev;
    int by;
    int bx;
    int *motionY;
    int *motionX;
    int32_mpeg_t bestSoFar;
    int searchRange;
{
    int32_mpeg_t    result;

    switch(psearchAlg) {
    case PSEARCH_SUBSAMPLE:
        result = FindBestMatchSubSample(block, currentBlock, prev, by, bx,
                        motionY, motionX, bestSoFar, searchRange);
        break;
    case PSEARCH_EXHAUSTIVE:
        result = FindBestMatchExhaust(block, currentBlock, prev, by, bx,
                      motionY, motionX, bestSoFar, searchRange);
        break;
    case PSEARCH_LOGARITHMIC:
        result = FindBestMatchLogarithmic(block, currentBlock, prev, by, bx,
                          motionY, motionX, bestSoFar, searchRange);
        break;
    case PSEARCH_TWOLEVEL:
        result = FindBestMatchTwoLevel(block, currentBlock, prev, by, bx,
                       motionY, motionX, bestSoFar, searchRange);
        break;
    default:
        fprintf(stderr, "ERROR:  Illegal P-search alg %d\n", psearchAlg);
        exit(1);
    }

    return result;
}


/*===========================================================================*
 *
 * FindBestMatchExhaust
 *
 *    tries to find matching motion vector
 *    see FindBestMatch for generic description
 *
 * DESCRIPTION:  uses an exhaustive search
 *
 *===========================================================================*/
static int32_mpeg_t
FindBestMatchExhaust(block, currentBlock, prev, by, bx, motionY, motionX,
             bestSoFar, searchRange)
    LumBlock block;
    LumBlock currentBlock;
    MpegFrame *prev;
    int by;
    int bx;
    int *motionY;
    int *motionX;
    int32_mpeg_t bestSoFar;
    int searchRange;
{
    register int mx, my;
    int32_mpeg_t diff, bestDiff;
    int        stepSize;
    int        leftMY, leftMX;
    int        rightMY, rightMX;
    int        distance;
    int        tempRightMY, tempRightMX;
    boolean changed = FALSE;

    stepSize = (pixelFullSearch ? 2 : 1);

    COMPUTE_MOTION_BOUNDARY(by,bx,stepSize,leftMY,leftMX,rightMY,rightMX);

    /* try old motion vector first */
    if ( VALID_MOTION(*motionY, *motionX) ) {
    bestDiff = LumAddMotionError(currentBlock, block, prev, by, bx,
                     *motionY, *motionX, bestSoFar);

    if ( bestSoFar < bestDiff ) {
        bestDiff = bestSoFar;
    }
    }
    else
    {
    *motionY = 0;
    *motionX = 0;

    bestDiff = bestSoFar;
    }

/* maybe should try spiral pattern centered around  prev motion vector? */


    /* try a spiral pattern */    
    for ( distance = stepSize; distance <= searchRange; distance += stepSize ) {
    tempRightMY = rightMY;
    if ( distance < tempRightMY ) {
        tempRightMY = distance;
    }
    tempRightMX = rightMX;
    if ( distance < tempRightMX ) {
        tempRightMX = distance;
    }

    /* do top, bottom */
    for ( my = -distance; my < tempRightMY;
          my += max(tempRightMY+distance-stepSize, stepSize) ) {
        if ( my < leftMY ) {
        continue;
        }

        for ( mx = -distance; mx < tempRightMX; mx += stepSize ) {
        if ( mx < leftMX ) {
            continue;
        }

        diff = LumAddMotionError(currentBlock, block, prev, by, bx,
                     my, mx, bestDiff);

        if ( diff < bestDiff ) {
            *motionY = my;
            *motionX = mx;
            bestDiff = diff;
        }
        }
    }

    /* do left, right */
    for ( mx = -distance; mx < tempRightMX; mx += max(tempRightMX+distance-stepSize, stepSize) ) {
        if ( mx < leftMX ) {
        continue;
        }

        for ( my = -distance+stepSize; my < tempRightMY-stepSize; my += stepSize ) {
        if ( my < leftMY ) {
            continue;
        }

        diff = LumAddMotionError(currentBlock, block, prev, by, bx,
                     my, mx, bestDiff);

        if ( diff < bestDiff ) {
            *motionY = my;
            *motionX = mx;
            bestDiff = diff;
            changed = TRUE;
        }
        }
    }
    }

    if ( ! changed ) {
    bestDiff++;
    }

    return bestDiff;
}


/*===========================================================================*
 *
 * FindBestMatchTwoLevel
 *
 *    tries to find matching motion vector
 *    see FindBestMatch for generic description
 *
 * DESCRIPTION:  uses an exhaustive full-pixel search, then looks at
 *         neighboring half-pixels
 *
 *===========================================================================*/
static int32_mpeg_t
FindBestMatchTwoLevel(block, currentBlock, prev, by, bx, motionY, motionX,
              bestSoFar, searchRange)
    LumBlock block;
    LumBlock currentBlock;
    MpegFrame *prev;
    int by;
    int bx;
    int *motionY;
    int *motionX;
    int32_mpeg_t bestSoFar;
    int searchRange;
{
    register int mx, my;
    int32_mpeg_t diff, bestDiff;
    int        leftMY, leftMX;
    int        rightMY, rightMX;
    int        distance;
    int        tempRightMY, tempRightMX;
    boolean changed = FALSE;
    int        yOffset, xOffset;

    /* exhaustive full-pixel search first */

    COMPUTE_MOTION_BOUNDARY(by,bx,2,leftMY,leftMX,rightMY,rightMX);

    rightMY--;
    rightMX--;

    /* convert vector into full-pixel vector */
    if ( *motionY > 0 ) {
    if ( ((*motionY) % 2) == 1 ) {
        (*motionY)--;
    }
    } else if ( ((-(*motionY)) % 2) == 1 ) {
    (*motionY)++;
    }

    if ( *motionX > 0 ) {
    if ( ((*motionX) % 2) == 1 ) {
        (*motionX)--;
    }
    } else if ( ((-(*motionX)) % 2) == 1 ) {
    (*motionX)++;
    }

    /* try old motion vector first */
    if ( VALID_MOTION(*motionY, *motionX) ) {
    bestDiff = LumAddMotionError(currentBlock, block, prev, by, bx,
                     *motionY, *motionX, bestSoFar);

    if ( bestSoFar < bestDiff ) {
        bestDiff = bestSoFar;
    }
    }
    else
    {
    *motionY = 0;
    *motionX = 0;

    bestDiff = bestSoFar;
    }

    rightMY++;
    rightMX++;

/* maybe should try spiral pattern centered around  prev motion vector? */


    /* try a spiral pattern */    
    for ( distance = 2; distance <= searchRange; distance += 2 ) {
    tempRightMY = rightMY;
    if ( distance < tempRightMY ) {
        tempRightMY = distance;
    }
    tempRightMX = rightMX;
    if ( distance < tempRightMX ) {
        tempRightMX = distance;
    }

    /* do top, bottom */
    for ( my = -distance; my < tempRightMY;
          my += max(tempRightMY+distance-2, 2) ) {
        if ( my < leftMY ) {
        continue;
        }

        for ( mx = -distance; mx < tempRightMX; mx += 2 ) {
        if ( mx < leftMX ) {
            continue;
        }

        diff = LumAddMotionError(currentBlock, block, prev, by, bx,
                     my, mx, bestDiff);

        if ( diff < bestDiff ) {
            *motionY = my;
            *motionX = mx;
            bestDiff = diff;
        }
        }
    }

    /* do left, right */
    for ( mx = -distance; mx < tempRightMX; mx += max(tempRightMX+distance-2, 2) ) {
        if ( mx < leftMX ) {
        continue;
        }

        for ( my = -distance+2; my < tempRightMY-2; my += 2 ) {
        if ( my < leftMY ) {
            continue;
        }

        diff = LumAddMotionError(currentBlock, block, prev, by, bx,
                     my, mx, bestDiff);

        if ( diff < bestDiff ) {
            *motionY = my;
            *motionX = mx;
            bestDiff = diff;
            changed = TRUE;
        }
        }
    }
    }

    /* now look at neighboring half-pixels */
    my = *motionY;
    mx = *motionX;

    rightMY--;
    rightMX--;

    for ( yOffset = -1; yOffset <= 1; yOffset++ ) {
    for ( xOffset = -1; xOffset <= 1; xOffset++ ) {
        if ( (yOffset == 0) && (xOffset == 0) )
        continue;

        if ( VALID_MOTION(my+yOffset, mx+xOffset) &&
         ((diff = LumAddMotionError(currentBlock, block, prev, by, bx,
             my+yOffset, mx+xOffset, bestDiff)) < bestDiff) ) {
        *motionY = my+yOffset;
        *motionX = mx+xOffset;
        bestDiff = diff;
        changed = TRUE;
        }
    }
    }

    if ( ! changed ) {
    bestDiff++;
    }

    return bestDiff;
}


/*===========================================================================*
 *
 * FindBestMatchLogarithmic
 *
 *    tries to find matching motion vector
 *    see FindBestMatch for generic description
 *
 * DESCRIPTION:  uses a logarithmic search
 *
 *===========================================================================*/
static int32_mpeg_t
FindBestMatchLogarithmic(block, currentBlock, prev, by, bx, motionY, motionX,
             bestSoFar, searchRange)
    LumBlock block;
    LumBlock currentBlock;
    MpegFrame *prev;
    int by;
    int bx;
    int *motionY;
    int *motionX;
    int32_mpeg_t bestSoFar;
    int searchRange;
{
    register int mx, my;
    int32_mpeg_t diff, bestDiff;
    int        stepSize;
    int        leftMY, leftMX;
    int        rightMY, rightMX;
    int        tempRightMY, tempRightMX;
    int        spacing;
    int        centerX, centerY;
    int        newCenterX, newCenterY;

    stepSize = (pixelFullSearch ? 2 : 1);

    COMPUTE_MOTION_BOUNDARY(by,bx,stepSize,leftMY,leftMX,rightMY,rightMX);

    bestDiff = 0x7fffffff;

    /* grid spacing */
    if ( stepSize == 2 ) {    /* make sure spacing is even */
    spacing = (searchRange+1)/2;
    if ( (spacing % 2) != 0 ) {
        spacing++;
    }
    }
    else
    spacing = (searchRange+1)/2;
    centerX = 0;
    centerY = 0;

    while ( spacing >= stepSize ) {
    newCenterY = centerY;
    newCenterX = centerX;

    tempRightMY = rightMY;
    if ( centerY+spacing+1 < tempRightMY ) {
        tempRightMY = centerY+spacing+1;
    }
    tempRightMX = rightMX;
    if ( centerX+spacing+1 < tempRightMX ) {
        tempRightMX = centerX+spacing+1;
    }

    for ( my = centerY-spacing; my < tempRightMY; my += spacing ) {
        if ( my < leftMY ) {
        continue;
        }

        for ( mx = centerX-spacing; mx < tempRightMX; mx += spacing ) {
        if ( mx < leftMX ) {
            continue;
        }

        diff = LumAddMotionError(currentBlock, block, prev, by, bx,
                     my, mx, bestDiff);

        if ( diff < bestDiff ) {
            newCenterY = my;
            newCenterX = mx;

            bestDiff = diff;
        }
        }
    }

    centerY = newCenterY;
    centerX = newCenterX;

    if ( stepSize == 2 ) {    /* make sure spacing is even */
        if ( spacing == 2 ) {
        spacing = 0;
        }
        else
        {
        spacing = (spacing+1)/2;
        if ( (spacing % 2) != 0 ) {
            spacing++;
        }
        }
    }
    else
    {
        if ( spacing == 1 ) {
        spacing = 0;
        }
        else
        spacing = (spacing+1)/2;
    }
    }

    /* check old motion -- see if it's better */
    if ( (*motionY >= leftMY) && (*motionY < rightMY) &&
     (*motionX >= leftMX) && (*motionX < rightMX) ) {
    diff = LumAddMotionError(currentBlock, block, prev, by, bx, *motionY, *motionX, bestDiff);
    } else {
    diff = 0x7fffffff;
    }

    if ( bestDiff < diff ) {
    *motionY = centerY;
    *motionX = centerX;
    }
    else
    bestDiff = diff;

    return bestDiff;
}


/*===========================================================================*
 *
 * FindBestMatchSubSample
 *
 *    tries to find matching motion vector
 *    see FindBestMatch for generic description
 *
 * DESCRIPTION:  should use subsampling method, but too lazy to write all
 *         the code for it (so instead just calls FindBestMatchExhaust)
 *
 *===========================================================================*/
static int32_mpeg_t
FindBestMatchSubSample(block, currentBlock, prev, by, bx, motionY, motionX,
             bestSoFar, searchRange)
    LumBlock block;
    LumBlock currentBlock;
    MpegFrame *prev;
    int by;
    int bx;
    int *motionY;
    int *motionX;
    int32_mpeg_t bestSoFar;
    int searchRange;
{
    /* too lazy to write the code for this... */

    return FindBestMatchExhaust(block, currentBlock, prev,
                by, bx, motionY, motionX, bestSoFar, searchRange);
}


/*===========================================================================*
 *
 * BMotionSearchNoInterp
 *
 *    finds the best backward and forward motion vectors
 *    if backNeeded == FALSE, then won't find best backward vector if it
 *    is worse than the best forward vector
 *
 * RETURNS:    (*fmy,*fmx) and associated error *forwardErr
 *        (*bmy,*bmx) and associated error *backErr
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
BMotionSearchNoInterp(currentBlock, prev, next, by, bx, fmy, fmx, forwardErr,
              bmy, bmx, backErr, backNeeded)
    LumBlock currentBlock;
    MpegFrame *prev;
    MpegFrame *next;
    int by;
    int bx;
    int *fmy;
    int *fmx;
    int32_mpeg_t *forwardErr;
    int *bmy;
    int *bmx;
    int32_mpeg_t *backErr;
    boolean backNeeded;
{
    /* CALL SEARCH PROCEDURE */
    switch(psearchAlg) {
    case PSEARCH_SUBSAMPLE:
        *forwardErr = PSubSampleSearch(currentBlock, prev, by, bx, 
                       fmy, fmx, searchRangeB);
        *backErr = PSubSampleSearch(currentBlock, next, by, bx, 
                    bmy, bmx, searchRangeB);
        break;
    case PSEARCH_EXHAUSTIVE:
        *forwardErr = PLocalSearch(currentBlock, prev, by, bx, fmy, fmx, 
                       0x7fffffff, searchRangeB);
        if ( backNeeded ) {
        *backErr = PLocalSearch(currentBlock, next, by, bx, bmy, bmx, 
                    0x7fffffff, searchRangeB);
        } else {
        *backErr = PLocalSearch(currentBlock, next, by, bx, bmy, bmx, 
                    *forwardErr, searchRangeB);
        }
        break;
    case PSEARCH_LOGARITHMIC:
        *forwardErr = PLogarithmicSearch(currentBlock, prev, by, bx, 
                         fmy, fmx, searchRangeB);
        *backErr = PLogarithmicSearch(currentBlock, next, by, bx, 
                      bmy, bmx, searchRangeB);
        break;
    case PSEARCH_TWOLEVEL:
        *forwardErr = PTwoLevelSearch(currentBlock, prev, by, bx, fmy, fmx, 
                      0x7fffffff, searchRangeB);
        if ( backNeeded ) {
        *backErr = PTwoLevelSearch(currentBlock, next, by, bx, bmy, bmx, 
                       0x7fffffff, searchRangeB);
        } else {
        *backErr = PTwoLevelSearch(currentBlock, next, by, bx, bmy, bmx, 
                       *forwardErr, searchRangeB);
        }
        break;
    default:
        fprintf(stderr, "ERROR:  Illegal PSEARCH ALG:  %d\n", psearchAlg);
        exit(1);    
        break;
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

/*===========================================================================*
 *
 * ValidBMotion
 *
 *    decides if the given B-frame motion is valid
 *
 * RETURNS:    TRUE if the motion is valid, FALSE otherwise
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
boolean
ValidBMotion(by, bx, mode, fmy, fmx, bmy, bmx)
    int by;
    int bx;
    int mode;
    int fmy;
    int fmx;
    int bmy;
    int bmx;
{
    if ( mode != MOTION_BACKWARD ) {
    /* check forward motion for bounds */
    if ( (by*DCTSIZE+(fmy-1)/2 < 0) || ((by+2)*DCTSIZE+(fmy+1)/2-1 >= Fsize_y) ) {
        return FALSE;
    }
    if ( (bx*DCTSIZE+(fmx-1)/2 < 0) || ((bx+2)*DCTSIZE+(fmx+1)/2-1 >= Fsize_x) ) {
        return FALSE;
    }
    }

    if ( mode != MOTION_FORWARD ) {
    /* check backward motion for bounds */
    if ( (by*DCTSIZE+(bmy-1)/2 < 0) || ((by+2)*DCTSIZE+(bmy+1)/2-1 >= Fsize_y) ) {
        return FALSE;
    }
    if ( (bx*DCTSIZE+(bmx-1)/2 < 0) || ((bx+2)*DCTSIZE+(bmx+1)/2-1 >= Fsize_x) ) {
        return FALSE;
    }
    }

    return TRUE;
}


#endif /* UNUSED_PROCEDURES */
