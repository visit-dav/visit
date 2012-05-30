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
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Cyrus Harrison, Thu Mar 25 09:57:34 PDT 2010
#    Added call(s) to DrawPlots() b/c of changes to the default plot state
#    behavior when an operator is added.
#
# ----------------------------------------------------------------------------


# Test 2D Case

OpenDatabase(silo_data_path("rect2d.silo"))


AddPlot("Pseudocolor", "d")
DrawPlots()
Test("ops_dualmesh_pc_plot_2d_d_00")

# test auto, Zones to Nodes
AddOperator("DualMesh")
DrawPlots()
Test("ops_dualmesh_pc_plot_2d_d_01")

# test auto, Nodes to Zones
AddOperator("DualMesh")
DrawPlots()
Test("ops_dualmesh_pc_plot_2d_d_02")

DeleteAllPlots()

# Test Mesh Plot auto
AddPlot("Mesh", "quadmesh2d")
DrawPlots()
Test("ops_dualmesh_mesh_plot_2d_00")

AddOperator("DualMesh")
DrawPlots()
Test("ops_dualmesh_mesh_plot_2d_01")

AddOperator("DualMesh")
DrawPlots()
Test("ops_dualmesh_mesh_plot_2d_02")
DeleteAllPlots()

# Test 3D Case
OpenDatabase(silo_data_path("rect3d.silo"))

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
DrawPlots()
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
DrawPlots()
Test("ops_dual_mesh_pc_plot_3d_t_02")


Exit()
