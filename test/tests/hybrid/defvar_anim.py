# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  defvar.py
#
#  Tests:      plots     - vector
#
#  Defect ID:  '3221
#
#  Programmer: Hank Childs
#  Date:       April 10, 2003
#
#  Modifications:
#    Jeremy Meredith, Thu Jun 24 12:58:06 PDT 2004
#    Set the vector origin explicitly for some cases because I changed the
#    default to Tail.
#
#    Brad Whitlock, Wed Mar 9 09:15:30 PDT 2005
#    Removed deprecated functions.
#
# ----------------------------------------------------------------------------



OpenDatabase("../data/wave.visit")


vec = VectorAttributes()
vec.vectorOrigin = vec.Middle
SetDefaultPlotOptions(vec)


AddPlot("Vector", "direction")
DrawPlots()

# Set the view
v = View3DAttributes()
v.viewNormal = (-0.293413, 0.703819, 0.646953)
v.focus = (5, 0.353448, 2.5)
v.viewUp = (0.228637, 0.708763, -0.667368)
v.viewAngle = 30
v.parallelScale = 3.87214
v.nearPlane = -11.2018
v.farPlane = 11.2018
v.perspective = 1
SetView3D(v)


Test("defvar_anim_01")

SetTimeSliderState(30)
Test("defvar_anim_02")

SetTimeSliderState(60)
Test("defvar_anim_03")

Exit()
