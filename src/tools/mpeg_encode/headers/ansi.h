/*===========================================================================*
 * ansi.h                                     *
 *                                         *
 *    macro for non-ansi compilers                         *
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
 *  $Header: /n/picasso/project/mpeg/mpeg_dist/mpeg_encode/headers/RCS/ansi.h,v 1.6 1995/08/15 23:43:13 smoot Exp $
 *  $Log: ansi.h,v $
 *  Revision 1.6  1995/08/15 23:43:13  smoot
 *  *** empty log message ***
 *
 *  Revision 1.5  1995/01/19 23:54:35  eyhung
 *  Changed copyrights
 *
 * Revision 1.4  1994/11/12  02:12:13  keving
 * nothing
 *
 * Revision 1.3  1993/07/22  22:24:23  keving
 * nothing
 *
 * Revision 1.2  1993/07/09  00:17:23  keving
 * nothing
 *
 * Revision 1.1  1993/06/14  22:50:22  keving
 * nothing
 *
 */


#ifndef ANSI_INCLUDED
#define ANSI_INCLUDED


/*  
 *  _ANSI_ARGS_ macro stolen from Tcl6.5 by John Ousterhout
 */
#undef _ANSI_ARGS_
#undef const
#ifdef NON_ANSI_COMPILER
#define _ANSI_ARGS_(x)       ()
#define CONST
#else
#define _ANSI_ARGS_(x)   x
#define CONST const
#ifdef __cplusplus
#define VARARGS (...)
#else
#define VARARGS ()
#endif
#endif


#endif /* ANSI_INCLUDED */
