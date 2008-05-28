# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  defer_expr.py
#
#  Defect ID:  '2433, '1527, '6630, '6631, '6676, '6637
#
#  Programmer: Hank Childs
#  Date:       September 21, 2005
#
#  Modifications:
#
#    Hank Childs, Thu Dec 29 11:29:08 PST 2005
#    Widen color range of vector plot to sidestep precision issues between
#    optimized and non-optimized binaries.
#
#    Hank Childs, Thu Jan 17 15:44:06 PST 2008
#    Explicitly specify the color table for the vector plot.  This is an issue
#    because this test runs with "-config" (*not* "-noconfig") and the default
#    color table can get confused if the user has a color table in his .visit
#    directory that collides with the normal default ("hot").  So we're 
#    skirting that issue by explicitly specifying the color table.
#
# ----------------------------------------------------------------------------


OpenDatabase("../data/globe.silo")
DefineScalarExpression("en", "external_node(mesh1)")
AddPlot("Pseudocolor", "en")
AddOperator("Isovolume")
iso_atts = IsovolumeAttributes()
iso_atts.variable = "t"
iso_atts.lbound = 500
SetOperatorOptions(iso_atts)
AddOperator("Slice")
DrawPlots()

# Save a picture before deferring the expression evaluation.
Test("defer_expr_01")

# Test that the external node operator can find the external nodes
# on a 2D slice.
d = DeferExpressionAttributes()
d.exprs = ("en")
SetDefaultOperatorOptions(d)
AddOperator("DeferExpression")
Test("defer_expr_02")

# Now test that the external node expression can find the external nodes
# on a 3D dataset (before the slice).  This should be an identical picture
# to what's above (bar interpolation from the slice).
DemoteOperator(2)
DrawPlots()
Test("defer_expr_03")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Isovolume")
iso_atts = IsovolumeAttributes()
iso_atts.variable = "t"
iso_atts.ubound = 500
SetOperatorOptions(iso_atts)
AddOperator("DeferExpression")
AddOperator("Threshold")
thres_atts = ThresholdAttributes()
thres_atts.upperBounds = (0.5)
thres_atts.outputMeshType = 1
thres_atts.listedVarNames = ("en")
SetOperatorOptions(thres_atts)
DrawPlots()
Test("defer_expr_04")

DeleteAllPlots()

DefineScalarExpression("X1", "coord(mesh1)[0]")
DefineScalarExpression("X2", "coord(mesh1)[0]")
DefineScalarExpression("X3", "coord(mesh1)[0]")

AddPlot("Pseudocolor", "X1")

# Transform by 45 degrees.
t = TransformAttributes()
t.doRotate = 1
t.rotateAxis = (0, 1, 0)
t.rotateAmount = 45
SetDefaultOperatorOptions(t)
AddOperator("Transform")

# Now evaluate X2 after transforming
d.exprs = ("X2")
SetDefaultOperatorOptions(d)
AddOperator("DeferExpression")

# Now threshold out where X2 is bigger than 0 (after the transform)
thres_atts.listedVarNames = ("X2")
thres_atts.lowerBounds = (0.0)
thres_atts.upperBounds = (1000.0)
SetDefaultOperatorOptions(thres_atts)
AddOperator("Threshold")

# Now transform by another 45 degrees
AddOperator("Transform")

# Evaluate X3 after the new transform.
d.exprs = ("X3")
SetDefaultOperatorOptions(d)
AddOperator("DeferExpression")

# Now threshold out where X3 is bigger than 0 (after the new transform)
thres_atts.listedVarNames = ("X3")
SetDefaultOperatorOptions(thres_atts)
AddOperator("Threshold")
DrawPlots()

Test("defer_expr_05")

# Now do the same thing, but defer X1 until the end of the transforms.
d.exprs = ("X1")
SetDefaultOperatorOptions(d)
AddOperator("DeferExpression")
# This should be the same geometry as '05, but the color should be different.
Test("defer_expr_06")

DeleteAllPlots()
DefineVectorExpression("normals", "surface_normal(mesh1)")
AddPlot("Vector", "normals")
v = VectorAttributes()
v.min = 0.5
v.max = 1.5
v.minFlag = 1
v.maxFlag = 1
v.colorTableName = "hot"
SetPlotOptions(v)

AddOperator("ExternalSurface")
AddOperator("DeferExpression")
d.exprs = "normals"
SetOperatorOptions(d)
DrawPlots()
Test("defer_expr_07")

DeleteAllPlots()
AddPlot("Contour", "t")
c = ContourAttributes()
c.contourMethod = c.Value
c.contourValue = (500)
SetPlotOptions(c)
AddPlot("Vector", "normals")
v = VectorAttributes()
v.colorTableName = "hot"
SetPlotOptions(v)
AddOperator("Isosurface")
i = IsosurfaceAttributes()
i.contourMethod = i.Value
i.contourValue = (500)
i.variable = "t"
SetOperatorOptions(i)
AddOperator("DeferExpression")
d.exprs = "normals"
SetOperatorOptions(d)
DrawPlots()
Test("defer_expr_08")

DeleteAllPlots()
AddPlot("Pseudocolor", "mesh_quality/max_edge_length")
AddOperator("Slice")
DrawPlots()
Test("defer_expr_09")

d.exprs = ("mesh_quality/max_edge_length")
AddOperator("DeferExpression")
SetOperatorOptions(d)
Test("defer_expr_10")

DeleteAllPlots()
r = ResamplePluginAttributes()
r.samplesX = 50
r.samplesY = 50
r.samplesZ = 50
r.defaultValue = -100
r.distributedResample = 0
DefineScalarExpression("procid", "procid(mesh1)")
AddPlot("Pseudocolor", "procid")
AddOperator("Resample")
SetOperatorOptions(r)
AddOperator("Threshold")
t = ThresholdAttributes()
t.lowerBounds = (-10.0)
t.listedVarNames = ("u")
t.outputMeshType = 1
SetOperatorOptions(t)
AddOperator("DeferExpression")
d.exprs = "procid"
SetOperatorOptions(d)
DrawPlots()

Test("defer_expr_11")

r.distributedResample = 1
SetOperatorOptions(r)
Test("defer_expr_12")

DeleteAllPlots()
DefineVectorExpression("sn", "recenter(-surface_normal(mesh1))")
AddPlot("Vector", "sn")
v = VectorAttributes()
v.colorTableName = "hot"
SetPlotOptions(v)
AddOperator("ExternalSurface")
AddOperator("DeferExpression")
d.exprs = "sn"
SetOperatorOptions(d)
DrawPlots()
Test("defer_expr_13")

Exit()
