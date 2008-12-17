#                         Copyright (c) 2000 - 2008
#                  Lawrence Livermore National Security, LLC
#            Produced at the Lawrence Livermore National Laboratory
#                              LLNL-CODE-400124
#                            All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at https://visit.llnl.gov/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided  that the following conditions are met:
#
# - Redistributions of  source code  must retain  the above  copyright notice,
#   this list of conditions and the disclaimer below.
# - Redistributions in binary form must  reproduce the above copyright notice,
#   this  list of  conditions  and  the  disclaimer  (as noted below)  in  the
#   documentation and/or other materials provided with the distribution.
# - Neither the name of the LLNS/LLNL nor the names of its contributors may be
#   used to endorse  or promote products  derived from  this software  without
#   specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE  COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES,  INCLUDING, BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL  DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF  LIABILITY, WHETHER IN CONTRACT, STRICT  LIABILITY, OR TORT
# (INCLUDING  NEGLIGENCE OR  OTHERWISE) ARISING IN ANY  WAY OUT OF  THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Modifications:
#   Brad Whitlock, Fri Oct 10 15:05:43 PST 2008
#   I updated slivr.
#
##############################################################################


##
## Programs and options...
##

##							-*- makefile -*-
## Variables
##
TOPDIR=/home/josh/visit/trunk/src
SHELL=/bin/sh
CC=gcc
CXX=g++
CFLAGS=-D_LARGEFILE64_SOURCE  -m64 -fPIC
CXXFLAGS=-O2 -D_LARGEFILE64_SOURCE  -m64 -fPIC
CPPFLAGS=-I. -I$(TOPDIR)/include 
TUVOK_INCLUDE=
TUVOK_LIB=-lTuvok
CXXFLAGSORIG=-O2 -D_LARGEFILE64_SOURCE  -m64 -fPIC
CPPFLAGSORIG= 
PY_CXXFLAGS=-I$(TOPDIR)/include/python
CMAKE=/home/josh/visit/externals/visit/cmake/2.4.5/linux-x86_64_gcc-4.2.4/bin/cmake
MESA_INCLUDE=-I$(TOPDIR)/include/mesa
ICET_CXXFLAGS=
JAVAC=
JAR=
JAVAFLAGS=
JAVACLASS=
JAVAPLUGINFLAGS=
F77=g77
F77FLAGS=-g -O2
LIBS=-lz -lm -ldl 
LDFLAGS=-L$(TOPDIR)/lib 
EXE_LDFLAGS= -Wl,--rpath,\$$ORIGIN/../lib
PLUGIN_LDFLAGS= -Wl,--rpath,\$$ORIGIN/../../lib
LDFLAGSORIG=-L$(TOPDIR)/lib 
PY_LDFLAGS=
# Start Qt variables
QT_CXXFLAGS=-I$(TOPDIR)/include/qt -I$(TOPDIR)/include/qt/Qt -I$(TOPDIR)/include/qt/QtCore -I$(TOPDIR)/include/qt/QtGui -I$(TOPDIR)/include/qt/QtDesigner -I$(TOPDIR)/include/qt/QtHelp -I$(TOPDIR)/include/qt/QtNetwork -I$(TOPDIR)/include/qt/QtOpenGL -I$(TOPDIR)/include/qt/QtScript -I$(TOPDIR)/include/qt/QtSvg -I$(TOPDIR)/include/qt/QtUiTools -I$(TOPDIR)/include/qt/QtXml
QT_LDFLAGS=-L$(TOPDIR)/lib
QT_LIBS=$(QT_CORE_LIB) $(QT_GUI_LIB) $(QT_DESIGNER_LIB) $(QT_HELP_LIB) $(QT_CLUCENE_LIB) $(QT_NETWORK_LIB) $(QT_OPENGL_LIB) $(QT_SCRIPT_LIB) $(QT_SQL_LIB) $(QT_SVG_LIB) $(QT_TEST_LIB) $(QT_XML_LIB)
QT_CORE=libQtCore.so
QT_CORE_LIB=-lQtCore
QT_GUI=libQtGui.so
QT_GUI_LIB=-lQtGui
QT_HELP=libQtHelp.so
QT_HELP_LIB=-lQtHelp
QT_CLUCENE=libQtCLucene.so
QT_CLUCENE_LIB=-lQtCLucene
QT_NETWORK=libQtNetwork.so
QT_NETWORK_LIB=-lQtNetwork
QT_OPENGL=libQtOpenGL.so
QT_OPENGL_LIB=-lQtOpenGL
QT_SCRIPT=libQtScript.so
QT_SCRIPT_LIB=-lQtScript
QT_SQL=libQtSql.so
QT_SQL_LIB=-lQtSql
QT_SVG=libQtSvg.so
QT_SVG_LIB=-lQtSvg
QT_TEST=libQtTest.so
QT_TEST_LIB=-lQtTest
QT_WEBKIT=@QT_WEBKIT@
QT_WEBKIT_LIB=@QT_WEBKIT_LIB@
QT_XML=libQtXml.so
QT_XML_LIB=-lQtXml
QT_XMLPATTERNS=@QT_XMLPATTERNS@
QT_XMLPATTERNS_LIB=@QT_XMLPATTERNS_LIB@
QT_QUI=libQtUiTools.a
QT_QUI_LIB=-lQtUiTools
# End Qt variables
MESA_LIBS=-lOSMesa -lMesaGL
ICET_ENG_MAIN_OBJ=
ICET_LIBS=
GL_LIBS=-lGL
GLEW_LIBS= -lGLEW
GLEW_INCLUDE=-I$(TOPDIR)/include/glew
SLIVR_LIBS=
SLIVR_INCLUDE=
SLIVR_GUI_IMPL=
SLIVR_GUI_HDR=
PY_LIB=-lpython2.5
PYTHON=/home/josh/visit/externals/visit/python/2.5/linux-x86_64_gcc-4.2.4/bin/python
VTK_LIBS=-lvtkCommon -lvtkDICOMParser -lvtkFiltering -lvtkGraphics -lvtkHybrid -lvtkIO -lvtkImaging -lvtkRendering -lvtkzlib -lvtkexpat -lvtkjpeg -lvtkpng -lvtktiff -lvtkftgl -lvtkfreetype -lvtksys -lvtkMPEG2Encode 
MPI_LIBS=
SHLIB_MPI_LIBS=
GUILIB=$(GDSO)
MDSERVERLIB=$(MDSO)
SCRIPTINGLIB=$(SDSO)
VIEWERLIB=$(VDSO)
ENGINELIBSER=$(ESERDSO)
ENGINELIBPAR=
ENGINEWRAPSER=libvisitenginev1_ser.so
ENGINEWRAPPAR=
BUILD_FORTRAN=$(BUILD_FORTRAN)
X_LIBS=-lX11 -lXt 
AR=ar
RANLIB=ranlib
INSTALL=$(TOPDIR)/exe/install-sh -c -m 755
INSTALL_DATA=$(TOPDIR)/exe/install-sh -c -m 644
RM=rm -f

ZLIB_LIB=-lz

PTHREAD_CXXFLAGS=
PTHREAD_LIB=-lpthread

VISIT_VTK_CPPFLAGS= -I$(TOPDIR)/include/vtk -I$(TOPDIR)/include/vtk/Common -I$(TOPDIR)/include/vtk/Filtering -I$(TOPDIR)/include/vtk/Graphics -I$(TOPDIR)/include/vtk/Hybrid -I$(TOPDIR)/include/vtk/IO -I$(TOPDIR)/include/vtk/Imaging -I$(TOPDIR)/include/vtk/Rendering -I$(TOPDIR)/include/vtk/Utilities -I$(TOPDIR)/include/visit
VISIT_VTK_LDFLAGS=

SILO_INCLUDE=
SILO_LIB=

EXODUS_FLAGS=
EXODUS_LIBS=

HDF4_FLAGS=
HDF4_LIBS=

HDF5_INCLUDE=-I/home/josh/visit/externals/visit/hdf5/1.6.5/linux-x86_64_gcc-4.2.4/include
HDF5_LIB=-lhdf5 

PDB_INCLUDE=
PDB_LIB=

MILI_FLAGS=
MILI_LIBS=

BOXLIB2D_INCLUDE=-I/home/josh/visit/externals/visit/boxlib/linux-x86_64_gcc-4.2.4/include/2D 
BOXLIB2D_LIBS=-L/home/josh/visit/externals/visit/boxlib/linux-x86_64_gcc-4.2.4/lib -lbox2D
BOXLIB3D_INCLUDE=-I/home/josh/visit/externals/visit/boxlib/linux-x86_64_gcc-4.2.4/include/3D 
BOXLIB3D_LIBS=-L/home/josh/visit/externals/visit/boxlib/linux-x86_64_gcc-4.2.4/lib -lbox3D

H5PART_INCLUDE=
H5PART_LIB=

VISUS_INCLUDE=
VISUS_LIB=

NETCDF_INCLUDE=-I/home/josh/visit/externals/visit/netcdf/3.6.0/linux-x86_64_gcc-4.2.4/include 
NETCDF_LIB=/home/josh/visit/externals/visit/netcdf/3.6.0/linux-x86_64_gcc-4.2.4/lib/libnetcdf.a 
NETCDF_CXX_LIB=/home/josh/visit/externals/visit/netcdf/3.6.0/linux-x86_64_gcc-4.2.4/lib/libnetcdf_c++.a 

CGNS_INCLUDE=
CGNS_LIB=

GDAL_INCLUDE=
GDAL_LIB=
GDAL_LIBS=

CFITSIO_INCLUDE=
CFITSIO_LIB=

## For multiple implementations of ITAPS interfaces (up to 5)
ITAPS_C0_INCLUDE=
ITAPS_C0_LIB=
ITAPS_C1_INCLUDE=
ITAPS_C1_LIB=
ITAPS_C2_INCLUDE=
ITAPS_C2_LIB=
ITAPS_C3_INCLUDE=
ITAPS_C3_LIB=
ITAPS_C4_INCLUDE=
ITAPS_C4_LIB=

## For the single ITAPS implementation case
ITAPS_C_INCLUDE=
ITAPS_C_LIB=

CCMIO_INCLUDE=
CCMIO_LIBS=
CCMIO_LIBDIR=

## Library
LIBSTATIC=$(TOPDIR)/archives/$(LIB).a
LIBSHARED=$(TOPDIR)/lib/$(LIB).so
LIBSHAREDTEMP=$(LIB).so

LIBSTATICSER=$(TOPDIR)/archives/$(LIB)_ser.a
LIBSHAREDSER=$(TOPDIR)/lib/$(LIB)_ser.so
LIBSHAREDTEMPSER=$(LIB)_ser.so

LIBSTATICPAR=$(TOPDIR)/archives/$(LIB)_par.a
LIBSHAREDPAR=$(TOPDIR)/lib/$(LIB)_par.so
LIBSHAREDTEMPPAR=$(LIB)_par.so

##
## OpenGL and Mesa objects
##
OPENGL_OBJ=$(OPENGL_SRC:.C=_ogl.o)
MESA_OBJ=$(MESA_SRC:.C=_mesa.o)
SEROPENGL_OBJ=$(OPENGL_OBJ)
SERMESA_OBJ=$(MESA_OBJ)
PAROPENGL_OBJ=$(OPENGL_SRC:.C=_par_ogl.o)
PARMESA_OBJ=$(MESA_SRC:.C=_par_mesa.o)

##
## Temporary installation directories
##
tmplib=$(TOPDIR)/archives
tmpbin=$(TOPDIR)/exe
tmpinc=$(TOPDIR)/include
libdir=$(TOPDIR)/lib
plotplugindir=$(TOPDIR)/plugins/plots
operatorplugindir=$(TOPDIR)/plugins/operators


##
## Permanent installation directories
##
prefix=/usr/local
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
libdir=$(TOPDIR)/lib
includedir=${prefix}/include


TUVOK_SRCS = \
./Tuvok/3rdParty/GLEW/glew.c \
./Tuvok/Basics/SystemInfo.cpp \
./Tuvok/Basics/SysTools.cpp \
./Tuvok/Basics/MathTools.cpp \
./Tuvok/Basics/ArcBall.cpp \
./Tuvok/Basics/Checksums/MD5.cpp \
./Tuvok/IO/KeyValueFileParser.cpp \
./Tuvok/IO/TransferFunction1D.cpp \
./Tuvok/IO/TransferFunction2D.cpp \
./Tuvok/IO/VolumeDataset.cpp \
./Tuvok/IO/IOManager.cpp \
./Tuvok/IO/DirectoryParser.cpp \
./Tuvok/IO/DICOM/DICOMParser.cpp \
./Tuvok/IO/Images/ImageParser.cpp \
./Tuvok/IO/UVF/DataBlock.cpp \
./Tuvok/IO/UVF/GlobalHeader.cpp \
./Tuvok/IO/UVF/KeyValuePairDataBlock.cpp \
./Tuvok/IO/UVF/LargeRAWFile.cpp \
./Tuvok/IO/UVF/RasterDataBlock.cpp \
./Tuvok/IO/UVF/UVF.cpp \
./Tuvok/IO/UVF/UVFTables.cpp \
./Tuvok/IO/UVF/Histogram1DDataBlock.cpp \
./Tuvok/IO/UVF/Histogram2DDataBlock.cpp \
./Tuvok/IO/UVF/MaxMinDataBlock.cpp \
./Tuvok/Controller/MasterController.cpp \
./Tuvok/DebugOut/TextfileOut.cpp \
./Tuvok/DebugOut/ConsoleOut.cpp \
./Tuvok/DebugOut/MultiplexOut.cpp \
./Tuvok/Renderer/CullingLOD.cpp \
./Tuvok/Renderer/GLFrameCapture.cpp \
./Tuvok/Renderer/GLSLProgram.cpp \
./Tuvok/Renderer/GLTexture.cpp \
./Tuvok/Renderer/GLTexture1D.cpp \
./Tuvok/Renderer/GLTexture2D.cpp \
./Tuvok/Renderer/GLTexture3D.cpp \
./Tuvok/Renderer/GLRenderer.cpp \
./Tuvok/Renderer/AbstrRenderer.cpp \
./Tuvok/Renderer/GLRaycaster.cpp \
./Tuvok/Renderer/GLSBVR.cpp \
./Tuvok/Renderer/GPUMemMan/GPUMemMan.cpp \
./Tuvok/Renderer/GPUMemMan/GPUMemManDataStructs.cpp \
./Tuvok/Renderer/SBVRGeogen.cpp \
./Tuvok/Renderer/GLFBOTex.cpp

CXXFLAGS += $(QT_CXXFLAGS) -I./Tuvok
TUVOK_OBJS = $(TUVOK_SRCS:.cpp=.o)
##.cpp.o:
##	g++ $(TUVOK_SRCS) -c $< $(CXXFLAGS)

##
## Standard targets...
##
all: message Tuvok

clean:
##	$(RM) -rf slivr
##	$(RM) -rf $(TOPDIR)/include/slivr
##	$(RM) -rf $(TOPDIR)/lib/slivr
##	$(RM) $(TOPDIR)/lib/libslivrG.so
##	$(RM) $(TOPDIR)/lib/libslivrV.so
##	$(RM) -rf *.o

Tuvok: $(TUVOK_OBJS)
	g++ -shared -fPIC -o $(TOPDIR)/lib/libTuvok.so $(TUVOK_OBJS)
	ln -sf $(TOPDIR)/third_party_builtin/Tuvok/Tuvok $(TOPDIR)/include/Tuvok

##
## Other targets
##
message:
	@echo
	@echo "****************************************************************************"
	@echo "*** Building Tuvok (libTuvok)"
	@echo "****************************************************************************"

##							-*- makefile -*-
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
##  Jeremy Meredith (via Hank Childs), Thu May 17 08:42:21 PDT 2007
##  Remove .pardepend as well.
##
##  Jeremy Meredith, Tue Jun  5 17:29:41 EDT 2007
##  Remove $(DEP) instead of .pardepend, since I changed the way
##  dependencies for GNU systems are generated.
##
distclean: clean
	-$(RM) $(DISTCLEAN_FILES) *~ .depend $(DEP) core
	-$(RM) Makefile

##
## Suffixes
##
## Cyrus Harrison, Fri Mar  9 08:38:51 PST 2007
## Added engine specific targets: "_eng.o", "_par_eng.o"
## 
## Jeremy Meredith, Tue Jun  5 17:30:37 EDT 2007
## Added .d suffixes to match all the .o targets.
##
## Tom Fogal, Sun Jun  8 16:44:22 EDT 2008
## Added a couple rules for IceT objects.
##    "_icet.o": link with IceT
##    "_mesa_icet.o" link with IceT AND mesa.
## Reformatted to fit within 80 columns.

.SUFFIXES: .C _eng.o _mds.o _par.o _ogl.o _moc.C _mesa.o          \
           _par_eng.o _par_ogl.o _par_mesa.o _icet.o _mesa_icet.o \
           .d _eng.d _mds.d _par.d _ogl.d _mesa.d                 \
           _par_eng.d _par_ogl.d _par_mesa.d _icet.d _mesa_icet.d

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
## Cyrus Harrison, Fri Mar  9 08:38:37 PST 2007
## Added engine specific targets: "_eng.o", "_par_eng.o"
## 
## Tom Fogal, Fri Jun  8 11:07:27 PDT 2007
## Use depcomp to build dependencies during compilation.
##
## Tom Fogal, Sun Jun  8 18:00:05 EDT 2008
## Added IceT specific targets.  Wrapped long lines.
##
## Tom Fogal, Wed Sep 17 17:11:21 MDT 2008
## Renamed temporary depend files so that it is not possible to have a conflict
## when one file is compiled in multiple modes.
##

.C.a:
	false

.c.o:
	source='$<' object='$@' libtool=no        \
	depfile='./$*.d'  tmpdepfile='./$*.tmp_d' \
	$(DEPMODE) $(TOPDIR)/bin/depcomp          \
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

.c_mds.o:
	source='$<' object='$@' libtool=no        \
	depfile='./$*_mds.d'  tmpdepfile='./$*.tmp_mds_d' \
	$(DEPMODE) $(TOPDIR)/bin/depcomp          \
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMDSERVER -c $< -o $@

.c_par.o:
	source='$<' object='$@' libtool=no        \
	depfile='./$*_par.d'  tmpdepfile='./$*.tmp_par_d' \
	$(DEPMODE) $(TOPDIR)/bin/depcomp          \
	$(CC) $(CFLAGS) $(CPPFLAGS)  -c $< -o $@

# There cannot be a tab before the OS_RULE line.  The rule contains its own
# tab character.
.C.o:
	source='$<' object='$@' libtool=no        \
	depfile='./$*.d'  tmpdepfile='./$*.tmp_d' \
	$(DEPMODE) $(TOPDIR)/bin/depcomp          \
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	: Linux 2.6.24-22-generic

.C_eng.o:
	source='$<' object='$@' libtool=no        \
	depfile='./$*_eng.d'  tmpdepfile='./$*.tmp_eng_d' \
	$(DEPMODE) $(TOPDIR)/bin/depcomp          \
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -DENGINE -c $< -o $@
	: Linux 2.6.24-22-generic

.C_mds.o:
	source='$<' object='$@' libtool=no        \
	depfile='./$*_mds.d'  tmpdepfile='./$*.tmp_mds_d' \
	$(DEPMODE) $(TOPDIR)/bin/depcomp          \
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -DMDSERVER -c $< -o $@
	: Linux 2.6.24-22-generic

.C_par.o:
	source='$<' object='$@' libtool=no        \
	depfile='./$*_par.d'  tmpdepfile='./$*.tmp_par_d' \
	$(DEPMODE) $(TOPDIR)/bin/depcomp          \
	$(CXX) $(CXXFLAGS) $(CPPFLAGS)  -c $< -o $@
	: Linux 2.6.24-22-generic

.C_ogl.o:
	source='$<' object='$@' libtool=no        \
	depfile='./$*_ogl.d'  tmpdepfile='./$*.tmp_ogl_d' \
	$(DEPMODE) $(TOPDIR)/bin/depcomp          \
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	: Linux 2.6.24-22-generic

.C_mesa.o:
	source='$<' object='$@' libtool=no        \
	depfile='./$*_mesa.d'  tmpdepfile='./$*.tmp_mesa_d' \
	$(DEPMODE) $(TOPDIR)/bin/depcomp          \
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(MESA_INCLUDE) -c $< -o $@
	: Linux 2.6.24-22-generic

.C_par_eng.o:
	source='$<' object='$@' libtool=no        \
	depfile='./$*_par_eng.d'  tmpdepfile='./$*.tmp_par_eng_d' \
	$(DEPMODE) $(TOPDIR)/bin/depcomp          \
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -DENGINE  -c $< -o $@
	: Linux 2.6.24-22-generic

.C_par_ogl.o:
	source='$<' object='$@' libtool=no        \
	depfile='./$*_par_ogl.d'  tmpdepfile='./$*.tmp_par_ogl_d' \
	$(DEPMODE) $(TOPDIR)/bin/depcomp          \
	$(CXX) $(CXXFLAGS) $(CPPFLAGS)  -c $< -o $@
	: Linux 2.6.24-22-generic

.C_par_mesa.o:
	source='$<' object='$@' libtool=no        \
	depfile='./$*_par_mesa.d'  tmpdepfile='./$*.tmp_par_mesa_d' \
	$(DEPMODE) $(TOPDIR)/bin/depcomp          \
	$(CXX) $(CXXFLAGS) $(CPPFLAGS)  $(MESA_INCLUDE) \
	-c $< -o $@
	: Linux 2.6.24-22-generic

# _icet implies parallel; the PARALLEL_CPPFLAGS are there on purpose.
.C_icet.o:
	source='$<' object='$@' libtool=no        \
	depfile='./$*_icet.d'  tmpdepfile='./$*.tmp_icet_d' \
	$(DEPMODE) $(TOPDIR)/bin/depcomp          \
	$(CXX) $(CXXFLAGS) $(CPPFLAGS)      \
	  \
	-c $< -o $@
	: Linux 2.6.24-22-generic

.C_mesa_icet.o:
	source='$<' object='$@' libtool=no        \
	depfile='./$*_mesa_icet.d'  tmpdepfile='./$*.tmp_mesa_icet_d' \
	$(DEPMODE) $(TOPDIR)/bin/depcomp          \
	$(CXX) $(CXXFLAGS) $(CPPFLAGS)                      \
	  $(MESA_INCLUDE) \
	-c $< -o $@
	: Linux 2.6.24-22-generic

.f.o:
	$(F77) $(F77FLAGS) -c $<


##
## Implicit rule for Qt MOC source.
##
## Jeremy Meredith, Thu Jun  7 13:24:49 EDT 2007
## Added initial version.
##
.h_moc.C:
	@$(RM) $@
	$(TOPDIR)/bin/moc $< > $@


## Jeremy Meredith, Fri May 11 13:51:37 PDT 2001
## Create GUI, Viewer and Engine shared libraries.  This is for plugins.
## 
## Brad Whitlock, Tue Jul 24 9:42:23 PDT 2001
## Added a target for building scripting plugins.
##
## Jeremy Meredith, Fri Sep 28 13:23:59 PDT 2001
## Added IDSO.
##
## Hank Childs, Tue May 21 16:18:10 PDT 2002
## Put LDFLAGS in front of the obj files, since g++ won't get to them in time
## otherwise.
##
## Jeremy Meredith, Fri Jun 14 12:09:38 PDT 2002
## Added PY_LIB to the scripting plugins since it is a static library.
##
## Jeremy Meredith, Thu Jul 18 13:12:23 PDT 2002
## Removed PY_LIB.  It was causing some very strange problems.  Instead
## libpython2.1 will just be a .so instead of a .a, and distributed with
## the runtime libraries.
##
## Jeremy Meredith, Thu Aug 22 13:22:28 PDT 2002
## Added MDServer target.  Added libraries which can be forcibly linked
## against the shared library target.
##
## Brad Whitlock, Mon Apr 26 14:41:46 PST 2004
## I added a postprocessing step that in the default case evaluates to
## sleep 1. On MacOS X, the post processing step is used to update the
## address file used to determine the segment offset.
##
## Brad Whitlock, Fri Feb 23 17:31:25 PST 2007
## Added VIEWERMOCOBJ
##
## Cyrus Harrison, Fri Sep 19 14:20:32 PDT 2008
## Added custom libs for gui,engine,mdserver, and viewer libs.
##
$(IDSO) noidso: $(IOBJ)
	$(CXX) -shared -o $@ $(LDFLAGS) $(IOBJ) 
	

$(GDSO) nogdso: $(GOBJ) $(MOCOBJ)
	$(CXX) -shared -o $@ $(LDFLAGS) $(GOBJ) $(MOCOBJ) $(SHLIB_FORCED) $(GUI_LIBS_FORCED) \
		
	

$(MDSO) nomdso: $(MOBJ)
	$(CXX) -shared -o $@ $(LDFLAGS) $(MOBJ) $(SHLIB_FORCED) $(MDSERVER_LIBS_FORCED) 
	

$(SDSO) nosdso: $(SOBJ)
	$(CXX) -shared -o $@ $(LDFLAGS) $(CXXFLAGS) $(SOBJ) 
	

$(VDSO) novdso: $(VOBJ) $(VIEWERMOCOBJ)
	$(CXX) -shared -o $@ $(LDFLAGS) $(VOBJ) $(VIEWERMOCOBJ) $(SHLIB_FORCED) $(VIEWER_LIBS_FORCED)  \
		
	

$(ESERDSO) noedso_ser: $(ESEROBJ)
	$(CXX) -shared -o $@ $(LDFLAGS) $(ESEROBJ) $(SHLIB_FORCED) $(ENGINE_LIBS_FORCED) \
		
	

$(EPARDSO) noedso_par: $(EPAROBJ)
	$(CXX) -shared -o $@ $(LDFLAGS) $(EPAROBJ) $(SHLIB_FORCED) $(ENGINE_LIBS_FORCED)  \
		  
	

## Brad Whitlock, Wed Aug 14 16:26:55 PST 2002
## I added the Java target.
java:
	$(JAVAC) $(JAVAFLAGS) $(JAVAPLUGINFLAGS) $(JAVASRC)

## ------------------------------------------------------------------
## Maintaining source dependencies... either GNU, generic, or none...
## ------------------------------------------------------------------

##							-*- makefile -*-
## Automatic dependencies...
##
## Programmer:  Jeremy Meredith
## Date:        June  5, 2007
##
## Modifications:
##    Jeremy Meredith, Fri Jun  8 12:59:19 EDT 2007
##    Removed -MG; it seemed to cause recursive dependency loops with -MP.
##    Added -MT to avoid basename target problem and add .o and .d 
##    targets in the first pass, avoiding the need for sed.  Also added a
##    fail command that aborts the make, since with no -MG errors can occur.
##
##    Tom Fogal, Fri Jun  8 11:25:35 PDT 2007
##    Simplified dependency generation: we generate during compilation via
##    depcomp.  Now we just set the `depmode'; the rest is taken care of in
##    make-targets.in
##
##    Tom Fogal, Fri Jun 20 09:46:17 EDT 2008
##    I changed hardcoded -DPARALLEL values to PARALLEL_CPPFLAGS, to pick up on
##    the include dir additions I made for IceT.

DEPMODE=depmode=gcc3

SERDEP:=\
 $(ALLOBJ:.o=.d) \
 $(COMMONOBJ:.o=.d) \
 $(CONFIGOBJ:.o=.d) \
 $(INITOBJ:.o=.d) \
 $(LIBRARYSEROBJ:.o=.d) \
 $(MAINOBJ:.o=.d) \
 $(MAINSEROBJ:.o=.d) \
 $(MOCOBJ:.o=.d) \
 $(OBJMOC:.o=.d) \
 $(OBJ:.o=.d) \
 $(FOBJ:.o=.d) \
 $(SEROBJ:.o=.d) \
 $(FSEROBJ:.o=.d) \
 $(TESTOBJ:.o=.d) \
 $(TESTOBJMOC:.o=.d) \
 $(ESEROBJ:.o=.d) \
 $(GOBJ:.o=.d) \
 $(IOBJ:.o=.d) \
 $(JAVAOBJ:.o=.d) \
 $(MOBJ:.o=.d) \
 $(SOBJ:.o=.d) \
 $(VOBJ:.o=.d) \
 $(VIEWERMOCOBJ:.o=.d) \
 $(MOCOBJ:.o=.d)

PARDEP:=\
 $(LIBRARYPAROBJ:.o=.d) \
 $(MAINPAROBJ:.o=.d) \
 $(PAROBJ:.o=.d) \
 $(FPAROBJ:.o=.d) \
 $(EPAROBJ:.o=.d)

DEP=$(SERDEP) 

-include $(DEP)

depend: $(DEP)

clean-dep:
	$(RM) -f $(DEP)

.PHONY: clean-dep depend
