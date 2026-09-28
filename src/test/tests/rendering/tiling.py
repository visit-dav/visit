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
#
# ----------------------------------------------------------------------------

TurnOffAllAnnotations()

OpenDatabase(silo_data_path("wave0000.silo"))

ra = RenderingAttributes()
ra.scalableActivationMode = ra.Always
ra.tiledRenderingWidth = 200
ra.tiledRenderingHeight = 200
SetRenderingAttributes(ra)

# Save images in various tile layouts with the pseudocolor and
# label plots.
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

Exit()
