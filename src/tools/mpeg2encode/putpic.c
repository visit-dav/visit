/* putpic.c, block and motion vector encoding routines                      */

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
#include "config.h"
#include "global.h"

/* private prototypes */
static void putmvs _ANSI_ARGS_((int MV[2][2][2], int PMV[2][2][2],
  int mv_field_sel[2][2], int dmvector[2], int s, int motion_type,
  int hor_f_code, int vert_f_code));

/* quantization / variable length encoding of a complete picture */
void putpict(frame)
unsigned char *frame;
{
  int i, j, k, comp, cc;
  int mb_type;
  int PMV[2][2][2];
  int prev_mquant;
  int cbp, MBAinc;

  rc_init_pict(frame); /* set up rate control */

  /* picture header and picture coding extension */
  putpicthdr();

  if (!mpeg1)
    putpictcodext();

  prev_mquant = rc_start_mb(); /* initialize quantization parameter */

  k = 0;

  for (j=0; j<mb_height2; j++)
  {
    /* macroblock row loop */

    for (i=0; i<mb_width; i++)
    {
      /* macroblock loop */
      if (i==0)
      {
        /* slice header (6.2.4) */
        alignbits();

        if (mpeg1 || vertical_size<=2800)
          putbits(SLICE_MIN_START+j,32); /* slice_start_code */
        else
        {
          putbits(SLICE_MIN_START+(j&127),32); /* slice_start_code */
          putbits(j>>7,3); /* slice_vertical_position_extension */
        }
  
        /* quantiser_scale_code */
        putbits(q_scale_type ? map_non_linear_mquant[prev_mquant]
                             : prev_mquant >> 1, 5);
  
        putbits(0,1); /* extra_bit_slice */
  
        /* reset predictors */

        for (cc=0; cc<3; cc++)
          dc_dct_pred[cc] = 0;

        PMV[0][0][0]=PMV[0][0][1]=PMV[1][0][0]=PMV[1][0][1]=0;
        PMV[0][1][0]=PMV[0][1][1]=PMV[1][1][0]=PMV[1][1][1]=0;
  
        MBAinc = i + 1; /* first MBAinc denotes absolute position */
      }

      mb_type = mbinfo[k].mb_type;

      /* determine mquant (rate control) */
      mbinfo[k].mquant = rc_calc_mquant(k);

      /* quantize macroblock */
      if (mb_type & MB_INTRA)
      {
        for (comp=0; comp<block_count; comp++)
          quant_intra(blocks[k*block_count+comp],blocks[k*block_count+comp],
                      dc_prec,intra_q,mbinfo[k].mquant);
        mbinfo[k].cbp = cbp = (1<<block_count) - 1;
      }
      else
      {
        cbp = 0;
        for (comp=0;comp<block_count;comp++)
          cbp = (cbp<<1) | quant_non_intra(blocks[k*block_count+comp],
                                           blocks[k*block_count+comp],
                                           inter_q,mbinfo[k].mquant);

        mbinfo[k].cbp = cbp;

        if (cbp)
          mb_type|= MB_PATTERN;
      }

      /* output mquant if it has changed */
      if (cbp && prev_mquant!=mbinfo[k].mquant)
        mb_type|= MB_QUANT;

      /* check if macroblock can be skipped */
      if (i!=0 && i!=mb_width-1 && !cbp)
      {
        /* no DCT coefficients and neither first nor last macroblock of slice */

        if (pict_type==P_TYPE && !(mb_type&MB_FORWARD))
        {
          /* P picture, no motion vectors -> skip */

          /* reset predictors */

          for (cc=0; cc<3; cc++)
            dc_dct_pred[cc] = 0;

          PMV[0][0][0]=PMV[0][0][1]=PMV[1][0][0]=PMV[1][0][1]=0;
          PMV[0][1][0]=PMV[0][1][1]=PMV[1][1][0]=PMV[1][1][1]=0;

          mbinfo[k].mb_type = mb_type;
          mbinfo[k].skipped = 1;
          MBAinc++;
          k++;
          continue;
        }

        if (pict_type==B_TYPE && pict_struct==FRAME_PICTURE
            && mbinfo[k].motion_type==MC_FRAME
            && ((mbinfo[k-1].mb_type^mb_type)&(MB_FORWARD|MB_BACKWARD))==0
            && (!(mb_type&MB_FORWARD) ||
                (PMV[0][0][0]==mbinfo[k].MV[0][0][0] &&
                 PMV[0][0][1]==mbinfo[k].MV[0][0][1]))
            && (!(mb_type&MB_BACKWARD) ||
                (PMV[0][1][0]==mbinfo[k].MV[0][1][0] &&
                 PMV[0][1][1]==mbinfo[k].MV[0][1][1])))
        {
          /* conditions for skipping in B frame pictures:
           * - must be frame predicted
           * - must be the same prediction type (forward/backward/interp.)
           *   as previous macroblock
           * - relevant vectors (forward/backward/both) have to be the same
           *   as in previous macroblock
           */

          mbinfo[k].mb_type = mb_type;
          mbinfo[k].skipped = 1;
          MBAinc++;
          k++;
          continue;
        }

        if (pict_type==B_TYPE && pict_struct!=FRAME_PICTURE
            && mbinfo[k].motion_type==MC_FIELD
            && ((mbinfo[k-1].mb_type^mb_type)&(MB_FORWARD|MB_BACKWARD))==0
            && (!(mb_type&MB_FORWARD) ||
                (PMV[0][0][0]==mbinfo[k].MV[0][0][0] &&
                 PMV[0][0][1]==mbinfo[k].MV[0][0][1] &&
                 mbinfo[k].mv_field_sel[0][0]==(pict_struct==BOTTOM_FIELD)))
            && (!(mb_type&MB_BACKWARD) ||
                (PMV[0][1][0]==mbinfo[k].MV[0][1][0] &&
                 PMV[0][1][1]==mbinfo[k].MV[0][1][1] &&
                 mbinfo[k].mv_field_sel[0][1]==(pict_struct==BOTTOM_FIELD))))
        {
          /* conditions for skipping in B field pictures:
           * - must be field predicted
           * - must be the same prediction type (forward/backward/interp.)
           *   as previous macroblock
           * - relevant vectors (forward/backward/both) have to be the same
           *   as in previous macroblock
           * - relevant motion_vertical_field_selects have to be of same
           *   parity as current field
           */

          mbinfo[k].mb_type = mb_type;
          mbinfo[k].skipped = 1;
          MBAinc++;
          k++;
          continue;
        }
      }

      /* macroblock cannot be skipped */
      mbinfo[k].skipped = 0;

      /* there's no VLC for 'No MC, Not Coded':
       * we have to transmit (0,0) motion vectors
       */
      if (pict_type==P_TYPE && !cbp && !(mb_type&MB_FORWARD))
        mb_type|= MB_FORWARD;

      putaddrinc(MBAinc); /* macroblock_address_increment */
      MBAinc = 1;

      putmbtype(pict_type,mb_type); /* macroblock type */

      if (mb_type & (MB_FORWARD|MB_BACKWARD) && !frame_pred_dct)
        putbits(mbinfo[k].motion_type,2);

      if (pict_struct==FRAME_PICTURE && cbp && !frame_pred_dct)
        putbits(mbinfo[k].dct_type,1);

      if (mb_type & MB_QUANT)
      {
        putbits(q_scale_type ? map_non_linear_mquant[mbinfo[k].mquant]
                             : mbinfo[k].mquant>>1,5);
        prev_mquant = mbinfo[k].mquant;
      }

      if (mb_type & MB_FORWARD)
      {
        /* forward motion vectors, update predictors */
        putmvs(mbinfo[k].MV,PMV,mbinfo[k].mv_field_sel,mbinfo[k].dmvector,0,
          mbinfo[k].motion_type,forw_hor_f_code,forw_vert_f_code);
      }

      if (mb_type & MB_BACKWARD)
      {
        /* backward motion vectors, update predictors */
        putmvs(mbinfo[k].MV,PMV,mbinfo[k].mv_field_sel,mbinfo[k].dmvector,1,
          mbinfo[k].motion_type,back_hor_f_code,back_vert_f_code);
      }

      if (mb_type & MB_PATTERN)
      {
        putcbp((cbp >> (block_count-6)) & 63);
        if (chroma_format!=CHROMA420)
          putbits(cbp,block_count-6);
      }

      for (comp=0; comp<block_count; comp++)
      {
        /* block loop */
        if (cbp & (1<<(block_count-1-comp)))
        {
          if (mb_type & MB_INTRA)
          {
            cc = (comp<4) ? 0 : (comp&1)+1;
            putintrablk(blocks[k*block_count+comp],cc);
          }
          else
            putnonintrablk(blocks[k*block_count+comp]);
        }
      }

      /* reset predictors */
      if (!(mb_type & MB_INTRA))
        for (cc=0; cc<3; cc++)
          dc_dct_pred[cc] = 0;

      if (mb_type & MB_INTRA || (pict_type==P_TYPE && !(mb_type & MB_FORWARD)))
      {
        PMV[0][0][0]=PMV[0][0][1]=PMV[1][0][0]=PMV[1][0][1]=0;
        PMV[0][1][0]=PMV[0][1][1]=PMV[1][1][0]=PMV[1][1][1]=0;
      }

      mbinfo[k].mb_type = mb_type;
      k++;
    }
  }

  rc_update_pict();
  vbv_end_of_picture();
}


/* output motion vectors (6.2.5.2, 6.3.16.2)
 *
 * this routine also updates the predictions for motion vectors (PMV)
 */
 
static void putmvs(MV,PMV,mv_field_sel,dmvector,s,motion_type,
  hor_f_code,vert_f_code)
int MV[2][2][2],PMV[2][2][2];
int mv_field_sel[2][2];
int dmvector[2];
int s,motion_type,hor_f_code,vert_f_code;
{
  if (pict_struct==FRAME_PICTURE)
  {
    if (motion_type==MC_FRAME)
    {
      /* frame prediction */
      putmv(MV[0][s][0]-PMV[0][s][0],hor_f_code);
      putmv(MV[0][s][1]-PMV[0][s][1],vert_f_code);
      PMV[0][s][0]=PMV[1][s][0]=MV[0][s][0];
      PMV[0][s][1]=PMV[1][s][1]=MV[0][s][1];
    }
    else if (motion_type==MC_FIELD)
    {
      /* field prediction */
      putbits(mv_field_sel[0][s],1);
      putmv(MV[0][s][0]-PMV[0][s][0],hor_f_code);
      putmv((MV[0][s][1]>>1)-(PMV[0][s][1]>>1),vert_f_code);
      putbits(mv_field_sel[1][s],1);
      putmv(MV[1][s][0]-PMV[1][s][0],hor_f_code);
      putmv((MV[1][s][1]>>1)-(PMV[1][s][1]>>1),vert_f_code);
      PMV[0][s][0]=MV[0][s][0];
      PMV[0][s][1]=MV[0][s][1];
      PMV[1][s][0]=MV[1][s][0];
      PMV[1][s][1]=MV[1][s][1];
    }
    else
    {
      /* dual prime prediction */
      putmv(MV[0][s][0]-PMV[0][s][0],hor_f_code);
      putdmv(dmvector[0]);
      putmv((MV[0][s][1]>>1)-(PMV[0][s][1]>>1),vert_f_code);
      putdmv(dmvector[1]);
      PMV[0][s][0]=PMV[1][s][0]=MV[0][s][0];
      PMV[0][s][1]=PMV[1][s][1]=MV[0][s][1];
    }
  }
  else
  {
    /* field picture */
    if (motion_type==MC_FIELD)
    {
      /* field prediction */
      putbits(mv_field_sel[0][s],1);
      putmv(MV[0][s][0]-PMV[0][s][0],hor_f_code);
      putmv(MV[0][s][1]-PMV[0][s][1],vert_f_code);
      PMV[0][s][0]=PMV[1][s][0]=MV[0][s][0];
      PMV[0][s][1]=PMV[1][s][1]=MV[0][s][1];
    }
    else if (motion_type==MC_16X8)
    {
      /* 16x8 prediction */
      putbits(mv_field_sel[0][s],1);
      putmv(MV[0][s][0]-PMV[0][s][0],hor_f_code);
      putmv(MV[0][s][1]-PMV[0][s][1],vert_f_code);
      putbits(mv_field_sel[1][s],1);
      putmv(MV[1][s][0]-PMV[1][s][0],hor_f_code);
      putmv(MV[1][s][1]-PMV[1][s][1],vert_f_code);
      PMV[0][s][0]=MV[0][s][0];
      PMV[0][s][1]=MV[0][s][1];
      PMV[1][s][0]=MV[1][s][0];
      PMV[1][s][1]=MV[1][s][1];
    }
    else
    {
      /* dual prime prediction */
      putmv(MV[0][s][0]-PMV[0][s][0],hor_f_code);
      putdmv(dmvector[0]);
      putmv(MV[0][s][1]-PMV[0][s][1],vert_f_code);
      putdmv(dmvector[1]);
      PMV[0][s][0]=PMV[1][s][0]=MV[0][s][0];
      PMV[0][s][1]=PMV[1][s][1]=MV[0][s][1];
    }
  }
}
