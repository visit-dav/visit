/*===========================================================================*
 *  specifics.h                                                              *
 *                                         *
 *    reading the specifics file                         *
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
 *  $Header: /u/smoot/md/mpeg_encode/headers/RCS/specifics.h,v 1.4 1995/04/14 23:11:39 smoot Exp $
 *  $Log: specifics.h,v $
 * Revision 1.4  1995/04/14  23:11:39  smoot
 * made types smalled
 *
 * Revision 1.3  1995/01/19  23:55:23  eyhung
 * Changed copyrights
 *
 * Revision 1.2  1995/01/17  21:53:32  smoot
 * killed prototypes
 *
 * Revision 1.1  1994/11/15  00:40:35  smoot
 * Initial revision
 *
 *
 */


/*==============*
 * HEADER FILES *
 *==============*/

#include "ansi.h"


/*===========*
 * TYPES     *
 *===========*/

typedef struct bs_def {
  int num;
  boolean relative;
  char qscale;
  BlockMV *mv;  /* defined in mtypes.h */
  struct bs_def *next;
} Block_Specifics;

typedef struct detail_def {
  int num;
  char qscale;
  struct detail_def *next;
}  Slice_Specifics;

typedef struct fsl_def {
  int framenum; 
  int frametype;
  char qscale;
  Slice_Specifics *slc;
  Block_Specifics *bs;
  struct fsl_def *next;
} FrameSpecList;

/*===========*
 * CONSTANTS *
 *===========*/


/*==================*
 * GLOBAL VARIABLES *
 *==================*/

