/* mpeg2enc.c, main() and parameter file reading                            */

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
#include <stdlib.h>
#include <string.h>

#define GLOBAL /* used by global.h */
#include "config.h"
#include "global.h"

/* private prototypes */
static void init _ANSI_ARGS_((void));
static void readparmfile _ANSI_ARGS_((char *fname));
static void readquantmat _ANSI_ARGS_((void));


int main(argc,argv)
int argc;
char *argv[];
{
  if (argc!=3)
  {
    printf("\n%s, %s\n",version,author);
    printf("Usage: mpeg2encode in.par out.m2v\n");
    exit(0);
  }

  /* read parameter file */
  readparmfile(argv[1]);

  /* read quantization matrices */
  readquantmat();

  /* open output file */
  if (!(outfile=fopen(argv[2],"wb")))
  {
    sprintf(errortext,"Couldn't create output file %s",argv[2]);
    error(errortext);
  }

  init();
  putseq();

  fclose(outfile);
  fclose(statfile);

  return 0;
}

static void init()
{
  int i, size;
  static int block_count_tab[3] = {6,8,12};

  initbits();
  init_fdct();
  init_idct();

  /* round picture dimensions to nearest multiple of 16 or 32 */
  mb_width = (horizontal_size+15)/16;
  mb_height = prog_seq ? (vertical_size+15)/16 : 2*((vertical_size+31)/32);
  mb_height2 = fieldpic ? mb_height>>1 : mb_height; /* for field pictures */
  width = 16*mb_width;
  height = 16*mb_height;

  chrom_width = (chroma_format==CHROMA444) ? width : width>>1;
  chrom_height = (chroma_format!=CHROMA420) ? height : height>>1;

  height2 = fieldpic ? height>>1 : height;
  width2 = fieldpic ? width<<1 : width;
  chrom_width2 = fieldpic ? chrom_width<<1 : chrom_width;
  
  block_count = block_count_tab[chroma_format-1];

  /* clip table */
  if (!(clp = (unsigned char *)malloc(1024)))
    error("malloc failed\n");
  clp+= 384;
  for (i=-384; i<640; i++)
    clp[i] = (i<0) ? 0 : ((i>255) ? 255 : i);

  for (i=0; i<3; i++)
  {
    size = (i==0) ? width*height : chrom_width*chrom_height;

    if (!(newrefframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(oldrefframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(auxframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(neworgframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(oldorgframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(auxorgframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(predframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
  }

  mbinfo = (struct mbinfo *)malloc(mb_width*mb_height2*sizeof(struct mbinfo));

  if (!mbinfo)
    error("malloc failed\n");

  blocks =
    (short (*)[64])malloc(mb_width*mb_height2*block_count*sizeof(short [64]));

  if (!blocks)
    error("malloc failed\n");

  /* open statistics output file */
  if (statname[0]=='-')
    statfile = stdout;
  else if (!(statfile = fopen(statname,"w")))
  {
    sprintf(errortext,"Couldn't create statistics output file %s",statname);
    error(errortext);
  }
}

void error(text)
char *text;
{
  fprintf(stderr,text);
  putc('\n',stderr);
  exit(1);
}

/*****************************************************************************
  Modifications:
    Kathleen Bonnell, Mon Jul  2 10:43:29 PDT 2007
    On Windows, spaces are valid characters in a path/filename, so if on 
    Windows, take that into account when reading tplorg and statname.
*****************************************************************************/
static void readparmfile(fname)
char *fname;
{
  int i;
  int h,m,s,f;
  FILE *fd;
  char line[256];
  char *commentBegin = NULL;
  int len = 0;
  static double ratetab[8]=
    {24000.0/1001.0,24.0,25.0,30000.0/1001.0,30.0,50.0,60000.0/1001.0,60.0};
  extern int r,Xi,Xb,Xp,d0i,d0p,d0b; /* rate control */
  extern double avg_act; /* rate control */

  if (!(fd = fopen(fname,"r")))
  {
    sprintf(errortext,"Couldn't open parameter file %s",fname);
    error(errortext);
  }

  fgets(id_string,254,fd);
#ifndef WIN32
  fgets(line,254,fd); sscanf(line,"%s",tplorg);
#else
  fgets(line,254,fd); 
  commentBegin = strstr(line, "/*");
  len = strlen(line) - strlen(commentBegin);
  strncpy(tplorg, line, len);
  while (tplorg[len-1] == ' ')
	  len--;
  tplorg[len] = '\0';
#endif
  fgets(line,254,fd); sscanf(line,"%s",tplref);
  fgets(line,254,fd); sscanf(line,"%s",iqname);
  fgets(line,254,fd); sscanf(line,"%s",niqname);
#ifndef WIN32
  fgets(line,254,fd); sscanf(line,"%s",statname);
#else
  fgets(line,254,fd); 
  commentBegin = strstr(line, "/*");
  len = strlen(line) - strlen(commentBegin);
  strncpy(statname, line, len);
  while (statname[len-1] == ' ')
	  len--;
  statname[len] = '\0';
#endif
  fgets(line,254,fd); sscanf(line,"%d",&inputtype);
  fgets(line,254,fd); sscanf(line,"%d",&nframes);
  fgets(line,254,fd); sscanf(line,"%d",&frame0);
  fgets(line,254,fd); sscanf(line,"%d:%d:%d:%d",&h,&m,&s,&f);
  fgets(line,254,fd); sscanf(line,"%d",&N);
  fgets(line,254,fd); sscanf(line,"%d",&M);
  fgets(line,254,fd); sscanf(line,"%d",&mpeg1);
  fgets(line,254,fd); sscanf(line,"%d",&fieldpic);
  fgets(line,254,fd); sscanf(line,"%d",&horizontal_size);
  fgets(line,254,fd); sscanf(line,"%d",&vertical_size);
  fgets(line,254,fd); sscanf(line,"%d",&aspectratio);
  fgets(line,254,fd); sscanf(line,"%d",&frame_rate_code);
  fgets(line,254,fd); sscanf(line,"%lf",&bit_rate);
  fgets(line,254,fd); sscanf(line,"%d",&vbv_buffer_size);   
  fgets(line,254,fd); sscanf(line,"%d",&low_delay);     
  fgets(line,254,fd); sscanf(line,"%d",&constrparms);
  fgets(line,254,fd); sscanf(line,"%d",&profile);
  fgets(line,254,fd); sscanf(line,"%d",&level);
  fgets(line,254,fd); sscanf(line,"%d",&prog_seq);
  fgets(line,254,fd); sscanf(line,"%d",&chroma_format);
  fgets(line,254,fd); sscanf(line,"%d",&video_format);
  fgets(line,254,fd); sscanf(line,"%d",&color_primaries);
  fgets(line,254,fd); sscanf(line,"%d",&transfer_characteristics);
  fgets(line,254,fd); sscanf(line,"%d",&matrix_coefficients);
  fgets(line,254,fd); sscanf(line,"%d",&display_horizontal_size);
  fgets(line,254,fd); sscanf(line,"%d",&display_vertical_size);
  fgets(line,254,fd); sscanf(line,"%d",&dc_prec);
  fgets(line,254,fd); sscanf(line,"%d",&topfirst);
  fgets(line,254,fd); sscanf(line,"%d %d %d",
    frame_pred_dct_tab,frame_pred_dct_tab+1,frame_pred_dct_tab+2);
  
  fgets(line,254,fd); sscanf(line,"%d %d %d",
    conceal_tab,conceal_tab+1,conceal_tab+2);
  
  fgets(line,254,fd); sscanf(line,"%d %d %d",
    qscale_tab,qscale_tab+1,qscale_tab+2);

  fgets(line,254,fd); sscanf(line,"%d %d %d",
    intravlc_tab,intravlc_tab+1,intravlc_tab+2);
  fgets(line,254,fd); sscanf(line,"%d %d %d",
    altscan_tab,altscan_tab+1,altscan_tab+2);
  fgets(line,254,fd); sscanf(line,"%d",&repeatfirst);
  fgets(line,254,fd); sscanf(line,"%d",&prog_frame);
/* intra slice interval refresh period */  
  fgets(line,254,fd); sscanf(line,"%d",&P);
  fgets(line,254,fd); sscanf(line,"%d",&r);
  fgets(line,254,fd); sscanf(line,"%lf",&avg_act);
  fgets(line,254,fd); sscanf(line,"%d",&Xi);
  fgets(line,254,fd); sscanf(line,"%d",&Xp);
  fgets(line,254,fd); sscanf(line,"%d",&Xb);
  fgets(line,254,fd); sscanf(line,"%d",&d0i);
  fgets(line,254,fd); sscanf(line,"%d",&d0p);
  fgets(line,254,fd); sscanf(line,"%d",&d0b);

  if (N<1)
    error("N must be positive");
  if (M<1)
    error("M must be positive");
  if (N%M != 0)
    error("N must be an integer multiple of M");

  motion_data = (struct motion_data *)malloc(M*sizeof(struct motion_data));
  if (!motion_data)
    error("malloc failed\n");

  for (i=0; i<M; i++)
  {
    fgets(line,254,fd);
    sscanf(line,"%d %d %d %d",
      &motion_data[i].forw_hor_f_code, &motion_data[i].forw_vert_f_code,
      &motion_data[i].sxf, &motion_data[i].syf);

    if (i!=0)
    {
      fgets(line,254,fd);
      sscanf(line,"%d %d %d %d",
        &motion_data[i].back_hor_f_code, &motion_data[i].back_vert_f_code,
        &motion_data[i].sxb, &motion_data[i].syb);
    }
  }

  fclose(fd);

  /* make flags boolean (x!=0 -> x=1) */
  mpeg1 = !!mpeg1;
  fieldpic = !!fieldpic;
  low_delay = !!low_delay;
  constrparms = !!constrparms;
  prog_seq = !!prog_seq;
  topfirst = !!topfirst;

  for (i=0; i<3; i++)
  {
    frame_pred_dct_tab[i] = !!frame_pred_dct_tab[i];
    conceal_tab[i] = !!conceal_tab[i];
    qscale_tab[i] = !!qscale_tab[i];
    intravlc_tab[i] = !!intravlc_tab[i];
    altscan_tab[i] = !!altscan_tab[i];
  }
  repeatfirst = !!repeatfirst;
  prog_frame = !!prog_frame;

  /* make sure MPEG specific parameters are valid */
  range_checks();

  frame_rate = ratetab[frame_rate_code-1];

  /* timecode -> frame number */
  tc0 = h;
  tc0 = 60*tc0 + m;
  tc0 = 60*tc0 + s;
  tc0 = (int)(frame_rate+0.5)*tc0 + f;

  if (!mpeg1)
  {
    profile_and_level_checks();
  }
  else
  {
    /* MPEG-1 */
    if (constrparms)
    {
      if (horizontal_size>768
          || vertical_size>576
          || ((horizontal_size+15)/16)*((vertical_size+15)/16)>396
          || ((horizontal_size+15)/16)*((vertical_size+15)/16)*frame_rate>396*25.0
          || frame_rate>30.0)
      {
        if (!quiet)
          fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
        constrparms = 0;
      }
    }

    if (constrparms)
    {
      for (i=0; i<M; i++)
      {
        if (motion_data[i].forw_hor_f_code>4)
        {
          if (!quiet)
            fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
          constrparms = 0;
          break;
        }

        if (motion_data[i].forw_vert_f_code>4)
        {
          if (!quiet)
            fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
          constrparms = 0;
          break;
        }

        if (i!=0)
        {
          if (motion_data[i].back_hor_f_code>4)
          {
            if (!quiet)
              fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
            constrparms = 0;
            break;
          }

          if (motion_data[i].back_vert_f_code>4)
          {
            if (!quiet)
              fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
            constrparms = 0;
            break;
          }
        }
      }
    }
  }

  /* relational checks */

  if (mpeg1)
  {
    if (!prog_seq)
    {
      if (!quiet)
        fprintf(stderr,"Warning: setting progressive_sequence = 1\n");
      prog_seq = 1;
    }

    if (chroma_format!=CHROMA420)
    {
      if (!quiet)
        fprintf(stderr,"Warning: setting chroma_format = 1 (4:2:0)\n");
      chroma_format = CHROMA420;
    }

    if (dc_prec!=0)
    {
      if (!quiet)
        fprintf(stderr,"Warning: setting intra_dc_precision = 0\n");
      dc_prec = 0;
    }

    for (i=0; i<3; i++)
      if (qscale_tab[i])
      {
        if (!quiet)
          fprintf(stderr,"Warning: setting qscale_tab[%d] = 0\n",i);
        qscale_tab[i] = 0;
      }

    for (i=0; i<3; i++)
      if (intravlc_tab[i])
      {
        if (!quiet)
          fprintf(stderr,"Warning: setting intravlc_tab[%d] = 0\n",i);
        intravlc_tab[i] = 0;
      }

    for (i=0; i<3; i++)
      if (altscan_tab[i])
      {
        if (!quiet)
          fprintf(stderr,"Warning: setting altscan_tab[%d] = 0\n",i);
        altscan_tab[i] = 0;
      }
  }

  if (!mpeg1 && constrparms)
  {
    if (!quiet)
      fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
    constrparms = 0;
  }

  if (prog_seq && !prog_frame)
  {
    if (!quiet)
      fprintf(stderr,"Warning: setting progressive_frame = 1\n");
    prog_frame = 1;
  }

  if (prog_frame && fieldpic)
  {
    if (!quiet)
      fprintf(stderr,"Warning: setting field_pictures = 0\n");
    fieldpic = 0;
  }

  if (!prog_frame && repeatfirst)
  {
    if (!quiet)
      fprintf(stderr,"Warning: setting repeat_first_field = 0\n");
    repeatfirst = 0;
  }

  if (prog_frame)
  {
    for (i=0; i<3; i++)
      if (!frame_pred_dct_tab[i])
      {
        if (!quiet)
          fprintf(stderr,"Warning: setting frame_pred_frame_dct[%d] = 1\n",i);
        frame_pred_dct_tab[i] = 1;
      }
  }

  if (prog_seq && !repeatfirst && topfirst)
  {
    if (!quiet)
      fprintf(stderr,"Warning: setting top_field_first = 0\n");
    topfirst = 0;
  }

  /* search windows */
  for (i=0; i<M; i++)
  {
    if (motion_data[i].sxf > (4<<motion_data[i].forw_hor_f_code)-1)
    {
      if (!quiet)
        fprintf(stderr,
          "Warning: reducing forward horizontal search width to %d\n",
          (4<<motion_data[i].forw_hor_f_code)-1);
      motion_data[i].sxf = (4<<motion_data[i].forw_hor_f_code)-1;
    }

    if (motion_data[i].syf > (4<<motion_data[i].forw_vert_f_code)-1)
    {
      if (!quiet)
        fprintf(stderr,
          "Warning: reducing forward vertical search width to %d\n",
          (4<<motion_data[i].forw_vert_f_code)-1);
      motion_data[i].syf = (4<<motion_data[i].forw_vert_f_code)-1;
    }

    if (i!=0)
    {
      if (motion_data[i].sxb > (4<<motion_data[i].back_hor_f_code)-1)
      {
        if (!quiet)
          fprintf(stderr,
            "Warning: reducing backward horizontal search width to %d\n",
            (4<<motion_data[i].back_hor_f_code)-1);
        motion_data[i].sxb = (4<<motion_data[i].back_hor_f_code)-1;
      }

      if (motion_data[i].syb > (4<<motion_data[i].back_vert_f_code)-1)
      {
        if (!quiet)
          fprintf(stderr,
            "Warning: reducing backward vertical search width to %d\n",
            (4<<motion_data[i].back_vert_f_code)-1);
        motion_data[i].syb = (4<<motion_data[i].back_vert_f_code)-1;
      }
    }
  }

}

static void readquantmat()
{
  int i,v;
  FILE *fd;

  if (iqname[0]=='-')
  {
    /* use default intra matrix */
    load_iquant = 0;
    for (i=0; i<64; i++)
      intra_q[i] = default_intra_quantizer_matrix[i];
  }
  else
  {
    /* read customized intra matrix */
    load_iquant = 1;
    if (!(fd = fopen(iqname,"r")))
    {
      sprintf(errortext,"Couldn't open quant matrix file %s",iqname);
      error(errortext);
    }

    for (i=0; i<64; i++)
    {
      fscanf(fd,"%d",&v);
      if (v<1 || v>255)
        error("invalid value in quant matrix");
      intra_q[i] = v;
    }

    fclose(fd);
  }

  if (niqname[0]=='-')
  {
    /* use default non-intra matrix */
    load_niquant = 0;
    for (i=0; i<64; i++)
      inter_q[i] = 16;
  }
  else
  {
    /* read customized non-intra matrix */
    load_niquant = 1;
    if (!(fd = fopen(niqname,"r")))
    {
      sprintf(errortext,"Couldn't open quant matrix file %s",niqname);
      error(errortext);
    }

    for (i=0; i<64; i++)
    {
      fscanf(fd,"%d",&v);
      if (v<1 || v>255)
        error("invalid value in quant matrix");
      inter_q[i] = v;
    }

    fclose(fd);
  }
}
