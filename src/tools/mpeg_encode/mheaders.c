/*===========================================================================*
 * mheaders.c                                     *
 *                                         *
 *    Procedures to generate MPEG headers                     *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    Mhead_GenPictureHeader                             *
 *    Mhead_GenSequenceHeader                             *
 *    Mhead_GenSequenceEnder                             *
 *    Mhead_GenGOPHeader                             *
 *    Mhead_GenSliceHeader                             *
 *    Mhead_GenSliceEnder                             *
 *    Mhead_GenMBHeader                             *
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
 *  $Header: /n/picasso/project/mpeg/mpeg_dist/mpeg_encode/RCS/mheaders.c,v 1.15 1995/08/07 21:45:19 smoot Exp $
 *  $Log: mheaders.c,v $
 *  Revision 1.15  1995/08/07 21:45:19  smoot
 *  check for illegal MVs (shouldnt ever be called, but....)
 *  fix bug which made us not weite Iframe Qscale changes
 *  warns if writing a size=0 mpeg
 *
 *  Revision 1.14  1995/05/22 20:53:35  smoot
 *  corrected bit_rate value in constrained params flag
 *
 * Revision 1.13  1995/05/02  01:50:38  eyhung
 * made VidRateNum un-static
 *
 * Revision 1.12  1995/03/27  19:28:23  smoot
 * auto-determines Qscale changes (was mb_quant)
 *
 * Revision 1.11  1995/02/16  09:12:39  eyhung
 * fixed compile bug with HP7xx
 *
 * Revision 1.10  1995/01/25  22:53:50  smoot
 * Better buf_size checking, and actually check constrained params
 *
 * Revision 1.9  1995/01/19  23:08:47  eyhung
 * Changed copyrights
 *
 * Revision 1.8  1995/01/16  08:45:10  eyhung
 * BLEAH'ed hsize and vsize
 *
 * Revision 1.7  1994/12/09  22:27:17  smoot
 * Fixed buffer size in stream
 *
 * Revision 1.6  1994/11/12  02:11:54  keving
 * nothing
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
 * Revision 1.6  1993/03/01  23:03:40  keving
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

#include "all.h"
#include "bitio.h"
#include "frames.h"
#include "mheaders.h"


/*==================*
 * STATIC VARIABLES *
 *==================*/

static int gopStartFrame = 0;
static int lastGOPStart = 0;
static int lastQSSet;

static uint32_mpeg_t mbAddrIncrTable[][2] = {
    {0x0, 0},
    {0x1, 1},
    {0x3, 3},
    {0x2, 3},
    {0x3, 4},
    {0x2, 4},
    {0x3, 5},
    {0x2, 5},
    {0x7, 7},
    {0x6, 7},
    {0xb, 8},
    {0xa, 8},
    {0x9, 8},
    {0x8, 8},
    {0x7, 8},
    {0x6, 8},
    {0x17, 10},
    {0x16, 10},
    {0x15, 10},
    {0x14, 10},
    {0x13, 10},
    {0x12, 10},
    {0x23, 11},
    {0x22, 11},
    {0x21, 11},
    {0x20, 11},
    {0x1f, 11},
    {0x1e, 11},
    {0x1d, 11},
    {0x1c, 11},
    {0x1b, 11},
    {0x1a, 11},
    {0x19, 11},
    {0x18, 11}};

static uint32_mpeg_t mbMotionVectorTable[][2] = {
    {0x19, 11},
    {0x1b, 11},
    {0x1d, 11},
    {0x1f, 11},
    {0x21, 11},
    {0x23, 11},
    {0x13, 10},
    {0x15, 10},
    {0x17, 10},
    {0x7, 8},
    {0x9, 8},
    {0xb, 8},
    {0x7, 7},
    {0x3, 5},
    {0x3, 4},
    {0x3, 3},
    {0x1, 1},
    {0x2, 3},
    {0x2, 4},
    {0x2, 5},
    {0x6, 7},
    {0xa, 8},
    {0x8, 8},
    {0x6, 8},
    {0x16, 10},
    {0x14, 10},
    {0x12, 10},
    {0x22, 11},
    {0x20, 11},
    {0x1e, 11},
    {0x1c, 11},
    {0x1a, 11},
    {0x18, 11}};

static uint32_mpeg_t mbPatTable[][2] = {
    {0x0, 0},
    {0xb, 5},
    {0x9, 5},
    {0xd, 6},
    {0xd, 4},
    {0x17, 7},
    {0x13, 7},
    {0x1f, 8},
    {0xc, 4},
    {0x16, 7},
    {0x12, 7},
    {0x1e, 8},
    {0x13, 5},
    {0x1b, 8},
    {0x17, 8},
    {0x13, 8},
    {0xb, 4},
    {0x15, 7},
    {0x11, 7},
    {0x1d, 8},
    {0x11, 5},
    {0x19, 8},
    {0x15, 8},
    {0x11, 8},
    {0xf, 6},
    {0xf, 8},
    {0xd, 8},
    {0x3, 9},
    {0xf, 5},
    {0xb, 8},
    {0x7, 8},
    {0x7, 9},
    {0xa, 4},
    {0x14, 7},
    {0x10, 7},
    {0x1c, 8},
    {0xe, 6},
    {0xe, 8},
    {0xc, 8},
    {0x2, 9},
    {0x10, 5},
    {0x18, 8},
    {0x14, 8},
    {0x10, 8},
    {0xe, 5},
    {0xa, 8},
    {0x6, 8},
    {0x6, 9},
    {0x12, 5},
    {0x1a, 8},
    {0x16, 8},
    {0x12, 8},
    {0xd, 5},
    {0x9, 8},
    {0x5, 8},
    {0x5, 9},
    {0xc, 5},
    {0x8, 8},
    {0x4, 8},
    {0x4, 9},
    {0x7, 3},
    {0xa, 5},    /* grrr... 61, 62, 63 added - Kevin */
    {0x8, 5},
    {0xc, 6}
};

/*===========*
 * CONSTANTS *
 *===========*/

#define SEQ_HEAD_CODE 0x000001b3
#define EXT_START_CODE 0x000001b5
#define USER_START_CODE 0x000001b2
#define GOP_START_CODE 0x000001b8
#define PICT_START_CODE 0x00000100
#define SLICE_BASE_CODE 0x00000100

#define SEQ_END_CODE    0x000001b7

/* not static anymore because information is used for computing frame rate 
 * and for statistics */
const double VidRateNum[9]={1.0, 23.976, 24.0, 25.0, 29.97, 30.0,
                             50.0 ,59.94, 60.0};


/*===============================*
 * INTERNAL PROCEDURE prototypes *
 *===============================*/

static void    GenMBAddrIncr _ANSI_ARGS_((BitBucket *bb, uint32_mpeg_t addr_incr));
static void    GenPictHead _ANSI_ARGS_((BitBucket *bb, uint32_mpeg_t temp_ref,
            uint32_mpeg_t code_type, uint32_mpeg_t vbv_delay,
            int32_mpeg_t full_pel_forw_flag, uint32_mpeg_t forw_f_code,
            int32_mpeg_t full_pel_back_flag, uint32_mpeg_t back_f_code,
            uint8_mpeg_t *extra_info, uint32_mpeg_t extra_info_size,
            uint8_mpeg_t *ext_data, uint32_mpeg_t ext_data_size,
            uint8_mpeg_t *user_data, uint32_mpeg_t user_data_size));
static void    GenMBType _ANSI_ARGS_((BitBucket *bb, uint32_mpeg_t pict_code_type,
          uint32_mpeg_t mb_quant, uint32_mpeg_t motion_forw, uint32_mpeg_t motion_back,
          uint32_mpeg_t mb_pattern, uint32_mpeg_t mb_intra));
static void    GenMotionCode _ANSI_ARGS_((BitBucket *bb, int32_mpeg_t vector));
static void    GenBlockPattern _ANSI_ARGS_((BitBucket *bb,
                         uint32_mpeg_t mb_pattern));


/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/


/*===========================================================================*
 *
 * SetGOPStartTime
 *
 *    sets the start frame of the GOP; to be used with GenPictureHeader
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
SetGOPStartTime(index)
    int index;
{
    lastGOPStart = gopStartFrame;
    gopStartFrame = index;
}


/*===========================================================================*
 *
 * Mhead_GenPictureHeader
 *
 *    generate picture header with given frame type and picture count
 *    append result to the specified bitstream
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Mhead_GenPictureHeader(bbPtr, frameType, pictCount, f_code)
    BitBucket *bbPtr;
    int frameType;
    int pictCount;
    int f_code;
{
    int        temporalRef;

    if ( pictCount >= gopStartFrame ) {
    temporalRef = (pictCount-gopStartFrame);
    } else {
    temporalRef = (pictCount-lastGOPStart);
    }
    temporalRef = (temporalRef % 1024);
    
    DBG_PRINT(("Picture Header\n"));
    GenPictHead(bbPtr, temporalRef, frameType,
        0 /* vbv_delay */,
        pixelFullSearch /* full_pel_forw_flag */,
        f_code /* forw_f_code */,
        pixelFullSearch /* full_pel_back_flag */,
        f_code /* back_f_code */,
        NULL, 0, NULL, 0, NULL, 0);
}


/*===========================================================================*
 *
 * Mhead_GenSequenceHeader
 *
 *    generate sequence header with given attributes
 *    append result to the specified bitstream
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Mhead_GenSequenceHeader(bbPtr, hsize, vsize, pratio, pict_rate, bit_rate,
            buf_size, c_param_flag, iq_matrix, niq_matrix,
            ext_data, ext_data_size, user_data, user_data_size)
    BitBucket *bbPtr;
    uint32_mpeg_t hsize;
    uint32_mpeg_t vsize;
    int32_mpeg_t pratio;
    int32_mpeg_t pict_rate;
    int32_mpeg_t bit_rate;
    int32_mpeg_t buf_size;
    int32_mpeg_t c_param_flag;
    int32_mpeg_t *iq_matrix;
    int32_mpeg_t *niq_matrix;
    uint8_mpeg_t *ext_data;
    int32_mpeg_t ext_data_size;
    uint8_mpeg_t *user_data;
    int32_mpeg_t user_data_size;
{
    extern int ZAG[];
    int i;

    /* Write seq start code. */

    Bitio_Write(bbPtr, SEQ_HEAD_CODE, 32);

    /* Write horiz. and vert. sizes. */

#ifdef BLEAH
fprintf(stdout, "hsize, vsize = %d, %d\n", hsize, vsize);
#endif

    if (hsize==0 || vsize==0) {
      fprintf(stderr, "Writing zero size to stream!\n");
    }
    Bitio_Write(bbPtr, hsize, 12);
    Bitio_Write(bbPtr, vsize, 12);

    /* Write pixel aspect ratio, negative values default to 1. */

    if (pratio < 0) {
    fprintf(stderr, "PROGRAMMER ERROR:  pratio = %d\n", pratio);
    exit(1);
    }
    Bitio_Write(bbPtr, pratio, 4);

    /* Wrtie picture rate, negative values default to 30 fps. */

    if (pict_rate < 0) {
    fprintf(stderr, "PROGRAMMER ERROR:  pict_rate = %d\n", pict_rate);
    exit(1);
    }
    Bitio_Write(bbPtr, pict_rate, 4);

    /* Write bit rate, negative values default to variable. */

    if (bit_rate < 0) {
    bit_rate = -1;
    } else {
    bit_rate = bit_rate / 400;
    }

    Bitio_Write(bbPtr, bit_rate, 18);

    /* Marker bit. */
    Bitio_Write(bbPtr, 0x1, 1);

    /* Write VBV buffer size. Negative values default to zero. */
    if (buf_size < 0) {
    buf_size = 0;
    }

    buf_size = (buf_size + (16*1024 - 1)) / (16*1024);
    if (buf_size>=0x400) buf_size=0x3ff;
    Bitio_Write(bbPtr, buf_size, 10);

    /* Write constrained parameter flag. */
    {
      int num_mb = ((hsize+15)/16) * ((vsize+15)/16);
      /* At present we cheat on buffer size */
      c_param_flag = ((bit_rate <= 4640) &&
                    (bit_rate >0) &&
                    (buf_size <= 20) &&
                    (pict_rate >= 1) &&
                    (pict_rate <= 5) &&
                    (hsize <= 768) &&
                    (vsize <= 576) &&
                    (num_mb <= 396) &&
                    (num_mb*VidRateNum[pict_rate] <= 9900) &&
                    (fCodeP<=4) &&
                    (fCodeB<=4));
    }

    if (c_param_flag) {
    Bitio_Write(bbPtr, 0x01, 1);
    } else {
    Bitio_Write(bbPtr, 0x00, 1);
    }

    /* Write intra quant matrix if present. */

    if (iq_matrix != NULL) {
    Bitio_Write(bbPtr, 0x01, 1);
    for (i = 0; i < 64; i++) {
        Bitio_Write(bbPtr, iq_matrix[ZAG[i]], 8);
    }
    } else {
    Bitio_Write(bbPtr, 0x00, 1);
    }

    /* Write non intra quant matrix if present. */

    if (niq_matrix != NULL) {
    Bitio_Write(bbPtr, 0x01, 1);
    for (i = 0; i < 64; i++) {
        Bitio_Write(bbPtr, niq_matrix[ZAG[i]], 8);
    }
    } else {
    Bitio_Write(bbPtr, 0x00, 1);
    }

    /* next start code */
    Bitio_BytePad(bbPtr);


    /* Write ext data if present. */

    if (ext_data != NULL) {
    Bitio_Write(bbPtr, EXT_START_CODE, 32);

    for (i = 0; i < ext_data_size; i++) {
        Bitio_Write(bbPtr, ext_data[i], 8);
    }
    Bitio_BytePad(bbPtr);
    }
    /* Write user data if present. */
    if ((user_data != NULL) && (user_data_size != 0)) {
    Bitio_Write(bbPtr, USER_START_CODE, 32);

    for (i = 0; i < user_data_size; i++) {
        Bitio_Write(bbPtr, user_data[i], 8);
    }
    Bitio_BytePad(bbPtr);
    }
}


/*===========================================================================*
 *
 * Mhead_GenSequenceEnder
 *
 *    generate sequence ender
 *    append result to the specified bitstream
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Mhead_GenSequenceEnder(bbPtr)
    BitBucket *bbPtr;
{
    Bitio_Write(bbPtr, SEQ_END_CODE, 32);
}


/*===========================================================================*
 *
 * Mhead_GenGOPHeader
 *
 *    generate GOP header with specified attributes
 *    append result to the specified bitstream
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Mhead_GenGOPHeader(bbPtr, drop_frame_flag, tc_hrs, tc_min, tc_sec, tc_pict,
           closed_gop, broken_link, ext_data, ext_data_size,
           user_data, user_data_size)
    BitBucket *bbPtr;
    int32_mpeg_t drop_frame_flag;
    int32_mpeg_t tc_hrs;
    int32_mpeg_t tc_min;
    int32_mpeg_t tc_sec;
    int32_mpeg_t tc_pict;
    int32_mpeg_t closed_gop;
    int32_mpeg_t broken_link;
    uint8_mpeg_t *ext_data;
    int32_mpeg_t ext_data_size;
    uint8_mpeg_t *user_data;
    int32_mpeg_t user_data_size;
{
    int i;

    /* Write gop start code. */
    Bitio_Write(bbPtr, GOP_START_CODE, 32);

        /* Construct and write timecode. */

    /* Drop frame flag. */
    if (drop_frame_flag) {
    Bitio_Write(bbPtr, 0x01, 1);
    } else {
    Bitio_Write(bbPtr, 0x00, 1);
    }

    /* Time code hours. */
    Bitio_Write(bbPtr, tc_hrs, 5);

    /* Time code minutes. */
    Bitio_Write(bbPtr, tc_min, 6);

    /* Marker bit. */
    Bitio_Write(bbPtr, 0x01, 1);

    /* Time code seconds. */
    Bitio_Write(bbPtr, tc_sec, 6);

    /* Time code pictures. */
    Bitio_Write(bbPtr, tc_pict, 6);


    /* Closed gop flag. */
    if (closed_gop) {
    Bitio_Write(bbPtr, 0x01, 1);
    } else {
    Bitio_Write(bbPtr, 0x00, 1);
    }

    /* Broken link flag. */
    if (broken_link) {
    Bitio_Write(bbPtr, 0x01, 1);
    } else {
    Bitio_Write(bbPtr, 0x00, 1);
    }

    /* next start code */
    Bitio_BytePad(bbPtr);

    /* Write ext data if present. */

    if (ext_data != NULL) {
    Bitio_Write(bbPtr, EXT_START_CODE, 32);

    for (i = 0; i < ext_data_size; i++) {
        Bitio_Write(bbPtr, ext_data[i], 8);
    }
    Bitio_BytePad(bbPtr);
    }
    /* Write user data if present. */
    if (user_data != NULL) {
    Bitio_Write(bbPtr, USER_START_CODE, 32);

    for (i = 0; i < user_data_size; i++) {
        Bitio_Write(bbPtr, user_data[i], 8);
    }
    Bitio_BytePad(bbPtr);
    }
}


/*===========================================================================*
 *
 * Mhead_GenSliceHeader
 *
 *    generate slice header with specified attributes
 *    append result to the specified bitstream
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Mhead_GenSliceHeader(bbPtr, verticalPos, qscale, extra_info, extra_info_size)
    BitBucket *bbPtr;
    uint32_mpeg_t verticalPos;
    uint32_mpeg_t qscale;
    uint8_mpeg_t *extra_info;
    uint32_mpeg_t extra_info_size;
{
    int i;

    /* Write slice start code. */
    Bitio_Write(bbPtr, (SLICE_BASE_CODE + verticalPos), 32);

    /* Quant. scale. */
    Bitio_Write(bbPtr, qscale, 5);
    lastQSSet = qscale;

    /* Extra bit slice info. */

    if (extra_info != NULL) {
    for (i = 0; i < extra_info_size; i++) {
        Bitio_Write(bbPtr, 0x01, 1);
        Bitio_Write(bbPtr, extra_info[i], 8);
    }
    }

    /* extra_bit_slice */
    Bitio_Write(bbPtr, 0x00, 1);
}


/*===========================================================================*
 *
 * Mhead_GenSliceEnder
 *
 *    generate slice ender
 *    append result to the specified bitstream
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Mhead_GenSliceEnder(bbPtr)
    BitBucket *bbPtr;
{
    Bitio_BytePad(bbPtr);
}


/*===========================================================================*
 *
 * Mhead_GenMBHeader
 *
 *    generate macroblock header with given attributes
 *    append result to the specified bitstream
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Mhead_GenMBHeader(bbPtr, pict_code_type, addr_incr, q_scale,
          forw_f_code, back_f_code, horiz_forw_r, vert_forw_r,
          horiz_back_r, vert_back_r, motion_forw, m_horiz_forw,
          m_vert_forw, motion_back, m_horiz_back, m_vert_back,
          mb_pattern, mb_intra)
    BitBucket *bbPtr;
    uint32_mpeg_t pict_code_type;
    uint32_mpeg_t addr_incr;
    uint32_mpeg_t q_scale;
    uint32_mpeg_t forw_f_code;
    uint32_mpeg_t back_f_code;
    uint32_mpeg_t horiz_forw_r;
    uint32_mpeg_t vert_forw_r;
    uint32_mpeg_t horiz_back_r;
    uint32_mpeg_t vert_back_r;
    int32_mpeg_t motion_forw;
    int32_mpeg_t m_horiz_forw;
    int32_mpeg_t m_vert_forw;
    int32_mpeg_t motion_back;
    int32_mpeg_t m_horiz_back;
    int32_mpeg_t m_vert_back;
    uint32_mpeg_t mb_pattern;
    uint32_mpeg_t mb_intra;
{
    uint32_mpeg_t mb_quant;

    /* MB escape sequences if necessary. */

#ifdef BLEAH
if ( addr_incr != 1 )
    fprintf(stdout, "Creating MB_INCR:  %d\n", addr_incr);
#endif

    while (addr_incr > 33) {
    Bitio_Write(bbPtr, 0x008, 11);
    addr_incr -= 33;
    }

    /* Generate addr incr code. */
    GenMBAddrIncr(bbPtr, addr_incr);

    /* Determine mb_quant  (true if change in q scale) */
    if ((q_scale != lastQSSet) && ((mb_pattern != 0) || (mb_intra == TRUE))) {
      mb_quant = TRUE;
      lastQSSet = q_scale;
    } else {
      mb_quant = FALSE;
    }

    /* Generate mb type code. */
    GenMBType(bbPtr, pict_code_type, mb_quant, motion_forw, motion_back, mb_pattern, mb_intra);

    /* MB quant. */
    if (mb_quant) {
    Bitio_Write(bbPtr, q_scale, 5);
    }
    /* Forward predictive vector stuff. */

    if (motion_forw) {
    int forw_f, forw_r_size;

    forw_r_size = forw_f_code - 1;
    forw_f = 1 << forw_r_size;    /* 1 > 0 */
    if ((m_horiz_forw > 16*forw_f-1) || (m_horiz_forw < -16*forw_f)) {
      fprintf(stderr, "Illegal motion? %d %d\n", m_horiz_forw, 16*forw_f);
    }
    if ((m_vert_forw > 16*forw_f-1) || (m_vert_forw < -16*forw_f)) {
      fprintf(stderr, "Illegal motion? %d %d\n", m_vert_forw, 16*forw_f);
    }
    GenMotionCode(bbPtr, m_horiz_forw);

    if ((forw_f != 1) && (m_horiz_forw != 0)) {
        Bitio_Write(bbPtr, horiz_forw_r, forw_r_size);
    }
    GenMotionCode(bbPtr, m_vert_forw);

    if ((forw_f != 1) && (m_vert_forw != 0)) {
        Bitio_Write(bbPtr, vert_forw_r, forw_r_size);
    }
    }
    /* Back predicted vector stuff. */

    if (motion_back) {
    int back_f, back_r_size;

    back_r_size = back_f_code - 1;
    back_f = 1 << back_r_size;    /* 1 > 0 */

    if ((m_horiz_back > 16*back_f-1) || (m_horiz_back < -16*back_f)) {
      fprintf(stderr, "Illegal motion? %d %d\n", m_horiz_back, 16*back_f);
    }
    if ((m_vert_back > 16*back_f-1) || (m_vert_back < -16*back_f)) {
      fprintf(stderr, "Illegal motion? %d %d\n", m_vert_back, 16*back_f);
    }

    GenMotionCode(bbPtr, m_horiz_back);

    if ((back_f != 1) && (m_horiz_back != 0)) {
        Bitio_Write(bbPtr, horiz_back_r, back_r_size);
    }
    GenMotionCode(bbPtr, m_vert_back);

    if ((back_f != 1) && (m_vert_back != 0)) {
        Bitio_Write(bbPtr, vert_back_r, back_r_size);
    }
    }
    /* MB pattern. */

    if (mb_pattern) {
    GenBlockPattern(bbPtr, mb_pattern);
    }
}


/*=====================*
 * INTERNAL PROCEDURES *
 *=====================*/

/*===========================================================================*
 *
 * GenMBType
 *
 *    generate macroblock type with given attributes
 *    append result to the specified bitstream
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
GenMBType(bbPtr, pict_code_type, mb_quant, motion_forw, motion_back,
      mb_pattern, mb_intra)
    BitBucket *bbPtr;
    uint32_mpeg_t pict_code_type;
    uint32_mpeg_t mb_quant;
    uint32_mpeg_t motion_forw;
    uint32_mpeg_t motion_back;
    uint32_mpeg_t mb_pattern;
    uint32_mpeg_t mb_intra;
{
    int code;

    switch (pict_code_type) {
    case 1:
    if ((motion_forw != 0) || (motion_back != 0) || (mb_pattern != 0) || (mb_intra != 1)) {
        perror("Illegal parameters for macroblock type.");
        exit(-1);
    }
    if (mb_quant) {
        Bitio_Write(bbPtr, 0x1, 2);
    } else {
        Bitio_Write(bbPtr, 0x1, 1);
    }
    break;

    case 2:
    code = 0;
    if (mb_quant) {
        code += 16;
    }
    if (motion_forw) {
        code += 8;
    }
    if (motion_back) {
        code += 4;
    }
    if (mb_pattern) {
        code += 2;
    }
    if (mb_intra) {
        code += 1;
    }

    switch (code) {
    case 1:
        Bitio_Write(bbPtr, 0x3, 5);
        break;
    case 2:
        Bitio_Write(bbPtr, 0x1, 2);
        break;
    case 8:
        Bitio_Write(bbPtr, 0x1, 3);
        break;
    case 10:
        Bitio_Write(bbPtr, 0x1, 1);
        break;
    case 17:
        Bitio_Write(bbPtr, 0x1, 6);
        break;
    case 18:
        Bitio_Write(bbPtr, 0x1, 5);
        break;
    case 26:
        Bitio_Write(bbPtr, 0x2, 5);
        break;
    default:
        perror("Illegal parameters for macroblock type.");
        exit(-1);
        break;
    }
    break;

    case 3:
    code = 0;
    if (mb_quant) {
        code += 16;
    }
    if (motion_forw) {
        code += 8;
    }
    if (motion_back) {
        code += 4;
    }
    if (mb_pattern) {
        code += 2;
    }
    if (mb_intra) {
        code += 1;
    }

    switch (code) {
    case 12:
        Bitio_Write(bbPtr, 0x2, 2);
        break;
    case 14:
        Bitio_Write(bbPtr, 0x3, 2);
        break;
    case 4:
        Bitio_Write(bbPtr, 0x2, 3);
        break;
    case 6:
        Bitio_Write(bbPtr, 0x3, 3);
        break;
    case 8:
        Bitio_Write(bbPtr, 0x2, 4);
        break;
    case 10:
        Bitio_Write(bbPtr, 0x3, 4);
        break;
    case 1:
        Bitio_Write(bbPtr, 0x3, 5);
        break;
    case 30:
        Bitio_Write(bbPtr, 0x2, 5);
        break;
    case 26:
        Bitio_Write(bbPtr, 0x3, 6);
        break;
    case 22:
        Bitio_Write(bbPtr, 0x2, 6);
        break;
    case 17:
        Bitio_Write(bbPtr, 0x1, 6);
        break;
    default:
        perror("Illegal parameters for macroblock type.");
        exit(-1);
        break;
    }
    break;
    }
}


/*===========================================================================*
 *
 * GenMotionCode
 *
 *    generate motion vector output with given value
 *    append result to the specified bitstream
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
GenMotionCode(bbPtr, vector)
    BitBucket *bbPtr;
    int32_mpeg_t vector;
{
    uint32_mpeg_t code, num;

    if ((vector < -16) || (vector > 16)) {
    perror("Motion vector out of range.");
    fprintf(stderr, "Motion vector out of range:  vector = %d\n", vector);
    exit(-1);
    }
    code = mbMotionVectorTable[vector + 16][0];
    num = mbMotionVectorTable[vector + 16][1];

    Bitio_Write(bbPtr, code, num);
}


/*===========================================================================*
 *
 * GenBlockPattern
 *
 *    generate macroblock pattern output
 *    append result to the specified bitstream
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
GenBlockPattern(bbPtr, mb_pattern)
    BitBucket *bbPtr;
    uint32_mpeg_t mb_pattern;
{
    uint32_mpeg_t code, num;

    code = mbPatTable[mb_pattern][0];
    num = mbPatTable[mb_pattern][1];

    Bitio_Write(bbPtr, code, num);
}


/*===========================================================================*
 *
 * GenMBAddrIncr
 *
 *    generate macroblock address increment output
 *    append result to the specified bitstream
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
GenMBAddrIncr(bbPtr, addr_incr)
    BitBucket *bbPtr;
    uint32_mpeg_t addr_incr;
{
    uint32_mpeg_t code;
    uint32_mpeg_t num;

    code = mbAddrIncrTable[addr_incr][0];
    num = mbAddrIncrTable[addr_incr][1];

    Bitio_Write(bbPtr, code, num);
}


/*===========================================================================*
 *
 * GenPictHead
 *
 *    generate picture header with given attributes
 *    append result to the specified bitstream
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
GenPictHead(bbPtr, temp_ref, code_type, vbv_delay, full_pel_forw_flag,
        forw_f_code, full_pel_back_flag, back_f_code, extra_info,
        extra_info_size, ext_data, ext_data_size, user_data,
        user_data_size)
    BitBucket *bbPtr;
    uint32_mpeg_t temp_ref;
    uint32_mpeg_t code_type;
    uint32_mpeg_t vbv_delay;
    int32_mpeg_t full_pel_forw_flag;
    uint32_mpeg_t forw_f_code;
    int32_mpeg_t full_pel_back_flag;
    uint32_mpeg_t back_f_code;
    uint8_mpeg_t *extra_info;
    uint32_mpeg_t extra_info_size;
    uint8_mpeg_t *ext_data;
    uint32_mpeg_t ext_data_size;
    uint8_mpeg_t *user_data;
    uint32_mpeg_t user_data_size;
{
    int i;

    /* Write picture start code. */
    Bitio_Write(bbPtr, PICT_START_CODE, 32);

    /* Temp reference. */
    Bitio_Write(bbPtr, temp_ref, 10);

    /* Code_type. */
    if (code_type == 0) {
    code_type = 1;
    }
    Bitio_Write(bbPtr, code_type, 3);

    /* vbv_delay. */
    vbv_delay = 0xffff;            /* see page 36 (section 2.4.3.4) */
    Bitio_Write(bbPtr, vbv_delay, 16);

    if ((code_type == 2) || (code_type == 3)) {

    /* Full pel forw flag. */

    if (full_pel_forw_flag) {
        Bitio_Write(bbPtr, 0x01, 1);
    } else {
        Bitio_Write(bbPtr, 0x00, 1);
    }

    /* Forw f code. */

    Bitio_Write(bbPtr, forw_f_code, 3);
    }
    if (code_type == 3) {

    /* Full pel back flag. */

    if (full_pel_back_flag) {
        Bitio_Write(bbPtr, 0x01, 1);
    } else {
        Bitio_Write(bbPtr, 0x00, 1);
    }

    /* Back f code. */

    Bitio_Write(bbPtr, back_f_code, 3);
    }
    /* Extra bit picture info. */

    if (extra_info != NULL) {
    for (i = 0; i < extra_info_size; i++) {
        Bitio_Write(bbPtr, 0x01, 1);
        Bitio_Write(bbPtr, extra_info[i], 8);
    }
    }
    Bitio_Write(bbPtr, 0x00, 1);

    /* next start code */
    Bitio_BytePad(bbPtr);

    /* Write ext data if present. */

    if (ext_data != NULL) {
    Bitio_Write(bbPtr, EXT_START_CODE, 32);

    for (i = 0; i < ext_data_size; i++) {
        Bitio_Write(bbPtr, ext_data[i], 8);
    }
    Bitio_BytePad(bbPtr);
    }
    /* Write user data if present. */
    if (user_data != NULL) {
    Bitio_Write(bbPtr, USER_START_CODE, 32);

    for (i = 0; i < user_data_size; i++) {
        Bitio_Write(bbPtr, user_data[i], 8);
    }
    Bitio_BytePad(bbPtr);
    }
}


#ifdef UNUSED_PROCEDURES

/* GenMBEnd only used for `D` pictures. Shouldn't really ever be called. */
/* - dwallach */
void
GenMBEnd(bbPtr)
    BitBucket *bbPtr;
{
    Bitio_Write(bbPtr, 0x01, 1);
}

#endif /* UNUSED_PROCEDURES */
