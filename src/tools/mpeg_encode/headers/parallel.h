/*===========================================================================*
 * parallel.h                                     *
 *                                         *
 *    parallel encoding                             *
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
 *  $Header: /n/picasso/project/mpeg/mpeg_dist/mpeg_encode/headers/RCS/parallel.h,v 1.5 1995/08/15 23:43:26 smoot Exp $
 *  $Log: parallel.h,v $
 *  Revision 1.5  1995/08/15 23:43:26  smoot
 *  *** empty log message ***
 *
 *  Revision 1.4  1995/01/19 23:55:08  eyhung
 *  Changed copyrights
 *
 * Revision 1.3  1994/11/12  02:12:53  keving
 * nothing
 *
 * Revision 1.2  1993/07/22  22:24:23  keving
 * nothing
 *
 * Revision 1.1  1993/07/09  00:17:23  keving
 * nothing
 *
 */


/*==============*
 * HEADER FILES *
 *==============*/

#include "ansi.h"
#include "bitio.h"
#include "frame.h"


/*===============================*
 * EXTERNAL PROCEDURE prototypes *
 *===============================*/

extern void StartMasterServer _ANSI_ARGS_((int numInputFiles,
                         char *paramFile,
                         char *outputFileName));
extern boolean    NotifyMasterDone _ANSI_ARGS_((char *hostName, int portNum,
                          int machineNumber,
                          int seconds,
                          int *frameStart, int *frameEnd));
extern void    StartIOServer _ANSI_ARGS_((int numInputFiles,
                       char *hostName, int portNum));
extern void    StartCombineServer _ANSI_ARGS_((int numInputFiles,
                           char *outputFileName,
                           char *hostName, int portNum));
extern void    StartDecodeServer _ANSI_ARGS_((int numInputFiles,
                           char *outputFileName,
                           char *hostName, int portNum));
extern void    WaitForOutputFile _ANSI_ARGS_((int number));
extern void    GetRemoteFrame _ANSI_ARGS_((MpegFrame *frame, int frameNumber));
extern void    SendRemoteFrame _ANSI_ARGS_((int frameNumber, BitBucket *bb));
extern void    NoteFrameDone _ANSI_ARGS_((int frameStart, int frameEnd));
extern void    SetIOConvert _ANSI_ARGS_((boolean separate));
void    SetRemoteShell _ANSI_ARGS_((char *shell));
extern void    NotifyDecodeServerReady _ANSI_ARGS_((int id));
extern void    WaitForDecodedFrame _ANSI_ARGS_((int id));
extern void    SendDecodedFrame _ANSI_ARGS_((MpegFrame *frame));
extern void    GetRemoteDecodedRefFrame _ANSI_ARGS_((MpegFrame *frame,
                              int frameNumber));
extern void    SetParallelPerfect _ANSI_ARGS_((boolean val));


/*==================*
 * GLOBAL VARIABLES *
 *==================*/

extern int parallelTestFrames;
extern int parallelTimeChunks;

extern char *IOhostName;
extern int ioPortNumber;
extern int combinePortNumber;
extern int decodePortNumber;

extern boolean    ioServer;
extern boolean    niceProcesses;
extern boolean    forceIalign;
extern int    machineNumber;
extern boolean remoteIO;
extern boolean    separateConversion;
