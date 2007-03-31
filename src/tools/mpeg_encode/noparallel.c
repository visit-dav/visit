/*===========================================================================*
 * noparallel.c                                     *
 *                                         *
 *    would be procedures to make encoder to run in parallel -- except     *
 *    this machine doesn't have sockets, so we can only run sequentially   *
 *    so this file has dummy procedures which lets it compile             *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    StartIOServer                                 *
 *    StartCombineServer                             *
 *    SendRemoteFrame                                 *
 *    GetRemoteFrame                                 *
 *    StartMasterServer                             *
 *    NotifyMasterDone                             *
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
 *  $Header: /n/picasso/project/mm/mpeg/mpeg_dist/mpeg_encode/RCS/noparallel.c,v 1.6 1995/05/12 00:00:06 smoot Exp $
 *  $Log: noparallel.c,v $
 * Revision 1.6  1995/05/12  00:00:06  smoot
 * sco defined check
 *
 * Revision 1.5  1995/01/19  23:08:58  eyhung
 * Changed copyrights
 *
 * Revision 1.4  1994/11/14  22:33:31  smoot
 * added ifdef for Linux
 *
 * Revision 1.3  1994/11/12  02:11:55  keving
 * nothing
 *
 * Revision 1.2  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.2  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.1  1993/07/22  22:23:43  keving
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
#include "parallel.h"
#include "frame.h"
#ifdef LINUX
#include <time.h>
#endif
#if defined(sco)
#include <time.h>
#endif


/*==================*
 * GLOBAL VARIABLES *
 *==================*/

int parallelTestFrames = 10;
int parallelTimeChunks = 60;
char *IOhostName;
int ioPortNumber;
int combinePortNumber;
int decodePortNumber;
boolean    niceProcesses = FALSE;
boolean    forceIalign = FALSE;
int        machineNumber = -1;
boolean    remoteIO = FALSE;
boolean    separateConversion;
time_t    IOtime = 0;


/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/

            /*=================*
             * IO SERVER STUFF *
             *=================*/


void
SetIOConvert(separate)
    boolean separate;
{
    /* do nothing -- this may be called during non-parallel execution */
}


void
SetParallelPerfect()
{
    /* do nothing -- this may be called during non-parallel execution */
}


void
SetRemoteShell(shell)
    char *shell;
{
    /* do nothing -- this may be called during non-parallel execution */
}


/* StartIOServer
 */ 
void
StartIOServer(numInputFiles, parallelHostName, portNum)
    int numInputFiles;
    char *parallelHostName;
    int portNum;
{
    fprintf(stdout, "ERROR:  (StartIOServer) This machine can NOT run parallel version\n");
    exit(1);
}


void
StartCombineServer(numInputFiles, outputFileName, parallelHostName, portNum)
    int numInputFiles;
    char *outputFileName;
    char *parallelHostName;
    int portNum;
{
    fprintf(stdout, "ERROR:  (StartCombineServer) This machine can NOT run parallel version\n");
    exit(1);
}


void
NoteFrameDone(frameStart, frameEnd)
    int frameStart;
    int frameEnd;
{
    fprintf(stdout, "ERROR:  (NoteFrameDone) This machine can NOT run parallel version\n");
    exit(1);
}

/* SendRemoteFrame
 */
void
SendRemoteFrame(frameNumber, bb)
    int frameNumber;
    BitBucket *bb;
{
    fprintf(stdout, "ERROR:  (SendRemoteFrame) This machine can NOT run parallel version\n");
    exit(1);
}


/* GetRemoteFrame
 */
void
GetRemoteFrame(frame, frameNumber)
    MpegFrame *frame;
    int frameNumber;
{
    fprintf(stdout, "ERROR:  (GetRemoteFrame) This machine can NOT run parallel version\n");
    exit(1);
}


void
WaitForOutputFile(number)
    int number;
{
    fprintf(stdout, "ERROR:  (WaitForOutputFile) This machine can NOT run parallel version\n");
    exit(1);
}


            /*=======================*
             * PARALLEL SERVER STUFF *
             *=======================*/


/* StartMasterServer
 */
void
StartMasterServer(numInputFiles, paramFile, outputFileName)
    int numInputFiles;
    char *paramFile;
    char *outputFileName;
{
    fprintf(stdout, "ERROR:  (StartMasterServer) This machine can NOT run parallel version\n");
    exit(1);
}


/* NotifyMasterDone
 */
boolean
NotifyMasterDone(hostName, portNum, machineNumber, seconds, frameStart,
         frameEnd)
    char *hostName;
    int portNum;
    int machineNumber;
    int seconds;
    int *frameStart;
    int *frameEnd;
{
    fprintf(stdout, "ERROR:  (NotifyMasterDone) This machine can NOT run parallel version\n");
    exit(1);
}


void
StartDecodeServer(numInputFiles, decodeFileName, parallelHostName, portNum)
    int numInputFiles;
    char *decodeFileName;
    char *parallelHostName;
    int portNum;
{
    fprintf(stdout, "ERROR:  (StartDecodeServer) This machine can NOT run parallel version\n");
    exit(1);
}


void
NotifyDecodeServerReady(id)
    int id;
{
    fprintf(stdout, "ERROR:  (NotifyDecodeServerReady) This machine can NOT run parallel version\n");
    exit(1);
}


void
WaitForDecodedFrame(id)
    int id;
{
    fprintf(stdout, "ERROR:  (WaitForDecodedFrame) This machine can NOT run parallel version\n");
    exit(1);
}

void
SendDecodedFrame(frame)
    MpegFrame *frame;
{
    fprintf(stdout, "ERROR:  (SendDecodedFrame) This machine can NOT run parallel version\n");
    exit(1);
}

void
GetRemoteDecodedRefFrame(frame, frameNumber)
    MpegFrame *frame;
    int frameNumber;
{
    fprintf(stdout, "ERROR:  (GetRemoteDecodedRefFrame) This machine can NOT run parallel version\n");
    exit(1);
}
