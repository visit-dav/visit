/*===========================================================================*
 * parallel.c                                         *
 *                                         *
 *    Procedures to make encoder run in parallel                 *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    StartIOServer                                 *
 *    StartCombineServer                             *
 *    StartDecodeServer                             *
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
 *  $Header: /n/picasso/project/mpeg/mpeg_dist/mpeg_encode/RCS/parallel.c,v 1.9 1995/08/16 18:22:08 smoot Exp $
 *  $Log: parallel.c,v $
 *  Revision 1.9  1995/08/16 18:22:08  smoot
 *  indents
 *
 *  Revision 1.8  1995/08/14 22:30:20  smoot
 *  added safe_fork to allow us to kill kids when we die.
 *
 *  Revision 1.7  1995/08/07 21:46:14  smoot
 *  spawns the same encoder as it is for combine, etc.
 *  uses new pattern tables to determine frame types
 *
 *  Revision 1.6  1995/06/21 18:32:14  smoot
 *  Defined SOMAXCONN when not (LINUX)
 *  added binary r/w (DOs    S!)
 *  ANSIified bcopy call
 *
 * Revision 1.5  1995/01/19  23:09:00  eyhung
 * Changed copyrights
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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "all.h"
#include "param.h"
#include "mpeg.h"
#include "prototypes.h"
#include "parallel.h"
#include "readframe.h"
#include "fsize.h"
#include "combine.h"
#include "frames.h"


#define MAX_IO_SERVERS    10
#ifndef SOMAXCONN
#define SOMAXCONN 5
#endif

/*==================*
 * CONSTANTS        *
 *==================*/

#define    TERMINATE_PID_SIGNAL    SIGTERM     /* signal used to terminate forked childs */
#ifndef MAXARGS
#define    MAXARGS        1024   /* Max Number of arguments in safe_fork command */
#endif

/*==================*
 * STATIC VARIABLES *
 *==================*/

static int32_mpeg_t   diffTime;
static char    rsh[256];
static struct hostent *hostEntry = NULL;
static boolean    *frameDone;
static int    outputServerSocket;
static int    decodeServerSocket;
static boolean    parallelPerfect = FALSE;
static    int    current_max_forked_pid=0;


/*==================*
 * GLOBAL VARIABLES *
 *==================*/

extern int yuvHeight, yuvWidth;
extern    time_t  timeStart, timeEnd;
extern char    statFileName[256];
extern FILE *statFile;
extern boolean  debugMachines;
extern boolean debugSockets;
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
extern char encoder_name[];
int     ClientPid[MAX_MACHINES+4];

/*===============================*
 * INTERNAL PROCEDURE prototypes *
 *===============================*/

static void    TransmitPortNum _ANSI_ARGS_((char *hostName, int portNum,
                           int ioPortNum));
static void    EndIOServer _ANSI_ARGS_((void));
static void SafeRead _ANSI_ARGS_((int fd, char *buf, int nbyte));
static void SafeWrite _ANSI_ARGS_((int fd, char *buf, int nbyte));
static int  CreateListeningSocket _ANSI_ARGS_((int *portNumber));
static int  ConnectToSocket _ANSI_ARGS_((char *machineName, int portNum,
                     struct hostent **hostEnt));
static int safe_fork _ANSI_ARGS_((char *command));
void cleanup_fork _ANSI_ARGS_ ((int dummy));


/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/

            /*=================*
             * IO SERVER STUFF *
             *=================*/


/*===========================================================================*
 *
 * SetIOConvert
 *
 *    sets the IO conversion to be separate or not.  If separate, then
 *    some post-processing is done at slave end
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
SetIOConvert(separate)
    boolean separate;
{
    separateConversion = separate;
}


/*===========================================================================*
 *
 * SetParallelPerfect
 *
 *    If this is called, then frames will be divided up completely, and
 *    evenly (modulo rounding) between all the processors
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    Sets parallelPerfect ....
 *
 *===========================================================================*/
void
SetParallelPerfect(val)
boolean val;
{
    parallelPerfect = val;
}


/*===========================================================================*
 *
 * SetRemoteShell
 *
 *    sets the remote shell program (usually rsh, but different on some
 *    machines)
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
SetRemoteShell(shell)
    char *shell;
{
    strcpy(rsh, shell);
}


/*===========================================================================*
 *
 * StartIOServer
 *
 *    start-up the IOServer with this process
 *    handles slave requests for frames, and exits when master tells it to
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
  StartIOServer(numInputFiles, parallelHostName, portNum)
int numInputFiles;
char *parallelHostName;
int portNum;
{
  int        ioPortNum;
  int        serverSocket;
  int        otherSock, otherSize;
  struct sockaddr otherSocket;
  int32_mpeg_t   buffer[8];
  boolean    done = FALSE;
  int        frameNumber;
  MpegFrame *frame;
  register int y;
  int        numBytes;
  unsigned char   *bigBuffer;
  unsigned char   smallBuffer[1000];
  int        bigBufferSize;
  FILE    *filePtr;
  uint32_mpeg_t  data;
  char    inputFileName[1024];
  char    fileName[1024];

  bigBufferSize = 0;
  bigBuffer = NULL;

  /* once we get IO port num, should transmit it to parallel server */

  serverSocket = CreateListeningSocket(&ioPortNum);

  if ( debugSockets ) {
    fprintf(stdout, "====I/O USING PORT %d\n", ioPortNum);
  }

  TransmitPortNum(parallelHostName, portNum, ioPortNum);

  otherSize = sizeof(otherSocket);

  if ( separateConversion ) {
    SetFileType(ioConversion);    /* for reading */
  } else {
    SetFileType(inputConversion);
  }

  /* now, wait until get done signal */
  while ( ! done ) {
    otherSock = accept(serverSocket, &otherSocket, &otherSize);
    if ( otherSock == -1 ) {
      fprintf(stderr, "ERROR:  I/O SERVER accept returned error %d\n", errno);
      exit(1);
    }

    SafeRead(otherSock, (char *)buffer, 4);
    frameNumber = ntohl(buffer[0]);

    if ( frameNumber == -1 ) {
      done = TRUE;
    } else if ( frameNumber == -2 ) {
      /* decoded frame to be output to disk */
      SafeRead(otherSock, (char *)buffer, 4);
      frameNumber = ntohl(buffer[0]);        

      if ( debugSockets ) {
    fprintf(stdout, "INPUT SERVER:  GETTING DECODED FRAME %d\n", frameNumber);
    fflush(stdout);
      }

      /* should read frame from socket, then write to disk */
      frame = Frame_New(frameNumber, 'i');

      Frame_AllocDecoded(frame, TRUE);

      for ( y = 0; y < Fsize_y; y++ ) {
    SafeRead(otherSock, (char *)frame->decoded_y[y], Fsize_x);
      }

      for (y = 0; y < (Fsize_y >> 1); y++) { /* U */
    SafeRead(otherSock, (char *)frame->decoded_cb[y], (Fsize_x >> 1));
      }

      for (y = 0; y < (Fsize_y >> 1); y++) { /* V */
    SafeRead(otherSock, (char *)frame->decoded_cr[y], (Fsize_x >> 1));
      }

      /* now output to disk */
      WriteDecodedFrame(frame);

      Frame_Free(frame);
    } else if ( frameNumber == -3 ) {
      /* request for decoded frame from disk */
      SafeRead(otherSock, (char *)buffer, 4);
      frameNumber = ntohl(buffer[0]);        

      if ( debugSockets ) {
    fprintf(stdout, "INPUT SERVER:  READING DECODED FRAME %d from DISK\n", frameNumber);
    fflush(stdout);
      }

      /* should read frame from disk, then write to socket */
      frame = Frame_New(frameNumber, 'i');

      Frame_AllocDecoded(frame, TRUE);

      ReadDecodedRefFrame(frame, frameNumber);

      /* now write to socket */
      for ( y = 0; y < Fsize_y; y++ ) {
    SafeWrite(otherSock, (char *)frame->decoded_y[y], Fsize_x);
      }

      for (y = 0; y < (Fsize_y >> 1); y++) { /* U */
    SafeWrite(otherSock, (char *)frame->decoded_cb[y], (Fsize_x >> 1));
      }

      for (y = 0; y < (Fsize_y >> 1); y++) { /* V */
    SafeWrite(otherSock, (char *)frame->decoded_cr[y], (Fsize_x >> 1));
      }

      Frame_Free(frame);
    } else if ( frameNumber == -4 ) {
      /* routing output frame from socket to disk */
      SafeRead(otherSock, (char *)buffer, 8);
      frameNumber = buffer[0];
      frameNumber = ntohl(frameNumber);

      /* read in number of bytes */
      numBytes = buffer[1];
      numBytes = ntohl(numBytes);

      /* make sure buffer is big enough for data */
      if ( numBytes > bigBufferSize ) {
    bigBufferSize = numBytes;
    if ( bigBuffer != NULL ) {
      free(bigBuffer);
    }

    bigBuffer = (unsigned char *) malloc(bigBufferSize*
                         sizeof(unsigned char));
      }

      /* now read in the bytes */
      SafeRead(otherSock, (char *) bigBuffer, numBytes);

      /* open file to output this stuff to */
      sprintf(fileName, "%s.frame.%d", outputFileName, frameNumber);
      if ( (filePtr = fopen(fileName, "wb")) == NULL ) {
    fprintf(stderr, "ERROR:  Could not open output file(3):  %s\n",
        fileName);
    exit(1);
      }

      /* now write the bytes here */
      fwrite(bigBuffer, sizeof(char), numBytes, filePtr);

      fclose(filePtr);

      if ( debugSockets ) {
    fprintf(stdout, "====I/O SERVER:  WROTE FRAME %d to disk\n",
        frameNumber);
    fflush(stdout);
      }
    } else {
      if ( debugSockets ) {
    fprintf(stdout, "I/O SERVER GETTING FRAME %d\n", frameNumber);
    fflush(stdout);
      }

      /* should read in frame, then write to socket */
      frame = Frame_New(frameNumber, 'i');

      if ( separateConversion ) {
    GetNthInputFileName(inputFileName, frameNumber);

    /* do conversion and send right to the socket */
    filePtr = ReadIOConvert(inputFileName);
    do {
      numBytes = fread(smallBuffer, 1, 1000, filePtr);

      if ( numBytes > 0 ) {
        data = numBytes;
        data = htonl(data);
        SafeWrite(otherSock, (char *)&data, 4);
        SafeWrite(otherSock, (char *)smallBuffer, numBytes);
      }
    }
    while ( numBytes == 1000 );

    if ( strcmp(ioConversion, "*") == 0 ) {
      fclose(filePtr);
    } else {
      pclose(filePtr);
    }
      } else {
    GetNthInputFileName(inputFileName, frameNumber);
    ReadFrame(frame, inputFileName, inputConversion, TRUE);

    /* should now transmit yuv values */
    for (y = 0; y < yuvHeight; y++) { /* Y */
      SafeWrite(otherSock, (char *)frame->orig_y[y], yuvWidth);
    }

    for (y = 0; y < (yuvHeight >> 1); y++) { /* U */
      SafeWrite(otherSock, (char *)frame->orig_cb[y], yuvWidth >> 1);
    }

    for (y = 0; y < (yuvHeight >> 1); y++) { /* V */
      SafeWrite(otherSock, (char *)frame->orig_cr[y], yuvWidth >> 1);
    }

    /* now, make sure we don't leave until other processor read everything */

    SafeRead(otherSock, (char *)buffer, 4);
    /* should = 0 */
      }

      if ( debugSockets ) {
    fprintf(stdout, "====I/O SERVER:  READ FRAME %d\n",
        frameNumber);
      }

      Frame_Free(frame);
    }

    close(otherSock);
  }

  close(serverSocket);

  if ( debugSockets ) {
    fprintf(stdout, "====I/O SERVER:  Shutting Down\n");
  }
}


/*===========================================================================*
 *
 * SendRemoteFrame
 *
 *    called by a slave to the I/O server; sends an encoded frame
 *    to the server to be sent to disk
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
SendRemoteFrame(frameNumber, bb)
    int frameNumber;
    BitBucket *bb;
{
    int    clientSocket;
    u_long  data;
    int        negativeFour = -4;
    time_t  tempTimeStart, tempTimeEnd;

    time(&tempTimeStart);

    clientSocket = ConnectToSocket(IOhostName, ioPortNumber, &hostEntry);

    data = htonl(negativeFour);
    SafeWrite(clientSocket, (char *)&data, 4);

    data = htonl(frameNumber);
    SafeWrite(clientSocket, (char *)&data, 4);

    if ( frameNumber != -1 ) {
    /* send number of bytes */
    data = (bb->totalbits+7)>>3;
    data = htonl(data);
    SafeWrite(clientSocket, (char *)&data, 4);

    /* now send the bytes themselves */
    Bitio_WriteToSocket(bb, clientSocket);
    }

    close(clientSocket);

    time(&tempTimeEnd);
    IOtime += (tempTimeEnd-tempTimeStart);
}




/*===========================================================================*
 *
 * NoteFrameDone
 *
 *    called by slave to the Combine server; tells it these frames are
 *    done
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
NoteFrameDone(frameStart, frameEnd)
    int frameStart;
    int frameEnd;
{
    int    clientSocket;
    u_long  data;
    int        negativeTwo = -2;
    time_t  tempTimeStart, tempTimeEnd;

    time(&tempTimeStart);

    clientSocket = ConnectToSocket(IOhostName, combinePortNumber, &hostEntry);

    data = negativeTwo;
    data = htonl(negativeTwo);
    SafeWrite(clientSocket, (char *)&data, 4);

    data = htonl(frameStart);
    SafeWrite(clientSocket, (char *)&data, 4);

    data = htonl(frameEnd);
    SafeWrite(clientSocket, (char *)&data, 4);

    close(clientSocket);

    time(&tempTimeEnd);
    IOtime += (tempTimeEnd-tempTimeStart);
}


/*===========================================================================*
 *
 * GetRemoteFrame
 *
 *    called by a slave; gets a remote frame from the I/O server
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
  GetRemoteFrame(frame, frameNumber)
MpegFrame *frame;
int frameNumber;
{
  FILE    *filePtr;
  int    clientSocket;
  unsigned char   smallBuffer[1000];
  register int y;
  int        numBytes;
  u_long  data;
  char    fileName[256];

  Fsize_Note(frameNumber, yuvWidth, yuvHeight);

  if ( debugSockets ) {
    fprintf(stdout, "MACHINE %s REQUESTING connection for FRAME %d\n",
        getenv("HOST"), frameNumber);
    fflush(stdout);
  }

  clientSocket = ConnectToSocket(IOhostName, ioPortNumber, &hostEntry);

  data = frameNumber;
  data = htonl(data);
  SafeWrite(clientSocket, (char *)&data, 4);

  if ( frameNumber != -1 ) {
    if ( separateConversion ) {
      sprintf(fileName, "/tmp/foobar%d", machineNumber);
      filePtr = fopen(fileName, "wb");

      /* read in stuff, SafeWrite to file, perform local conversion */
      do {
    SafeRead(clientSocket, (char *)&numBytes, 4);
    numBytes = ntohl(numBytes);

    SafeRead(clientSocket, (char *)smallBuffer, numBytes);

    fwrite(smallBuffer, 1, numBytes, filePtr);
      } while ( numBytes == 1000 );
      fflush(filePtr);
      fclose(filePtr);

      /* now do slave conversion */
      ReadFrame(frame, fileName, slaveConversion, FALSE);
    } else {
      Frame_AllocYCC(frame);

      if ( debugSockets ) {
    fprintf(stdout, "MACHINE %s allocated YCC FRAME %d\n",
        getenv("HOST"), frameNumber);
    fflush(stdout);
      }

      /* should now read yuv values */
      for (y = 0; y < yuvHeight; y++) {    /* Y */
    SafeRead(clientSocket, (char *)frame->orig_y[y], yuvWidth);
      }

      for (y = 0; y < (yuvHeight >> 1); y++) { /* U */
    SafeRead(clientSocket, (char *)frame->orig_cb[y], yuvWidth>>1);
      }

      for (y = 0; y < (yuvHeight >> 1); y++) { /* V */
    SafeRead(clientSocket, (char *)frame->orig_cr[y], yuvWidth>>1);
      }
    }
  }

  data = 0;
  data = htonl(data);
  SafeWrite(clientSocket, (char *)&data, 4);

  close(clientSocket);

  if ( debugSockets ) {
    fprintf(stdout, "MACHINE %s READ COMPLETELY FRAME %d\n",
        getenv("HOST"), frameNumber);
    fflush(stdout);
  }
}


/*===========================================================================*
 *
 * StartCombineServer
 *
 *    start-up the CombineServer with this process
 *    handles combination of frames, and tells the
 *    master when it's done
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
  StartCombineServer(numInputFiles, outputFileName, parallelHostName, portNum)
int numInputFiles;
char *outputFileName;
char *parallelHostName;
int portNum;
{
  int        combinePortNum;
  FILE    *ofp;
  
  /* once we get Combine port num, should transmit it to parallel server */
  
  outputServerSocket = CreateListeningSocket(&combinePortNum);
  
  if ( debugSockets ) {
    fprintf(stdout, "====OUTPUT USING PORT %d\n", combinePortNum);
  }
  
  TransmitPortNum(parallelHostName, portNum, combinePortNum);
  
  frameDone = (boolean *) malloc(numInputFiles*sizeof(boolean));
  memset((char *)frameDone, 0, numInputFiles*sizeof(boolean));
  
  if ( (ofp = fopen(outputFileName, "wb")) == NULL ) {
    fprintf(stderr, "ERROR:  Could not open output file!!\n");
    fflush(stderr);
    exit(1);
  }
  FramesToMPEG(numInputFiles, outputFileName, ofp, TRUE);
  
  if ( debugSockets ) {
    fprintf(stdout, "====COMBINE SERVER:  Shutting Down\n");
    fflush(stdout);
  }
  
  /* tell Master server we are done */
  TransmitPortNum(parallelHostName, portNum, combinePortNum);
  
  close(outputServerSocket);
}


/*===========================================================================*
 *
 * WaitForOutputFile
 *
 *    keep handling output events until we get the specified frame
 *    number
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
  WaitForOutputFile(number)
int number;
{
  int        otherSock;
  static int otherSize = sizeof(struct sockaddr);
  struct sockaddr otherSocket;
  int        frameNumber;
  int32_mpeg_t   buffer[8];
  int frameStart, frameEnd;

  while ( ! frameDone[number] ) {
    otherSock = accept(outputServerSocket, &otherSocket, &otherSize);
    if ( otherSock == -1 ) {
      fprintf(stderr, "ERROR:  Combine SERVER accept returned error %d\n", errno);
      exit(1);
    }

    SafeRead(otherSock, (char *)buffer, 4);
    frameNumber = ntohl(buffer[0]);

    if ( frameNumber == -2 ) {
      /* this is notification from non-remote process that a frame is done */

      SafeRead(otherSock, (char *)buffer, 8);
      frameStart = buffer[0];
      frameStart = ntohl(frameStart);
      frameEnd = buffer[1];
      frameEnd = ntohl(frameEnd);

      for ( frameNumber = frameStart; frameNumber <= frameEnd;
       frameNumber++ ) {
    frameDone[frameNumber] = TRUE;
      }
    }

    close(otherSock);
  }

  if ( debugSockets ) {
    fprintf(stdout, "WAIT FOR FRAME %d over\n", number);
    fflush(stdout);
  }
}


/*=====================*
 * MASTER SERVER STUFF *
 *=====================*/


/*===========================================================================*
 *
 * StartMasterServer
 *
 *    start the master server with this process
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
  StartMasterServer(numInputFiles, paramFile, outputFileName)
int numInputFiles;
char *paramFile;
char *outputFileName;
{
  FILE    *filePtr;
  register int ind, ind2;
  int        framesPerMachine;
  char    command[1024];
  char    *hostName;
  int        portNum;
  int        serverSocket;
  boolean finished[MAX_MACHINES];
  int        numFinished;
  int        otherSock, otherSize;
  struct sockaddr otherSocket;
  int        seconds;
  int32_mpeg_t   buffer[8];
  int ioPortNum[MAX_IO_SERVERS];
  int        combinePortNum, decodePortNum;
  int        nextFrame;
  int        startFrames[MAX_MACHINES];
  int        numFrames[MAX_MACHINES];
  int        lastNumFrames[MAX_MACHINES];
  int        numSeconds[MAX_MACHINES];
  float   fps[MAX_MACHINES];
  int        numMachinesToEstimate;
  float   framesPerSecond;
  float   totalFPS, localFPS;
  int        framesDone;
  float   avgFPS;
  char    niceNess[256];
  int32_mpeg_t   startFrame, endFrame;
  int numInputPorts = 0;
  int    numRemote = SOMAXCONN;
  int totalRemote = 0;
  time_t  startUpBegin, startUpEnd;
  time_t  shutDownBegin, shutDownEnd;
  float   timeChunk;

  time(&startUpBegin);

  if ( niceProcesses ) {
    sprintf(niceNess, "nice");
  } else {
    niceNess[0] = '\0';
  }

  time(&timeStart);

  PrintStartStats(-1, 0);

  /* create a server socket */
  hostName = getenv("HOST");

  if ( hostName == NULL ) {
    fprintf(stderr, "ERROR:  Set HOST environment variable\n");
    exit(1);
  }

  hostEntry = gethostbyname(hostName);
  if ( hostEntry == NULL ) {
    fprintf(stderr, "ERROR:  Could not find host %s in database\n",
        hostName);
    exit(1);
  }

  hostName = hostEntry->h_name;

  serverSocket = CreateListeningSocket(&portNum);
  if ( debugSockets ) {
    fprintf(stdout, "---USING PORT %d\n", portNum);
  }

  /* START COMBINE SERVER */
  sprintf(command, "%s -max_machines %d -output_server %s %d %d %s",
      encoder_name, numMachines, hostName, portNum, numInputFiles, paramFile);
  safe_fork(command);

  /* should now listen for connection from Combine server */
  otherSize = sizeof(otherSocket);
  otherSock = accept(serverSocket, &otherSocket, &otherSize);
  if ( otherSock == -1 ) {
    fprintf(stderr, "ERROR:  MASTER SERVER accept returned error %d\n", errno);
    exit(1);
  }

  SafeRead(otherSock, (char *)(&combinePortNum), 4);
  combinePortNum = ntohl(combinePortNum);
  combinePortNumber = combinePortNum;
  close(otherSock);

  if ( debugSockets ) {
    fprintf(stdout, "---MASTER SERVER:  Combine port number = %d\n",
        combinePortNum);
  }

  /* START DECODE SERVER if necessary */
  if ( referenceFrame == DECODED_FRAME ) {
    sprintf(command, "%s -max_machines %d -decode_server %s %d %d %s",
        encoder_name, numMachines, hostName, portNum, numInputFiles, paramFile);
    safe_fork(command);

    /* should now listen for connection from Decode server */
    otherSize = sizeof(otherSocket);
    otherSock = accept(serverSocket, &otherSocket, &otherSize);
    if ( otherSock == -1 ) {
      fprintf(stderr, "ERROR:  MASTER SERVER accept returned error %d\n", errno);
      exit(1);
    }

    SafeRead(otherSock, (char *)(&decodePortNum), 4);
    decodePortNum = ntohl(decodePortNum);
    close(otherSock);

    if ( debugSockets ) {
      fprintf(stdout, "---MASTER SERVER:  Decode port number = %d\n",
          decodePortNum);
    }
  }

  /* we are doing whole thing (if not, see above) */

  framesPerMachine = numInputFiles/numMachines;

  numFinished = 0;

  /* count number of remote machines */
  for ( ind = 0; ind < numMachines; ind++ ) {
    fps[ind] = -1.0;        /* illegal value as flag */
    if ( remote[ind] ) {
      totalRemote++;
    }
  }

  /* DO INITIAL TIME TESTS */
  nextFrame = 0;
  for ( ind = 0; ind < numMachines; ind++ ) {
    if ( (totalRemote != 0) && (numRemote == SOMAXCONN) ) {
      /* Create an I/O server */
      sprintf(command, "%s -max_machines %d -io_server %s %d %s",
          encoder_name, numMachines, hostName, portNum, paramFile);
      safe_fork(command);

      /* should now listen for connection from I/O server */
      otherSize = sizeof(otherSocket);
      otherSock = accept(serverSocket, &otherSocket, &otherSize);
      if ( otherSock == -1 ) {
    fprintf(stderr, "ERROR:  MASTER SERVER accept returned error %d\n", errno);
    exit(1);
      }

      SafeRead(otherSock, (char *)(&ioPortNum[numInputPorts]), 4);
      ioPortNum[numInputPorts] = ntohl(ioPortNum[numInputPorts]);
      close(otherSock);

      if ( debugSockets ) {
    fprintf(stdout, "---MASTER SERVER:  I/O port number = %d\n",
        ioPortNum[numInputPorts]);
      }

      numInputPorts++;
      numRemote = 0;
    }

    finished[ind] = FALSE;
    numSeconds[ind] = 0;

    startFrame = nextFrame;
    if ( parallelPerfect ) {
      endFrame = startFrame+((numInputFiles-startFrame)/
                 (numMachines-ind))  -1;

      if ( forceIalign ) {
    while (FType_Type(endFrame) != 'i') {endFrame++;}
      }

      /* always give at least 1 frame */
      if ( endFrame < startFrame ) {
    endFrame = startFrame;
      }

      /* make sure not out of bounds */
      if ( endFrame >= numInputFiles ) {
    endFrame = numInputFiles-1;
      }
    } else if ( forceIalign ) {
      endFrame = startFrame+framePatternLen-1;
      while (FType_Type(endFrame) != 'i') {endFrame++;}
    } else {
      endFrame = startFrame+parallelTestFrames-1;
    }
        
    if ( remote[ind] ) {
      sprintf(command, "%s %s -l %s %s %s -child %s %d %d %d %d %d %d -frames %d %d %s",
          rsh,
          machineName[ind], userName[ind], niceNess,
          executable[ind],
          hostName, portNum, ioPortNum[numInputPorts-1],
          combinePortNum, decodePortNum, ind,
          remote[ind],
          startFrame, endFrame,
          remoteParamFile[ind]);
      numRemote++;
      totalRemote--;
    } else {
      sprintf(command, "%s %s -l %s %s %s -child %s %d %d %d %d %d %d -frames %d %d %s",
          rsh,
          machineName[ind], userName[ind], niceNess,
          executable[ind],
          hostName, portNum, ioPortNum[numInputPorts-1],
          combinePortNum, decodePortNum, ind,
          remote[ind],
          startFrame, endFrame,
          paramFile);
    }

    if ( debugMachines ) {
      fprintf(stdout, "---%s:  frames %d to %d\n",
          machineName[ind],
          startFrame, endFrame);
    }
    

    safe_fork(command);

    nextFrame = endFrame+1;
    startFrames[ind] = startFrame;
    numFrames[ind] = endFrame-startFrame+1;
    lastNumFrames[ind] = endFrame-startFrame+1;
  }

  framesDone = 0;

  time(&startUpEnd);

  /* now, wait for other processes to finish and boss them around */
  while ( numFinished != numMachines ) {
    otherSize = sizeof(otherSocket);
    otherSock = accept(serverSocket, &otherSocket, &otherSize);
    if ( otherSock == -1 ) {
      fprintf(stderr, "ERROR:  MASTER SERVER 2 accept returned error %d\n", errno);
      exit(1);
    }

    SafeRead(otherSock, (char *)buffer, 8);

    ind = ntohl(buffer[0]);
    seconds = ntohl(buffer[1]);

    NoteFrameDone(startFrames[ind],
          startFrames[ind]+lastNumFrames[ind]-1);

    numSeconds[ind] += seconds;
    fps[ind] = (float)numFrames[ind]/(float)numSeconds[ind];

    if ( seconds != 0 )
      framesPerSecond = (float)lastNumFrames[ind]/(float)seconds;
    else
      framesPerSecond = (float)lastNumFrames[ind]*2.0;

    framesDone += lastNumFrames[ind];

    if ( nextFrame >= numInputFiles ) {
      buffer[0] = htonl(-1);
      buffer[1] = htonl(0);
      SafeWrite(otherSock, (char *)buffer, 8);
      numFinished++;

      if ( debugMachines ) {
    fprintf(stdout, "---%s FINISHED job (%f fps) (%d/%d done):  DONE\n",
        machineName[ind], framesPerSecond, numFinished,
        numMachines);
      }
    } else {
      if (numSeconds[ind] != 0) {
    avgFPS = (float)numFrames[ind]/(float)numSeconds[ind];
      } else {
    avgFPS = 0.1;        /* arbitrary small value */
      }

      startFrame = nextFrame;

      if ( parallelTimeChunks == -1 ) {    /* TAPER STUFF */
    /* estimate time left */
    /* frames left = numInputFiles-nextFrame */
    totalFPS = 0.0;
    numMachinesToEstimate = 0;
    for ( ind2 = 0; ind2 < numMachines; ind2++ ) {
      if ( fps[ind2] < 0.0 ) {
        numMachinesToEstimate++;
      } else {
        totalFPS += fps[ind2];
      }
    }

    totalFPS = (float)numMachines*
      (totalFPS/(float)(numMachines-numMachinesToEstimate));

    timeChunk = (float)(numInputFiles-nextFrame)/totalFPS;

    fprintf(stdout, "ASSIGNING %s %.2f seconds of work\n",
        machineName[ind], timeChunk);
    fflush(stdout);
    endFrame = nextFrame +
      (int)((float)timeChunk*avgFPS) - 1;
      } else {
    endFrame = nextFrame +
      (int)((float)parallelTimeChunks*avgFPS) - 1;
      }

      if ( forceIalign ) {
    while (FType_Type(endFrame) != 'i') {endFrame++;}
      }

      if ( endFrame < startFrame ) { /* always give at least 1 frame */
    endFrame = startFrame;
      }
      if ( endFrame >= numInputFiles ) {
    endFrame = numInputFiles-1;
      }

      nextFrame = endFrame+1;

      startFrames[ind] = startFrame;
      numFrames[ind] += (endFrame-startFrame+1);
      lastNumFrames[ind] = (endFrame-startFrame+1);

      if ( debugMachines ) {
    fprintf(stdout, "---%s FINISHED job (%f fps) (%d/%d done):  next:  %d to %d\n",
        machineName[ind], framesPerSecond, numFinished,
        numMachines, startFrame, endFrame);
      }

      buffer[0] = htonl(startFrame);
      buffer[1] = htonl(endFrame);

      SafeWrite(otherSock, (char *)buffer, 8);
    }

    close(otherSock);

    if ( debugMachines ) {
      fprintf(stdout, "---FRAMES DONE:  %d\tFARMED OUT:  %d\tLEFT:  %d\n",
          framesDone, nextFrame-framesDone, numInputFiles-nextFrame);
    }
  }

  time(&shutDownBegin);

  /* end all input servers */
  IOhostName = hostName;
  for ( ind = 0; ind < numInputPorts; ind++ ) {
    ioPortNumber = ioPortNum[ind];
    EndIOServer();
  }

  /* now wait for CombineServer to tell us they're done */
  otherSize = sizeof(otherSocket);
  otherSock = accept(serverSocket, &otherSocket, &otherSize);
  if ( otherSock == -1 ) {
    fprintf(stderr, "ERROR:  MASTER SERVER accept returned error %d\n", errno);
    exit(1);
  }

  SafeRead(otherSock, (char *)buffer, 4);
  close(otherSock);
    
  close(serverSocket);

  time(&timeEnd);
  diffTime = (int32_mpeg_t)(timeEnd-timeStart);

  time(&shutDownEnd);

  for ( ind2 = 0; ind2 < 2; ind2++ ) {
    if ( ind2 == 0 ) {
      filePtr = stdout;
    } else if ( statFile != NULL ) {
      filePtr = statFile;
    } else {
      continue;
    }

    fprintf(filePtr, "\n\n");
    fprintf(filePtr, "PARALLEL SUMMARY\n");
    fprintf(filePtr, "----------------\n");
    fprintf(filePtr, "\n");
    fprintf(filePtr, "START UP TIME:  %d seconds\n",
        (int)startUpEnd-(int)startUpBegin);
    fprintf(filePtr, "SHUT DOWN TIME:  %d seconds\n",
        (int)shutDownEnd-(int)shutDownBegin);

    fprintf(filePtr, "%14s\tFrames\tSeconds\tFrames Per Second\tSelf Time\n",
        "MACHINE");
    fprintf(filePtr, "--------------\t------\t-------\t-----------------\t---------\n");
    totalFPS = 0.0;
    for ( ind = 0; ind < numMachines; ind++ ) {
      localFPS = (float)numFrames[ind]/(float)numSeconds[ind];
      fprintf(filePtr, "%14s\t%d\t%d\t%f\t\t%d\n",
          machineName[ind], numFrames[ind], numSeconds[ind],
          localFPS, (int)((float)numInputFiles/localFPS));
      totalFPS += localFPS;
    }

    fprintf(filePtr, "--------------\t------\t-------\t-----------------\t---------\n");

    fprintf(filePtr, "%14s\t\t%d\t%f\n", "OPTIMAL", 
        (int)((float)numInputFiles/totalFPS),
        totalFPS);
    fprintf(filePtr, "%14s\t\t%d\t%f\n", "ACTUAL", diffTime, 
        (float)numInputFiles/(float)diffTime);

    fprintf(filePtr, "\n\n");
  }

  if ( statFile != NULL ) {
    fclose(statFile);
  }
}


/*===========================================================================*
 *
 * NotifyMasterDone
 *
 *    called by a slave process; tells the master process it is done
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
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
  int    clientSocket;
  int32_mpeg_t   buffer[8];
  time_t  tempTimeStart, tempTimeEnd;

  time(&tempTimeStart);

  clientSocket = ConnectToSocket(hostName, portNum, &hostEntry);

  buffer[0] = htonl(machineNumber);
  buffer[1] = htonl(seconds);

  SafeWrite(clientSocket, (char *)buffer, 8);

  SafeRead(clientSocket, (char *)buffer, 8);
  *frameStart = ntohl(buffer[0]);
  *frameEnd = ntohl(buffer[1]);

  close(clientSocket);

  time(&tempTimeEnd);
  IOtime += (tempTimeEnd-tempTimeStart);

  return ((*frameStart) >= 0);
}


/*===========================================================================*
 *
 * StartDecodeServer
 *
 *    start-up the DecodeServer with this process
 *    handles transfer of decoded frames to/from processes, and exits
 *    when master tells it to
 *    this is necessary only if referenceFrame == DECODED_FRAME
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
  StartDecodeServer(numInputFiles, decodeFileName, parallelHostName, portNum)
int numInputFiles;
char *decodeFileName;
char *parallelHostName;
int portNum;
{
  int        otherSock, otherSize;
  struct sockaddr otherSocket;
  int        decodePortNum;
  int32_mpeg_t   buffer[8];
  int        frameReady;
  boolean *ready;
  int        *waitMachine;
  int        *waitPort;
  int        *waitList;
  int        slaveNumber;
  int        slavePort;
  int        waitPtr;
  struct hostent *nullHost = NULL;
  int        clientSocket;

  /* should keep list of port numbers to notify when frames become ready */

  ready = (boolean *) calloc(numInputFiles, sizeof(boolean));
  waitMachine = (int *) calloc(numInputFiles, sizeof(int));
  waitPort = (int *) malloc(numMachines*sizeof(int));
  waitList = (int *) calloc(numMachines, sizeof(int));

  /* once we get Decode port num, should transmit it to parallel server */

  decodeServerSocket = CreateListeningSocket(&decodePortNum);

  if ( debugSockets ) {
    fprintf(stdout, "====DECODE USING PORT %d\n", decodePortNum);
  }

  TransmitPortNum(parallelHostName, portNum, decodePortNum);

  frameDone = (boolean *) malloc(numInputFiles*sizeof(boolean));
  memset((char *)frameDone, 0, numInputFiles*sizeof(boolean));

  /* wait for ready signals and requests */
  while ( TRUE ) {
    otherSize = sizeof(otherSocket);
    otherSock = accept(decodeServerSocket, &otherSocket, &otherSize);
    if ( otherSock == -1 ) {
      fprintf(stderr, "ERROR:  DECODE SERVER accept returned error %d\n", errno);
      exit(1);
    }

    SafeRead(otherSock, (char *)buffer, 4);
    frameReady = buffer[0];
    frameReady = ntohl(frameReady);

    if ( frameReady == -2 ) {
      SafeRead(otherSock, (char *)buffer, 4);
      frameReady = buffer[0];
      frameReady = ntohl(frameReady);

      if ( debugSockets ) {
    fprintf(stdout, "====DECODE SERVER:  REQUEST FOR %d\n", frameReady);
    fflush(stdout);        
      }

      /* now respond if it's ready yet */
      buffer[0] = frameDone[frameReady];
      buffer[0] = htonl(buffer[0]);
      SafeWrite(otherSock, (char *)buffer, 4);

      if ( ! frameDone[frameReady] ) {
    /* read machine number, port number */
    SafeRead(otherSock, (char *)buffer, 8);
    slaveNumber = buffer[0];
    slaveNumber = ntohl(slaveNumber);
    slavePort = buffer[1];
    slavePort = ntohl(slavePort);

    if ( debugSockets ) {
      fprintf(stdout, "WAITING:  SLAVE %d, PORT %d\n",
          slaveNumber, slavePort);
    }

    waitPort[slaveNumber] = slavePort;
    if ( waitMachine[frameReady] == 0 ) {
      waitMachine[frameReady] = slaveNumber+1;
    } else {
      /* someone already waiting for this frame */
      /* follow list of waiters to the end */
      waitPtr = waitMachine[frameReady]-1;
      while ( waitList[waitPtr] != 0 ) {
        waitPtr = waitList[waitPtr]-1;
      }

      waitList[waitPtr] = slaveNumber+1;
      waitList[slaveNumber] = 0;
    }
      }
    } else {
      frameDone[frameReady] = TRUE;

      if ( debugSockets ) {
    fprintf(stdout, "====DECODE SERVER:  FRAME %d READY\n", frameReady);
    fflush(stdout);
      }

      if ( waitMachine[frameReady] ) {
    /* need to notify one or more machines it's ready */
    waitPtr = waitMachine[frameReady]-1;
    while ( waitPtr >= 0 ) {
      clientSocket = ConnectToSocket(machineName[waitPtr],
                     waitPort[waitPtr],
                     &nullHost);
      close(clientSocket);
      waitPtr = waitList[waitPtr]-1;
    }
      }
    }

    close(otherSock);
  }

  if ( debugSockets ) {
    fprintf(stdout, "====DECODE SERVER:  Shutting Down\n");
    fflush(stdout);
  }

  /* tell Master server we are done */
  TransmitPortNum(parallelHostName, portNum, decodePortNum);

  close(decodeServerSocket);
}


/*=====================*
 * INTERNAL PROCEDURES *
 *=====================*/


/*===========================================================================*
 *
 * TransmitPortNum
 *
 *    called by the I/O or Combine server; transmits the appropriate
 *    port number to the master
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
  TransmitPortNum(hostName, portNum, newPortNum)
char *hostName;
int portNum;
int newPortNum;
{
  int    clientSocket;
  u_long  data;

  clientSocket = ConnectToSocket(hostName, portNum, &hostEntry);

  data = htonl(newPortNum);
  SafeWrite(clientSocket, (char *) &data, 4);

  close(clientSocket);
}


/*===========================================================================*
 *
 * SafeRead
 *
 *    safely read from the given socket; the procedure keeps reading until
 *    it gets the number of bytes specified
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
  SafeRead(fd, buf, nbyte)
int fd;
char *buf;
int nbyte;
{
  int numRead;
  int result;

  numRead = 0;

  while ( numRead != nbyte ) {
    result = read(fd, &buf[numRead], nbyte-numRead);

    if ( result == -1 ) {
      fprintf(stderr, "ERROR:  read (of %d bytes (total %d) ) returned error %d\n",
          nbyte-numRead, nbyte, errno);
      exit(1);
    }
    numRead += result;
  }
}


/*===========================================================================*
 *
 * SafeWrite
 *
 *    safely write to the given socket; the procedure keeps writing until
 *    it sends the number of bytes specified
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
  SafeWrite(fd, buf, nbyte)
int fd;
char *buf;
int nbyte;
{
  int numWritten;
  int result;

  numWritten = 0;

  while ( numWritten != nbyte ) {
    result = write(fd, &buf[numWritten], nbyte-numWritten);

    if ( result == -1 ) {
      fprintf(stderr, "ERROR:  read (of %d bytes (total %d) ) returned error %d\n",
          nbyte-numWritten, nbyte, errno);
      exit(1);
    }
    numWritten += result;
  }
}


/*===========================================================================*
 *
 * EndIOServer
 *
 *    called by the master process -- tells the I/O server to commit
 *    suicide
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
  EndIOServer()
{
  /* send signal to IO server:  -1 as frame number */
  GetRemoteFrame(NULL, -1);
}


/*===========================================================================*
 *
 * NotifyDecodeServerReady
 *
 *    called by a slave to the Decode Server to tell it a decoded frame
 *    is ready and waiting
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
  NotifyDecodeServerReady(id)
int id;
{
  int    clientSocket;
  u_long  data;
  time_t  tempTimeStart, tempTimeEnd;

  time(&tempTimeStart);

  clientSocket = ConnectToSocket(IOhostName, decodePortNumber, &hostEntry);

  data = htonl(id);
  SafeWrite(clientSocket, (char *)&data, 4);

  close(clientSocket);

  time(&tempTimeEnd);
  IOtime += (tempTimeEnd-tempTimeStart);
}


/*===========================================================================*
 *
 * WaitForDecodedFrame
 *
 *    blah blah blah
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
  WaitForDecodedFrame(id)
int id;
{
  int    clientSocket;
  u_long  data;
  int        negativeTwo = -2;
  int     ready;

  /* wait for a decoded frame */
  if ( debugSockets ) {
    fprintf(stdout, "WAITING FOR DECODED FRAME %d\n", id);
  }

  clientSocket = ConnectToSocket(IOhostName, decodePortNumber, &hostEntry);

  /* first, tell DecodeServer we're waiting for this frame */
  data = negativeTwo;
  data = htonl(negativeTwo);
  SafeWrite(clientSocket, (char *)&data, 4);

  data = htonl(id);
  SafeWrite(clientSocket, (char *)&data, 4);

  SafeRead(clientSocket, (char *)&data, 4);
  ready = data;
  ready = ntohl(ready);

  if ( ! ready ) {
    int        waitSocket;
    int        waitPort;
    int        otherSock, otherSize;
    struct sockaddr otherSocket;

    /* it's not ready; set up a connection and wait for decode server */
    waitSocket = CreateListeningSocket(&waitPort);

    /* tell decode server where we are */
    data = machineNumber;
    data = ntohl(data);
    SafeWrite(clientSocket, (char *)&data, 4);

    data = waitPort;
    data = ntohl(data);
    SafeWrite(clientSocket, (char *)&data, 4);

    close(clientSocket);

    if ( debugSockets ) {
      fprintf(stdout, "SLAVE:  WAITING ON SOCKET %d\n", waitPort);
      fflush(stdout);
    }

    otherSize = sizeof(otherSocket);
    otherSock = accept(waitSocket, &otherSocket, &otherSize);
    if ( otherSock == -1 ) {
      fprintf(stderr, "ERROR:  I/O SERVER accept returned error %d\n", errno);
      exit(1);
    }

    /* should we verify this is decode server? */
    /* for now, we won't */

    close(otherSock);

    close(waitSocket);
  } else {
    close(clientSocket);
  }

  if ( debugSockets ) {
    fprintf(stdout, "YE-HA FRAME %d IS NOW READY\n", id);
  }
}


/*===========================================================================*
 *
 * CreateListeningSocket
 *
 *    create a socket, using the first unused port number we can find
 *
 * RETURNS:    the socket; portNumber is modified appropriately
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static int
  CreateListeningSocket(portNumber)
int *portNumber;
{
  int        resultSocket;
  u_short tempShort;
  int        result;
  struct sockaddr_in    nameEntry;

  resultSocket = socket(AF_INET, SOCK_STREAM, 0);
  if ( resultSocket == -1 ) {
    fprintf(stderr, "ERROR:  Call to socket() gave error %d\n", errno);
    exit(1);
  }

  memset((char *) &nameEntry, 0, sizeof(nameEntry));
  nameEntry.sin_family = AF_INET;

  /* find a port number that isn't used */
  (*portNumber) = 2048;
  do {
    (*portNumber)++;
    tempShort = (*portNumber);
    nameEntry.sin_port = htons(tempShort);
    result = bind(resultSocket, (struct sockaddr *) &nameEntry,
          sizeof(struct sockaddr));
  }
  while ( result == -1 );

  /* would really like to wait for 1+numMachines machines, but this is max
   * allowable, unfortunately
   */
  result = listen(resultSocket, SOMAXCONN);
  if ( result == -1 ) {
    fprintf(stderr, "ERROR:  call to listen() gave error %d\n", errno);
    exit(1);
  }

  return resultSocket;
}


/*===========================================================================*
 *
 * ConnectToSocket
 *
 *    creates a socket and connects it to the specified socket
 *    hostEnt either is the host entry, or is NULL and needs to be
 *    found by using machineName
 *
 * RETURNS:    the socket
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static int
  ConnectToSocket(machineName, portNum, hostEnt)
char *machineName;
int    portNum;
struct hostent **hostEnt;
{
  int    resultSocket;
  int        result;
  u_short        tempShort;
  struct sockaddr_in  nameEntry;

  if ( (*hostEnt) == NULL ) {
    (*hostEnt) = gethostbyname(machineName);
    if ( (*hostEnt) == NULL ) {
      fprintf(stderr, "ERROR:  Couldn't get host by name (%s)\n",
          machineName);
      exit(1);
    }
  }

  resultSocket = socket(AF_INET, SOCK_STREAM, 0);
  if ( resultSocket == -1 ) {
    fprintf(stderr, "ERROR:  socket returned error %d\n", errno);
    exit(1);
  }

  nameEntry.sin_family = AF_INET;
  memset((void *) nameEntry.sin_zero, 0, 8);
  memcpy((void *) &(nameEntry.sin_addr.s_addr),
     (void *) (*hostEnt)->h_addr_list[0],
     (size_t) (*hostEnt)->h_length);
  tempShort = portNum;
  nameEntry.sin_port = htons(tempShort);

  result = connect(resultSocket, (struct sockaddr *) &nameEntry,
           sizeof(struct sockaddr));
  if ( result == -1 ) {
    fprintf(stderr, "ERROR:  connect (ConnectToSocket, port %d) from machine %s returned error %d\n",
        portNum, getenv("HOST"), errno);
    exit(1);
  }

  return resultSocket;
}


/*===========================================================================*
 *
 * SendDecodedFrame
 *
 *  Send the frame to the decode server.
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
  SendDecodedFrame(frame)
MpegFrame *frame;
{
  int    clientSocket;
  register int y;
  int        negativeTwo = -2;
  uint32_mpeg_t  data;

  /* send to IOServer */
  clientSocket = ConnectToSocket(IOhostName, ioPortNumber, &hostEntry);

  data = negativeTwo;
  data = htonl(data);
  SafeWrite(clientSocket, (char *)&data, 4);

  data = frame->id;
  data = htonl(data);
  SafeWrite(clientSocket, (char *)&data, 4);

  for ( y = 0; y < Fsize_y; y++ ) {
    SafeWrite(clientSocket, (char *)frame->decoded_y[y], Fsize_x);
  }

  for (y = 0; y < (Fsize_y >> 1); y++) { /* U */
    SafeWrite(clientSocket, (char *)frame->decoded_cb[y], (Fsize_x >> 1));
  }

  for (y = 0; y < (Fsize_y >> 1); y++) { /* V */
    SafeWrite(clientSocket, (char *)frame->decoded_cr[y], (Fsize_x >> 1));
  }

  close(clientSocket);
}


/*===========================================================================*
 *
 * GetRemoteDecodedFrame
 *
 *  get the decoded frame from the decode server.
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:   
 *
 *===========================================================================*/
void
  GetRemoteDecodedRefFrame(frame, frameNumber)
MpegFrame *frame;
int frameNumber;
{
  int    clientSocket;
  register int y;
  int        negativeThree = -3;
  uint32_mpeg_t  data;

  /* send to IOServer */
  clientSocket = ConnectToSocket(IOhostName, ioPortNumber, &hostEntry);

  /* ask IOServer for decoded frame */
  data = negativeThree;
  data = htonl(data);
  SafeWrite(clientSocket, (char *)&data, 4);

  data = frame->id;
  data = htonl(data);
  SafeWrite(clientSocket, (char *)&data, 4);

  for ( y = 0; y < Fsize_y; y++ ) {
    SafeRead(clientSocket, (char *)frame->decoded_y[y], Fsize_x);
  }

  for (y = 0; y < (Fsize_y >> 1); y++) { /* U */
    SafeRead(clientSocket, (char *)frame->decoded_cb[y], (Fsize_x >> 1));
  }

  for (y = 0; y < (Fsize_y >> 1); y++) { /* V */
    SafeRead(clientSocket, (char *)frame->decoded_cr[y], (Fsize_x >> 1));
  }

  close(clientSocket);
    
}


/*********
  routines handling forks, execs, PIDs and signals
  save, system-style forks
  apian@ise.fhg.de
  *******/


/*===========================================================================*
 *
 * cleanup_fork
 *
 *  Kill all the children, to be used when we get killed
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:   kills other processes
 *
 *===========================================================================*/
void cleanup_fork( dummy )            /* try to kill all child processes */
     int dummy;
{
  register int i;
  for (i = 0;  i < current_max_forked_pid;  ++i ) {

#ifdef DEBUG_FORK
    fprintf(stderr, "cleanup_fork: killing PID %d\n", ClientPid[i]);
#endif

    if (kill(ClientPid[i], TERMINATE_PID_SIGNAL)) {
      fprintf(stderr, "cleanup_fork: killed PID=%d failed (errno %d)\n", 
          ClientPid[i], errno);
    }
  }
}

/*===========================================================================*
 *
 * safe_fork
 *
 *  fork a command
 *
 * RETURNS:     success/failure
 *
 * SIDE EFFECTS:   Fork the command, and save to PID so you can kil it later!
 *
 *===========================================================================*/
static int safe_fork(command)        /* fork child process and remember its PID */
     char *command;
{
  static int init=0;
  char *argis[MAXARGS];
  register int i=1;
  
  if (!(argis[0] = strtok(command, " \t"))) return(0); /* tokenize */
  while ((argis[i] = strtok(NULL, " \t")) && i < MAXARGS) ++i;
  argis[i] = NULL;
  
#ifdef DEBUG_FORK
  {register int i=0; 
   fprintf(stderr, "Command %s becomes:\n", command);
   while(argis[i]) {fprintf(stderr, "--%s--\n", argis[i]); ++i;} }
#endif
  
  if (!init) {            /* register clean-up routine */
    signal (SIGQUIT, cleanup_fork);
    signal (SIGTERM, cleanup_fork);
    signal (SIGINT , cleanup_fork);
    init=1;
  }
  
  if (-1 == (ClientPid[current_max_forked_pid] = fork()) )  {
    perror("safe_fork: fork failed ");
    return(-1);
  }
  if( !ClientPid[current_max_forked_pid]) { /* we are in child process */
    execvp(argis[0], argis );
    perror("safe_fork child: exec failed ");
    exit(1);
  }
#ifdef DEBUG_FORK
  fprintf(stderr, "parallel: forked PID=%d\n", ClientPid[current_max_forked_pid]);
#endif
  current_max_forked_pid++;
  return(0);
}
