# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  kulllite.py
#
#  Tests:      mesh      - 2D, 3D unstructured
#              plots     - Pseudocolor, mesh, boundary, subset
#
#  Defect ID:  '6251, '6326, '7043
#
#  Programmer: Hank Childs
#  Date:       June 14, 2005
#
#  Modifications:
#
#    Hank Childs, Thu May 11 11:35:39 PDT 2006
#    Test reading of density values.
#
# ----------------------------------------------------------------------------

a = AnnotationAttributes()
TurnOffAllAnnotations(a)
a.axesFlag2D = 1
SetAnnotationAttributes(a)

OpenDatabase("../data/KullLite_test_data/tagtest_xy_3.pdb")
AddPlot("Mesh", "mesh")
DrawPlots()
Test("kulllite_01")

DeleteAllPlots()
OpenDatabase("../data/KullLite_test_data/tagtest_rz_1_0.pdb")
AddPlot("FilledBoundary", "Material(mesh)")
AddPlot("Mesh", "mesh_tags/edges_mesh")
DrawPlots()
Test("kulllite_02")

# Now turn off 2D axes too.
a.axesFlag2D = 0
SetAnnotationAttributes(a)

DeleteActivePlots()
AddPlot("Mesh", "mesh")
AddPlot("Mesh", "mesh_tags/nodes_mesh")
m = MeshAttributes()
m.pointType = m.Box
SetPlotOptions(m)
DrawPlots()
Test("kulllite_03")

DeleteAllPlots()
AddPlot("FilledBoundary", "mesh_tags/nodes(mesh_tags/nodes_mesh)")
fb = FilledBoundaryAttributes()
fb.pointType = fb.Box
SetPlotOptions(fb)
DrawPlots()
Test("kulllite_04")

DeleteAllPlots()
OpenDatabase("../data/KullLite_test_data/T.pdb")
AddPlot("FilledBoundary", "Material")
DrawPlots()
Test("kulllite_05")

thres = ThresholdAttributes()
thres.listedVarNames = ("mesh_quality/shear")
thres.upperBounds = (0.5)
SetDefaultOperatorOptions(thres)
AddOperator("Threshold")
Test("kulllite_06")

DeleteAllPlots()

# Test reading of densities, for both per-zone quantities and per-zone,
# per-material quantities.
OpenDatabase("../data/2dOverlayTest21.pdb")
AddPlot("Pseudocolor", "density")
DrawPlots()
Test("kulllite_07")
m = MaterialAttributes()
m.forceMIR = 1
SetMaterialAttributes(m)
ReOpenDatabase("../data/2dOverlayTest21.pdb")
Test("kulllite_08")

Exit()


