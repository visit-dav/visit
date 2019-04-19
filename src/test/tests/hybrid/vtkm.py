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
#              operators - Isosurface, Slice
#
#  Programmer: Eric Brugger
#  Date:       Wed Sep 19 12:18:14 PDT 2018
#
#  Modifications:
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

#
# Test a 2d curvilinear mesh.
#
DeleteAllPlots()

OpenDatabase(silo_data_path("curv3d.silo"))

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

Exit()
