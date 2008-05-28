# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  rect3d-contour.py
#
#  Tests:      mesh      - 3D rectilinear, single domain
#              plots     - Contour, with transparent contours
#
#  Notes:      This test case uses a session file to ensure that the Contour
#              plot can be restored from a session file and have its correct
#              colors and opacities, from  a bug reported in VisIt00004115.
#
#  Programmer: Kathleen Bonnell 
#  Date:       January 13, 2005 
#
#  Modifications:
#
# ----------------------------------------------------------------------------

TurnOnAllAnnotations()
RestoreSession("tests/session/rect3d-contour.session", 0)
Test("rect3d-contour00")
Exit()
