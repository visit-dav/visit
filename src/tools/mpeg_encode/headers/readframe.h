/*===========================================================================*
 * readframe.h                                     *
 *                                         *
 *    stuff dealing with reading frames                     *
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
 *  $Header: /n/charlie-brown/project/mm/mpeg/mpeg_dist/mpeg_encode/headers/RCS/readframe.h,v 1.6 1995/01/19 23:55:17 eyhung Exp $
 *  $Log: readframe.h,v $
 * Revision 1.6  1995/01/19  23:55:17  eyhung
 * Changed copyrights
 *
 * Revision 1.5  1995/01/14  00:05:54  smoot
 * *** empty log message ***
 *
 * Revision 1.4  1995/01/13  23:44:54  smoot
 * added B&W (Y files)
 *
 * Revision 1.3  1994/11/12  02:12:57  keving
 * nothing
 *
 * Revision 1.2  1993/07/22  22:24:23  keving
 * nothing
 *
 * Revision 1.1  1993/07/09  00:17:23  keving
 * nothing
 *
 */


/*===========*
 * CONSTANTS *
 *===========*/

#define    PPM_FILE_TYPE        0
#define YUV_FILE_TYPE        2
#define ANY_FILE_TYPE        3
#define BASE_FILE_TYPE        4
#define PNM_FILE_TYPE        5
#define SUB4_FILE_TYPE        6
#define JPEG_FILE_TYPE        7
#define JMOVIE_FILE_TYPE    8
#define Y_FILE_TYPE        9

/*===============================*
 * EXTERNAL PROCEDURE prototypes *
 *===============================*/

extern void    ReadFrame _ANSI_ARGS_((MpegFrame *frame, char *fileName,
                       char *conversion, boolean addPath));
extern void    SetFileType _ANSI_ARGS_((char *conversion));
extern void    SetFileFormat _ANSI_ARGS_((char *format));
extern FILE    *ReadIOConvert _ANSI_ARGS_((char *fileName));
extern void    SetResize _ANSI_ARGS_((boolean set));


extern int    baseFormat;
