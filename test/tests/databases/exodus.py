# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  exodus.py
#
#  Tests:      mesh      - 2D, 3D unstructured
#              plots     - 3D - Pseudocolor, mesh, boundary, subset,
#                          2D - Filled boundary, label 
#
#  Defect ID:  none
#
#  Programmer: Hank Childs
#  Date:       March 31, 2005
#
#  Modifications:
#
#    Hank Childs, Fri Oct  7 14:27:33 PDT 2005
#    Explicitly reference the name of the variable to displace by, since the
#    default changed.
#
#    Brad Whitlock, Mon Nov 14 14:31:46 PST 2005
#    Added testing for SHELL4 support.
#
#    Eric Brugger, Wed Mar 14 14:04:32 PDT 2007
#    Added testing for element block names.
#
#    Cyrus Harrison, Thu Mar 25 09:57:34 PDT 2010
#    Added call(s) to DrawPlots() b/c of changes to the default plot state
#    behavior when an operator is added.
#
# ----------------------------------------------------------------------------

OpenDatabase(data_path("exodus_test_data/balls.exodus"))

AddPlot("Pseudocolor", "EQPS")
DrawPlots()

v = GetView3D()
v.viewNormal = (-0.75, 0.55, 0.36)
SetView3D(v)

Test("exodus_01")

s = SILRestriction()
sets = s.SetsInCategory("ElementBlock")
s.TurnOffSet(sets[0])
s.TurnOffSet(sets[1])
s.TurnOffSet(sets[2])
SetPlotSILRestriction(s)

Test("exodus_02")

SetTimeSliderState(18)
Test("exodus_03")

files = s.SetsInCategory("File")
s.TurnOffSet(files[3])
SetPlotSILRestriction(s)
Test("exodus_04")

d = DisplaceAttributes()
d.variable = "DISPL"
SetDefaultOperatorOptions(d)
AddOperator("Displace")
DrawPlots()
Test("exodus_05")


TestSection("Testing SHELL4 support")
DeleteAllPlots()
CloseDatabase(data_path("exodus_test_data/balls.exodus"))
OpenDatabase(data_path("exodus_test_data/aircraft.exoII"))

AddPlot("Mesh", "Mesh")
DrawPlots()
v = View3DAttributes()
v.viewNormal = (-0.51221, 0.429119, 0.743974)
v.focus = (0, 0, -4)
v.viewUp = (0.222628, 0.902964, -0.367549)
v.viewAngle = 30
v.parallelScale = 66
v.nearPlane = -132
v.farPlane = 132
v.imagePan = (-0.0367114, 0.0152384)
v.imageZoom = 1.51675
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0, 0, -4)
SetView3D(v)
Test("exodus_06")

TestSection("Testing support element block names")
DeleteAllPlots()
CloseDatabase(data_path("exodus_test_data/aircraft.exoII"))
OpenDatabase(data_path("exodus_test_data/test.exo"))

AddPlot("FilledBoundary", "ElementBlock")
AddPlot("Label", "ElementBlock")
DrawPlots()
Test("exodus_07")

TestSection("Testing 2D \"NSIDED\" element type")
DeleteAllPlots()
CloseDatabase(data_path("exodus_test_data/test.exo"))
OpenDatabase(data_path("exodus_test_data/porflow5_2_1_r1.exo"))

AddPlot("Mesh","Mesh")
DrawPlots()
v = View2DAttributes()
v.windowCoords = (-115, 70, -80, 100)
SetView2D(v)
Test("exodus_08")

CloseDatabase(data_path("exodus_test_data/porflow5_2_1_r1.exo"))

Exit()


