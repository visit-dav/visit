# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  mesh_quality.py
#
#  Tests:      mesh      - 3D unstructured
#              plots     - Pseudocolor
#
#  Programmer: Hank Childs
#  Date:       November 25, 2003
#
#  Modifications:
#
#    Hank Childs, Fri Jun 25 07:58:32 PDT 2004
#    The name of the mesh quality variables for ucd3d changed, since there are
#    now multiple meshes in that file.  Use the new names.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Cyrus Harrison, Thu Mar 25 09:57:34 PDT 2010
#    Added call(s) to DrawPlots() b/c of changes to the default plot state
#    behavior when an operator is added.
#
# ----------------------------------------------------------------------------



OpenDatabase(silo_data_path("ucd3d.silo"))

AddPlot("Pseudocolor", "mesh_quality/ucdmesh3d/volume")
DrawPlots()

v = View3DAttributes()
v.viewNormal = (-0.178, -0.29, 0.94)
v.focus = (0, 2.5, 10)
v.viewUp = (-0.11, 0.95, 0.26)
v.parallelScale = 11.45
v.nearPlane = -22.9
v.farPlane = 22.9
v.imageZoom = 1.46
SetView3D(v)

Test("mesh_quality_01")

DeleteAllPlots()
AddPlot("Pseudocolor", "mesh_quality/ucdmesh3d/skew")
DrawPlots()

Test("mesh_quality_02")

DeleteAllPlots()

OpenDatabase(silo_data_path("tire.silo"))

AddPlot("Pseudocolor", "mesh_quality/aspect")
DrawPlots()
v = View3DAttributes()
v.viewNormal = (-0.128, 0.26, 0.958)
v.focus = (0, 0, 0)
v.viewUp = (0.31, 0.92, -0.21)
v.parallelScale = 83
v.nearPlane = -165
v.farPlane = 165
SetView3D(v)

pc = PseudocolorAttributes()
pc.minFlag = 1
pc.maxFlag = 1
pc.min=11
pc.max=22
SetPlotOptions(pc)

Test("mesh_quality_03")

pc.minFlag=0
pc.maxFlag=0
SetPlotOptions(pc)
Test("mesh_quality_04")

AddOperator("Threshold")
thres = ThresholdAttributes()
thres.lowerBounds = (0.5)
thres.upperBounds = (0.55)
thres.listedVarNames = ("mesh_quality/skew")
SetOperatorOptions(thres)
DrawPlots()
Test("mesh_quality_05")

Exit()
