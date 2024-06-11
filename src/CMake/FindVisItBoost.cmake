# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

# Use the BOOST_DIR hint from the config-site .cmake file

set(BOOST_LIBS NO_LIBS)

if(NEKTAR++_FOUND)
    set(BOOST_LIBS
        boost_chrono
        boost_iostreams
        boost_thread
        boost_date_time
        boost_filesystem
        boost_regex
        boost_system
        boost_timer
        boost_program_options)
endif()

SET_UP_THIRD_PARTY(BOOST LIBS ${BOOST_LIBS} )

