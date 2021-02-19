# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  tessellate.py
#
#  Tests:      mesh      - quadratic_triangle
#                          biquadratic_quad
#                          quadratic_linear_quad
#                          quadratic_quad
#                          quadratic_hex
#                          triquadratic_hex
#              plots     - pc, mesh
#              operators - tessellate, clip
#
#  Programmer: Eric Brugger
#  Date:       July 24, 2020
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Quadratic_triangle
OpenDatabase(data_path("vtk_test_data/quadratic_triangle.vtk"))

AddPlot("Pseudocolor", "x_c")
AddPlot("Mesh", "mesh")
DrawPlots()

v = GetView3D()
v.viewNormal = (0.200511, 0.543812, 0.814901)
v.focus = (0, 0.5, 1)
v.viewUp = (-0.232184, 0.834474, -0.499744)
v.viewAngle = 30
v.parallelScale = 1.5
v.nearPlane = -3
v.farPlane = 3
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
SetView3D(v)

Test("quadratic_triangle_01")

AddOperator("Tessellate", 1)
DrawPlots()

Test("quadratic_triangle_02")

tess = TessellateAttributes()
tess.chordError = 0.01
SetOperatorOptions(tess, 0, 1)

Test("quadratic_triangle_03")

DeleteAllPlots()
CloseDatabase(data_path("vtk_test_data/quadratic_triangle.vtk"))

# Biquadratic_quad
OpenDatabase(data_path("vtk_test_data/biquadratic_quad.vtk"))

AddPlot("Pseudocolor", "x_c")
AddPlot("Mesh", "mesh")
DrawPlots()

v = GetView3D()
v.viewNormal = (0.200511, 0.543812, 0.814901)
v.focus = (0, 0.5, 1)
v.viewUp = (-0.232184, 0.834474, -0.499744)
v.viewAngle = 30
v.parallelScale = 1.5
v.nearPlane = -3
v.farPlane = 3
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
SetView3D(v)

Test("biquadratic_quad_01")

AddOperator("Tessellate", 1)
DrawPlots()

Test("biquadratic_quad_02")

tess = TessellateAttributes()
tess.chordError = 0.01
SetOperatorOptions(tess, 0, 1)

Test("biquadratic_quad_03")

DeleteAllPlots()
CloseDatabase(data_path("vtk_test_data/biquadratic_quad.vtk"))

# Biquadratic_quad_field
OpenDatabase(data_path("vtk_test_data/biquadratic_quad_field.vtk"))

AddPlot("Pseudocolor", "d")
AddPlot("Mesh", "mesh")
DrawPlots()

Test("biquadratic_quad_field_01")

AddOperator("Tessellate", 1)
DrawPlots()

Test("biquadratic_quad_field_02")

tess = TessellateAttributes()
tess.chordError = 0.01
tess.fieldCriterion = 0.01
SetOperatorOptions(tess, 0, 1)

Test("biquadratic_quad_field_03")

CloseDatabase(data_path("vtk_test_data/biquadratic_quad_field.vtk"))
DeleteAllPlots()

# Quadratic_linear_quad
OpenDatabase(data_path("vtk_test_data/quadratic_linear_quad.vtk"))

AddPlot("Pseudocolor", "x_c")
AddPlot("Mesh", "mesh")
DrawPlots()

v = GetView3D()
v.viewNormal = (0.200511, 0.543812, 0.814901)
v.focus = (0, 0.5, 1)
v.viewUp = (-0.232184, 0.834474, -0.499744)
v.viewAngle = 30
v.parallelScale = 1.5
v.nearPlane = -3
v.farPlane = 3
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
SetView3D(v)

Test("quadratic_linear_quad_01")

DeleteAllPlots()
CloseDatabase(data_path("vtk_test_data/quadratic_linear_quad.vtk"))

# Quadratic_quad
OpenDatabase(data_path("vtk_test_data/quadratic_quad.vtk"))

AddPlot("Pseudocolor", "x_c")
AddPlot("Mesh", "mesh")
DrawPlots()

v = GetView3D()
v.viewNormal = (0.200511, 0.543812, 0.814901)
v.focus = (0, 0.5, 1)
v.viewUp = (-0.232184, 0.834474, -0.499744)
v.viewAngle = 30
v.parallelScale = 1.5
v.nearPlane = -3
v.farPlane = 3
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
SetView3D(v)

Test("quadratic_quad_01")

AddOperator("Tessellate", 1)
DrawPlots()

Test("quadratic_quad_02")

tess = TessellateAttributes()
tess.chordError = 0.01
SetOperatorOptions(tess, 0, 1)

Test("quadratic_quad_03")

DeleteAllPlots()
CloseDatabase(data_path("vtk_test_data/quadratic_quad.vtk"))

# Mixed biquadratic_quad and quadratic_triangle
OpenDatabase(data_path("vtk_test_data/quadratic_mixed.vtk"))

AddPlot("Pseudocolor", "x_c")
AddPlot("Mesh", "mesh")
DrawPlots()

v = GetView3D()
v.viewNormal = (0.200511, 0.543812, 0.814901)
v.focus = (0, 0.5, 1)
v.viewUp = (-0.232184, 0.834474, -0.499744)
v.viewAngle = 30
v.parallelScale = 1.5
v.nearPlane = -3
v.farPlane = 3
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
SetView3D(v)

Test("quadratic_mixed_01")

AddOperator("Tessellate", 1)
DrawPlots()

Test("quadratic_mixed_02")

tess = TessellateAttributes()
tess.chordError = 0.01
SetOperatorOptions(tess, 0, 1)

Test("quadratic_mixed_03")

DeleteAllPlots()
CloseDatabase(data_path("vtk_test_data/quadratic_mixed.vtk"))

# Quadratic_hex
OpenDatabase(data_path("vtk_test_data/quadratic_hex.vtk"))

AddPlot("Pseudocolor", "x_c")
AddPlot("Mesh", "mesh")
DrawPlots()

v = GetView3D()
v.viewNormal = (0.491097, 0.334402, 0.804363)
v.focus = (0.7, 0.7, 0.5)
v.viewUp = (-0.0787305, 0.936642, -0.341326)
v.viewAngle = 30
v.parallelScale = 1.10905
v.nearPlane = -2.21811
v.farPlane = 2.21811
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
SetView3D(v)

Test("quadratic_hex_01")

AddOperator("Tessellate", 1)
DrawPlots()

Test("quadratic_hex_02")

tess = TessellateAttributes()
tess.chordError = 0.01
SetOperatorOptions(tess, 0, 1)

Test("quadratic_hex_03")

AddOperator("Clip", 1)
clip = ClipAttributes()
clip.plane1Origin = (0.5, 0.5, 0.5)
SetOperatorOptions(clip, 0, 1)

DrawPlots()

Test("quadratic_hex_04")

DeleteAllPlots()
CloseDatabase(data_path("vtk_test_data/quadratic_hex.vtk"))

# Triquadratic_hex
OpenDatabase(data_path("vtk_test_data/triquadratic_hex.vtk"))

AddPlot("Pseudocolor", "x_c")
AddPlot("Mesh", "mesh")
DrawPlots()

v = GetView3D()
v.viewNormal = (0.491097, 0.334402, 0.804363)
v.focus = (0.7, 0.7, 0.5)
v.viewUp = (-0.0787305, 0.936642, -0.341326)
v.viewAngle = 30
v.parallelScale = 1.10905
v.nearPlane = -2.21811
v.farPlane = 2.21811
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
SetView3D(v)

Test("triquadratic_hex_01")

DeleteAllPlots()
CloseDatabase(data_path("vtk_test_data/triquadratic_hex.vtk"))

Exit()
