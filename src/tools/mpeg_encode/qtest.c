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
 *  $Header: /n/charlie-brown/project/mm/mpeg/mpeg_dist/mpeg_encode/RCS/qtest.c,v 1.5 1995/01/19 23:09:15 eyhung Exp $
 *  $Log: qtest.c,v $
 * Revision 1.5  1995/01/19  23:09:15  eyhung
 * Changed copyrights
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

#include <stdio.h>
#include "mtypes.h"
#include "mproto.h"

main()
{
    Block a;
    FlatBlock b;
    BitBucket *bb;
    int i, j;

    bb = new_bitbucket();

    for (i = 0; i < 8; i++)
    for (j = 0; j < 8; j++)
        a[i][j] = random() % 100;
    mp_quant_zig_block(a, b, 1, 1);
    for (i = 0; i < 64; i++)
    printf("%6d ", b[i]);
    printf("\n");

    mp_rle_huff_block(b, bb);    /* intuititve names, huh? */

    printf("Huffman output is %d bits\n", bb->totalbits);
}
