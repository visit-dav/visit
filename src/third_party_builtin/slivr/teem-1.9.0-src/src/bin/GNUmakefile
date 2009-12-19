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

#### Name of the library goes here.
#### 
#### 
L := bin
####
#### We're not a library at all actually ...
####

# boilerplate: default targets (except usable) and include tricks
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

#### By the ordering and guarding of includes, this will ALWAYS be the 
#### last "library" makefile to be parsed, regardless of the directory
#### in which make started.

## BINS: all the command-line executables associated with teem
## 
BINS = airSanity affine cubic nrrdSanity overrgb idx2pos emap undos talkweb \
  pos2idx unu pprobe vprobe qbert mrender miter gkms ungantry ilk tend spots \
  ninspect

## _binD(bins): list of development locations for given bins
## _binI(bins): list of install locations for given bins
## _binDI(bins): list of install and development locations for given bins
##
_binD = $(foreach bin,$(1),$(TEEM_SRC)/bin/$(bin))
_binI = $(foreach bin,$(1),$(BDEST)/$(bin))
_binDI = $(foreach bin,$(1),$(TEEM_SRC)/bin/$(bin) $(BDEST)/$(bin))

## BINS.{DEV,INST}: full paths to development and install versions
## 
BINS.DEV := $(call _binD,$(BINS))
BINS.INST := $(call _binI,$(BINS))


## Entry-point targets for binaries.  There is no "usable" for
## binaries, since they aren't libraries.
##
bin/dev: $(BINS.DEV)
bin/install: $(BINS.INST)
bin/clean: 
	$(RM) $(addsuffix $(DOTEXE),$(BINS.DEV))
bin/clobber: bin/clean
	$(RM) $(addsuffix $(DOTEXE),$(BINS.INST))

## All the prerequisite usables of the bins, **RECURSIVELY EXPANDED**
## (yes, by hand, by you, Gordon!).  The calls to need() will work
## correctly because we've gotten here only after going through all
## the other library makesfiles.  The arguments to need() are
## libraries for which we need a header and/or libraries we need to
## link against.
##	
airSanity.need  = $(call meneed,air)
affine.need     = $(call meneed,air)
cubic.need      = $(call meneed,ell)
nrrdSanity.need = $(call meneed,nrrd biff)
undos.need      = $(call meneed,hest air)
idx2pos.need    = $(call meneed,nrrd hest air)
pos2idx.need    = $(call meneed,nrrd hest air)
overrgb.need    = $(call meneed,nrrd hest biff air)
qbert.need      = $(call meneed,bane gage nrrd hest air)
ilk.need        = $(call meneed,moss unrrdu nrrd ell biff hest air)
emap.need       = $(call meneed,limn nrrd ell biff hest air)
gkms.need       = $(call meneed,bane nrrd biff air)
ninspect.need   = $(call meneed,nrrd biff hest air)
unu.need        = $(call meneed,unrrdu nrrd biff hest air)
miter.need      = $(call meneed,mite hoover limn nrrd ell biff air)
ungantry.need   = $(call meneed,gage nrrd biff hest air)
talkweb.need    = $(call meneed,nrrd biff hest air)
tend.need       = $(call meneed,ten limn gage dye unrrdu nrrd ell biff air)
mrender.need    = $(call meneed,ten hoover limn gage nrrd biff hest air)
vprobe.need     = $(call meneed,ten limn gage nrrd ell biff hest air)
pprobe.need     = $(call meneed,ten limn gage nrrd ell biff hest air)
spots.need      = $(call meneed,alan nrrd ell biff hest air)

## Old: "We need some way of expressing the fact that a given binary
## (install and dev) depends on the usable of its top dependency."
## Problem: Its not sufficient to use the "top dependency" because the
## dependencies are not strict subsets/supersets: mrender's "top"
## dependency is hoover, but hoover doesn't depend on gage, which
## mrender needs
##
$(call _binDI,airSanity)  : $(call used,$(airSanity.need))
$(call _binDI,affine)     : $(call used,$(affine.need))
$(call _binDI,cubic)      : $(call used,$(cubic.need))
$(call _binDI,nrrdSanity) : $(call used,$(nrrdSanity.need))
$(call _binDI,undos)      : $(call used,$(undos.need))
$(call _binDI,idx2pos)    : $(call used,$(idx2pos.need))
$(call _binDI,pos2idx)    : $(call used,$(pos2idx.need))
$(call _binDI,overrgb)    : $(call used,$(overrgb.need))
$(call _binDI,qbert)      : $(call used,$(qbert.need))
$(call _binDI,ilk)        : $(call used,$(ilk.need))
$(call _binDI,emap)       : $(call used,$(emap.need))
$(call _binDI,vprobe)     : $(call used,$(vprobe.need))
$(call _binDI,pprobe)     : $(call used,$(pprobe.need))
$(call _binDI,gkms)       : $(call used,$(gkms.need))
$(call _binDI,ninspect)   : $(call used,$(ninspect.need))
$(call _binDI,unu)        : $(call used,$(unu.need))
$(call _binDI,mrender)    : $(call used,$(mrender.need))
$(call _binDI,miter)      : $(call used,$(miter.need))
$(call _binDI,ungantry)   : $(call used,$(ungantry.need))
$(call _binDI,talkweb)    : $(call used,$(talkweb.need))
$(call _binDI,tend)       : $(call used,$(tend.need))
$(call _binDI,spots)      : $(call used,$(spots.need))

## How to make bins.dev and bins.inst: both kinds link against the
## same (installed) libraries and include the same (installed)
## headers, so the commands aren't actually any different ...
## 
$(TEEM_SRC)/bin/% $(BDEST)/% : $(TEEM_SRC)/bin/%.c
	$(P) $(CC) $(CFLAGS) $(BIN_CFLAGS) $(IPATH) -o $@ $< \
	$(LPATH) $(call link,$($(notdir $@).need)) \
	$(call xtern.Lpath,$($(notdir $@).need)) \
	$(call xtern.link,$($(notdir $@).need)) -lm

# we're not a library; template.mk doesn't apply to us
endif
