/*===========================================================================*
 * frame.h                                     *
 *                                         *
 *    basic frames procedures                             *
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


#ifndef FRAME_INCLUDED
#define FRAME_INCLUDED

/*==============*
 * HEADER FILES *
 *==============*/

#include "general.h"
#include "ansi.h"
#include "mtypes.h"


/*===========*
 * CONSTANTS *
 *===========*/
#define TYPE_IFRAME    2
#define TYPE_PFRAME    3
#define TYPE_BFRAME    4


/*=======================*
 * STRUCTURE DEFINITIONS *
 *=======================*/

typedef struct mpegFrame {
    int type;
    char    inputFileName[256];
    int id;           /* the frame number -- starts at 0 */
    boolean inUse;    /* TRUE iff this frame is currently being used */
            /* FALSE means any data here can be thrashed */

    uint8_mpeg_t   **ppm_data;
    xel **rgb_data;         /* pnm format -- see pbmplus docs */
    xelval rgb_maxval;      /* largest value of any pixel index */
    int rgb_format;         /* more info from pnm */

    /*  
     *  now, the YCrCb data.  All pixel information is stored in unsigned
     *  8-bit pieces.  We separate y, cr, and cb because cr and cb are
     *  subsampled by a factor of 2.
     */
    uint8_mpeg_t **orig_y, **orig_cr, **orig_cb;

    /* now, the decoded data -- relevant only if
     *        referenceFrame == DECODED_FRAME
     *
     */
    uint8_mpeg_t **decoded_y, **decoded_cr, **decoded_cb;

    /* reference data */
    uint8_mpeg_t **ref_y, **ref_cr, **ref_cb;

    /*  
     *  these are the Blocks which will ultimately compose MacroBlocks.
     *  A Block is in a format that mp_fwddct() can crunch.
     */
    Block **y_blocks, **cr_blocks, **cb_blocks;

    /*
     *  this is the half-pixel luminance data (for reference frames)
     */
    uint8_mpeg_t **halfX, **halfY, **halfBoth;

    boolean   halfComputed;        /* TRUE iff half-pixels already computed */

    struct mpegFrame *next;  /* points to the next B-frame to be encoded, if
               * stdin is used as the input. 
               */
} MpegFrame;


extern MpegFrame *Frame_New _ANSI_ARGS_((int id, int type));
extern void      Frame_Init _ANSI_ARGS_((void));
extern void      Frame_Free _ANSI_ARGS_((MpegFrame *frame));
extern void      Frame_Exit _ANSI_ARGS_((void));
extern void      Frame_AllocPPM _ANSI_ARGS_((MpegFrame * frame));
extern void      Frame_AllocYCC _ANSI_ARGS_((MpegFrame * mf));
extern void      Frame_AllocDecoded _ANSI_ARGS_((MpegFrame *frame,
                          boolean makeReference));
extern void      Frame_AllocHalf _ANSI_ARGS_((MpegFrame *frame));
extern void      Frame_AllocBlocks _ANSI_ARGS_((MpegFrame *mf));
extern void      Frame_Resize _ANSI_ARGS_((MpegFrame *omf, MpegFrame *mf,
                        int insize_x, int insize_y,
                        int outsize_x, int outsize_y));


#endif /* FRAME_INCLUDED */
