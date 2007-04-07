# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  colortable.py
#
#  Tests:      mesh      - 3D unstructured, single domain
#              plots     - Pseudocolor, Mesh
#
#  Notes:      This test case uses a session file to create a plot that uses
#              a user defined color table. This makes sure that user defined
#              color tables can be used from session files. VisIt00004000.
#
#  Programmer: Brad Whitlock
#  Date:       Thu Nov 13 17:10:35 PST 2003
#
#  Modifications:
#
# ----------------------------------------------------------------------------

RestoreSession("tests/session/colortable.session", 0)
Test("colortable00")
Exit()
