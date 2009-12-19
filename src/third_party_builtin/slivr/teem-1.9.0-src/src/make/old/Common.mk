#
# The contents of this file are subject to the University of Utah Public
# License (the "License"); you may not use this file except in
# compliance with the License.
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
# the License for the specific language governing rights and limitations
# under the License.
#
# The Original Source Code is "teem", released March 23, 2001.
#  
# The Original Source Code was developed by the University of Utah.
# Portions created by UNIVERSITY are Copyright (C) 2001, 1998 University
# of Utah. All Rights Reserved.
#
#
#
# Common makefile variables and rules for all teem libraries
#
# This is included by all the individual library/utility Makefiles,
# at the *end* of the Makefile.
#
#

#
# all the architectures currently supported
#
KNOWN_ARCH = irix6.n32 irix6.64 linux cygwin

#
# there is no default architecture
#
ifndef TEEM_ARCH
  $(warning *)
  $(warning *)
  $(warning *    Env variable TEEM_ARCH not set.)
  $(warning *    Possible settings currently supported:)
  $(warning *    $(KNOWN_ARCH))
  $(warning *)
  $(warning *)
  $(error Make quitting)
endif

#
# the architecture name may have two parts, ARCH and SUBARCH,
# seperated by one period
#
ARCH = $(basename $(TEEM_ARCH))
SUBARCH = $(patsubst .%,%,$(suffix $(TEEM_ARCH)))

#
# verify that we can recognize the architecture setting
#
ifeq (,$(strip $(findstring $(TEEM_ARCH),$(KNOWN_ARCH))))
  $(warning *)
  $(warning *)
  $(warning *    Env variable TEEM_ARCH = "$(TEEM_ARCH)" unknown)
  $(warning *    Possible settings currently supported:)
  $(warning *    $(KNOWN_ARCH))
  $(warning *)
  $(warning *)
  $(error Make quitting)
endif

#
# the root of the teem tree, as seen from the library subdirectories
# of the the "src" directory
#
ifndef TEEM_ROOT
  PREFIX = ../..
else
  PREFIX = $(TEEM_ROOT)
endif

#
# the basic programs. Flags for CC and LD are set later.  As usual,
# if these variables are defined with command-line settings, those
# will override these.
#

#
# set some directory related variables
#
IDEST = $(PREFIX)/include
LDEST = $(PREFIX)/$(TEEM_ARCH)/lib
BDEST = $(PREFIX)/$(TEEM_ARCH)/bin
ODEST = $(PREFIX)/$(TEEM_ARCH)/obj
IPATH = -I. -I$(IDEST)
LPATH = -L$(LDEST)

###
### effect the architecture-dependent settings by reading through the
### file specific to the chosen architecture
###
include ../make/$(ARCH).mk
###
###
###

ifdef LIB
  AR_OBJ_PREF = $(ODEST)/$(LIB)/
  AR_OBJS = $(addprefix $(AR_OBJ_PREF),$(LIBOBJS))
  ifeq ($(PIC_MATTERS),true)
    SH_OBJ_PREF = $(ODEST)/PIC/$(LIB)/
    SH_OBJS = $(addprefix $(SH_OBJ_PREF),$(LIBOBJS))
    RM_SH_OBJS = $(SH_OBJS)
  else
    SH_OBJ_PREF = $(AR_OBJ_PREF)
    SH_OBJS = $(AR_OBJS)
  endif
  _LIB.A = lib$(LIB).a
  _LIB.S = lib$(LIB).$(SHEXT)
  LIB.A = $(AR_OBJ_PREF)$(_LIB.A)
  LIB.S = $(SH_OBJ_PREF)$(_LIB.S)
endif

#
# flags.
#
ifdef $(TEEM_LINK_SHARED)
  BIN_CFLAGS += $(SHARED_CFLAG)
  ifdef LIB
    THISLIB = $(LIB.S)
    THISLIB_LPATH = -L$(SH_OBJ_PREF)
  endif
else
  BIN_CFLAGS += $(STATIC_CFLAG)
  ifdef LIB
    THISLIB = $(LIB.A)
    THISLIB_LPATH = -L$(AR_OBJ_PREF)
  endif
endif  
ifeq ($(PIC_MATTERS),true)
  ifdef $(TEEM_LINK_SHARED)
    BIN_CFLAGS += $(PIC_CFLAG)
  else
    BIN_CFLAGS += $(NONPIC_CFLAG)
  endif
endif
CFLAGS += $(OPT_CFLAG) $(ARCH_CFLAG)
LDFLAGS += $(ARCH_LDFLAG) $(SHARED_LDFLAG)
ARFLAGS = ru

#
# the rules
#

#all: $(AR_OBJS) $(LIB.A) $(SH_OBJS) $(LIB.S) $(TESTBINS) $(BINS)
all: $(AR_OBJS) $(LIB.A) $(SH_OBJS) $(LIB.S)

testbins: $(TESTBINS)

bins: $(BINS)

# even if AR_OBJS = SH_OBJS, this is a harmless repetition of the
# same dependencies
$(AR_OBJS): $(HEADERS) $(PRIVHEADERS)
$(SH_OBJS): $(HEADERS) $(PRIVHEADERS)

# this is a little tricky.  If PIC_MATTERS is false, then the targets
# of the next two rules are identical.  Normally it is erroneous to
# have two seperate commands for the same targets, gnu make permits it
# as a concession to the defacto standard of previous make
# implementations (see printed manual, pg. 27).  This can be more
# conscientiously handled by double-colon rules, even though this
# isn't a good example of why double-colon rules are needed.  Since we
# want the rule for non-pic to be done "by default", than that rule
# goes second.
$(SH_OBJ_PREF)%.o:: %.c
	$(CC) $(CFLAGS) $(PIC_CFLAG) $(IPATH) -c $< -o $@
$(AR_OBJ_PREF)%.o:: %.c
	$(CC) $(CFLAGS) $(NONPIC_CFLAG) $(IPATH) -c $< -o $@

# the libraries are dependent on the respective object files
$(LIB.A): $(AR_OBJS)
	$(AR) $(ARFLAGS) $(LIB.A) $(AR_OBJS)
$(LIB.S): $(SH_OBJS)
	$(LD) -o $(LIB.S) $(LDFLAGS) $(SH_OBJS) 

# we have a special rule for binaries which are in a subdirectory
# called "test"- we depend on the library file $(THISLIB), which will
# be defined ifdef $(LIB), and will be set to lib$(LIB).a or
# lib$(LIB).$(SHEXT), according to $(TEEM_LINK_SHARED).  Also we link
# against that library (with $(THISLIB_LPATH) preceeding $(LPATH)).
# This way, the test binaries can be compiled without having to
# install the library being tested, and the style of linking is still
# controlled by $(TEEM_LINK_SHARED).
test/%: test/%.c $(THISLIB)
	$(CC) $(CFLAGS) $(BIN_CFLAGS) $(IPATH) -o $@ $< \
	$(THISLIB_LPATH) $(LPATH) $(BINLIBS)

#%: %.c
#	$(CC) $(CFLAGS) $(BIN_CFLAGS) $(IPATH) -o $@ $< $(LIB.A) \
#	$(LPATH) $(BINLIBS)

install: $(LIB.A) $(LIB.S) $(BINS)
	$(if $(HEADERS), $(INSTALL) -m 644 $(HEADERS) $(IDEST))
	$(if $(LIB), $(INSTALL) -m 755 $(LIB.A) $(LDEST))
	$(if $(LIB), $(INSTALL) -m 755 $(LIB.S) $(LDEST))
	$(if $(BINS), $(INSTALL) -m 755 $(BINS) $(BDEST))

clean:

	$(RM) $(AR_OBJS) $(LIB.A) $(RM_SH_OBJS) $(LIB.S) $(TESTBINS) $(BINS)

uninstall: clean
	$(if $(HEADERS), $(RM) $(foreach h, $(HEADERS), $(IDEST)/$(h)))
	$(if $(LIB), $(RM) $(LDEST)/$(_LIB.A))
	$(if $(LIB), $(RM) $(LDEST)/$(_LIB.S))
	$(if $(BINS), $(RM) $(foreach b, $(BINS), $(BDEST)/$(b)))
