# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  maintainlimits.py
#
#  Tests:      mesh      - 2D curvilinear
#              plots     - pseudocolor, contour
#
#  Defect ID:  none
#
#  Programmer: Eric Brugger
#  Date:       April 8, 2004
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

OpenDatabase("../data/curv2d.silo")

# Test maintain data with the pseudocolor plot.
ToggleMaintainDataMode()
AddPlot("Pseudocolor", "p")
DrawPlots()

ChangeActivePlotsVar("u")
Test("maintaindata1")

ToggleMaintainDataMode()
Test("maintaindata2")

# Test maintain data with the contour plot.
DeleteAllPlots()
ToggleMaintainDataMode()
AddPlot("Contour", "p")
DrawPlots()

ChangeActivePlotsVar("u")
Test("maintaindata3")

ToggleMaintainDataMode()
Test("maintaindata4")

Exit()

