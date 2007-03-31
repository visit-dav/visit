/*============================================================================*
 * rate.c                                      *
 *                                          * 
 *    Procedures concerned with rate control                                *
 *                                          *
 * EXPORTED PROCEDURES:                                  *
 *      initRatecontrol()                                                     *
 *      targetRateControl()                                                   *
 *      updateRateControl()                                                   *
 *      MB_RateOut()                                                          *
 *      needQScaleChange()                                                    *
 *      incNumBlocks()                                                        *
 *      incQuant()                                                            *
 *    incMacroBlockBits()                                                   *
 *      setPictureRate()                                                      *
 *      setBitRate()                                                          *
 *      getBitRate()                                                          *
 *      setBufferSize()                                                       *
 *      getBufferSize()                                                       *
 *                                                                            *
 * NOTES:                                                                     *
 *    Naming conventions follow those of MPEG-2 draft algorithm (chap. 10)  *
 *============================================================================*/


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

/*==============*
 * HEADER FILES *
 *==============*/

#include <sys/times.h>
#include "all.h"
#include "mtypes.h"
#include "bitio.h"
#include "frames.h"
#include "prototypes.h"
#include "param.h"
#include "mheaders.h"
#include "fsize.h"
#include "postdct.h"
#include "mpeg.h"
#include "parallel.h"
#include "dct.h"
#include "rate.h"


/*==================*
 * GLOBAL VARIABLES *
 *==================*/

#define MAX_BIT_RATE 104857600        /* 18 digit number in units of 400 */
#define MAX_BUFFER_SIZE 16760832        /* 10 digit number in units of 16k */
#define DEFAULT_BUFFER_SIZE 327680      /* maximun for "constrained" bitstream */
#define DEFAULT_VBV_FULLNESS 3          /* wait till 1/3 full */
#define DEFAULT_PICT_RATE_CODE 5        /* code for 30 Frames/sec */
#define DEFAULT_PICT_RATE 30            /* 30 frames per second */
#define MAX_VBV_DELAY 32768             /* 16 digits */


/*      Variables from Parameter File */

static int    RateControlMode = VARIABLE_RATE;
static int32_mpeg_t buffer_size = DEFAULT_BUFFER_SIZE;
static int32_mpeg_t bit_rate = -1;


/*   Variables for the VBV buffer defined in MPEG specs */
static int32_mpeg_t VBV_delay =0;        /* delay in units of 1/90000 seconds */
static int32_mpeg_t VBV_buffer = 0;        /* fullness of the theoretical VBV buffer */
static int32_mpeg_t bufferFillRate = 0;    /* constant rate at which buffer filled */
static int32_mpeg_t frameDelayIncrement = 0;    /* number of "delay" units/Frame */

/*  Global complexity measure variables */
static int Xi, Xp, Xb;  /*  Global complexity measure  */

static int Si, Sp, Sb;  /*  Total # bits for last pict of type (Overhead?) */

static float Qi, Qp, Qb; /* avg quantizaton for last picture of type  */
     
/*  Target bit allocations for each type of picture*/
int Ti, Tp, Tb;

int current_Tx;    /* allocation for current frame */

/*  Count of number of pictures of each type remaining */
int GOP_X = 0;
int GOP_I = 0;
int GOP_P = 0;
int GOP_B = 0;

int Nx = 0;
int Ni = 0;
int Np = 0;
int Nb = 0;

/*   Counters used while encoding frames   */

int rc_numBlocks = 0;
int rc_totalQuant = 0;
int rc_bitsThisMB;
int rc_totalMBBits;
int rc_totalFrameBits;
int rc_totalOverheadBits = 0;


/*    Want to print out Macroblock info every Nth MB */
int RC_MB_SAMPLE_RATE = 0;

static float Ki = .7;
static float Kp = 1;
static float Kb = 1.4;
static int rc_R;
static int rc_G;

/*   Rate Control variables   */

/*   Virtual buffers for each frame type */
static int d0_i;   /* Initial fullnesses */
static int d0_p;
static int d0_b;

static int lastFrameVirtBuf;   /* fullness after last frame of this type */
static int currentVirtBuf;     /* fullness during current encoding*/

static int MB_cnt = -1;           /* Number of MB's in picture */

static int rc_Q;               /* reference quantization parameter */

static int reactionParameter;  /*  Reaction parameter */

/*    Adaptive Quantization variables */
static int act_j;              /*  spatial activity measure */
static float N_act;            /*  Normalised spacial activity */
static int avg_act;       /*  average activity value in last picture encoded */
static int total_act_j;           /*  Sum of activity values in current frame */

static int var_sblk;           /* sub-block activity */
static int P_mean;           /* Mean value of pixels in 8x8 sub-block */

static int mquant;           /* Raw Quantization value */
static int Qscale;           /* Clipped, truncated quantization value */



/*  Output-related variables */
#ifdef RC_STATS_FILE
static FILE *RC_FILE;
#endif

static char *Frame_header1 = "  Fm         #     Bit      GOP                    V                ";
static char *Frame_header2 = "   #  type   MBs   Alloc    left  Ni Np Nb  N_act  buff   Q_rc Qscale";
static char *Frame_header3 = "----     -  ----  ------ -------  -- -- --  -----  ------ ----   ----";
static char *Frame_trailer1 = "                      avg          virt     %    GOP      %     VBV";
static char *Frame_trailer2 = "    Sx    Qx      Xx  act N_act  buffer alloc    left  left     buf  delay";
static char *Frame_trailer3 = "------ --.-- -------  --- --.-- -------   --- -------   --- ------- ------";

static char *MB_header1 = "MB#  #bits  Q mqt     Dj  Q_j   actj  N_act  totbits b/MB %alloc %done";
static char *MB_header2 = "---  ----- -- --- ------  ---  -----  --.--   ------ ----    ---   ---";

static char rc_buffer[101];

/*    EXTERNAL Variables  */
extern char *framePattern;
extern int framePatternLen;


/*===============================*
 * INTERNAL PROCEDURE prototypes *
 *===============================*/

int initGOPRateControl _ANSI_ARGS_((void));
     int determineMBCount _ANSI_ARGS_((void));
     void checkBufferFullness _ANSI_ARGS_((int count));
     void checkSpatialActivity _ANSI_ARGS_((Block blk0, Block blk1, Block blk2, Block blk3));
     void incNumBlocks _ANSI_ARGS_((int num));
     void calculateVBVDelay _ANSI_ARGS_((int num));
     void updateVBVBuffer _ANSI_ARGS_((int frameBits));
     int BlockExperiments  _ANSI_ARGS_((int16_mpeg_t *OrigBlock, int16_mpeg_t *NewBlock, int control));
     
     
     /*=====================*
      * EXPORTED PROCEDURES *
      *=====================*/
     
     /*===========================================================================*
      *
      * initRateControl
      *
      *    initialize the allocation parameters.
      *
      * RETURNS:    nothing
      *
      * SIDE EFFECTS:   many global variables 
      *
      * NOTES:  Get rid of the redundant pattern stuff!!
      *===========================================================================*/
     int
       initRateControl()
{
  int index;
  int result;
  
  DBG_PRINT(("\tInitializing Allocation Data\n"));
  
#ifdef RC_STATS_FILE
  RC_FILE = fopen("RC_STATS_FILE", "w");
  if ( RC_FILE  == NULL) {
    DBG_PRINT(("\tOpen of RC file failed, using stderr\n"));
    RC_FILE = stderr;
    fprintf(RC_FILE, "\tOpen of RC file failed, using stderr\n");
    fflush(RC_FILE);
  }
#endif
  
  /*  Initialize Pattern info */
  GOP_X = framePatternLen;
  for ( index = 0; index < framePatternLen; index++ ) {
    switch( framePattern[index] ) {
    case 'i':
      GOP_I++;
      break;
    case 'p':
      GOP_P++;
      break;
    case 'b':
      GOP_B++;
      break;
    default:
      printf("\n\tERROR rate.c - BAD PATTERN!\n");
      RateControlMode = VARIABLE_RATE;
      return (0);
    }
  }
  if (GOP_X != (GOP_I + GOP_P + GOP_B )) {
    printf("\n\tERROR rate.c - Pattern Length Mismatch\n");
    RateControlMode = VARIABLE_RATE;
    return (-1);
  }
  
  /* Initializing GOP bit allocation */    
  rc_R = 0;
  rc_G = (bit_rate * GOP_X/frameRateRounded);
  
  /*   Initialize the "global complexity measures" */
  Xi = (160 * bit_rate/115);
  Xp = (60 * bit_rate/115);
  Xb = (42 * bit_rate/115);
  
  /*   Initialize MB counters */
  rc_totalMBBits= rc_bitsThisMB= rc_totalFrameBits=rc_totalOverheadBits = 0;
  rc_numBlocks = rc_totalQuant = 0;
  
  /*   init virtual buffers  */
  reactionParameter = (2 * bit_rate / frameRateRounded);
  d0_i = (10 * reactionParameter / 31);
  d0_p = (Kp * d0_i);
  d0_b = (Kb * d0_i);
  
  lastFrameVirtBuf = d0_i;    /*  start with I Frame */
  rc_Q = lastFrameVirtBuf  * 31 / reactionParameter;
  
  /*   init spatial activity measures */
  avg_act = 400;        /* Suggested initial value */
  N_act = 1;
  
  mquant = rc_Q * N_act;
  
  frameDelayIncrement = (90000 / frameRateRounded); /* num of "delay" units per frame */
  bufferFillRate = bit_rate / frameRateRounded; /* VBV buf fills at constant rate */
  VBV_buffer = buffer_size;
  DBG_PRINT(("\tVBV- delay: %d, fill rate: %d, delay/Frame: %d units, buffer size: %d\n",
         VBV_delay, bufferFillRate, frameDelayIncrement, buffer_size));
  
  result = initGOPRateControl();
  
  return result;
}

/*===========================================================================*
 *
 * initGOPRateControl
 *
 *        (re)-initialize the RC for the a new Group of Pictures.
 *    New bit allocation, but carry over complexity measures.
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:   many global variables 
 *
 *===========================================================================*/
int
  initGOPRateControl()
{
  DBG_PRINT(("\tInitializing new GOP\n"));
  
  Nx = GOP_X;
  Ni = GOP_I;
  Np = GOP_P;
  Nb = GOP_B;
  
  rc_R += rc_G;
  
  DBG_PRINT(("\tbufsize: %d, bitrate: %d, pictrate: %d, GOP bits: %d\n",
         buffer_size, bit_rate, frameRateRounded, rc_R));
  DBG_PRINT(("\tXi: %d, Xp: %d, Xb: %d Nx: %d, Ni: %d, Np: %d, Nb: %d\n",
         Xi, Xp, Xb, Nx,Ni,Np,Nb));
  DBG_PRINT(("\td0_i: %d, d0_p: %d, d0_b: %d, avg_act: %d, rc_Q: %d, mquant: %d\n",
         d0_i, d0_p, d0_b, avg_act, rc_Q, mquant));
  return 1;
}


/*===========================================================================*
 *
 * targetRateControl
 *
 *      Determine the target allocation for given picture type, initiates
 *  variables for rate control process.
 *
 * RETURNS:     nothing.
 *
 * SIDE EFFECTS:   many global variables
 *
 *===========================================================================*/
void
  targetRateControl(frame)
MpegFrame   *frame;
{
  float temp1, minimumBits;
  float tempX, tempY, tempZ;
  int result;
  int frameType;
  char *strPtr;
  
  minimumBits = (bit_rate / (8 * frameRateRounded));
  
  /*   Check if new GOP */
  if (Nx == 0) {
    initGOPRateControl();
  }
  
  if (MB_cnt < 0) {MB_cnt = determineMBCount();}
  
  switch (frame->type) {
  case TYPE_IFRAME:
    frameType = 'I';
    
    /*        temp1 = ( rc_R / ( 1+ ((Np * Xp) / (Xi * Kp)) + ((Nb*Xb) / (Xi*Kb))))); */
    
    tempX = ( (Np * Ki * Xp) / (Xi * Kp) );
    tempY = ( (Nb * Ki * Xb) / (Xi*Kb) );
    tempZ = Ni + tempX + tempY;
    temp1 = (rc_R / tempZ);
    result = (int) (temp1 > minimumBits ? temp1 :  minimumBits);
    current_Tx = Ti = result;
    lastFrameVirtBuf = d0_i;
    break;
    
  case TYPE_PFRAME:
    frameType = 'P';
    tempX =  ( (Ni * Kp * Xi) / (Ki * Xp) );
    tempY =  ( (Nb * Kp * Xb) / (Kb * Xp) );
    tempZ = Np + tempX + tempY;
    temp1 = (rc_R/ tempZ);
    result = (int) (temp1 > minimumBits ? temp1 :  minimumBits);
    current_Tx = Tp = result;
    lastFrameVirtBuf = d0_p;
    break;
    
  case TYPE_BFRAME:
    frameType = 'B';
    tempX =  ( (Ni * Kb * Xi) / (Ki * Xb) );
    tempY =  ( (Np * Kb * Xp) / (Kp * Xb) );
    tempZ = Nb + tempX + tempY;
    temp1 = (rc_R/ tempZ);
    result = (int) (temp1 > minimumBits ? temp1 :  minimumBits);
    current_Tx = Tb = result;
    lastFrameVirtBuf = d0_b;
    break;
    
  default:
    frameType = 'X';
  }
  
  N_act = 1;
  rc_Q = lastFrameVirtBuf  * 31 / reactionParameter;
  mquant = rc_Q * N_act;
  Qscale = (mquant > 31 ? 31 : mquant);
  Qscale = (Qscale < 1 ? 1 : Qscale);
  
  /*   Print headers for Frame info */
  strPtr = Frame_header1;
  DBG_PRINT(("%s\n",strPtr));
  strPtr = Frame_header2;
  DBG_PRINT(("%s\n",strPtr));
  strPtr = Frame_header3;
  DBG_PRINT(("%s\n",strPtr));
  
  /*   Print Frame info */
  sprintf(rc_buffer, "%4d     %1c  %4d  %6d %7d  %2d %2d %2d   %2.2f  %6d %4d    %3d",
      frame->id,frameType,MB_cnt,current_Tx,rc_R,Ni,Np,Nb, N_act, lastFrameVirtBuf, rc_Q, Qscale);
  
#ifdef RC_STATS_FILE
  fprintf(RC_FILE,"%s\n", rc_buffer);
  fflush(RC_FILE);
#endif
  DBG_PRINT(("%s\n",rc_buffer));
  
  /*  Print headers for Macroblock info */
  if (RC_MB_SAMPLE_RATE) {
    strPtr = MB_header1;
    DBG_PRINT(("%s\n",strPtr));
    strPtr = MB_header2;
    DBG_PRINT(("%s\n",strPtr));
  } else {
    return;
  }
  
  return;
}



/*===========================================================================*
 *
 * updateRateControl
 *
 *      Update the statistics kept, after end of frame.  Resets
 *  various global variables
 *
 * RETURNS:     nothing
 *
 * SIDE EFFECTS:   many global variables
 *
 *===========================================================================*/
void
  updateRateControl(type)
int type; 
{
  int totalBits, frameComplexity, pctAllocUsed, pctGOPUsed;
  float avgQuant;
  char *strPtr;
  
  totalBits = rc_totalFrameBits;
  avgQuant = ((float) rc_totalQuant / (float) rc_numBlocks);
  frameComplexity = totalBits * avgQuant;
  pctAllocUsed = (totalBits *100 / current_Tx);
  rc_R -= totalBits;
  pctGOPUsed = (rc_R *100/ rc_G);
  
  avg_act = (total_act_j / MB_cnt);
  
  updateVBVBuffer(totalBits);
  
  switch (type) {
  case TYPE_IFRAME:
    Ti = current_Tx;
    d0_i = currentVirtBuf;
    Ni--;
    Si = totalBits;
    Qi = avgQuant;
    Xi = frameComplexity;
    break;
  case TYPE_PFRAME:
    Tp = current_Tx;
    d0_p = currentVirtBuf;
    Np--;
    Sp = totalBits;
    Qp = avgQuant;
    Xp = frameComplexity;
    break;
  case TYPE_BFRAME:
    Tb = current_Tx;
    d0_b = currentVirtBuf;
    Nb--;
    Sb = totalBits;
    Qb = avgQuant;
    Xb = frameComplexity;
    break;
  }
  
  
  /*  Print Frame info */
  strPtr = Frame_trailer1;
  DBG_PRINT(("%s\n",strPtr));
  strPtr = Frame_trailer2;
  DBG_PRINT(("%s\n",strPtr));
  strPtr = Frame_trailer3;
  DBG_PRINT(("%s\n",strPtr));
  
  sprintf(rc_buffer, "%6d  %2.2f  %6d  %3d  %2.2f %7d   %3d %7d   %3d  %6d %6d",
      totalBits, avgQuant, frameComplexity, avg_act, N_act, currentVirtBuf, pctAllocUsed, rc_R, pctGOPUsed, VBV_buffer, VBV_delay);
#ifdef RC_STATS_FILE
  fprintf(RC_FILE,"%s\n", rc_buffer);
  fflush(RC_FILE);
#endif
  DBG_PRINT(("%s\n",rc_buffer));
  
  Nx--;
  rc_totalMBBits= rc_bitsThisMB= rc_totalFrameBits=rc_totalOverheadBits = 0;
  rc_numBlocks = rc_totalQuant = total_act_j = currentVirtBuf = 0;
  
  DBG_PRINT(("GOP now has %d bits remaining (%3d%%) for %d frames .. , Ni= %d, Np= %d, Nb= %d\n", rc_R, (rc_R*100/rc_G), (Ni+Np+Nb), Ni, Np, Nb));
  
}


/*===========================================================================*
 *
 * MB_RateOut
 *
 *      Prints out sampling of MB rate control data.  Every "nth" block
 *    stats are printed, with "n" controled by global RC_MB_SAMPLE_RATE
 *    (NB. "skipped" blocks do not go through this function and thus do not
 *        show up in the sample )
 *
 * RETURNS:     nothing
 *
 * SIDE EFFECTS:   none
 *
 * NOTES:
 *
 *===========================================================================*/
void
  MB_RateOut(type)
int type;
{
  int totalBits;
  int pctUsed, pctDone;
  int bitsThisMB;
  int bitsPerMB;
  
  bitsThisMB = rc_bitsThisMB;
  totalBits = rc_totalFrameBits;
  bitsPerMB = (totalBits / rc_numBlocks); 
  pctDone = (rc_numBlocks * 100/ MB_cnt); 
  pctUsed = (totalBits *100/current_Tx);
  
  sprintf(rc_buffer, "%3d  %5d %2d %3d %6d  %3d %6d   %2.2f   %6d %4d    %3d   %3d\n",
      (rc_numBlocks - 1), bitsThisMB, Qscale, mquant, currentVirtBuf, 
      rc_Q, act_j, N_act, totalBits, bitsPerMB, pctUsed, pctDone);
#ifdef RC_STATS_FILE
  fprintf(RC_FILE, "%s", rc_buffer);
  fflush(RC_FILE);
#endif
  
  if ( (RC_MB_SAMPLE_RATE) && ((rc_numBlocks -1) % RC_MB_SAMPLE_RATE)) {
    DBG_PRINT(("%s\n", rc_buffer));
  } else {
    return;
  }
}



/*===========================================================================*
 *
 * incNumBlocks()
 *
 *
 * RETURNS:   nothing
 *
 * SIDE EFFECTS:  rc_numBlocks
 *
 * NOTES:
 *
 *===========================================================================*/
void incNumBlocks(num)
     int num;
{
  rc_numBlocks += num;
}


/*===========================================================================*
 *
 * incMacroBlockBits()
 *
 *    Increments the number of Macro Block bits and the total of Frame
 *  bits by the number passed.
 *
 * RETURNS:   nothing
 *
 * SIDE EFFECTS:  rc_totalMBBits
 *
 * NOTES:
 *
 *===========================================================================*/
void incMacroBlockBits(num)
     int num;
{
  rc_bitsThisMB = num;
  rc_totalMBBits += num;
  rc_totalFrameBits += num;
}


/*===========================================================================*
 *
 *       needQScaleChange(current Q scale, 4 luminance blocks)
 *
 *
 * RETURNS:     new Qscale
 *
 * SIDE EFFECTS:   
 *
 *===========================================================================*/
int needQScaleChange(oldQScale, blk0, blk1, blk2, blk3)
     int oldQScale;
     Block blk0;
     Block blk1;
     Block blk2;
     Block blk3;
{
  
  /*   One more MacroBlock seen */
  rc_numBlocks++;        /* this notes each block num in MB */
  
  checkBufferFullness(oldQScale);
  
  checkSpatialActivity(blk0, blk1, blk2, blk3);
  
  mquant = rc_Q * N_act;
  Qscale = (mquant > 31 ? 31 : mquant);
  Qscale = (Qscale < 1 ? 1 : Qscale);
  rc_totalQuant += Qscale;
  
  if (oldQScale == Qscale)
    return -1;
  else
    return Qscale;
}


/*===========================================================================*
 *
 * determineMBCount() 
 *
 *      Determines number of Macro Blocks in frame from the frame sizes
 *    passed.
 *
 * RETURNS:     nothing
 *
 * SIDE EFFECTS:   sets the count passed
 *
 *===========================================================================*/
int
  determineMBCount ()
{
  int y,x;
  
  x = (Fsize_x +15)/16;
  y = (Fsize_y +15)/16;
  return  (x * y);
}



/*===========================================================================*
 *
 * void checkBufferFullness ()
 *
 *      Calculates the fullness of the virtual buffer for each
 *  frame type.  Called before encoding each macro block.  Along
 *  with the normalisec spatial activity measure (N_act), it
 *  determine the quantization factor for the next macroblock.
 *
 * RETURNS:     nothing
 *
 * SIDE EFFECTS:   the "currentVirtBuf" variable
 *
 * NOTES:
 *
 *===========================================================================*/
void checkBufferFullness (oldQScale)
     int oldQScale;
{
  int temp;
  
  temp = lastFrameVirtBuf + rc_totalFrameBits;
  temp -=  (current_Tx * rc_numBlocks / MB_cnt);
  currentVirtBuf = temp;
  
  rc_Q = (currentVirtBuf * 31 / reactionParameter);
  return;
}


/*===========================================================================*
 *
 * void checkSpatialActivity()
 *
 *      Calcualtes the spatial activity for the four luminance blocks of the
 *    macroblock.  Along with the normalised reference quantization parameter 
 *  (rc_Q) , it determines the quantization factor for the next macroblock.
 *
 * RETURNS:     nothing
 *
 * SIDE EFFECTS:   the Adaptive quantization variables- act_j, N_act.
 *
 * NOTES:
 *
 *===========================================================================*/
void checkSpatialActivity(blk0, blk1, blk2, blk3)
     Block blk0;
     Block blk1;
     Block blk2;
     Block blk3;
{
  int temp;
  int16_mpeg_t *blkArray[4]; 
  int16_mpeg_t *curBlock;
  int16_mpeg_t *blk_ptr;
  int var[4];
  int i, j;
  
  
  blkArray[0] = (int16_mpeg_t *) blk0;
  blkArray[1] = (int16_mpeg_t *) blk1;
  blkArray[2] = (int16_mpeg_t *) blk2;
  blkArray[3] = (int16_mpeg_t *) blk3;
  
  
  for (i =0; i < 4; i++) {    /* Compute the activity in each block */
    curBlock = blkArray[i];
    blk_ptr = curBlock;
    P_mean = 0;
    /*  Find the mean pixel value */
    for (j=0; j < DCTSIZE_SQ; j ++) {
      P_mean += *(blk_ptr++);
      /*            P_mean += curBlock[j]; 
                if (curBlock[j] != *(blk_ptr++)) {
                printf("\n\tARRAY ERROR: block %d\n", j);
                }
                */
    }
    P_mean /= DCTSIZE_SQ;
    
    /*  Now find the variance  */
    curBlock = blkArray[i];
    blk_ptr = curBlock;
    var[i] = 0;
    for (j=0; j < DCTSIZE_SQ; j++) {
#ifdef notdef
      if (curBlock[j] != *(blk_ptr++)) {
    printf("\n\tARRAY ERROR: block %d\n", j);
      }
      temp = curBlock[j] - P_mean;
#endif      
      temp = *(blk_ptr++) - P_mean;
      var[i] += (temp * temp);
    }
    var[i] /= DCTSIZE_SQ;
  }
  
  /*  Choose the minimum variance from the 4 blocks and use as the activity */
  var_sblk  = var[0];
  for (i=1; i < 4; i++) {
    var_sblk = (var_sblk < var[i] ? var_sblk : var[i]);
  }
  
  
  act_j = 1 + var_sblk;
  total_act_j += act_j;
  temp = (2 * act_j + avg_act);
  N_act = ( (float) temp / (float) (act_j + 2*avg_act) );
  
  return;
}




/*============================================================================*
 *
 * getRateMode ()
 *
 *      Returns the rate mode- interpreted as either Fixed or Variable
 *
 * RETURNS:     integer
 *
 * SIDE EFFECTS:   none
 *
 *
 *==========================================================================*/
int getRateMode()
{
  return RateControlMode;
}


/*===========================================================================*
 *
 * setBitRate ()
 *
 *      Checks the string parsed from the parameter file.  Verifies
 *  number and sets global values. MPEG standard specifies that bit rate
 *    be rounded up to nearest 400 bits/sec.
 *
 * RETURNS:     nothing
 *
 * SIDE EFFECTS:   global variables
 *
 * NOTES:    Should this be in the 400-bit units used in sequence header?
 *
 *===========================================================================*/
void setBitRate (charPtr)
     char * charPtr;
{
  int rate, rnd;
  
  rate = atoi(charPtr);
  if (rate > 0) {
    RateControlMode = FIXED_RATE;
  } else {
    printf("Parameter File Error:  invalid BIT_RATE: \"%s\", defaults to Variable ratemode\n",
       charPtr);
    RateControlMode = VARIABLE_RATE;
    bit_rate = -1;
  }
  rnd = (rate % 400);
  rate += (rnd ? 400 -rnd : 0); /* round UP to nearest 400 bps */
  rate = (rate > MAX_BIT_RATE ? MAX_BIT_RATE : rate);
  bit_rate = rate;
  DBG_PRINT(("Bit rate is: %d\n", bit_rate));
} 



/*===========================================================================*
 *
 * getBitRate ()
 *
 *      Returns the bit rate read from the parameter file.  This is the
 *  real rate in bits per second, not in 400 bit units as is written to
 *  the sequence header.
 *
 * RETURNS:     int (-1 if Variable mode operation)
 *
 * SIDE EFFECTS:   none
 *
 *===========================================================================*/
int getBitRate ()
{
  return bit_rate;
}




/*===========================================================================*
 *
 * setBufferSize ()
 *
 *      Checks the string parsed from the parameter file.  Verifies
 *  number and sets global values.
 *
 * RETURNS:     nothing
 *
 * SIDE EFFECTS:   buffer_size global variable.
 *
 * NOTES:    The global is in bits, NOT the 16kb units used in sequence header
 *
 *===========================================================================*/
void setBufferSize (charPtr)
     char * charPtr;
{
  int size;
  
  size = atoi(charPtr);
  size = (size > MAX_BUFFER_SIZE ? MAX_BUFFER_SIZE : size);
  if (size > 0) {
    size = (16*1024) * ((size + (16*1024 - 1)) / (16*1024));
    buffer_size = size;
  } else {
    buffer_size = DEFAULT_BUFFER_SIZE;
    printf("Parameter File Error:  invalid BUFFER_SIZE: \"%s\", defaults to : %d\n",
       charPtr, buffer_size);
  }
  DBG_PRINT(("Buffer size is: %d\n", buffer_size));
}


/*===========================================================================*
 *
 * getBufferSize ()
 *
 *      returns the buffer size read from the parameter file.  Size is
 *  in bits- not in units of 16k as written to the sequence header.
 *
 * RETURNS:     int (or -1 if invalid)
 *
 * SIDE EFFECTS:   none
 *
 *===========================================================================*/
int getBufferSize ()
{
  return buffer_size;
}


/*===========================================================================*
 *
 * updateVBVBuffer ()
 *
 *      Update the VBV buffer after each frame.  This theoretical 
 * buffer is being filled at constant rate, given by the bit rate.
 * It is emptied as each frame is grabbed by the decoder.  Exception 
 * is that the deocder will wait until the "delay" is over.
 *
 * RETURNS:     nothing
 *
 * SIDE EFFECTS:   VBV_buffer
 *
 * NOTES:    
 *
 *===========================================================================*/
void updateVBVBuffer (frameBits)
     int frameBits;
{
  if (VBV_delay) {
    VBV_delay -= frameDelayIncrement;
    if (VBV_delay < 0) {
      VBV_delay = 0;
    }
    
  } else {
    VBV_buffer -= frameBits;
  }
  VBV_buffer += bufferFillRate;
  if (VBV_buffer < 0) {
    fprintf(stderr, "\tWARNING - VBV buffer underflow (%d)\n", VBV_buffer);
  }
  if (VBV_buffer > buffer_size) {
    fprintf(stderr, "WARNING - VBV buffer overflow (%d > %d)\n",
        VBV_buffer, buffer_size);
  }
}
