/*===========================================================================*
 * nojpeg.c                                     *
 *                                         *
 *    procedures to deal with JPEG files                     *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    JMovie2JPEG                                 *
 *      ReadJPEG                                 *
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
 *  $Header: /n/charlie-brown/project/mm/mpeg/mpeg_dist/mpeg_encode/RCS/nojpeg.c,v 1.2 1995/01/19 23:08:55 eyhung Exp $
 *  $Log: nojpeg.c,v $
 * Revision 1.2  1995/01/19  23:08:55  eyhung
 * Changed copyrights
 *
 * Revision 1.1  1994/03/15  00:27:11  keving
 * nothing
 *
 * Revision 1.2  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.1  1993/07/22  22:23:43  keving
 * nothing
 *
 */

/*==============*
 * HEADER FILES *
 *==============*/

#include "all.h"
#include "mtypes.h"
#include "frames.h"
#include "prototypes.h"
#include "param.h"
#include "readframe.h"
#include "fsize.h"
#include "rgbtoycc.h"
#include "jpeg.h"



/*=======================================================================*
 *                                                                       *
 * JMovie2JPEG                                                           *
 *                                                                       *
 *      Splits up a Parallax J_Movie into a set of JFIF image files      *
 *                                                                       *
 * RETURNS:     nothing                                                  *
 *                                                                       *
 * SIDE EFFECTS:    none                                                 *
 *                                                                       *
 *   Contributed By James Boucher(jboucher@flash.bu.edu)                 *
 *               Boston University Multimedia Communications Lab         *
 * This code was adapted from the Berkeley Playone.c and Brian Smith's   *
 * JGetFrame code after being modified on 10-7-93 by Dinesh Venkatesh    *
 * of BU.                                                                *
 *       This code converts a version 2 Parallax J_Movie into a          *
 * set of JFIF compatible JPEG images. It works for all image            *
 * sizes and qualities.                                                  *
 ************************************************************************/
void
JMovie2JPEG(infilename,obase,start,end)
    char *infilename;       /* input filename string */
    char *obase;            /* output filename base string=>obase##.jpg */ 
    int start;              /* first frame to be extracted */
    int end;                /* last frame to be extracted */
{
    fprintf(stderr, "ERROR:  This has not been compiled with JPEG support\n");
    exit(1);
}




/*===========================================================================*
 *
 * ReadJPEG  contributed by James Arthur Boucher of Boston University's
 *                                Multimedia Communications Lab
 *
 *      read a JPEG file and copy data into frame original data arrays
 *
 * RETURNS:     mf modified
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
/*************************JPEG LIBRARY INTERFACE*********************/
/*
 * THE BIG PICTURE:
 *
 * The rough outline this JPEG decompression operation is:
 *
 *      allocate and initialize JPEG decompression object
 *      specify data source (eg, a file)
 *      jpeg_read_header();     // obtain image dimensions and other parameters
 *      set parameters for decompression
 *      jpeg_start_decompress();
 *      while (scan lines remain to be read)
 *              jpeg_read_scanlines(...);
 *      jpeg_finish_decompress();
 *      release JPEG decompression object
 *
 */
void
ReadJPEG(mf, fp)
    MpegFrame *mf;
    FILE *fp;
{
    fprintf(stderr, "ERROR:  This has not been compiled with JPEG support\n");
    exit(1);
}
