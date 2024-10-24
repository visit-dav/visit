# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  colortable.py
#
#  Tests:      mesh      - 3D unstructured, single domain
#              plots     - Pseudocolor, Mesh
#
#  Notes:      This test case uses a session file to create a plot that uses
#              a specific built-in color table. This makes sure that choice
#              of color tables is preserved with session files.
# 
# OUTDATED
#  Notes:      This test case uses a session file to create a plot that uses
#              a user defined color table. This makes sure that user defined
#              color tables can be used from session files. VisIt00004000.
#
#  Programmer: Brad Whitlock
#  Date:       Thu Nov 13 17:10:35 PST 2003
#
#  Modifications:
#      Justin Privitera, Wed Aug  3 15:04:31 PDT 2022
#      Changed the session file and the purpose of the test to reflect that
#      we no longer save info about color tables to or read info about color 
#      tables from session files.
#
# ----------------------------------------------------------------------------

TurnOnAllAnnotations()
RestoreSessionWithDifferentSources(tests_path("session","colortable.session"), 0,
                                   silo_data_path("globe.silo"))
Test("colortable00")
Exit()
