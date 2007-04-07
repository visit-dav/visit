# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  view3d.py
#
#  Tests:      mesh      - 3D unstructured, single domain
#              plots     - Pseudocolor
#
#  Defect ID:  visit00004724
#
#  Notes:      This test case makes sure that all the 3d view parameters
#              are properly restored from a session.  In particular the
#              focus, parallelScale, nearPlane and farPlane.
#
#  Programmer: Eric Brugger
#  Date:       Thu Apr 22 15:05:59 PDT 2004
#
# ----------------------------------------------------------------------------

# Open the session file
RestoreSession("tests/session/view3d.session", 0)
Test("view3d00")

Exit()
