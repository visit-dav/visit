# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  transform.py
#
#  Tests:      mesh      - 3D unstructured, single domain,
#                          3D rectilinear, single domain.
#              plots     - pc, mesh, contour, vector
#              operators - transform
#              selection - none
#
#  Defect ID:  '6310, '6694, '6699
#
#  Programmer: Hank Childs
#  Date:       July 24, 2002
#
#  Modifications:
#    Kathleen Bonnell, Wed Sep  3 09:31:25 PDT 2003
#    Opaque mode for MeshPlot is ON by default now, so turn it off
#    for this test.
#
#    Kathleen Bonnell, Thu Sep  4 11:10:11 PDT 2003
#    Change the way MeshPlot's opaque mode is set (now an enum, not a bool).
#
#    Hank Childs, Wed Jun  8 15:36:22 PDT 2005
#    Added tests for coordinate system conversion.  ['6310]
#
#    Hank Childs, Tue Nov 15 15:59:00 PST 2005
#    Added tests for 2D transformations.  ['6694 '6699]
#
#    Kathleen Bonnell,  Fri Mar 28 15:24:18 PDT 2008
#    Added test for curve transformation.  ['8277]
#
#    Jeremy Meredith
#    Added test for linear transforms.
#
#    Jeremy Meredith, Mon Jul 14 11:17:29 EDT 2008
#    Allow the clipped vector plot to show all vectors (even ones not from
#    from original cells/nodes) to match the old behavior.
#
#    Jeremy Meredith, Thu Aug 13 13:34:42 EDT 2009
#    Made transform04 test be explicit about the coord sys it chooses.
#    Made transform05 use Cyl instead of Spher coords; it's a 2D problem!
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Kathleen Biagas, Thu Jul 11 08:20:24 PDT 2013
#    Removed legacy style annotation setting.
#
# ----------------------------------------------------------------------------

# Turn off all annotation
a = AnnotationAttributes()
TurnOffAllAnnotations(a)

OpenDatabase(silo_data_path("globe.silo"))


AddPlot("Pseudocolor", "u")
clip = ClipAttributes()
clip.plane1Status = 1
clip.SetPlane1Origin(0,0,0)
clip.SetPlane1Normal(-1,1,0)
clip.funcType = 0
SetDefaultOperatorOptions(clip)
AddOperator("Clip")

trans = TransformAttributes()
trans.doTranslate = 1
trans.translateX = -3
SetDefaultOperatorOptions(trans)
AddOperator("Transform")
DrawPlots()

AddPlot("Vector", "vel")
vec = VectorAttributes()
vec.origOnly = 0
SetPlotOptions(vec)

clip.SetPlane1Normal(1,-1,0)
SetDefaultOperatorOptions(clip)
AddOperator("Clip")

trans.translateX = -6
SetDefaultOperatorOptions(trans)
AddOperator("Transform")

DrawPlots()

Test("ops_transform01")
DeleteAllPlots()

OpenDatabase(silo_data_path("rect3d.silo"))


AddPlot("Mesh", "quadmesh3d")
m = MeshAttributes()
m.opaqueMode = m.Off
SetPlotOptions(m)
trans.doTranslate = 0
trans.translateX = 0
trans.doScale = 1
trans.scaleX = 4
trans.scaleZ = 3
SetDefaultOperatorOptions(trans)
AddOperator("Transform")
DrawPlots()

ResetView()
Test("ops_transform02")
DeleteAllPlots()

AddPlot("Contour", "d")
trans.doRotate = 1
trans.rotateAmount = 45
SetDefaultOperatorOptions(trans)
AddOperator("Transform")
DrawPlots()

ResetView()
Test("ops_transform03")

DeleteAllPlots()

a.axes3D.visible = 1
a.axes3D.bboxFlag = 1
SetAnnotationAttributes(a)

AddPlot("Contour", "d")
AddOperator("Transform")
trans = TransformAttributes() # Get reset version.
trans.transformType = trans.Coordinate
trans.inputCoordSys = trans.Cartesian
trans.outputCoordSys = trans.Spherical
SetOperatorOptions(trans)
DrawPlots()
SetViewExtentsType("actual")
ResetView()

Test("ops_transform04")

# '6699
DeleteAllPlots()
OpenDatabase(silo_data_path("rect2d.silo"))

AddPlot("Pseudocolor", "d")
AddOperator("Transform")
trans.transformType = trans.Coordinate
trans.inputCoordSys = trans.Cartesian
trans.outputCoordSys = trans.Cylindrical
SetOperatorOptions(trans)
DrawPlots()
Test("ops_transform05")

# '6694
DeleteAllPlots()
DefineVectorExpression("polar", "polar(quadmesh2d) - coord(quadmesh2d)")
AddPlot("Pseudocolor", "d")
AddOperator("Displace")
disp = DisplaceAttributes()
disp.variable = "polar"
SetOperatorOptions(disp)
DrawPlots()
Test("ops_transform06")

# '8277
DeleteAllPlots()
OpenDatabase(data_path("curve_test_data/zonecent.ultra"))

AddPlot("Curve", "curve2")
DrawPlots()
Test("ops_transform07")

AddOperator("Transform")
trans.transformType = trans.Similarity
trans.doRotate = 0
trans.doTranslate = 0
trans.doScale = 1
trans.scaleX = 1
trans.scaleZ = 1
trans.scaleY = -3
SetOperatorOptions(trans)
DrawPlots()
Test("ops_transform08")

# Test linear transform
DeleteAllPlots()
OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Pseudocolor", "u")
AddOperator("Transform")

trans.transformType = trans.Linear
trans.m00 = .8
trans.m01 = .3
trans.m02 = 0

trans.m10 = 0
trans.m11 = 1
trans.m12 = 0

trans.m20 = .5
trans.m21 = .5
trans.m22 = 1

trans.invertLinearTransform = 0

SetOperatorOptions(trans)
DrawPlots()
ResetView()

Test("ops_transform09")

trans.invertLinearTransform = 1
SetOperatorOptions(trans)
Test("ops_transform10")

Exit()
