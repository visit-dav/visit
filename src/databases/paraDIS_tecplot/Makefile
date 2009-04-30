##
## Programs and options...
##
include /usr/gapps/visit/1.11.2/linux-x86_64/include/make-variables
TOPDIR=/usr/gapps/visit/1.11.2/linux-x86_64


##
## Libraries and includes...
##
VTK_INCLUDE= \
  -I/usr/gapps/visit/1.11.2/linux-x86_64/include/vtk \
  -I/usr/gapps/visit/1.11.2/linux-x86_64/include/vtk/Common \
  -I/usr/gapps/visit/1.11.2/linux-x86_64/include/vtk/Filtering \
  -I/usr/gapps/visit/1.11.2/linux-x86_64/include/vtk/Graphics \
  -I/usr/gapps/visit/1.11.2/linux-x86_64/include/vtk/Hybrid \
  -I/usr/gapps/visit/1.11.2/linux-x86_64/include/vtk/IO \
  -I/usr/gapps/visit/1.11.2/linux-x86_64/include/vtk/Imaging \
  -I/usr/gapps/visit/1.11.2/linux-x86_64/include/vtk/Rendering \
  -I/usr/gapps/visit/1.11.2/linux-x86_64/include/vtk/Utilities
CXXFLAGS=$(CXXFLAGSORIG) -IRC_c_lib -IRC_cpp_lib
CPPFLAGS=$(CPPFLAGSORIG) $(VTK_INCLUDE) -I. -I/usr/gapps/visit/1.11.2/linux-x86_64/include -I/usr/gapps/visit/1.11.2/linux-x86_64/include/visit $(PY_CXXFLAGS)
LDFLAGS=$(LDFLAGSORIG) $(PLUGIN_LDFLAGS) -L/g/g0/rcook/.visit/linux-x86_64/plugins/databases

##
## Files...
##
PLUGINDIR=databases
PLUGINNAME=paraDIS_tecplotDatabase

WIDGETS=
VIEWERWIDGETS=
ISRC=paraDIS_tecplotPluginInfo.C
COMMONSRC=paraDIS_tecplotPluginInfo.C paraDIS_tecplotCommonPluginInfo.C
MSRC=paraDIS_tecplotMDServerPluginInfo.C avtparaDIS_tecplotFileFormat.C RC_c_lib/args.c RC_c_lib/debugutil.c RC_c_lib/signals.c RC_c_lib/fileutils.c RC_c_lib/inventor.c RC_cpp_lib/RCDebugStream.C RC_cpp_lib/Prefs.C RC_cpp_lib/RangeList.C RC_cpp_lib/test.C RC_cpp_lib/timer.C RC_cpp_lib/pathutil.C
ESRC=paraDIS_tecplotEnginePluginInfo.C avtparaDIS_tecplotFileFormat.C RC_c_lib/args.c RC_c_lib/debugutil.c RC_c_lib/signals.c RC_c_lib/fileutils.c RC_c_lib/inventor.c RC_cpp_lib/RCDebugStream.C RC_cpp_lib/Prefs.C RC_cpp_lib/RangeList.C RC_cpp_lib/test.C RC_cpp_lib/timer.C RC_cpp_lib/pathutil.C


SRC=$(ISRC) $(COMMONSRC) $(MSRC) $(ESRC)

SHLIB_FORCED=

ELIBS_FOR_MACOSX_PREBINDING=$(ZLIB_LIB) $(GLEW_LIBS) $(MESA_LIBS) $(GL_LIBS)
MLIBS_FOR_MACOSX_PREBINDING=$(GLEW_LIBS) $(ZLIB_LIB) $(MESA_LIBS) $(GL_LIBS)
ILIBS=
MLIBS=-lpipeline_ser -ldatabase_ser -lmir_ser -lavtmath_ser -ldbatts -lavtexceptions -lstate -lmisc -lcomm -lexpr -lparser -lutility -lplugin -lvisit_vtk -llightweight_visit_vtk  $(MLIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS)
ESERLIBS=-lpipeline_ser -ldatabase_ser -lmir_ser -lavtmath_ser -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lexpr -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk $(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS)
EPARLIBS=-lpipeline_par -ldatabase_par -lmir_par -lavtmath_par -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lexpr -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk $(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS) $(SHLIB_MPI_LIBS)

IDSO=/g/g0/rcook/.visit/linux-x86_64/plugins/databases/libIparaDIS_tecplotDatabase.so
MDSO=/g/g0/rcook/.visit/linux-x86_64/plugins/databases/libMparaDIS_tecplotDatabase.so
ESERDSO=/g/g0/rcook/.visit/linux-x86_64/plugins/databases/libEparaDIS_tecplotDatabase_ser.so
EPARDSO=/g/g0/rcook/.visit/linux-x86_64/plugins/databases/libEparaDIS_tecplotDatabase_par.so


DISTRIB=

##
## Derived objects
##
IOBJ_tmp=$(ISRC:.C=.o)
GOBJ_tmp=$(COMMONSRC:.C=.o) $(GSRC:.C=.o)
SOBJ_tmp=$(COMMONSRC:.C=.o) $(SSRC:.C=.o)
VOBJ_tmp=$(COMMONSRC:.C=.o) $(VSRC:.C=.o)
MOBJ_tmp=$(COMMONSRC:.C=.o) $(MSRC:.C=.o) $(MSPECIFICSRC:.C=_mds.o)
ESEROBJ_tmp=$(COMMONSRC:.C=.o) $(ESRC:.C=.o)     $(ESPECIFICSRC:.C=_eng.o)
EPAROBJ_tmp=$(COMMONSRC:.C=.o) $(ESRC:.C=_par.o) $(ESPECIFICSRC:.C=_par_eng.o)

IOBJ=$(IOBJ_tmp:.c=.o)
GOBJ=$(GOBJ_tmp:.c=.o)
SOBJ=$(SOBJ_tmp:.c=.o)
VOBJ=$(VOBJ_tmp:.c=.o)
MOBJ=$(MOBJ_tmp:.c=.o)
ESEROBJ=$(ESEROBJ_tmp:.c=.o)
EPAROBJ=$(EPAROBJ_tmp:.c=.o)

MOCSRC = $(WIDGETS:.h=_moc.C)
MOCOBJ = $(MOCSRC:.C=.o)

VIEWERMOCSRC = $(VIEWERWIDGETS:.h=_moc.C)
VIEWERMOCOBJ = $(VIEWERMOCSRC:.C=.o)

##
## Standard targets...
##
all: message $(IDSO) $(MDSERVERLIB) $(ENGINELIBSER) $(ENGINELIBPAR)

clean:
	$(RM) $(IOBJ) $(COMMONOBJ)
	$(RM) $(GOBJ) $(SOBJ) $(VOBJ) $(MOBJ) $(ESEROBJ) $(EPAROBJ)
	$(RM) $(MOCSRC) $(MOCOBJ) $(VIEWERMOCSRC) $(VIEWERMOCOBJ)
	$(RM) $(IDSO) $(GDSO) $(SDSO) $(VDSO) $(MDSO) $(ESERDSO) $(EPARDSO)
	$(RM) $(JAVAOBJ)

##
## Other targets...
##
message:
	@echo
	@echo "****************************************************************************"
	@echo "*** Building paraDIS simulation dump metafile Database Plugin"
	@echo "****************************************************************************"

##
## Automatic dependency stuff
##
include /usr/gapps/visit/1.11.2/linux-x86_64/include/make-targets
