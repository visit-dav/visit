/*===========================================================================*
 * rgbtoycc.c                                     *
 *                                         *
 *    Procedures to convert from RGB space to YUV space             *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    PNMtoYUV                                 *
 *    PPMtoYUV                                 *
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
 *  $Header: /n/picasso/project/mpeg/mpeg_dist/mpeg_encode/RCS/rgbtoycc.c,v 1.5 1995/08/14 22:32:16 smoot Exp $
 *  $Log: rgbtoycc.c,v $
 *  Revision 1.5  1995/08/14 22:32:16  smoot
 *  added better error message
 *
 *  Revision 1.4  1995/01/19 23:09:23  eyhung
 *  Changed copyrights
 *
 * Revision 1.3  1994/11/12  02:12:00  keving
 * nothing
 *
 * Revision 1.2  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.2  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.1  1993/07/22  22:23:43  keving
 * nothing
 *
 */


/*==============*
 * HEADER FILES *
 *==============*/

#include "all.h"
#include "frame.h"
#include "fsize.h"
#include "rgbtoycc.h"


/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/


/*===========================================================================*
 *
 * PNMtoYUV
 *
 *    convert PNM data into YUV data
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
PNMtoYUV(frame)
    MpegFrame *frame;
{
    register int x, y;
    register uint8_mpeg_t *dy0, *dy1;
    register uint8_mpeg_t *dcr, *dcb;
    register xel *src0, *src1;
    register int ydivisor, cdivisor;
    static boolean  first = TRUE;
    static float  mult299[1024], mult587[1024], mult114[1024];
    static float  mult16874[1024], mult33126[1024], mult5[1024];
    static float mult41869[1024], mult08131[1024];

    if ( first ) {
        register int index;
    register int maxValue;

    maxValue = frame->rgb_maxval;

        for ( index = 0; index <= maxValue; index++ ) {
        mult299[index] = index*0.29900;
        mult587[index] = index*0.58700;
        mult114[index] = index*0.11400;
        mult16874[index] = -0.16874*index;
        mult33126[index] = -0.33126*index;
        mult5[index] = index*0.50000;
        mult41869[index] = -0.41869*index;
        mult08131[index] = -0.08131*index;
    }
    
    first = FALSE;
    }

    Frame_AllocYCC(frame);

    /*
     * okay.  Now, convert everything into YCrCb space. (the specific
     * numbers come from the JPEG source, jccolor.c) The conversion
     * equations to be implemented are therefore
     *
     * Y  =  0.29900 * R + 0.58700 * G + 0.11400 * B
     * Cb = -0.16874 * R - 0.33126 * G + 0.50000 * B
     * Cr =  0.50000 * R - 0.41869 * G - 0.08131 * B
     */

/* ydivisor should be a FLOAT, shouldn't it?!?! */

    ydivisor = (frame->rgb_maxval + 1) >> 8;    /* for normalizing values
                         * 0-255, divide by 256 */
    cdivisor = (ydivisor << 2);        /* because we're averaging 4 pixels */

    for (y = 0; y < Fsize_y; y += 2) {
    for (x = 0, src0 = frame->rgb_data[y], src1 = frame->rgb_data[y + 1],
         dy0 = frame->orig_y[y], dy1 = frame->orig_y[y + 1],
         dcr = frame->orig_cr[y >> 1], dcb = frame->orig_cb[y >> 1];
         x < Fsize_x;
         x += 2, dy0 += 2, dy1 += 2, dcr++,
         dcb++, src0 += 2, src1 += 2) {

        *dy0 = (mult299[PPM_GETR(*src0)] +
            mult587[PPM_GETG(*src0)] +
            mult114[PPM_GETB(*src0)]) / ydivisor;

        *dy1 = (mult299[PPM_GETR(*src1)] +
            mult587[PPM_GETG(*src1)] +
            mult114[PPM_GETB(*src1)]) / ydivisor;

        dy0[1] = (mult299[PPM_GETR(src0[1])] +
              mult587[PPM_GETG(src0[1])] +
              mult114[PPM_GETB(src0[1])]) / ydivisor;

        dy1[1] = (mult299[PPM_GETR(src1[1])] +
              mult587[PPM_GETG(src1[1])] +
              mult114[PPM_GETB(src1[1])]) / ydivisor;

        *dcb = ((mult16874[PPM_GETR(*src0)] +
             mult33126[PPM_GETG(*src0)] +
             mult5[PPM_GETB(*src0)] +
             mult16874[PPM_GETR(*src1)] +
             mult33126[PPM_GETG(*src1)] +
             mult5[PPM_GETB(*src1)] +
             mult16874[PPM_GETR(src0[1])] +
             mult33126[PPM_GETG(src0[1])] +
             mult5[PPM_GETB(src0[1])] +
             mult16874[PPM_GETR(src1[1])] +
             mult33126[PPM_GETG(src1[1])] +
             mult5[PPM_GETB(src1[1])]) / cdivisor) + 128;

        *dcr = ((mult5[PPM_GETR(*src0)] +
             mult41869[PPM_GETG(*src0)] +
             mult08131[PPM_GETB(*src0)] +
             mult5[PPM_GETR(*src1)] +
             mult41869[PPM_GETG(*src1)] +
             mult08131[PPM_GETB(*src1)] +
             mult5[PPM_GETR(src0[1])] +
             mult41869[PPM_GETG(src0[1])] +
             mult08131[PPM_GETB(src0[1])] +
             mult5[PPM_GETR(src1[1])] +
             mult41869[PPM_GETG(src1[1])] +
             mult08131[PPM_GETB(src1[1])]) / cdivisor) + 128;

        /* if your floating point is faster than your loads, you
         * might consider this:
         */
#ifdef BLEAH
        *dy0 = (PPM_GETR(*src0) * 0.29900 +
            PPM_GETG(*src0) * 0.58700 +
            PPM_GETB(*src0) * 0.11400) / ydivisor;
        *dy1 = (PPM_GETR(*src1) * 0.29900 +
            PPM_GETG(*src1) * 0.58700 +
            PPM_GETB(*src1) * 0.11400) / ydivisor;

        dy0[1] = (PPM_GETR(src0[1]) * 0.29900 +
              PPM_GETG(src0[1]) * 0.58700 +
              PPM_GETB(src0[1]) * 0.11400) / ydivisor;

        dy1[1] = (PPM_GETR(src1[1]) * 0.29900 +
              PPM_GETG(src1[1]) * 0.58700 +
              PPM_GETB(src1[1]) * 0.11400) / ydivisor;

        *dcb = ((PPM_GETR(*src0) * -0.16874 +
             PPM_GETG(*src0) * -0.33126 +
             PPM_GETB(*src0) * 0.50000 +
             PPM_GETR(*src1) * -0.16874 +
             PPM_GETG(*src1) * -0. +
             PPM_GETB(*src1) * 0.50000 +
             PPM_GETR(src0[1]) * -0.16874 +
             PPM_GETG(src0[1]) * -0.33126 +
             PPM_GETB(src0[1]) * 0.50000 +
             PPM_GETR(src1[1]) * -0.16874 +
             PPM_GETG(src1[1]) * -0.33126 +
             PPM_GETB(src1[1]) * 0.50000) / cdivisor) + 128;

        *dcr = ((PPM_GETR(*src0) * 0.50000 +
             PPM_GETG(*src0) * -0.41869 +
             PPM_GETB(*src0) * -0.08131 +
             PPM_GETR(*src1) * 0.50000 +
             PPM_GETG(*src1) * -0.41869 +
             PPM_GETB(*src1) * -0.08131 +
             PPM_GETR(src0[1]) * 0.50000 +
             PPM_GETG(src0[1]) * -0.41869 +
             PPM_GETB(src0[1]) * -0.08131 +
             PPM_GETR(src1[1]) * 0.50000 +
             PPM_GETG(src1[1]) * -0.41869 +
             PPM_GETB(src1[1]) * -0.08131) / cdivisor) + 128;
#endif

        DBG_PRINT(("%3d,%3d: (%3d,%3d,%3d) --> (%3d,%3d,%3d)\n", x, y, PPM_GETR(*src0), PPM_GETG(*src0), PPM_GETB(*src0), *dy0, *dcb, *dcr));
    }
    }
}



/*===========================================================================*
 *
 * PPMtoYUV
 *
 *    convert PPM data into YUV data
 *    same as PNMtoYUV, except extracts data from ppm_data, and
 *    assumes that ydivisor = 1
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
PPMtoYUV(frame)
    MpegFrame *frame;
{
    register int x, y;
    register uint8_mpeg_t *dy0, *dy1;
    register uint8_mpeg_t *dcr, *dcb;
    register uint8_mpeg_t *src0, *src1;
    register int cdivisor;
    static boolean  first = TRUE;
    static float  mult299[1024], mult587[1024], mult114[1024];
    static float  mult16874[1024], mult33126[1024], mult5[1024];
    static float mult41869[1024], mult08131[1024];

    if ( first ) {
        register int index;
    register int maxValue;

    maxValue = frame->rgb_maxval;

        for ( index = 0; index <= maxValue; index++ ) {
        mult299[index] = index*0.29900;
        mult587[index] = index*0.58700;
        mult114[index] = index*0.11400;
        mult16874[index] = -0.16874*index;
        mult33126[index] = -0.33126*index;
        mult5[index] = index*0.50000;
        mult41869[index] = -0.41869*index;
        mult08131[index] = -0.08131*index;
    }
    
    first = FALSE;
    }

    Frame_AllocYCC(frame);

    /* assume ydivisor = 1, so cdivisor = 4 */
    if ( frame->rgb_maxval != 255 ) {
    fprintf(stderr, "PPM max gray value != 255.  Exiting.\n\tTry PNM type, not PPM\n");
    exit(1);
    }

    cdivisor = 4;

    for (y = 0; y < Fsize_y; y += 2) {
    src0 = frame->ppm_data[y];
    src1 = frame->ppm_data[y + 1];
    dy0 = frame->orig_y[y];
    dy1 = frame->orig_y[y + 1];
    dcr = frame->orig_cr[y >> 1];
    dcb = frame->orig_cb[y >> 1];

    for ( x = 0; x < Fsize_x; x += 2, dy0 += 2, dy1 += 2, dcr++,
                   dcb++, src0 += 6, src1 += 6) {
        *dy0 = (mult299[*src0] +
            mult587[src0[1]] +
            mult114[src0[2]]);

        *dy1 = (mult299[*src1] +
            mult587[src1[1]] +
            mult114[src1[2]]);

        dy0[1] = (mult299[src0[3]] +
              mult587[src0[4]] +
              mult114[src0[5]]);

        dy1[1] = (mult299[src1[3]] +
              mult587[src1[4]] +
              mult114[src1[5]]);

        *dcb = ((mult16874[*src0] +
             mult33126[src0[1]] +
             mult5[src0[2]] +
             mult16874[*src1] +
             mult33126[src1[1]] +
             mult5[src1[2]] +
             mult16874[src0[3]] +
             mult33126[src0[4]] +
             mult5[src0[5]] +
             mult16874[src1[3]] +
             mult33126[src1[4]] +
             mult5[src1[5]]) / cdivisor) + 128;

        *dcr = ((mult5[*src0] +
             mult41869[src0[1]] +
             mult08131[src0[2]] +
             mult5[*src1] +
             mult41869[src1[1]] +
             mult08131[src1[2]] +
             mult5[src0[3]] +
             mult41869[src0[4]] +
             mult08131[src0[5]] +
             mult5[src1[3]] +
             mult41869[src1[4]] +
             mult08131[src1[5]]) / cdivisor) + 128;

        DBG_PRINT(("%3d,%3d: (%3d,%3d,%3d) --> (%3d,%3d,%3d)\n", x, y, PPM_GETR(*src0), PPM_GETG(*src0), PPM_GETB(*src0), *dy0, *dcb, *dcr));
    }
    }
}

