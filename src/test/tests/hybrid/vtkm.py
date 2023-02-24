# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  vtkm.py
#
#  Tests:      mesh      - 3D rectilinear, single domain
#                          3D curvilinear, single domain
#                          3D unstructured, single domain
#                          3D unstructured, multi domain
#              plots     - Contour, Pseudocolor
#              operators - Isosurface, Isovolume, Slice, Threshold
#
#  Programmer: Eric Brugger
#  Date:       Wed Sep 19 12:18:14 PDT 2018
#
#  Modifications:
#    James Kress, Tue Oct  5 14:00:28 PST 2021
#    Adding additional threshold tests.
#
#    Eric Brugger, Fri Feb 24 14:57:15 PST 2023
#    Added additional tests.
#
# ----------------------------------------------------------------------------

SetBackendType("vtkm")

#
# Test a 2d rectilinear mesh.
#
OpenDatabase(silo_data_path("rect2d.silo"))

AddPlot("Contour", "u")
DrawPlots()

Test("vtkm_rect2d_01")

DeleteAllPlots()

AddPlot("Contour", "d")
DrawPlots()

Test("vtkm_rect2d_02")

#
# Test a 3d rectilinear mesh.
#
DeleteAllPlots()

OpenDatabase(silo_data_path("rect3d.silo"))

AddPlot("Contour", "u")
DrawPlots()

v = View3DAttributes()
v.viewNormal = (0.4854, 0.3984, 0.7782)
v.focus = (0.5, 0.5, 0.5)
v.viewUp = (-0.1209, 0.9122, -0.3916)
v.parallelScale = 0.8660
v.nearPlane = -1.7321
v.farPlane = 1.7321
SetView3D(v)

Test("vtkm_rect3d_01")

DeleteAllPlots()

AddPlot("Contour", "d")
DrawPlots()

Test("vtkm_rect3d_01a")

DeleteAllPlots()

AddPlot("Pseudocolor", "d")
AddOperator("Slice")
atts = SliceAttributes()
atts.project2d = 0
atts.normal = (0., 1., 0.)
atts.originType = atts.Point
atts.originPoint = (0.5, 0.5, 0.5)
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_rect3d_02")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Slice")
atts = SliceAttributes()
atts.project2d = 0
atts.normal = (0., 1., 0.)
atts.originType = atts.Point
atts.originPoint = (0.5, 0.5, 0.5)
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_rect3d_03")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Isosurface")
atts = IsosurfaceAttributes()
atts.contourNLevels = 10
atts.contourMethod = atts.Level
atts.minFlag = 0
atts.maxFlag = 0
atts.variable = "u"
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_rect3d_04")

DeleteAllPlots()

AddPlot("Pseudocolor", "d")
AddOperator("Isosurface")
atts = IsosurfaceAttributes()
atts.contourNLevels = 10
atts.contourMethod = atts.Level
atts.minFlag = 0
atts.maxFlag = 0
atts.variable = "u"
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_rect3d_05")

DeleteAllPlots()

AddPlot("Pseudocolor", "d")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.zonePortions = (0)
ThresholdAtts.lowerBounds = (0.2)
ThresholdAtts.upperBounds = (0.59)
SetOperatorOptions(ThresholdAtts)
DrawPlots()

Test("vtkm_rect3d_06")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.zonePortions = (1)
ThresholdAtts.lowerBounds = (-0.2)
ThresholdAtts.upperBounds = (0.39)
SetOperatorOptions(ThresholdAtts)
DrawPlots()

Test("vtkm_rect3d_07")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.zonePortions = (0)
ThresholdAtts.lowerBounds = (-0.2)
ThresholdAtts.upperBounds = (0.39)
SetOperatorOptions(ThresholdAtts)
DrawPlots()

Test("vtkm_rect3d_08")

DeleteAllPlots()

AddPlot("Pseudocolor", "d")
AddOperator("Isovolume")
IsovolumeAtts = IsovolumeAttributes()
IsovolumeAtts.lbound = -0.2
IsovolumeAtts.ubound = 0.39
IsovolumeAtts.variable = "u"
SetOperatorOptions(IsovolumeAtts)
DrawPlots()

Test("vtkm_rect3d_09")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Isovolume")
IsovolumeAtts = IsovolumeAttributes()
IsovolumeAtts.lbound = -0.2
IsovolumeAtts.ubound = 0.39
IsovolumeAtts.variable = "default"
SetOperatorOptions(IsovolumeAtts)
DrawPlots()

Test("vtkm_rect3d_10")

#
# Test a 2d curvilinear mesh.
#
DeleteAllPlots()

OpenDatabase(silo_data_path("curv2d.silo"))

AddPlot("Contour", "u")
DrawPlots()

Test("vtkm_curv2d_01")

DeleteAllPlots()

AddPlot("Contour", "d")
DrawPlots()

Test("vtkm_curv2d_02")

#
# Test a 3d curvilinear mesh.
#
DeleteAllPlots()

OpenDatabase(silo_data_path("curv3d.silo"))

AddPlot("Contour", "u")
DrawPlots()

v = View3DAttributes()
v.viewNormal = (0.4854, 0.3984, 0.7782)
v.focus = (0., 2.5, 15.)
v.viewUp = (-0.1209, 0.9122, -0.3916)
v.parallelScale = 16.0078
v.nearPlane = -32.0156
v.farPlane = 32.0156
SetView3D(v)

Test("vtkm_curv3d_01")

DeleteAllPlots()

AddPlot("Contour", "d")
DrawPlots()

Test("vtkm_curv3d_01a")

DeleteAllPlots()

AddPlot("Pseudocolor", "d")
AddOperator("Slice")
atts = SliceAttributes()
atts.project2d = 0
atts.normal = (0., 1., 0.)
atts.originType = atts.Point
atts.originPoint = (0., 2.5, 15.)
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_curv3d_02")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Slice")
atts = SliceAttributes()
atts.project2d = 0
atts.normal = (0., 1., 0.)
atts.originType = atts.Point
atts.originPoint = (0., 2.5, 15.)
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_curv3d_03")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Isosurface")
atts = IsosurfaceAttributes()
atts.contourNLevels = 10
atts.contourMethod = atts.Level
atts.minFlag = 0
atts.maxFlag = 0
atts.variable = "u"
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_curv3d_04")

DeleteAllPlots()

AddPlot("Pseudocolor", "d")
AddOperator("Isosurface")
atts = IsosurfaceAttributes()
atts.contourNLevels = 10
atts.contourMethod = atts.Level
atts.minFlag = 0
atts.maxFlag = 0
atts.variable = "u"
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_curv3d_05")

DeleteAllPlots()

AddPlot("Pseudocolor", "d")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.zonePortions = (0)
ThresholdAtts.lowerBounds = (1)
ThresholdAtts.upperBounds = (3)
SetOperatorOptions(ThresholdAtts)
DrawPlots()

Test("vtkm_curv3d_06")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.zonePortions = (1)
ThresholdAtts.lowerBounds = (-0.5)
ThresholdAtts.upperBounds = (0.5)
SetOperatorOptions(ThresholdAtts)
DrawPlots()

Test("vtkm_curv3d_07")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.zonePortions = (0)
ThresholdAtts.lowerBounds = (-0.5)
ThresholdAtts.upperBounds = (0.5)
SetOperatorOptions(ThresholdAtts)
DrawPlots()

Test("vtkm_curv3d_08")

DeleteAllPlots()

AddPlot("Pseudocolor", "d")
AddOperator("Isovolume")
IsovolumeAtts = IsovolumeAttributes()
IsovolumeAtts.lbound = -0.5
IsovolumeAtts.ubound = 0.5
IsovolumeAtts.variable = "u"
SetOperatorOptions(IsovolumeAtts)
DrawPlots()

Test("vtkm_curv3d_09")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Isovolume")
IsovolumeAtts = IsovolumeAttributes()
IsovolumeAtts.lbound = -0.5
IsovolumeAtts.ubound = 0.5
IsovolumeAtts.variable = "default"
SetOperatorOptions(IsovolumeAtts)
DrawPlots()

Test("vtkm_curv3d_10")

#
# Test a 2d unstructured mesh.
#
DeleteAllPlots()

OpenDatabase(silo_data_path("ucd2d.silo"))

AddPlot("Contour", "u")
DrawPlots()

Test("vtkm_ucd2d_01")

DeleteAllPlots()

AddPlot("Contour", "d")
DrawPlots()

Test("vtkm_ucd2d_02")

#
# Test a 3d unstructured mesh.
#
DeleteAllPlots()

OpenDatabase(silo_data_path("ucd3d.silo"))

AddPlot("Contour", "u")
DrawPlots()

v = View3DAttributes()
v.viewNormal = (0.4854, 0.3984, 0.7782)
v.focus = (0., 2.5, 10.)
v.viewUp = (-0.1209, 0.9122, -0.3916)
v.parallelScale = 11.4564
v.nearPlane = -22.9129
v.farPlane = 22.9129
SetView3D(v)

Test("vtkm_ucd3d_01")

DeleteAllPlots()

AddPlot("Contour", "d")
DrawPlots()

Test("vtkm_ucd3d_01a")

DeleteAllPlots()

AddPlot("Pseudocolor", "d")
AddOperator("Slice")
atts = SliceAttributes()
atts.project2d = 0
atts.normal = (0., 1., 0.)
atts.originType = atts.Point
atts.originPoint = (0., 2.5, 10.)
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_ucd3d_02")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Slice")
atts = SliceAttributes()
atts.project2d = 0
atts.normal = (0., 1., 0.)
atts.originType = atts.Point
atts.originPoint = (0., 2.5, 10.)
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_ucd3d_03")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Isosurface")
atts = IsosurfaceAttributes()
atts.contourNLevels = 10
atts.contourMethod = atts.Level
atts.minFlag = 0
atts.maxFlag = 0
atts.variable = "u"
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_ucd3d_04")

DeleteAllPlots()

AddPlot("Pseudocolor", "d")
AddOperator("Isosurface")
atts = IsosurfaceAttributes()
atts.contourNLevels = 10
atts.contourMethod = atts.Level
atts.minFlag = 0
atts.maxFlag = 0
atts.variable = "u"
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_ucd3d_05")

DeleteAllPlots()

AddPlot("Pseudocolor", "d")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.zonePortions = (0)
ThresholdAtts.lowerBounds = (2.5)
ThresholdAtts.upperBounds = (4.0)
SetOperatorOptions(ThresholdAtts)
DrawPlots()

Test("vtkm_ucd3d_06")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.zonePortions = (1)
ThresholdAtts.lowerBounds = (0.8)
ThresholdAtts.upperBounds = (0.95)
SetOperatorOptions(ThresholdAtts)
DrawPlots()

Test("vtkm_ucd3d_07")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.zonePortions = (0)
ThresholdAtts.lowerBounds = (0.8)
ThresholdAtts.upperBounds = (0.95)
SetOperatorOptions(ThresholdAtts)
DrawPlots()

Test("vtkm_ucd3d_08")

DeleteAllPlots()

AddPlot("Pseudocolor", "d")
AddOperator("Isovolume")
atts = IsovolumeAttributes()
atts.lbound = 2.5
atts.ubound = 4.0
atts.variable = "default"
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_ucd3d_09")

#
# Test 3d unstructured with zoo elements.
#
DeleteAllPlots()

OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Contour", "u")
DrawPlots()

v = View3DAttributes()
v.viewNormal = (-0.3057, 0.7186, 0.6247)
v.focus = (0., 0., 0.)
v.viewUp = (0.5003, 0.6794, -0.5367)
v.parallelScale = 17.3205
v.nearPlane = -34.641
v.farPlane = 34.641
SetView3D(v)

Test("vtkm_globe_01")

DeleteAllPlots()

AddPlot("Contour", "dx")
DrawPlots()

Test("vtkm_globe_02")

DeleteAllPlots()

AddPlot("Pseudocolor", "v")
AddOperator("Slice")
atts = SliceAttributes()
atts.project2d = 0
atts.normal = (0., 1., 0.)
atts.originType = atts.Point
atts.originPoint = (0., 0., 0.)
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_globe_03")

DeleteAllPlots()

AddPlot("Pseudocolor", "dx")
AddOperator("Slice")
atts = SliceAttributes()
atts.project2d = 0
atts.normal = (0., 1., 0.)
atts.originType = atts.Point
atts.originPoint = (0., 0., 0.)
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_globe_04")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Isosurface")
atts = IsosurfaceAttributes()
atts.contourNLevels = 10
atts.contourMethod = atts.Level
atts.minFlag = 0
atts.maxFlag = 0
atts.variable = "u"
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_globe_05")

DeleteAllPlots()

AddPlot("Pseudocolor", "dx")
AddOperator("Isosurface")
atts = IsosurfaceAttributes()
atts.contourNLevels = 10
atts.contourMethod = atts.Level
atts.minFlag = 0
atts.maxFlag = 0
atts.variable = "u"
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_globe_06")

DeleteAllPlots()

AddPlot("Pseudocolor", "dx")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.zonePortions = (0)
ThresholdAtts.lowerBounds = (-2.0)
ThresholdAtts.upperBounds = (2.0)
SetOperatorOptions(ThresholdAtts)
DrawPlots()

Test("vtkm_globe_07")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.zonePortions = (1)
ThresholdAtts.lowerBounds = (-5.0)
ThresholdAtts.upperBounds = (5.0)
SetOperatorOptions(ThresholdAtts)
DrawPlots()

Test("vtkm_globe_08")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.zonePortions = (0)
ThresholdAtts.lowerBounds = (-5.0)
ThresholdAtts.upperBounds = (5.0)
SetOperatorOptions(ThresholdAtts)
DrawPlots()

Test("vtkm_globe_09")

DeleteAllPlots()

AddPlot("Pseudocolor", "dx")
AddOperator("Isovolume")
atts = IsovolumeAttributes()
atts.lbound = -5.0
atts.ubound = 5.0
atts.variable = "u"
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_globe_10")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Isovolume")
atts = IsovolumeAttributes()
atts.lbound = -5.0
atts.ubound = 5.0
atts.variable = "default"
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_globe_11")

#
# Test a multi-domain unstructured mesh.
#
DeleteAllPlots()

OpenDatabase(silo_data_path("multi_ucd3d.silo"))

AddPlot("Contour", "u")
DrawPlots()

v = View3DAttributes()
v.viewNormal = (0.4854, 0.3984, 0.7782)
v.focus = (0., 2.5, 10.)
v.viewUp = (-0.1209, 0.9122, -0.3916)
v.parallelScale = 11.4564
v.nearPlane = -22.9129
v.farPlane = 22.9129
SetView3D(v)

Test("vtkm_multi_ucd3d_01")

DeleteAllPlots()

AddPlot("Pseudocolor", "d")
AddOperator("Slice")
atts = SliceAttributes()
atts.project2d = 0
atts.normal = (0., 1., 0.)
atts.originType = atts.Point
atts.originPoint = (0., 2.5, 10.)
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_multi_ucd3d_02")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Slice")
atts = SliceAttributes()
atts.project2d = 0
atts.normal = (0., 1., 0.)
atts.originType = atts.Point
atts.originPoint = (0., 2.5, 10.)
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_multi_ucd3d_03")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Isosurface")
atts = IsosurfaceAttributes()
atts.contourNLevels = 10
atts.contourMethod = atts.Level
atts.minFlag = 0
atts.maxFlag = 0
atts.variable = "u"
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_multi_ucd3d_04")

DeleteAllPlots()

AddPlot("Pseudocolor", "d")
AddOperator("Isosurface")
atts = IsosurfaceAttributes()
atts.contourNLevels = 10
atts.contourMethod = atts.Level
atts.minFlag = 0
atts.maxFlag = 0
atts.variable = "u"
SetOperatorOptions(atts)
DrawPlots()

Test("vtkm_multi_ucd3d_05")

DeleteAllPlots()

AddPlot("Pseudocolor", "d")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.zonePortions = (0)
ThresholdAtts.lowerBounds = (2.5)
ThresholdAtts.upperBounds = (4.0)
SetOperatorOptions(ThresholdAtts)
DrawPlots()

Test("vtkm_multi_ucd3d_06")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.zonePortions = (1)
ThresholdAtts.lowerBounds = (0.995)
ThresholdAtts.upperBounds = (0.9985)
SetOperatorOptions(ThresholdAtts)
DrawPlots()

Test("vtkm_multi_ucd3d_07")

DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.zonePortions = (0)
ThresholdAtts.lowerBounds = (0.995)
ThresholdAtts.upperBounds = (0.9985)
SetOperatorOptions(ThresholdAtts)
DrawPlots()

Test("vtkm_multi_ucd3d_08")

Exit()
