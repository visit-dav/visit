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
 *  $Header: /n/charlie-brown/project/mm/mpeg/mpeg_dist/mpeg_encode/RCS/memory.c,v 1.3 1995/01/19 23:08:43 eyhung Exp $
 *  $Log: memory.c,v $
 * Revision 1.3  1995/01/19  23:08:43  eyhung
 * Changed copyrights
 *
 * Revision 1.2  1993/06/03  21:08:08  keving
 * nothing
 *
 * Revision 1.1  1993/04/27  21:32:26  keving
 * nothing
 *
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include "memory.h"


/* memory handling routines
 *
 */

long    totalMemory = 0;
long    maxMemory = 0;


char    *MemAlloc(size_t size)
{
    totalMemory += (long)size;
    if ( totalMemory > maxMemory )
    {
    maxMemory = totalMemory;
    }

    return malloc(size);
}

void    MemFree(char *ptr, long bytes)
{
    totalMemory -= bytes;
    free(ptr);
}

void    PrintMaxMemory(void)
{
    fprintf(stdout, "MMMMM-----MAX MEMORY-----MMMMM = %ld\n", maxMemory);
}
