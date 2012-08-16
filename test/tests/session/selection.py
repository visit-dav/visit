# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  selection.py
#
#  Tests:      mesh      - 2D curvilinear, multiple domain
#              plots     - FilledBoundary, Mesh, Pseudocolor
#
#  Notes:      This test case makes sure that VisIt can read session files
#              that have subset selection. Session files from before 1.3 and
#              from version 1.3 are tested.
#
#  Programmer: Brad Whitlock
#  Date:       Thu Mar 25 08:34:37 PDT 2004
#
#  Modifications:
#    Brad Whitlock, Tue Jan 6 09:17:15 PDT 2009
#    I removed support for pre-1.3 session files.
#
# ----------------------------------------------------------------------------

TurnOnAllAnnotations()
RestoreSessionWithDifferentSources(tests_path("session","selection.session"), 0,
                                   silo_data_path("multi_curv2d.silo"))

Test("selection_00")

Exit()
