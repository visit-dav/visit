#
# Teem: Gordon Kindlmann's research software
# Copyright (C) 2005  Gordon Kindlmann
# Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah
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

#### Library name
####
####
L := ten
####
####
####

# boilerplate: default targets and include tricks
TEEM_ROOT ?= ../..
TEEM_SRC ?= ..
ifeq (,$(DEF_TARGETS))
DEF_TARGETS = true
dev     : $(L)/dev
install : $(L)/install
clean   : $(L)/clean
clobber : $(L)/clobber
include ../GNUmakefile
endif
ifeq (,$($(L).SEEN))
$(L).SEEN := true

#### Describe library here
####
#### tendCalc.o nixed
$(L).NEED = echo limn gage dye unrrdu ell nrrd biff air
$(L).PUBLIC_HEADERS = ten.h tenMacros.h
$(L).PRIVATE_HEADERS = privateTen.h
$(L).OBJS = tensor.o chan.o aniso.o glyph.o enumsTen.o grads.o miscTen.o \
	mod.o estimate.o tenGage.o tenDwiGage.o \
        fiberMethods.o fiber.o epireg.o defaultsTen.o bimod.o bvec.o \
	tendFlotsam.o tendGrads.o tendAnplot.o tendAnvol.o tendEval.o \
	tendEvec.o tendSten.o tendExpand.o tendEvq.o tendPoint.o \
	tendAnhist.o tendMake.o tendSatin.o tendShrink.o tendGlyph.o \
	tendFiber.o tendEpireg.o tendBmat.o tendEstim.o tendSim.o \
	tendSlice.o tendEllipse.o tendEvecrgb.o tendNorm.o tendAnscale.o \
	tendEvalpow.o tendEvalclamp.o tendEvaladd.o tendHelix.o tendBfit.o \
	tendUnmf.o
$(L).TESTS = test/tg test/tt test/tem test/rotedge test/tsoid test/odf-hist \
        test/to
####
####
####

# boilerplate: declare rules for this library
include $(TEEM_SRC)/make/template.mk
endif
ifeq (,$(INCLUDED))
  include $(TEEM_SRC)/bin/GNUmakefile
endif
