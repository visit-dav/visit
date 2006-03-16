#                            Copyright 2000 - 2006
#                 The Regents of the University of California.
#                             All rights reserved.
#
# This work  was produced at the  University of California, Lawrence Livermore
# National Laboratory  (UC LLNL)  under contract no.   W-7405-ENG-48 (Contract
# 48) between  the U.S.  Department  of Energy (DOE)  and  The Regents  of the
# University  of   California (University)   for   the operation  of UC  LLNL.
# Copyright  is reserved  to     the University for   purposes  of  controlled
# dissemination,   commercialization   through   formal licensing,   or  other
# disposition  under  terms  of Contract   48; DOE  policies, regulations  and
# orders;  and U.S.   statutes.  The  rights of   the  Federal Government  are
# reserved under  Contract 48 subject to the  restrictions agreed  upon by DOE
# and University.
#
#                                 DISCLAIMER
#
# This software was prepared as an  account of work sponsored  by an agency of
# the United  States Government. Neither the United  States Government nor the
# University of California  nor any  of their  employees, makes any  warranty,
# express or  implied,  or assumes  any  liability  or responsiblity for   the
# accuracy, completeness,  or   usefullness of   any  information,  apparatus,
# product, or process disclosed, or represents that its use would not infringe
# privately   owned  rights. Reference    herein to   any specific  commercial
# products, process, or service   by trade name, trademark,   manufacturer, or
# otherwise,  does not    necessarily  constitute or    imply its endorsement,
# recommendation,  or  favoring by    the  United  States Government  or   the
# University of California. The views and opinions of authors expressed herein
# do not necessarily state or reflect those of the United States Government or
# the University of  California,  and shall not  be  used for  advertising  or
# product endorsement purposes.
#
##############################################################################

# set TOPDIR to point to where vist is installed
# The standard place is  /usr/gapps/visit/current/<plateform> where plateform
# should be replaced with the type of plateform you are running on, eg.
# linux-intel
TOPDIR=../..

##                                                      -*- makefile -*-
## Variables
##

CC=gcc
SIMSER=simulation
SIMPAR=psimulation
SIMLIB=libsim
SIMLIBSHARED=$(SIMLIB).so
SIMLIBSTATIC=$(SIMLIB).a
SIMSRC=sim.c simconnection.c
SIMLIBSCR=VisItControlInterface_V1.c UI_Connection.c SimHelperFunc.c
SIMSEROBJ=$(SIMSRC:.c=.o)
SIMPAROBJ=$(SIMSRC:.c=_par.o)
SIMLIBOBJ=$(SIMLIBSCR:.c=.o)

SIMFSER=simf
SIMFSRC=simf.f
SIMFSEROBJ=$(SIMFSRC:.f=.o)

CPPFLAGS= -I. 
F77FLAGS= -I$(TOPDIR)/include/visit
LDFLAGS=-L. -L$(TOPDIR)/archives
SERLIBS=
PARLIBS=

CFLAGS=-g -I. -I$(TOPDIR)/include/visit -Wimplicit-function-declaration
#PARLIBS=-lmpich

LIBS=-ldl -lpthread -lm -lsim
FLIBS=-ldl -lpthread -lm -lsimf -lsim

# override for suns:
#PARLIBS=-L/usr/gapps/mpich/1.2.4/SunOS/serial/default/optim/lib -lmpich -lrt -lthread
#CFLAGS=-g -I. -I../include/visit -Wimplicit-function-declaration -I/usr/gapps/mpich/1.2.4/SunOS/serial/default/optim/include

all:  $(SIMSER) $(SIMPAR) $(SIMFSER)

$(SIMSER): $(SIMSEROBJ)
	$(CC) -Wl,--export-dynamic $(SIMSEROBJ) $(LDFLAGS) $(LIBS)    -o $@

$(SIMPAR): $(SIMPAROBJ)
	$(CC) -Wl,--export-dynamic $(LDFLAGS) $(SIMPAROBJ) $(LIBS)  -o $@

$(SIMSER).pure: $(SIMSEROBJ)
	purify $(LD) $(LDFLAGS) $(SIMSEROBJ) $(LIBS) -o $@

$(SIMFSER): $(SIMFSEROBJ)
	$(F77) -Wl,--export-dynamic $(SIMFSEROBJ) $(LDFLAGS) $(FLIBS) $(SERLIBS) -o $@

clean:
	rm -f $(SIMSEROBJ) $(SIMSER) $(SIMSER).pure
	rm -f $(SIMPAROBJ) $(SIMPAR)
	rm -f $(SIMFSEROBJ) $(SIMFSER)


##                                                      -*- makefile -*-
## The following targets come from `include/targets'
##
install: install_lib install_bin

##
## Targets `install_lib' and `install_bin' are specified here with
## no dependencies and no commands so that they don't have to be
## defined in every Makefile.in.
##
install_lib:

install_bin:

##
## Same as `clean' and then remove Emacs backup files, .depend files,
## core dumps, Makefiles, and any files listed in DISTCLEAN_FILES.  Then
## go to the temporary include directory and remove those copies of the
## public header files.
##
## The Makefile line is separate from the *~, .depend and core line
## because some rm's won't get to Makefile if there are no backup files,
## .depend files, or core files.
##
## The DISTCLEAN_FILES variable is something that only exists in some
## Makefiles.  It is used to specify additional files that need to be
## removed when doing a distclean.  Since distclean is used to get rid of
## anything that can be automatically generated, the files that are in
## DISTCLEAN_FILES are things that are generated through *.in files by
## configure.
##
distclean: clean
	-$(RM) $(DISTCLEAN_FILES) *~ .depend core
	-$(RM) Makefile

##
## Suffixes
##
.SUFFIXES: .C _mds.o _par.o _ogl.o _mesa.o _par_ogl.o _par_mesa.o

##
## We separate C compiler flags from C preprocessor flags and most
## make's don't have the correct implicit rules, so we redefine them
## here.  If you don't like seeing the `ar' and `rm' commands when
## you run make, just add a `@' to the beginning of the line.
##
.c.a:
	false

##
## We separate C++ compiler flags from C preprocessor flags and most
## make's don't have the correct implicit rules, so we redefine them
## here.  If you don't like seeing the `ar' and `rm' commands when
## you run make, just add a `@' to the beginning of the line.
##
## Jeremy Meredith, Wed Jul  7 16:13:53 PDT 2004
## Removed the "_ser.o" build line because it was inefficient.
## Added a "_mds.o" build line to reduce the number of libraries
## needed by the mdserver by using preprocessor wrappers in plugins.
##
.C.a:
	false

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

.c_mds.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMDSERVER -c $< -o $@

.c_par.o:
	$(CC) $(CFLAGS) $(CPPFLAGS)  -c $< -o $@

# There cannot be a tab before the OS_RULE line.  The rule contains its own
# tab character.
.C.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	: Linux 2.6.9-23chaos

.C_mds.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -DMDSERVER -c $< -o $@
	: Linux 2.6.9-23chaos

.C_par.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS)  -c $< -o $@
	: Linux 2.6.9-23chaos

.C_ogl.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	: Linux 2.6.9-23chaos

.C_mesa.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(MESA_INCLUDE) -c $< -o $@
	: Linux 2.6.9-23chaos

.C_par_ogl.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS)  -c $< -o $@
	: Linux 2.6.9-23chaos

.C_par_mesa.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS)  $(MESA_INCLUDE) -c $< -o $@
	: Linux 2.6.9-23chaos

.f.o:
	$(F77) $(F77FLAGS) -c $<


$(IDSO) noidso: $(IOBJ)
	$(CXX) -shared -o $@ $(LDFLAGS) $(IOBJ) 
	sleep 1

$(GDSO) nogdso: $(GOBJ) $(MOCOBJ)
	$(CXX) -shared -o $@ $(LDFLAGS) $(GOBJ) $(MOCOBJ) $(SHLIB_FORCED) 
	sleep 1

$(MDSO) nomdso: $(MOBJ) $(MOCOBJ)
	$(CXX) -shared -o $@ $(LDFLAGS) $(MOBJ) $(SHLIB_FORCED) 
	sleep 1

$(SDSO) nosdso: $(SOBJ)
	$(CXX) -shared -o $@ $(LDFLAGS) $(CXXFLAGS) $(SOBJ) $(SHLIB_FORCED) 
	sleep 1

$(VDSO) novdso: $(VOBJ)
	$(CXX) -shared -o $@ $(LDFLAGS) $(VOBJ) $(SHLIB_FORCED) 
	sleep 1

$(ESERDSO) noedso_ser: $(ESEROBJ)
	$(CXX) -shared -o $@ $(LDFLAGS) $(ESEROBJ) $(SHLIB_FORCED) 
	sleep 1

$(EPARDSO) noedso_par: $(EPAROBJ)
	$(CXX) -shared -o $@ $(LDFLAGS) $(EPAROBJ) $(SHLIB_FORCED)   
	sleep 1

## Brad Whitlock, Wed Aug 14 16:26:55 PST 2002
## I added the Java target.
java:
	$(JAVAC) $(JAVAFLAGS) $(JAVAPLUGINFLAGS) $(JAVASRC)

## ------------------------------------------------------------------
## Maintaining source dependencies... either GNU, generic, or none...
## ------------------------------------------------------------------


##                                                      -*- makefile -*-
## Automatic dependencies...  We keep a list of dependencies in `.depend'
## for each of the source files on which it depends.  When one of the source
## files is modified we remove its record from .depend and regenerate the
## dependencies, tacking them onto the end of .depend.  By including the
## .depend file into the makefile, we're telling make that the makefile
## depends on the dependency list in the .depend file.
##
## This is as fast as the `.d' method described in the GNU make manual
## for automatic dependencies, but has the added advantage that all
## dependencies are stored in one place.  The advantage over the
## `makedepend' program is that only those files that are out of date
## have dependency information rebuilt, and the Makefile is not modified.
##
## Modifications:
##    Jeremy Meredith, Thu May  9 11:58:51 PDT 2002
##    1) Made it work with .C, not .c files
##    2) Made it work correctly with subdirectories, e.g. "dir/file.C"
##
.PRECIOUS: .depend
dep depend: .depend

.depend: $(SRC)
	@touch .depend
	@for dep in $? dummy; do \
	   if [ $$dep != "dummy" ]; then \
	      echo Building dependencies for $$dep ;\
	      obj=`echo $$dep | sed 's/\.C/.o/'` ;\
	      sed "\|^$$obj|,\|[^\\]$$|d" <$@ > $@- && mv $@- $@ ;\
	      dir=`dirname $$obj`;\
	      if [ $$dir != "." ]; then \
	          printf "$$dir/" >> $@;\
              fi;\
	      $(CXX) -M -MG $(CXXFLAGS) $(CPPFLAGS) $$dep >>$@;\
	   fi;\
	done

-include .depend


$(SUBDIRS:=_obj) nosubdirs:
	$(MAKE)  $($(@:_obj=_src):.C=.o)
	if [ -n "" ]; then $(MAKE) $($(@:_obj=_src):.C=_par.o); fi

## We need different ones for the serial and parallel versions.

$(LIBSHAREDSER) nolibsharedser: $(SEROBJ)
	$(CXX) -shared -o $(LIBSHAREDTEMPSER) $(LDFLAGS) $(SEROBJ) 
	mv $(LIBSHAREDTEMPSER) $@

$(LIBSHAREDPAR) nolibsharedpar: $(PAROBJ)
	$(CXX) -shared -o $(LIBSHAREDTEMPPAR) $(LDFLAGS) $(PAROBJ)  
	mv $(LIBSHAREDTEMPPAR) $@
