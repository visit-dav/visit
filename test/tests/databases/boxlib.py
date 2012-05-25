# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  boxlib.py
#
#  Tests:      mesh      - 2D AMR
#              plots     - Pseudocolor, mesh, boundary, subset
#
#  Defect ID:  none
#
#  Programmer: Hank Childs
#  Date:       March 31, 2005
#
#  Modifications:
#  
#    Mark C. Miller, Mon Sep 15 12:05:44 PDT 2008
#    Added 3D test
# ----------------------------------------------------------------------------


OpenDatabase(data_path("boxlib_test_data/2D/plt0822/Header"))

AddPlot("Pseudocolor", "density")
AddPlot("Boundary", "materials")
b = BoundaryAttributes()
b.colorType = b.ColorBySingleColor
b.lineWidth = 2
SetPlotOptions(b)

AddPlot("Subset", "patches")
s = SubsetAttributes()
s.wireframe = 1
s.colorType = s.ColorBySingleColor
s.singleColor = (255, 255, 255, 128)
s.lineWidth = 3
SetPlotOptions(s)

DrawPlots()

v = GetView2D()
v.viewportCoords = (0, 1, 0, 1)
v.windowCoords = (0, 0.04, 0.09, 0.13)
SetView2D(v)

Test("boxlib_01")

SetActivePlots((0,1,2))
s = SILRestriction()
levels = s.SetsInCategory("levels")
level2 = levels[2]
s.TurnOffSet(level2)
SetPlotSILRestriction(s)
Test("boxlib_02")

s.TurnOnAll()
level0 = levels[0]
s.TurnOffSet(level0)
SetPlotSILRestriction(s)
Test("boxlib_03")

DeleteAllPlots()
CloseDatabase(data_path("boxlib_test_data/2D/plt0822/Header"))

OpenDatabase(data_path("boxlib_test_data/3D/plt_asgc05_0309/Header"))

AddPlot("Pseudocolor","tracer")
DrawPlots()
ResetView()
v=GetView3D()
v.RotateAxis(2,-45.0)
v.RotateAxis(0,65.0)
SetView3D(v)
Test("boxlib_04")

Exit()


