/*===========================================================================*
 * postdct.c                                     *
 *                                         *
 *    Procedures concerned with MPEG post-DCT processing:             *
 *        quantization and RLE Huffman encoding                 *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    Mpost_QuantZigBlock                             *
 *    Mpost_RLEHuffIBlock                             *
 *    Mpost_RLEHuffPBlock                             *
 *    Mpost_UnQuantZigBlock                             *
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
 *  $Header: /n/picasso/project/mm/mpeg/mpeg_dist/mpeg_encode/RCS/postdct.c,v 1.12 1995/06/21 18:26:39 smoot Exp $
 *  $Log: postdct.c,v $
 * Revision 1.12  1995/06/21  18:26:39  smoot
 * added length estimator for P-blocks
 *
 * Revision 1.11  1995/04/23  23:22:59  eyhung
 * nothing changed
 *
 * Revision 1.10  1995/04/14  23:10:46  smoot
 * Added overflow detection to MPOST_DCT so it will adjust Qscales (above)
 *
 * Revision 1.9  1995/02/15  23:15:32  smoot
 * killed useless asserts
 *
 * Revision 1.8  1995/02/01  21:48:41  smoot
 * assure out is set properly, short circuit 0 revquant
 *
 * Revision 1.7  1995/01/30  19:56:37  smoot
 * Killed a <0 shift
 *
 * Revision 1.6  1995/01/25  23:07:33  smoot
 * Better DBG_PRINTs, multiply/divide instead of shifts
 *
 * Revision 1.5  1995/01/19  23:09:10  eyhung
 * Changed copyrights
 *
 * Revision 1.4  1995/01/16  08:17:08  eyhung
 * added realQuiet
 *
 * Revision 1.3  1994/11/12  02:11:58  keving
 * nothing
 *
 * Revision 1.2  1994/03/15  00:27:11  keving
 * nothing
 *
 * Revision 1.2  1994/03/15  00:27:11  keving
 * nothing
 *
 * Revision 1.1  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.11  1993/07/22  22:23:43  keving
 * nothing
 *
 * Revision 1.10  1993/06/30  20:06:09  keving
 * nothing
 *
 * Revision 1.9  1993/06/03  21:08:08  keving
 * nothing
 *
 * Revision 1.8  1993/02/24  18:57:19  keving
 * nothing
 *
 * Revision 1.7  1993/02/23  22:58:36  keving
 * nothing
 *
 * Revision 1.6  1993/02/23  22:54:56  keving
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


/*==============*
 * HEADER FILES *
 *==============*/

#include <assert.h>
#include "all.h"
#include "mtypes.h"
#include "bitio.h"
#include "huff.h"
#include "postdct.h"
#include "opts.h"

/*==================*
 * STATIC VARIABLES *
 *==================*/

/* ZAG[i] is the natural-order position of the i'th element of zigzag order. */
int ZAG[] = {
    0, 1, 8, 16, 9, 2, 3, 10,
    17, 24, 32, 25, 18, 11, 4, 5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13, 6, 7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

/*
 * possible optimization: reorder the qtable in the correct zigzag order, to
 * reduce the number of necessary lookups
 *
 * this table comes from the MPEG draft, p. D-16, Fig. 2-D.15.
 */
int32_mpeg_t qtable[] = {
    8, 16, 19, 22, 26, 27, 29, 34,
    16, 16, 22, 24, 27, 29, 34, 37,
    19, 22, 26, 27, 29, 34, 34, 38,
    22, 22, 26, 27, 29, 34, 37, 40,
    22, 26, 27, 29, 32, 35, 40, 48,
    26, 27, 29, 32, 35, 40, 48, 58,
    26, 27, 29, 34, 38, 46, 56, 69,
    27, 29, 35, 38, 46, 56, 69, 83
};

int32_mpeg_t niqtable[] = {
     16, 16, 16, 16, 16, 16, 16, 16,
     16, 16, 16, 16, 16, 16, 16, 16,
     16, 16, 16, 16, 16, 16, 16, 16,
     16, 16, 16, 16, 16, 16, 16, 16,
     16, 16, 16, 16, 16, 16, 16, 16,
     16, 16, 16, 16, 16, 16, 16, 16,
     16, 16, 16, 16, 16, 16, 16, 16,
     16, 16, 16, 16, 16, 16, 16, 16
};

int32_mpeg_t    *customQtable = NULL;
int32_mpeg_t    *customNIQtable = NULL;

/*==================*
 * GLOBAL VARIABLES *
 *==================*/

extern boolean realQuiet;

/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/

/*===========================================================================*
 *
 * Mpost_UnQuantZigBlock
 *
 *    unquantize and zig-zag (decode) a single block
 *    see section 2.4.4.1 of MPEG standard
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Mpost_UnQuantZigBlock(in, out, qscale, iblock)
    FlatBlock in;
    Block out;
    int qscale;
    boolean iblock;
{
    register int index;
    int        start;
    int        position;
    register int        qentry;
    int        level, coeff;
    
    if ( iblock ) {
    /* qtable[0] must be 8 */
    out[0][0] = (int16_mpeg_t)(in[0] * 8);

    /* don't need to do anything fancy here, because we saved orig
        value, not encoded dc value */
    start = 1;
    } else {
    start = 0;
    }

    for ( index = start;  index < DCTSIZE_SQ;  index++ ) {
    position = ZAG[index];
    level = in[index];

    if (level == 0) {
      ((int16_mpeg_t *)out)[position] = 0;
      continue;
    }


    if ( iblock ) {
        qentry = qtable[position] * qscale;
        coeff = (level*qentry)/8;
        if ( (coeff & 1) == 0 ) {
        if ( coeff < 0 ) {
            coeff++;
        } else if ( coeff > 0 ) {
            coeff--;
        }
        }
    } else {
        qentry = niqtable[position] * qscale;
        if ( level == 0 ) {
        coeff = 0;
        } else if ( level < 0 ) {
        coeff = (((2*level)-1)*qentry) / 16;
        if ( (coeff & 1) == 0 ) {
            coeff++;
        }
        } else {
        coeff = (((2*level)+1)*qentry) >> 4;
        if ( (coeff & 1) == 0 ) {
            coeff--;
        }
        }

        if ( coeff > 2047 ) {
        coeff = 2047;
        } else if ( coeff < -2048 ) {
        coeff = -2048;
        }
        }

    ((int16_mpeg_t *)out)[position] = coeff;
    }
}


/*===========================================================================*
 *
 * Mpost_QuantZigBlock
 *
 *    quantize and zigzags a block
 *
 * RETURNS:    MPOST_OVERFLOW if a generated value is outside |255|
 *              MPOST_ZERO     if all coeffs are zero
 *              MPOST_NON_ZERO otherwisw
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
int
Mpost_QuantZigBlock(in, out, qscale, iblock)
    Block in;
    FlatBlock out;
    register int qscale;
    int iblock;
{
  register int i;
  register int16_mpeg_t temp;
  register int qentry;
  register int position;
  boolean nonZero = FALSE;
  boolean overflow = FALSE;
  
  DBG_PRINT(("Mpost_QuantZigBlock...\n"));
  if (iblock) {
    /*
     * the DC coefficient is handled specially -- it's not
     * sensitive to qscale, but everything else is
     */
    temp = ((int16_mpeg_t *) in)[ZAG[0]];
    qentry = qtable[ZAG[0]];
    
    if (temp < 0) {
      temp = -temp;
      temp += (qentry >> 1);
      temp /= qentry;
      temp = -temp;
    } else {
      temp += (qentry >> 1);
      temp /= qentry;
    }
    if ( temp != 0 ) {
      nonZero = TRUE;
    }
    out[0] = temp;
    
    for (i = 1; i < DCTSIZE_SQ; i++) {
      position = ZAG[i];
      temp = ((int16_mpeg_t *) in)[position];
      qentry = qtable[position] * qscale;
      
      /* see 1993 MPEG doc, section D.6.3.4 */
      if (temp < 0) {
    temp = -temp;
    temp = (temp << 3);    /* temp > 0 */
    temp += (qentry >> 1);
    temp /= qentry;
    temp = -temp;
      } else {
    temp = (temp << 3);    /* temp > 0 */
    temp += (qentry >> 1);
    temp /= qentry;
      }
      
      if ( temp != 0 ) {
    nonZero = TRUE;
    out[i] = temp;
    if (temp < -255) {
      temp = -255;
      overflow = TRUE;
    } else if (temp > 255) {
      temp = 255;
      overflow = TRUE;
    }
      } else out[i]=0;
    }
  } else {
    for (i = 0; i < DCTSIZE_SQ; i++) {
      position = ZAG[i];
      temp = ((int16_mpeg_t *) in)[position];
      
      /* multiply by non-intra qtable */
      qentry = qscale * niqtable[position];
      
      /* see 1993 MPEG doc, D.6.4.5 */
      temp *= 8;
      temp /= qentry;        /* truncation toward 0 -- correct */
      
      if ( temp != 0 ) {
    nonZero = TRUE;
    out[i] = temp;
    if (temp < -255) {
      temp = -255;
      overflow = TRUE;
    } else if (temp > 255) {
      temp = 255;
      overflow = TRUE;
    }
    
      } else out[i]=0;
    }
  }

 if (overflow) return MPOST_OVERFLOW;
 if (nonZero)  return MPOST_NON_ZERO;
 return MPOST_ZERO;
}



/*===========================================================================*
 *
 * Mpost_RLEHuffIBlock
 *
 *    generate the huffman bits from an I-block
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Mpost_RLEHuffIBlock(in, out)
    FlatBlock in;
    BitBucket *out;
{
    register int i;
    register int nzeros = 0;
    register int16_mpeg_t cur;
    register int16_mpeg_t acur;
    register uint32_mpeg_t code;
    register int nbits;

    /*
     * yes, Virginia, we start at 1.  The DC coefficient is handled
     * specially, elsewhere.  Not here.
     */
    for (i = 1; i < DCTSIZE_SQ; i++) {
    cur = in[i];
    acur = ABS(cur);
    if (cur) {
        if ( (nzeros < HUFF_MAXRUN) && (acur < huff_maxlevel[nzeros])) {
            /*
         * encode using the Huffman tables
         */

        DBG_PRINT(("rle_huff %02d.%02d: Run %02d, Level %4d\n", i,  ZAG[i], nzeros, cur));
        code = (huff_table[nzeros])[acur];
        nbits = (huff_bits[nzeros])[acur];

        if (cur < 0) {
            code |= 1;    /* the sign bit */
        }
        Bitio_Write(out, code, nbits);
        } else {
        /*
         * encode using the escape code
         */
        DBG_PRINT(("Escape\n"));
        Bitio_Write(out, 0x1, 6);    /* ESCAPE */
        DBG_PRINT(("Run Length\n"));
        Bitio_Write(out, nzeros, 6);    /* Run-Length */

        /*
             * this shouldn't happen, but the other
             * choice is to bomb out and dump core...
         * Hmmm, seems to happen with small Qtable entries (1) -srs
             */
        if (cur < -255) {
            cur = -255;
        } else if (cur > 255) {
            cur = 255;
        }

        DBG_PRINT(("Level\n"));
        if (acur < 128) {
            Bitio_Write(out, cur, 8);
        } else {
            if (cur < 0) {
            Bitio_Write(out, 0x8001 + cur + 255, 16);
            } else {
            Bitio_Write(out, cur, 16);
            }
        }
        }
        nzeros = 0;
    } else {
        nzeros++;
    }
    }
    DBG_PRINT(("End of block\n"));
    Bitio_Write(out, 0x2, 2);    /* end of block marker */
}


/*===========================================================================*
 *
 * Mpost_RLEHuffPBlock
 *
 *    generate the huffman bits from an P-block
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
Mpost_RLEHuffPBlock(in, out)
    FlatBlock in;
    BitBucket *out;
{
    register int i;
    register int nzeros = 0;
    register int16_mpeg_t cur;
    register int16_mpeg_t acur;
    register uint32_mpeg_t code;
    register int nbits;
    boolean first_dct = TRUE;

    /*
     * yes, Virginia, we start at 0.
     */
    for (i = 0; i < DCTSIZE_SQ; i++) {
    cur = in[i];
    acur = ABS(cur);
    if (cur) {
        if ((nzeros < HUFF_MAXRUN) && (acur < huff_maxlevel[nzeros])) {
            /*
         * encode using the Huffman tables
         */

        DBG_PRINT(("rle_huff %02d.%02d: Run %02d, Level %4d\n", i, ZAG[i], nzeros, cur));
        if ( first_dct && (nzeros == 0) && (acur == 1) ) {
            /* actually, only needs = 0x2 */
            code = (cur == 1) ? 0x2 : 0x3;
            nbits = 2;
        } else {
            code = (huff_table[nzeros])[acur];
            nbits = (huff_bits[nzeros])[acur];
          }

        assert(nbits);

        if (cur < 0) {
            code |= 1;    /* the sign bit */
        }
        Bitio_Write(out, code, nbits);
        first_dct = FALSE;
        } else {
        /*
         * encode using the escape code
         */
        DBG_PRINT(("Escape\n"));
        Bitio_Write(out, 0x1, 6);    /* ESCAPE */
        DBG_PRINT(("Run Length\n"));
        Bitio_Write(out, nzeros, 6);    /* Run-Length */

        /*
             * this shouldn't happen, but the other
             * choice is to bomb out and dump core...
         * Hmmm, seems to happen with small Qtable entries (1) -srs
             */
        if (cur < -255) {
          cur = -255;
        } else if (cur > 255) {
          cur = 255;
        }

        DBG_PRINT(("Level\n"));
        if (acur < 128) {
            Bitio_Write(out, cur, 8);
        } else {
            if (cur < 0) {
            Bitio_Write(out, 0x8001 + cur + 255, 16);
            } else {
            Bitio_Write(out, cur, 16);
            }
        }

        first_dct = FALSE;
        }
        nzeros = 0;
    } else {
        nzeros++;
    }
    }

    /* actually, should REALLY return FALSE and not use this! */

    if ( first_dct ) {    /* have to give a first_dct even if all 0's */
    fprintf(stderr, "HUFF called with all-zero coefficients\n");
    fprintf(stderr, "exiting...\n");
    exit(1);
    }

    DBG_PRINT(("End of block\n"));
    Bitio_Write(out, 0x2, 2);    /* end of block marker */
}


/*===========================================================================*
 *
 * CalcRLEHuffLength
 *
 *    count the huffman bits for an P-block
 *
 * RETURNS:    number of bits
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
int
CalcRLEHuffLength(in)
    FlatBlock in;
{
  register int i;
  register int nzeros = 0;
  register int16_mpeg_t cur;
  register int16_mpeg_t acur;
  register int nbits;
  register int countbits=0;
  boolean first_dct = TRUE;
  
  for (i = 0; i < DCTSIZE_SQ; i++) {
    cur = in[i];
    acur = ABS(cur);
    if (cur) {
      if ((nzeros < HUFF_MAXRUN) && (acur < huff_maxlevel[nzeros])) {
    /*
     * encode using the Huffman tables
     */

    if ( first_dct && (nzeros == 0) && (acur == 1) ) {
      nbits = 2;
    } else {
      nbits = (huff_bits[nzeros])[acur];
    }
    countbits += nbits;
    first_dct = FALSE;
      } else {
    countbits += 12;    /* ESCAPE + runlength */

    if (acur < 128) {
      countbits += 8;
    } else {
      countbits += 16;
    }

    first_dct = FALSE;
      }
      nzeros = 0;
    } else {
      nzeros++;
    }
  }
  
  countbits += 2; /* end of block marker */
  return countbits;
}
