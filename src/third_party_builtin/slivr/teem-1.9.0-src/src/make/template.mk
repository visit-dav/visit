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

#### 
#### template.mk: Defines rules which have the same structure for each
#### library, but which refer to the specific consituents and
#### prerequisites of the library.  The rules defined here are
#### effectively templated on the name of the library.  The variable L
#### is assumed to contain the name of the library for which we're
#### creating rules; L is an immediate set by the library GNUmakefile.
####

## To minimize calls to "need", save this once.
##
$(L).need := $(call need,$(L))
$(L).meneed := $($(L).need) $(L)
#$(warning $(L).need = |$($(L).need)|, $(L).meneed = |$($(L).meneed)|)

## In a rule, the contexts of the target and the prerequisite are
## immediate, the contexts of the commands are deferred; there is no
## getting around this.  Thus, if the commands to satisfy $(L)/clean
## include $(RM) $(call OBJS.DEV,$(L)), then this will remove the
## object files for library $(L), but the value of $(L) that is used
## is the one in effect with WHEN THE COMMAND IS RUN, not the one when
## the rule was read by make.  Not useful.
##
## For all the "entry-point" targets, we enstate a pattern-specific
## immediate variable value _L.  This bridges the immediate and
## deferred stages by remembering the value of L at the time the rule
## was read, so that it can be used in the deferred context of the
## rule's commands.  Fortunately, the sementics of pattern-specific
## variables mean that the value of _L will be set the same when
## satisfying all prerequisites of $(L)/%, which is exactly what we
## want.
##
$(L)/% : _L := $(L)

## added this to enable _L when the target is not explicitly a library
## but, for example, an individually named binary.  This may subsume the
## the definition above, but it doesn't hurt to have both.
##
$(call libs.inst,$(L)) : _L := $(L)


## not sure why these are needed- version 3.78.1 on a solaris box
## definately needed the last one, while others didn't
$(call hdrs.inst,$(L)) : _L := $(L)
$(call objs.dev,$(L)) : _L := $(L)
$(call tests.dev,$(L)): _L := $(L)

## The prequisites of .$(L).usable are the .usable's of all our
## prerequisites, and our own libs and headers, if either:
## 1) any of the libs and headers are missing, or
## 2) a prerequisite .usable is newer than ours.
## Naming our libs and headers should effectively trigger an install.
##
## "usable" stuff nixed July 7, 2003, but "used" functions from main
## GNUmakefile are still used.  Yea.

## $(L)/install depends on usable prerequisite libraries and $(L)'s
## installed libs and headers.
##
$(L)/install : $(call used,$($(L).need)) \
  $(call libs.inst,$(L)) $(call hdrs.inst,$(L))

## $(L)/dev depends on usable prerequisites and $(L)'s local
## development builds of the libs and tests
##
$(L)/dev : $(call used,$($(L).need)) \
  $(call libs.dev,$(L)) $(call tests.dev,$(L))

## $(L)/clean undoes $(L)/dev.
##
$(L)/clean :
	$(RM) $(call objs.dev,$(_L)) $(call libs.dev,$(_L)) \
	  $(addsuffix $(DOTEXE),$(call tests.dev,$(_L)))
	$(if $(TEEM_LITTER),$(RM) $(TEEM_ROOT)/src/$(_L)/$(TEEM_LITTER))

## $(L)/clobber undoes $(L)/install.
##
$(L)/clobber : $(L)/clean
	$(RM) $(call libs.inst,$(_L)) $(call hdrs.inst,$(_L))

## The objects of a lib depend on the headers of the libraries we
## depend on, and on our own headers.
##
$(call objs.dev,$(L)) : $(call used.hdrs,$($(L).need)) $(call hdrs.dev,$(L))

## Development tests depend on usable prerequiste libraries (headers
## and libraries), and the development libs.  Dev tests also
## indirectly depend on our own headers (through the objects, below)
## and on the source file for the test (below)
##
$(call tests.dev,$(L)) : $(call used,$($(L).need)) \
  $(call hdrs.dev,$(L)) $(call libs.dev,$(L))

## How to create development static and shared libs (libs.dev) from
## the objects on which they depend.
##
$(ODEST)/lib$(L).a : $(call objs.dev,$(L))
	$(AR) $(ARFLAGS) $@ $^
	$(if $(RANLIB),$(RANLIB) $@,)
ifdef TEEM_SHEXT
$(ODEST)/lib$(L).$(TEEM_SHEXT) : $(call objs.dev,$(L))
	$(LD) -o $@ $(if $(TEEM_DEST),$(if $(SHARED_INSTALL_NAME), $(SHARED_INSTALL_NAME) $(TEEM_DEST)/lib/lib$(_L).$(TEEM_SHEXT),),) $(LDFLAGS) $(LPATH) $^ $(call link,$(call need,$(_L))) $(call xtern.Lpath,$($(_L).meneed)) $(call xtern.link,$($(_L).meneed))
endif

## comments on madness above:
## - if both TEEM_DEST and (architecture-specific) SHARED_INSTALL_NAME are set,
##   then add SHARED_INSTALL_NAME flag to the link command making the shared lib
## - ... $(LDFLAGS) $(LPATH) ...
## - all architectures: add "-l<lib>" for all <lib> that this lib relies on
## - all architectures: add -L<path> and -l<lib> for external libraries that 
##   this library could depend on

## maybebanner.(L)(obj) returns "echo ..." to show a library banner 
## progress indicator, but only if obj is the first object in $(L).OBJS.
## This mimics the behavior under the old recursive teem makefile.
##
maybebanner.$(L) = $(if $(filter $(notdir $(1:.c=.o)),\
$(word 1,$($(_L).OBJS))),$(call banner,$(_L)))

## How to compile a .o file. We're giving a pattern rule constrained
## to the objects we know we need to make for this library.  Or, we
## could use vpath to locate the sources in the library subdirectory,
## but why start cheating now.
##
$(call objs.dev,$(L)) : $(ODEST)/%.o : $(TEEM_SRC)/$(L)/%.c
	@$(call maybebanner.$(_L),$<)
	$(P) $(CC) $(CFLAGS) $(call more.cflags,$(_L)) $(IPATH) \
	  $(call xtern.Dflag,$(_L)) $(call xtern.Ipath,$(_L)) -c $< -o $@

## How to make development tests.  It doesn't actually matter in this
## case where the source files are, we just put the executable in the
## same place.
##
$(call tests.dev,$(L)) : % : %.c
	$(P) $(CC) $(CFLAGS) $(BIN_CFLAGS) \
	  $(call more.cflags,$(_L)) $(IPATH) -o $@ $< -L$(ODEST) -l$(_L) \
	  $(LPATH) $(call link,$($(_L).need)) \
	  $(call xtern.Lpath,$($(_L).meneed)) $(call xtern.link,$($(_L).meneed)) -lm

## How to install a libs (libs.inst), static and shared: This really
## should be in the top-level GNUmakefile, since there is really
## nothing library specific about this, but it looked funny to just
## have one rule there and all the rest in here.  In order to prevent
## redefining rules for the same target ($(LDEST)/%.a), we
## artificially make the rule specific to the library with a static
## pattern rule.
##
$(LDEST)/lib$(L).a : $(LDEST)/% : $(ODEST)/%
	$(CP) $< $@; $(CHMOD) 644 $@
	$(if $(RANLIB),$(RANLIB) $@,)
	$(if $(SIGH),$(SLEEP) $(SIGH); touch $@)
ifdef TEEM_SHEXT
  $(LDEST)/lib$(L).$(TEEM_SHEXT) : $(LDEST)/% : $(ODEST)/%
	$(CP) $< $@; $(CHMOD) 755 $@
	$(if $(SIGH),$(SLEEP) $(SIGH); touch $@)
endif

## How to install headers: another instance where vpath could simplify
## things, but why bother.
##
$(call hdrs.inst,$(L)) : $(IDEST)/teem/%.h : $(TEEM_SRC)/$(L)/%.h
	$(CP) $< $@; $(CHMOD) 644 $@
	$(if $(SIGH),$(SLEEP) $(SIGH); touch $@)
