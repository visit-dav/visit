# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  radial_resample.py
#
#  Tests:      mesh    - 2D rectilinear, single domain,
#                        3D rectilinear, single domain
#                        3D unstructured, multiple domain
#              plots   - pseudocolor
#
#  Defect ID:  1827 
#
#  Programmer: Kevin Griffin
#  Date:       Tue Jun  3 11:00:41 EST 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# 2D, Rectilinear
ds = silo_data_path("rect2d.silo")
OpenDatabase(ds)

# clean-up 1's
AddPlot("Mesh", "quadmesh2d", 1, 1) 
AddOperator("RadialResample")

RadialResampleAttrs = RadialResampleAttributes()
RadialResampleAttrs.isFast = 0
RadialResampleAttrs.minTheta = -45
RadialResampleAttrs.maxTheta = 90
RadialResampleAttrs.deltaTheta = 5
RadialResampleAttrs.radius = 0.5
RadialResampleAttrs.deltaRadius = 0.05
RadialResampleAttrs.center = (0.5, 0.5, 0.5)
RadialResampleAttrs.is3D = 0

SetOperatorOptions(RadialResampleAttrs)

AddPlot("Pseudocolor", "t", 1, 1)
DrawPlots()
Test("ops_radialresampleop_rect2d")

DeleteAllPlots()
CloseDatabase(ds)

#3D, Rectilinear
ds = silo_data_path("rect3d.silo")
OpenDatabase(ds)

AddPlot("Mesh", "quadmesh3d", 1, 1)
AddOperator("RadialResample")

RadialResampleAtts = RadialResampleAttributes()
RadialResampleAtts.isFast = 0
RadialResampleAtts.minTheta = -90
RadialResampleAtts.maxTheta = 90
RadialResampleAtts.deltaTheta = 5
RadialResampleAtts.radius = 0.5
RadialResampleAtts.deltaRadius = 0.05
RadialResampleAtts.center = (0.5, 0.5, 0.5)
RadialResampleAtts.is3D = 1
RadialResampleAtts.minAzimuth = 0
RadialResampleAtts.maxAzimuth = 360
RadialResampleAtts.deltaAzimuth = 5

SetOperatorOptions(RadialResampleAtts, 1)

AddPlot("Pseudocolor", "w", 1, 1)
DrawPlots()
Test("ops_radialresampleop_rect3d")

DeleteAllPlots()
CloseDatabase(ds)

#2D, Rectilinear, Multiple Domains
ds = silo_data_path("multi_rect2d.silo")
OpenDatabase(ds)

AddPlot("Mesh", "mesh1", 1, 1)
AddOperator("RadialResample", 1)

RadialResampleAtts = RadialResampleAttributes()
RadialResampleAtts.isFast = 0
RadialResampleAtts.minTheta = 0
RadialResampleAtts.maxTheta = 360
RadialResampleAtts.deltaTheta = 5
RadialResampleAtts.radius = 0.5
RadialResampleAtts.deltaRadius = 0.05
RadialResampleAtts.center = (0.5, 0.5, 0.5)
RadialResampleAtts.is3D = 0
RadialResampleAtts.minAzimuth = 0
RadialResampleAtts.maxAzimuth = 180
RadialResampleAtts.deltaAzimuth = 5

SetOperatorOptions(RadialResampleAtts, 1)
AddPlot("Pseudocolor", "vec_magnitude", 1, 1)

DrawPlots()
Test("ops_radialresampleop_multi_rect2d")

DeleteAllPlots()
CloseDatabase(ds)

# 3D, Rectilinear, Multiple Domains
ds = silo_data_path("multi_rect3d.silo")
OpenDatabase(ds)

AddPlot("Mesh", "mesh1", 1, 1)
AddOperator("RadialResample", 1)

RadialResampleAtts = RadialResampleAttributes()
RadialResampleAtts.isFast = 0
RadialResampleAtts.minTheta = -90
RadialResampleAtts.maxTheta = 90 
RadialResampleAtts.deltaTheta = 5
RadialResampleAtts.radius = 0.5
RadialResampleAtts.deltaRadius = 0.05
RadialResampleAtts.center = (0.5, 0.5, 0.5)
RadialResampleAtts.is3D = 1
RadialResampleAtts.minAzimuth = 0
RadialResampleAtts.maxAzimuth = 360 
RadialResampleAtts.deltaAzimuth = 5

SetOperatorOptions(RadialResampleAtts)

AddPlot("Pseudocolor", "w")
DrawPlots()
Test("ops_radialresampleop_multi_rect3d")

DeleteAllPlots()
CloseDatabase(ds)

Exit()
