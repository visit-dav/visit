#
# Makefile for RC_cpp_lib 
#
# 
#
#

default: all

ifndef SYS_TYPE
export SYS_TYPE = $(shell uname)
endif
INSTALL_DIR ?= $(SYS_TYPE)
MF-INCLUDE = $(shell cd ..; pwd)/mf-include
include $(MF-INCLUDE)/mf-include.$(SYS_TYPE)

#LDFLAGS=
#CXX = insure
#CC = insure
#CXX = insure

TESTNAME = test_program
#LIBEFENCE = /g/g0/rcook/lib/redhat_7a_ia32/libefence.a
#CC = purify gcc-3.2.3
#CXX = purify g++-3.2.3
DBG_FLAG ?= -O2
DEBUG=$(DBG_FLAG) 
#DEBUG=$(OPT_FLAG)

#USER_CFLAGS= -g

USER_CXXFLAGS=$(INCLUDES) -DNO_SETBUF 

SFILES = RCDebugStream.C Prefs.C RangeList.C test.C timer.C pathutil.C



# separate idx lib target, so it doesn't get deleted during make clean...
ARCHIVE= $(SYS_TYPE)/lib/librccpp.a
TESTPROG=$(SYS_TYPE)/bin/testprogram
LIBOBJS=$(SYS_TYPE)/Prefs.o $(SYS_TYPE)/RCDebugStream.o $(SYS_TYPE)/RangeList.o $(SYS_TYPE)/timer.o

all: $(TESTPROG) $(ARCHIVE) 

install: $(INSTALL_DIR)/lib/librccpp.a

$(INSTALL_DIR)/lib/librccpp.a: $(ARCHIVE)
	mkdir -p $(INSTALL_DIR)/lib
	cp $(ARCHIVE) $@


MAKEDIR= [ -d $(1) ] || mkdir -p $(1) 


$(ARCHIVE):  $(LIBOBJS)
	mkdir -p $$(dirname $(ARCHIVE)) $(INSTALL_DIR)/include
	ar rsc $@ $(LIBOBJS)
	ranlib $@
	@echo $@ created
	cp -p *h $(INSTALL_DIR)/include

testprog: $(TESTPROG) 

$(TESTPROG): $(SYS_TYPE)/test.o $(ARCHIVE) 
	mkdir -p $$(dirname $(TESTPROG))
	$(CXX) -o $(TESTPROG) $(LIBEFENCE) $(LDLIBS) $(ARCHIVE) $^ 
	@echo $@ created

clean: undepend
	rm -rf $(SYS_TYPE)/*.o $(SYS_TYPE)/*.a

uninstall:
	rm -f $(READERDIR)/$(LIBNAME) $(READERDIR)/$(TESTLIBNAME) 

clean-install: clean install

$(SYS_TYPE)/boost_1_33_1/boost/version.hpp: 
	$(call MKDIR_SYSTYPE)
	cd $(SYS_TYPE); \
	tar -xvf ../boost_1_33_1.tar

$(SYS_TYPE)/boost_1_35_0/boost/version.hpp: 
	$(call MKDIR_SYSTYPE)
	cd $(SYS_TYPE); \
	tar -xvzf ../boost_1_35_0.tar.gz; 

libboost:  $(SYS_TYPE)/boost_1_35_0/boost/version.hpp 

# DO NOT DELETE
