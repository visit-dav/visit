/*===========================================================================*
 * bitio.c                                     *
 *                                         *
 *    Procedures concerned with the bit-wise I/O                 *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    Bitio_New                                 *
 *    Bitio_Free                                 *
 *    Bitio_Write                                 *
 *    Bitio_Flush                                 *
 *    Bitio_WriteToSocket                             *
 *    Bitio_BytePad                                 *
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
 *  $Header: /n/picasso/project/mm/mpeg/mpeg_dist/mpeg_encode/RCS/bitio.c,v 1.13 1995/06/21 18:36:06 smoot Exp $
 *  $Log: bitio.c,v $
 * Revision 1.13  1995/06/21  18:36:06  smoot
 * added a flush when done with file
 *
 * Revision 1.12  1995/01/19  23:07:15  eyhung
 * Changed copyrights
 *
 * Revision 1.11  1994/11/12  02:11:43  keving
 * nothing
 *
 * Revision 1.10  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.10  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.9  1993/07/22  22:23:43  keving
 * nothing
 *
 * Revision 1.8  1993/06/30  20:06:09  keving
 * nothing
 *
 * Revision 1.7  1993/06/03  21:08:08  keving
 * nothing
 *
 * Revision 1.6  1993/02/17  23:21:41  dwallach
 * checkin prior to keving's joining the project
 *
 * Revision 1.5  1993/01/18  10:20:02  dwallach
 * *** empty log message ***
 *
 * Revision 1.4  1993/01/18  10:17:29  dwallach
 * RCS headers installed, code indented uniformly
 *
 * Revision 1.4  1993/01/18  10:17:29  dwallach
 * RCS headers installed, code indented uniformly
 *
 */

/*==============*
 * HEADER FILES *
 *==============*/

#include <assert.h>
#include "all.h"
#include "byteorder.h"
#include "bitio.h"
#include "mtypes.h"


/*===============================*
 * INTERNAL PROCEDURE prototypes *
 *===============================*/
 
static void Dump _ANSI_ARGS_((BitBucket *bbPtr));


/*==================*
 * STATIC VARIABLES *
 *==================*/

static uint32_mpeg_t lower_mask[33] = {
    0,
    0x1, 0x3, 0x7, 0xf,
    0x1f, 0x3f, 0x7f, 0xff,
    0x1ff, 0x3ff, 0x7ff, 0xfff,
    0x1fff, 0x3fff, 0x7fff, 0xffff,
    0x1ffff, 0x3ffff, 0x7ffff, 0xfffff,
    0x1fffff, 0x3fffff, 0x7fffff, 0xffffff,
    0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
    0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
};


extern time_t IOtime;


/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/


/*===========================================================================*
 *
 * Bitio_New
 *
 *    Create a new bit bucket; filePtr is a pointer to the open file the
 *    bits should ultimately be written to.
 *
 * RETURNS:    pointer to the resulting bit bucket
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
BitBucket *
Bitio_New(filePtr)
    FILE *filePtr;
{
    BitBucket *bbPtr;

    bbPtr = (BitBucket *) malloc(sizeof(BitBucket));
    ERRCHK(bbPtr, "malloc");

    bbPtr->firstPtr = bbPtr->lastPtr = (struct bitBucket *) malloc(sizeof(struct bitBucket));
    ERRCHK(bbPtr->firstPtr, "malloc");

    bbPtr->totalbits = 0;
    bbPtr->cumulativeBits = 0;
    bbPtr->bitsWritten = 0;
    bbPtr->filePtr = filePtr;

    bbPtr->firstPtr->nextPtr = NULL;
    bbPtr->firstPtr->bitsleft = MAXBITS_PER_BUCKET;
    bbPtr->firstPtr->bitsleftcur = 32;
    bbPtr->firstPtr->currword = 0;
    memset((char *)bbPtr->firstPtr->bits, 0, sizeof(uint32_mpeg_t) * WORDS_PER_BUCKET);

    return bbPtr;
}


/*===========================================================================*
 *
 * Bitio_Free
 *
 *    Frees the memory associated with the given bit bucket
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Bitio_Free(bbPtr)
    BitBucket *bbPtr;
{
    struct bitBucket *tmpPtr, *nextPtr;

    for (tmpPtr = bbPtr->firstPtr; tmpPtr != NULL; tmpPtr = nextPtr) {
    nextPtr = tmpPtr->nextPtr;
    free(tmpPtr);
    }
    free(bbPtr);
}


/*===========================================================================*
 *
 * Bitio_Write
 *
 *    Writes 'nbits' bits from 'bits' into the given bit bucket
 *    'nbits' must be between 0 and 32
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    if the number of bits in the bit bucket surpasses
 *            MAX_BITS, then that many bits are flushed to the
 *            appropriate output file
 *
 *===========================================================================*/
void
Bitio_Write(bbPtr, bits, nbits)
    BitBucket *bbPtr;
    uint32_mpeg_t bits;
    int nbits;
{
    register struct bitBucket *lastPtr, *newPtr;
    register int delta;

    assert(nbits <= 32 && nbits >= 0);

    /*
     * Clear top bits if not part of data, necessary due to down and
     * dirty calls of Bitio_Write with unecessary top bits set.
     */

    bits = bits & lower_mask[nbits];

    bbPtr->totalbits += nbits;
    bbPtr->cumulativeBits += nbits;
    lastPtr = bbPtr->lastPtr;

    delta = nbits - lastPtr->bitsleft;
    if (delta >= 0) {
    /*
         * there's not enough room in the current bucket, so we're
         * going to have to allocate another bucket
         */
    newPtr = lastPtr->nextPtr = (struct bitBucket *) malloc(sizeof(struct bitBucket));
    ERRCHK(newPtr, "malloc");
    newPtr->nextPtr = NULL;
    newPtr->bitsleft = MAXBITS_PER_BUCKET;
    newPtr->bitsleftcur = 32;
    newPtr->currword = 0;
    memset((char *)newPtr->bits, 0, sizeof(uint32_mpeg_t) * WORDS_PER_BUCKET);
    bbPtr->lastPtr = newPtr;

    assert(lastPtr->currword == WORDS_PER_BUCKET - 1);
    lastPtr->bits[WORDS_PER_BUCKET - 1] |= (bits >> delta);
    lastPtr->bitsleft = 0;
    lastPtr->bitsleftcur = 0;
    /* lastPtr->currword++; */

    if (!delta) {
        if ( bbPtr->totalbits > MAX_BITS ) {
        Dump(bbPtr);
        }
    }

    assert(delta <= 32);
    newPtr->bits[0] = (bits & lower_mask[delta]) << (32 - delta);
    newPtr->bitsleft -= delta;
    newPtr->bitsleftcur -= delta;
    } else {
    /*
         * the current bucket will be sufficient
     */
    delta = nbits - lastPtr->bitsleftcur;
    lastPtr->bitsleftcur -= nbits;
    lastPtr->bitsleft -= nbits;

    if (delta >= 0)
    {
        /*
         * these bits will span more than one word
         */
        lastPtr->bits[lastPtr->currword] |= (bits >> delta);
        lastPtr->currword++;
        lastPtr->bits[lastPtr->currword] = (bits & lower_mask[delta]) << (32 - delta);
        lastPtr->bitsleftcur = 32 - delta;
    } else {
        /*
         * these bits will fit, whole
         */
        lastPtr->bits[lastPtr->currword] |= (bits << (-delta));
    }
    }

    if ( bbPtr->totalbits > MAX_BITS )    /* flush bits */
    Dump(bbPtr);
}


/*===========================================================================*
 *
 * Bitio_Flush
 *
 *    Flushes all of the remaining bits in the given bit bucket to the
 *    appropriate output file.  It will generate up to the nearest 8-bit
 *    unit of bits, which means that up to 7 extra 0 bits will be appended
 *    to the end of the file.
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    frees the bit bucket
 *
 *===========================================================================*/
void
Bitio_Flush(bbPtr)
    BitBucket *bbPtr;
{
    struct bitBucket *ptr, *tempPtr;
    uint32_mpeg_t buffer[WORDS_PER_BUCKET];
    uint32_mpeg_t  lastWord;
    int i, nitems;
    int        bitsWritten = 0;
    int        bitsLeft;
    int        numWords;
    uint8_mpeg_t   charBuf[4];
    boolean    flushHere = FALSE;
    time_t  tempTimeStart, tempTimeEnd;

    time(&tempTimeStart);

    bitsLeft = bbPtr->totalbits;

    for (ptr = bbPtr->firstPtr; ptr; ptr = ptr->nextPtr) {
    if (ptr->bitsleftcur == 32 && ptr->currword == 0) {
        continue;        /* empty */
    }

    if ( bitsLeft >= 32 ) {
        if ( ((ptr->currword + 1) * 32) > bitsLeft ) {
        numWords = ptr->currword;
        flushHere = TRUE;
        } else {
        numWords = ptr->currword+1;
        }

        for (i = 0; i < numWords; i++) {
        buffer[i] = htonl(ptr->bits[i]);
        }

        nitems = fwrite(buffer, sizeof(uint32_mpeg_t), numWords, bbPtr->filePtr);
        if (nitems != numWords) {
        fprintf(stderr, "Whoa!  Trouble writing %d bytes (got %d items)!  Game over, dude!\n",
            (int)(numWords), nitems);
        exit(1);
        }

        bitsWritten += (numWords * 32);
        bitsLeft -= (numWords * 32);
    } else {
        flushHere = TRUE;
    }

    if ( (bitsLeft < 32) && flushHere ) {
        lastWord = ptr->bits[ptr->currword];

        /* output the lastPtr word in big-endian order (network) */

        /* now write out lastPtr bits */
        while ( bitsLeft > 0 ) {
        charBuf[0] = (lastWord >> 24);
        charBuf[0] &= lower_mask[8];
        fwrite(charBuf, 1, sizeof(uint8_mpeg_t), bbPtr->filePtr);
        lastWord = (lastWord << 8);
        bitsLeft -= 8;
        bitsWritten += 8;
        }
    }
    }
    fflush(bbPtr->filePtr);
    while ( bbPtr->firstPtr != ptr ) {
    tempPtr = bbPtr->firstPtr;
    bbPtr->firstPtr = tempPtr->nextPtr;
    free(tempPtr);
    }

    free(bbPtr);

    time(&tempTimeEnd);
    IOtime += (tempTimeEnd-tempTimeStart);
}


/*===========================================================================*
 *
 * Bitio_WriteToSocket
 *
 *    Writes all of the remaining bits in the given bit bucket to the
 *    given socket.  May pad the end of the socket stream with extra 0
 *    bits as does Bitio_Flush.
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    frees the bit bucket
 *
 *===========================================================================*/
void
Bitio_WriteToSocket(bbPtr, socket)
    BitBucket *bbPtr;
    int socket;
{
    struct bitBucket *ptr, *tempPtr;
    uint32_mpeg_t buffer[WORDS_PER_BUCKET];
    uint32_mpeg_t  lastWord;
    int i, nitems;
    int        bitsWritten = 0;
    int        bitsLeft;
    int        numWords;
    uint8_mpeg_t   charBuf[4];
    boolean    flushHere = FALSE;

    bitsLeft = bbPtr->totalbits;

    for (ptr = bbPtr->firstPtr; ptr; ptr = ptr->nextPtr) {
    if (ptr->bitsleftcur == 32 && ptr->currword == 0) {
        continue;        /* empty */
    }

    if ( bitsLeft >= 32 ) {
        if ( ((ptr->currword + 1) * 32) > bitsLeft ) {
        numWords = ptr->currword;
        flushHere = TRUE;
        } else {
        numWords = ptr->currword+1;
        }

        for (i = 0; i < numWords; i++) {
        buffer[i] = htonl(ptr->bits[i]);
        }

        nitems = write(socket, buffer, numWords * sizeof(uint32_mpeg_t));
        if (nitems != numWords*sizeof(uint32_mpeg_t)) {
        fprintf(stderr, "Whoa!  Trouble writing %d bytes (got %d bytes)!  Game over, dude!\n",
            (int)(numWords*sizeof(uint32_mpeg_t)), nitems);
        exit(1);
        }

        bitsWritten += (numWords * 32);
        bitsLeft -= (numWords * 32);
    } else {
        flushHere = TRUE;
    }

    if ( (bitsLeft < 32) && flushHere ) {
        lastWord = ptr->bits[ptr->currword];

        /* output the lastPtr word in big-endian order (network) */

        /* now write out lastPtr bits */
        while ( bitsLeft > 0 ) {
        charBuf[0] = (lastWord >> 24);
        charBuf[0] &= lower_mask[8];
        if ( write(socket, charBuf, 1) != 1 ) {
            fprintf(stderr, "ERROR:  write of lastPtr bits\n");
            exit(1);
        }
        lastWord = (lastWord << 8);
        bitsLeft -= 8;
        bitsWritten += 8;
        }
    }
    }

    while ( bbPtr->firstPtr != ptr ) {
    tempPtr = bbPtr->firstPtr;
    bbPtr->firstPtr = tempPtr->nextPtr;
    free(tempPtr);
    }

    free(bbPtr);
}


/*===========================================================================*
 *
 * Bitio_BytePad
 *
 *    Pads the end of the bit bucket to the nearest byte with 0 bits
 *
 * RETURNS:    nothing
 *
 *===========================================================================*/
void
Bitio_BytePad(bbPtr)
    BitBucket *bbPtr;
{
    struct bitBucket *lastPtrPtr = bbPtr->lastPtr;

    if (lastPtrPtr->bitsleftcur % 8) {
    Bitio_Write(bbPtr, 0, lastPtrPtr->bitsleftcur % 8);
    }
}


/*=====================*
 * INTERNAL PROCEDURES *
 *=====================*/


/*===========================================================================*
 *
 * Dump
 *
 *    Writes out the first MAX_BITS bits of the bit bucket to the
 *    appropriate output file
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:  none
 *
 *===========================================================================*/
static void
Dump(bbPtr)
    BitBucket *bbPtr;
{
    struct bitBucket *ptr, *tempPtr;
    uint32_mpeg_t buffer[WORDS_PER_BUCKET];
    int i, nitems;
    int        bitsWritten = 0;
    time_t  tempTimeStart, tempTimeEnd;

    time(&tempTimeStart);

    for (ptr = bbPtr->firstPtr; ptr && (bitsWritten < MAX_BITS);
     ptr = ptr->nextPtr) {
    if (ptr->bitsleftcur == 32 && ptr->currword == 0) {
        continue;        /* empty */
    }

    for (i = 0; i <= ptr->currword; i++) {
        buffer[i] = htonl(ptr->bits[i]);
    }

    nitems = fwrite((uint8_mpeg_t *)buffer, sizeof(uint32_mpeg_t), (ptr->currword + 1), bbPtr->filePtr);
    if (nitems != (ptr->currword+1)) {
        fprintf(stderr, "Whoa!  Trouble writing %d bytes (got %d items)!  Game over, dude!\n",
            (int)((ptr->currword+1)), nitems);
        assert(0);
        exit(1);
    }

    bitsWritten += ((ptr->currword + 1) * 32);
    }

    while ( bbPtr->firstPtr != ptr ) {
    tempPtr = bbPtr->firstPtr;
    bbPtr->firstPtr = tempPtr->nextPtr;
    free(tempPtr);
    }

    bbPtr->totalbits -= bitsWritten;
    bbPtr->bitsWritten += bitsWritten;

    time(&tempTimeEnd);
    IOtime += (tempTimeEnd-tempTimeStart);
}


