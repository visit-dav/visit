/*===========================================================================*
 * main.c                                     *
 *                                         *
 *    Main procedure                                 *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    main                                     *
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
 *  $Header: /n/picasso/project/mpeg/mpeg_dist/mpeg_encode/RCS/main.c,v 1.25 1995/08/07 21:44:21 smoot Exp $
 *  $Log: main.c,v $
 *  Revision 1.25  1995/08/07 21:44:21  smoot
 *  renamed index -> idx; save the encoder's name; compute frame types ahead of time
 *
 *  Revision 1.24  1995/06/21 18:25:57  smoot
 *  added binary write flag (DOS!)
 *
 * Revision 1.23  1995/05/16  06:25:28  smoot
 * added TUNEing init and float-dct == float_dct
 *
 * Revision 1.22  1995/05/16  00:15:05  smoot
 * fixed usage print
 *
 * Revision 1.21  1995/05/11  23:59:56  smoot
 * *** empty log message ***
 *
 * Revision 1.20  1995/02/02  20:05:37  eyhung
 * fixed smoot typo in 1.19
 *
 * Revision 1.19  1995/02/02  18:56:11  smoot
 * ANSI-ified some prototypes
 *
 * Revision 1.18  1995/02/01  21:47:37  smoot
 * cleanup
 *
 * Revision 1.17  1995/01/31  22:22:49  eyhung
 * Fixed steve's typo and added float_dct to Usage()
 *
 * Revision 1.16  1995/01/31  21:44:08  smoot
 * Added -float_dct option
 *
 * Revision 1.15  1995/01/31  01:19:39  eyhung
 * removed -interactive
 *
 * Revision 1.14  1995/01/27  21:56:57  eyhung
 * Deleted setting JMOVIE_TYPE to JPEG_TYPE since we need to know
 * if we started with a JMOVIE for getting input files
 *
 * Revision 1.13  1995/01/19  23:50:06  eyhung
 * Removed printing of output file to screen - done at end of encoding now.
 *
 * Revision 1.12  1995/01/19  23:08:41  eyhung
 * Changed copyrights
 *
 * Revision 1.11  1995/01/17  08:25:44  eyhung
 * added -interactive to Usage
 *
 * Revision 1.10  1995/01/17  08:24:53  eyhung
 * Added -interactive option
 *
 * Revision 1.9  1995/01/16  08:04:10  eyhung
 * More realQuiet stuff.
 *
 * Revision 1.8  1995/01/16  07:38:49  eyhung
 * Added realquiet option
 *
 * Revision 1.7  1994/11/14  22:32:01  smoot
 * Merged specifics and rate control
 *
 * Revision 1.6  1994/11/12  02:11:52  keving
 * nothing
 *
 * Revision 1.5  1994/03/15  00:27:11  keving
 * nothing
 *
 * Revision 1.4  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.3  1993/07/22  22:23:43  keving
 * nothing
 *
 * Revision 1.2  1993/06/30  20:06:09  keving
 * nothing
 *
 * Revision 1.1  1993/02/17  23:18:20  dwallach
 * Initial revision
 *
 */


/*==============*
 * HEADER FILES *
 *==============*/

#include <assert.h>
#include "all.h"
#include "mtypes.h"
#include "mpeg.h"
#include "search.h"
#include "prototypes.h"
#include "param.h"
#include "parallel.h"
#include "readframe.h"
#include "combine.h"
#include "frames.h"
#include "jpeg.h"
#include "specifics.h"
#include "opts.h"
#include <time.h>

int    main _ANSI_ARGS_((int argc, char **argv));

/*==================*
 * STATIC VARIABLES *
 *==================*/

static int    frameStart = -1;
static int    frameEnd;


/*==================*
 * GLOBAL VARIABLES *
 *==================*/

extern time_t IOtime;
int    whichGOP = -1;
boolean    childProcess = FALSE;
boolean    ioServer = FALSE;
boolean    outputServer = FALSE;
boolean    decodeServer = FALSE;
int    quietTime = 0;
boolean realQuiet = FALSE;
boolean    frameSummary = TRUE;
boolean debugSockets = FALSE;
boolean debugMachines = FALSE;
boolean showBitRatePerFrame = FALSE;
boolean    computeMVHist = FALSE;
int     baseFormat;
extern  boolean specificsOn;
extern  FrameSpecList *fsl;
boolean pureDCT=FALSE;
char    encoder_name[1024];

/*===============================*
 * INTERNAL PROCEDURE prototypes *
 *===============================*/

static void Usage _ANSI_ARGS_((void));
static void CompileTests _ANSI_ARGS_((void));


/*================================*
 * External PROCEDURE prototypes  *
 *================================*/

void init_idctref _ANSI_ARGS_((void));
void init_fdct _ANSI_ARGS_((void));

/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/


/*===========================================================================*
 *
 * main
 *
 *    see man page.  run without arguments to see usage
 *
 * RETURNS:    0 if all is well; 1 on most if not all errors
 *
 *===========================================================================*/
int
main(argc, argv)
    int argc;
    char **argv;
{
    FILE *ofp = NULL;
    register int idx;
    int        function = ENCODE_FRAMES;
    int        portNumber = 0;
    char    *hostName = NULL;
    int32_mpeg_t   totalTime = -1;
    int        maxMachines = 0x7fffffff;
    int        outputFrames = 0;
    time_t  initTimeStart;
    time_t  framesTimeStart, framesTimeEnd;

    strcpy(encoder_name, argv[0]);

    CompileTests();

    time(&initTimeStart);

    if ( argc == 1 ) {
    Usage();
    }

    SetStatFileName("");

    /* parse the arguments */
    idx = 1;
    while ( idx < argc-1 ) {
    if ( argv[idx][0] != '-' ) {
        Usage();
    }

    if ( strcmp(argv[idx], "-stat") == 0 ) {
        if ( idx+1 < argc-1 ) {
        SetStatFileName(argv[idx+1]);
        idx += 2;
        } else {
        Usage();
        }
    } else if ( strcmp(argv[idx], "-gop") == 0 ) {
        if ( (function != ENCODE_FRAMES) || (frameStart != -1) ) {
        Usage();
        }

        if ( idx+1 < argc-1 ) {
        whichGOP = atoi(argv[idx+1]);
        idx += 2;
        } else {
        Usage();
        }
    } else if ( strcmp(argv[idx], "-frames") == 0 ) {
        if ( (function != ENCODE_FRAMES) || (whichGOP != -1) ) {
        Usage();
        }

        if ( idx+2 < argc-1 ) {
        frameStart = atoi(argv[idx+1]);
        frameEnd = atoi(argv[idx+2]);

        if ( (frameStart > frameEnd) || (frameStart < 0) ) {
            fprintf(stderr, "ERROR:  bad frame numbers!\n");
            Usage();
        }

        idx += 3;
        } else {
        Usage();
        }
    } else if ( strcmp(argv[idx], "-combine_gops") == 0 ) {
        if ( (function != ENCODE_FRAMES) || (whichGOP != -1) || 
         (frameStart != -1) ) {
        Usage();
        }

        function = COMBINE_GOPS;
        idx++;
    } else if ( strcmp(argv[idx], "-combine_frames") == 0 ) {
        if ( (function != ENCODE_FRAMES) || (whichGOP != -1) ||
         (frameStart != -1) ) {
        Usage();
        }

        function = COMBINE_FRAMES;
        idx++;
    } else if ( strcmp(argv[idx], "-child") == 0 ) {
        if ( idx+7 < argc-1 ) {
        hostName = argv[idx+1];
        portNumber = atoi(argv[idx+2]);
        ioPortNumber = atoi(argv[idx+3]);
        combinePortNumber = atoi(argv[idx+4]);
        decodePortNumber = atoi(argv[idx+5]);
        machineNumber = atoi(argv[idx+6]);
        remoteIO = atoi(argv[idx+7]);

        IOhostName = hostName;
        } else {
        Usage();
        }

        childProcess = TRUE;
        idx += 8;
    } else if ( strcmp(argv[idx], "-io_server") == 0 ) {
        if ( idx+2 < argc-1 ) {
        hostName = argv[idx+1];
        portNumber = atoi(argv[idx+2]);
        } else {
        Usage();
        }

        ioServer = TRUE;
        idx += 3;
    } else if ( strcmp(argv[idx], "-output_server") == 0 ) {
        if ( idx+3 < argc-1 ) {
        hostName = argv[idx+1];
        portNumber = atoi(argv[idx+2]);
        outputFrames = atoi(argv[idx+3]);
        } else {
        Usage();
        }

        function = COMBINE_FRAMES;
        outputServer = TRUE;
        idx += 4;
    } else if ( strcmp(argv[idx], "-decode_server") == 0 ) {
        if ( idx+3 < argc-1 ) {
        hostName = argv[idx+1];
        portNumber = atoi(argv[idx+2]);
        outputFrames = atoi(argv[idx+3]);
        } else {
        Usage();
        }

        function = COMBINE_FRAMES;
        decodeServer = TRUE;
        idx += 4;
    } else if ( strcmp(argv[idx], "-nice") == 0 ) {
        niceProcesses = TRUE;
        idx++;
    } else if ( strcmp(argv[idx], "-max_machines") == 0 ) {
        if ( idx+1 < argc-1 ) {
        maxMachines = atoi(argv[idx+1]);
        } else {
        Usage();
        }

        idx += 2;
    } else if ( strcmp(argv[idx], "-quiet") == 0 ) {
        if ( idx+1 < argc-1 ) {
        quietTime = atoi(argv[idx+1]);
        } else {
        Usage();
        }

        idx += 2;
    } else if ( strcmp(argv[idx], "-realquiet") == 0 ) {
            realQuiet = TRUE;
        idx++;
    } else if (( strcmp(argv[idx], "-float_dct") == 0 ) ||
           ( strcmp(argv[idx], "-float-dct") == 0 )) {
        pureDCT = TRUE;
          init_idctref();
        init_fdct();
        idx++;
    } else if ( strcmp(argv[idx], "-no_frame_summary") == 0 ) {
        if ( idx < argc-1 ) {
        frameSummary = FALSE;
        } else {
        Usage();
        }

        idx++;
    } else if ( strcmp(argv[idx], "-snr") == 0 ) {
        printSNR = TRUE;
        idx++;
    } else if ( strcmp(argv[idx], "-mse") == 0 ) {
        printSNR =  printMSE = TRUE;
        idx++;
    } else if ( strcmp(argv[idx], "-debug_sockets") == 0 ) {
        debugSockets = TRUE;
        idx++;
    } else if ( strcmp(argv[idx], "-debug_machines") == 0 ) {
        debugMachines = TRUE;
        idx++;
    } else if ( strcmp(argv[idx], "-bit_rate_info") == 0 ) {
        if ( idx+1 < argc-1 ) {
        showBitRatePerFrame = TRUE;
        SetBitRateFileName(argv[idx+1]);
        idx += 2;
        } else {
        Usage();
        }
    } else if ( strcmp(argv[idx], "-mv_histogram") == 0 ) {
        computeMVHist = TRUE;
        idx++;
    } else {
        Usage();
    }
    }

    if ( ! ReadParamFile(argv[argc-1], function) ) {
    Usage();
    }

    /* Jim Boucher's stuff:
    if we are using a movie format then break up into frames*/
    if ( (!childProcess) && (baseFormat == JMOVIE_FILE_TYPE) ) {
         JM2JPEG();
    }

    if ( printSNR || (referenceFrame == DECODED_FRAME) ) {
    decodeRefFrames = TRUE;
    }

    numMachines = min(numMachines, maxMachines);

    Tune_Init();
    Frame_Init();

#ifdef BLEAH
    time_t  initTimeEnd;

    time(&initTimeEnd);
    fprintf(stdout, "INIT TIME:  %d seconds\n",
        initTimeEnd-initTimeStart);
    fflush(stdout);
#endif

    if (specificsOn) Specifics_Init();

    ComputeFrameTable();

    if ( ioServer ) {
    StartIOServer(numInputFiles, hostName, portNumber);
    return 0;
    } else if ( outputServer ) {
    StartCombineServer(outputFrames, outputFileName, hostName, portNumber);
    return 0;
    } else if ( decodeServer ) {
    StartDecodeServer(outputFrames, outputFileName, hostName, portNumber);
    return 0;
    }

    if ( (frameStart == -1) &&
     ((numMachines == 0) || (function != ENCODE_FRAMES)) ) {
    if ( (ofp = fopen(outputFileName, "wb")) == NULL ) {
        fprintf(stderr, "ERROR:  Could not open output file!\n");
        exit(1);
    }
    }

    if ( function == ENCODE_FRAMES ) {
    if ( (numMachines == 0) || (frameStart != -1) ) {
        time(&framesTimeStart);
        totalTime = GenMPEGStream(whichGOP, frameStart, frameEnd,
                      customQtable, customNIQtable,
                      numInputFiles, ofp,
                      outputFileName);
        time(&framesTimeEnd);
        if ( childProcess && (! realQuiet) ) {
#ifdef BLEAH
        fprintf(stdout, "SCHEDULE:  MACHINE %d FRAMES %d-%d TIME %d-%d IOTIME %d\n",
            machineNumber, frameStart, frameEnd,
            framesTimeStart, framesTimeEnd,
            IOtime);
#endif
        fprintf(stdout, "%s:  FRAMES %d-%d (%d seconds)\n",
            getenv("HOST"), frameStart, frameEnd,
            (int) (framesTimeEnd-framesTimeStart));
        fflush(stdout);
        }
    } else {
        /* check if parameter file has absolute path */
        if ( (argv[argc-1][0] != '/') && (argv[argc-1][0] != '~') ) {
        fprintf(stderr, "ERROR:  For parallel execution, please use absolute path for parameter file!\n");
        exit(1);
        } else {
        StartMasterServer(numInputFiles, argv[argc-1], outputFileName);
        }
    }
    } else if ( function == COMBINE_GOPS ) {
    GOPStoMPEG(numInputFiles, outputFileName, ofp);
    } else if ( function == COMBINE_FRAMES ) {
    FramesToMPEG(numInputFiles, outputFileName, ofp, FALSE);
    }

    if ( childProcess ) {
    while ( NotifyMasterDone(hostName, portNumber, machineNumber,
                 totalTime,
                 &frameStart, &frameEnd) ) {
        /* do more frames */
        time(&framesTimeStart);
        totalTime = GenMPEGStream(-1, frameStart, frameEnd,
                      customQtable, customNIQtable,
                      numInputFiles, NULL,
                      outputFileName);
        time(&framesTimeEnd);

        if (! realQuiet) {
#ifdef BLEAH
        fprintf(stdout, "SCHEDULE:  MACHINE %d FRAMES %d-%d TIME %d-%d IOTIME %d\n",
            machineNumber, frameStart, frameEnd,
            framesTimeStart, framesTimeEnd,
            IOtime);
#endif
        fprintf(stdout, "%s:  FRAMES %d-%d (%d seconds)\n",
            getenv("HOST"), frameStart, frameEnd,
            (int) (framesTimeEnd-framesTimeStart));
        fflush(stdout);
        }

    }
    }

    Frame_Exit();

    return 0;    /* all is well */
}


/*=====================*
 * INTERNAL PROCEDURES *
 *=====================*/

/*===========================================================================*
 *
 * Usage
 *
 *    prints out usage for the program
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
Usage()
{
    fprintf(stderr, "Usage:  mpeg_encode [options] param_file\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "\t-stat stat_file:  append stats to stat_file\n");
    fprintf(stderr, "\t-quiet n:  don't report remaining time for at least n seconds\n");
    fprintf(stderr, "\t-realquiet:  output nothing at all if successful\n");
    fprintf(stderr, "\t-no_frame_summary:  suppress frame summary lines\n");
    fprintf(stderr, "\t-float_dct:  use more accurate floating point DCT\n");
    fprintf(stderr, "\t-gop gop_num:  encode only the numbered GOP\n");
    fprintf(stderr, "\t-combine_gops:  combine GOP files instead of encode\n");
    fprintf(stderr, "\t-frames first_frame last_frame:  encode only the specified frames\n");
    fprintf(stderr, "\t-combine_frames:  combine frame files instead of encode\n");
    fprintf(stderr, "\t-nice:  run slave processes nicely\n");
    fprintf(stderr, "\t-max_machines num_machines:  use at most num_machines machines\n");
    fprintf(stderr, "\t-snr:  print signal-to-noise ratio\n");
    fprintf(stderr, "\t-bit_rate_info rate_file:  put bit rate in specified file\n");
    fprintf(stderr, "\t-mv_histogram:  show histograms of motion vectors\n");
    exit(1);

/* extended usage (used by parallel code; shouldn't be called by user):
    -child parallelHostName portNumber ioPortNumber combinePortNumber machineNumber remote
    -io_server parallelHostName portNumber
    
    (remote = 1 if need to use ioPortNumber)
 */
}


static void
CompileTests()
{
    assert(sizeof(uint8_mpeg_t) == 1);
    assert(sizeof(uint16_mpeg_t) == 2);
    assert(sizeof(uint32_mpeg_t) == 4);
    assert(sizeof(int8_mpeg_t) == 1);
    assert(sizeof(int16_mpeg_t) == 2);
    assert(sizeof(int32_mpeg_t) == 4);

    if ( (-8 >> 3) != -1 ) {
    fprintf(stderr, "ERROR:  Right shifts are NOT arithmetic!!!\n");
    fprintf(stderr, "Change >> to multiplies by powers of 2\n");
    exit(1);
    }
}
