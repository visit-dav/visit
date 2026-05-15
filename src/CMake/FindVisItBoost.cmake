# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Apr 9, 2025
#   Utilize visit_import_third_party.
#
#*************************************************************************

# Uses the BOOST_DIR hint from the config-site .cmake file

if(HAVE_NEKTAR_PP)
    visit_import_third_party(BOOST LIBS
        boost_chrono
        boost_iostreams
        boost_thread
        boost_date_time
        boost_filesystem
        boost_regex
        boost_system
        boost_timer
        boost_program_options)
else()
    visit_import_third_party(BOOST HEADER_ONLY)
endif()

