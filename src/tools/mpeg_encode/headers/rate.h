/*===========================================================================*
 * rate.h                                     *
 *                                         *
 *    Procedures concerned with rate control
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *  getRateMode()
 *  setBitRate()
 *  getBitRate()
 *  setBufferSize()
 *  getBufferSize()
 *    initRateControl()
 *    targetRateControl()
 *     updateRateControl()
 *    MB_RateOut()
 *                                         *
 *===========================================================================*/

/*     COPYRIGHT INFO HERE    */

#define VARIABLE_RATE 0
#define FIXED_RATE 1


/*==================*
 * Exported VARIABLES *
 *==================*/


extern int rc_bitsThisMB;
extern int rc_numBlocks;
extern int rc_totalQuant;
extern int rc_quantOverride;


/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/

/*===========================================================================*
 *
 * initRateControl
 *
 *    initialize the allocation parameters.
 *===========================================================================*/
extern int initRateControl _ANSI_ARGS_((void));


/*===========================================================================*
 *
 * targetRateControl
 *
 *      Determine the target allocation for given picture type.
 *
 * RETURNS:     target size in bits
 *===========================================================================*/
extern void  targetRateControl _ANSI_ARGS_((MpegFrame   *frame));


/*===========================================================================*
 *
 * MB_RateOut
 *
 *      Prints out sampling of MB rate control data.  Every "nth" block
 *    stats are printed, with "n" controled by global RC_MB_SAMPLE_RATE
 *
 * RETURNS:     nothing
 *===========================================================================*/
extern void MB_RateOut _ANSI_ARGS_((int type));


/*===========================================================================*
 *
 * updateRateControl
 *
 *      Update the statistics kept, after end of frame
 *
 * RETURNS:     nothing
 *
 * SIDE EFFECTS:   many global variables
 *===========================================================================*/
extern void updateRateControl _ANSI_ARGS_((int type));


/*===========================================================================*
 *
 * needQScaleChange(current Q scale, 4 luminance blocks)
 *
 *
 * RETURNS:     new Qscale
 *===========================================================================*/
extern int needQScaleChange _ANSI_ARGS_((int oldQScale,  Block blk0, Block blk1, Block blk2, Block blk3));

/*===========================================================================*
 *
 * incNumBlocks()
 *
 *
 * RETURNS:   nothing
 *===========================================================================*/
extern void incNumBlocks _ANSI_ARGS_((int num));


/*===========================================================================*
 *
 * incMacroBlockBits()
 *
 *  Increments the number of Macro Block bits and the total of Frame
 *  bits by the number passed.
 *
 * RETURNS:   nothing
 *===========================================================================*/
extern void incMacroBlockBits _ANSI_ARGS_((int num));


/*===========================================================================*
 *
 * SetRateControl ()
 *
 *      Checks the string parsed from the parameter file.  Verifies
 *  number and sets global values.
 *
 * RETURNS:     nothing
 *===========================================================================*/
extern void SetRateControl _ANSI_ARGS_((char *charPtr));


/*===========================================================================*
 *
 * setBufferSize ()
 *
 *      Checks the string parsed from the parameter file.  Verifies
 *  number and sets global values.
 *
 * RETURNS:     nothing
 *===========================================================================*/
extern void setBufferSize _ANSI_ARGS_((char *charPtr));


/*===========================================================================*
 *
 * getBufferSize ()
 *
 *      returns the buffer size read from the parameter file.  Size is
 *  in bits- not in units of 16k as written to the sequence header.
 *
 * RETURNS:     int (or -1 if invalid)
 *===========================================================================*/
extern int getBufferSize _ANSI_ARGS_((void));


/*===========================================================================*
 *
 * setBitRate ()
 *
 *      Checks the string parsed from the parameter file.  Verifies
 *  number and sets global values.
 *
 * RETURNS:     nothing
 *
 * SIDE EFFECTS:   global variables
 *===========================================================================*/
extern void setBitRate _ANSI_ARGS_((char *charPtr));


/*===========================================================================*
 *
 * getBitRate ()
 *
 *      Returns the bit rate read from the parameter file.  This is the
 *  real rate in bits per second, not in 400 bit units as is written to
 *  the sequence header.
 *
 * RETURNS:     int (-1 if Variable mode operation)
 *===========================================================================*/
extern int getBitRate _ANSI_ARGS_((void));


/*===========================================================================*
 *
 * getRateMode ()
 *
 *      Returns the rate mode- interpreted waa either Fixed or Variable
 *
 * RETURNS:     integer
 *===========================================================================*/
extern int getRateMode _ANSI_ARGS_((void));


/*===========================================================================*
 *
 * incQuantOverride()
 *
 *  counter of override of quantization
 *
 * RETURNS:   nothing
 *===========================================================================*/
extern void incQuantOverride  _ANSI_ARGS_((int num));

