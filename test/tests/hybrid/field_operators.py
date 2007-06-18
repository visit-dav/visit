# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  field_operators.py
#
#  Tests:      mesh      - rectilinear, curvilinear
#              plots     - vector, contour
#
#  Defect ID:  '4141, '5343, '6297, '6460, '7063, '8025
#
#  Programmer: Hank Childs
#  Date:       December 13, 2003
#
#  Modifications:
#    Jeremy Meredith, Thu Jun 24 12:58:06 PDT 2004
#    Set the vector origin explicitly for some cases because I changed the
#    default to Tail.
#
#    Hank Childs, Mon Jan  3 10:35:56 PST 2005
#    Renamed to field_operators.  Added testing for divergence, Laplacian,
#    and curl.
#
#    Hank Childs, Mon Jun  6 11:28:45 PDT 2005
#    Add tests for 2D divergence, Laplacian.
#
#    Hank Childs, Mon Aug 15 14:19:49 PDT 2005
#    2D curl now produces a scalar.  Change our test appropriately.
#
#    Hank Childs, Fri Mar  3 09:01:08 PST 2006
#    Add test for Jacobian (2D determinants).  '7063
#
#    Hank Childs, Mon Jun 18 09:54:09 PDT 2007
#    Add test for dot of curl with a vector.  This is really testing the
#    ability of macro expressions to maintain secondary variables. '8025
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

OpenDatabase("../data/globe.silo")

DefineVectorExpression("grad_t", "gradient(t)")
AddPlot("Vector", "grad_t")
v = VectorAttributes()
v.vectorOrigin = v.Middle
SetPlotOptions(v)
DrawPlots()

# Zonal UCD gradient.
Test("field_op_01")
DeleteAllPlots()

DefineVectorExpression("grad_u", "gradient(u)")
AddPlot("Vector", "grad_u")
v = VectorAttributes()
v.vectorOrigin = v.Middle
SetPlotOptions(v)
DrawPlots()

# Nodal UCD gradient.
Test("field_op_02")
DeleteAllPlots()

OpenDatabase("../data/rect2d.silo")

DefineVectorExpression("grad_d", "gradient(d)")
AddPlot("Vector", "grad_d")
v = VectorAttributes()
v.colorByMag = 1
v.vectorOrigin = v.Middle
SetPlotOptions(v)
DrawPlots()

# Zonal rect-2d gradient.
Test("field_op_03")
DeleteAllPlots()

AddPlot("Vector", "grad_u")
v = VectorAttributes()
v.colorByMag = 1
v.vectorOrigin = v.Middle
SetPlotOptions(v)
DrawPlots()

# Nodal rect-2d gradient.
Test("field_op_04")
DeleteAllPlots()

OpenDatabase("../data/rect3d.silo")

AddPlot("Vector", "grad_d")
v = VectorAttributes()
v.nVectors = 447
v.vectorOrigin = v.Middle
SetPlotOptions(v)
DrawPlots()

# Zonal rect-3d gradient.
Test("field_op_05")
DeleteAllPlots()

AddPlot("Vector", "grad_u")
v = VectorAttributes()
v.vectorOrigin = v.Middle
SetPlotOptions(v)
DrawPlots()

# Nodal rect-3d gradient.
Test("field_op_06")
DeleteAllPlots()

AddPlot("Vector", "vel")
DefineScalarExpression("mag", "magnitude(vel)")
AddOperator("Threshold")
t = ThresholdAttributes()
t.lowerBounds = (0.5)
t.upperBounds = (1.5)
t.listedVarNames = ("mag")
SetOperatorOptions(t)
DrawPlots()

# Test vector with threshold ('4178)
Test("field_op_07")
DeleteAllPlots()


AddPlot("Vector", "grad_u")
v = VectorAttributes()
v.vectorOrigin = v.Middle
SetPlotOptions(v)
DefineScalarExpression("u_mag", "magnitude(grad_u)")
AddOperator("Isosurface")
iso = IsosurfaceAttributes()
iso.contourNLevels = 3
iso.variable = "u_mag"
SetOperatorOptions(iso)
DrawPlots()

v = GetView3D()
v.imageZoom = 4.5
SetView3D(v)

# Test vector with isosurface ('4207)
Test("field_op_08")

# Add a contour plot just for grins to show that our isosurface operator is
# picking up the same surface as the contour plot.
AddPlot("Contour", "u_mag")
con = ContourAttributes()
con.contourNLevels = 3
SetPlotOptions(con)
DrawPlots()

Test("field_op_09")
DeleteAllPlots()

# Test that we can calculate the Laplacian.
OpenDatabase("../data/rect3d.silo")

v = GetView3D()
v.imageZoom = 1
v.viewNormal = (0.41, 0.08, -0.91)
SetView3D(v)

DefineScalarExpression("LP", "abs(Laplacian(u))")
AddPlot("Contour", "LP")
c = ContourAttributes()
c.scaling = c.Log
SetPlotOptions(c)
DrawPlots()
Test("field_op_10")

DefineScalarExpression("div", "divergence(vel)")
DeleteAllPlots()
AddPlot("Contour", "div")
c = ContourAttributes()
c.scaling = c.Log
SetPlotOptions(c)
DrawPlots()
Test("field_op_11")


# Test multiple macro expressions together -- divergence and Laplacian.
DeleteAllPlots()
AddPlot("Pseudocolor", "div")
pc = PseudocolorAttributes()
pc.scaling = pc.Log
SetPlotOptions(pc)
AddOperator("Isosurface")
iso = IsosurfaceAttributes()
iso.scaling = iso.Log
iso.variable = "LP"
SetOperatorOptions(iso)
DrawPlots()

Test("field_op_12")


DefineVectorExpression("curl", "curl(vel)")
DeleteAllPlots()
AddPlot("Vector", "curl")
v = VectorAttributes()
v.scaleByMagnitude = 0
SetPlotOptions(v)
DrawPlots()
Test("field_op_13")

DeleteAllPlots()
OpenDatabase("../data/rect2d.silo")
DefineScalarExpression("div2", "divergence(vel)")
AddPlot("Pseudocolor", "div2")
DrawPlots()
Test("field_op_14")

DeleteAllPlots()
DefineScalarExpression("LP2", "Laplacian(d)")
AddPlot("Pseudocolor", "LP2")
DrawPlots()
Test("field_op_15")

DeleteAllPlots()
DefineScalarExpression("curl2", "curl(vel)")
AddPlot("Pseudocolor", "curl2")
DrawPlots()
Test("field_op_16")

DeleteAllPlots()
DefineScalarExpression("jacobian", "determinant({gradient(u), gradient(v)})")
AddPlot("Pseudocolor", "jacobian")
DrawPlots()
Test("field_op_17")

OpenDatabase("../data/rect3d.silo")
DefineVectorExpression("mycurl", "curl(vel)")
DefineScalarExpression("mydot", "dot(curl,vel)")
DeleteAllPlots()
AddPlot("Pseudocolor", "mydot")
DrawPlots()
Test("field_op_18")

Exit()
