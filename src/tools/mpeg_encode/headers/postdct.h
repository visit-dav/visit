/*===========================================================================*
 * postdct.h                                     *
 *                                         *
 *    MPEG post-DCT processing                         *
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



int Mpost_QuantZigBlock _ANSI_ARGS_((Block in, FlatBlock out, int qscale,
                     int iblock));
void    Mpost_UnQuantZigBlock _ANSI_ARGS_((FlatBlock in, Block out,
                       int qscale, boolean iblock));
void    Mpost_RLEHuffIBlock _ANSI_ARGS_((FlatBlock in, BitBucket *out));
void    Mpost_RLEHuffPBlock _ANSI_ARGS_((FlatBlock in, BitBucket *out));

#define MPOST_ZERO 0
#define MPOST_NON_ZERO 1
#define MPOST_OVERFLOW (-1)
