# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  specular.py
#
#  Tests:      mesh      - 2D/3D rectilinear, one domain
#              plots     - pseudocolor, surface
#              operators - isosurface
#              selection - no
#
#  Defect ID:  5555, 5580
#
#  Programmer: Hank Childs
#  Date:       November 2, 2004
#
#  Modifications:
#
#    Hank Childs, Wed Nov  3 16:41:56 PST 2004
#    Commented out specular_11, since it was failing in SR mode.  Submitted
#    new ticket, '5580 to address this.
#
#    Hank Childs, Fri Dec  9 08:01:46 PST 2005
#    Uncomment specular_11.
#
# ----------------------------------------------------------------------------

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'

OpenDatabase("../data/noise.silo")
AddPlot("Pseudocolor", "hardyglobal")

AddOperator("Isosurface")
i = IsosurfaceAttributes()
i.contourNLevels = 1
SetOperatorOptions(i)

DrawPlots()
Test("specular_01")

r = GetRenderingAttributes()
r.specularFlag = 1
SetRenderingAttributes(r)
Test("specular_02")

# Highly concentrated, low power
r.specularCoeff = 0.9
r.specularPower = 100
SetRenderingAttributes(r)
Test("specular_03")

# Poorly concentrated, high power
r.specularCoeff = 0.1
r.specularPower = 1
SetRenderingAttributes(r)
Test("specular_04")

# Test that specular is properly turned off when PC's lighting is turned off.
pc_atts = PseudocolorAttributes()
pc_atts.lightingFlag = 0
SetPlotOptions(pc_atts)
Test("specular_05")

# Make sure that specular is not affecting 2D plots.
DeleteAllPlots()
OpenDatabase("../data/rect2d.silo")
AddPlot("Pseudocolor", "d")
DrawPlots()
Test("specular_06")

# Now do the same things over again with the surface plot.
DeleteAllPlots()
r.specularFlag = 0
SetRenderingAttributes(r)
OpenDatabase("../data/noise.silo")
AddPlot("Surface", "hgslice")
DrawPlots()

v = GetView3D()
v.viewNormal = (-0.35795, -0.730451, 0.581647)
v.focus = (0, 0, 10)
v.viewUp = (0.0642975, 0.60216, 0.795782)
v.viewAngle = 30
v.parallelScale = 17.3205
v.nearPlane = -34.641
v.farPlane = 34.641
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
SetView3D(v)
Test("specular_07")

r = GetRenderingAttributes()
r.specularCoeff = 0.6
r.specularPower = 10
r.specularFlag = 1
SetRenderingAttributes(r)
Test("specular_08")

# Highly concentrated, low power
r.specularCoeff = 0.9
r.specularPower = 100
SetRenderingAttributes(r)
Test("specular_09")

# Poorly concentrated, high power
r.specularCoeff = 0.1
r.specularPower = 1
SetRenderingAttributes(r)
Test("specular_10")

# Test that specular is properly turned off when PC's lighting is turned off.
s_atts = SurfaceAttributes()
s_atts.lightingFlag = 0
SetPlotOptions(s_atts)
Test("specular_11")

Exit()
