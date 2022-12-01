# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

# Use the BOOST_DIR hint from the config-site .cmake file

SET(BOOST_LIBS NO_LIBS)

IF(NEKTAR++_FOUND)
  SET(BOOST_LIBS
   boost_chrono
   boost_iostreams
   boost_thread
   boost_date_time
   boost_filesystem
   boost_regex
   boost_system
   boost_timer
   boost_program_options)
ENDIF()

SET_UP_THIRD_PARTY(BOOST LIBS ${BOOST_LIBS} )

if(BOOST_FOUND)
    set(HAVE_BOOST true CACHE BOOL "Have boost ")
endif()

