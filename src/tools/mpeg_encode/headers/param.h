/*===========================================================================*
 * param.h                                     *
 *                                         *
 *    reading the parameter file                         *
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
 *  $Header: /n/video/199/eyhung/encode/headers/RCS/param.h,v 1.8 1995/05/02 01:51:08 eyhung Exp $
 *  $Log: param.h,v $
 * Revision 1.8  1995/05/02  01:51:08  eyhung
 * added VidRateNum for determining frame rate
 *
 * Revision 1.7  1995/01/19  23:55:10  eyhung
 * Changed copyrights
 *
 * Revision 1.6  1995/01/17  04:47:26  eyhung
 * added coding-on-the-fly variable
 *
 * Revision 1.5  1995/01/16  06:06:58  eyhung
 * added yuvConversion global variable
 *
 * Revision 1.4  1994/12/08  20:13:28  smoot
 * Killed linux MAXPATHLEN warning
 *
 * Revision 1.3  1994/11/12  02:12:54  keving
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


/*===========*
 * CONSTANTS *
 *===========*/

#define MAX_MACHINES        256
#ifndef MAXPATHLEN
#define MAXPATHLEN  1024
#endif

#define    ENCODE_FRAMES    0
#define COMBINE_GOPS    1
#define COMBINE_FRAMES    2


/*===============================*
 * EXTERNAL PROCEDURE prototypes *
 *===============================*/

boolean    ReadParamFile _ANSI_ARGS_((char *fileName, int function));
void    GetNthInputFileName _ANSI_ARGS_((char *fileName, int n));
extern void    JM2JPEG _ANSI_ARGS_((void));


/*==================*
 * GLOBAL VARIABLES *
 *==================*/

extern int numInputFiles;
extern char    outputFileName[256];
extern int    whichGOP;
extern int numMachines;
extern char    machineName[MAX_MACHINES][256];
extern char    userName[MAX_MACHINES][256];
extern char    executable[MAX_MACHINES][1024];
extern char    remoteParamFile[MAX_MACHINES][1024];
extern boolean    remote[MAX_MACHINES];
extern boolean    childProcess;
extern char    currentPath[MAXPATHLEN];
extern char inputConversion[1024];
extern char yuvConversion[256];
extern int  yuvWidth, yuvHeight;
extern int  realWidth, realHeight;
extern char ioConversion[1024];
extern char slaveConversion[1024];
extern FILE *bitRateFile;
extern boolean showBitRatePerFrame;
extern boolean computeMVHist;
extern boolean stdinUsed;
extern const double VidRateNum[9];
