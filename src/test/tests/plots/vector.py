# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  vector.py
#
#  Tests:      vector    - 3D rectilinear, unstructured single domain
#              plots     - vector, plus interactions with some operators.
#
#  Defect ID:  '4954
#
#  Programmer: Hank Childs
#  Date:       June 2, 2004
#
#  Modifications:
#    Jeremy Meredith, Thu Jun 24 12:58:06 PDT 2004
#    Set the vector origin explicitly for some cases because I changed the
#    default to Tail.
#
#    Jeremy Meredith, Tue Jul  8 12:54:58 EDT 2008
#    Added test for new limit to original cells/nodes only.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Cyrus Harrison, Thu Mar 25 09:57:34 PDT 2010
#    Added call(s) to DrawPlots() b/c of changes to the default plot state 
#    behavior when an operator is added.
#
# ----------------------------------------------------------------------------



OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Vector", "vel")
vector_atts = VectorAttributes()
vector_atts.autoScale = 0
SetPlotOptions(vector_atts)
DrawPlots()

v = GetView3D()
v.viewNormal = (-0.453335, 0.442704, 0.773628)
v.focus = (0, 0, 0)
v.viewUp = (0.256972, 0.895999, -0.362148)
v.viewAngle = 30
v.parallelScale = 17.3205
v.nearPlane = -34.641
v.farPlane = 34.641
v.imagePan = (0, 0)
v.imageZoom = 1.3354
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0, 0, 0)
SetView3D(v)

Test("vector_01")

#
# Test that we can thin the vector field correctly.
#
vector_atts.vectorOrigin = vector_atts.Middle
vector_atts.nVectors = 1005
SetPlotOptions(vector_atts)
Test("vector_02")

vector_atts.nVectors = 50
SetPlotOptions(vector_atts)
Test("vector_03")

vector_atts.useStride = 1
vector_atts.stride = 5
SetPlotOptions(vector_atts)
Test("vector_04")

vector_atts.stride = 1
SetPlotOptions(vector_atts)
Test("vector_05")

vector_atts.stride = 50
SetPlotOptions(vector_atts)
Test("vector_06")

#
# Test some rendering portions of the vector plot.
#
vector_atts.useStride = 0
vector_atts.nVectors = 400
vector_atts.scale = 0.5
SetPlotOptions(vector_atts)
Test("vector_07")

vector_atts.headOn = 0
SetPlotOptions(vector_atts)
Test("vector_08")

vector_atts.headOn = 1
vector_atts.colorByMag = 0
vector_atts.vectorColor = (255, 0, 255, 255)
SetPlotOptions(vector_atts)
Test("vector_09")


#
# Test zonal vectors.
#
vector_atts.colorByMag = 1
SetPlotOptions(vector_atts)
ChangeActivePlotsVar("disp")
Test("vector_10")


#
# Test that we can have other plots in the same window as the vector plot.
#
DefineScalarExpression("mag", "magnitude(disp)")
AddPlot("Pseudocolor", "mag")
AddOperator("Isovolume")
iso_atts = IsovolumeAttributes()
iso_atts.lbound = 3
SetOperatorOptions(iso_atts)
DrawPlots()
Test("vector_11")


#
# Test that vector plots still work when restricted to a slice, etc.
#

DeleteAllPlots()
OpenDatabase(silo_data_path("rect3d.silo"))


DefineVectorExpression("v1", "{ vel[0]*coord(quadmesh3d)[0], vel[1]*coord(quadmesh3d)[1], vel[2]*coord(quadmesh3d)[2] }")
DefineScalarExpression("mag2", "magnitude(v1)")

AddPlot("Vector", "v1")
vector_atts = VectorAttributes()
vector_atts.autoScale = 0
SetPlotOptions(vector_atts)
DrawPlots()

v.viewNormal = (-0.450822, 0.819923, -0.352824)
v.focus = (0.5, 0.5, 0.5)
v.viewUp = (0.868253, 0.311093, -0.386468)
v.viewAngle = 30
v.parallelScale = 0.866025
v.nearPlane = -1.73205
v.farPlane = 1.73205
v.imagePan = (0, 0)
v.imageZoom = 1.3354
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0, 0, 0)
SetView3D(v)
Test("vector_12")

iso_surf_atts = IsosurfaceAttributes()
iso_surf_atts.variable = "mag2"
iso_surf_atts.contourMethod = iso_surf_atts.Value
iso_surf_atts.contourValue = (0.2)
SetDefaultOperatorOptions(iso_surf_atts)
AddOperator("Isosurface")
DrawPlots()
Test("vector_13")

RemoveLastOperator()

slice_atts = SliceAttributes()
slice_atts.originType = slice_atts.Point
slice_atts.originPoint = (0.5, 0.5, 0.5)
slice_atts.normal = (1, 1, 1)
SetDefaultOperatorOptions(slice_atts)
AddOperator("Slice")
DrawPlots()
Test("vector_14")

slice_atts.project2d = 0
slice_atts.normal = (1, -0.5, 0.5)
SetOperatorOptions(slice_atts)
v.viewNormal = (-0.00395109, 0.999884, -0.0147199)
v.focus = (0.5, 0.5, 0.5)
v.viewUp = (0.987418, 0.00157403, -0.158122)
v.viewAngle = 30
v.parallelScale = 0.866025
v.nearPlane = -1.73205
v.farPlane = 1.73205
v.imagePan = (0, 0)
v.imageZoom = 1.3354
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0, 0, 0)
SetView3D(v)

Test("vector_15")

#
# Test the "limit vectors to original node/cell" option
#
DeleteAllPlots()
OpenDatabase(silo_data_path("rect2d.silo"))

DefineVectorExpression("cvel","recenter(vel)")
AddPlot("Vector", "vel", 1, 0)
AddPlot("Boundary", "mat1", 1, 0)
AddPlot("Mesh", "quadmesh2d", 1, 0)
SetActivePlots(0)
vec=VectorAttributes()
vec.useStride=1
vec.stride=1
vec.scale=0.1
v = GetView2D()
v.windowCoords=(0,.3,1,1.3)
SetView2D(v)
SetPlotOptions(vec)
TurnMaterialsOff(("3","7","8"))
DrawPlots()

vec.origOnly = 0
SetPlotOptions(vec)
Test("vector_16")

vec.origOnly = 1
SetPlotOptions(vec)
Test("vector_17")

ChangeActivePlotsVar("cvel")

vec.origOnly = 0
SetPlotOptions(vec)
Test("vector_18")

vec.origOnly = 1
SetPlotOptions(vec)
Test("vector_19")


Exit()
