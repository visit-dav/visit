# $Id: Makefile,v 1.12 2009/04/30 00:55:15 rcook Exp $ 
# $File$
# paraDIS makefile
#
# Makefile for paraDIS library
#
DEBUG ?= -g
INSTALL_DIR ?= $(HOME)/$(SYS_TYPE)

ifndef SYS_TYPE
export SYS_TYPE = $(shell uname)
endif
CWD = $(shell pwd)
MF-INCLUDE ?= $(CWD)/mf-include
include $(MF-INCLUDE)/mf-include.$(SYS_TYPE)

SFILES = paradis.C paradis_c_interface.C paradisStreaming.C paradisTest.C analyzeParaDIS.C


INCLUDES =  -I$(CWD)/RC_c_lib -I$(CWD)/RC_cpp_lib
LDFLAGS+=-lstdc++
#ELECTRIC_FENCE = libefence.a -lpthread
#CXX = purify g++-3.2.3
#CC = purify gcc-3.2.3

#USER_CFLAGS= -g
USER_CXXFLAGS+=$(INCLUDES) -DNO_SETBUF 

PARADISLIB = $(CWD)/$(SYS_TYPE)/lib/libparadis.a
RCCLIB=$(INSTALL_DIR)/lib/librcc.a 
RCCPPLIB=$(INSTALL_DIR)/lib/librccpp.a 
TARGETS = $(CWD)/$(SYS_TYPE)/bin/paradisTest $(CWD)/$(SYS_TYPE)/bin/paradisStreamingTest $(CWD)/$(SYS_TYPE)/bin/analyzeParaDIS

all: $(TARGETS)

debug:
	DEBUG=-g $(MAKE) all

install: all
	mkdir -p $(INSTALL_DIR)/bin && cp $(TARGETS) $(INSTALL_DIR)/bin


libparadis: $(PARADISLIB) 

$(SYS_TYPE)/paradis.o: paradis.C
	$(CXX)  $(CXXFLAGS) -c -o $@  $<
	@echo DONE WITH $@

$(SYS_TYPE)/paradisStreaming.o: paradisStreaming.C
	$(CXX)  $(CXXFLAGS) -c -o $@  $<
	@echo DONE WITH $@

$(SYS_TYPE)/paradis_c_interface.o: paradis_c_interface.C
	$(CXX)  $(CXXFLAGS) -c -o $@  $<
	@echo DONE WITH $@


$(PARADISLIB): $(RCCLIB) $(RCCPPLIB) $(SYS_TYPE)/paradis_c_interface.o  $(SYS_TYPE)/paradis.o $(SYS_TYPE)/paradisStreaming.o 
	[ -d $(SYS_TYPE)/lib/rclibobjs ] ||  mkdir -p $(SYS_TYPE)/lib/rclibobjs
	cd $(SYS_TYPE)/lib/rclibobjs && ar -x $(RCCPPLIB) &&  ar -x $(RCCLIB)
#	ar -rc $@ $(SYS_TYPE)/paradis_c_interface.o  $(SYS_TYPE)/paradis.o $(RCCLIB) $(RCCPPLIB)
	ar -rc $@ $(SYS_TYPE)/paradisStreaming.o $(SYS_TYPE)/paradis_c_interface.o  $(SYS_TYPE)/paradis.o $(SYS_TYPE)/lib/rclibobjs/*.o
	@echo DONE WITH $@

$(CWD)/$(SYS_TYPE)/bin/paradisTest: $(SYS_TYPE)/paradisTest.o $(PARADISLIB)  $(RCCLIB)
	[ -d $(SYS_TYPE)/bin ] || mkdir -p $(SYS_TYPE)/bin
	$(LD) -o $@ $(LDFLAGS)  $^ $(ELECTRIC_FENCE)

$(CWD)/$(SYS_TYPE)/bin/analyzeParaDIS: $(SYS_TYPE)/analyzeParaDIS.o $(PARADISLIB)  $(RCCLIB)
	[ -d $(SYS_TYPE)/bin ] || mkdir -p $(SYS_TYPE)/bin
	$(LD) -o $@ $(LDFLAGS)  $^ $(ELECTRIC_FENCE)


$(CWD)/$(SYS_TYPE)/bin/paradisStreamingTest: $(SYS_TYPE)/paradisStreamingTest.o $(PARADISLIB)  $(RCCLIB)
	[ -d $(SYS_TYPE)/bin ] || mkdir -p $(SYS_TYPE)/bin
	$(LD) -o $@ $(LDFLAGS)  $^ $(ELECTRIC_FENCE)

.DUMMY: 

$(RCCLIB): .DUMMY
	cd RC_c_lib;  $(MAKE) depend; $(MAKE) install

$(RCCPPLIB): .DUMMY
	cd RC_cpp_lib;  $(MAKE) depend; $(MAKE) install


clean: undepend
	rm -rf $(PROGNAME) makeworms *.o $(SYS_TYPE)/* *~ $(SYS_TYPE)/lib/*o $(SYS_TYPE)/lib/*a 

realclean: clean
	cd RC_cpp_lib;  $(MAKE) undepend; $(MAKE) clean;
	cd RC_c_lib;  $(MAKE) undepend; $(MAKE) clean;

undepend: 
	makedepend -Y $(SFILES)
	for dir in `pwd`/RC_c_lib `pwd`/RC_cpp_lib ; do cd $$dir; $(MAKE) undepend; done

# DO NOT DELETE

paradis.o: paradis.h
paradis_c_interface.o: paradis.h paradis_c_interface.h
paradisStreaming.o: paradisStreaming.h paradis.h
paradisTest.o: paradis.h RC_c_lib/debugutil.h RC_c_lib/args.h
analyzeParaDIS.o: paradis.h RC_c_lib/args.h
