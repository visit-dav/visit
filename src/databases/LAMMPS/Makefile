##
## Programs and options...
##
include /home/js9/visitdev/2008-12-29_volumeplot/trunk/src/include/make-variables
TOPDIR=/home/js9/visitdev/2008-12-29_volumeplot/trunk/src


##
## Libraries and includes...
##
VTK_INCLUDE= \
  -I/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/include/vtk \
  -I/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/include/vtk/Common \
  -I/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/include/vtk/Filtering \
  -I/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/include/vtk/Graphics \
  -I/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/include/vtk/Hybrid \
  -I/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/include/vtk/IO \
  -I/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/include/vtk/Imaging \
  -I/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/include/vtk/Rendering \
  -I/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/include/vtk/Utilities
CXXFLAGS=$(CXXFLAGSORIG)
CPPFLAGS=$(CPPFLAGSORIG) $(VTK_INCLUDE) -I. -I/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/include -I/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/include/visit $(PY_CXXFLAGS)
LDFLAGS=$(LDFLAGSORIG) $(PLUGIN_LDFLAGS) -L/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/plugins/databases

LIBG_QT_LIBS=$(QT_CORE_LIB) $(QT_GUI_LIB)
LIBV_QT_LIBS=$(QT_CORE_LIB) $(QT_GUI_LIB) $(QT_OPENGL_LIB)

##
## Files...
##
PLUGINDIR=databases
PLUGINNAME=LAMMPSDatabase

WIDGETS=
VIEWERWIDGETS=
ISRC=LAMMPSPluginInfo.C
COMMONSRC=LAMMPSPluginInfo.C LAMMPSCommonPluginInfo.C
MSRC=LAMMPSMDServerPluginInfo.C avtLAMMPSFileFormat.C avtLAMMPSStructureFileFormat.C avtLAMMPSDumpFileFormat.C
ESRC=LAMMPSEnginePluginInfo.C avtLAMMPSFileFormat.C avtLAMMPSStructureFileFormat.C avtLAMMPSDumpFileFormat.C


SRC=$(ISRC) $(COMMONSRC) $(MSRC) $(ESRC)

SHLIB_FORCED=

GUI_LIBS_FORCED=
ENGINE_LIBS_FORCED=
MDSERVER_LIBS_FORCED=
VIEWER_LIBS_FORCED=
ELIBS_FOR_MACOSX_PREBINDING=$(ZLIB_LIB) $(GLEW_LIBS) $(MESA_LIBS) $(GL_LIBS)
MLIBS_FOR_MACOSX_PREBINDING=$(GLEW_LIBS) $(ZLIB_LIB) $(MESA_LIBS) $(GL_LIBS)
ILIBS=
MLIBS=-lpipeline_ser -ldatabase_ser -lmir_ser -lavtmath_ser -ldbatts -lavtexceptions -lstate -lmisc -lcomm -lexpr -lparser -lutility -lplugin -lvisit_vtk -llightweight_visit_vtk  $(MLIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS)
ESERLIBS=-lpipeline_ser -ldatabase_ser -lmir_ser -lavtmath_ser -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lexpr -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk $(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS)
EPARLIBS=-lpipeline_par -ldatabase_par -lmir_par -lavtmath_par -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lexpr -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk $(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS) $(SHLIB_MPI_LIBS)

IDSO=/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/plugins/databases/libILAMMPSDatabase.so
MDSO=/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/plugins/databases/libMLAMMPSDatabase.so
ESERDSO=/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/plugins/databases/libELAMMPSDatabase_ser.so
EPARDSO=/home/js9/visitdev/2008-12-29_volumeplot/trunk/src/plugins/databases/libELAMMPSDatabase_par.so


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
	@echo "*** Building LAMMPS Database Plugin"
	@echo "****************************************************************************"

##
## Automatic dependency stuff
##
include /home/js9/visitdev/2008-12-29_volumeplot/trunk/src/include/make-targets
