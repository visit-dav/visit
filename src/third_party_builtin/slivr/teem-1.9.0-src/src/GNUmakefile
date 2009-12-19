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

####
#### top-level GNUmakefile: Master makefile for teem
####

## ".SUFFIXES :" speeds debugging with make -d (and probably make in
## general) by eliminating the list of suffixes checked by implicit
## pattern rules.  The rest of the rules are to tell make to forget
## about trying to automatically update the files that we "include"
##
.SUFFIXES :
% : %,v
% : RCS/%,v
% : RCS/%
% : s.%
% : SCCS/s.%
%.c : %.w     # this doesn't seem to work, unfortunately

## TEEM_ROOT: a relative path to the directory which contains the
## "src", "include", and all the architecture-specific directories
## (which in turn contain "bin", "lib", "obj", and "purify"). Whether
## make started on this makefile or on an individual library's
## makefile determines who gets to set TEEM_ROOT first.  Same for
## TEEM_SRC, a relative path to the "src" directory
##
TEEM_ROOT ?= ..
TEEM_SRC ?= .

## When making Windows project files, set TEEM_ARCH to cygwin
ifeq (project, $(MAKECMDGOALS))
TEEM_ARCH ?= cygwin
endif

## read in the check* functions, and check on TEEM_ARCH
##
include $(TEEM_SRC)/make/errorCheck.mk
$(checkArchSet)
$(checkArchLinux)
$(checkArchLinux64)
$(checkArchDarwin)
$(checkArchValid)
$(checkTeemDest)

## the architecture name may have two parts, ARCH and SUBARCH,
## seperated by one period
##
ARCH = $(basename $(TEEM_ARCH))
SUBARCH = $(patsubst .%,%,$(suffix $(TEEM_ARCH)))

## Before we read in the architecture-dependent stuff, take a stab at
## defining the various programs we'll need, and some of their flags.
## If these are not over-written, we assume that they'll work.
##
CC ?= cc
LD = ld
AR = ar
ARFLAGS = ru
RM = rm -f
CP = cp
CHMOD = chmod
SLEEP = sleep

## Enstate the architecture-dependent settings by reading through the
## file specific to the chosen architecture, then check the things that
## are set there.
##
include $(TEEM_SRC)/make/$(ARCH).mk
$(checkShext)
$(checkPurify)

## information about optional external libraries to link with
##
include $(TEEM_SRC)/make/externals.mk

## LIBS: all the teem libraries we'll try to build
## NUMS: "numbering" of all the libraries in dependency order
## RNUMS: in link order, and "top-dep" order needed by bin/GNUmakefile
## {R}NUM/LIBS: used for craziness below
##
LIBS = air hest biff nrrd ell unrrdu dye moss gage bane limn hoover alan echo ten mite coil push
NUMS =  0   1    2    3    4    5    6    7    8    9    A    B     C    D    E   F    G  H
RNUMS = H G   F   E    D    C    B    A    9    8    7    6    5    4     3    2    1   0

NUM/LIBS = $(join $(NUMS),$(LIBS:%=/%))
RNUM/LIBS = $(join $(RNUMS),$(LIBS:%=/%))

## MODES: the different kinds of builds that we support.  Declaring
## these modes (for all libraries) as phony saves make the effort of
## trying implicit rules to update them.
##
MODES = install dev clean clobber
.PHONY : $(foreach LIB,teem $(LIBS),$(MODES:%=$(LIB)/%)) teem.dsp

## Top-level/default rules.  If make started with this file, then
## these will be the first rules that make sees, so that "make"
## defaults to "make X/install" for *every* library X.  If make
## started in a library subdirectory, these won't be seen at all.
##
ifeq (,$(DEF_TARGETS))
  install : teem/install
  dev     : teem/dev
  clean   : teem/clean
  clobber : teem/clobber
  DEF_TARGETS := true
endif

## Top-level rules, available regardless of where make started
##
teem/install : $(addsuffix /install,$(LIBS) bin) megalibs
teem/dev     : $(addsuffix /dev,$(LIBS) bin)
teem/clean   : $(addsuffix /clean,$(LIBS) bin)
	$(if $(TEEM_LITTER),$(RM) $(TEEM_ROOT)/src/$(TEEM_LITTER))
teem/clobber : teem/clean $(addsuffix /clobber,$(LIBS) bin) unmegalibs
nothing : ;

## if.missing(stuff): returns $stuff if one or more of the files
## listed in stuff don't exist
##
if.missing = $(if $(strip $(foreach x,$(1),$(if $(wildcard $(x)),,no))),$(1))

## create.if.missing(dir): creates directory dir if it doesn't exist,
## and gives a warning to that effect
##
create.if.missing = \
  $(if $(call if.missing,$(1)),\
    $(warning *** WARNING ***: Creating needed directory $(1)) \
      $(shell mkdir $(1)))

## Set directory-related variables: where to install things, as well
## as the directories used in conjunction with the -I and -L path
## flags for cc and ld
##
ifeq (undefined,$(origin TEEM_DEST))
  IDEST = $(TEEM_ROOT)/include
  LDEST = $(TEEM_ROOT)/$(TEEM_ARCH)/lib
  BDEST = $(TEEM_ROOT)/$(TEEM_ARCH)/bin
else
  IDEST = $(TEEM_DEST)/include
  LDEST = $(TEEM_DEST)/lib
  BDEST = $(TEEM_DEST)/bin
  $(foreach dir,$(IDEST) $(LDEST) $(BDEST),\
     $(call create.if.missing,$(dir)))
endif
## create IDEST/teem if necessary
$(call create.if.missing,$(IDEST)/teem)
ODEST = $(TEEM_ROOT)/$(TEEM_ARCH)/obj
PCACHE = $(TEEM_ROOT)/$(TEEM_ARCH)/purify
IPATH += -I$(IDEST)
LPATH += -L$(LDEST)
ifneq (undefined,$(origin TEEM_DEST))
  # we still need this for the teem*.h headers (such as teem32bit.h)
  IPATH += -I$(TEEM_ROOT)/include
endif

## By giving a list of library extensions we care about, LIBEXTS determines
## what kinds of libraries are built (just static, or both static and
## shared)
## 
ifdef TEEM_SHEXT
  LIBEXTS = $(TEEM_SHEXT)
endif
LIBEXTS += a

#######################################
## Flags
##
ifneq (undefined,$(origin TEEM_LINK_SHARED))
  # If we ever have absolutify-this-path working, then having an 
  # absolute TEEM_DEST path is no longer a pre-requisite
  BIN_CFLAGS += $(SHARED_CFLAG) $(if $(TEEM_DEST),$(if $(SHARED_RPATH),$(SHARED_RPATH)$(LDEST),),)
else
  BIN_CFLAGS += $(STATIC_CFLAG)
endif

CFLAGS += $(OPT_CFLAG) $(ARCH_CFLAG)
LDFLAGS += $(ARCH_LDFLAG) $(SHARED_LDFLAG)

## The AIR_SIZE_T_FMT macro requires knowing TEEM_32BIT for all
## source files wishing to use it, which might as well be all teem
## source files.
##
CFLAGS += -DTEEM_32BIT=$(TEEM_32BIT)

## SGI's C pre-processor errors aren't fatal by default
##
CFLAGS += $(TEEM_CPP_ERROR_DIE)

## If using purify, set P to all the purify stuff that will prefix
## compile and link commands
## 
ifneq (undefined,$(origin TEEM_PURIFY))
  P = $(TEEM_PURIFY_CMD) \
    -inuse-at-exit=yes -add-suppression-files=$(PCACHE)/.purify
#    -always-use-cache-dir -cache-dir=$(PCACHE)
endif

#######################################
## "Functions"
## The bread and butter of how template.mk works.

## Each of these can be $(call)ed with a library name, as the one
## and only argument, in order to get a list of files or flags related to
## library.  
##
## {libs,hdrs}.inst(L): installed libs and headers for library L
## hdrs.dev(L): the local (original) copies of public and private headers
## tests.dev(L): library L tests for debugging, built in-place
## {libs,objs}.dev(L): object files for library L, and the libs used
##   when compiling the tests
##
libs.inst = $(foreach ext,$(LIBEXTS),$(LDEST)/lib$(1).$(ext))
hdrs.inst = $(addprefix $(IDEST)/teem/,$($(1).PUBLIC_HEADERS))
hdrs.dev = $(addprefix $(TEEM_SRC)/$(1)/,\
  $($(1).PUBLIC_HEADERS) $($(1).PRIVATE_HEADERS))
tests.dev = $(addprefix $(TEEM_SRC)/$(1)/,$($(1).TESTS))
libs.dev = $(foreach ext,$(LIBEXTS),$(ODEST)/lib$(1).$(ext))
objs.dev = $(addprefix $(ODEST)/,$($(1).OBJS))
hdrs.dev = $(addprefix $(TEEM_SRC)/$(1)/,\
  $($(1).PUBLIC_HEADERS) $($(1).PRIVATE_HEADERS))

## need(L): pseudo-recursive expansion of all the libraries which
## library L depends on, either directly or indirectly.  Because make
## doesn't allow recursive functions, and because I can't figure out
## how to do fixed-point determination in gmake, I'm only doing a few
## levels of prerequisite expansion. Each level is done by "dmnl"
## (discover more needed libs).  The number of levels is more than
## enough for the current teem; and adding more levels later is
## trivial.
## 
## Since we rely on $(sort) to remove redundancies, we need a way of
## putting the libraries back in dependency order (!= lexical order).
## So, we prefix the needed library names with [0..D] (via $(join)) to
## create NUM/LIBS (above), re-sort them, and then lose the prefix
## with $(notdir).  This is done by "deporder". On the link line,
## however, the ordering needs to be reversed, this is done by
## "linkorder".
## 
dmnl = $(sort $(foreach LIB,$(1),$(LIB) $($(LIB).NEED)))
deporder = $(notdir $(sort $(foreach LIB,$(1),\
  $(filter %/$(LIB),$(NUM/LIBS)))))
linkorder = $(notdir $(sort $(foreach LIB,$(1),\
  $(filter %/$(LIB),$(RNUM/LIBS)))))
need = $(call deporder,\
$(call dmnl,\
$(call dmnl,\
$(call dmnl,\
$(foreach LIB,$(1),$($(LIB).NEED))))))
meneed = $(call deporder,$(1) $(call need,$(1)))

## link(libs): "-lLIB" for all LIBs in libs, in correct link order
##
link = $(foreach LIB,$(call linkorder,$(1)),-l$(LIB))

## For XXX in: "ENDIAN", "QNANHIBIT", "DIO"
##
## TEEM_XXX is set in the architecture-specific makefile, and
## $(L).NEED_XXX is set in the Makefile for the library which needs
## that info.  Meanwhile, teemXxx.h in teem's top-level include
## directory contains C-preprocessor code to make sure that the
## variable has been set, and set to something
## reasonable. more.cflags(L) is the list of -D flags for all required
## XXX.
##
more.cflags = $(if $($(1).NEED_ENDIAN),-DTEEM_ENDIAN=$(TEEM_ENDIAN))\
 $(if $($(1).NEED_DIO),-DTEEM_DIO=$(TEEM_DIO))\
 $(if $($(1).NEED_QNANHIBIT),-DTEEM_QNANHIBIT=$(TEEM_QNANHIBIT)) \

## banner(L) is progress indication for compiling library L; see
## template.mk.  The double colon rules previously used to print this
## fatally confused parallel builds
##
banner = echo "--------------------" $(1) "--------------------"

#src2lib = $(notdir $(patsubst %/,%,$(dir $(1:.c=))))
#HDR2LIB = $(notdir $(patsubst %/,%,$(dir $(1:.h=))))
#STL2LIB = $(patsubst lib%,%,$(notdir $(1:.a=)))
#SHL2LIB = $(patsubst lib%,%,$(notdir $(1:.$(TEEM_SHEXT)=)))

#used.hdrs = $(foreach lib,$(1),$(call hdrs.inst,$(lib)))
#used.libs = $(foreach lib,$(1),$(call libs.inst,$(lib)))
#used = $(call used.libs,$(1)) $(call used.hdrs,$(1))

## related to external optional libraries.  Called "xtern"s to avoid
## similarities to library "ext"s above.
##
## wants.xtern(XTERN): returns XTERN if TEEM_<XTERN> has been set,
##   otherwise nothing
## xterns: list of all wanted XTERNs (a subset of $(XTERNS))
## for.xtern(libs,patt): for all xtern in xterns, for all libraries
##   in $(libs) that benefit from $(xtern),
##   replace XXX in $(patt) with $(xtern)
## evallist(list): evaluates each element of list as a variable
## xtern.Ipath(LIB): the external-specific -I flags needed for compiling
##   objects in library LIB
## xtern.Dflag(LIB): the -D flags turned on for library LIB in order to
##   enable the desired externals
## xtern.Lpath(libs): paths -Lblah for any wanted xterns whose base
##   teem dependency is within libs.  libs should probably be $($(L).meneed)
##   for some library L
## xtern.link(libs): -l link flags for any wanted externals whose base
##   teem dependency is within libs.
##
wants.xtern = $(if $(findstring undefined,$(origin TEEM_$(1))),,$(1))
xterns = $(foreach xtern,$(XTERNS),$(call wants.xtern,$(xtern)))
for.xtern = $(strip $(foreach xtern,$(xterns),$(foreach lib,$(1),\
  $(if $(filter $(xtern),$($(lib).XTERN)),\
  $(subst XXX,$(xtern),$(2))))))
evallist = $(foreach var,$(1),$($(var)))
xtern.Ipath = $(call evallist,$(call for.xtern,$(1),TEEM_XXX_IPATH))
xtern.Dflag = $(call for.xtern,$(1),-DTEEM_XXX=1)
xtern.Lpath = $(call evallist,$(call for.xtern,$(1),TEEM_XXX_LPATH))
xtern.link  = $(call evallist,$(call for.xtern,$(1),XXX.LINK))


#######################################

## Read in the makefiles for all the libraries, and the bins.  Run-away
## recursive inclusion is prevented by having set DEF_TARGETS. 
## Note: "include" is a directive, not a function, which eliminates
## the possibility of iterating through the libraries, reading the
## make file, and then setting variables based on what was just read.
##
## Lsave is used to preserve the value of L, in case we're being included
## from a lower-level makefile which set a value for L.  If we didn't put
## L back the way it was, L would have to be set twice in the lower level
## makefiles
##
INCLUDED = true
ifneq (,$(L))
  ifeq (undefined,$(origin Lsave))
    Lsave := $(L)
  endif
endif
include $(foreach LIB,$(LIBS),$(TEEM_SRC)/$(LIB)/GNUmakefile)
ifneq (,$(Lsave))
  L := $(Lsave)
  INCLUDED =
else
  # Lsave wasn't set, which means that make was invoked on this file,
  # and since the library make files do not include the bin makefile,
  # we do so now
  include $(TEEM_SRC)/bin/GNUmakefile
endif

## megalibs: libteem.a and libteem.$(TEEM_SHEXT)
##
megalibs : $(foreach ext,$(LIBEXTS),$(LDEST)/libteem.$(ext))
$(LDEST)/libteem.a : $(foreach lib,$(LIBS),$(call objs.dev,$(lib)))
	$(AR) $(ARFLAGS) $@ $^
ifdef TEEM_SHEXT
$(LDEST)/libteem.$(TEEM_SHEXT) : $(foreach lib,$(LIBS),$(call objs.dev,$(lib)))
	$(LD) -o $@ \
          $(if $(TEEM_DEST),$(if $(SHARED_LINK_NAME),$(SHARED_LINK_NAME)$@,),) \
          $(LDFLAGS) $(LPATH) $^ \
          $(foreach X,$(xterns),$(TEEM_$(X)_LPATH)) \
          $(foreach X,$(xterns),$($(X).LINK))
endif
unmegalibs:
	$(RM) $(foreach ext,$(LIBEXTS),$(LDEST)/libteem.$(ext))

include $(TEEM_SRC)/make/win32.mk
