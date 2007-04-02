##
## Programs and options...
##
include /data_vobs/VisIt/include/make-variables
TOPDIR=/data_vobs/VisIt


##
## Libraries and includes...
##
VTK_INCLUDE= \
  -I/data_vobs/VisIt/include/vtk \
  -I/data_vobs/VisIt/include/vtk/Common \
  -I/data_vobs/VisIt/include/vtk/Filtering \
  -I/data_vobs/VisIt/include/vtk/Graphics \
  -I/data_vobs/VisIt/include/vtk/Hybrid \
  -I/data_vobs/VisIt/include/vtk/IO \
  -I/data_vobs/VisIt/include/vtk/Imaging \
  -I/data_vobs/VisIt/include/vtk/Rendering \
  -I/data_vobs/VisIt/include/vtk/Utilities
MOC=/data_vobs/VisIt/bin/moc
CXXFLAGS=$(CXXFLAGSORIG)
CPPFLAGS=$(CPPFLAGSORIG) $(VTK_INCLUDE) -I. -I/data_vobs/VisIt/include -I/data_vobs/VisIt/include/visit
LDFLAGS=$(LDFLAGSORIG)  -L/data_vobs/VisIt/plugins/databases

##
## Files...
##
PLUGINDIR=databases
PLUGINNAME=MirandaDatabase

WIDGETS=
ISRC=MirandaPluginInfo.C
COMMONSRC=MirandaPluginInfo.C MirandaCommonPluginInfo.C avtMirandaOptions.C
MSRC=MirandaMDServerPluginInfo.C avtMirandaFileFormat.C
ESRC=MirandaEnginePluginInfo.C avtMirandaFileFormat.C


SRC=$(ISRC) $(COMMONSRC) $(MSRC) $(ESRC)

SHLIB_FORCED=

ELIBS_FOR_MACOSX_PREBINDING=$(BZIP2_LIBS)
MLIBS_FOR_MACOSX_PREBINDING=$(BZIP2_LIBS)
ILIBS=
MLIBS=-lpipeline_ser -ldatabase_ser -lmir_ser -lavtmath_ser -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lexpr -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk  -L/data_vobs/VisIt/plugins/databases -lI$(PLUGINNAME) $(MLIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS)
ESERLIBS=-lpipeline_ser -ldatabase_ser -lmir_ser -lavtmath_ser -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lexpr -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk  -L/data_vobs/VisIt/plugins/databases -lI$(PLUGINNAME) $(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS)
EPARLIBS=-lpipeline_par -ldatabase_par -lmir_par -lavtmath_par -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lexpr -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk  -L/data_vobs/VisIt/plugins/databases -lI$(PLUGINNAME) $(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS) $(SHLIB_MPI_LIBS)

IDSO=/data_vobs/VisIt/plugins/databases/libIMirandaDatabase.so
MDSO=/data_vobs/VisIt/plugins/databases/libMMirandaDatabase.so
ESERDSO=/data_vobs/VisIt/plugins/databases/libEMirandaDatabase_ser.so
EPARDSO=/data_vobs/VisIt/plugins/databases/libEMirandaDatabase_par.so


DISTRIB=

##
## Derived objects
##
IOBJ=$(ISRC:.C=.o)
GOBJ=$(COMMONSRC:.C=.o) $(GSRC:.C=.o)
SOBJ=$(COMMONSRC:.C=.o) $(SSRC:.C=.o)
VOBJ=$(COMMONSRC:.C=.o) $(VSRC:.C=.o)
MOBJ=$(COMMONSRC:.C=.o) $(MSRC:.C=.o) $(MSPECIFICSRC:.C=_mds.o)
ESEROBJ=$(COMMONSRC:.C=.o) $(ESRC:.C=.o)
EPAROBJ=$(COMMONSRC:.C=.o) $(ESRC:.C=_par.o)

MOCSRC = $(WIDGETS:.h=_moc.C)
MOCOBJ = $(MOCSRC:.C=.o)

##
## Standard targets...
##
all: message $(IDSO) $(MDSERVERLIB) $(ENGINELIBSER) $(ENGINELIBPAR)

clean:
	$(RM) $(IOBJ) $(COMMONOBJ)
	$(RM) $(GOBJ) $(SOBJ) $(VOBJ) $(MOBJ) $(ESEROBJ) $(EPAROBJ)
	$(RM) $(MOCSRC) $(MOCOBJ)
	$(RM) $(IDSO) $(GDSO) $(SDSO) $(VDSO) $(MDSO) $(ESERDSO) $(EPARDSO)
	$(RM) $(JAVAOBJ)

##
## Other targets...
##
message:
	@echo
	@echo "****************************************************************************"
	@echo "*** Building Miranda Database Plugin"
	@echo "****************************************************************************"

##
## moc
##
$(MOCSRC) or_no_widgets: $(WIDGETS)
	@rm -f $@
	$(MOC) $(@:_moc.C=.h) > $@


##
## Automatic dependency stuff
##
include /data_vobs/VisIt/include/make-targets
