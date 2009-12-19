#
# teem: Gordon Kindlmann's research software
# Copyright (C) 2005  Gordon Kindlmann
# Copyright (C) 2003, 2002, 2001, 2000, 1999, 1998  University of Utah
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#

## XTERNS: list of all the identifiers for the various external
## libraries that we can _optionally_ link against.  Teem has no
## notion of depending on these in the makefile sense.  Teem doesn't
## try to represent inter-external dependencies (e.g. PNG on zlib)
## explicitly, but the ordering of the xterns below has to reflect
## the ordering on the link line (e.g. PNG preceeds ZLIB)
##
## Extern EXT is enabled during make by setting the environment
## variable TEEM_EXT (just set it, not to anything in particular).  If
## external EXT is enabled during make, then TEEM_EXT will be defined
## as "1" during source file compilation.
##
XTERNS = PNG ZLIB BZIP2 PTHREAD

## ZLIB: for the zlib library underlying gzip and the PNG image
## format.  Using zlib enables the "gzip" nrrd data encoding.  Header
## file is <zlib.h>.
##
## Arch-specific .mk files may need to set TEEM_ZLIB_IPATH and
## TEEM_ZLIB_LPATH to "-I<path>" and "-L<path>" for the compile and
## link lines, respectively.
ZLIB.LINK = -lz
nrrd.XTERN += ZLIB

## BZIP2: for the bzip2 compression library.  Using bzip2 enables
## the "bzip2" nrrd data encoding.  Header file is <bzlib.h>.
##
## Arch-specific .mk files may need to set TEEM_BZIP2_IPATH and
## TEEM_BZIP2_LPATH to "-I<path>" and "-L<path>" for the compile and
## link lines, respectively.
BZIP2.LINK = -lbz2
nrrd.XTERN += BZIP2

## PNG: for PNG images.  Using PNG enables the "png" nrrd format.
## Header file is <png.h>
##
## Arch-specific .mk files may need to set TEEM_PNG_IPATH and
## TEEM_PNG_LPATH to "-I<path>" and "-L<path>" for the compile and
## link lines, respectively.
PNG.LINK = -lpng
nrrd.XTERN += PNG

## PTHREAD: use pthread-based multi-threading in airThreads.  Note
## that Windows has its own multithreading capabilities, which is used
## in airThread if !TEEM_PTHREAD, and we are on windows. Header file is
## <pthread.h>
##
## Arch-specific .mk files may need to set TEEM_PTHREAD_IPATH and
## TEEM_PTHREAD_LPATH to "-I<path>" and "-L<path>" for the compile and
## link lines, respectively.
PTHREAD.LINK = -lpthread
air.XTERN += PTHREAD
