/*===========================================================================*
 * combine.c                                     *
 *                                         *
 *    Procedures to combine frames or GOPS into an MPEG sequence         *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    GOPStoMPEG                                 *
 *    FramesToMPEG                                 *
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
 *  $Header: /n/picasso/project/mpeg/mpeg_dist/mpeg_encode/RCS/combine.c,v 1.9 1995/08/07 21:42:38 smoot Exp $
 *  $Log: combine.c,v $
 *  Revision 1.9  1995/08/07 21:42:38  smoot
 *  Sleeps when files do not exist.
 *  renamed index to idx
 *
 *  Revision 1.8  1995/06/21 22:20:45  smoot
 *  added a sleep for NFS to complete file writes
 *
 * Revision 1.7  1995/06/08  20:23:19  smoot
 * added "b"'s to fopen so PCs are happy
 *
 * Revision 1.6  1995/01/19  23:07:22  eyhung
 * Changed copyrights
 *
 * Revision 1.5  1995/01/16  07:53:55  eyhung
 * Added realQuiet
 *
 * Revision 1.4  1994/11/12  02:11:46  keving
 * nothing
 *
 * Revision 1.3  1994/03/15  00:27:11  keving
 * nothing
 *
 * Revision 1.2  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.1  1993/07/22  22:23:43  keving
 * nothing
 *
 */


/*==============*
 * HEADER FILES *
 *==============*/

#include "all.h"
#include <time.h>
#include <errno.h>
#include "mtypes.h"
#include "frames.h"
#include "search.h"
#include "mpeg.h"
#include "prototypes.h"
#include "parallel.h"
#include "param.h"
#include "readframe.h"
#include "mheaders.h"
#include "fsize.h"
#include "combine.h"
#include <unistd.h>

/* note, remove() might not have a prototype in the standard header files,
 * but it really should -- it's not my fault!
 */


static int    currentGOP;

#define READ_ATTEMPTS 5 /* number of times (seconds) to retry an input file */

/*==================*
 * GLOBAL VARIABLES *
 *==================*/
extern int  yuvWidth, yuvHeight;
char    currentGOPPath[MAXPATHLEN];
char    currentFramePath[MAXPATHLEN];


/*===============================*
 * INTERNAL PROCEDURE prototypes *
 *===============================*/

static void    AppendFile _ANSI_ARGS_((FILE *outputFile, FILE *inputFile));


/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/

/*===========================================================================*
 *
 * GOPStoMPEG
 *
 *    convert some number of GOP files into a single MPEG sequence file
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
GOPStoMPEG(numGOPS, outputFileName, outputFilePtr)
    int numGOPS;
    char *outputFileName;
    FILE *outputFilePtr;
{
    register int ind;
    BitBucket *bb;
    char    fileName[1024];
    char    inputFileName[1024];
    FILE *inputFile;
    int q;

    {
      /* Why is this reset called? */
      int x=Fsize_x, y=Fsize_y;
      Fsize_Reset();
      Fsize_Note(0, yuvWidth, yuvHeight);
      if (Fsize_x == 0 || Fsize_y == 0) {
    Fsize_Note(0, x, y);
      }}
    
    bb = Bitio_New(outputFilePtr);

    Mhead_GenSequenceHeader(bb, Fsize_x, Fsize_y, /* pratio */ aspectRatio,
           /* pict_rate */ frameRate, /* bit_rate */ -1,
           /* buf_size */ -1, /*c_param_flag */ 1,
           /* iq_matrix */ customQtable, /* niq_matrix */ customNIQtable,
           /* ext_data */ NULL, /* ext_data_size */ 0,
           /* user_data */ NULL, /* user_data_size */ 0);

    /* it's byte-padded, so we can dump it now */
    Bitio_Flush(bb);

    if ( numGOPS > 0 ) {
    for ( ind = 0; ind < numGOPS; ind++ ) {
        GetNthInputFileName(inputFileName, ind);
        sprintf(fileName, "%s/%s", currentGOPPath, inputFileName);

        for (q = 0;   q < READ_ATTEMPTS;  ++q ) {
          if ( (inputFile = fopen(fileName, "rb")) != NULL ) break;
          fprintf(stderr, "ERROR:  Couldn't read (GOPStoMPEG):  %s retry %d\n", 
              fileName, q);
          fflush(stderr);
          sleep(1);
        }
        if (q == READ_ATTEMPTS) {
          fprintf(stderr, "Giving up (%d attepmts).\n", READ_ATTEMPTS);
          exit(1);
        }
        
        if (! realQuiet) {
            fprintf(stdout, "appending file:  %s\n", fileName);
        }

        AppendFile(outputFilePtr, inputFile);
    }
    } else {
    ind = 0;
    while ( TRUE ) {
        sprintf(fileName, "%s.gop.%d", outputFileName, ind);

        if ( (inputFile = fopen(fileName, "rb")) == NULL ) {
        break;
        }

        if (! realQuiet) {
            fprintf(stdout, "appending file:  %s\n", fileName);
        }

        AppendFile(outputFilePtr, inputFile);

        ind++;
    }
    }

    bb = Bitio_New(outputFilePtr);

    /* SEQUENCE END CODE */
    Mhead_GenSequenceEnder(bb);

    Bitio_Flush(bb);

    fclose(outputFilePtr);
}


/*===========================================================================*
 *
 * FramestoMPEG
 *
 *    convert some number of frame files into a single MPEG sequence file
 *
 *    if parallel == TRUE, then when appending a file, blocks until that
 *    file is actually ready
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
FramesToMPEG(numFrames, outputFileName, outputFile, parallel)
    int numFrames;
    char *outputFileName;
    FILE *outputFile;
    boolean parallel;
{
    register int ind;
    BitBucket *bb;
    char    fileName[1024];
    char    inputFileName[1024];
    FILE *inputFile;
    int    pastRefNum = -1;
    int    futureRefNum = -1;
    int q;

    tc_hrs = 0;    tc_min = 0; tc_sec = 0; tc_pict = 0; tc_extra = 0;

    {
      /* Why is this reset called? */
      int x=Fsize_x, y=Fsize_y;
      Fsize_Reset();
      Fsize_Note(0, yuvWidth, yuvHeight);
      if (Fsize_x == 0 || Fsize_y == 0) {
    Fsize_Note(0, x, y);
      }}
    SetBlocksPerSlice();

    bb = Bitio_New(outputFile);
    Mhead_GenSequenceHeader(bb, Fsize_x, Fsize_y, /* pratio */ aspectRatio,
           /* pict_rate */ frameRate, /* bit_rate */ -1,
           /* buf_size */ -1, /*c_param_flag */ 1,
           /* iq_matrix */ qtable, /* niq_matrix */ niqtable,
           /* ext_data */ NULL, /* ext_data_size */ 0,
           /* user_data */ NULL, /* user_data_size */ 0);
    /* it's byte-padded, so we can dump it now */
    Bitio_Flush(bb);

    /* need to do these in the right order!!! */
    /* also need to add GOP headers */

    currentGOP = gopSize;
    totalFramesSent = 0;

    if ( numFrames > 0 ) {
    for ( ind = 0; ind < numFrames; ind++ ) {
        if ( FRAME_TYPE(ind) == 'b' ) {
        continue;
        }

        pastRefNum = futureRefNum;
        futureRefNum = ind;

        if ( (FRAME_TYPE(ind) == 'i') && (currentGOP >= gopSize) ) {
        int closed;

        /* first, check to see if closed GOP */
        if ( totalFramesSent == ind ) {
            closed = 1;
        } else {
            closed = 0;
        }

        if (! realQuiet) {
            fprintf(stdout, "Creating new GOP (closed = %d) after %d frames\n",
                closed, currentGOP);
        }

        /* new GOP */
        bb = Bitio_New(outputFile);
        Mhead_GenGOPHeader(bb, /* drop_frame_flag */ 0,
               tc_hrs, tc_min, tc_sec, tc_pict,
               closed, /* broken_link */ 0,
               /* ext_data */ NULL, /* ext_data_size */ 0,
               /* user_data */ NULL, /* user_data_size */ 0);
        Bitio_Flush(bb);
        SetGOPStartTime(ind);
        
        currentGOP -= gopSize;
        }

        if ( parallel ) {
        WaitForOutputFile(ind);
        sprintf(fileName, "%s.frame.%d", outputFileName, ind);
        } else {
        GetNthInputFileName(inputFileName, ind);
        sprintf(fileName, "%s/%s", currentFramePath, inputFileName);
        }

        for (q = 0;   q < READ_ATTEMPTS;  ++q ) {
          if ( (inputFile = fopen(fileName, "rb")) != NULL ) break;
          fprintf(stderr, "ERROR:  Couldn't read 2:  %s retry %d\n", fileName, q);
          fflush(stderr);
          sleep(1);
        }
        if (q == READ_ATTEMPTS) {
          fprintf(stderr, "Giving up (%d attepmts).\n", READ_ATTEMPTS);
          exit(1);
        }
        
        AppendFile(outputFile, inputFile);
        if ( parallel ) {
        remove(fileName);
        }

        currentGOP++;
        IncrementTCTime();

        /* now, output the B-frames */
        if ( pastRefNum != -1 ) {
        register int bNum;

        for ( bNum = pastRefNum+1; bNum < futureRefNum; bNum++ ) {
            if ( parallel ) {
            WaitForOutputFile(bNum);
            sprintf(fileName, "%s.frame.%d", outputFileName, bNum);
            } else {
            GetNthInputFileName(inputFileName, bNum);
            sprintf(fileName, "%s/%s", currentFramePath, inputFileName);
            }


            for (q = 0;   q < READ_ATTEMPTS;  ++q ) {
              if ( (inputFile = fopen(fileName, "rb")) != NULL ) break;
              fprintf(stderr, "ERROR:  Couldn't read (bNum=%d):  %s retry %d\n", 
                  bNum, fileName, q);
              fflush(stderr);
              sleep(1);
            }
            if (q == READ_ATTEMPTS) {
              fprintf(stderr, "Giving up (%d attepmts).\n", READ_ATTEMPTS);
              exit(1);
            }
            
            AppendFile(outputFile, inputFile);
            if ( parallel ) {
            remove(fileName);
            }
            
            currentGOP++;
            IncrementTCTime();
        }
        }
    }
    } else {
    if ( parallel ) {
        fprintf(stderr, "ERROR:  PARALLEL COMBINE WITH 0 FRAMES\n");
        fprintf(stderr, "(please send bug report!)\n");
        exit(1);
    }

    ind = 0;
    while ( TRUE ) {
        if ( FRAME_TYPE(ind) == 'b' ) {
        ind++;
        continue;
        }

        if ( (FRAME_TYPE(ind) == 'i') && (currentGOP >= gopSize) ) {
        int closed;

        /* first, check to see if closed GOP */
        if ( totalFramesSent == ind ) {
            closed = 1;
        } else {
            closed = 0;
        }

        if (! realQuiet) {
        fprintf(stdout, "Creating new GOP (closed = %d) before frame %d\n",
            closed, ind);
        }

        /* new GOP */
        bb = Bitio_New(outputFile);
        Mhead_GenGOPHeader(bb, /* drop_frame_flag */ 0,
               tc_hrs, tc_min, tc_sec, tc_pict,
               closed, /* broken_link */ 0,
               /* ext_data */ NULL, /* ext_data_size */ 0,
               /* user_data */ NULL, /* user_data_size */ 0);
        Bitio_Flush(bb);
        SetGOPStartTime(ind);

        currentGOP -= gopSize;
        }

        sprintf(fileName, "%s.frame.%d", outputFileName, ind);

        if ( (inputFile = fopen(fileName, "rb")) == NULL ) {
        break;
        }

        AppendFile(outputFile, inputFile);
        if ( parallel ) {
        remove(fileName);
        }

        currentGOP++;
        IncrementTCTime();

        /* now, output the B-frames */
        if ( pastRefNum != -1 ) {
        register int bNum;

        for ( bNum = pastRefNum+1; bNum < futureRefNum; bNum++ ) {
            sprintf(fileName, "%s.frame.%d", outputFileName, bNum);

            for (q = 0;   q < READ_ATTEMPTS;  ++q ) {
              if ( (inputFile = fopen(fileName, "rb")) != NULL ) break;
              fprintf(stderr, "ERROR:  Couldn't read (FramestoMPEG):  %s retry %d\n", 
                  fileName, q);
              fflush(stderr);
              sleep(1);
            }
            if (q == READ_ATTEMPTS) {
              fprintf(stderr, "Giving up (%d attepmts).\n", READ_ATTEMPTS);
              exit(1);
            }

            AppendFile(outputFile, inputFile);
            if ( parallel ) {
            remove(fileName);
            }

            currentGOP++;
            IncrementTCTime();
        }
        }

        ind++;
    }
    }

    if (! realQuiet) {
    fprintf(stdout, "Wrote %d frames\n", totalFramesSent);
    fflush(stdout);
    }

    bb = Bitio_New(outputFile);

    /* SEQUENCE END CODE */
    Mhead_GenSequenceEnder(bb);

    Bitio_Flush(bb);

    fclose(outputFile);
}


/*=====================*
 * INTERNAL PROCEDURES *
 *=====================*/

/*===========================================================================*
 *
 * AppendFile
 *
 *    appends the output file with the contents of the given input file
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
AppendFile(outputFile, inputFile)
    FILE *outputFile;
    FILE *inputFile;
{
    uint8_mpeg_t   data[9999];
    int        readItems;

    readItems = 9999;
    while ( readItems == 9999 ) {
    readItems = fread(data, sizeof(uint8_mpeg_t), 9999, inputFile);
    if ( readItems > 0 ) {
        fwrite(data, sizeof(uint8_mpeg_t), readItems, outputFile);
    }
    }

    fclose(inputFile);
}


