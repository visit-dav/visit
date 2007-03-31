/*===========================================================================*
 * mproto.h                                     *
 *                                         *
 *    basically a lot of miscellaneous prototypes                 *
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
 *  $Header: /n/picasso/project/mm/mpeg/mpeg_dist/mpeg_encode/headers/RCS/mproto.h,v 1.12 1995/03/29 20:14:29 smoot Exp $
 *  $Log: mproto.h,v $
 * Revision 1.12  1995/03/29  20:14:29  smoot
 * deleted unneeded dct prototype
 *
 * Revision 1.11  1995/01/19  23:55:02  eyhung
 * Changed copyrights
 *
 * Revision 1.10  1995/01/16  06:20:10  eyhung
 * Changed ReadYUV to ReadEYUV
 *
 * Revision 1.9  1993/07/22  22:24:23  keving
 * nothing
 *
 * Revision 1.8  1993/07/09  00:17:23  keving
 * nothing
 *
 * Revision 1.7  1993/06/03  21:08:53  keving
 * nothing
 *
 * Revision 1.6  1993/02/24  19:13:33  keving
 * nothing
 *
 * Revision 1.5  1993/02/17  23:18:20  dwallach
 * checkin prior to keving's joining the project
 *
 * Revision 1.4  1993/01/18  10:20:02  dwallach
 * *** empty log message ***
 *
 * Revision 1.3  1993/01/18  10:17:29  dwallach
 * RCS headers installed, code indented uniformly
 *
 * Revision 1.3  1993/01/18  10:17:29  dwallach
 * RCS headers installed, code indented uniformly
 *
 */


/*==============*
 * HEADER FILES *
 *==============*/

#include "general.h"
#include "ansi.h"
#include "bitio.h"


#define DCTSIZE2    DCTSIZE*DCTSIZE
typedef short DCTELEM;
typedef DCTELEM DCTBLOCK[DCTSIZE2];



/*===============================*
 * EXTERNAL PROCEDURE prototypes *
 *===============================*/

/*  
 *  from mbasic.c:
 */
void mp_reset _ANSI_ARGS_((void));
void mp_free _ANSI_ARGS_((MpegFrame *mf));
MpegFrame *mp_new _ANSI_ARGS_((int fnumber, char type, MpegFrame *oldFrame));
void mp_ycc_calc _ANSI_ARGS_((MpegFrame *mf));
void mp_dct_blocks _ANSI_ARGS_((MpegFrame *mf));
void    AllocDecoded _ANSI_ARGS_((MpegFrame *frame));

/*  
 *  from moutput.c:
 */
boolean mp_quant_zig_block _ANSI_ARGS_((Block in, FlatBlock out, int qscale, int iblock));
void    UnQuantZig _ANSI_ARGS_((FlatBlock in, Block out, int qscale, boolean iblock));
void mp_rle_huff_block _ANSI_ARGS_((FlatBlock in, BitBucket *out));
void mp_rle_huff_pblock _ANSI_ARGS_((FlatBlock in, BitBucket *out));
void mp_create_blocks _ANSI_ARGS_((MpegFrame *mf));




void    ReadEYUV _ANSI_ARGS_((MpegFrame * mf, FILE *fpointer, int width,
                int height));
boolean    ReadPPM _ANSI_ARGS_((MpegFrame *mf, FILE *fpointer));
void PPMtoYCC _ANSI_ARGS_((MpegFrame * mf));

void    MotionSearchPreComputation _ANSI_ARGS_((MpegFrame *frame));
boolean    PMotionSearch _ANSI_ARGS_((LumBlock currentBlock, MpegFrame *prev,
                   int by, int bx, int *motionY, int *motionX));
void    ComputeHalfPixelData _ANSI_ARGS_((MpegFrame *frame));
void mp_validate_size _ANSI_ARGS_((int *x, int *y));
void AllocYCC _ANSI_ARGS_((MpegFrame * mf));


/* jrevdct.c */
void init_pre_idct _ANSI_ARGS_((void ));
void j_rev_dct_sparse _ANSI_ARGS_((DCTBLOCK data , int pos ));
void j_rev_dct _ANSI_ARGS_((DCTBLOCK data ));
void j_rev_dct_sparse _ANSI_ARGS_((DCTBLOCK data , int pos ));
void j_rev_dct _ANSI_ARGS_((DCTBLOCK data ));

/* block.c */
void    BlockToData _ANSI_ARGS_((uint8_mpeg_t **data, Block block, int by, int bx));
void    AddMotionBlock _ANSI_ARGS_((Block block, uint8_mpeg_t **prev, int by, int bx,
               int my, int mx));
