# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  tiling.py
#
#  Tests:      Image saving with tiled rendering.
#              plots - pseudocolor, label
#
#  Programmer: Eric Brugger
#  Date:       Wed Sep  9 14:10:52 PDT 2026
#
#  Modifications:
#    Eric Brugger, Fri Oct  2 16:14:53 PDT 2026
#    Add tests with the label plot in 2D with different tiling
#    configurations.
#
# ----------------------------------------------------------------------------

TurnOffAllAnnotations()

OpenDatabase(silo_data_path("wave0000.silo"))

ra = RenderingAttributes()
ra.scalableActivationMode = ra.Always
ra.tiledRenderingWidth = 200
ra.tiledRenderingHeight = 200
SetRenderingAttributes(ra)

# Save images in various tile layouts in 3D with the pseudocolor
# and label plots.
#
# They include: 2x2, 3x2, 3x3, 4x3, 4x4.
AddPlot("Label", "chars")
l = LabelAttributes()
l.depthTestMode = l.LABEL_DT_NEVER
SetPlotOptions(l)
AddPlot("Pseudocolor", "pressure")
DrawPlots()
v3d=GetView3D()
v3d.viewNormal=(0, 1, 0)
v3d.viewUp=(0, 0, -1)
SetView3D(v3d)

save = SaveWindowAttributes()
save.format = save.PNG
save.width = 400
save.height = 400
save.screenCapture = 0
save.resConstraint = save.NoConstraint
Test("tiled_2x2", save)

save.width = 500
save.height = 400
Test("tiled_3x2", save)

save.width = 500
save.height = 500
Test("tiled_3x3", save)

save.width = 700
save.height = 500
Test("tiled_4x3", save)

save.width = 800
save.height = 800
Test("tiled_4x4", save)

DeleteAllPlots()

CloseDatabase(silo_data_path("wave0000.silo"))

# Save images in various tile layouts in 2D with the pseudocolor
# and label plots.
#
# They include: 2x2, 3x2, 3x3, 4x3, 4x4.
OpenDatabase(silo_data_path("rect2d.silo"))

AddPlot("Label", "quadmesh2d")
l = LabelAttributes()
l.depthTestMode = l.LABEL_DT_NEVER
l.numberOfLabels = 90
SetPlotOptions(l)
AddPlot("Pseudocolor", "d")
DrawPlots()
v2d=GetView2D()
v2d.viewportCoords=(0, 1, 0, 1)
v2d.windowCoords=(0.25, 0.75, 0.25, 0.75)
SetView2D(v2d)

save.width = 400
save.height = 400
Test("tiled_2D_2x2", save)

save.width = 500
save.height = 400
v2d.windowCoords=(0.1875, 0.8125, 0.25, 0.75)
SetView2D(v2d)
Test("tiled_2D_3x2", save)

save.width = 500
save.height = 500
v2d.windowCoords=(0.25, 0.75, 0.25, 0.75)
SetView2D(v2d)
Test("tiled_2D_3x3", save)

save.width = 700
save.height = 500
v2d.windowCoords=(0.15, 0.85, 0.25, 0.75)
SetView2D(v2d)
Test("tiled_2D_4x3", save)

save.width = 800
save.height = 800
v2d.windowCoords=(0.25, 0.75, 0.25, 0.75)
SetView2D(v2d)
Test("tiled_2D_4x4", save)

DeleteAllPlots()

CloseDatabase(silo_data_path("rect2d.silo"))

Exit()
