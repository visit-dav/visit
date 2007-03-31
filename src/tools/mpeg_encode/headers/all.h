/*===========================================================================*
 * all.h                                     *
 *                                         *
 *    stuff included from ALL source files                     *
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
 *  $Header: /u/smoot/md/mpeg_encode/headers/RCS/all.h,v 1.9 1995/06/05 21:11:06 smoot Exp $
 *  $Log: all.h,v $
 * Revision 1.9  1995/06/05  21:11:06  smoot
 * added little_endian force for irizx
 *
 * Revision 1.8  1995/02/02  22:02:18  smoot
 * added ifdefs for compatability on stranger and stranger architectures...
 *
 * Revision 1.7  1995/02/02  07:26:45  eyhung
 * added parens to all.h to remove compiler warning
 *
 * Revision 1.6  1995/02/02  01:47:11  eyhung
 * added MAXINT
 *
 * Revision 1.5  1995/01/19  23:54:33  eyhung
 * Changed copyrights
 *
 * Revision 1.4  1994/11/14  22:52:04  smoot
 * Added linux #include for time.h
 *
 * Revision 1.3  1994/11/12  02:12:13  keving
 * nothing
 *
 * Revision 1.2  1993/07/22  22:24:23  keving
 * nothing
 *
 * Revision 1.1  1993/07/09  00:17:23  keving
 * nothing
 *
 */


#ifndef ENCODE_ALL_INCLUDED
#define ENCODE_ALL_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <limits.h>

/* There's got to be a better way.... */
#ifdef LINUX
#include <time.h>
#endif
#ifdef MIPS
#include <time.h>
#endif
#ifdef IRIX
#define FORCE_LITTLE_ENDIAN
#include <time.h>
#endif

#include "libpnmrw.h"
#include "ansi.h"
#include "general.h"

/* some machines have #define index strchr; get rid of this nonsense */
#ifdef index
#undef index
#endif /* index */

#ifndef MAXINT
#define MAXINT 0x7fffffff
#endif

#endif /* ENCODE_ALL_INCLUDED */
