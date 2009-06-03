##
## Programs and options...
##
include /apps/visit/branches/1.12RC_trunk/src/include/make-variables
TOPDIR=/apps/visit/branches/1.12RC_trunk/src


##
## Libraries and includes...
##
VTK_INCLUDE= \
  -I/apps/visit/branches/1.12RC_trunk/src/include/vtk \
  -I/apps/visit/branches/1.12RC_trunk/src/include/vtk/Common \
  -I/apps/visit/branches/1.12RC_trunk/src/include/vtk/Filtering \
  -I/apps/visit/branches/1.12RC_trunk/src/include/vtk/Graphics \
  -I/apps/visit/branches/1.12RC_trunk/src/include/vtk/Hybrid \
  -I/apps/visit/branches/1.12RC_trunk/src/include/vtk/IO \
  -I/apps/visit/branches/1.12RC_trunk/src/include/vtk/Imaging \
  -I/apps/visit/branches/1.12RC_trunk/src/include/vtk/Rendering \
  -I/apps/visit/branches/1.12RC_trunk/src/include/vtk/Utilities
CXXFLAGS=$(CXXFLAGSORIG) $(NETCDF_INCLUDE)
CPPFLAGS=$(CPPFLAGSORIG) $(VTK_INCLUDE) -I. -I/apps/visit/branches/1.12RC_trunk/src/include -I/apps/visit/branches/1.12RC_trunk/src/include/visit $(PY_CXXFLAGS)
LDFLAGS=$(LDFLAGSORIG) $(PLUGIN_LDFLAGS) -L/apps/visit/branches/1.12RC_trunk/src/plugins/databases

##
## Files...
##
PLUGINDIR=databases
PLUGINNAME=PlasmaStateDatabase

WIDGETS=
VIEWERWIDGETS=
ISRC=PlasmaStatePluginInfo.C
COMMONSRC=PlasmaStatePluginInfo.C PlasmaStateCommonPluginInfo.C
MSRC=PlasmaStateMDServerPluginInfo.C avtPlasmaStateFileFormat.C
ESRC=PlasmaStateEnginePluginInfo.C avtPlasmaStateFileFormat.C


SRC=$(ISRC) $(COMMONSRC) $(MSRC) $(ESRC)

SHLIB_FORCED=$(NETCDF_CXX_LIB) $(NETCDF_LIB) 

ELIBS_FOR_MACOSX_PREBINDING=$(ZLIB_LIB) $(GLEW_LIBS) $(MESA_LIBS) $(GL_LIBS)
MLIBS_FOR_MACOSX_PREBINDING=$(GLEW_LIBS) $(ZLIB_LIB) $(MESA_LIBS) $(GL_LIBS)
ILIBS=
MLIBS=-lpipeline_ser -ldatabase_ser -lmir_ser -lavtmath_ser -ldbatts -lavtexceptions -lstate -lmisc -lcomm -lexpr -lparser -lutility -lplugin -lvisit_vtk -llightweight_visit_vtk  $(MLIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS)
ESERLIBS=-lpipeline_ser -ldatabase_ser -lmir_ser -lavtmath_ser -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lexpr -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk $(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS)
EPARLIBS=-lpipeline_par -ldatabase_par -lmir_par -lavtmath_par -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lexpr -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk $(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS) $(SHLIB_MPI_LIBS)

IDSO=/apps/visit/branches/1.12RC_trunk/src/plugins/databases/libIPlasmaStateDatabase.so
MDSO=/apps/visit/branches/1.12RC_trunk/src/plugins/databases/libMPlasmaStateDatabase.so
ESERDSO=/apps/visit/branches/1.12RC_trunk/src/plugins/databases/libEPlasmaStateDatabase_ser.so
EPARDSO=/apps/visit/branches/1.12RC_trunk/src/plugins/databases/libEPlasmaStateDatabase_par.so


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
	@echo "*** Building PlasmaState Database Plugin"
	@echo "****************************************************************************"

##
## Automatic dependency stuff
##
include /apps/visit/branches/1.12RC_trunk/src/include/make-targets
