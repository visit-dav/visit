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

TEEM_LITTER = so_locations
TEEM_SHEXT = so
SHARED_LINK_NAME = -rpath 
TEEM_CPP_ERROR_DIE = -diag_error 1035
SHARED_RPATH = -rpath 

OPT_CFLAG ?= -O2
STATIC_CFLAG = -Bstatic
SHARED_CFLAG = -Bdynamic
SHARED_LDFLAG = -shared
ifeq ($(SUBARCH),n32)
  ARCH_CFLAG = -n32
  ARCH_LDFLAG = -n32
  TEEM_32BIT = 1
else
  ifeq ($(SUBARCH),64)
    ARCH_CFLAG = -64
    ARCH_LDFLAG = -64
    TEEM_32BIT = 0
  else 
    $(error irix6 sub-architecture "$(SUBARCH)" not recognized) 
  endif
endif

TEEM_ENDIAN = 4321
TEEM_QNANHIBIT = 0
TEEM_DIO = 1

TEEM_PURIFY_CMD = purify

TEEM_ZLIB.IPATH ?=
TEEM_ZLIB.LPATH ?=

TEEM_BZIP2.IPATH ?=
TEEM_BZIP2.LPATH ?=
