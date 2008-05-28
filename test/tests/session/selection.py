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
#
# ----------------------------------------------------------------------------

TurnOnAllAnnotations()
RestoreSession("tests/session/selection-pre1.3.session", 0)
Test("selection_00")

RestoreSession("tests/session/selection-post1.3.session", 0)
Test("selection_01")

Exit()
