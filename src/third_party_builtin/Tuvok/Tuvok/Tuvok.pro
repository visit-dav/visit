TEMPLATE          = lib
CONFIG           += staticlib static
TARGET            = Tuvok
VERSION           = 0.0.1
TARGET            = Build/Tuvok
RCC_DIR           = Build/rcc
OBJECTS_DIR       = Build/objects
DEPENDPATH       += .
INCLUDEPATH      += .
QT               += opengl

# Input
HEADERS += StdDefines.h \
           Basics/Grids.h \
           Basics/SysTools.h \
           Basics/Vectors.h \
           Basics/MathTools.h \
           Basics/ArcBall.h \
           Basics/Checksums/MD5.h \
           Basics/Checksums/crc32.h \
           IO/DirectoryParser.h \
           IO/KeyValueFileParser.h \
           IO/Transferfunction1D.h \
           IO/Transferfunction2D.h \
           IO/VolumeDataset.h \
           IO/IOManager.h \
           IO/DICOM/DICOMParser.h \
           IO/Images/ImageParser.h \
           IO/UVF/DataBlock.h \
           IO/UVF/GlobalHeader.h \
           IO/UVF/KeyValuePairDataBlock.h \
           IO/UVF/LargeRAWFile.h \
           IO/UVF/RasterDataBlock.h \
           IO/UVF/UVF.h \
           IO/UVF/UVFBasic.h \
           IO/UVF/UVFTables.h \
           IO/UVF/Histogram1DDataBlock.h \
           IO/UVF/Histogram2DDataBlock.h \
           IO/UVF/MaxMinDataBlock.h \
           Controller/MasterController.h \
           DebugOut/AbstrDebugOut.h \
           DebugOut/TextfileOut.h \
           DebugOut/ConsoleOut.h \
           DebugOut/MultiplexOut.h \
           3rdParty/GLEW/glew.h \
           3rdParty/GLEW/glxew.h \
           Renderer/FrameCapture.h \
           Renderer/GLFrameCapture.h \
           Renderer/CullingLOD.h \
           Renderer/GLSLProgram.h \
           Renderer/GLInclude.h \
           Renderer/GLObject.h \
           Renderer/GLTexture.h \
           Renderer/GLTexture1D.h \
           Renderer/GLTexture2D.h \
           Renderer/GLTexture3D.h \
           Renderer/GLRenderer.h \
           Renderer/AbstrRenderer.h \
           Renderer/GLRaycaster.h \
           Renderer/GLSBVR.h \
           Renderer/GPUMemMan/GPUMemMan.h \
           Renderer/GPUMemMan/GPUMemManDataStructs.h \
           Renderer/SBVRGeogen.h

SOURCES += 3rdParty/GLEW/glew.c \
           Basics/SystemInfo.cpp \
           Basics/SysTools.cpp \
           Basics/MathTools.cpp \
           Basics/ArcBall.cpp \
           Basics/Checksums/MD5.cpp \
           IO/KeyValueFileParser.cpp \           
           IO/Transferfunction1D.cpp \
           IO/Transferfunction2D.cpp \
           IO/VolumeDataset.cpp \
           IO/IOManager.cpp \
           IO/DirectoryParser.cpp \
           IO/DICOM/DICOMParser.cpp \
           IO/Images/ImageParser.cpp \           
           IO/UVF/DataBlock.cpp \
           IO/UVF/GlobalHeader.cpp \
           IO/UVF/KeyValuePairDataBlock.cpp \
           IO/UVF/LargeRAWFile.cpp \
           IO/UVF/RasterDataBlock.cpp \
           IO/UVF/UVF.cpp \
           IO/UVF/UVFTables.cpp \
           IO/UVF/Histogram1DDataBlock.cpp \
           IO/UVF/Histogram2DDataBlock.cpp \                     
           IO/UVF/MaxMinDataBlock.cpp \
           Controller/MasterController.cpp \
           DebugOut/TextfileOut.cpp \
           DebugOut/ConsoleOut.cpp \
           DebugOut/MultiplexOut.cpp \ 
           Renderer/CullingLOD.cpp \
           Renderer/GLFrameCapture.cpp \
           Renderer/GLSLProgram.cpp \
           Renderer/GLTexture.cpp \
           Renderer/GLTexture1D.cpp \
           Renderer/GLTexture2D.cpp \
           Renderer/GLTexture3D.cpp \
           Renderer/GLRenderer.cpp \           
           Renderer/AbstrRenderer.cpp \
           Renderer/GLRaycaster.cpp \
           Renderer/GLSBVR.cpp \
           Renderer/GPUMemMan/GPUMemMan.cpp \
           Renderer/GPUMemMan/GPUMemManDataStructs.cpp \
           Renderer/SBVRGeogen.cpp \  
           Renderer/GLFBOTex.cpp
