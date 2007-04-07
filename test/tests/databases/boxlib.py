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
# ----------------------------------------------------------------------------

# Turn off all annotation

a = AnnotationAttributes()
a.axesFlag2D = 0
a.axesFlag = 0
a.triadFlag = 0
a.bboxFlag = 0
a.userInfoFlag = 0
a.databaseInfoFlag = 0
a.legendInfoFlag = 0
a.backgroundMode = 0
a.foregroundColor = (0, 0, 0, 255)
a.backgroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)

OpenDatabase("../data/boxlib_test_data/2D/plt0822/Header")
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

Exit()


