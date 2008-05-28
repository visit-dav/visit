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
