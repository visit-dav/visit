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
RequiredDatabasePlugin("Exodus")

OpenDatabase(data_path("exodus_test_data/balls.e.4.* database"))

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

ToggleMaintainViewMode()
d = DisplaceAttributes()
d.variable = "DISPL"
SetDefaultOperatorOptions(d)
AddOperator("Displace")
DrawPlots()
Test("exodus_05")
ToggleMaintainViewMode()

TestSection("Testing SHELL4 support")
DeleteAllPlots()
CloseDatabase(data_path("exodus_test_data/balls.e.4.* database"))
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

AddPlot("Subset", "ElementBlock")
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

DeleteAllPlots()
CloseDatabase(data_path("exodus_test_data/porflow5_2_1_r1.exo"))
OpenDatabase(data_path("exodus_test_data/kassbohm1.exo"))
AddPlot("Mesh","Mesh")
DrawPlots()
v = View3DAttributes()
v.viewNormal = (-0.664463, 0.34202, 0.664463)
v.focus = (1, 1, 0.75)
v.viewUp = (0.241845, 0.939693, -0.241845)
v.parallelScale = 3.25
v.nearPlane = -6.5
v.farPlane = 6.5
v.imageZoom = 1.5
SetView3D(v)
Test("exodus_09")

DeleteAllPlots()
CloseDatabase(data_path("exodus_test_data/kassbohm1.exo"))
OpenDatabase(data_path("exodus_test_data/dodecahedron.exo"))
AddPlot("Mesh","Mesh")
DrawPlots()
ResetView()
Test("exodus_10")

DeleteAllPlots()
CloseDatabase(data_path("exodus_test_data/dodecahedron.exo"))
OpenDatabase(data_path("exodus_test_data/tri3.exo"))
AddPlot("Mesh","Mesh")
AddPlot("Pseudocolor","elem_map")
DrawPlots()
ResetView()
Test("exodus_11")

DeleteAllPlots()
CloseDatabase(data_path("exodus_test_data/tri3.exo"))
OpenDatabase(data_path("exodus_test_data/tri6.exo"))
AddPlot("Mesh","Mesh")
AddPlot("Pseudocolor","elem_map")
DrawPlots()
ResetView()
Test("exodus_12")

DeleteAllPlots()
CloseDatabase(data_path("exodus_test_data/tri6.exo"))
OpenDatabase(data_path("exodus_test_data/exo-plugin-problem-kassbohm.exo"))
AddPlot("Pseudocolor","SP1_S11_avg")
DrawPlots()
ResetView()
Test("exodus_13a")
DeleteAllPlots()
AddPlot("Pseudocolor","SP3_S11_avg")
DrawPlots()
Test("exodus_13b")
DeleteAllPlots()
AddPlot("Mesh","Mesh")
AddPlot("Subset","ElementBlock")
DrawPlots()
ResetView()
Test("exodus_13c")

s = SILRestriction()
ebsets = s.SetsInCategory("ElementBlock")
s.TurnOffSet(ebsets[1]) # SHELL
SetPlotSILRestriction(s)
sa = SubsetAttributes()
sa.lineWidth = 4
SetPlotOptions(sa)
Test("exodus_14")

DeleteAllPlots()
CloseDatabase(data_path("exodus_test_data/exo-plugin-problem-kassbohm.exo"))
exoOpts = GetDefaultFileOpenOptions("Exodus")
exoOpts['Automagically Detect Compound Variables'] = 1
exoOpts['Use Material Convention'] = 1 # ALEGRA
SetDefaultFileOpenOptions("Exodus", exoOpts)
OpenDatabase(data_path("exodus_test_data/snl_data/mixed2d/explwire2d.exo.8.* database"))
AddPlot("FilledBoundary", "Materials")
DrawPlots()
ResetView()
Test("exodus_15")
TimeSliderSetState(5)
Test("exodus_16")
DeleteAllPlots()
AddPlot("Vector","VELOCITY")
DrawPlots()
Test("exodus_17")
TimeSliderNextState()
Test("exodus_18")

DeleteAllPlots()
CloseDatabase(data_path("exodus_test_data/snl_data/mixed2d/explwire2d.exo.8.* database"))
OpenDatabase(data_path("exodus_test_data/snl_data/mixed3d/coax3D-wedge-tm.exo.8.* database"))
AddPlot("FilledBoundary", "Materials")
DrawPlots()
ResetView()
Test("exodus_19")
TimeSliderSetState(4)
Test("exodus_20")

s = SILRestriction()
msets = s.SetsInCategory("Materials")
s.TurnOffSet(msets[1]) 
SetPlotSILRestriction(s)
Test("exodus_21")
s.TurnOnSet(msets[1]) 
s.TurnOffSet(msets[2]) 
SetPlotSILRestriction(s)
Test("exodus_22")

TestSection("High Order Elements")
DeleteAllPlots()
CloseDatabase(data_path("exodus_test_data/snl_data/mixed3d/coax3D-wedge-tm.exo.8.* database"))
OpenDatabase(data_path("exodus_test_data/sphere-HEX20.ex2"))
AddPlot("Pseudocolor","coordx")
DrawPlots()
ResetView()
Test("exodus_23")
AddOperator("Slice")
DrawPlots()
Test("exodus_24")

DeleteAllPlots()
CloseDatabase(data_path("exodus_test_data/sphere-HEX20.ex2"))
OpenDatabase(data_path("exodus_test_data/sphere-HEX27.ex2"))
AddPlot("Pseudocolor","coordx")
DrawPlots()
ResetView()
Test("exodus_25")
AddOperator("Slice")
DrawPlots()
Test("exodus_26")

DeleteAllPlots()
CloseDatabase(data_path("exodus_test_data/sphere-HEX27.ex2"))
OpenDatabase(data_path("exodus_test_data/abl_551/abl_5km_5km_1km_neutral.e.8.* database"))
AddPlot("Contour","velocity_magnitude")
ca = ContourAttributes()
ca.contourMethod = ca.Value
ca.contourValue = (7)
SetPlotOptions(ca)
DrawPlots()
ResetView()
Test("exodus_27")
v = GetView3D()
v.RotateAxis(1,-30)
SetView3D(v)
Test("exodus_28")
v.RotateAxis(1,-30)
SetView3D(v)
Test("exodus_29")

DeleteAllPlots()
CloseDatabase(data_path("exodus_test_data/abl_551/abl_5km_5km_1km_neutral.e.8.* database"))

Exit()
