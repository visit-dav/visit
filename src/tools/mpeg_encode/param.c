/*===========================================================================*
 * param.c                                     *
 *                                         *
 *    Procedures to read in parameter file                     *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    ReadParamFile                                 *
 *    GetNthInputFileName                             *
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
 *  $Header: /n/picasso/project/mpeg/mpeg_dist/mpeg_encode/RCS/param.c,v 1.38 1995/08/14 22:29:13 smoot Exp smoot $
 *  $Log: param.c,v $
 *  Revision 1.38  1995/08/14 22:29:13  smoot
 *  cleaned up subcommand for file name generation
 *  fixed bug where foo* didnt work as input string
 *
 *  Revision 1.37  1995/08/07 21:47:01  smoot
 *  allows spaces after lines in param files
 *  added SIZE == YUV_SIZE for parallel encoding (otherwise you'd have
 *  to parse PPM files in the master server... Ick.
 *  So we have it specified)
 *
 *  Revision 1.36  1995/06/26 21:52:52  smoot
 *  allow uppercase stdin
 *
 *  Revision 1.35  1995/06/21 18:35:01  smoot
 *  moved TUNE stuff to opts.c
 *  Added ability to do:
 *  INPUT
 *  file.ppm  [1-10]
 *  to do 10 copies
 *  added CDL = SPECIFICS
 *
 * Revision 1.34  1995/05/02  01:48:25  eyhung
 * deleted some smootisms. fixed framerate settings, added check for
 * invalid input range
 *
 * Revision 1.33  1995/05/02  01:20:10  smoot
 * *** empty log message ***
 *
 * Revision 1.31  1995/03/09  06:22:36  eyhung
 * more robust input checking (whitespace at end of input file spec)
 *
 * Revision 1.30  1995/02/02  01:06:18  eyhung
 * Added error checking for JMOVIE and stdin ; deleted smoot's "understand"
 * param file ideas.
 *
 * Revision 1.29  1995/02/01  21:09:59  eyhung
 * Finished infinite coding-on-the-fly
 *
 * Revision 1.28  1995/01/31  22:34:28  eyhung
 * Added stdin as a parameter to INPUT_DIR for interactive encoding
 *
 * Revision 1.27  1995/01/27  21:58:07  eyhung
 * Fixed a bug with reading JMOVIES in GetNthInputFileName
 *
 * Revision 1.26  1995/01/25  23:00:05  smoot
 * Checks out Qtable entries
 *
 * Revision 1.25  1995/01/23  02:09:51  darryl
 * removed "PICT_RATE" code
 *
 * Revision 1.24  1995/01/20  00:07:46  smoot
 * requires unistd.c now
 *
 * Revision 1.23  1995/01/20  00:05:33  smoot
 * Added gamma correction option
 *
 * Revision 1.22  1995/01/19  23:55:55  eyhung
 * Fixed up smoot's "style" and made YUV_FORMAT default to EYUV
 *
 * Revision 1.21  1995/01/19  23:09:03  eyhung
 * Changed copyrights
 *
 * Revision 1.20  1995/01/17  22:04:14  smoot
 * Added `commands` to file name listing
 *
 * Revision 1.19  1995/01/17  06:28:01  eyhung
 * StdinUsed added.
 *
 * Revision 1.18  1995/01/16  09:33:35  eyhung
 * Fixed stupid commenting error.
 *
 * Revision 1.17  1995/01/16  06:07:53  eyhung
 * Made it look a little nicer
 *
 * Revision 1.16  1995/01/13  23:57:25  smoot
 * added Y format
 *
 * Revision 1.15  1995/01/08  06:20:39  eyhung
 * *** empty log message ***
 *
 * Revision 1.14  1995/01/08  06:15:57  eyhung
 * *** empty log message ***
 *
 * Revision 1.13  1995/01/08  05:50:32  eyhung
 * Added YUV Format parameter
 *
 * Revision 1.12  1994/12/16  00:55:30  smoot
 * Fixed INPU_FILES bug
 *
 * Revision 1.11  1994/12/12  23:54:36  smoot
 * Fixed GOP-missing error message (GOP to GOP_SIZE)
 *
 * Revision 1.10  1994/12/07  00:40:36  smoot
 * Added seperate P and B search ranges
 *
 * Revision 1.9  1994/11/18  23:19:22  smoot
 * Added USER_DATA parameter
 *
 * Revision 1.8  1994/11/16  22:33:40  smoot
 * Put in ifdef for rsh in param.c
 *
 * Revision 1.7  1994/11/16  22:25:05  smoot
 * Corrected ASPECT_RATIO bug
 *
 * Revision 1.6  1994/11/14  22:39:26  smoot
 * merged specifics and rate control
 *
 * Revision 1.5  1994/11/01  05:01:41  darryl
 *  with rate control changes added
 *
 * Revision 1.1  1994/09/27  00:16:28  darryl
 * Initial revision
 *
 * Revision 1.4  1994/03/15  00:27:11  keving
 * nothing
 *
 * Revision 1.3  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.2  1993/07/22  22:23:43  keving
 * nothing
 *
 * Revision 1.1  1993/06/30  20:06:09  keving
 * nothing
 *
 */


/*==============*
 * HEADER FILES *
 *==============*/

#include "all.h"
#include "mtypes.h"
#include "mpeg.h"
#include "search.h"
#include "prototypes.h"
#include "parallel.h"
#include "param.h"
#include "readframe.h"
#include "fsize.h"
#include "frames.h"
#include "jpeg.h"
#include <string.h>
#include <ctype.h>
#include "rate.h"
#include "opts.h"

/*===========*
 * CONSTANTS *
 *===========*/

#define INPUT_ENTRY_BLOCK_SIZE   128

#define FIRST_OPTION           0
#define OPTION_GOP             0
#define OPTION_PATTERN         1
#define OPTION_PIXEL           2
#define OPTION_PQSCALE         3
#define OPTION_OUTPUT          4
#define OPTION_RANGE           5
#define OPTION_PSEARCH_ALG     6
#define OPTION_IQSCALE         7
#define OPTION_INPUT_DIR       8
#define OPTION_INPUT_CONVERT   9
#define OPTION_INPUT          10
#define OPTION_BQSCALE        11
#define OPTION_BASE_FORMAT    12
#define OPTION_SPF            13
#define OPTION_BSEARCH_ALG    14
#define OPTION_REF_FRAME      15
#define LAST_OPTION           15

/* put any non-required options after LAST_OPTION */
#define OPTION_RESIZE          16
#define OPTION_IO_CONVERT     17
#define OPTION_SLAVE_CONVERT  18
#define OPTION_IQTABLE          19
#define OPTION_NIQTABLE          20
#define OPTION_FRAME_RATE     21
#define OPTION_ASPECT_RATIO   22
#define OPTION_YUV_SIZE          23
#define OPTION_SPECIFICS      24
#define OPTION_DEFS_SPECIFICS 25
#define OPTION_BUFFER_SIZE    26
#define OPTION_BIT_RATE       27
#define OPTION_USER_DATA      28
#define OPTION_YUV_FORMAT     29
#define OPTION_GAMMA          30
#define OPTION_PARALLEL       31

#define NUM_OPTIONS           31

/*=======================*
 * STRUCTURE DEFINITIONS *
 *=======================*/

typedef struct InputFileEntryStruct {
    char    left[256];
    char    right[256];
    boolean glob;        /* if FALSE, left is complete name */
    int        startID;
    int        endID;
    int        skip;
    int        numPadding;        /* -1 if there is none */
    int        numFiles;
    boolean repeat;
} InputFileEntry;


/*==================*
 * STATIC VARIABLES *
 *==================*/

static InputFileEntry **inputFileEntries;
static int numInputFileEntries = 0;
static int  maxInputFileEntries;


/*==================*
 * GLOBAL VARIABLES *
 *==================*/

extern char currentPath[MAXPATHLEN];
extern char currentGOPPath[MAXPATHLEN];
extern char currentFramePath[MAXPATHLEN];
char    outputFileName[256];
int    outputWidth, outputHeight;
int numInputFiles = 0;
char inputConversion[1024];
char ioConversion[1024];
char slaveConversion[1024];
char yuvConversion[256];
char specificsFile[256],specificsDefines[1024]="";
boolean GammaCorrection=FALSE;
float   GammaValue;
unsigned char userDataFileName[256]={0};
boolean specificsOn = FALSE;
boolean optionSeen[NUM_OPTIONS+1];
int numMachines;
char    machineName[MAX_MACHINES][256];
char    userName[MAX_MACHINES][256];
char    executable[MAX_MACHINES][1024];
char    remoteParamFile[MAX_MACHINES][1024];
boolean    remote[MAX_MACHINES];
boolean stdinUsed = FALSE;
int mult_seq_headers = 0;  /* 0 for none, N for header/N GOPs */

/*===============================*
 * INTERNAL PROCEDURE prototypes *
 *===============================*/
static void    ReadInputFileNames _ANSI_ARGS_((FILE *fpointer,
                        char *endInput));
static void    ReadMachineNames _ANSI_ARGS_((FILE *fpointer));
static int    GetAspectRatio _ANSI_ARGS_((char *p));
static int    GetFrameRate _ANSI_ARGS_((char *p));

/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/


/*===========================================================================*
 *
 * ReadParamFile
 *
 *    read the parameter file
 *    function is ENCODE_FRAMES, COMBINE_GOPS, or COMBINE_FRAMES, and
 *        will slightly modify the procedure's behavior as to what it
 *        is looking for in the parameter file
 *
 * RETURNS:    TRUE if the parameter file was read correctly; FALSE if not
 *
 * SIDE EFFECTS:    sets parameters accordingly, as well as machine info for
 *            parallel execution and input file names
 *
 *===========================================================================*/
boolean
  ReadParamFile(fileName, function)
char *fileName;
int function;
{
  FILE *fpointer;
  char    input[256];
  char    *charPtr;
  boolean yuvUsed = FALSE;
  static char *optionText[LAST_OPTION+1] = { "GOP_SIZE", "PATTERN", "PIXEL", "PQSCALE",
                                             "OUTPUT", "RANGE", "PSEARCH_ALG", "IQSCALE", "INPUT_DIR",
                                             "INPUT_CONVERT", "INPUT", "BQSCALE", "BASE_FILE_FORMAT",
                                             "SLICES_PER_FRAME", "BSEARCH_ALG", "REFERENCE_FRAME"};
  register int index;
  register int row, col;

  if ( (fpointer = fopen(fileName, "r")) == NULL ) {
    fprintf(stderr, "Error:  Cannot open parameter file:  %s\n", fileName);
    return FALSE;
  }

  /* should set defaults */
  numInputFiles = 0;
  numMachines = 0;
  sprintf(currentPath, ".");
  sprintf(currentGOPPath, ".");
  sprintf(currentFramePath, ".");
#ifndef HPUX
  SetRemoteShell("rsh");
#else
  SetRemoteShell("remsh");
#endif

  switch(function) {
  case ENCODE_FRAMES:
    for ( index = FIRST_OPTION; index <= LAST_OPTION; index++ ) {
      optionSeen[index] = FALSE;
    }
    optionSeen[OPTION_IO_CONVERT] = FALSE;
    optionSeen[OPTION_SLAVE_CONVERT] = FALSE;
    break;
  case COMBINE_GOPS:
    for ( index = FIRST_OPTION; index <= LAST_OPTION; index++ ) {
      optionSeen[index] = TRUE;
    }

    optionSeen[OPTION_OUTPUT] = FALSE;
    break;
  case COMBINE_FRAMES:
    for ( index = FIRST_OPTION; index <= LAST_OPTION; index++ ) {
      optionSeen[index] = TRUE;
    }

    optionSeen[OPTION_GOP] = FALSE;
    optionSeen[OPTION_OUTPUT] = FALSE;
    break;
  }

  for( index=LAST_OPTION+1; index<= NUM_OPTIONS; index++ ) {
    optionSeen[index]=FALSE;
  }

  while ( fgets(input, 256, fpointer) != NULL ) {
    /* skip comments */
    if ( input[0] == '#' ) {            
      continue;
    }

    {
      int len = strlen(input);
      if (input[len-1] == '\n') {
    len--;
        input[len] = '\0'; /* get rid of newline */
    /* Junk whitespace */
    while ((len >= 0) && ((input[len-1] == ' ') || (input[len-1] == '\t'))) {
      input[--len] = '\0';
    }
      }
    }

    if (strlen(SkipSpacesTabs(input)) == 0) continue;

    switch(input[0]) {
    case 'A':
      if ( strncmp(input, "ASPECT_RATIO", 12) == 0 ) {
        charPtr = SkipSpacesTabs(&input[12]);
        aspectRatio = GetAspectRatio(charPtr);
        optionSeen[OPTION_ASPECT_RATIO] = TRUE;
      }
      break;

    case 'B':
      if ( strncmp(input, "BQSCALE", 7) == 0 ) {
        charPtr = SkipSpacesTabs(&input[7]);
        SetBQScale(atoi(charPtr));
        optionSeen[OPTION_BQSCALE] = TRUE;
      } else if ( strncmp(input, "BASE_FILE_FORMAT", 16) == 0 ) {
        charPtr = SkipSpacesTabs(&input[16]);
        SetFileFormat(charPtr);
        if ( (strncmp(charPtr,"YUV",3) == 0) || 
            (strcmp(charPtr,"Y") == 0) ) {
          yuvUsed = TRUE;
        }
        optionSeen[OPTION_BASE_FORMAT] = TRUE;
      } else if ( strncmp(input, "BSEARCH_ALG", 11) == 0 ) {
        charPtr = SkipSpacesTabs(&input[11]);
        SetBSearchAlg(charPtr);
        optionSeen[OPTION_BSEARCH_ALG] = TRUE;
      } else if ( strncmp(input, "BIT_RATE", 8) == 0 ) {
        charPtr = SkipSpacesTabs(&input[8]);
        setBitRate(charPtr);
        optionSeen[OPTION_BIT_RATE] = TRUE;
      } else if ( strncmp(input, "BUFFER_SIZE", 11) == 0 ) {
        charPtr = SkipSpacesTabs(&input[11]);
        setBufferSize(charPtr);
        optionSeen[OPTION_BUFFER_SIZE] = TRUE;                  
      }
      break;

    case 'C':
      if ( strncmp(input, "CDL_FILE", 8) == 0 ) {
        charPtr = SkipSpacesTabs(&input[8]);
        strcpy(specificsFile, charPtr);
        specificsOn=TRUE;
        optionSeen[OPTION_SPECIFICS] = TRUE;
      } else if ( strncmp(input, "CDL_DEFINES", 11) == 0 ) {
        charPtr = SkipSpacesTabs(&input[11]);
        strcpy(specificsDefines, charPtr);
        optionSeen[OPTION_DEFS_SPECIFICS] = TRUE;
      }
      break;

    case 'F':
      if ( strncmp(input, "FRAME_INPUT_DIR", 15) == 0 ) {
        charPtr = SkipSpacesTabs(&input[15]);
    if ( strncmp(charPtr, "stdin", 5) == 0 ||
        strncmp(charPtr, "STDIN", 5) == 0 ) {
          stdinUsed = TRUE;
          numInputFiles = MAXINT;
          numInputFileEntries = 1;
        }
        strcpy(currentFramePath, charPtr);
      } else if ( strncmp(input, "FRAME_INPUT", 11) == 0 ) {
        if ( function == COMBINE_FRAMES ) {
          ReadInputFileNames(fpointer, "FRAME_END_INPUT");
        }
      } else if ( strncmp(input, "FORCE_I_ALIGN", 13) == 0 ) {
        forceIalign = TRUE;
      } else if ( strncmp(input, "FORCE_ENCODE_LAST_FRAME", 23) == 0 ) {
        forceEncodeLast = TRUE;
      } else if ( strncmp(input, "FRAME_RATE", 10) == 0 ) {
        charPtr = SkipSpacesTabs(&input[10]);
        frameRate = GetFrameRate(charPtr);
        frameRateRounded = (int) VidRateNum[frameRate];
        if ( (frameRate % 3) == 1) {
          frameRateInteger = FALSE;
        }
        optionSeen[OPTION_FRAME_RATE] = TRUE;
      }
      break;

    case 'G':
      if ( strncmp(input, "GOP_SIZE", 8) == 0 ) {
        charPtr = SkipSpacesTabs(&input[8]);
        SetGOPSize(atoi(charPtr));
        optionSeen[OPTION_GOP] = TRUE;
      } else if ( strncmp(input, "GOP_INPUT_DIR", 13) == 0 ) {
        charPtr = SkipSpacesTabs(&input[13]);
        if ( strncmp(charPtr, "stdin", 5) == 0 ||
        strncmp(charPtr, "STDIN", 5) == 0 ) {
          stdinUsed = TRUE;
          numInputFiles = MAXINT;
          numInputFileEntries = 1;
        }
        strcpy(currentGOPPath, charPtr);
      } else if ( strncmp(input, "GOP_INPUT", 9) == 0 ) {
        if ( function == COMBINE_GOPS ) {
          ReadInputFileNames(fpointer, "GOP_END_INPUT");
        }
      } else if ( strncmp(input, "GAMMA", 5) == 0) {
        charPtr = SkipSpacesTabs(&input[5]);
        GammaCorrection = TRUE;
        sscanf(charPtr,"%f",&GammaValue);
        optionSeen[OPTION_GAMMA] = TRUE;
      }
      break;

    case 'I':
      if ( strncmp(input, "IQSCALE", 7) == 0 ) {
        charPtr = SkipSpacesTabs(&input[7]);
        SetIQScale(atoi(charPtr));
        optionSeen[OPTION_IQSCALE] = TRUE;
      } else if ( strncmp(input, "INPUT_DIR", 9) == 0 ) {
        charPtr = SkipSpacesTabs(&input[9]);
        if ( strncmp(charPtr, "stdin", 5) == 0 ||
        strncmp(charPtr, "STDIN", 5) == 0 ) {
          stdinUsed = TRUE;
          numInputFiles = MAXINT;
          numInputFileEntries = 1;
        }
        strcpy(currentPath, charPtr);
        optionSeen[OPTION_INPUT_DIR] = TRUE;
      } else if ( strncmp(input, "INPUT_CONVERT", 13) == 0 ) {
        charPtr = SkipSpacesTabs(&input[13]);
        strcpy(inputConversion, charPtr);
        optionSeen[OPTION_INPUT_CONVERT] = TRUE;
      } else if ( strcmp(input, "INPUT") == 0 ) { /* yes, strcmp */
        if ( function == ENCODE_FRAMES ) {
          ReadInputFileNames(fpointer, "END_INPUT");
          optionSeen[OPTION_INPUT] = TRUE;
        }
      } else if ( strncmp(input, "IO_SERVER_CONVERT", 17) == 0 ) {
        charPtr = SkipSpacesTabs(&input[17]);
        strcpy(ioConversion, charPtr);
        optionSeen[OPTION_IO_CONVERT] = TRUE;
      } else if ( strncmp(input, "IQTABLE", 7) == 0 ) {
        for ( row = 0; row < 8; row ++ ) {
          fgets(input, 256, fpointer);
          charPtr = input;
          if (8!=sscanf(charPtr,"%d %d %d %d %d %d %d %d",
                        &qtable[row*8+0],  &qtable[row*8+1],
                        &qtable[row*8+2],  &qtable[row*8+3],
                        &qtable[row*8+4],  &qtable[row*8+5],
                        &qtable[row*8+6],  &qtable[row*8+7])) {
            fprintf(stderr, "Line %d of IQTABLE doesn't have 8 elements!\n", row);
            exit(1);
          }
          for ( col = 0; col < 8; col ++ ) {
            if ((qtable[row*8+col]<1) || (qtable[row*8+col]>255)) {
              fprintf(stderr,
                      "Warning:  IQTable Element %1d,%1d (%d) corrected to 1-255.\n",
                      row+1, col+1, qtable[row*8+col]);
              qtable[row*8+col] = (qtable[row*8+col]<1)?1:255;
            }}
        }

        if ( qtable[0] != 8 ) {
          fprintf(stderr, "Warning:  IQTable Element 1,1 reset to 8, since it must be 8.\n");
          qtable[0] = 8;
        }
        customQtable = qtable;
        optionSeen[OPTION_IQTABLE] = TRUE;
      } else if ( strncmp(input, "INPUT", 5) == 0 ) { /* handle spaces after input */
        log(10.0);
        charPtr = SkipSpacesTabs(&input[5]);
        if ( function == ENCODE_FRAMES && *charPtr==0) {
          ReadInputFileNames(fpointer, "END_INPUT");
          optionSeen[OPTION_INPUT] = TRUE;
        }
      }
      break;

    case 'N':
      if ( strncmp(input, "NIQTABLE", 8) == 0 ) {
        for ( row = 0; row < 8; row ++ ) {
          fgets(input, 256, fpointer);
          charPtr = input;
          if (8!=sscanf(charPtr,"%d %d %d %d %d %d %d %d",
                        &niqtable[row*8+0],  &niqtable[row*8+1],
                        &niqtable[row*8+2],  &niqtable[row*8+3],
                        &niqtable[row*8+4],  &niqtable[row*8+5],
                        &niqtable[row*8+6],  &niqtable[row*8+7])) {
            fprintf(stderr, "Line %d of NIQTABLE doesn't have 8 elements!\n", row);
            exit(1);
          }
          for ( col = 0; col < 8; col++ ) {
            if ((niqtable[row*8+col]<1) || (niqtable[row*8+col]>255)) {
              fprintf(stderr,
                      "Warning:  NIQTable Element %1d,%1d (%d) corrected to 1-255.\n",
                      row+1, col+1, niqtable[row*8+col]);
              niqtable[row*8+col]=(niqtable[row*8+col]<1)?1:255;
            }}
        }

        customNIQtable = niqtable;
        optionSeen[OPTION_NIQTABLE] = TRUE;
      }
      break;

    case 'O':
      if ( strncmp(input, "OUTPUT", 6) == 0 ) {
        charPtr = SkipSpacesTabs(&input[6]);
        if ( whichGOP == -1 ) {
          strcpy(outputFileName, charPtr);
        } else {
          sprintf(outputFileName, "%s.gop.%d",
                  charPtr, whichGOP);
        }

        optionSeen[OPTION_OUTPUT] = TRUE;
      }
      break;

    case 'P':
      if ( strncmp(input, "PATTERN", 7) == 0 ) {
        charPtr = SkipSpacesTabs(&input[7]);
        SetFramePattern(charPtr);
        optionSeen[OPTION_PATTERN] = TRUE;
      } else if ( strncmp(input, "PIXEL", 5) == 0 ) {
        charPtr = SkipSpacesTabs(&input[5]);
        SetPixelSearch(charPtr);
        optionSeen[OPTION_PIXEL] = TRUE;
      } else if ( strncmp(input, "PQSCALE", 7) == 0 ) {
        charPtr = SkipSpacesTabs(&input[7]);
        SetPQScale(atoi(charPtr));
        optionSeen[OPTION_PQSCALE] = TRUE;
      } else if ( strncmp(input, "PSEARCH_ALG", 11) == 0 ) {
        charPtr = SkipSpacesTabs(&input[11]);
        SetPSearchAlg(charPtr);
        optionSeen[OPTION_PSEARCH_ALG] = TRUE;
      } else if ( strncmp(input, "PARALLEL_TEST_FRAMES", 20) == 0 ) {
        SetParallelPerfect(FALSE);
        charPtr = SkipSpacesTabs(&input[20]);
        parallelTestFrames = atoi(charPtr);
      } else if ( strncmp(input, "PARALLEL_TIME_CHUNKS", 20) == 0 ) {
        SetParallelPerfect(FALSE);
        charPtr = SkipSpacesTabs(&input[20]);
        parallelTimeChunks = atoi(charPtr);
      } else if ( strncmp(input, "PARALLEL_CHUNK_TAPER", 20) == 0 ) {
        SetParallelPerfect(FALSE);
        parallelTimeChunks = -1;
      } else if ( strncmp(input, "PARALLEL_PERFECT", 16) == 0 ) {
        SetParallelPerfect(TRUE);
      } else if ( strncmp(input, "PARALLEL", 8) == 0 ) {
        ReadMachineNames(fpointer);
        optionSeen[OPTION_PARALLEL] = TRUE;
      }
      break;

    case 'R':
      if ( strncmp(input, "RANGE", 5) == 0 ) {
        int num_ranges=0,a,b;
        charPtr = SkipSpacesTabs(&input[5]);
        optionSeen[OPTION_RANGE] = TRUE;
        num_ranges=sscanf(charPtr,"%d %d",&a,&b);
        if (num_ranges==2) {
          SetSearchRange(a,b);
        } else if (sscanf(charPtr,"%d [%d]",&a,&b)==2) {
          SetSearchRange(a,b);
        } else SetSearchRange(a,a);
      } else if ( strncmp(input, "REFERENCE_FRAME", 15) == 0 ) {
        charPtr = SkipSpacesTabs(&input[15]);
        SetReferenceFrameType(charPtr);
        optionSeen[OPTION_REF_FRAME] = TRUE;
      } else if ( strncmp(input, "RSH", 3) == 0 ) {
        charPtr = SkipSpacesTabs(&input[3]);
        SetRemoteShell(charPtr);
      } else if ( strncmp(input, "RESIZE", 6) == 0 ) {
        charPtr = SkipSpacesTabs(&input[6]);                    
        sscanf(charPtr, "%dx%d", &outputWidth, &outputHeight);
        outputWidth &= ~(DCTSIZE * 2 - 1);
        outputHeight &= ~(DCTSIZE * 2 - 1);
        optionSeen[OPTION_RESIZE] = TRUE;
      }
      break;

    case 'S':
      if ( strncmp(input, "SLICES_PER_FRAME", 16) == 0 ) {
        charPtr = SkipSpacesTabs(&input[16]);
        SetSlicesPerFrame(atoi(charPtr));
        optionSeen[OPTION_SPF] = TRUE;
      } else if ( strncmp(input, "SLAVE_CONVERT", 13) == 0 ) {
        charPtr = SkipSpacesTabs(&input[13]);
        strcpy(slaveConversion, charPtr);
        optionSeen[OPTION_SLAVE_CONVERT] = TRUE;
      } else if ( strncmp(input, "SPECIFICS_FILE", 14) == 0 ) {
        charPtr = SkipSpacesTabs(&input[14]);
        strcpy(specificsFile, charPtr);
        specificsOn=TRUE;
        optionSeen[OPTION_SPECIFICS] = TRUE;
      } else if ( strncmp(input, "SPECIFICS_DEFINES", 16) == 0 ) {
        charPtr = SkipSpacesTabs(&input[17]);
        strcpy(specificsDefines, charPtr);
        optionSeen[OPTION_DEFS_SPECIFICS] = TRUE;
      } else if (strncmp(input, "SEQUENCE_SIZE", 13) == 0) {
        charPtr = SkipSpacesTabs(&input[13]);
        mult_seq_headers = atoi(charPtr);
      } else if (strncmp(input, "SIZE", 4) == 0 ) {
        charPtr = SkipSpacesTabs(&input[4]);
        sscanf(charPtr, "%dx%d", &yuvWidth, &yuvHeight);
        realWidth = yuvWidth;
        realHeight = yuvHeight;
        Fsize_Validate(&yuvWidth, &yuvHeight);
        optionSeen[OPTION_YUV_SIZE] = TRUE;
      }
      break;

    case 'T':
      if ( strncmp(input, "TUNE", 4) == 0) {
        tuneingOn = TRUE;
        charPtr = SkipSpacesTabs(&input[4]);
    ParseTuneParam(charPtr);
      }
      break;

    case 'U':
      if ( strncmp(input, "USER_DATA", 9) == 0 ) {
        charPtr = SkipSpacesTabs(&input[9]);
        strcpy(userDataFileName, charPtr);
        optionSeen[OPTION_USER_DATA] = TRUE;
      }
      break;

    case 'Y':
      if (strncmp(input, "YUV_SIZE", 8) == 0 ) {
        charPtr = SkipSpacesTabs(&input[8]);
        sscanf(charPtr, "%dx%d", &yuvWidth, &yuvHeight);
        realWidth = yuvWidth;
        realHeight = yuvHeight;
        Fsize_Validate(&yuvWidth, &yuvHeight);
        optionSeen[OPTION_YUV_SIZE] = TRUE;
      }
      else if (strncmp(input, "Y_SIZE", 6) == 0 ) {
        charPtr = SkipSpacesTabs(&input[6]);
        sscanf(charPtr, "%dx%d", &yuvWidth, &yuvHeight);
        realWidth = yuvWidth;
        realHeight = yuvHeight;
        Fsize_Validate(&yuvWidth, &yuvHeight);
        optionSeen[OPTION_YUV_SIZE] = TRUE;
      }
      else if ( strncmp(input, "YUV_FORMAT", 10) == 0 ) {
        charPtr = SkipSpacesTabs(&input[10]);
        strcpy(yuvConversion, charPtr);
        optionSeen[OPTION_YUV_FORMAT] = TRUE;
      }
      break;
    }
  }

  fclose(fpointer);

  for ( index = FIRST_OPTION; index <= LAST_OPTION; index++ ) {
    if ( ! optionSeen[index] ) {

      /* INPUT unnecessary when stdin is used */
      if ((index == OPTION_INPUT) && stdinUsed) {
        continue;
      }

      fprintf(stderr, "ERROR:  Missing option '%s'\n", optionText[index]);
      exit(1);
    }
  }

  /* error checking */
  if ( yuvUsed ) {

    if (! optionSeen[OPTION_YUV_SIZE]) {
      fprintf(stderr, "ERROR:  YUV format used but YUV_SIZE not given\n");
      exit(1);
    }

    if (! optionSeen[OPTION_YUV_FORMAT]) {
      strcpy (yuvConversion, "EYUV");
      fprintf(stderr, "WARNING:  YUV format not specified; defaulting to Berkeley YUV (EYUV)\n\n");
    }

  }

  if ( stdinUsed && optionSeen[OPTION_PARALLEL] ) {
    fprintf(stderr, "ERROR:  stdin reading for parallel execution not enabled yet.\n");
    exit(1);
  }


  if ( optionSeen[OPTION_PARALLEL] && !optionSeen[OPTION_YUV_SIZE]) {
    fprintf(stderr, "ERROR:  Specify SIZE WxH for parallel encoding\n");
    exit(1);
  }

  if ( optionSeen[OPTION_IO_CONVERT] != optionSeen[OPTION_SLAVE_CONVERT] ) {
    fprintf(stderr, "ERROR:  must have either both IO_SERVER_CONVERT and SLAVE_CONVERT\n");
    fprintf(stderr, "        or neither\n");
    exit(1);
  }

  if ( optionSeen[OPTION_DEFS_SPECIFICS] && !optionSeen[OPTION_SPECIFICS]) {
    fprintf(stderr, "ERROR:  does not make sense to define Specifics file options, but no specifics file!\n");
    exit(1);
  }

  SetIOConvert(optionSeen[OPTION_IO_CONVERT]);

  SetResize(optionSeen[OPTION_RESIZE]);

  if ( function == ENCODE_FRAMES ) {
    SetFCode();

    if ( psearchAlg == PSEARCH_TWOLEVEL )
      SetPixelSearch("HALF");
  }

  return TRUE;
}


/*===========================================================================*
 *
 * GetNthInputFileName
 *
 *    finds the nth input file name
 *
 * RETURNS:    name is placed in already allocated fileName string
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
  GetNthInputFileName(fileName, n)
char *fileName;
int n;
{
  static int    lastN = 0, lastMapN = 0, lastSoFar = 0;
  int        mapN;
  register int index;
  int        soFar;
  int        loop;
  int        numPadding;
  char    numBuffer[33];

  if ( stdinUsed ) {
    return;
  }

  /* assumes n is within bounds 0...numInputFiles-1 */

  if ( n >= lastN ) {
    soFar = lastSoFar;
    index = lastMapN;
  } else {
    soFar = 0;
    index = 0;
  }

  while ( soFar + inputFileEntries[index]->numFiles <= n ) {
    soFar +=  inputFileEntries[index]->numFiles;
    index++;
  }

  mapN = index;

  index = inputFileEntries[mapN]->startID +
    inputFileEntries[mapN]->skip*(n-soFar);

  numPadding = inputFileEntries[mapN]->numPadding;

  if ( numPadding != -1 ) {
    sprintf(numBuffer, "%32d", index);
    for ( loop = 32-numPadding; loop < 32; loop++ ) {
      if ( numBuffer[loop] != ' ' ) {
        break;
      } else {
        numBuffer[loop] = '0';
      }
    }

    if (inputFileEntries[mapN]->repeat != TRUE) {
      sprintf(fileName, "%s%s%s",
          inputFileEntries[mapN]->left,
          &numBuffer[32-numPadding],
          inputFileEntries[mapN]->right);
    } else {
      sprintf(fileName, "%s", inputFileEntries[mapN]->left);
    }
  } else {
    if (inputFileEntries[mapN]->repeat != TRUE) {
      sprintf(fileName, "%s%d%s",
          inputFileEntries[mapN]->left,
          index,
          inputFileEntries[mapN]->right);
    } else {
      sprintf(fileName, "%s", inputFileEntries[mapN]->left);
    }
  }

  lastN = n;
  lastMapN = mapN;
  lastSoFar = soFar;
}


/*=====================*
 * INTERNAL PROCEDURES *
 *=====================*/

/*===========================================================================*
 *
 * ReadMachineNames
 *
 *    read a list of machine names for parallel execution
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    machine info updated
 *
 *===========================================================================*/
static void
  ReadMachineNames(fpointer)
FILE *fpointer;
{
  char    input[256];
  char    *charPtr;

  while ( (fgets(input, 256, fpointer) != NULL) &&
     (strncmp(input, "END_PARALLEL", 12) != 0) ) {
    if ( input[0] == '#' || input[0] == '\n') {
      continue;
    }

    if ( strncmp(input, "REMOTE", 6) == 0 ) {
      charPtr = SkipSpacesTabs(&input[6]);
      remote[numMachines] = TRUE;

      sscanf(charPtr, "%s %s %s %s", machineName[numMachines],
         userName[numMachines], executable[numMachines],
         remoteParamFile[numMachines]);
    } else {
      remote[numMachines] = FALSE;

      sscanf(input, "%s %s %s", machineName[numMachines],
         userName[numMachines], executable[numMachines]);
    }

    numMachines++;
  }
}


/*===========================================================================*
 *
 * ReadInputFileNames
 *
 *    read a list of input file names
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    info stored for retrieval using GetNthInputFileName
 *
 *===========================================================================*/
static void
  ReadInputFileNames(fpointer, endInput)
FILE *fpointer;
char *endInput;
{
  char      input[256];
  char      left[256], right[256];
  char      *globPtr, *charPtr;
  char      leftNumText[256], rightNumText[256];
  char      skipNumText[256];
  int        leftNum, rightNum;
  int        skipNum;
  boolean   padding;
  int        numPadding = 0;
  int        length;
  char      full_path[MAXPATHLEN + 256];
  FILE      *jmovie;

  inputFileEntries = (InputFileEntry **) malloc(INPUT_ENTRY_BLOCK_SIZE*
                        sizeof(InputFileEntry *));
  maxInputFileEntries = INPUT_ENTRY_BLOCK_SIZE;

  length = strlen(endInput);

  /* read input files up until endInput */
  while ( (fgets(input, 256, fpointer) != NULL) &&
     (strncmp(input, endInput, length) != 0) ) {

    /* if input is coming in via standard input, keep on looping till the 
     * endInput string is reached so that the program points to the right 
     * place on exit.
     */
    if ( stdinUsed ) {
      continue;
    }
            
    /* ignore comments and newlines */
    if ( (input[0] == '#') || (input[0] == '\n') ) { 
      continue;
    }

    if (input[0] == '`' ) {    /* Recurse for commands */
      FILE *fp;
      char cmd[300], *start, *end, tmp[300], cdcmd[110];

      start = &input[1];
      end = &input[strlen(input)-1];

      while (*end != '`') {
        end--;
      }

      end--;

      if (optionSeen[OPTION_INPUT_DIR] == TRUE) {
        sprintf(cdcmd,"cd %s;",currentPath);
      } else {
        strcpy(cdcmd,"");
      }
      strncpy(tmp,start,end-start+1);
      sprintf(cmd,"(%s %s)", cdcmd, tmp);

      fp = popen(cmd,"r");
      if (fp == NULL) {
        fprintf(stderr,"Command failed! Could not open piped command:\n%s\n",cmd);
        continue;
      }
      ReadInputFileNames(fp,"HOPE-THIS_ISNT_A_FILENAME.xyz5555");
      continue;
    }


    /* get rid of trailing whitespace including newline */
    while (isspace(input[strlen(input)-1])) {
      input[strlen(input)-1] = '\0';
    }

    if ( numInputFileEntries == maxInputFileEntries ) {    /* more space! */
      maxInputFileEntries += INPUT_ENTRY_BLOCK_SIZE;
      inputFileEntries = (InputFileEntry **) realloc(inputFileEntries,
                             maxInputFileEntries*
                             sizeof(InputFileEntry *));
    }

    inputFileEntries[numInputFileEntries] = (InputFileEntry *)
      malloc(sizeof(InputFileEntry));

    if ( input[strlen(input)-1] == ']' ) {
      inputFileEntries[numInputFileEntries]->glob = TRUE;
      inputFileEntries[numInputFileEntries]->repeat = FALSE;

      /* star expand */

      globPtr = input;
      charPtr = left;
      /* copy left of '*' */
      while ( (*globPtr != '\0') && (*globPtr != '*') ) {
        *charPtr = *globPtr;
        charPtr++;
        globPtr++;
      }
      *charPtr = '\0';

      if (*globPtr == '\0') {
    fprintf(stderr, "WARNING: expanding non-star regular expression\n");
    inputFileEntries[numInputFileEntries]->repeat = TRUE;
    globPtr = input;
    charPtr = left;
    /* recopy left of whitespace */
    while ( (*globPtr != '\0') && (*globPtr != '*') && 
            (*globPtr != ' ')  && (*globPtr != '\t')) {
      *charPtr = *globPtr;
      charPtr++;
      globPtr++;
    }
    *charPtr = '\0';
    *right = '\0';
      } else {

    globPtr++;
    charPtr = right;
    /* copy right of '*' */
    while ( (*globPtr != '\0') && (*globPtr != ' ') &&
           (*globPtr != '\t') ) {
      *charPtr = *globPtr;
      charPtr++;
      globPtr++;
    }
    *charPtr = '\0';
      }
      
      globPtr = SkipSpacesTabs(globPtr);

      if ( *globPtr != '[' ) {
        fprintf(stderr, "ERROR:  Invalid input file expansion expression (no '[')\n");
        exit(1);
      }

      globPtr++;
      charPtr = leftNumText;
      /* copy left number */
      while ( isdigit(*globPtr) ) {
        *charPtr = *globPtr;
        charPtr++;
        globPtr++;
      }
      *charPtr = '\0';

      if ( *globPtr != '-' ) {
        fprintf(stderr, "ERROR:  Invalid input file expansion expression (no '-')\n");
        exit(1);
      }

      globPtr++;
      charPtr = rightNumText;
      /* copy right number */
      while ( isdigit(*globPtr) ) {
        *charPtr = *globPtr;
        charPtr++;
        globPtr++;
      }
      *charPtr = '\0';
      if ( atoi(rightNumText) < atoi(leftNumText) ) {
        fprintf(stderr, "ERROR:  Beginning of input range is higher than end.\n");
        exit(1);
      }


      if ( *globPtr != ']' ) {
        if ( *globPtr != '+' ) {
          fprintf(stderr, "ERROR:  Invalid input file expansion expression (no ']')\n");
          exit(1);
        }

        globPtr++;
        charPtr = skipNumText;
        /* copy skip number */
        while ( isdigit(*globPtr) ) {
          *charPtr = *globPtr;
          charPtr++;
          globPtr++;
        }
        *charPtr = '\0';

        if ( *globPtr != ']' ) {
          fprintf(stderr, "ERROR:  Invalid input file expansion expression (no ']')\n");
          exit(1);
        }

        skipNum = atoi(skipNumText);
      } else {
        skipNum = 1;
      }

      leftNum = atoi(leftNumText);
      rightNum = atoi(rightNumText);

      if ( (leftNumText[0] == '0') && (leftNumText[1] != '\0') ) {
        padding = TRUE;
        numPadding = strlen(leftNumText);
      } else {
        padding = FALSE;
      }

      inputFileEntries[numInputFileEntries]->startID = leftNum;
      inputFileEntries[numInputFileEntries]->endID = rightNum;
      inputFileEntries[numInputFileEntries]->skip = skipNum;
      inputFileEntries[numInputFileEntries]->numFiles = (rightNum-leftNum+1)/skipNum;
      strcpy(inputFileEntries[numInputFileEntries]->left, left);
      strcpy(inputFileEntries[numInputFileEntries]->right, right);
      if ( padding ) {
        inputFileEntries[numInputFileEntries]->numPadding = numPadding;
      } else {
        inputFileEntries[numInputFileEntries]->numPadding = -1;
      }
    } else {
      strcpy(inputFileEntries[numInputFileEntries]->left, input);
      if (baseFormat == JMOVIE_FILE_TYPE) {
        inputFileEntries[numInputFileEntries]->glob = TRUE;
        full_path[0] = '\0';
        strcpy(full_path, currentPath);
    
        if (! stdinUsed) {
          strcat(full_path, "/");
          strcat(full_path, input);
          jmovie = fopen(input, "rb"); 

          if (jmovie == NULL) {
            perror (input); 
            exit (1);
          }

          fseek (jmovie, (8*sizeof(char)), 0);
          fseek (jmovie, (2*sizeof(int)), 1);

          if (fread (&(inputFileEntries[numInputFileEntries]->numFiles),
                     sizeof(int), 1, jmovie) != 1) {
            perror ("Error in reading number of frames in JMOVIE");
            exit(1);
          }
          fclose (jmovie);
        }  

        strcpy(inputFileEntries[numInputFileEntries]->right,".jpg");
        inputFileEntries[numInputFileEntries]->numPadding = -1;
        inputFileEntries[numInputFileEntries]->startID = 1;
        inputFileEntries[numInputFileEntries]->endID = (inputFileEntries[numInputFileEntries]->numFiles-1);
        inputFileEntries[numInputFileEntries]->skip = 1;
        if (! realQuiet) {
          fprintf (stdout, "Encoding all %d frames from JMOVIE.\n", inputFileEntries[numInputFileEntries]->endID);
        }
      } else {
        inputFileEntries[numInputFileEntries]->glob = FALSE;
        inputFileEntries[numInputFileEntries]->numFiles = 1;
        /* fixes a bug from version 1.3: */
        inputFileEntries[numInputFileEntries]->numPadding = 0;
        /* fixes a bug from version 1.4 */
        strcpy(inputFileEntries[numInputFileEntries]->right,"\0");
        inputFileEntries[numInputFileEntries]->startID = 0;
        inputFileEntries[numInputFileEntries]->endID = 0;
        inputFileEntries[numInputFileEntries]->skip = 0;
      }
    }

    numInputFiles += inputFileEntries[numInputFileEntries]->numFiles;
    numInputFileEntries++;
  }
}


/*===========================================================================*
 *
 * SkipSpacesTabs
 *
 *    skip all spaces and tabs
 *
 * RETURNS:    point to next character not a space or tab
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
char *
  SkipSpacesTabs(start)
char *start;
{
  while ( (*start == ' ') || (*start == '\t') ) {
    start++;
  }

  return start;
}


/*===========================================================================*
 *
 * GetFrameRate
 *
 * take a character string with the input frame rate 
 * and return the correct frame rate code for use in the Sequence header
 *
 * RETURNS: frame rate code as per MPEG-I spec
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static int
  GetFrameRate(p)
char *p;
{
  float   rate;
  int        thouRate;

  sscanf(p, "%f", &rate);
  thouRate = (int)(0.5+1000.0*rate);

  if ( thouRate == 23976 )         return 1;
  else if ( thouRate == 24000 )    return 2;
  else if ( thouRate == 25000 )    return 3;
  else if ( thouRate == 29970 )    return 4;
  else if ( thouRate == 30000 )    return 5;
  else if ( thouRate == 50000 )    return 6;
  else if ( thouRate == 59940 )    return 7;
  else if ( thouRate == 60000 )    return 8;
  else {
    fprintf(stderr,"INVALID FRAME RATE: %s frames/sec\n", p);
    exit(1);
  }
}


/*===========================================================================*
 *
 * GetAspectRatio
 *
 * take a character string with the pixel aspect ratio
 * and returns the correct aspect ratio code for use in the Sequence header
 *
 * RETURNS: aspect ratio code as per MPEG-I spec
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static int
  GetAspectRatio(p)
char *p;
{
  float   ratio;
  int        ttRatio;

  sscanf(p, "%f", &ratio);
  ttRatio = (int)(0.5+ratio*10000.0);

  if ( ttRatio == 10000 )       return 1;
  else if ( ttRatio ==  6735 )    return 2;
  else if ( ttRatio ==  7031 )    return 3;
  else if ( ttRatio ==  7615 )    return 4;
  else if ( ttRatio ==  8055 )    return 5;
  else if ( ttRatio ==  8437 )    return 6;
  else if ( ttRatio ==  8935 )    return 7;
  else if ( ttRatio ==  9157 )    return 8;
  else if ( ttRatio ==  9815 )    return 9;
  else if ( ttRatio == 10255 )    return 10;
  else if ( ttRatio == 10695 )    return 11;
  else if ( ttRatio == 10950 )    return 12;
  else if ( ttRatio == 11575 )    return 13;
  else if ( ttRatio == 12015 )    return 14;
  else {
    fprintf(stderr,"INVALID ASPECT RATIO: %s frames/sec\n", p);
    exit(1);
  }
}






/****************************************************************
 *  Jim Boucher's code
 *
 *
 ****************************************************************/
void
JM2JPEG()
{
  char full_path[MAXPATHLEN + 256];
  char inter_file[MAXPATHLEN +256]; 
  int ci;

  for(ci = 0; ci < numInputFileEntries; ci++) {
    inter_file[0] = '\0';
    full_path[0] = '\0';
    strcpy(full_path, currentPath);
    
    if (! stdinUsed) {
      strcat(full_path, "/");
      strcat(full_path, inputFileEntries[ci]->left);
      strcpy(inter_file,full_path);
    
      if (! realQuiet) {
        fprintf(stdout, "Extracting JPEG's in the JMOVIE from %s\n",full_path);
      }
    
      JMovie2JPEG(full_path,
                  inter_file,
                  inputFileEntries[ci]->startID, inputFileEntries[ci]->endID);
    } else {
      fprintf (stderr, "ERROR: JMovie format not supported with stdin yet.\n");
      exit(1);
    }
      
  }
}
