# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  replace.py
#
#  Tests:      the ReplaceDatabase function.
#
#  Defect ID:  VisIt00003002
#
#  Programmer: Brad Whitlock
#  Date:       Wed Jan 22 11:06:49 PDT 2003
#
#  Modifications:
#    Kathleen Bonnell, Thu Aug 28 14:34:57 PDT 2003
#    Remove compound var name from subset plots.
# 
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------

# Turn off annotation
a = AnnotationAttributes()
TurnOffAllAnnotations(a)
a.axes2D.visible = 1
a.axes2D.xAxis.label.visible = 0
a.axes2D.yAxis.label.visible = 0
a.axes2D.xAxis.title.visible = 0
a.axes2D.yAxis.title.visible = 0
SetAnnotationAttributes(a)

# Set up the plots.
OpenDatabase(data_path("silo_%s_test_data/wave0110.silo")%SILO_MODE)

AddPlot("Subset", "Material")
DrawPlots()
v = View3DAttributes()
v.viewNormal = (-0.427729, 0.776091, 0.463391)
v.focus = (4.37669, 0.376992, 2.57924)
v.viewUp = (0.67875, 0.614328, -0.402368)
v.viewAngle = 30.
v.parallelScale = 5.03337
v.nearPlane = -11.2758
v.farPlane = 11.2758
v.perspective = 1
SetView3D(v)

# Show what it looks like with all materials
Test("AllMaterials")
# Turn off the green material
TurnMaterialsOff("2 water")
Test("Material2Off")

# Try replacing with a later time step and see if the SIL selection
# still is applied. It should be still applied since the files have
# equivalent SILs.
ReplaceDatabase(data_path("silo_%s_test_data/wave0340.silo")%SILO_MODE)

Test("Replace1")

# Make different materials active and replace again.
TurnMaterialsOn()
TurnMaterialsOff("1 barrier")
ReplaceDatabase(data_path("silo_%s_test_data/wave0470.silo")%SILO_MODE)

Test("Replace2")

# Open one of our simple databases, do a plot and then replace it with
# an unrelated database.
DeleteAllPlots()
OpenDatabase(data_path("silo_%s_test_data/globe.silo")%SILO_MODE)

AddPlot("Pseudocolor", "u")
DrawPlots()
ReplaceDatabase(data_path("silo_%s_test_data/curv3d.silo")%SILO_MODE)

v2 = View3DAttributes()
v2.viewNormal = (-0.499092, 0.420388, 0.757747)
v2.focus = (0,2.5,15)
v2.viewUp = (0.195607, 0.906528, -0.374093)
v2.viewAngle = 30.
v2.parallelScale = 16.0078
v2.nearPlane = -32.0156
v2.farPlane = 32.0156
v2.perspective = 1
SetView3D(v2)
Test("Replace3")

# Replace it with a 2d database.
ReplaceDatabase(data_path("silo_%s_test_data/curv2d.silo")%SILO_MODE)

Test("Replace4")
Exit()
