# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  point.py
#
#  Tests:      mesh      - 3D point
#              plots     - pseudocolor, subset, mesh
#              operators - none
#              selection - none
#
#  Defect ID:  none
#
#  Programmer: Jeremy Meredith
#  Date:       May  4, 2004
#
#  Modificatons:
#    Kathleen Bonnell, Thu Aug 19 15:07:23 PDT 2004
#    Added tests 4-7.
#
#    Kathleen Bonnell, Thu Nov 11 17:28:15 PST 2004 
#    Added tests 8-17.
#
#    Brad Whitlock, Thu Jul 21 15:44:05 PST 2005
#    Added tests for setting the point size for points rendered as pixels.
#
#    Brad Whitlock, Fri Aug 26 13:59:08 PST 2005
#    Needed to add more rendering tests so I moved the point rendering
#    stuff to renderpoint.py.
#
# ----------------------------------------------------------------------------

TurnOffAllAnnotations() # defines global object 'a'

v = GetView3D()
v.viewNormal = (0.3, 0.5, 0.8)
SetView3D(v)

OpenDatabase("../data/noise.silo")

AddPlot("Pseudocolor", "PointVar")
DrawPlots()

p=PseudocolorAttributes()
p.pointType = p.Axis
p.pointSize = 0.2
SetPlotOptions(p)
Test("meshtype_point_01")

p.pointSizeVarEnabled = 1
p.pointSize = 0.5
SetPlotOptions(p)
Test("meshtype_point_02")

p.pointType = p.Point
p.pointSizePixels = 1
SetPlotOptions(p)
Test("meshtype_point_03")

DeleteAllPlots()


#Test some more of the point-related atts for the pc plot.
OpenDatabase("../data/multi_point2d.silo")
AddPlot("Pseudocolor", "u")
p = PseudocolorAttributes()
SetPlotOptions(p)
DrawPlots()
ResetView()

Test("meshtype_point_04")

p.pointSize = 0.05
p.pointType = p.Axis
p.pointSizeVarEnabled = 1
p.pointSizeVar = "d"
SetPlotOptions(p)
Test("meshtype_point_05")

p.pointSize = 0.25
p.pointType = p.Box
p.pointSizeVar = "p"
SetPlotOptions(p)
Test("meshtype_point_06")

p.pointSize = 0.25
p.pointType = p.Icosahedron
p.pointSizeVar = "u"
SetPlotOptions(p)
Test("meshtype_point_07")

DeleteAllPlots()

#Test Subsets of point meshes
AddPlot("Subset", "domains")
s = SubsetAttributes()
s.pointSizePixels = 1
SetPlotOptions(s)
DrawPlots()
ResetView()
Test("meshtype_point_08")

s.pointSizeVarEnabled = 1
s.pointSizeVar = "d"
s.pointType = s.Icosahedron
SetPlotOptions(s)
Test("meshtype_point_09")

s.pointSizeVar = "u"
s.pointType = s.Box
SetPlotOptions(s)
Test("meshtype_point_10")

s.pointSizeVar = "v"
SetPlotOptions(s)
Test("meshtype_point_11")

s.colorType = s.ColorBySingleColor
s.singleColor = (255, 0, 0, 255)
SetPlotOptions(s)
Test("meshtype_point_12")

DeleteAllPlots()


OpenDatabase("../data/noise.silo")
AddPlot("Pseudocolor", "PointVar")
p.pointSize = 0.5
p.pointSizeVarEnabled = 0
SetPlotOptions(p)
DrawPlots()
Test("meshtype_point_13")

# the next two not only test using poingSizeVar, but also DataSetVerifier which
# will reduce the number of values in the var array to match the actual number 
# of points in the point mesh
p.pointSize = 0.5
p.pointSizeVarEnabled = 1
p.pointSizeVar = "hardyglobal"
p.pointType = p.Box
SetPlotOptions(p)
Test("meshtype_point_14")

#can we scale by a a vector variable?
p.pointSizeVar = "grad"
SetPlotOptions(p)
Test("meshtype_point_15")

DeleteAllPlots()

AddPlot("Mesh", "PointMesh")
m = MeshAttributes()
m.foregroundFlag = 0
m.meshColor = (0, 122, 200, 255)
m.pointType = m.Box
m.pointSize = 0.5
SetPlotOptions(m)
DrawPlots()
Test("meshtype_point_16")

m.pointSizeVarEnabled = 1
m.pointSizeVar = "PointVar"
SetPlotOptions(m)
Test("meshtype_point_17")

Exit()
