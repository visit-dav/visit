/*===========================================================================*
 * mpeg.c                                     *
 *                                         *
 *    Procedures to generate the MPEG sequence                 *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    GetMPEGStream                                 *
 *    IncrementTCTime                                 *
 *    SetStatFileName                                 *
 *    SetGOPSize                                 *
 *    PrintStartStats                                 *
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
 *  $Header: /n/picasso/project/mpeg/mpeg_dist/mpeg_encode/RCS/mpeg.c,v 1.24 1995/08/16 18:10:48 smoot Exp $
 *  $Log: mpeg.c,v $
 *  Revision 1.24  1995/08/16 18:10:48  smoot
 *  *** empty log message ***
 *
 *  Revision 1.23  1995/08/07 21:48:08  smoot
 *  stdin bugs fixed
 *
 *  Revision 1.22  1995/06/26 21:49:19  smoot
 *  added new frame ordering (hacks)^H^H^H^H^H code ;-)
 *
 *  Revision 1.21  1995/06/21 18:30:41  smoot
 *  changed time structure to be ANSI
 *  changed file access to be binary (DOS!)
 *  added time to userdata
 *  Added a sleep to remote reads (NFS delay)
 *
 * Revision 1.20  1995/05/02  01:49:21  eyhung
 * prints out true output bit rate and slightly untabified
 *
 * Revision 1.19  1995/05/02  00:45:35  eyhung
 * endstats now contain correct output fbit rate at the specified frame rate
 *
 * Revision 1.18  1995/03/27  23:43:20  smoot
 * killed printing long as int (compiler warning)
 *
 * Revision 1.17  1995/03/27  19:18:54  smoot
 * fixed divide by zero for very quick encodings
 *
 * Revision 1.16  1995/02/02  22:03:37  smoot
 * added types for MIPS
 *
 * Revision 1.15  1995/02/02  07:26:58  eyhung
 * removed unused tempframe
 *
 * Revision 1.14  1995/02/01  05:01:35  eyhung
 * Completed infinite coding-on-the-fly
 *
 * Revision 1.13  1995/02/01  02:34:02  eyhung
 * Added full coding-on-the-fly
 *
 * Revision 1.12  1995/01/31  23:05:14  eyhung
 * Added some stdin stuff
 *
 * Revision 1.11  1995/01/20  00:01:16  eyhung
 * Added output file to PrintEndStats
 *
 * Revision 1.10  1995/01/19  23:08:51  eyhung
 * Changed copyrights
 *
 * Revision 1.9  1995/01/17  18:55:54  smoot
 * added right version number, and error if no frames selected
 *
 * Revision 1.8  1995/01/16  08:12:54  eyhung
 * added realQuiet
 *
 * Revision 1.7  1994/12/07  00:40:36  smoot
 * Added seperate P and B search ranges
 *
 * Revision 1.6  1994/11/28  21:46:45  smoot
 * Added version printing
 *
 * Revision 1.5  1994/11/19  01:33:05  smoot
 * put in userdata
 *
 * Revision 1.4  1994/11/14  22:36:22  smoot
 * Merged specifics and rate control
 *
 * Revision 1.2  1994/03/15  00:27:11  keving
 * nothing
 *
 * Revision 1.1  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.6  1993/07/22  22:23:43  keving
 * nothing
 *
 * Revision 1.5  1993/06/30  20:06:09  keving
 * nothing
 *
 * Revision 1.4  1993/06/03  21:08:08  keving
 * nothing
 *
 * Revision 1.3  1993/02/19  18:10:12  keving
 * nothing
 *
 * Revision 1.2  1993/02/17  23:18:20  dwallach
 * checkin prior to keving's joining the project
 *
 */


/*==============*
 * HEADER FILES *
 *==============*/

#include "all.h"
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include "mtypes.h"
#include "frames.h"
#include "search.h"
#include "mpeg.h"
#include "prototypes.h"
#include "parallel.h"
#include "param.h"
#include "readframe.h"
#include "fsize.h"
#include "mheaders.h"
#include "rate.h"
#ifdef MIPS
#include <sys/types.h>
#endif
#include <sys/stat.h>

/*===========*
 *  VERSION  *
 *===========*/

#define VERSION "1.5b"


/*===========*
 * CONSTANTS *
 *===========*/

#define    FPS_30    0x5   /* from MPEG standard sect. 2.4.3.2 */
#define ASPECT_1    0x1    /* aspect ratio, from MPEG standard sect. 2.4.3.2 */


/*==================*
 * STATIC VARIABLES *
 *==================*/

static int32_mpeg_t   diffTime;
static int framesOutput;
static int        realStart, realEnd;
static int    currentGOP;
static int        timeMask;
static int        numI, numP, numB;


/*==================*
 * GLOBAL VARIABLES *    
 *==================*/

/* important -- don't initialize anything here */
/* must be re-initted anyway in GenMPEGStream */

extern int  IOtime;
extern boolean    resizeFrame;
extern int outputWidth, outputHeight;
int        gopSize = 100;  /* default */
int32_mpeg_t        tc_hrs, tc_min, tc_sec, tc_pict, tc_extra;
int        totalFramesSent;
int        yuvWidth, yuvHeight;
int        realWidth, realHeight;
char        currentPath[MAXPATHLEN];
char        statFileName[256];
char        bitRateFileName[256];
time_t        timeStart, timeEnd;
FILE       *statFile;
FILE       *bitRateFile = NULL;
char       *framePattern;
int        framePatternLen;
int        referenceFrame;
static int  framesRead;
MpegFrame  *pastRefFrame;
MpegFrame  *futureRefFrame;
int        frameRate = FPS_30;
int        frameRateRounded = 30;
boolean        frameRateInteger = TRUE;
int        aspectRatio = ASPECT_1;
extern unsigned char userDataFileName[];
extern int mult_seq_headers;

int32_mpeg_t bit_rate, buf_size;

/*===============================*
 * INTERNAL PROCEDURE prototypes *
 *===============================*/

static void    ShowRemainingTime _ANSI_ARGS_((void));
static void    ComputeDHMSTime _ANSI_ARGS_((int32_mpeg_t someTime, char *timeText));
static void    ComputeGOPFrames _ANSI_ARGS_((int whichGOP, int *firstFrame,
                          int *lastFrame, int numFrames));
static void    PrintEndStats _ANSI_ARGS_((int inputFrameBits, int32_mpeg_t totalBits));
static void    ProcessRefFrame _ANSI_ARGS_((MpegFrame *frame,
                          BitBucket *bb, int lastFrame,
                          char *outputFileName));
static void    OpenBitRateFile _ANSI_ARGS_((void));
static void    CloseBitRateFile _ANSI_ARGS_((void));


/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/

/*===========================================================================*
 *
 * SetReferenceFrameType
 *
 *    set the reference frame type to be original or decoded
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    referenceFrame
 *
 *===========================================================================*/
void
SetReferenceFrameType(type)
    char *type;
{
    if ( strcmp(type, "ORIGINAL") == 0 ) {
    referenceFrame = ORIGINAL_FRAME;
    } else if ( strcmp(type, "DECODED") == 0 ) {
    referenceFrame = DECODED_FRAME;
    } else {
    fprintf(stderr, "ERROR:  Illegal reference frame type: '%s'\n",
        type);
    exit(1);
    }
}

void
SetBitRateFileName(fileName)
    char *fileName;
{
    strcpy(bitRateFileName, fileName);
}


/*===========================================================================*
 *
 * GenMPEGStream
 *
 *    generate an MPEG sequence stream (generally)
 *    if whichGOP == frameStart == -1 then does complete MPEG sequence
 *    if whichGOP != -1 then does numbered GOP only (without sequence
 *                   header)
 *    if frameStart != -1 then does numbered frames only (without any
 *                 sequence or GOP headers)               
 *
 * RETURNS:    amount of time it took
 *
 * SIDE EFFECTS:    too numerous to mention
 *
 *===========================================================================*/
int32_mpeg_t
GenMPEGStream(whichGOP, frameStart, frameEnd, qtable, niqtable, numFrames,
              ofp, outputFileName)
    int whichGOP;
    int frameStart;
    int frameEnd;
    int32_mpeg_t   qtable[];
    int32_mpeg_t   niqtable[];
    int numFrames;
    FILE *ofp;
    char *outputFileName;
{
    extern void        PrintItoIBitRate _ANSI_ARGS_((int numBits, int frameNum));
    BitBucket *bb;
    int i;
    char frameType;
    MpegFrame        *frame = NULL;
    MpegFrame *tempFrame;
    int            firstFrame, lastFrame;
    int     inputFrameBits = 0;
    char    inputFileName[1024];
    time_t  tempTimeStart, tempTimeEnd;
    boolean firstFrameDone = FALSE;
    int numBits;
    int32_mpeg_t bitstreamMode, res;

    if ( (whichGOP == -1) && (frameStart == -1) &&
         (! stdinUsed) && (FType_Type(numFrames-1) == 'b') ) {
        fprintf(stderr, "\n");
        fprintf(stderr, "WARNING:  One or more B-frames at end will not be encoded.\n");
        fprintf(stderr, "          See FORCE_ENCODE_LAST_FRAME option in man page.\n");
        fprintf(stderr, "\n");
    }

    time(&timeStart);

    framesRead = 0;

    ResetIFrameStats();
    ResetPFrameStats();
    ResetBFrameStats();

    Fsize_Reset();

    framesOutput = 0;

    if ( childProcess && separateConversion ) {
        SetFileType(slaveConversion);
    } else {
        SetFileType(inputConversion);
    }

    if ( whichGOP != -1 ) {
        ComputeGOPFrames(whichGOP, &firstFrame, &lastFrame, numFrames);

        realStart = firstFrame;
        realEnd = lastFrame;

        if ( FType_Type(firstFrame) == 'b' ) {

            /* can't find the previous frame interactively */
            if ( stdinUsed ) {
                fprintf(stderr, "ERROR: Cannot encode GOP from stdin when first frame is a B-frame.\n");
                exit(1);
            }

            /* need to load in previous frame; call it an I frame */
            frame = Frame_New(firstFrame-1, 'i');

            time(&tempTimeStart);

            if ( (referenceFrame == DECODED_FRAME) &&
                 childProcess ) {
                WaitForDecodedFrame(firstFrame);

                if ( remoteIO ) {
                    GetRemoteDecodedRefFrame(frame, firstFrame-1);
                } else {
                    ReadDecodedRefFrame(frame, firstFrame-1);
                }
            } else {
                if ( remoteIO ) {
                    GetRemoteFrame(frame, firstFrame-1);
                } else {
                    GetNthInputFileName(inputFileName, firstFrame-1);

                    if ( childProcess && separateConversion ) {
                        ReadFrame(frame, inputFileName, slaveConversion, TRUE);
                    } else {
                        ReadFrame(frame, inputFileName, inputConversion, TRUE);
                    }
                }
            }

            framesRead++;

            time(&tempTimeEnd);
            IOtime += (tempTimeEnd-tempTimeStart);
        }
    } else if ( frameStart != -1 ) {
        if ( frameEnd > numFrames-1 ) {
            fprintf(stderr, "ERROR:  Specified last frame is out of bounds\n");
            exit(1);
        }

        realStart = frameStart;
        realEnd = frameEnd;

        firstFrame = frameStart;
        lastFrame = frameEnd;

        /* if first frame is P or B, need to read in P or I frame before it */
        if ( FType_Type(firstFrame) != 'i' ) {

            /* can't find the previous frame interactively */
            if ( stdinUsed ) {
                fprintf(stderr, "ERROR: Cannot encode frames from stdin when first frame is not an I-frame.\n");
                exit(1);
            }

            firstFrame = FType_PastRef(firstFrame);
        }

        /* if last frame is B, need to read in P or I frame after it */
        if ( (FType_Type(lastFrame) == 'b') && (lastFrame != numFrames-1) ) {

            /* can't find the next reference frame interactively */
            if ( stdinUsed ) {
                fprintf(stderr, "ERROR: Cannot encode frames from stdin when last frame is a B-frame.\n");
                exit(1);
            }

            lastFrame = FType_FutureRef(lastFrame);
        }

        if ( lastFrame > numFrames-1 ) {            /* can't go last frame! */
            lastFrame = numFrames-1;
        }

    } else {
        firstFrame = 0;
        lastFrame = numFrames-1;

        realStart = 0;
        realEnd = numFrames-1;
        if ( numFrames == 0 )  {
            fprintf(stderr, "ERROR:  No frames selected!\n");
            exit(1);
        }
    }

    /* count number of I, P, and B frames */
    numI = 0;        numP = 0;   numB = 0;
    timeMask = 0;
    if (stdinUsed) {
      numI = numP = numB = MAXINT/4;
    } else {
      for ( i = firstFrame; i <= lastFrame; i++ ) {
        frameType = FType_Type(i);
        switch(frameType) {
    case 'i':        numI++;            timeMask |= 0x1;    break;
    case 'p':        numP++;            timeMask |= 0x2;        break;
    case 'b':        numB++;            timeMask |= 0x4;        break;
        }
      }
    }

    if ( ! childProcess ) {
        if ( showBitRatePerFrame )
            OpenBitRateFile();
        PrintStartStats(realStart, realEnd);
    }

    if ( frameStart == -1 ) {
        bb = Bitio_New(ofp);
    } else {
        bb = NULL;
    }

    tc_hrs = 0;        tc_min = 0; tc_sec = 0; tc_pict = 0; tc_extra = 0;
    for ( i = 0; i < firstFrame; i++ ) {
        IncrementTCTime();
    }

    totalFramesSent = firstFrame;
    currentGOP = gopSize;        /* so first I-frame generates GOP Header */

    /* Rate Control Initialization  */
    bitstreamMode = getRateMode();
    if (bitstreamMode == FIXED_RATE) {
      res = initRateControl();
      /*
        SetFrameRate();
        */
         }
    
#ifdef BLEAH
fprintf(stdout, "firstFrame, lastFrame = %d, %d;  real = %d, %d\n",
        firstFrame, lastFrame, realStart, realEnd);
fflush(stdout);
#endif

    pastRefFrame = NULL;
    futureRefFrame = NULL;
    for ( i = firstFrame; i <= lastFrame; i++) {

        /* break out of the near-infinite loop if input from stdin is done */
#if 0 
      char eofcheck[1];
      if ( stdinUsed ) {
    if (scanf("%c", eofcheck) != EOF) {
      ungetc(eofcheck[0], stdin);
    } else {
      break;
    }
      }
#else
    /*
     ** For some reason the above version of this stdin EOF check does not
     ** work right with jpeg files, the ungetc() is not padding anything to
     ** stdin, I have no idea why (perhaps because a char is passed instead
     ** of an int?), and it drove me nuts, so I wrote my own, slightly
     ** cleaner version, and this one seems to work.
     ** Dave Scott (dhs), UofO, 7/19/95.
     */
    if ( stdinUsed) {
      int eofcheck_;
      eofcheck_ = fgetc(stdin);
      if ( eofcheck_ == EOF) 
        break;
      else 
        ungetc(eofcheck_, stdin);
    } 
#endif
        frameType = FType_Type(i);
    
        time(&tempTimeStart);

        /* skip non-reference frames if non-interactive
         * read in non-reference frames if interactive */
        if ( frameType == 'b' ) {
          if ( stdinUsed ) {
            frame = Frame_New(i, frameType);
            ReadFrame(frame, "stdin", inputConversion, TRUE);

            framesRead++;

            time(&tempTimeEnd);
            IOtime += (tempTimeEnd-tempTimeStart);

            /* Add the B frame to the end of the queue of B-frames 
             * for later encoding
             */

            if (futureRefFrame != NULL) {
              tempFrame = futureRefFrame;
              while (tempFrame->next != NULL) {
        tempFrame = tempFrame->next;
              }
          tempFrame->next = frame;
            } else {
          fprintf(stderr, "Yow, something wrong in neverland! (hit bad code in mpeg.c\n");
        }
          }
          continue;
        }

        frame = Frame_New(i, frameType);

        pastRefFrame = futureRefFrame;
        futureRefFrame = frame;

        if ( (referenceFrame == DECODED_FRAME) &&
             ((i < realStart) || (i > realEnd)) ) {
            WaitForDecodedFrame(i);

            if ( remoteIO ) {
                GetRemoteDecodedRefFrame(frame, i);
            } else {
                ReadDecodedRefFrame(frame, i);
            }
        } else {
            if ( remoteIO ) {
                GetRemoteFrame(frame, i);
            } else {
                GetNthInputFileName(inputFileName, i);
                if ( childProcess && separateConversion ) {
                    ReadFrame(frame, inputFileName, slaveConversion, TRUE);
                } else {
                    ReadFrame(frame, inputFileName, inputConversion, TRUE);
                }
            }
        }

        framesRead++;

        time(&tempTimeEnd);
        IOtime += (tempTimeEnd-tempTimeStart);

        if ( ! firstFrameDone ) {
          char *userData = (char *)NULL;
          int userDataSize = 0;

          inputFrameBits = 24*Fsize_x*Fsize_y;
          SetBlocksPerSlice();
          
          if ( (whichGOP == -1) && (frameStart == -1) ) {
            DBG_PRINT(("Generating sequence header\n"));
            bitstreamMode = getRateMode();
            if (bitstreamMode == FIXED_RATE) {
              bit_rate = getBitRate();
              buf_size = getBufferSize();
            }
            else {
              bit_rate = -1;
              buf_size = -1;
            }
        
        if (strlen(userDataFileName) != 0) {
          struct stat statbuf;
          FILE *fp;
          
          stat(userDataFileName,&statbuf);
          userDataSize = statbuf.st_size;
          userData = malloc(userDataSize);
          if ((fp = fopen(userDataFileName,"rb")) == NULL) {
        fprintf(stderr,"Could not open userdata file-%s.\n",
            userDataFileName);
        userData = NULL;
        userDataSize = 0;
        goto write;
          }
        if (fread(userData,1,userDataSize,fp) != userDataSize) {
            fprintf(stderr,"Could not read %d bytes from userdata file-%s.\n",
                    userDataSize,userDataFileName);
            userData = NULL;
            userDataSize = 0;
            goto write;
          }
        } else { /* Put in our UserData Header */
      time_t now;

      time(&now);
          userData = malloc(100);
          sprintf(userData,"MPEG stream encoded by UCB Encoder (mpeg_encode) v%s on %s.",
          VERSION, ctime(&now));
          userDataSize = strlen(userData);
        }
          write:
            Mhead_GenSequenceHeader(bb, Fsize_x, Fsize_y,
                                    /* pratio */ aspectRatio,
                                    /* pict_rate */ frameRate, /* bit_rate */ bit_rate,
                                    /* buf_size */ buf_size, /*c_param_flag */ 1,
                                    /* iq_matrix */ qtable, /* niq_matrix */ niqtable,
                                    /* ext_data */ NULL, /* ext_data_size */ 0,
                                    /* user_data */ userData, /* user_data_size */ userDataSize);
          }
          
          firstFrameDone = TRUE;
        }
        
        ProcessRefFrame(frame, bb, lastFrame, outputFileName);

    }

    if ( frame != NULL ) {
        Frame_Free(frame);
    }

    /* SEQUENCE END CODE */
    if ( (whichGOP == -1) && (frameStart == -1) ) {
        Mhead_GenSequenceEnder(bb);
    }

    if ( frameStart == -1 ) {
      /* I think this is right, since (bb == NULL) if (frameStart != -1).
         See above where "bb" is initialized  */
      numBits = bb->cumulativeBits;
    } else {
      /* What should the correct value be?  Most likely 1.  "numBits" is
         used below, so we need to make sure it's properly initialized 
       to somthing (anything).  */
      numBits = 1;
    }

    if ( frameStart == -1 ) {
        Bitio_Flush(bb);
        bb = NULL;
        fclose(ofp);

        time(&timeEnd);
        diffTime = (int32_mpeg_t)(timeEnd-timeStart);

        if ( ! childProcess ) {
            PrintEndStats(inputFrameBits, numBits);
        }
    } else {
        time(&timeEnd);
        diffTime = (int32_mpeg_t)(timeEnd-timeStart);

        if ( ! childProcess ) {
            PrintEndStats(inputFrameBits, 1);
        }
    }

    if ( FType_Type(realEnd) != 'i' ) {
        PrintItoIBitRate(numBits, realEnd+1);
    }

    if ( (! childProcess) && showBitRatePerFrame )
        CloseBitRateFile();

#ifdef BLEAH
    if ( childProcess ) {
        NoteFrameDone(frameStart, frameEnd);
    }
#endif

    if (! realQuiet) {
        fprintf(stdout, "======FRAMES READ:  %d\n", framesRead);
        fflush(stdout);
    }

    return diffTime;
}


/*===========================================================================*
 *
 * IncrementTCTime
 *
 *    increment the tc time by one second (and update min, hrs if necessary)
 *    also increments totalFramesSent
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    totalFramesSent, tc_pict, tc_sec, tc_min, tc_hrs, tc_extra
 *
 *===========================================================================*/
void
IncrementTCTime()
{
    /* if fps = an integer, then tc_extra = 0 and is ignored

       otherwise, it is the number of extra 1/1001 frames we've passed by

       so far; for example, if fps = 24000/1001, then 24 frames = 24024/24000
       seconds = 1 second + 24/24000 seconds = 1 + 1/1000 seconds; similary,
       if fps = 30000/1001, then 30 frames = 30030/30000 = 1 + 1/1000 seconds
       and if fps = 60000/1001, then 60 frames = 1 + 1/1000 seconds

       if fps = 24000/1001, then 1/1000 seconds = 24/1001 frames
       if fps = 30000/1001, then 1/1000 seconds = 30/1001 frames
       if fps = 60000/1001, then 1/1000 seconds = 60/1001 frames     
     */

    totalFramesSent++;
    tc_pict++;
    if ( tc_pict >= frameRateRounded ) {
    tc_pict = 0;
    tc_sec++;
    if ( tc_sec == 60 ) {
        tc_sec = 0;
        tc_min++;
        if ( tc_min == 60 ) {
        tc_min = 0;
        tc_hrs++;
        }
    }
    if ( ! frameRateInteger ) {
        tc_extra += frameRateRounded;
        if ( tc_extra >= 1001 ) {    /* a frame's worth */
        tc_pict++;
        tc_extra -= 1001;
        }
    }
    }
}


/*===========================================================================*
 *
 * SetStatFileName
 *
 *    set the statistics file name
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    statFileName
 *
 *===========================================================================*/
void
SetStatFileName(fileName)
    char *fileName;
{
    strcpy(statFileName, fileName);
}


/*===========================================================================*
 *
 * SetGOPSize
 *
 *    set the GOP size (frames per GOP)
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    gopSize
 *
 *===========================================================================*/
void
SetGOPSize(size)
    int size;
{
    gopSize = size;
}


/*===========================================================================*
 *
 * PrintStartStats
 *
 *    print out the starting statistics (stuff from the param file)
 *    firstFrame, lastFrame represent the first, last frames to be
 *    encoded
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
PrintStartStats(firstFrame, lastFrame)
    int firstFrame;
    int lastFrame;
{
    FILE *fpointer;
    register int i;
    char    inputFileName[1024];

    if ( statFileName[0] == '\0' ) {
    statFile = NULL;
    } else {
    statFile = fopen(statFileName, "a");    /* open for appending */
    if ( statFile == NULL ) {
        fprintf(stderr, "ERROR:  Could not open stat file:  %s\n", statFileName);
        fprintf(stderr, "        Sending statistics to stdout only.\n");
        fprintf(stderr, "\n\n");
    } else if (! realQuiet) {
        fprintf(stdout, "Appending statistics to file:  %s\n", statFileName);
        fprintf(stdout, "\n\n");
    }
    }
    
    for ( i = 0; i < 2; i++ ) {
    if ( ( i == 0 ) && (! realQuiet) ) {
        fpointer = stdout;
    } else if ( statFile != NULL ) {
        fpointer = statFile;
    } else {
        continue;
    }

    fprintf(fpointer, "MPEG ENCODER STATS (%s)\n",VERSION);
    fprintf(fpointer, "------------------------\n");
    fprintf(fpointer, "TIME STARTED:  %s", ctime(&timeStart));
    if ( getenv("HOST") != NULL ) {
        fprintf(fpointer, "MACHINE:  %s\n", getenv("HOST"));
    } else {
        fprintf(fpointer, "MACHINE:  unknown\n");
    }

    if ( stdinUsed ) {
        fprintf(fpointer, "INPUT:  stdin\n");
      }

    
    if ( firstFrame == -1 ) {
        fprintf(fpointer, "OUTPUT:  %s\n", outputFileName);
    } else if ( ! stdinUsed ) {
        GetNthInputFileName(inputFileName, firstFrame);
        fprintf(fpointer, "FIRST FILE:  %s/%s\n", currentPath, inputFileName);
        GetNthInputFileName(inputFileName, lastFrame);
        fprintf(fpointer, "LAST FILE:  %s/%s\n", currentPath,
            inputFileName);
    }
    if ( resizeFrame )
        fprintf(fpointer, "RESIZED TO:  %dx%d\n",
            outputWidth, outputHeight);
    fprintf(fpointer, "PATTERN:  %s\n", framePattern);
    fprintf(fpointer, "GOP_SIZE:  %d\n", gopSize);
    fprintf(fpointer, "SLICES PER FRAME:  %d\n", slicesPerFrame);
    if (searchRangeP==searchRangeB)
      fprintf(fpointer, "RANGE:  +/-%d\n", searchRangeP/2);
    else fprintf(fpointer, "RANGES:  +/-%d %d\n", 
             searchRangeP/2,searchRangeB/2);
    fprintf(fpointer, "PIXEL SEARCH:  %s\n", pixelFullSearch ? "FULL" : "HALF");
    fprintf(fpointer, "PSEARCH:  %s\n", PSearchName());
    fprintf(fpointer, "BSEARCH:  %s\n", BSearchName());
    fprintf(fpointer, "QSCALE:  %d %d %d\n", qscaleI, 
        GetPQScale(), GetBQScale());
    if (specificsOn) 
      fprintf(fpointer, "(Except as modified by Specifics file)\n");
    if ( referenceFrame == DECODED_FRAME ) {
        fprintf(fpointer, "REFERENCE FRAME:  DECODED\n");
    } else if ( referenceFrame == ORIGINAL_FRAME ) {
        fprintf(fpointer, "REFERENCE FRAME:  ORIGINAL\n");
    } else {
        fprintf(stderr, "ERROR:  Illegal referenceFrame!!!\n");
        exit(1);
    }
     /*    For new Rate control parameters */
     if (getRateMode() == FIXED_RATE) {
      fprintf(fpointer, "PICTURE RATE:  %d\n", frameRateRounded);
      if (getBitRate() != -1) {
        fprintf(fpointer, "\nBIT RATE:  %d\n", getBitRate());
      }
      if (getBufferSize() != -1) {
        fprintf(fpointer, "BUFFER SIZE:  %d\n", getBufferSize());
      }
    }
      }
    if (! realQuiet) {
        fprintf(stdout, "\n\n");
    }    
}


/*===========================================================================*
 *
 * NonLocalRefFrame
 *
 *    decides if this frame can be referenced from a non-local process
 *
 * RETURNS:    TRUE or FALSE
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
boolean
NonLocalRefFrame(id)
    int id;
{
    int        lastIPid;
    int        nextIPid;

    if ( ! childProcess ) {
    return FALSE;
    }

    lastIPid = FType_PastRef(id);

    /* might be accessed by B-frame */
    if ( lastIPid+1 < realStart ) {
    return TRUE;
    }

    /* if B-frame is out of range, then current frame can be ref'd by it */
    nextIPid = FType_FutureRef(id);

    /* might be accessed by B-frame */
    if ( nextIPid-1 > realEnd ) {
    return TRUE;
    }

    /* might be accessed by P-frame */
    if ( (nextIPid > realEnd) && (FType_Type(nextIPid) == 'p') ) {
    return TRUE;
    }

    return FALSE;
}


 
/*===========================================================================*
 *
 * SetFrameRate
 *
 *    sets global frame rate variables.  value passed is MPEG frame rate code.
 *
 * RETURNS:    TRUE or FALSE
 *
 * SIDE EFFECTS:    frameRateRounded, frameRateInteger
 *
 *===========================================================================*/
void
SetFrameRate()
{
    switch(frameRate) {
    case 1:
        frameRateRounded = 24;
        frameRateInteger = FALSE;
        break;
    case 2:
        frameRateRounded = 24;
        frameRateInteger = TRUE;
        break;
    case 3:
        frameRateRounded = 25;
        frameRateInteger = TRUE;
        break;
    case 4:
        frameRateRounded = 30;
        frameRateInteger = FALSE;
        break;
    case 5:
        frameRateRounded = 30;
        frameRateInteger = TRUE;
        break;
    case 6:
        frameRateRounded = 50;
        frameRateInteger = TRUE;
        break;
    case 7:
        frameRateRounded = 60;
        frameRateInteger = FALSE;
        break;
    case 8:
        frameRateRounded = 60;
        frameRateInteger = TRUE;
        break;
    }
    printf("frame rate(%d) set to %d\n", frameRate, frameRateRounded);
}


/*=====================*
 * INTERNAL PROCEDURES *
 *=====================*/

/*===========================================================================*
 *
 * ComputeDHMSTime
 *
 *    turn some number of seconds (someTime) into a string which
 *    summarizes that time according to scale (days, hours, minutes, or
 *    seconds)
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
ComputeDHMSTime(someTime, timeText)
    int32_mpeg_t someTime;
    char *timeText;
{
    int        days, hours, mins, secs;

    days = someTime / (24*60*60);
    someTime -= days*24*60*60;
    hours = someTime / (60*60);
    someTime -= hours*60*60;
    mins = someTime / 60;
    secs = someTime - mins*60;

    if ( days > 0 ) {
        sprintf(timeText, "Total time:  %d days and %d hours", days, hours);
    } else if ( hours > 0 ) {
        sprintf(timeText, "Total time:  %d hours and %d minutes", hours, mins);
    } else if ( mins > 0 ) {
        sprintf(timeText, "Total time:  %d minutes and %d seconds", mins, secs);
    } else {
    sprintf(timeText, "Total time:  %d seconds", secs);
    }
}


/*===========================================================================*
 *
 * ComputeGOPFrames
 *
 *    calculate the first, last frames of the numbered GOP
 *
 * RETURNS:    lastFrame, firstFrame changed
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
ComputeGOPFrames(whichGOP, firstFrame, lastFrame, numFrames)
    int whichGOP;
    int *firstFrame;
    int *lastFrame;
    int numFrames;
{
    int        passedB;
    int        currGOP;
    int        gopNum, frameNum;

    /* calculate first, last frames of whichGOP GOP */

    *firstFrame = -1;
    *lastFrame = -1;
    gopNum = 0;
    frameNum = 0;
    passedB = 0;
    currGOP = 0;
    while ( *lastFrame == -1 ) {
    if ( frameNum >= numFrames ) {
        fprintf(stderr, "ERROR:  There aren't that many GOPs!\n");
        exit(1);
    }

#ifdef BLEAH
if (! realQuiet) {
fprintf(stdout, "GOP STARTS AT %d\n", frameNum-passedB);
}
#endif

    if ( gopNum == whichGOP ) {
        *firstFrame = frameNum;
    }

    /* go past one gop */
    /* must go past at least one frame */
    do {
        currGOP += (1 + passedB);

        frameNum++;

        passedB = 0;
        while ( (frameNum < numFrames) && (FType_Type(frameNum) == 'b') ) {
        frameNum++;
        passedB++;
        }
    } while ( (frameNum < numFrames) && 
          ((FType_Type(frameNum) != 'i') || (currGOP < gopSize)) );

    currGOP -= gopSize;

    if ( gopNum == whichGOP ) {
        *lastFrame = (frameNum-passedB-1);
    }

#ifdef BLEAH
if (! realQuiet) {
fprintf(stdout, "GOP ENDS at %d\n", frameNum-passedB-1);
}
#endif

    gopNum++;
    }
}


/*===========================================================================*
 *
 * PrintEndStats
 *
 *    print end statistics (summary, time information)
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
PrintEndStats(inputFrameBits, totalBits)
    int inputFrameBits;
    int32_mpeg_t totalBits;
{
    FILE *fpointer;
    register int i;
    char    timeText[256];
    float   totalCPU;

    if (! realQuiet) {
        fprintf(stdout, "\n\n");
    }

    ComputeDHMSTime(diffTime, timeText);

    for ( i = 0; i < 2; i++ ) {
    if ( ( i == 0 ) && (! realQuiet) ) {
        fpointer = stdout;
    } else if ( statFile != NULL ) {
        fpointer = statFile;
    } else {
        continue;
    }

    fprintf(fpointer, "TIME COMPLETED:  %s", ctime(&timeEnd));
    fprintf(fpointer, "%s\n\n", timeText);

    totalCPU = 0.0;
    totalCPU += ShowIFrameSummary(inputFrameBits, totalBits, fpointer);
    totalCPU += ShowPFrameSummary(inputFrameBits, totalBits, fpointer);
    totalCPU += ShowBFrameSummary(inputFrameBits, totalBits, fpointer);
    fprintf(fpointer, "---------------------------------------------\n");
    fprintf(fpointer, "Total Compression:  %3d:1     (%9.4f bpp)\n",
        framesOutput*inputFrameBits/totalBits,
        24.0*(float)(totalBits)/(float)(framesOutput*inputFrameBits));
    if (diffTime > 0) {
      fprintf(fpointer, "Total Frames Per Second:  %f (%ld mps)\n",
          (float)framesOutput/(float)diffTime,
          (long)((float)framesOutput*(float)inputFrameBits/(256.0*24.0*(float)diffTime)));
    } else {
      fprintf(fpointer, "Total Frames Per Second:  Infinite!\n");
    }
    if ( totalCPU == 0.0 ) {
        fprintf(fpointer, "CPU Time:  NONE!\n");
    } else {
        fprintf(fpointer, "CPU Time:  %f fps     (%ld mps)\n",
            (float)framesOutput/totalCPU,
             (long)((float)framesOutput*(float)inputFrameBits/(256.0*24.0*totalCPU)));
    }
    fprintf(fpointer, "Total Output Bit Rate (%d fps):  %d bits/sec\n",
        frameRateRounded, frameRateRounded*totalBits/framesOutput);
    fprintf(fpointer, "MPEG file created in :  %s\n", outputFileName);
    fprintf(fpointer, "\n\n");

    if ( computeMVHist ) {
        ShowPMVHistogram(fpointer);
        ShowBBMVHistogram(fpointer);
        ShowBFMVHistogram(fpointer);
    }
    }

    if ( statFile != NULL ) {
        fclose(statFile);
    }
}


/*===========================================================================*
 *
 * ProcessRefFrame
 *
 *    process an I or P frame -- encode it, and process any B frames that
 *    we can now
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    stuff appended to bb
 *
 *===========================================================================*/
static void
  ProcessRefFrame(frame, bb, lastFrame, outputFileName)
MpegFrame *frame;
BitBucket *bb;
int lastFrame;
char *outputFileName;
{
  MpegFrame *bFrame = NULL;
  char    fileName[1024];
  char    inputFileName[1024];
  FILE    *fpointer = NULL;
  boolean separateFiles;
  int        id;
  time_t  tempTimeStart, tempTimeEnd;
  
  separateFiles = (bb == NULL);
  
  if ( separateFiles && (frame->id >= realStart) &&
      (frame->id <= realEnd) ) {
    if ( remoteIO ) {
      bb = Bitio_New(NULL);
    } else {
      sprintf(fileName, "%s.frame.%d", outputFileName, frame->id);
      if ( (fpointer = fopen(fileName, "wb")) == NULL ) {
    fprintf(stderr, "ERROR:  Could not open output file(1):  %s\n",
        fileName);
    exit(1);
      }
      
      bb = Bitio_New(fpointer);
    }
  }
  
  /* nothing to do */
  if ( frame->id < realStart ) {
    return;
  }
  
  /* first, output this frame */
  if ( frame->type == TYPE_IFRAME ) {
      
#ifdef BLEAH
    fprintf(stdout, "I-frame %d, currentGOP = %d\n",
        frame->id, currentGOP);
    fflush(stdout);
#endif
      
    /* only start a new GOP with I */
    /* don't start GOP if only doing frames */
    if ( (! separateFiles) && (currentGOP >= gopSize) ) {
      int closed;
      static int num_gop = 0;
    
      /* first, check to see if closed GOP */
      if ( totalFramesSent == frame->id || pastRefFrame == NULL) {
    closed = 1;
      } else {
    closed = 0;
      }
    
      /* new GOP */
      if (num_gop != 0 && mult_seq_headers && num_gop % mult_seq_headers == 0) {
    if (! realQuiet) {
      fprintf(stdout, "Creating new Sequence before GOP %d\n", num_gop);
      fflush(stdout);
    }
      
    Mhead_GenSequenceHeader(bb, Fsize_x, Fsize_y,
                /* pratio */    aspectRatio,
                /* pict_rate */ frameRate, /* bit_rate */ bit_rate,
                /* buf_size */  buf_size,  /* c_param_flag */ 1,
                /* iq_matrix */ customQtable, /* niq_matrix */ customNIQtable,
                /* ext_data */ NULL,  /* ext_data_size */ 0,
                /* user_data */ NULL, /* user_data_size */ 0);
      }
    
      if (! realQuiet) {
    fprintf(stdout, "Creating new GOP (closed = %c) before frame %d\n",
        "FT"[closed], frame->id);
    fflush(stdout);
      }
    
      num_gop++;
      Mhead_GenGOPHeader(bb,    /* drop_frame_flag */ 0,
             tc_hrs, tc_min, tc_sec, tc_pict,
             closed, /* broken_link */ 0,
             /* ext_data */ NULL, /* ext_data_size */ 0,
             /* user_data */ NULL, /* user_data_size */ 0);
      currentGOP -= gopSize;
      if (pastRefFrame == NULL) {
    SetGOPStartTime(0);
      } else {
    SetGOPStartTime(pastRefFrame->id+1);
      }
    }
      
    if ( (frame->id >= realStart) && (frame->id <= realEnd) ) {
      GenIFrame(bb, frame);
    
      framesOutput++;
    
      if ( separateFiles ) {
    if ( remoteIO ) {
      SendRemoteFrame(frame->id, bb);
    } else {
      Bitio_Flush(bb);
      fclose(fpointer);
    }
      }
    }
      
    numI--;
    timeMask &= 0x6;
      
    currentGOP++;
    IncrementTCTime();
  } else {
    if ( (frame->id >= realStart) && (frame->id <= realEnd) ) {
      GenPFrame(bb, frame, pastRefFrame);
    
      framesOutput++;
    
      if ( separateFiles ) {
    if ( remoteIO ) {
      SendRemoteFrame(frame->id, bb);
    } else {
      Bitio_Flush(bb);
      fclose(fpointer);
    }
      }
    }
    
    numP--;
    timeMask &= 0x5;
    ShowRemainingTime();
    
    currentGOP++;
    IncrementTCTime();
  }
  
  /* now, output B-frames */
  if ( pastRefFrame != NULL ) {
    for ( id = pastRefFrame->id+1; id < futureRefFrame->id; id++ ) {
      if ( ! ((id >= realStart) && (id <= realEnd)) )
    continue;
    
      if ( ! stdinUsed ) {
    bFrame = Frame_New(id, 'b');
      
    time(&tempTimeStart);
      
    /* read B frame, output it */
    if ( remoteIO ) {
      GetRemoteFrame(bFrame, bFrame->id);
    } else {
      GetNthInputFileName(inputFileName, id);
      if ( childProcess && separateConversion ) {
        ReadFrame(bFrame, inputFileName, slaveConversion, TRUE);
      } else {
        ReadFrame(bFrame, inputFileName, inputConversion, TRUE);
      }
    }
      
    time(&tempTimeEnd);
    IOtime += (tempTimeEnd-tempTimeStart);
      
    framesRead++;
      } else {
      
    /* retrieve and remove B-frame from queue set up in 
     * GenMPEGStream 
     */
    bFrame = pastRefFrame->next;
    pastRefFrame->next = bFrame->next;
      }
    
    
      if ( separateFiles ) {
    if ( remoteIO ) {
      bb = Bitio_New(NULL);
    } else {
      sprintf(fileName, "%s.frame.%d", outputFileName, 
          bFrame->id);
      if ( (fpointer = fopen(fileName, "wb")) == NULL ) {
        fprintf(stderr, "ERROR:  Could not open output file(2):  %s\n",
            fileName);
        exit(1);
      }
      bb = Bitio_New(fpointer);
    }
      }
    
      GenBFrame(bb, bFrame, pastRefFrame, futureRefFrame);
      framesOutput++;
    
      if ( separateFiles ) {
    if ( remoteIO ) {
      SendRemoteFrame(bFrame->id, bb);
    } else {
      Bitio_Flush(bb);
      fclose(fpointer);
    }
      }
    
      /* free this B frame right away */
      Frame_Free(bFrame);
    
      numB--;
      timeMask &= 0x3;
      ShowRemainingTime();
    
      currentGOP++;
      IncrementTCTime();
    }
  } else {
    /* SRS replicated code */
    for ( id = 0; id < futureRefFrame->id; id++ ) {
      if ( ! ((id >= realStart) && (id <= realEnd)) )
    continue;

      if ( ! stdinUsed ) {
    bFrame = Frame_New(id, 'b');

    time(&tempTimeStart);

    /* read B frame, output it */
    if ( remoteIO ) {
      GetRemoteFrame(bFrame, bFrame->id);
    } else {
      GetNthInputFileName(inputFileName, id);
      if ( childProcess && separateConversion ) {
        ReadFrame(bFrame, inputFileName, slaveConversion, TRUE);
      } else {
        ReadFrame(bFrame, inputFileName, inputConversion, TRUE);
      }
    }

    time(&tempTimeEnd);
    IOtime += (tempTimeEnd-tempTimeStart);

    framesRead++;
      } else {
        
    /* retrieve and remove B-frame from queue set up in 
     * GenMPEGStream 
     */
    printf("Yow, I doubt this works!\n");
    bFrame = pastRefFrame->next;
    pastRefFrame->next = bFrame->next;
      }


      if ( separateFiles ) {
    if ( remoteIO ) {
      bb = Bitio_New(NULL);
    } else {
      sprintf(fileName, "%s.frame.%d", outputFileName, 
          bFrame->id);
      if ( (fpointer = fopen(fileName, "wb")) == NULL ) {
        fprintf(stderr, "ERROR:  Could not open output file(2):  %s\n",
            fileName);
        exit(1);
      }
      bb = Bitio_New(fpointer);
    }
      }

      GenBFrame(bb, bFrame, (MpegFrame *)NULL, futureRefFrame);
      framesOutput++;

      if ( separateFiles ) {
    if ( remoteIO ) {
      SendRemoteFrame(bFrame->id, bb);
    } else {
      Bitio_Flush(bb);
      fclose(fpointer);
    }
      }

      /* free this B frame right away */
      Frame_Free(bFrame);

      numB--;
      timeMask &= 0x3;
      ShowRemainingTime();

      currentGOP++;
      IncrementTCTime();
    }
    
  }
  
  /* now free previous frame, if there was one */
  if ( pastRefFrame != NULL ) {
    Frame_Free(pastRefFrame);
  }
  
  /* note, we may still not free last frame if lastFrame is incorrect
   * (if the last frames are B frames, they aren't output!)
   */
}


/*===========================================================================*
 *
 * ShowRemainingTime
 *
 *    print out an estimate of the time left to encode
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
ShowRemainingTime()
{
    static int    lastTime = 0;
    float   total;
    time_t  nowTime;
    float   secondsPerFrame;

    if ( childProcess ) {
    return /* nothing */;
    }

    if ( numI + numP + numB == 0 ) {    /* no time left */
    return /* nothing */ ;
    }

    if ( timeMask != 0 ) {        /* haven't encoded all types yet */
    return /* nothing */ ;
    }

    time(&nowTime);
    secondsPerFrame = (nowTime-timeStart)/(float)framesOutput;
    total = secondsPerFrame*(float)(numI+numP+numB);

#ifdef BLEAH
    float   timeI, timeP, timeB;

    timeI = EstimateSecondsPerIFrame();
    timeP = EstimateSecondsPerPFrame();
    timeB = EstimateSecondsPerBFrame();
    total = (float)numI*timeI + (float)numP*timeP + (float)numB*timeB;
#endif

    if ( (quietTime >= 0) && (! realQuiet) && (! stdinUsed) &&
     ((lastTime < (int)total) || ((lastTime-(int)total) >= quietTime) ||
      (lastTime == 0) || (quietTime == 0)) ) {
    if ( total > 270.0 ) {
        fprintf(stdout, "ESTIMATED TIME OF COMPLETION:  %d minutes\n",
            ((int)total+30)/60);
    } else {
        fprintf(stdout, "ESTIMATED TIME OF COMPLETION:  %d seconds\n",
            (int)total);
    }

    lastTime = (int)total;
    }
}


void
ReadDecodedRefFrame(frame, frameNumber)
    MpegFrame *frame;
    int frameNumber;
{
    FILE    *fpointer;
    char    fileName[256];
    int    width, height;
    register int y;

    width = Fsize_x;
    height = Fsize_y;

    sprintf(fileName, "%s.decoded.%d", outputFileName, frameNumber);
    if (! realQuiet) {
    fprintf(stdout, "reading %s\n", fileName);
    fflush(stdout);
    }

    if ((fpointer = fopen(fileName, "rb")) == NULL) {
      sleep(1);
      if ((fpointer = fopen(fileName, "rb")) == NULL) {
    fprintf(stderr, "Cannot open %s\n", fileName);
    exit(1);
      }}

    Frame_AllocDecoded(frame, TRUE);
    
    for ( y = 0; y < height; y++ ) {
      if (fread(frame->decoded_y[y], 1, width, fpointer) != width) {
    fprintf(stderr, "Could not read enough bytes from %s\n", fileName);
      }
    }
    
    for (y = 0; y < (height >> 1); y++) {            /* U */
      if (fread(frame->decoded_cb[y], 1, width >> 1, fpointer) != (width>>1)) {
    fprintf(stderr, "Could not read enough bytes from %s\n", fileName);
      }
    }
    
    for (y = 0; y < (height >> 1); y++) {            /* V */
      if (fread(frame->decoded_cr[y], 1, width >> 1, fpointer) != (width>>1)) {
    fprintf(stderr, "Could not read enough bytes from %s\n", fileName);
      }
    }
    
    fclose(fpointer);
}


static void
OpenBitRateFile()
{
    bitRateFile = fopen(bitRateFileName, "w");
    if ( bitRateFile == NULL ) {
    fprintf(stderr, "ERROR:  Could not open bit rate file:  %s\n", bitRateFileName);
    fprintf(stderr, "\n\n");
    showBitRatePerFrame = FALSE;
    }
}


static void
CloseBitRateFile()
{
#ifdef BLEAH
    char command[256];
#endif

    fclose(bitRateFile);
#ifdef BLEAH
    sprintf(command, "sort -n %s > /tmp/fubahr", bitRateFileName);
    system(command);
    sprintf(command, "mv /tmp/fubahr %s", bitRateFileName);
    system(command);
#endif
}
