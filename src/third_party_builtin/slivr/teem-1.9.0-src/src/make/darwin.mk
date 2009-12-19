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

# this was causing some troubles, will wait until GNU 
# configure/libtool solution is implemented ...
# TEEM_SHEXT = dylib

AR = libtool
ARFLAGS = -static -o
RANLIB = ranlib

LD = gcc

OPT_CFLAG ?= -O2
STATIC_CFLAG = -Wl,-prebind
SHARED_CFLAG =
SHARED_LDFLAG = -dynamic -dynamiclib -fno-common
SHARED_INSTALL_NAME = -install_name

ARCH_CFLAG = -W -Wall
ARCH_LDFLAG =

ifeq ($(SUBARCH),64)
  TEEM_32BIT = 0
else
  ifeq ($(SUBARCH),32)
    TEEM_32BIT = 1
  else
    $(error darwin sub-architecture "$(SUBARCH)" not recognized)
  endif
endif

TEEM_ENDIAN = 4321
TEEM_QNANHIBIT = 1
TEEM_DIO = 0
TEEM_32BIT = 1

TEEM_ZLIB.IPATH ?=
TEEM_ZLIB.LPATH ?=

TEEM_BZIP2.IPATH ?=
TEEM_BZIP2.LPATH ?=
