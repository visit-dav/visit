##
## Programs and options...
##
include /g/g17/brugger/ddcMD_plugin/visit070702/src/include/make-variables
TOPDIR=/g/g17/brugger/ddcMD_plugin/visit070702/src


##
## Libraries and includes...
##
VTK_INCLUDE= \
  -I/g/g17/brugger/ddcMD_plugin/visit070702/src/include/vtk \
  -I/g/g17/brugger/ddcMD_plugin/visit070702/src/include/vtk/Common \
  -I/g/g17/brugger/ddcMD_plugin/visit070702/src/include/vtk/Filtering \
  -I/g/g17/brugger/ddcMD_plugin/visit070702/src/include/vtk/Graphics \
  -I/g/g17/brugger/ddcMD_plugin/visit070702/src/include/vtk/Hybrid \
  -I/g/g17/brugger/ddcMD_plugin/visit070702/src/include/vtk/IO \
  -I/g/g17/brugger/ddcMD_plugin/visit070702/src/include/vtk/Imaging \
  -I/g/g17/brugger/ddcMD_plugin/visit070702/src/include/vtk/Rendering \
  -I/g/g17/brugger/ddcMD_plugin/visit070702/src/include/vtk/Utilities
CXXFLAGS=$(CXXFLAGSORIG)
CPPFLAGS=$(CPPFLAGSORIG) $(VTK_INCLUDE) -I. -I/g/g17/brugger/ddcMD_plugin/visit070702/src/include -I/g/g17/brugger/ddcMD_plugin/visit070702/src/include/visit $(PY_CXXFLAGS)
LDFLAGS=$(LDFLAGSORIG)  -L/g/g17/brugger/ddcMD_plugin/visit070702/src/plugins/databases

##
## Files...
##
PLUGINDIR=databases
PLUGINNAME=DDCMDDatabase

WIDGETS=
VIEWERWIDGETS=
ISRC=DDCMDPluginInfo.C
COMMONSRC=DDCMDPluginInfo.C DDCMDCommonPluginInfo.C
MSRC=DDCMDMDServerPluginInfo.C
MSPECIFICSRC= avtDDCMDFileFormat.C object.C error.C
ESRC=DDCMDEnginePluginInfo.C avtDDCMDFileFormat.C object.C error.C


SRC=$(ISRC) $(COMMONSRC) $(MSRC) $(ESRC)

SHLIB_FORCED=

ELIBS_FOR_MACOSX_PREBINDING=$(BZIP2_LIBS) $(GLEW_LIBS) $(MESA_LIBS) $(GL_LIBS)
MLIBS_FOR_MACOSX_PREBINDING=$(GLEW_LIBS) $(BZIP2_LIBS) $(MESA_LIBS) $(GL_LIBS)
ILIBS=
MLIBS=-lpipeline_ser -ldatabase_ser -lmir_ser -lavtmath_ser -ldbatts -lavtexceptions -lstate -lmisc -lcomm -lexpr -lparser -lutility -lplugin -lvisit_vtk -llightweight_visit_vtk  -L/g/g17/brugger/ddcMD_plugin/visit070702/src/plugins/databases -lI$(PLUGINNAME) $(MLIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS)
ESERLIBS=-lpipeline_ser -ldatabase_ser -lmir_ser -lavtmath_ser -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lexpr -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk  -L/g/g17/brugger/ddcMD_plugin/visit070702/src/plugins/databases -lI$(PLUGINNAME) $(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS)
EPARLIBS=-lpipeline_par -ldatabase_par -lmir_par -lavtmath_par -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lexpr -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk  -L/g/g17/brugger/ddcMD_plugin/visit070702/src/plugins/databases -lI$(PLUGINNAME) $(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS) $(SHLIB_MPI_LIBS)

IDSO=/g/g17/brugger/ddcMD_plugin/visit070702/src/plugins/databases/libIDDCMDDatabase.so
MDSO=/g/g17/brugger/ddcMD_plugin/visit070702/src/plugins/databases/libMDDCMDDatabase.so
ESERDSO=/g/g17/brugger/ddcMD_plugin/visit070702/src/plugins/databases/libEDDCMDDatabase_ser.so
EPARDSO=/g/g17/brugger/ddcMD_plugin/visit070702/src/plugins/databases/libEDDCMDDatabase_par.so


DISTRIB=

##
## Derived objects
##
IOBJ=$(ISRC:.C=.o)
GOBJ=$(COMMONSRC:.C=.o) $(GSRC:.C=.o)
SOBJ=$(COMMONSRC:.C=.o) $(SSRC:.C=.o)
VOBJ=$(COMMONSRC:.C=.o) $(VSRC:.C=.o)
MOBJ=$(COMMONSRC:.C=.o) $(MSRC:.C=.o) $(MSPECIFICSRC:.C=_mds.o)
ESEROBJ=$(COMMONSRC:.C=.o) $(ESRC:.C=.o)     $(ESPECIFICSRC:.C=_eng.o)
EPAROBJ=$(COMMONSRC:.C=.o) $(ESRC:.C=_par.o) $(ESPECIFICSRC:.C=_par_eng.o)

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
	@echo "*** Building ddcMD files Database Plugin"
	@echo "****************************************************************************"

##
## Automatic dependency stuff
##
include /g/g17/brugger/ddcMD_plugin/visit070702/src/include/make-targets
