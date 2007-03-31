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
 *  $Header: /n/charlie-brown/project/mm/mpeg/mpeg_dist/mpeg_encode/RCS/huff.c,v 1.6 1995/01/19 23:07:39 eyhung Exp $
 */

/*  
 *  THIS FILE IS MACHINE GENERATED!  DO NOT EDIT!
 */
#include "mtypes.h"
#include "huff.h"

int huff_maxlevel[32] = { 41, 19, 6, 5, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };

uint32_mpeg_t huff_table0[41] = { 0x0, 0x6, 0x8, 0xa, 0xc, 0x4c, 0x42, 0x14, 0x3a, 0x30, 0x26, 0x20, 0x34, 0x32, 0x30, 0x2e, 0x3e, 0x3c, 0x3a, 0x38, 0x36, 0x34, 0x32, 0x30, 0x2e, 0x2c, 0x2a, 0x28, 0x26, 0x24, 0x22, 0x20, 0x30, 0x2e, 0x2c, 0x2a, 0x28, 0x26, 0x24, 0x22, 0x20 };
int huff_bits0[41] = { 0, 3, 5, 6, 8, 9, 9, 11, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16 };

uint32_mpeg_t huff_table1[19] = { 0x0, 0x6, 0xc, 0x4a, 0x18, 0x36, 0x2c, 0x2a, 0x3e, 0x3c, 0x3a, 0x38, 0x36, 0x34, 0x32, 0x26, 0x24, 0x22, 0x20 };
int huff_bits1[19] = { 0, 4, 7, 9, 11, 13, 14, 14, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17 };

uint32_mpeg_t huff_table2[6] = { 0x0, 0xa, 0x8, 0x16, 0x28, 0x28 };
int huff_bits2[6] = { 0, 5, 8, 11, 13, 14 };

uint32_mpeg_t huff_table3[5] = { 0x0, 0xe, 0x48, 0x38, 0x26 };
int huff_bits3[5] = { 0, 6, 9, 13, 14 };

uint32_mpeg_t huff_table4[4] = { 0x0, 0xc, 0x1e, 0x24 };
int huff_bits4[4] = { 0, 6, 11, 13 };

uint32_mpeg_t huff_table5[4] = { 0x0, 0xe, 0x12, 0x24 };
int huff_bits5[4] = { 0, 7, 11, 14 };

uint32_mpeg_t huff_table6[4] = { 0x0, 0xa, 0x3c, 0x28 };
int huff_bits6[4] = { 0, 7, 13, 17 };

uint32_mpeg_t huff_table7[3] = { 0x0, 0x8, 0x2a };
int huff_bits7[3] = { 0, 7, 13 };

uint32_mpeg_t huff_table8[3] = { 0x0, 0xe, 0x22 };
int huff_bits8[3] = { 0, 8, 13 };

uint32_mpeg_t huff_table9[3] = { 0x0, 0xa, 0x22 };
int huff_bits9[3] = { 0, 8, 14 };

uint32_mpeg_t huff_table10[3] = { 0x0, 0x4e, 0x20 };
int huff_bits10[3] = { 0, 9, 14 };

uint32_mpeg_t huff_table11[3] = { 0x0, 0x46, 0x34 };
int huff_bits11[3] = { 0, 9, 17 };

uint32_mpeg_t huff_table12[3] = { 0x0, 0x44, 0x32 };
int huff_bits12[3] = { 0, 9, 17 };

uint32_mpeg_t huff_table13[3] = { 0x0, 0x40, 0x30 };
int huff_bits13[3] = { 0, 9, 17 };

uint32_mpeg_t huff_table14[3] = { 0x0, 0x1c, 0x2e };
int huff_bits14[3] = { 0, 11, 17 };

uint32_mpeg_t huff_table15[3] = { 0x0, 0x1a, 0x2c };
int huff_bits15[3] = { 0, 11, 17 };

uint32_mpeg_t huff_table16[3] = { 0x0, 0x10, 0x2a };
int huff_bits16[3] = { 0, 11, 17 };

uint32_mpeg_t huff_table17[2] = { 0x0, 0x3e };
int huff_bits17[2] = { 0, 13 };

uint32_mpeg_t huff_table18[2] = { 0x0, 0x34 };
int huff_bits18[2] = { 0, 13 };

uint32_mpeg_t huff_table19[2] = { 0x0, 0x32 };
int huff_bits19[2] = { 0, 13 };

uint32_mpeg_t huff_table20[2] = { 0x0, 0x2e };
int huff_bits20[2] = { 0, 13 };

uint32_mpeg_t huff_table21[2] = { 0x0, 0x2c };
int huff_bits21[2] = { 0, 13 };

uint32_mpeg_t huff_table22[2] = { 0x0, 0x3e };
int huff_bits22[2] = { 0, 14 };

uint32_mpeg_t huff_table23[2] = { 0x0, 0x3c };
int huff_bits23[2] = { 0, 14 };

uint32_mpeg_t huff_table24[2] = { 0x0, 0x3a };
int huff_bits24[2] = { 0, 14 };

uint32_mpeg_t huff_table25[2] = { 0x0, 0x38 };
int huff_bits25[2] = { 0, 14 };

uint32_mpeg_t huff_table26[2] = { 0x0, 0x36 };
int huff_bits26[2] = { 0, 14 };

uint32_mpeg_t huff_table27[2] = { 0x0, 0x3e };
int huff_bits27[2] = { 0, 17 };

uint32_mpeg_t huff_table28[2] = { 0x0, 0x3c };
int huff_bits28[2] = { 0, 17 };

uint32_mpeg_t huff_table29[2] = { 0x0, 0x3a };
int huff_bits29[2] = { 0, 17 };

uint32_mpeg_t huff_table30[2] = { 0x0, 0x38 };
int huff_bits30[2] = { 0, 17 };

uint32_mpeg_t huff_table31[2] = { 0x0, 0x36 };
int huff_bits31[2] = { 0, 17 };

uint32_mpeg_t *huff_table[32] = { huff_table0, huff_table1, huff_table2, huff_table3, huff_table4, huff_table5, huff_table6, huff_table7, huff_table8, huff_table9, huff_table10, huff_table11, huff_table12, huff_table13, huff_table14, huff_table15, huff_table16, huff_table17, huff_table18, huff_table19, huff_table20, huff_table21, huff_table22, huff_table23, huff_table24, huff_table25, huff_table26, huff_table27, huff_table28, huff_table29, huff_table30, huff_table31 };
int *huff_bits[32] = { huff_bits0, huff_bits1, huff_bits2, huff_bits3, huff_bits4, huff_bits5, huff_bits6, huff_bits7, huff_bits8, huff_bits9, huff_bits10, huff_bits11, huff_bits12, huff_bits13, huff_bits14, huff_bits15, huff_bits16, huff_bits17, huff_bits18, huff_bits19, huff_bits20, huff_bits21, huff_bits22, huff_bits23, huff_bits24, huff_bits25, huff_bits26, huff_bits27, huff_bits28, huff_bits29, huff_bits30, huff_bits31 };
