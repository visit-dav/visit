/*===========================================================================*
 * prototypes.h                                     *
 *                                         *
 *    miscellaneous prototypes                         *
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
 *  $Header: /n/picasso/project/mm/mpeg/mpeg_dist/mpeg_encode/headers/RCS/prototypes.h,v 1.9 1995/03/30 01:46:58 smoot Exp smoot $
 *  $Log: prototypes.h,v $
 *  Revision 1.9  1995/03/30 01:46:58  smoot
 *  added SpecType
 *
 * Revision 1.8  1995/03/21  00:27:10  smoot
 * added pnm stuff
 *
 * Revision 1.7  1995/02/18  01:48:27  smoot
 * changed SpecLookup for version 2 specfiles
 *
 * Revision 1.6  1995/01/19  23:55:14  eyhung
 * Changed copyrights
 *
 * Revision 1.5  1995/01/17  21:53:10  smoot
 * Added Specs like
 * Make that: Added specifics prototypes
 *
 * Revision 1.4  1994/12/07  00:42:01  smoot
 * Added seperate P and B search ranges
 *
 * Revision 1.3  1994/11/12  02:12:56  keving
 * nothing
 *
 * Revision 1.2  1993/07/22  22:24:23  keving
 * nothing
 *
 * Revision 1.1  1993/07/09  00:17:23  keving
 * nothing
 *
 */


/*==============*
 * HEADER FILES *
 *==============*/

#include "general.h"
#include "ansi.h"
#include "frame.h"


/*===============================*
 * EXTERNAL PROCEDURE prototypes *
 *===============================*/

int    GetBQScale _ANSI_ARGS_((void));
int    GetPQScale _ANSI_ARGS_((void));
void    ResetBFrameStats _ANSI_ARGS_((void));
void    ResetPFrameStats _ANSI_ARGS_((void));
void    SetSearchRange _ANSI_ARGS_((int pixelsP, int pixelsB));
void    ResetIFrameStats _ANSI_ARGS_((void));
void    SetPixelSearch _ANSI_ARGS_((char *searchType));
void    SetIQScale _ANSI_ARGS_((int qI));
void    SetPQScale _ANSI_ARGS_((int qP));
void    SetBQScale _ANSI_ARGS_((int qB));
float    EstimateSecondsPerIFrame _ANSI_ARGS_((void));
float    EstimateSecondsPerPFrame _ANSI_ARGS_((void));
float    EstimateSecondsPerBFrame _ANSI_ARGS_((void));
void    SetGOPSize _ANSI_ARGS_((int size));
void    SetStatFileName _ANSI_ARGS_((char *fileName));
void    SetSlicesPerFrame _ANSI_ARGS_((int number));
void    SetBlocksPerSlice _ANSI_ARGS_((void));


void DCTFrame _ANSI_ARGS_((MpegFrame * mf));

void PPMtoYCC _ANSI_ARGS_((MpegFrame * mf));

void    MotionSearchPreComputation _ANSI_ARGS_((MpegFrame *frame));
boolean    PMotionSearch _ANSI_ARGS_((LumBlock currentBlock, MpegFrame *prev,
                   int by, int bx, int *motionY, int *motionX));
void    ComputeHalfPixelData _ANSI_ARGS_((MpegFrame *frame));
void mp_validate_size _ANSI_ARGS_((int *x, int *y));

/* block.c */
void    BlockToData _ANSI_ARGS_((uint8_mpeg_t **data, Block block, int by, int bx));
void    AddMotionBlock _ANSI_ARGS_((Block block, uint8_mpeg_t **prev, int by, int bx,
               int my, int mx));
void    AddBMotionBlock _ANSI_ARGS_((Block block, uint8_mpeg_t **prev, uint8_mpeg_t **next,
                     int by, int bx, int mode,
                     int fmy, int fmx, int bmy, int bmx));

void    BlockifyFrame _ANSI_ARGS_((MpegFrame *frame));



extern void    SetFCode _ANSI_ARGS_((void));


/* frametype.c */
int    FType_Type _ANSI_ARGS_((int frameNum));
int    FType_FutureRef _ANSI_ARGS_((int currFrameNum));
int    FType_PastRef _ANSI_ARGS_((int currFrameNum));
void    SetFramePattern _ANSI_ARGS_((char *pattern));
void    ComputeFrameTable _ANSI_ARGS_((void));

/* psearch.c */
void    ShowPMVHistogram _ANSI_ARGS_((FILE *fpointer));
void    ShowBBMVHistogram _ANSI_ARGS_((FILE *fpointer));
void    ShowBFMVHistogram _ANSI_ARGS_((FILE *fpointer));

/* specifics.c */
void    Specifics_Init _ANSI_ARGS_((void));
int     SpecLookup _ANSI_ARGS_((int fn, int typ, int num, 
                BlockMV **info, int start_qs));
int SpecTypeLookup _ANSI_ARGS_((int fn));

/* libpnm.c */

/*
 * Commented out by Brad Whitlock, Wed Jul 30 10:16:03 PDT 2003 so it builds 
 * on Tru64.
 *
void pnm_promoteformat _ANSI_ARGS_(( xel** xels, int cols, int rows, xelval maxval,
int format, xelval newmaxval, int newformat ));
void pnm_promoteformatrow _ANSI_ARGS_(( xel* xelrow, int cols, xelval maxval, 
int format, xelval newmaxval, int newformat ));
*/
