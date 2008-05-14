# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  dual_mesh.py
#
#  Tests:      mesh      - 2D rectilinear, single domain.
#                          3D rectilinear, single domain.
#              plots     - pc, mesh
#              operators - DualMesh
#
#
#  Programmer: Cyrus Harrison
#  Date:       May 13, 2008
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Turn off all annotations
a = AnnotationAttributes()
a.axesFlag2D = 0
a.axesFlag = 0
a.triadFlag = 0
a.bboxFlag = 0
a.userInfoFlag = 0
a.databaseInfoFlag = 0
a.legendInfoFlag = 0
a.backgroundMode = 0
a.foregroundColor = (0, 0, 0, 255)
a.backgroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)

# Test 2D Case

OpenDatabase("../data/rect2d.silo")

AddPlot("Pseudocolor", "d")
DrawPlots()
Test("ops_dualmesh_pc_plot_2d_d_00")

# test auto, Zones to Nodes
AddOperator("DualMesh")
Test("ops_dualmesh_pc_plot_2d_d_01")

# test auto, Nodes to Zones
AddOperator("DualMesh")
Test("ops_dualmesh_pc_plot_2d_d_02")

DeleteAllPlots()

# Test Mesh Plot auto
AddPlot("Mesh", "quadmesh2d")
DrawPlots()
Test("ops_dualmesh_mesh_plot_2d_00")

AddOperator("DualMesh")
Test("ops_dualmesh_mesh_plot_2d_01")

AddOperator("DualMesh")
Test("ops_dualmesh_mesh_plot_2d_02")
DeleteAllPlots()

# Test 3D Case
OpenDatabase("../data/rect3d.silo")
AddPlot("Pseudocolor", "d")
view = GetView3D()
view.viewNormal = (-0.5, 0.5, 0.7172)
SetView3D(view)
DrawPlots()
Test("ops_dual_mesh_pc_plot_3d_d_00")

# Test explicit Zones to Nodes
AddOperator("DualMesh")
DualMeshAtts = DualMeshAttributes()
DualMeshAtts.mode = DualMeshAtts.ZonesToNodes
SetOperatorOptions(DualMeshAtts)
DrawPlots()
Test("ops_dual_mesh_pc_plot_3d_d_01")

# Test explicit Nodes to Zones (round trip)
AddOperator("DualMesh")
DualMeshAtts = DualMeshAttributes()
DualMeshAtts.mode = DualMeshAtts.NodesToZones
SetOperatorOptions(DualMeshAtts)
Test("ops_dual_mesh_pc_plot_3d_d_02")

DeleteAllPlots()

# Test Nodes to Zones <-> Zones to Nodes Round Trip
AddPlot("Pseudocolor", "t")
DrawPlots()
Test("ops_dual_mesh_pc_plot_3d_t_00")

AddOperator("DualMesh")
DualMeshAtts = DualMeshAttributes()
DualMeshAtts.mode = DualMeshAtts.NodesToZones
SetOperatorOptions(DualMeshAtts)
DrawPlots()
Test("ops_dual_mesh_pc_plot_3d_t_01")

AddOperator("DualMesh")
DualMeshAtts = DualMeshAttributes()
DualMeshAtts.mode = DualMeshAtts.ZonesToNodes
SetOperatorOptions(DualMeshAtts)
Test("ops_dual_mesh_pc_plot_3d_t_02")


Exit()
