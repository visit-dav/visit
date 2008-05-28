# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  claw.py 
#
#  Tests:      Claw files 
#
#  Programmer: Mark C. Miller 
#  Date:       September 13, 2007 
#
# ----------------------------------------------------------------------------


# test the mesh
OpenDatabase("../data/claw_test_data/2d/fort.claw")
TimeSliderNextState()
AddPlot("Mesh","claw_mesh")
DrawPlots()
SetActivePlots((1,))
silr = SILRestriction()
for i in range(silr.NumSets()):
    if silr.SetName(i) == "level3":
        silr.TurnOffSet(i)
SetPlotSILRestriction(silr)
Test("claw_01")
SetActivePlots((1,))
DeleteActivePlots()

# test a variable being read 
AddPlot("Pseudocolor","col_00")
DrawPlots()
Test("claw_02")
DeleteAllPlots()
CloseDatabase("../data/claw_test_data/2d/fort.claw")

OpenDatabase("../data/claw_test_data/3d/fort.claw")
TimeSliderNextState()
ss = SubsetAttributes()
#ss.colorType = ss.ColorBySingleColor
ss.lineWidth = 2 
ss.singleColor = (0, 0, 0, 255)
ss.wireframe = 1
SetPlotOptions(ss)
AddPlot("Pseudocolor","col_00")
AddPlot("Mesh","claw_mesh")
SetActivePlots((1,2))
#AddOperator("Clip", 1)
AddOperator("Clip")
clipAtts = ClipAttributes()
clipAtts.plane1Origin = (0.25, 0.5, 0.75)
clipAtts.plane1Normal = (0, 0, 1)
SetOperatorOptions(clipAtts)
DrawPlots()
SetActivePlots((0,))
DrawPlots()
v=GetView3D()
v.viewNormal = (0.383221, 0.582931, 0.716473)
v.viewUp = (-0.29125, 0.812382, -0.505182)
SetView3D(v)
Test("claw_03")

Exit()
