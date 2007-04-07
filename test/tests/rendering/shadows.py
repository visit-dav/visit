# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  shadows.py
#
#  Tests:      mesh      - 2D/3D rectilinear, one domain
#              plots     - pseudocolor, surface
#              operators - isosurface
#              selection - no
#
#  Defect ID:  5539, 7068
#
#  Programmer: Hank Childs
#  Date:       March 3, 2006
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

OpenDatabase("../data/noise.silo")
AddPlot("Pseudocolor", "hardyglobal")

AddOperator("Isosurface")
i = IsosurfaceAttributes()
i.contourNLevels = 1
SetOperatorOptions(i)

r = RenderingAttributes()
r.scalableActivationMode = r.Always
r.doShadowing = 1
SetRenderingAttributes(r)
l = GetLight(0)
l.direction = (-0.707, 0, -0.707)
SetLight(0, l)
DrawPlots()
Test("shadows_01")

r.shadowStrength = 0.9
SetRenderingAttributes(r)
Test("shadows_02")

# Now test that we don't crash with 2D.
DeleteAllPlots()
OpenDatabase("../data/curv2d.silo")
AddPlot("Pseudocolor", "d")
DrawPlots()
Test("shadows_03")

Exit()
