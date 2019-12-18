# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Eric Brugger, Fri Jan 22 16:51:39 PST 2010
#   I added the data generator unic, which generates UNIC test data.
#
#   Mark C. Miller, Fri Jan 22 16:40:43 PST 2010
#   Added function LINK_DATA_TARGETS and invokation to create sesame.pdb.lnk
#   and largefile.silo.lnk.
#
#   Mark C. Miller, Fri Jan 22 17:27:47 PST 2010
#   Removed preceding change to satisfy purists ;)
#
#   Cyrus Harrison, Mon Feb  1 12:46:56 PST 2010
#   Added missing visit includes to basic targets.
#
#   Brad Whitlock, Wed Feb 10 14:44:23 PST 2010
#   I added Velodyne test data.
#
#   Eric Brugger, Thu Mar 11 15:04:16 PST 2010
#   I added logic to create the example data files for a visit distribution
#   if the file visit_dist_data.tar.gz was not present.
#
#   Eric Brugger, Fri Mar 12 13:08:21 PST 2010
#   I added dependencies and additional outputs to the custom commands I
#   added yesterday so that parallel builds would work and make clean would
#   remove all the files.
#
#   Mark C. Miller, Fri Apr 23 23:51:46 PDT 2010
#   Use glob to produce list of archived targets. That way, don't have to
#   edit this makefile every time one is added or removed.
#
#   Brad Whitlock, Thu Apr 29 11:50:42 PDT 2010
#   I added xdmf2d.C.
#
#   Eric Brugger, Thu Jun 17 15:24:51 PDT 2010
#   I added logic to only create the Silo data files if VisIt is being
#   built with the Silo library.
#
#   Kathleen Bonnell, Mon Nov 15 15:46:11 MST 2010
#   Add support for building on windows.
#
#   Cyrus Harrison, Wed Feb  9 11:20:26 PST 2011
#   Fix install of data files from out of source build.
#
#   Eric Brugger, Fri Nov 18 14:52:52 PST 2011
#   I added xdmf_binary.C.
#
#   Eric Brugger, Fri Oct 18 14:55:19 PDT 2013
#   I changed the name of the test target to testdata, since test no longer
#   works with cmake 2.8.10.2.
#
#   Kathleen Biagas, Tue Nov 25 18:06:23 MST 2014
#   Add /STACK Link flag to multi_test target on Windows so it will run
#   properly.  Reworked the create_data_examples targets so things work
#   correctly on Windows.
#
#   Eric Brugger, Wed Sep 20 17:35:37 PDT 2017
#   Modify the script to untar files of the name "*test_data*.tar.gz" instead
#   of just "*test_data.tar.gz".
#
#   Mark C. Miller, Mon Jun 25 17:23:58 PDT 2018
#   Moved data generation .[Ch] sources to src/tools/data/datagen.
#   Switch to 7z compression reducing total size by almost half.
#   Since 7z itself is both a compressor and an archiver, we no longer
#   need to use tar and files have only .7z extension and do not include
#   .tar in file name extensions. Added `help` target for info on using 7z.
#   Added convenience targets so deep knowledge of using 7z isn't necessary.
#
#   Kathleen Biagas, Thu Dec 13 09:54:33 PST 2018
#   Add 'PATHS' and 'PATH_SUFFIXES' when finding 7zip on non-windows.
#   Beefed up error message if 7-zip not found.
#
#   Mark C. Miller, Tue Apr 16 13:11:15 PDT 2019
#   Reduced min cmake vesion required to 3.7
#
#   Mark C. Miller, Tue Dec 10 09:24:24 PST 2019
#   Remove custom help target.
#****************************************************************************

CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

IF(WIN32)
    PROJECT(VISIT_DATA)
ENDIF(WIN32)

MESSAGE(STATUS "Configuring VisIt Binary Tarball Data Targets")

#-----------------------------------------------------------------------------
# Set up archiver executable name (7z, 7za and p7zip are common ones)
#-----------------------------------------------------------------------------
IF(NOT VISIT_DATA_ARCHIVER_NAME)
    list(APPEND VISIT_DATA_ARCHIVER_NAME "7z" "7za" "p7zip")
ENDIF()

#-----------------------------------------------------------------------------
# Find the archiver exectuable
#-----------------------------------------------------------------------------
IF(WIN32)
    SET(PF1 "PROGRAMFILES")
    SET(PF2 "PROGRAMFILES(X86)")
    FIND_PROGRAM(ARCHIVER_EXE NAMES ${VISIT_DATA_ARCHIVER_NAME}
        PATHS ${VISIT_SEVEN_ZIP_DIR} 
              $ENV{${PF1}}/7-Zip 
              $ENV{${PF2}}/7-Zip)
    UNSET(PF2)
    UNSET(PF1)
ELSE(WIN32)
    FIND_PROGRAM(ARCHIVER_EXE NAMES ${VISIT_DATA_ARCHIVER_NAME}
                              PATHS ${VISIT_SEVEN_ZIP_DIR}
                              PATH_SUFFIXES bin)
ENDIF(WIN32)

#-----------------------------------------------------------------------------
# Set arguments for archiver creation, extraction and list convenience targets
#-----------------------------------------------------------------------------
IF("${ARCHIVER_EXE}" STREQUAL "ARCHIVER_EXE-NOTFOUND")
    MESSAGE(WARNING "Archiver \"${VISIT_DATA_ARCHIVER_NAME}\" not found, data files cannot be extracted. Try setting VISIT_SEVEN_ZIP_DIR to location of 7-zip package, and VISIT_DATA_ARCHIVER_NAME to correct 7-zip executable name (eg, 7za, p7zip, 7z).")
ELSE("${ARCHIVER_EXE}" STREQUAL "ARCHIVER_EXE-NOTFOUND")
    SET(ARCHIVER_CMD ${ARCHIVER_EXE})
    SET(ARCHIVER_XARGS x -y)                 # Expand archive args
    SET(ARCHIVER_CARGS a -y -m0=lzma2 -mx=9) # Create archive args
    SET(ARCHIVER_LARGS l -y)                 # List archive args
ENDIF("${ARCHIVER_EXE}" STREQUAL "ARCHIVER_EXE-NOTFOUND")

#-----------------------------------------------------------------------------
# Create the list of archived data targets.
#
# Each archived target extracts data into a directory matching the root name
# of the source tarball. This makes tracking the dependencies simple.
#
#-----------------------------------------------------------------------------
FILE(GLOB ARCHIVED_TARGETS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *test_data*.7z)

#-----------------------------------------------------------------------------
# Add a custom command that extracts all archived dataset *and* collect
# output directory names in a dependency list.
#
# Note 1: Although we could also define convenience targets with the expanded
# names of the UNarchived directories (e.g. silo_test_data), doing so
# signifcantly increases the time for CMake to complete its configuring and
# generating phases. This is because it winds up populuating several makefile
# snipits in files in subdirs of ./CMakeFiles for each such target.
# In addition, such targets are useful only for archives known at cmake-time.
# It does not help, for example, when adding a new archive here.
#
# Note 2: With traditional Make, this could have been easily handled as a
# pattern rule of the form:
#   %_test_data : %_test_data.7z
#       $(ARCHIVER_CMD) $(ARCHIVER_XARGS) $<
# which is also processed at make-time, not configure/cmake-time.
# But not all target platforms understand or use make pattern rules.    
#-----------------------------------------------------------------------------
SET(DATA_TARGET_DEPENDS "")
FOREACH(DATASET_TARGET_FILE ${ARCHIVED_TARGETS})
    STRING(REPLACE ".7z" "" DATASET_TARGET "${DATASET_TARGET_FILE}")
    ADD_CUSTOM_COMMAND(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${DATASET_TARGET}
        COMMAND ${ARCHIVER_CMD}
        ARGS ${ARCHIVER_XARGS} ${CMAKE_CURRENT_SOURCE_DIR}/${DATASET_TARGET_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    SET(DATA_TARGET_DEPENDS ${DATA_TARGET_DEPENDS} ${CMAKE_CURRENT_BINARY_DIR}/${DATASET_TARGET})
ENDFOREACH()

#-----------------------------------------------------------------------------
# Add custom targets: "data" & "testdata"
#-----------------------------------------------------------------------------
ADD_CUSTOM_TARGET(data DEPENDS ${DATA_TARGET_DEPENDS})
ADD_CUSTOM_TARGET(testdata)
ADD_DEPENDENCIES(testdata data)

#-----------------------------------------------------------------------------
# CMake variable names for make variables used in coneneience targets below.
# Note the {} surrounding the make variables allowing them to be expanded at
# make time, not cmake time.
#-----------------------------------------------------------------------------
SET(ARCHIVE_NAME {ANAME})
SET(ARCHIVE_FILES {AFILES})

#-----------------------------------------------------------------------------
# Define convenience command/target to create an archive
#-----------------------------------------------------------------------------
ADD_CUSTOM_COMMAND(
    OUTPUT _archive_create
    COMMAND ${ARCHIVER_EXE} ${ARCHIVER_CARGS} $${ARCHIVE_NAME} $${ARCHIVE_FILES})
ADD_CUSTOM_TARGET(archive DEPENDS _archive_create)

#-----------------------------------------------------------------------------
# Define convenience command/target to expand an archive
#-----------------------------------------------------------------------------
ADD_CUSTOM_COMMAND(
    OUTPUT _archive_expand
    COMMAND ${ARCHIVER_EXE} ${ARCHIVER_XARGS} $${ARCHIVE_NAME})
ADD_CUSTOM_TARGET(expand DEPENDS _archive_expand)

#-----------------------------------------------------------------------------
# Define convenience command/target to list an archive
#-----------------------------------------------------------------------------
ADD_CUSTOM_COMMAND(
    OUTPUT _archive_list
    COMMAND ${ARCHIVER_EXE} ${ARCHIVER_LARGS} $${ARCHIVE_NAME})
ADD_CUSTOM_TARGET(list DEPENDS _archive_list)

#-----------------------------------------------------------------------------
# Define target for help on 7z
#-----------------------------------------------------------------------------
ADD_CUSTOM_TARGET(
    7z-more-help VERBATIM
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "             More help using 7z archiver/compresser"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "7z (or 7-Zip) is an archiving and compression tool that replaces"  
    COMMAND ${CMAKE_COMMAND} -E echo "the combination of tools like tar and gzip (.tar.gz or .tgz) and"
    COMMAND ${CMAKE_COMMAND} -E echo "often does a better job at compression. 7z supports a variety of"
    COMMAND ${CMAKE_COMMAND} -E echo "archival and comression file formats including .tar.gz. Here are"
    COMMAND ${CMAKE_COMMAND} -E echo "examples of command-lines to use 7z. Sometimes the 7z executable"
    COMMAND ${CMAKE_COMMAND} -E echo "is named 7za, p7zip or 7-Zip. See www.7-zip.org for more info."
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "creating an archive for VisIt test data"
    COMMAND ${CMAKE_COMMAND} -E echo "  7z a -m0=lzma2 -mx=9 <archive-file-name>.7z <file1>|<dir1>|<file2>..."
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "expand an archive"
    COMMAND ${CMAKE_COMMAND} -E echo "  7z x <archive-file-name>.7z"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "list contents of an archive"
    COMMAND ${CMAKE_COMMAND} -E echo "  7z l <archive-file-name>.7z"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "For VisIt test data archives, we want maximal compression and this"
    COMMAND ${CMAKE_COMMAND} -E echo "is usually best acchived using ultra compression (-mx=9) and lzma2"
    COMMAND ${CMAKE_COMMAND} -E echo "algorithm (-m0=lzma2). Also, we require compression be used on any"
    COMMAND ${CMAKE_COMMAND} -E echo "single file or collections of related files exceeding 1Mb in size."
    COMMAND ${CMAKE_COMMAND} -E echo "It is a best practice to put all related files in a directory named"
    COMMAND ${CMAKE_COMMAND} -E echo "<format>_test_data where <format> (e.g. vtk_test_data) is a moniker"
    COMMAND ${CMAKE_COMMAND} -E echo "indicating the data format and then archive that single directory."
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "")

#-----------------------------------------------------------------------------
# Define help target. Note, in later versions of CMake, CMP0037 needs to be
# set to old.
#-----------------------------------------------------------------------------
ADD_CUSTOM_TARGET(
    7z-help VERBATIM
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "               Conveninent make targets:"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "To create an archive..."
    COMMAND ${CMAKE_COMMAND} -E echo "  make ANAME=<arname> AFILES=... archive"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "  ANAME is the name of the archive without the extension (e.g .7z)"
    COMMAND ${CMAKE_COMMAND} -E echo "  AFILES can a directory name, a file name, or space separated"
    COMMAND ${CMAKE_COMMAND} -E echo "    list of files and/or directories bracketed in quotes."
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "To expand a single, specific archive..."
    COMMAND ${CMAKE_COMMAND} -E echo "  make ANAME=<arname> expand"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "To list contents of a single, specific archive..."
    COMMAND ${CMAKE_COMMAND} -E echo "  make ANAME=<arname> list"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Examples..."
    COMMAND ${CMAKE_COMMAND} -E echo "  make ANAME=silo_hdf5_test_data.7z list"
    COMMAND ${CMAKE_COMMAND} -E echo "  make ANAME=new_archive AFILES=\"*.txt\" archive"
    COMMAND ${CMAKE_COMMAND} -E echo "  make ANAME=foodir_archive AFILES=foodir archive"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "To get more help on using the 7z archiver"
    COMMAND ${CMAKE_COMMAND} -E echo "  make 7z-more-help"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "")
