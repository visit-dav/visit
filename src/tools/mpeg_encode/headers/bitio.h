/*===========================================================================*
 * bitio.h                                     *
 *                                         *
 *    bitwise input/output                             *
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
 *  $Header: /n/charlie-brown/project/mm/mpeg/mpeg_dist/mpeg_encode/headers/RCS/bitio.h,v 1.8 1995/01/19 23:54:37 eyhung Exp $
 *  $Log: bitio.h,v $
 * Revision 1.8  1995/01/19  23:54:37  eyhung
 * Changed copyrights
 *
 * Revision 1.7  1994/11/12  02:12:14  keving
 * nothing
 *
 * Revision 1.6  1993/07/22  22:24:23  keving
 * nothing
 *
 * Revision 1.5  1993/07/09  00:17:23  keving
 * nothing
 *
 * Revision 1.4  1993/06/03  21:08:53  keving
 * nothing
 *
 * Revision 1.3  1993/01/18  10:20:02  dwallach
 * *** empty log message ***
 *
 * Revision 1.2  1993/01/18  10:17:29  dwallach
 * RCS headers installed, code indented uniformly
 *
 * Revision 1.2  1993/01/18  10:17:29  dwallach
 * RCS headers installed, code indented uniformly
 *
 */


#ifndef BIT_IO_INCLUDED
#define BIT_IO_INCLUDED


/*==============*
 * HEADER FILES *
 *==============*/

#include "general.h"
#include "ansi.h"


/*===========*
 * CONSTANTS *
 *===========*/

#define WORDS_PER_BUCKET 128
#define MAXBITS_PER_BUCKET    (WORDS_PER_BUCKET * 32)
#define    MAX_BUCKETS    128
#define MAX_BITS    MAX_BUCKETS*MAXBITS_PER_BUCKET


/*=======================*
 * STRUCTURE DEFINITIONS *
 *=======================*/

typedef struct bitBucket {
    struct bitBucket *nextPtr;
    uint32_mpeg_t bits[WORDS_PER_BUCKET];
    int bitsleft, bitsleftcur, currword;
} ActualBucket;

typedef struct _BitBucket {
    int totalbits;
    int    cumulativeBits;
    int    bitsWritten;
    FILE    *filePtr;
    ActualBucket *firstPtr;
    ActualBucket *lastPtr;
} BitBucket;


/*========*
 * MACROS *
 *========*/

#define    SET_ITH_BIT(bits, i)    (bits |= (1 << (i)))
#define    GET_ITH_BIT(bits, i)    (bits & (1 << (i)))


/*===============================*
 * EXTERNAL PROCEDURE prototypes *
 *===============================*/

void        Bitio_Free _ANSI_ARGS_((BitBucket *bbPtr));
void        Bitio_Write _ANSI_ARGS_((BitBucket *bbPtr, uint32_mpeg_t bits, int nbits));
void        Bitio_BytePad _ANSI_ARGS_((BitBucket *bbPtr));
BitBucket  *Bitio_New _ANSI_ARGS_((FILE *filePtr));
void        Bitio_Flush _ANSI_ARGS_((BitBucket *bbPtr));
void        Bitio_WriteToSocket _ANSI_ARGS_((BitBucket *bbPtr, int socket));


#endif /* BIT_IO_INCLUDED */
