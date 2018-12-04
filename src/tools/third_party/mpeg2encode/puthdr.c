/* puthdr.c, generation of headers                                          */

/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * Commercial implementations of MPEG-1 and MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */

#include <stdio.h>
#include <math.h>
#include "config.h"
#include "global.h"

/* private prototypes */
static int frametotc _ANSI_ARGS_((int frame));

/* generate sequence header (6.2.2.1, 6.3.3)
 *
 * matrix download not implemented
 */
void putseqhdr()
{
  int i;

  alignbits();
  putbits(SEQ_START_CODE,32); /* sequence_header_code */
  putbits(horizontal_size,12); /* horizontal_size_value */
  putbits(vertical_size,12); /* vertical_size_value */
  putbits(aspectratio,4); /* aspect_ratio_information */
  putbits(frame_rate_code,4); /* frame_rate_code */
  putbits((int)ceil(bit_rate/400.0),18); /* bit_rate_value */
  putbits(1,1); /* marker_bit */
  putbits(vbv_buffer_size,10); /* vbv_buffer_size_value */
  putbits(constrparms,1); /* constrained_parameters_flag */

  putbits(load_iquant,1); /* load_intra_quantizer_matrix */
  if (load_iquant)
    for (i=0; i<64; i++)  /* matrices are always downloaded in zig-zag order */
      putbits(intra_q[zig_zag_scan[i]],8); /* intra_quantizer_matrix */

  putbits(load_niquant,1); /* load_non_intra_quantizer_matrix */
  if (load_niquant)
    for (i=0; i<64; i++)
      putbits(inter_q[zig_zag_scan[i]],8); /* non_intra_quantizer_matrix */
}

/* generate sequence extension (6.2.2.3, 6.3.5) header (MPEG-2 only) */
void putseqext()
{
  alignbits();
  putbits(EXT_START_CODE,32); /* extension_start_code */
  putbits(SEQ_ID,4); /* extension_start_code_identifier */
  putbits((profile<<4)|level,8); /* profile_and_level_indication */
  putbits(prog_seq,1); /* progressive sequence */
  putbits(chroma_format,2); /* chroma_format */
  putbits(horizontal_size>>12,2); /* horizontal_size_extension */
  putbits(vertical_size>>12,2); /* vertical_size_extension */
  putbits(((int)ceil(bit_rate/400.0))>>18,12); /* bit_rate_extension */
  putbits(1,1); /* marker_bit */
  putbits(vbv_buffer_size>>10,8); /* vbv_buffer_size_extension */
  putbits(0,1); /* low_delay  -- currently not implemented */
  putbits(0,2); /* frame_rate_extension_n */
  putbits(0,5); /* frame_rate_extension_d */
}

/* generate sequence display extension (6.2.2.4, 6.3.6)
 *
 * content not yet user setable
 */
void putseqdispext()
{
  alignbits();
  putbits(EXT_START_CODE,32); /* extension_start_code */
  putbits(DISP_ID,4); /* extension_start_code_identifier */
  putbits(video_format,3); /* video_format */
  putbits(1,1); /* colour_description */
  putbits(color_primaries,8); /* colour_primaries */
  putbits(transfer_characteristics,8); /* transfer_characteristics */
  putbits(matrix_coefficients,8); /* matrix_coefficients */
  putbits(display_horizontal_size,14); /* display_horizontal_size */
  putbits(1,1); /* marker_bit */
  putbits(display_vertical_size,14); /* display_vertical_size */
}

/* output a zero terminated string as user data (6.2.2.2.2, 6.3.4.1)
 *
 * string must not emulate start codes
 */
void putuserdata(userdata)
char *userdata;
{
  alignbits();
  putbits(USER_START_CODE,32); /* user_data_start_code */
  while (*userdata)
    putbits(*userdata++,8);
}

/* generate group of pictures header (6.2.2.6, 6.3.9)
 *
 * uses tc0 (timecode of first frame) and frame0 (number of first frame)
 */
void putgophdr(frame,closed_gop)
int frame,closed_gop;
{
  int tc;

  alignbits();
  putbits(GOP_START_CODE,32); /* group_start_code */
  tc = frametotc(tc0+frame);
  putbits(tc,25); /* time_code */
  putbits(closed_gop,1); /* closed_gop */
  putbits(0,1); /* broken_link */
}

/* convert frame number to time_code
 *
 * drop_frame not implemented
 */
static int frametotc(frame)
int frame;
{
  int fps, pict, sec, minute, hour, tc;

  fps = (int)(frame_rate+0.5);
  pict = frame%fps;
  frame = (frame-pict)/fps;
  sec = frame%60;
  frame = (frame-sec)/60;
  minute = frame%60;
  frame = (frame-minute)/60;
  hour = frame%24;
  tc = (hour<<19) | (minute<<13) | (1<<12) | (sec<<6) | pict;

  return tc;
}

/* generate picture header (6.2.3, 6.3.10) */
void putpicthdr()
{
  alignbits();
  putbits(PICTURE_START_CODE,32); /* picture_start_code */
  calc_vbv_delay();
  putbits(temp_ref,10); /* temporal_reference */
  putbits(pict_type,3); /* picture_coding_type */
  putbits(vbv_delay,16); /* vbv_delay */

  if (pict_type==P_TYPE || pict_type==B_TYPE)
  {
    putbits(0,1); /* full_pel_forward_vector */
    if (mpeg1)
      putbits(forw_hor_f_code,3);
    else
      putbits(7,3); /* forward_f_code */
  }

  if (pict_type==B_TYPE)
  {
    putbits(0,1); /* full_pel_backward_vector */
    if (mpeg1)
      putbits(back_hor_f_code,3);
    else
      putbits(7,3); /* backward_f_code */
  }

  putbits(0,1); /* extra_bit_picture */
}

/* generate picture coding extension (6.2.3.1, 6.3.11)
 *
 * composite display information (v_axis etc.) not implemented
 */
void putpictcodext()
{
  alignbits();
  putbits(EXT_START_CODE,32); /* extension_start_code */
  putbits(CODING_ID,4); /* extension_start_code_identifier */
  putbits(forw_hor_f_code,4); /* forward_horizontal_f_code */
  putbits(forw_vert_f_code,4); /* forward_vertical_f_code */
  putbits(back_hor_f_code,4); /* backward_horizontal_f_code */
  putbits(back_vert_f_code,4); /* backward_vertical_f_code */
  putbits(dc_prec,2); /* intra_dc_precision */
  putbits(pict_struct,2); /* picture_structure */
  putbits((pict_struct==FRAME_PICTURE)?topfirst:0,1); /* top_field_first */
  putbits(frame_pred_dct,1); /* frame_pred_frame_dct */
  putbits(0,1); /* concealment_motion_vectors  -- currently not implemented */
  putbits(q_scale_type,1); /* q_scale_type */
  putbits(intravlc,1); /* intra_vlc_format */
  putbits(altscan,1); /* alternate_scan */
  putbits(repeatfirst,1); /* repeat_first_field */
  putbits(prog_frame,1); /* chroma_420_type */
  putbits(prog_frame,1); /* progressive_frame */
  putbits(0,1); /* composite_display_flag */
}

/* generate sequence_end_code (6.2.2) */
void putseqend()
{
  alignbits();
  putbits(SEQ_END_CODE,32);
}
