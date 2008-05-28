# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  sessionview.py
#
#  Tests:      mesh      - 3D unstructured, 3D rectilinear, single domain
#              plots     - Pseudocolor, Mesh
#              operators - Reflect
#
#  Notes:      This test case makes sure that we can switch back and forth
#              between session files that use expressions.
#
#  Programmer: Brad Whitlock
#  Date:       Wed Dec 31 15:21:23 PST 2003
#
#  Modifications:
#
# ----------------------------------------------------------------------------

TurnOnAllAnnotations()

# Open the first session file
RestoreSession("tests/session/sessionview1.session", 0)
Test("sessionview00")

# Open the second session file
RestoreSession("tests/session/sessionview2.session", 0)
Test("sessionview01")

# Open the first session file again. This used to crash VisIt
RestoreSession("tests/session/sessionview1.session", 0)
Test("sessionview02")

# Open the second session file again.
RestoreSession("tests/session/sessionview2.session", 0)
Test("sessionview03")

# Open the first session file
RestoreSession("tests/session/sessionview1.session", 0)
Test("sessionview04")

Exit()
