/*===========================================================================*
 * psearch.c                                     *
 *                                         *
 *    Procedures concerned with the P-frame motion search             *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    SetPixelSearch                                 *
 *    SetPSearchAlg                                 *
 *    SetSearchRange                                 *
 *    MotionSearchPreComputation                         *
 *    PMotionSearch                                 *
 *    PSearchName                                 *
 *    PSubSampleSearch                             *
 *    PLogarithmicSearch                             *
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
 *  $Header: /u/smoot/md/mpeg_encode/RCS/psearch.c,v 1.9 1995/01/19 23:09:12 eyhung Exp $
 *  $Log: psearch.c,v $
 * Revision 1.9  1995/01/19  23:09:12  eyhung
 * Changed copyrights
 *
 * Revision 1.9  1995/01/19  23:09:12  eyhung
 * Changed copyrights
 *
 * Revision 1.8  1994/12/07  00:40:36  smoot
 * Added seperate P and B search ranges
 *
 * Revision 1.7  1994/11/12  02:09:45  eyhung
 * full pixel bug
 * fixed on lines 512 and 563
 *
 * Revision 1.6  1994/03/15  00:27:11  keving
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
#include "prototypes.h"
#include "fsize.h"
#include "param.h"


/*==================*
 * STATIC VARIABLES *
 *==================*/

/* none */


/*==================*
 * GLOBAL VARIABLES *
 *==================*/

int **pmvHistogram = NULL;    /* histogram of P-frame motion vectors */
int **bbmvHistogram = NULL;    /* histogram of B-frame motion vectors */
int **bfmvHistogram = NULL;    /* histogram of B-frame motion vectors */
int pixelFullSearch;
int searchRangeP,searchRangeB;
int psearchAlg;


/*===============================*
 * INTERNAL PROCEDURE prototypes *
 *===============================*/


/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/

/*===========================================================================*
 *
 * PMotionSearch
 *
 *    compute the best P-frame motion vector we can
 *    
 *
 * RETURNS:    TRUE        =    motion vector valid
 *        FALSE        =    motion vector invalid; should code I-block
 *
 * PRECONDITIONS:    The relevant block in 'current' is valid (it has not
 *            been dct'd).  Thus, the data in 'current' can be
 *            accesed through y_blocks, cr_blocks, and cb_blocks.
 *            This is not the case for the blocks in 'prev.'
 *            Therefore, references into 'prev' should be done
 *            through the struct items ref_y, ref_cr, ref_cb
 *
 * POSTCONDITIONS:    current, prev should be unchanged.
 *            Some computation could be saved by requiring
 *            the dct'd difference to be put into current's block
 *            elements here, depending on the search technique.
 *            However, it was decided that it mucks up the code
 *            organization a little, and the saving in computation
 *            would be relatively little (if any).
 *
 * NOTES:    the search procedure need not check the (0,0) motion vector
 *        the calling procedure has a preference toward (0,0) and it
 *        will check it itself
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
boolean
PMotionSearch(currentBlock, prev, by, bx, motionY, motionX)
    LumBlock currentBlock;
    MpegFrame *prev;
    int by;
    int bx;
    int *motionY;
    int *motionX;
{
    /* CALL SEARCH PROCEDURE */

    switch(psearchAlg) {
    case PSEARCH_SUBSAMPLE:
        PSubSampleSearch(currentBlock, prev, by, bx, motionY, motionX, searchRangeP);
        break;
    case PSEARCH_EXHAUSTIVE:
        PLocalSearch(currentBlock, prev, by, bx, motionY, motionX,
             0x7fffffff, searchRangeP);
        break;
    case PSEARCH_LOGARITHMIC:
        PLogarithmicSearch(currentBlock, prev, by, bx, motionY, motionX, searchRangeP);
        break;
    case PSEARCH_TWOLEVEL:
        PTwoLevelSearch(currentBlock, prev, by, bx, motionY, motionX,
                0x7fffffff, searchRangeP);
        break;
    default:
        fprintf(stderr, "ILLEGAL PSEARCH ALG:  %d\n", psearchAlg);
        exit(1);
    }

    return TRUE;
}


/*===========================================================================*
 *
 * SetPixelSearch
 *
 *    set the pixel search type (half or full)
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    pixelFullSearch
 *
 *===========================================================================*/
void
SetPixelSearch(searchType)
    char *searchType;
{
    if ( (strcmp(searchType, "FULL") == 0 ) || ( strcmp(searchType, "WHOLE") == 0 )) {
    pixelFullSearch = TRUE;
    } else if ( strcmp(searchType, "HALF") == 0 ) {
    pixelFullSearch = FALSE;
    } else {
    fprintf(stderr, "ERROR:  Invalid pixel search type:  %s\n",
        searchType);
    exit(1);
    }
}


/*===========================================================================*
 *
 * SetPSearchAlg
 *
 *    set the P-search algorithm
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    psearchAlg
 *
 *===========================================================================*/
void
SetPSearchAlg(alg)
    char *alg;
{
    if ( strcmp(alg, "EXHAUSTIVE") == 0 ) {
    psearchAlg = PSEARCH_EXHAUSTIVE;
    } else if (strcmp(alg, "SUBSAMPLE") == 0 ) {
    psearchAlg = PSEARCH_SUBSAMPLE;
    } else if ( strcmp(alg, "LOGARITHMIC") == 0 ) {
    psearchAlg = PSEARCH_LOGARITHMIC;
    } else if ( strcmp(alg, "TWOLEVEL") == 0 ) {
    psearchAlg = PSEARCH_TWOLEVEL;
    } else {
    fprintf(stderr, "ERROR:  Invalid psearch algorithm:  %s\n", alg);
    exit(1);
    }
}


/*===========================================================================*
 *
 * PSearchName
 *
 *    returns a string containing the name of the search algorithm
 *
 * RETURNS:    pointer to the string
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
char *
PSearchName()
{
    switch(psearchAlg) {
    case PSEARCH_EXHAUSTIVE:
        return "EXHAUSTIVE";
    case PSEARCH_SUBSAMPLE:
        return "SUBSAMPLE";
    case PSEARCH_LOGARITHMIC:
        return "LOGARITHMIC";
    case PSEARCH_TWOLEVEL:
        return "TWOLEVEL";
    default:
        exit(1);
        break;
    }
}


/*===========================================================================*
 *
 * SetSearchRange
 *
 *    sets the range of the search to the given number of pixels
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    searchRange*, fCode
 *
 *===========================================================================*/
void
SetSearchRange(pixelsP,pixelsB)
    int pixelsP,pixelsB;
{
    register int index;

    searchRangeP = 2*pixelsP;    /* +/- 'pixels' pixels */
    searchRangeB = 2*pixelsB;
    searchRangeB = 2*pixelsB;

    if ( computeMVHist ) {
      int max_search;
      max_search=(searchRangeP>searchRangeB) ? 
    ((searchRangeP>searchRangeB)?searchRangeP:searchRangeB)
      : ((searchRangeB>searchRangeB)?searchRangeB:searchRangeB);
    
    pmvHistogram = (int **) malloc((2*searchRangeP+3)*sizeof(int *));
    bbmvHistogram = (int **) malloc((2*searchRangeB+3)*sizeof(int *));
    bfmvHistogram = (int **) malloc((2*searchRangeB+3)*sizeof(int *));
    for ( index = 0; index < 2*max_search+3; index++ ) {
        pmvHistogram[index] = (int *) calloc(2*searchRangeP+3, sizeof(int));
        bbmvHistogram[index] = (int *) calloc(2*searchRangeB+3, sizeof(int));
        bfmvHistogram[index] = (int *) calloc(2*searchRangeB+3, sizeof(int));
    }
    }
}


/*===========================================================================*
 *
 *                USER-MODIFIABLE
 *
 * MotionSearchPreComputation
 *
 *    do whatever you want here; this is called once per frame, directly
 *    after reading
 *
 * RETURNS:    whatever
 *
 * SIDE EFFECTS:    whatever
 *
 *===========================================================================*/
void
MotionSearchPreComputation(frame)
    MpegFrame *frame;
{
    /* do nothing */
}


/*===========================================================================*
 *
 * PSubSampleSearch
 *
 *    uses the subsampling algorithm to compute the P-frame vector
 *
 * RETURNS:    motion vector
 *
 * SIDE EFFECTS:    none
 *
 * REFERENCE:  Liu and Zaccarin:  New Fast Algorithms for the Estimation
 *        of Block Motion Vectors, IEEE Transactions on Circuits
 *        and Systems for Video Technology, Vol. 3, No. 2, 1993.
 *
 *===========================================================================*/
int32_mpeg_t
PSubSampleSearch(currentBlock, prev, by, bx, motionY, motionX, searchRange)
    LumBlock currentBlock;
    MpegFrame *prev;
    int by;
    int bx;
    int *motionY;
    int *motionX;
    int searchRange;
{
    register int mx, my;
    int32_mpeg_t diff, bestBestDiff;
    int        stepSize;
    register int x;
    int        bestMY[4], bestMX[4], bestDiff[4];
    int        leftMY, leftMX;
    int        rightMY, rightMX;

    stepSize = (pixelFullSearch ? 2 : 1);

    COMPUTE_MOTION_BOUNDARY(by,bx,stepSize,leftMY,leftMX,rightMY,rightMX);

    if ( searchRange < rightMY ) {
    rightMY = searchRange;
    }

    if ( searchRange < rightMX ) {
    rightMX = searchRange;
    }

    for ( x = 0; x < 4; x++ ) {
    bestMY[x] = 0;
    bestMX[x] = 0;
    bestDiff[x] = 0x7fffffff;
    }

    /* do A pattern */
    for ( my = -searchRange; my < rightMY; my += 2*stepSize ) {
    if ( my < leftMY ) {
        continue;
    }

    for ( mx = -searchRange; mx < rightMX; mx += 2*stepSize ) {
        if ( mx < leftMX ) {
        continue;
        }

        diff = LumMotionErrorA(currentBlock, prev, by, bx, my, mx, bestDiff[0]);

        if ( diff < bestDiff[0] ) {
        bestMY[0] = my;
        bestMX[0] = mx;
        bestDiff[0] = diff;
        }
    }
    }

    /* do B pattern */
    for ( my = stepSize-searchRange; my < rightMY; my += 2*stepSize ) {
    if ( my < leftMY ) {
        continue;
    }

    for ( mx = -searchRange; mx < rightMX; mx += 2*stepSize ) {
        if ( mx < leftMX ) {
        continue;
        }

        diff = LumMotionErrorB(currentBlock, prev, by, bx, my, mx, bestDiff[1]);

        if ( diff < bestDiff[1] ) {
        bestMY[1] = my;
        bestMX[1] = mx;
        bestDiff[1] = diff;
        }
    }
    }

    /* do C pattern */
    for ( my = stepSize-searchRange; my < rightMY; my += 2*stepSize ) {
    if ( my < leftMY ) {
        continue;
    }

    for ( mx = stepSize-searchRange; mx < rightMX; mx += 2*stepSize ) {
        if ( mx < leftMX ) {
        continue;
        }

        diff = LumMotionErrorC(currentBlock, prev, by, bx, my, mx, bestDiff[2]);

        if ( diff < bestDiff[2] ) {
        bestMY[2] = my;
        bestMX[2] = mx;
        bestDiff[2] = diff;
        }
    }
    }

    /* do D pattern */
    for ( my = -searchRange; my < rightMY; my += 2*stepSize ) {
    if ( my < leftMY ) {
        continue;
    }

    for ( mx = stepSize-searchRange; mx < rightMX; mx += 2*stepSize ) {
        if ( mx < leftMX ) {
        continue;
        }

        diff = LumMotionErrorD(currentBlock, prev, by, bx, my, mx, bestDiff[3]);

        if ( diff < bestDiff[3] ) {
        bestMY[3] = my;
        bestMX[3] = mx;
        bestDiff[3] = diff;
        }
    }
    }

    /* first check old motion */
    if ( (*motionY >= leftMY) && (*motionY < rightMY) &&
     (*motionX >= leftMX) && (*motionX < rightMX) ) {
    bestBestDiff = LumMotionError(currentBlock, prev, by, bx, *motionY, *motionX, 0x7fffffff);
    } else {
    bestBestDiff = 0x7fffffff;
    }

    /* look at Error of 4 different motion vectors */
    for ( x = 0; x < 4; x++ ) {
    bestDiff[x] = LumMotionError(currentBlock, prev, by, bx,
                 bestMY[x], bestMX[x], bestBestDiff);

    if ( bestDiff[x] < bestBestDiff ) {
        bestBestDiff = bestDiff[x];
        *motionY = bestMY[x];
        *motionX = bestMX[x];
    }
    }

    return bestBestDiff;
}


/*===========================================================================*
 *
 * PLogarithmicSearch
 *
 *    uses logarithmic search to compute the P-frame vector
 *
 * RETURNS:    motion vector
 *
 * SIDE EFFECTS:    none
 *
 * REFERENCE:  MPEG-I specification, pages 32-33
 *
 *===========================================================================*/
int32_mpeg_t
PLogarithmicSearch(currentBlock, prev, by, bx, motionY, motionX, searchRange)
    LumBlock currentBlock;
    MpegFrame *prev;
    int by;
    int bx;
    int *motionY;
    int *motionX;
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
        spacing--;
    }
    } else {
    spacing = (searchRange+1)/2;
    }
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

        diff = LumMotionError(currentBlock, prev, by, bx, my, mx, bestDiff);

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
        } else {
        spacing = (spacing+1)/2;
        if ( (spacing % 2) != 0 ) {
            spacing--;
        }
        }
    } else {
        if ( spacing == 1 ) {
        spacing = 0;
        } else {
        spacing = (spacing+1)/2;
        }
    }
    }

    /* check old motion -- see if it's better */
    if ( (*motionY >= leftMY) && (*motionY < rightMY) &&
     (*motionX >= leftMX) && (*motionX < rightMX) ) {
    diff = LumMotionError(currentBlock, prev, by, bx, *motionY, *motionX, bestDiff);
    } else {
    diff = 0x7fffffff;
    }

    if ( bestDiff < diff ) {
    *motionY = centerY;
    *motionX = centerX;
    } else {
    bestDiff = diff;
    }

    return bestDiff;
}


/*===========================================================================*
 *
 * PLocalSearch
 *
 *    uses local exhaustive search to compute the P-frame vector
 *
 * RETURNS:    motion vector
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
int32_mpeg_t
PLocalSearch(currentBlock, prev, by, bx, motionY, motionX, bestSoFar, searchRange)
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

    stepSize = (pixelFullSearch ? 2 : 1);

    COMPUTE_MOTION_BOUNDARY(by,bx,stepSize,leftMY,leftMX,rightMY,rightMX);

    /* try old motion vector first */
    if ( VALID_MOTION(*motionY, *motionX) ) {
    bestDiff = LumMotionError(currentBlock, prev, by, bx, *motionY, *motionX, bestSoFar);

    if ( bestSoFar < bestDiff ) {
        bestDiff = bestSoFar;
    }
    } else {
    *motionY = 0;
    *motionX = 0;

    bestDiff = bestSoFar;
    }

    /* try a spiral pattern */    
    for ( distance = stepSize; distance <= searchRange;
      distance += stepSize ) {
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

        diff = LumMotionError(currentBlock, prev, by, bx, my, mx, bestDiff);

        if ( diff < bestDiff ) {
            *motionY = my;
            *motionX = mx;
            bestDiff = diff;
        }
        }
    }

    /* do left, right */
    for ( mx = -distance; mx < tempRightMX;
          mx += max(tempRightMX+distance-stepSize, stepSize) ) {
        if ( mx < leftMX ) {
        continue;
        }

        for ( my = -distance+stepSize; my < tempRightMY-stepSize;
          my += stepSize ) {
        if ( my < leftMY ) {
            continue;
        }

        diff = LumMotionError(currentBlock, prev, by, bx, my, mx, bestDiff);

        if ( diff < bestDiff ) {
            *motionY = my;
            *motionX = mx;
            bestDiff = diff;
        }
        }
    }
    }

    return bestDiff;
}


/*===========================================================================*
 *
 * PTwoLevelSearch
 *
 *    uses two-level search to compute the P-frame vector
 *    first does exhaustive full-pixel search, then looks at neighboring
 *    half-pixel motion vectors
 *
 * RETURNS:    motion vector
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
int32_mpeg_t
PTwoLevelSearch(currentBlock, prev, by, bx, motionY, motionX, bestSoFar, searchRange)
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
    register int   loopInc;
    int32_mpeg_t diff, bestDiff;
    int        leftMY, leftMX;
    int        rightMY, rightMX;
    int        distance;
    int        tempRightMY, tempRightMX;
    int        xOffset, yOffset;

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
    bestDiff = LumMotionError(currentBlock, prev, by, bx, *motionY, *motionX, bestSoFar);

    if ( bestSoFar < bestDiff ) {
        bestDiff = bestSoFar;
    }
    } else {
    *motionY = 0;
    *motionX = 0;

    bestDiff = bestSoFar;
    }

    rightMY++;
    rightMX++;

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
    loopInc = max(tempRightMY+distance-2, 2);
    for ( my = -distance; my < tempRightMY; my += loopInc ) {
        if ( my < leftMY ) {
        continue;
        }

        for ( mx = -distance; mx < tempRightMX; mx += 2 ) {
        if ( mx < leftMX ) {
            continue;
        }

        diff = LumMotionError(currentBlock, prev, by, bx, my, mx, bestDiff);

        if ( diff < bestDiff ) {
            *motionY = my;
            *motionX = mx;
            bestDiff = diff;
        }
        }
    }

    /* do left, right */
    loopInc = max(tempRightMX+distance-2, 2);
    for ( mx = -distance; mx < tempRightMX; mx += loopInc ) {
        if ( mx < leftMX ) {
        continue;
        }

        for ( my = -distance+2; my < tempRightMY-2; my += 2 ) {
        if ( my < leftMY ) {
            continue;
        }

        diff = LumMotionError(currentBlock, prev, by, bx, my, mx, bestDiff);

        if ( diff < bestDiff ) {
            *motionY = my;
            *motionX = mx;
            bestDiff = diff;
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
         ((diff = LumMotionError(currentBlock, prev, by, bx,
             my+yOffset, mx+xOffset, bestDiff)) < bestDiff) ) {
        *motionY = my+yOffset;
        *motionX = mx+xOffset;
        bestDiff = diff;
        }
    }
    }

    return bestDiff;
}


void
ShowPMVHistogram(fpointer)
    FILE *fpointer;
{
    register int x, y;
    int    *columnTotals;
    int rowTotal;

    columnTotals = (int *) calloc(2*searchRangeP+3, sizeof(int));

#ifdef COMPLETE_DISPLAY
    fprintf(fpointer, "    ");
    for ( y = 0; y < 2*searchRange+3; y++ ) {
    fprintf(fpointer, "%3d ", y-searchRangeP-1);
    }
    fprintf(fpointer, "\n");
#endif

    for ( x = 0; x < 2*searchRangeP+3; x++ ) {
#ifdef COMPLETE_DISPLAY
    fprintf(fpointer, "%3d ", x-searchRangeP-1);
#endif
    rowTotal = 0;
    for ( y = 0; y < 2*searchRangeP+3; y++ ) {
        fprintf(fpointer, "%3d ", pmvHistogram[x][y]);
        rowTotal += pmvHistogram[x][y];
        columnTotals[y] += pmvHistogram[x][y];
    }
#ifdef COMPLETE_DISPLAY
    fprintf(fpointer, "%4d\n", rowTotal);
#else
    fprintf(fpointer, "\n");
#endif
    }

#ifdef COMPLETE_DISPLAY
    fprintf(fpointer, "Tot ");
    for ( y = 0; y < 2*searchRangeP+3; y++ ) {
    fprintf(fpointer, "%3d ", columnTotals[y]);
    }
#endif
    fprintf(fpointer, "\n");
}


void
ShowBBMVHistogram(fpointer)
    FILE *fpointer;
{
    register int x, y;
    int    *columnTotals;
    int rowTotal;

    fprintf(fpointer, "B-frame Backwards:\n");

    columnTotals = (int *) calloc(2*searchRangeB+3, sizeof(int));

#ifdef COMPLETE_DISPLAY
    fprintf(fpointer, "    ");
    for ( y = 0; y < 2*searchRangeB+3; y++ ) {
    fprintf(fpointer, "%3d ", y-searchRangeB-1);
    }
    fprintf(fpointer, "\n");
#endif

    for ( x = 0; x < 2*searchRangeB+3; x++ ) {
#ifdef COMPLETE_DISPLAY
    fprintf(fpointer, "%3d ", x-searchRangeB-1);
#endif
    rowTotal = 0;
    for ( y = 0; y < 2*searchRangeB+3; y++ ) {
        fprintf(fpointer, "%3d ", bbmvHistogram[x][y]);
        rowTotal += bbmvHistogram[x][y];
        columnTotals[y] += bbmvHistogram[x][y];
    }
#ifdef COMPLETE_DISPLAY
    fprintf(fpointer, "%4d\n", rowTotal);
#else
    fprintf(fpointer, "\n");
#endif
    }

#ifdef COMPLETE_DISPLAY
    fprintf(fpointer, "Tot ");
    for ( y = 0; y < 2*searchRangeB+3; y++ ) {
    fprintf(fpointer, "%3d ", columnTotals[y]);
    }
#endif
    fprintf(fpointer, "\n");
}


void
ShowBFMVHistogram(fpointer)
    FILE *fpointer;
{
    register int x, y;
    int    *columnTotals;
    int rowTotal;

    fprintf(fpointer, "B-frame Forwards:\n");

    columnTotals = (int *) calloc(2*searchRangeB+3, sizeof(int));

#ifdef COMPLETE_DISPLAY
    fprintf(fpointer, "    ");
    for ( y = 0; y < 2*searchRangeB+3; y++ ) {
    fprintf(fpointer, "%3d ", y-searchRangeB-1);
    }
    fprintf(fpointer, "\n");
#endif

    for ( x = 0; x < 2*searchRangeB+3; x++ ) {
#ifdef COMPLETE_DISPLAY
    fprintf(fpointer, "%3d ", x-searchRangeB-1);
#endif
    rowTotal = 0;
    for ( y = 0; y < 2*searchRangeB+3; y++ ) {
        fprintf(fpointer, "%3d ", bfmvHistogram[x][y]);
        rowTotal += bfmvHistogram[x][y];
        columnTotals[y] += bfmvHistogram[x][y];
    }
#ifdef COMPLETE_DISPLAY
    fprintf(fpointer, "%4d\n", rowTotal);
#else
    fprintf(fpointer, "\n");
#endif
    }

#ifdef COMPLETE_DISPLAY
    fprintf(fpointer, "Tot ");
    for ( y = 0; y < 2*searchRangeB+3; y++ ) {
    fprintf(fpointer, "%3d ", columnTotals[y]);
    }
#endif
    fprintf(fpointer, "\n");
}


/*=====================*
 * INTERNAL PROCEDURES *
 *=====================*/

    /* none */

