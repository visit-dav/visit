# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  vtkm.py
#
#  Tests:      mesh      - 3D rectilinear, single domain
#                          3D curvilinear, single domain
#                          3D unstructured, single domain
#              plots     - Contour, Pseudocolor
#              operators - Slice
#
#  Programmer: Eric Brugger
#  Date:       Wed Sep 19 12:18:14 PDT 2018
#
#  Modifications:
#
# ----------------------------------------------------------------------------

SetBackendType("vtkm")

#
# Test a rectilinear mesh.
#
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

Test("vtkm_01")

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

Test("vtkm_02")

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

Test("vtkm_03")

#
# Test an curvilinear mesh.
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

Test("vtkm_04")

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

Test("vtkm_05")

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

Test("vtkm_06")

#
# Test an unstructured mesh.
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

Test("vtkm_07")

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

Test("vtkm_08")

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

Test("vtkm_09")

Exit()
