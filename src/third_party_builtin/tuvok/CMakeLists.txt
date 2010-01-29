#*****************************************************************************
#
# Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-400142
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# Modifications:
#
#****************************************************************************/

PROJECT(tuvok)

SET(TUVOK_SOURCES
VisItDebugOut.C
tuvok/Scripting/Scripting.cpp
tuvok/Controller/MasterController.cpp
tuvok/Renderer/GL/GLRaycaster.cpp
tuvok/Renderer/GL/GLTexture1D.cpp
tuvok/Renderer/GL/GLFBOTex.cpp
tuvok/Renderer/GL/GLSLProgram.cpp
tuvok/Renderer/GL/GLTexture2D.cpp
tuvok/Renderer/GL/GLTexture.cpp
tuvok/Renderer/GL/GLTargetBinder.cpp
tuvok/Renderer/GL/GLRenderer.cpp
tuvok/Renderer/GL/GLTexture3D.cpp
tuvok/Renderer/GL/GLFrameCapture.cpp
tuvok/Renderer/GL/GLSBVR.cpp
tuvok/Renderer/GPUMemMan/GPUMemMan.cpp
tuvok/Renderer/GPUMemMan/GPUMemManDataStructs.cpp
tuvok/Renderer/DX/DXRaycaster.cpp
tuvok/Renderer/DX/DXRenderer.cpp
tuvok/Renderer/DX/DXSBVR.cpp
tuvok/Renderer/DX/DXTexture.cpp
tuvok/Renderer/DX/DXTexture1D.cpp
tuvok/Renderer/DX/DXTexture2D.cpp
tuvok/Renderer/DX/DXTexture3D.cpp
tuvok/Renderer/CullingLOD.cpp
tuvok/Renderer/SBVRGeogen.cpp
tuvok/Renderer/AbstrRenderer.cpp
tuvok/Renderer/FrameCapture.cpp
tuvok/Renderer/TFScaling.cpp
tuvok/3rdParty/bzip2/bzlib.c
tuvok/3rdParty/bzip2/blocksort.c
tuvok/3rdParty/bzip2/decompress.c
tuvok/3rdParty/bzip2/crctable.c
tuvok/3rdParty/bzip2/huffman.c
tuvok/3rdParty/bzip2/compress.c
tuvok/3rdParty/bzip2/randtable.c
tuvok/3rdParty/tiff/tif_swab.c
tuvok/3rdParty/tiff/tif_luv.c
tuvok/3rdParty/tiff/tif_thunder.c
tuvok/3rdParty/tiff/tif_packbits.c
tuvok/3rdParty/tiff/tif_zip.c
tuvok/3rdParty/tiff/tif_aux.c
tuvok/3rdParty/tiff/tif_close.c
tuvok/3rdParty/tiff/tif_color.c
tuvok/3rdParty/tiff/tif_dirinfo.c
tuvok/3rdParty/tiff/tif_error.c
tuvok/3rdParty/tiff/tif_fax3sm.c
tuvok/3rdParty/tiff/tif_pixarlog.c
tuvok/3rdParty/tiff/tif_tile.c
tuvok/3rdParty/tiff/tif_predict.c
tuvok/3rdParty/tiff/tif_codec.c
tuvok/3rdParty/tiff/tif_compress.c
tuvok/3rdParty/tiff/tif_dirwrite.c
tuvok/3rdParty/tiff/tif_fax3.c
tuvok/3rdParty/tiff/tif_lzw.c
tuvok/3rdParty/tiff/tif_unix.c
tuvok/3rdParty/tiff/tif_next.c
tuvok/3rdParty/tiff/tif_version.c
tuvok/3rdParty/tiff/tif_open.c
tuvok/3rdParty/tiff/tif_warning.c
tuvok/3rdParty/tiff/tif_dir.c
tuvok/3rdParty/tiff/tif_dumpmode.c
tuvok/3rdParty/tiff/tif_flush.c
tuvok/3rdParty/tiff/tif_print.c
tuvok/3rdParty/tiff/tif_dirread.c
tuvok/3rdParty/tiff/tif_extension.c
tuvok/3rdParty/tiff/tif_getimage.c
tuvok/3rdParty/tiff/tif_write.c
tuvok/3rdParty/tiff/tif_read.c
tuvok/3rdParty/tiff/tif_strip.c
tuvok/3rdParty/jpeglib/jidctred.c
tuvok/3rdParty/jpeglib/jcmaster.c
tuvok/3rdParty/jpeglib/jcphuff.c
tuvok/3rdParty/jpeglib/jdcoefct.c
tuvok/3rdParty/jpeglib/jdmaster.c
tuvok/3rdParty/jpeglib/jcapistd.c
tuvok/3rdParty/jpeglib/jcdctmgr.c
tuvok/3rdParty/jpeglib/jclhuff.c
tuvok/3rdParty/jpeglib/jcodec.c
tuvok/3rdParty/jpeglib/jcpred.c
tuvok/3rdParty/jpeglib/jdcolor.c
tuvok/3rdParty/jpeglib/jdmerge.c
tuvok/3rdParty/jpeglib/jcprepct.c
tuvok/3rdParty/jpeglib/jdphuff.c
tuvok/3rdParty/jpeglib/jcsample.c
tuvok/3rdParty/jpeglib/jddctmgr.c
tuvok/3rdParty/jpeglib/jdpostct.c
tuvok/3rdParty/jpeglib/jmemmgr.c
tuvok/3rdParty/jpeglib/jcdiffct.c
tuvok/3rdParty/jpeglib/jclossls.c
tuvok/3rdParty/jpeglib/jcomapi.c
tuvok/3rdParty/jpeglib/jcscale.c
tuvok/3rdParty/jpeglib/jddiffct.c
tuvok/3rdParty/jpeglib/jdpred.c
tuvok/3rdParty/jpeglib/jmemnobs.c
tuvok/3rdParty/jpeglib/jdsample.c
tuvok/3rdParty/jpeglib/jmemsrc.c
tuvok/3rdParty/jpeglib/jdscale.c
tuvok/3rdParty/jpeglib/cdjpeg.c
tuvok/3rdParty/jpeglib/jcapimin.c
tuvok/3rdParty/jpeglib/jccoefct.c
tuvok/3rdParty/jpeglib/jchuff.c
tuvok/3rdParty/jpeglib/jclossy.c
tuvok/3rdParty/jpeglib/jcshuff.c
tuvok/3rdParty/jpeglib/jdhuff.c
tuvok/3rdParty/jpeglib/jdshuff.c
tuvok/3rdParty/jpeglib/jctrans.c
tuvok/3rdParty/jpeglib/jdtrans.c
tuvok/3rdParty/jpeglib/jerror.c
tuvok/3rdParty/jpeglib/jquant1.c
tuvok/3rdParty/jpeglib/jquant2.c
tuvok/3rdParty/jpeglib/jutils.c
tuvok/3rdParty/jpeglib/jcmainct.c
tuvok/3rdParty/jpeglib/jdapimin.c
tuvok/3rdParty/jpeglib/jdinput.c
tuvok/3rdParty/jpeglib/jfdctflt.c
tuvok/3rdParty/jpeglib/jdapistd.c
tuvok/3rdParty/jpeglib/jdlhuff.c
tuvok/3rdParty/jpeglib/jfdctfst.c
tuvok/3rdParty/jpeglib/jdlossls.c
tuvok/3rdParty/jpeglib/jfdctint.c
tuvok/3rdParty/jpeglib/rdcolmap.c
tuvok/3rdParty/jpeglib/jdlossy.c
tuvok/3rdParty/jpeglib/jidctflt.c
tuvok/3rdParty/jpeglib/rdswitch.c
tuvok/3rdParty/jpeglib/jdatadst.c
tuvok/3rdParty/jpeglib/jdmainct.c
tuvok/3rdParty/jpeglib/jidctfst.c
tuvok/3rdParty/jpeglib/transupp.c
tuvok/3rdParty/jpeglib/jccolor.c
tuvok/3rdParty/jpeglib/jcinit.c
tuvok/3rdParty/jpeglib/jcmarker.c
tuvok/3rdParty/jpeglib/jcparam.c
tuvok/3rdParty/jpeglib/jdatasrc.c
tuvok/3rdParty/jpeglib/jdmarker.c
tuvok/3rdParty/jpeglib/jidctint.c
tuvok/DebugOut/AbstrDebugOut.cpp
tuvok/DebugOut/ConsoleOut.cpp
tuvok/DebugOut/MultiplexOut.cpp
tuvok/DebugOut/TextfileOut.cpp
tuvok/Basics/Plane.cpp
tuvok/Basics/Appendix.cpp
tuvok/Basics/LargeRAWFile.cpp
tuvok/Basics/Checksums/MD5.cpp
tuvok/Basics/GeometryGenerator.cpp
tuvok/Basics/MathTools.cpp
tuvok/Basics/MemMappedFile.cpp
tuvok/Basics/SysTools.cpp
tuvok/Basics/Systeminfo/VidMemViaDDraw.cpp
tuvok/Basics/Systeminfo/VidMemViaDXGI.cpp
tuvok/Basics/SystemInfo.cpp
tuvok/Basics/MC.cpp
tuvok/Basics/ArcBall.cpp
tuvok/Basics/DynamicDX.cpp
tuvok/IO/TransferFunction2D.cpp
tuvok/IO/uvfMetadata.cpp
tuvok/IO/RAWConverter.cpp
tuvok/IO/UVF/GlobalHeader.cpp
tuvok/IO/UVF/Histogram1DDataBlock.cpp
tuvok/IO/UVF/Histogram2DDataBlock.cpp
tuvok/IO/UVF/KeyValuePairDataBlock.cpp
tuvok/IO/UVF/UVF.cpp
tuvok/IO/UVF/DataBlock.cpp
tuvok/IO/UVF/UVFTables.cpp
tuvok/IO/UVF/MaxMinDataBlock.cpp
tuvok/IO/UVF/RasterDataBlock.cpp
tuvok/IO/uvfDataset.cpp
tuvok/IO/I3MConverter.cpp
tuvok/IO/TiffVolumeConverter.cpp
tuvok/IO/UnbrickedDataset.cpp
tuvok/IO/Metadata.cpp
tuvok/IO/QVISConverter.cpp
tuvok/IO/TransferFunction1D.cpp
tuvok/IO/BOVConverter.cpp
tuvok/IO/TuvokJPEG.cpp
tuvok/IO/NRRDConverter.cpp
tuvok/IO/Dataset.cpp
tuvok/IO/StkConverter.cpp
tuvok/IO/gzio.c
tuvok/IO/VFFConverter.cpp
tuvok/IO/UnbrickedDSMetadata.cpp
tuvok/IO/Images/ImageParser.cpp
tuvok/IO/KeyValueFileParser.cpp
tuvok/IO/REKConverter.cpp
tuvok/IO/DICOM/DICOMParser.cpp
tuvok/IO/AbstrConverter.cpp
tuvok/IO/DirectoryParser.cpp
tuvok/IO/IOManager.cpp
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}/tuvok
${CMAKE_CURRENT_SOURCE_DIR}/tuvok/3rdParty
${CMAKE_CURRENT_SOURCE_DIR}/tuvok/IO/3rdParty
${CMAKE_CURRENT_SOURCE_DIR}/tuvok/IO/3rdParty/boost
${VISIT_SOURCE_DIR}/third_party_builtin/glew/glew/include
${QT_INCLUDES}
)

# Build a static tuvok library
SET(BUILD_SHARED_LIBS 0)

ADD_DEFINITIONS(-DQT_SHARED -DTUVOK_NO_IO -DGLSL_DEBUG)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${QT_LIBRARY_DIR})

ADD_LIBRARY(tuvok ${TUVOK_SOURCES})

##
## These are needed when building tuvok as a shared library.
##

#TARGET_LINK_LIBRARIES(tuvok
#visitcommon
#GLEW
#${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY} 
#z
#)

#IF(APPLE)
#    SET_TARGET_PROPERTIES(tuvok
#        PROPERTIES
#        LINK_FLAGS "-framework Carbon"
#    )
#ENDIF(APPLE)

#INSTALL(DIRECTORY tuvok/Shaders
#    DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
#    RENAME shaders
#    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
#)
