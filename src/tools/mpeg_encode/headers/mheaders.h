/*===========================================================================*
 * mheaders.h                                     *
 *                                         *
 *    MPEG headers                                 *
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
 *  $Header: /n/picasso/project/mm/mpeg/mpeg_dist/mpeg_encode/headers/RCS/mheaders.h,v 1.4 1995/03/27 19:29:24 smoot Exp $
 *  $Log: mheaders.h,v $
 * Revision 1.4  1995/03/27  19:29:24  smoot
 * changed to remove mb_quant
 *
 * Revision 1.3  1995/01/19  23:54:56  eyhung
 * Changed copyrights
 *
 * Revision 1.2  1994/11/12  02:12:51  keving
 * nothing
 *
 * Revision 1.1  1993/07/22  22:24:23  keving
 * nothing
 *
 *
 */


#ifndef MHEADERS_INCLUDED
#define MHEADERS_INCLUDED


/*==============*
 * HEADER FILES *
 *==============*/

#include "general.h"
#include "ansi.h"
#include "bitio.h"


/*===============================*
 * EXTERNAL PROCEDURE prototypes *
 *===============================*/

void    SetGOPStartTime _ANSI_ARGS_((int index));
void    Mhead_GenSequenceHeader _ANSI_ARGS_((BitBucket *bbPtr,
            uint32_mpeg_t hsize, uint32_mpeg_t vsize,
            int32_mpeg_t pratio, int32_mpeg_t pict_rate,
            int32_mpeg_t bit_rate, int32_mpeg_t buf_size,
            int32_mpeg_t c_param_flag, int32_mpeg_t *iq_matrix,
            int32_mpeg_t *niq_matrix, uint8_mpeg_t *ext_data,
            int32_mpeg_t ext_data_size, uint8_mpeg_t *user_data, int32_mpeg_t user_data_size));
void    Mhead_GenSequenceEnder _ANSI_ARGS_((BitBucket *bbPtr));
void    Mhead_GenGOPHeader _ANSI_ARGS_((BitBucket *bbPtr,
       int32_mpeg_t drop_frame_flag,
           int32_mpeg_t tc_hrs, int32_mpeg_t tc_min,
           int32_mpeg_t tc_sec, int32_mpeg_t tc_pict,
           int32_mpeg_t closed_gop, int32_mpeg_t broken_link,
           uint8_mpeg_t *ext_data, int32_mpeg_t ext_data_size,
           uint8_mpeg_t *user_data, int32_mpeg_t user_data_size));
void    Mhead_GenPictureHeader _ANSI_ARGS_((BitBucket *bbPtr, int frameType,
                        int pictCount, int f_code));
void    Mhead_GenSliceHeader _ANSI_ARGS_((BitBucket *bbPtr, uint32_mpeg_t slicenum,
                      uint32_mpeg_t qscale, uint8_mpeg_t *extra_info,
                      uint32_mpeg_t extra_info_size));
void    Mhead_GenSliceEnder _ANSI_ARGS_((BitBucket *bbPtr));
void    Mhead_GenMBHeader _ANSI_ARGS_((BitBucket *bbPtr,
      uint32_mpeg_t pict_code_type, uint32_mpeg_t addr_incr,
          uint32_mpeg_t q_scale,
          uint32_mpeg_t forw_f_code, uint32_mpeg_t back_f_code,
          uint32_mpeg_t horiz_forw_r, uint32_mpeg_t vert_forw_r,
          uint32_mpeg_t horiz_back_r, uint32_mpeg_t vert_back_r,
          int32_mpeg_t motion_forw, int32_mpeg_t m_horiz_forw,
          int32_mpeg_t m_vert_forw, int32_mpeg_t motion_back,
          int32_mpeg_t m_horiz_back, int32_mpeg_t m_vert_back,
          uint32_mpeg_t mb_pattern, uint32_mpeg_t mb_intra));


#endif /* MHEADERS_INCLUDED */
