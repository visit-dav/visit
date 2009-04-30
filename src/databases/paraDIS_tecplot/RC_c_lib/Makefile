#
# Makefile for RC_c_lib
#
#
ifndef SYS_TYPE
export SYS_TYPE = $(shell uname)
endif
export MF-INCLUDE = $(shell cd ..; pwd)/mf-include
CWD = $(shell pwd)
export INSTALL_DIR ?= $(CWD)/$(SYS_TYPE)
$(warning INSTALL_DIR is $(INSTALL_DIR))
include $(MF-INCLUDE)/mf-include.$(SYS_TYPE)
SFILES=args.c debugutil.c signals.c fileutils.c inventor.c
#LDFLAGS=
#CXX = insure
#CC = insure
#CXX = insure
#export CC = purify gcc-3.2.3
#export CXX = purify g++-3.2.3

ARCHIVE=${INSTALL_DIR}/lib/librcc.a
TARGET=$(ARCHIVE)
LIBOBJS= ${SYS_TYPE}/debugutil.o ${SYS_TYPE}/args.o ${SYS_TYPE}/signals.o ${SYS_TYPE}/fileutils.o ${SYS_TYPE}/inventor.o 
EXEOBJ=
EXE=
$(warning starting paradislib makefile)

all:  library  $(INSTALL_DIR)/lib/gprof-helper.so

debuglib: 
	export DEBUG=-g; make library

library: $(ARCHIVE)


$(ARCHIVE): $(LIBOBJS)
	mkdir -p $(INSTALL_DIR)/lib $(INSTALL_DIR)/include
	ar -r $(ARCHIVE) $(LIBOBJS) 
	ranlib $(ARCHIVE)
	cp -p *h $(INSTALL_DIR)/include

$(INSTALL_DIR)/lib/gprof-helper.so: gprof-helper.c
	$(CC) -shared -nostdlib -fPIC $^ -o $@ -lpthread -ldl

clean:
	rm -rf $(SYS_TYPE)/*

uninstall:
	rm -f $(TARGET)

install: $(INSTALL_DIR)/lib/librcc.a

MAKEDIR= [ -d $(1) ] || mkdir -p $(1) 


clean-install: 
	make clean
	make uninstall
	make install

# DO NOT DELETE
