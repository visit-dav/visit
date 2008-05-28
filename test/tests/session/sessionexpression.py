# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  sessionexpression.py
#
#  Tests:      mesh      - 3D unstructured, single domain
#              plots     - Pseudocolor
#
#  Notes:      This test case creates a plot of a user-defined expression
#              using a session file to ensure that session files containing
#              plots of user-defined expressions work. This test case tests
#              for a bug reported in VisIt00006070.
#
#  Programmer: Brad Whitlock
#  Date:       Tue Apr 5 14:26:56 PST 2005
#
#  Modifications:
#
# ----------------------------------------------------------------------------

TurnOnAllAnnotations()
RestoreSession("tests/session/sessionexpression.session", 0)
Test("sessionexpression00")
Exit()
