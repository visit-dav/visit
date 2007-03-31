/*===========================================================================*
 * frame.c                                     *
 *                                         *
 *    basic frame procedures                             *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    Frame_Init                                 *
 *    Frame_Exit                                 *
 *    Frame_New                                 *
 *    Frame_Free                                 *
 *    Frame_AllocPPM                                 *
 *    Frame_AllocBlocks                             *
 *    Frame_AllocYCC                                 *
 *    Frame_AllocDecoded                             *
 *    Frame_AllocHalf                                     *
 *    Frame_Resize                                     * 
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


/*==============*
 * HEADER FILES *
 *==============*/

#include "all.h"
#include "mtypes.h"
#include "frames.h"
#include "frame.h"
#include "fsize.h"
#include "dct.h"

/*===========*
 * CONSTANTS *
 *===========*/

/* The maximum number of B-Frames allowed between reference frames. */
#define  B_FRAME_RUN  16    

/*==================*
 * GLOBAL VARIABLES *
 *==================*/

MpegFrame      *frameMemory[B_FRAME_RUN+2];
extern boolean stdinUsed;
extern char    *framePattern;


/*===============================*
 * INTERNAL PROCEDURE prototypes *
 *===============================*/

static void FreeFrame _ANSI_ARGS_((MpegFrame * mf));
static MpegFrame *GetUnusedFrame _ANSI_ARGS_((void));
static void GetNumOfFrames _ANSI_ARGS_((int *numOfFrames));
static void ResetFrame _ANSI_ARGS_((int fnumber, int type, MpegFrame *frame));
static void Resize_Width _ANSI_ARGS_((MpegFrame *omfrw,MpegFrame *mfrw, int in_x,
       int in_y, int out_x));
static void Resize_Height _ANSI_ARGS_((MpegFrame *omfrh,MpegFrame *mfrh,
       int in_x,
       int in_y,  int out_y));
static void Resize_Array_Width _ANSI_ARGS_((uint8_mpeg_t **inarray,int in_x,
       int in_y,uint8_mpeg_t **outarray, int out_x));
static void Resize_Array_Height _ANSI_ARGS_((uint8_mpeg_t **inarray,int in_x,
       int in_y,uint8_mpeg_t **outarray, int out_y));


/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/

/*===============================================================
 *
 * Frame_Resize                  by James Boucher
 *                Boston University Multimedia Communications Lab
 *  
 *     This function takes the mf input frame, read in READFrame(),
 * and resizes all the input component arrays to the output
 * dimensions specified in the parameter file as OUT_SIZE.
 * The new frame is returned with the omf pointer.  As well,
 * the values of Fsize_x and Fsize_y are adjusted.
 ***************************************************************/
void
 Frame_Resize(omf,mf,insize_x,insize_y,outsize_x,outsize_y)
 MpegFrame *omf,*mf;
 int insize_x,insize_y,outsize_x,outsize_y;
{
MpegFrame *frameA;  /* intermediate frame */

frameA = (MpegFrame *)malloc(sizeof(MpegFrame));

if((insize_x != outsize_x)&&(insize_y != outsize_y)){
Resize_Width(frameA,mf,insize_x,insize_y,outsize_x);
Resize_Height(omf,frameA,outsize_x,insize_y,outsize_y);
}else 
if((insize_x ==outsize_x)&&(insize_y != outsize_y)){
Resize_Height(omf,mf,insize_x,insize_y,outsize_y);
} else
if((insize_x !=outsize_x)&&(insize_y == outsize_y)){
Resize_Width(omf,mf,insize_x,insize_y,outsize_x);
}
else{
  exit(1);
  }
/* Free memory */
free(frameA);
free(mf);
}
/*========================================================
* Resize_Width
*======================================================*/
static void  
Resize_Width(omfrw,mfrw,in_x,in_y, out_x)
MpegFrame *omfrw,*mfrw;
int in_x,in_y, out_x;
{
register int y;
int i;

omfrw->orig_y = NULL;
Fsize_x = out_x;
/* Allocate new frame memory */
    omfrw->orig_y = (uint8_mpeg_t **) malloc(sizeof(uint8_mpeg_t *) * Fsize_y);
    ERRCHK(omfrw->orig_y, "malloc");
    for (y = 0; y < Fsize_y; y++) {
    omfrw->orig_y[y] = (uint8_mpeg_t *) malloc(sizeof(uint8_mpeg_t) * out_x);
    ERRCHK(omfrw->orig_y[y], "malloc");
    }

    omfrw->orig_cr = (uint8_mpeg_t **) malloc(sizeof(int8_mpeg_t *) * Fsize_y / 2);
    ERRCHK(omfrw->orig_cr, "malloc");
    for (y = 0; y < Fsize_y / 2; y++) {
    omfrw->orig_cr[y] = (uint8_mpeg_t *) malloc(sizeof(int8_mpeg_t) * out_x / 2);
    ERRCHK(omfrw->orig_cr[y], "malloc");
    }

    omfrw->orig_cb = (uint8_mpeg_t **) malloc(sizeof(int8_mpeg_t *) * Fsize_y / 2);
    ERRCHK(omfrw->orig_cb, "malloc");
    for (y = 0; y < Fsize_y / 2; y++) {
    omfrw->orig_cb[y] = (uint8_mpeg_t *) malloc(sizeof(int8_mpeg_t) * out_x / 2);
    ERRCHK(omfrw->orig_cb[y], "malloc");
    }

    if ( referenceFrame == ORIGINAL_FRAME ) {
    omfrw->ref_y = omfrw->orig_y;
    omfrw->ref_cr = omfrw->orig_cr;
    omfrw->ref_cb = omfrw->orig_cb;
    }

/* resize each component array separately */
Resize_Array_Width(mfrw->orig_y,in_x,in_y,omfrw->orig_y,out_x);
Resize_Array_Width(mfrw->orig_cr,(in_x/2),(in_y/2),omfrw->orig_cr,(out_x/2));
Resize_Array_Width(mfrw->orig_cb,(in_x/2),(in_y/2),omfrw->orig_cb,(out_x/2));

/* Free old frame memory */
    if (mfrw->orig_y) {
    for (i = 0; i < in_y; i++) {
        free(mfrw->orig_y[i]);
    }
    free(mfrw->orig_y);

    for (i = 0; i < in_y / 2; i++) {
        free(mfrw->orig_cr[i]);
    }
    free(mfrw->orig_cr);

    for (i = 0; i < in_y / 2; i++) {
        free(mfrw->orig_cb[i]);
    }
    free(mfrw->orig_cb);
    }

}

/*=======================================================
* Resize_Height
*
*   Resize Frame height up or down
*=======================================================*/
static  void
Resize_Height(omfrh,mfrh,in_x,in_y,out_y)
MpegFrame *omfrh,*mfrh;
int in_x,in_y, out_y;
{
register int y; 
int i;

Fsize_y = out_y;

/* Allocate new frame memory */
    omfrh->orig_y = (uint8_mpeg_t **) malloc(sizeof(uint8_mpeg_t *) * out_y);
    ERRCHK(omfrh->orig_y, "malloc");
    for (y = 0; y < out_y; y++) {
    omfrh->orig_y[y] = (uint8_mpeg_t *) malloc(sizeof(uint8_mpeg_t) * Fsize_x);
    ERRCHK(omfrh->orig_y[y], "malloc");
    }

    omfrh->orig_cr = (uint8_mpeg_t **) malloc(sizeof(int8_mpeg_t *) * out_y / 2);
    ERRCHK(omfrh->orig_cr, "malloc");
    for (y = 0; y < out_y / 2; y++) {
    omfrh->orig_cr[y] = (uint8_mpeg_t *) malloc(sizeof(int8_mpeg_t) * Fsize_x / 2);
    ERRCHK(omfrh->orig_cr[y], "malloc");
    }

    omfrh->orig_cb = (uint8_mpeg_t **) malloc(sizeof(int8_mpeg_t *) * out_y / 2);
    ERRCHK(omfrh->orig_cb, "malloc");
    for (y = 0; y < out_y / 2; y++) {
    omfrh->orig_cb[y] = (uint8_mpeg_t *) malloc(sizeof(int8_mpeg_t) * Fsize_x / 2);
    ERRCHK(omfrh->orig_cb[y], "malloc");
    }

    if ( referenceFrame == ORIGINAL_FRAME ) {
    omfrh->ref_y = omfrh->orig_y;
    omfrh->ref_cr = omfrh->orig_cr;
    omfrh->ref_cb = omfrh->orig_cb;
    }

/* resize component arrays separately */
Resize_Array_Height(mfrh->orig_y,in_x,in_y,omfrh->orig_y,out_y);
Resize_Array_Height(mfrh->orig_cr,(in_x/2),(in_y/2),omfrh->orig_cr,(out_y/2));
Resize_Array_Height(mfrh->orig_cb,(in_x/2),(in_y/2),omfrh->orig_cb,(out_y/2));

/* Free old frame memory */
    if (mfrh->orig_y) {
    for (i = 0; i < in_y; i++) {
        free(mfrh->orig_y[i]);
    }
    free(mfrh->orig_y);

    for (i = 0; i < in_y / 2; i++) {
        free(mfrh->orig_cr[i]);
    }
    free(mfrh->orig_cr);

    for (i = 0; i < in_y / 2; i++) {
        free(mfrh->orig_cb[i]);
    }
    free(mfrh->orig_cb);
    }

}
/*====================================================
* Resize_Array_Width
*    
*   This function will resize any array width up
* or down in size.  The algorithm is based on the
* least common multiple approach more commonly
* used in audio frequency adjustments.
*=====================================================*/
static void 
Resize_Array_Width(inarray,in_x,in_y,outarray,out_x)
uint8_mpeg_t **inarray;
int in_x;
int in_y;
uint8_mpeg_t **outarray;
int out_x;
{
int i,j; 
int in_total;
int out_total;
uint8_mpeg_t *inptr;
uint8_mpeg_t *outptr;
uint8_mpeg_t pointA,pointB;
/* double slope,diff; */

 for(i=0;i<in_y;i++){     /* For every row */
  inptr = &inarray[i][0];
  outptr = &outarray[i][0];
  in_total = 0;
  out_total = 0;
  for(j=0;j<out_x;j++){      /* For every output value */
    if(in_total == out_total){  
      *outptr = *inptr;
      outptr++;
      out_total=out_total+in_x;
      while(in_total < out_total){
    in_total = in_total + out_x;
    inptr++;
      }
      if(in_total > out_total){
    in_total = in_total - out_x;
    inptr--;
      }
    } else {  
      pointA = *inptr;
      inptr++;
      pointB = *inptr;
      inptr--;
/*Interpolative solution */
/*      slope = ((double)(pointB -pointA))/((double)(out_x));
      diff = (((double)(out_total - in_total)));
      if(diff < (out_x/2)){
      *outptr = (pointA + (uint8_mpeg_t)(slope*diff));
    } else {
      *outptr = (pointB - (uint8_mpeg_t)(slope*(((float)(out_x)) - diff)));
    } */
/* Non-Interpolative solution */
    *outptr = *inptr;  

      outptr++;
      out_total=out_total+in_x;
      while(in_total < out_total){
    in_total = in_total + out_x;
    inptr++;
      }
      if(in_total > out_total){
    in_total = in_total - out_x;
    inptr--;
      }
    }  /* end if */
  }  /* end for each output value */

 }  /* end for each row */
}  /* end main */
/*==============================
* Resize_Array_Height
*
*    Resize any array height larger or smaller.
* Same as Resize_array_Width except pointer
* manipulation must change.
*===============================*/
static void 
Resize_Array_Height(inarray,in_x,in_y,outarray,out_y)
uint8_mpeg_t **inarray;
int in_x;
int in_y;
uint8_mpeg_t **outarray;
int out_y;
{
int i,j,k; 
int in_total;
int out_total;
uint8_mpeg_t pointA,pointB;
double slope,diff;

 for(i=0;i<in_x;i++){    /* for each column */
  in_total = 0;
  out_total = 0;
  k = 0;
  for(j=0;j<out_y;j++){  /* for each output value */
    if(in_total == out_total){  
      outarray[j][i] = inarray[k][i];
      out_total=out_total+in_y;
      while(in_total < out_total){
    in_total = in_total + out_y;
    k++;
      }
      if(in_total > out_total){
    in_total = in_total - out_y;
    k--;
      }
    } else {  
 
      pointA = inarray[k][i];
      if(k != (in_y -1)){
      pointB = inarray[k+1][i];
      } else {
      pointB = pointA;
      }
/* Interpolative case */
      slope = ((double)(pointB -pointA))/(double)(out_y);
      diff = (double)(out_total - in_total);
/*      outarray[j][i] = (inarray[k][i] + (uint8_mpeg_t)(slope*diff));
*/
/* Non-Interpolative case */
    outarray[j][i] = inarray[k][i];
      out_total=out_total+in_y;
      while(in_total < out_total){
    in_total = in_total + out_y;
    k++;
      }
      if(in_total > out_total){
    in_total = in_total - out_y;
    k--;
      }
    } 
  }
 }

}



/*===========================================================================*
 *
 * Frame_Init
 *
 *    initializes the memory associated with all frames ever
 *    If the input is not coming in from stdin, only 3 frames are needed ;
 *      else, the program must create frames equal to the greatest distance
 *      between two reference frames to hold the B frames while it is parsing
 *      the input from stdin.
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    frameMemory
 *
 *===========================================================================*/
void
Frame_Init()
{
    register int idx;
    int numOfFrames = 0;

    GetNumOfFrames(&numOfFrames);

    for ( idx = 0; idx < numOfFrames; idx++ ) {
    frameMemory[idx] = (MpegFrame *) malloc(sizeof(MpegFrame));
    frameMemory[idx]->inUse = FALSE;
    frameMemory[idx]->ppm_data = NULL;
    frameMemory[idx]->rgb_data = NULL;
    frameMemory[idx]->orig_y = NULL;    /* if NULL, then orig_cr, orig_cb invalid */
    frameMemory[idx]->y_blocks = NULL; /* if NULL, then cr_blocks, cb_blocks invalid */
    frameMemory[idx]->decoded_y = NULL;    /* if NULL, then blah blah */
    frameMemory[idx]->halfX = NULL;
    frameMemory[idx]->next = NULL;
    }

#ifdef BLEAH
fprintf (stderr, "%d frames allocated.\n", numOfFrames);
#endif
}


/*===========================================================================*
 *
 * Frame_Exit
 *
 *    frees the memory associated with frames
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    frameMemory
 *
 *===========================================================================*/
void
Frame_Exit()
{
    register int idx;
    int numOfFrames = 0;

    GetNumOfFrames(&numOfFrames);

    for ( idx = 0; idx < numOfFrames; idx++ ) {
    FreeFrame(frameMemory[idx]);
    }
}


/*===========================================================================*
 *
 * Frame_Free
 *
 *    frees the given frame -- allows it to be re-used
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Frame_Free(frame)
    MpegFrame *frame;
{
    frame->inUse = FALSE;
}


/*===========================================================================*
 *
 * Frame_New
 *
 *    finds a frame that isn't currently being used and resets it
 *
 * RETURNS:    the frame
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
MpegFrame *
Frame_New(id, type)
    int id;
    int type;
{
    MpegFrame *frame;

    frame = GetUnusedFrame();
    ResetFrame(id, type, frame);

    return frame;
}


/*===========================================================================*
 *
 * Frame_AllocPPM
 *
 *    allocate memory for ppm data for the given frame, if required
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Frame_AllocPPM(frame)
    MpegFrame *frame;
{
    register int y;

    if ( frame->ppm_data != NULL ) {    /* already allocated */
    return;
    }

    frame->ppm_data = (uint8_mpeg_t **) malloc(sizeof(uint8_mpeg_t *) * Fsize_y);
    ERRCHK(frame->ppm_data, "malloc");

    for ( y = 0; y < Fsize_y; y++ ) {
    frame->ppm_data[y] = (uint8_mpeg_t *) malloc(3*sizeof(uint8_mpeg_t) * Fsize_x);
    ERRCHK(frame->ppm_data[y], "malloc");
    }
}


/*===========================================================================*
 *
 * Frame_AllocBlocks
 *
 *    allocate memory for blocks for the given frame, if required
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Frame_AllocBlocks(frame)
    MpegFrame *frame;
{
    int dctx, dcty;
    int i;

    if ( frame->y_blocks != NULL ) {        /* already allocated */
    return;
    }

    dctx = Fsize_x / DCTSIZE;
    dcty = Fsize_y / DCTSIZE;

    frame->y_blocks = (Block **) malloc(sizeof(Block *) * dcty);
    ERRCHK(frame->y_blocks, "malloc");
    for (i = 0; i < dcty; i++) {
    frame->y_blocks[i] = (Block *) malloc(sizeof(Block) * dctx);
    ERRCHK(frame->y_blocks[i], "malloc");
    }

    frame->cr_blocks = (Block **) malloc(sizeof(Block *) * (dcty >> 1));
    frame->cb_blocks = (Block **) malloc(sizeof(Block *) * (dcty >> 1));
    ERRCHK(frame->cr_blocks, "malloc");
    ERRCHK(frame->cb_blocks, "malloc");
    for (i = 0; i < (dcty >> 1); i++) {
    frame->cr_blocks[i] = (Block *) malloc(sizeof(Block) * (dctx >> 1));
    frame->cb_blocks[i] = (Block *) malloc(sizeof(Block) * (dctx >> 1));
    ERRCHK(frame->cr_blocks[i], "malloc");
    ERRCHK(frame->cb_blocks[i], "malloc");
    }
}


/*===========================================================================*
 *
 * Frame_AllocYCC
 *
 *    allocate memory for YCC info for the given frame, if required
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Frame_AllocYCC(frame)
    MpegFrame *frame;
{
    register int y;

    if ( frame->orig_y != NULL ) {    /* already allocated */
    return /* nothing */ ;
    }

    DBG_PRINT(("ycc_calc:\n"));
    /*
     * first, allocate tons of memory
     */
    frame->orig_y = (uint8_mpeg_t **) malloc(sizeof(uint8_mpeg_t *) * Fsize_y);
    ERRCHK(frame->orig_y, "malloc");
    for (y = 0; y < Fsize_y; y++) {
    frame->orig_y[y] = (uint8_mpeg_t *) malloc(sizeof(uint8_mpeg_t) * Fsize_x);
    ERRCHK(frame->orig_y[y], "malloc");
    }

    frame->orig_cr = (uint8_mpeg_t **) malloc(sizeof(int8_mpeg_t *) * (Fsize_y >> 1));
    ERRCHK(frame->orig_cr, "malloc");
    for (y = 0; y < (Fsize_y >> 1); y++) {
    frame->orig_cr[y] = (uint8_mpeg_t *) malloc(sizeof(int8_mpeg_t) * (Fsize_x >> 1));
    ERRCHK(frame->orig_cr[y], "malloc");
    }

    frame->orig_cb = (uint8_mpeg_t **) malloc(sizeof(int8_mpeg_t *) * (Fsize_y >> 1));
    ERRCHK(frame->orig_cb, "malloc");
    for (y = 0; y < (Fsize_y >> 1); y++) {
    frame->orig_cb[y] = (uint8_mpeg_t *) malloc(sizeof(int8_mpeg_t) * (Fsize_x >> 1));
    ERRCHK(frame->orig_cb[y], "malloc");
    }

    if ( referenceFrame == ORIGINAL_FRAME ) {
    frame->ref_y = frame->orig_y;
    frame->ref_cr = frame->orig_cr;
    frame->ref_cb = frame->orig_cb;
    }
}



/*===========================================================================*
 *
 * Frame_AllocHalf
 *
 *    allocate memory for half-pixel values for the given frame, if required
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Frame_AllocHalf(frame)
    MpegFrame *frame;
{
    register int y;

    if ( frame->halfX != NULL ) {
        return;
    }

    frame->halfX = (uint8_mpeg_t **) malloc(Fsize_y*sizeof(uint8_mpeg_t *));
    ERRCHK(frame->halfX, "malloc");
    frame->halfY = (uint8_mpeg_t **) malloc((Fsize_y-1)*sizeof(uint8_mpeg_t *));
    ERRCHK(frame->halfY, "malloc");
    frame->halfBoth = (uint8_mpeg_t **) malloc((Fsize_y-1)*sizeof(uint8_mpeg_t *));
    ERRCHK(frame->halfBoth, "malloc");
    for ( y = 0; y < Fsize_y; y++ ) {
        frame->halfX[y] = (uint8_mpeg_t *) malloc((Fsize_x-1)*sizeof(uint8_mpeg_t));
        ERRCHK(frame->halfX[y], "malloc");
    }
    for ( y = 0; y < Fsize_y-1; y++ ) {
        frame->halfY[y] = (uint8_mpeg_t *) malloc(Fsize_x*sizeof(uint8_mpeg_t));
        ERRCHK(frame->halfY[y], "malloc");
    }
    for ( y = 0; y < Fsize_y-1; y++ ) {
        frame->halfBoth[y] = (uint8_mpeg_t *) malloc((Fsize_x-1)*sizeof(uint8_mpeg_t));
        ERRCHK(frame->halfBoth[y], "malloc");
    }
}


/*===========================================================================*
 *
 * Frame_AllocDecoded
 *
 *    allocate memory for decoded frame for the given frame, if required
 *    if makeReference == TRUE, then makes it reference frame
 * 
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Frame_AllocDecoded(frame, makeReference)
    MpegFrame *frame;
    boolean makeReference;
{
    register int y;

    if ( frame->decoded_y != NULL) {    /* already allocated */
    return;
    }

    /* allocate memory for decoded image */
    /* can probably reuse original image memory, but may decide to use
       it for some reason, so do it this way at least for now -- more
       flexible
     */
    frame->decoded_y = (uint8_mpeg_t **) malloc(sizeof(uint8_mpeg_t *) * Fsize_y);
    ERRCHK(frame->decoded_y, "malloc");
    for (y = 0; y < Fsize_y; y++) {
    frame->decoded_y[y] = (uint8_mpeg_t *) malloc(sizeof(uint8_mpeg_t) * Fsize_x);
    ERRCHK(frame->decoded_y[y], "malloc");
    }

    frame->decoded_cr = (uint8_mpeg_t **) malloc(sizeof(int8_mpeg_t *) * (Fsize_y >> 1));
    ERRCHK(frame->decoded_cr, "malloc");
    for (y = 0; y < (Fsize_y >> 1); y++) {
    frame->decoded_cr[y] = (uint8_mpeg_t *) malloc(sizeof(uint8_mpeg_t) * (Fsize_x >> 1));
    ERRCHK(frame->decoded_cr[y], "malloc");
    }

    frame->decoded_cb = (uint8_mpeg_t **) malloc(sizeof(int8_mpeg_t *) * (Fsize_y >> 1));
    ERRCHK(frame->decoded_cb, "malloc");
    for (y = 0; y < (Fsize_y >> 1); y++) {
    frame->decoded_cb[y] = (uint8_mpeg_t *) malloc(sizeof(uint8_mpeg_t) * (Fsize_x >> 1));
    ERRCHK(frame->decoded_cb[y], "malloc");
    }

    if ( makeReference ) {
    frame->ref_y = frame->decoded_y;
    frame->ref_cr = frame->decoded_cr;
    frame->ref_cb = frame->decoded_cb;
    }
}


/*=====================*
 * INTERNAL PROCEDURES *
 *=====================*/


/*===========================================================================*
 *
 * GetUnusedFrame
 *
 *    return an unused frame
 *
 * RETURNS:    the frame
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static MpegFrame *
GetUnusedFrame()
{
    register int idx;
    int numOfFrames;

    GetNumOfFrames(&numOfFrames);

    for ( idx = 0; idx < numOfFrames; idx++ ) {
    if ( ! frameMemory[idx]->inUse ) {
        frameMemory[idx]->inUse = TRUE;
        return frameMemory[idx];
    }
    }

    fprintf(stderr, "ERROR:  No unused frames!!!\n");
    fprintf(stderr, "        If you are using stdin for input, it is likely that you have too many\n");
    fprintf(stderr, "        B-frames between two reference frames.  See the man page for help.\n");
    exit(1);
}


/*===========================================================================*
 *
 * GetNumOfFrames
 *
 *    return the number of frames to allocate
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    numOfFrames contains the number to allocate
 *
 *===========================================================================*/
static void
GetNumOfFrames(numOfFrames)
    int *numOfFrames;
{
    int idx, bcount;

    if (stdinUsed) {
      for ( idx = 0, bcount = 0; idx < strlen(framePattern); idx++) {

    /* counts the maximum number of B frames between two reference
     * frames. 
     */

    switch( framePattern[idx] ) {
      case 'b': 
        bcount++;
        break;
      case 'i':
      case 'p':
            if (bcount > *numOfFrames) {
              *numOfFrames = bcount;
            }
        bcount = 0;
        break;
        }

    /* add 2 to hold the forward and past reference frames in addition
     * to the maximum number of B's 
     */
      }

      *numOfFrames += 2;

    } else {
      /* non-interactive, only 3 frames needed */
      *numOfFrames = 3;
    }
}

/*===========================================================================*
 *
 * ResetFrame
 *
 *    reset a frame to the given id and type
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
ResetFrame(id, type, frame)
    int id;
    int type;
    MpegFrame *frame;
{
    switch (type) {
    case 'i':
    frame->type = TYPE_IFRAME;
    break;
    case 'p':
    frame->type = TYPE_PFRAME;
    break;
    case 'b':
    frame->type = TYPE_BFRAME;
    break;
    default:
    fprintf(stderr, "frame type %c: not supported\n", type);
    exit(1);
    }

    frame->id = id;
    frame->halfComputed = FALSE;
    frame->next = NULL;
}


/*===========================================================================*
 *
 * FreeFrame
 *
 *    frees the memory associated with the given frame
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
FreeFrame(frame)
    MpegFrame *frame;
{
    int i;

    if (!frame) {
    return;
    }

    if ( frame->ppm_data ) {
    /* it may be a little bigger than Fsize_y, but that's fine for
       our purposes, since we aren't going to free until we exit anyway,
       so by the time we call this we won't care
     */
    pnm_freearray(frame->ppm_data, Fsize_y);
    frame->ppm_data = NULL;
    }

    if (frame->rgb_data) {
    pnm_freearray(frame->rgb_data, Fsize_y);
    }
    if (frame->orig_y) {
    for (i = 0; i < Fsize_y; i++) {
        free(frame->orig_y[i]);
    }
    free(frame->orig_y);

    for (i = 0; i < (Fsize_y >> 1); i++) {
        free(frame->orig_cr[i]);
    }
    free(frame->orig_cr);

    for (i = 0; i < (Fsize_y >> 1); i++) {
        free(frame->orig_cb[i]);
    }
    free(frame->orig_cb);
    }
    if ( frame->decoded_y ) {
    for (i = 0; i < Fsize_y; i++) {
        free(frame->decoded_y[i]);
    }
    free(frame->decoded_y);

    for (i = 0; i < (Fsize_y >> 1); i++) {
        free(frame->decoded_cr[i]);
    }
    free(frame->decoded_cr);

    for (i = 0; i < (Fsize_y >> 1); i++) {
        free(frame->decoded_cb[i]);
    }
    free(frame->decoded_cb);
    }

    if (frame->y_blocks) {
    for (i = 0; i < Fsize_y / DCTSIZE; i++) {
        free(frame->y_blocks[i]);
    }
    free(frame->y_blocks);

    for (i = 0; i < Fsize_y / (2 * DCTSIZE); i++) {
        free(frame->cr_blocks[i]);
    }
    free(frame->cr_blocks);

    for (i = 0; i < Fsize_y / (2 * DCTSIZE); i++) {
        free(frame->cb_blocks[i]);
    }
    free(frame->cb_blocks);
    }
    if ( frame->halfX ) {
    for ( i = 0; i < Fsize_y; i++ ) {
        free(frame->halfX[i]);
    }
    free(frame->halfX);

    for ( i = 0; i < Fsize_y-1; i++ ) {
        free(frame->halfY[i]);
    }
    free(frame->halfY);

    for ( i = 0; i < Fsize_y-1; i++ ) {
        free(frame->halfBoth[i]);
    }
    free(frame->halfBoth);
    }

        
    free(frame);
}


