# NOTE: This file is included into the CMakeLists.txt for the VCell reader. This
# strategy is employed so we may add source files directly to the LIBM_SOURCES
# and LIBE_SOURCES variables used in the CMakeLists.txt. We do this now instead 
# of relying on intermediate static libraries, which is discouraged and even
# broken on some platforms. We add to the source variables in this file so we
# can keep all of this extra cmake logic outside of the xml2cmake-generated
# CMakeLists.txt.
#
# We create 3 variables to contain the names of the source files that used to
# go into the 3 static libraries. We create EXPRESSIONPARSER_SRC_FILES,
# UNZIP_SRC_FILES, and ZIP_SRC_FILES. We set special source file properties
# on the UNZIP_SRC_FILES and ZIP_SRC_FILES sources to influence how they
# are built.
#
# Note that including both UNZIP_SRC_FILES and ZIP_SRC_FILES in the sources
# caused a duplicate symbol link time error for crc32(). Consequently, we are
# only adding the UNZIP_SRC_FILES to the LIBM_SOURCES and LIBE_SOURCES. This
# is a reader after all and the ZIP_SRC_FILES so not seem to be needed in order
# to link.

#
# Set up the expression parser source files.
#
FILE (GLOB EXPRESSIONPARSER_SRC_FILES ExpressionParser/*.cpp)    

#
# Set up the unzip source files.
#
FILE (GLOB UNZIP_SRC_FILES unzip/*.c)
IF (WIN32)
    LIST(APPEND UNZIP_SRC_FILES
         unzip/win32/nt.c
         unzip/win32/win32.c
    )
    SET_SOURCE_FILES_PROPERTIES( ${UNZIP_SRC_FILES} PROPERTIES 
        LANGUAGE CXX
        COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/unzip -I${CMAKE_CURRENT_SOURCE_DIR}/unzip/win32")
ELSE (WIN32)
    LIST(APPEND UNZIP_SRC_FILES
         unzip/unix/unix.c
    )
    SET_SOURCE_FILES_PROPERTIES( ${UNZIP_SRC_FILES} PROPERTIES 
        COMPILE_DEFINITIONS UNIX
        LANGUAGE CXX
        COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/unzip -I${CMAKE_CURRENT_SOURCE_DIR}/unzip/unix")
ENDIF (WIN32)


#
# Set up the zip source files. (We don't use these currently)
#
FILE (GLOB ZIP_SRC_FILES zip/*.c)
IF (WIN32)
    LIST(APPEND ZIP_SRC_FILES
         zip/win32/nt.c
         zip/win32/win32.c
    )
    SET_SOURCE_FILES_PROPERTIES( ${ZIP_SRC_FILES} PROPERTIES
        COMPILE_DEFINITIONS "USE_ZIPMAIN NO_ASM WIN32 MSDOS"
        LANGUAGE CXX
        COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/zip -I${CMAKE_CURRENT_SOURCE_DIR}/zip/win32")
ELSE (WIN32)
    LIST(APPEND ZIP_SRC_FILES
         zip/unix/unix.c
    )
    SET_SOURCE_FILES_PROPERTIES( ${ZIP_SRC_FILES} PROPERTIES 
        COMPILE_DEFINITIONS "USE_ZIPMAIN NO_ASM UNIX HAVE_DIRENT_H HAVE_TERMIOS_H"
        LANGUAGE CXX
        COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/zip -I${CMAKE_CURRENT_SOURCE_DIR}/zip/unix")
ENDIF (WIN32)

#
# Augment the LIBM_SOURCES and LIBE_SOURCES with the additional files that
# we need to include in those libraries.
#    
SET(LIBM_SOURCES 
${LIBM_SOURCES}
${EXPRESSIONPARSER_SRC_FILES}
${UNZIP_SRC_FILES}
)

SET(LIBE_SOURCES 
${LIBE_SOURCES}
${EXPRESSIONPARSER_SRC_FILES}
${UNZIP_SRC_FILES}
)
