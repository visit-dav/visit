# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  surface.py
#
#  Tests:      mesh      - 2D structured, single domain
#                        - 2D rectilinear, single domain
#              plots     - surface
#
#  Defect ID:  none
#
#  Programmer: Kathleen Bonnell 
#  Date:       August 22, 2003
#
#  Modifications:
#    Kathleen Bonnell, Wed Sep  3 17:21:44 PDT 2003
#    Add tests 4-10, testing more options of the Surface plot. 
# ----------------------------------------------------------------------------




OpenDatabase("../data/curv2d.silo")

#Test default surface plot (surface on, wireframe off)
AddPlot("Surface", "u")
DrawPlots()
Test("surface_01")

#Turn on  Wireframe
s = SurfaceAttributes()
s.wireframeFlag=1
SetPlotOptions(s)
Test("surface_02")

#Turn off surface, leaving only wireframe
s.surfaceFlag=0
SetPlotOptions(s)
Test("surface_03")

DeleteAllPlots()

OpenDatabase("../data/noise2d.silo")
AddPlot("Surface", "shepardglobal")
DrawPlots()

v = GetView3D()
v.viewNormal = (-0.25, -0.95, -0.2)
v.focus = (0, 0, 10)
v.viewUp = (0.04, 0.2, -0.98)
v.imageZoom = 1.001
SetView3D(v)
Test("surface_04")

# Turn off lighting
s = SurfaceAttributes()
s.lightingFlag = 0
SetPlotOptions(s)
Test("surface_05")

# Lighting on, wireframe on, log scaling 
s.lightingFlag = 1
s.wireframeFlag = 1
s.scaling = s.Log
SetPlotOptions(s)
Test("surface_06")

# Skew scaling, skew factor = 100
s.scaling = s.Skew
s.skewFactor = 100
SetPlotOptions(s)
Test("surface_07")

# Skew scaling, skew factor = 0.001
s.skewFactor = 0.001
SetPlotOptions(s)
Test("surface_08")

# Solid color for surface
s.colorByZFlag = 0
s.surfaceColor = (0, 200, 122, 255)
SetPlotOptions(s)
Test("surface_09")

# change wire color, style, turn off surface
s.wireframeColor = (0, 0, 255, 255)
s.lineStyle = 2
s.surfaceFlag = 0
SetPlotOptions(s)
Test("surface_10")





Exit()
