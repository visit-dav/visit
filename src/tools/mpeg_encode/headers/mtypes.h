/*===========================================================================*
 * mtypes.h                                     *
 *                                         *
 *    MPEG data types                                 *
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
 *  $Header: /u/smoot/md/mpeg_encode/headers/RCS/mtypes.h,v 1.12 1995/04/14 23:12:11 smoot Exp $
 *  $Log: mtypes.h,v $
 * Revision 1.12  1995/04/14  23:12:11  smoot
 * added ChromBlock for future color MV searches
 *
 * Revision 1.11  1995/01/19  23:55:05  eyhung
 * Changed copyrights
 *
 * Revision 1.10  1994/11/14  22:48:57  smoot
 * added defines for Specifics operation
 *
 * Revision 1.9  1994/11/12  02:12:52  keving
 * nothing
 *
 * Revision 1.8  1993/07/22  22:24:23  keving
 * nothing
 *
 * Revision 1.7  1993/07/09  00:17:23  keving
 * nothing
 *
 * Revision 1.6  1993/06/03  21:08:53  keving
 * nothing
 *
 * Revision 1.5  1993/02/17  23:18:20  dwallach
 * checkin prior to keving's joining the project
 *
 * Revision 1.4  1993/01/18  10:20:02  dwallach
 * *** empty log message ***
 *
 * Revision 1.3  1993/01/18  10:17:29  dwallach
 * RCS headers installed, code indented uniformly
 *
 * Revision 1.3  1993/01/18  10:17:29  dwallach
 * RCS headers installed, code indented uniformly
 *
 */


#ifndef MTYPES_INCLUDED
#define MTYPES_INCLUDED


/*==============*
 * HEADER FILES *
 *==============*/

#include "general.h"
#include "dct.h"


/*===========*
 * CONSTANTS *
 *===========*/

#define TYPE_BOGUS    0   /* for the header of the circular list */
#define TYPE_VIRGIN    1

#define STATUS_EMPTY    0
#define STATUS_LOADED    1
#define STATUS_WRITTEN    2


/*==================*
 * TYPE DEFINITIONS *
 *==================*/

/*  
 *  your basic Block type
 */
typedef int16_mpeg_t Block[DCTSIZE][DCTSIZE];
typedef int16_mpeg_t FlatBlock[DCTSIZE_SQ];
typedef        int32_mpeg_t   LumBlock[2*DCTSIZE][2*DCTSIZE];
typedef        int32_mpeg_t   ChromBlock[DCTSIZE][DCTSIZE];

/*========*
 * MACROS *
 *========*/

#ifdef ABS
#undef ABS
#endif

#define ABS(x) (((x)<0)?-(x):(x))

#ifdef HEINOUS_DEBUG_MODE
#define DBG_PRINT(x) {printf x; fflush(stdout);}
#else
#define DBG_PRINT(x)
#endif

#define ERRCHK(bool, str) {if(!(bool)) {perror(str); exit(1);}}

/* For Specifics */
typedef struct detalmv_def {
  int typ,fx,fy,bx,by;
} BlockMV;
#define TYP_SKIP 0
#define TYP_FORW 1
#define TYP_BACK 2
#define TYP_BOTH 3


#endif /* MTYPES_INCLUDED */


