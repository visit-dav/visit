# ----------------------------------------------------------------------------
#  MODES: serial parallel
#  CLASSES: nightly
#
#  Test Case:  blueprint_export.py
#
#  Tests:      Tests blueprint export features "partition" and "flatten".
#
#  Programmer: Christopher Laganella
#  Date:       Tue Dec 14 12:35:16 EST 2021
#
#  Modifications:
#
#  Chris Laganella, Mon Feb 14 14:39:48 EST 2022
#  I added a test case for user provided JSON/YAML options
# ----------------------------------------------------------------------------
import time
import sys
import os.path
import conduit

# Uncomment these functions to run the script through the regular CLI
# def Exit():
#     print("Exiting test...")
#     return

# def RequiredDatabasePlugin(name):
#     print("This test requires database plugin " + name)
#     return

# def TestSection(name):
#     print("---- " + name + " ----")
#     return

# def Test(name):
#     s = SaveWindowAttributes()
#     s.fileName = name
#     SetSaveWindowAttributes(s)
#     SaveWindow()
#     return

# def TestText(name):
#     print("Testing text file {}".format(name))

# def silo_data_path(name):
#     retval = os.path.join("/mnt/data/il/VisIt/VisItClass/data", name)
#     # print(retval)
#     return retval

def load_text(file_name):
    # print("Loading text from {}".format(os.path.abspath(file_name)), file=sys.stderr)
    with open(file_name, 'r') as f:
        return f.read()

def set_view(case_name, view=None):
    if "2d" in case_name:
        ResetView()
    else:
        SetView3D(view)


def test_name(case, i):
    return case + "_" + str(i) + "_"

# Export DB as bp data set
def export_mesh_bp(case_name, varname):
    export_name = case_name
    e = ExportDBAttributes()
    e.db_type = "Blueprint"
    e.filename = export_name
    e.variables = (varname,)
    ExportDatabase(e)
    time.sleep(1)
    return export_name + ".cycle_000000.root"

# Export DB as csv, return the folder name
def create_csv_output(case_name):
    export_name = case_name
    e = ExportDBAttributes()
    e.db_type = "Blueprint"
    e.filename = export_name
    e.variables = ('mesh_coords', 'zc_mesh_coords')
    opts = GetExportOptions("Blueprint")
    opts["Operation"] = "Flatten_CSV"
    ExportDatabase(e, opts)
    time.sleep(1)
    return export_name + ".csv"

def test_csv_output(case_name):
    vert_file = os.path.join(case_name, "vertex_data.csv")
    elem_file = os.path.join(case_name, "element_data.csv")
    vert_baseline = case_name + "-" + "vertex_data.csv"
    elem_baseline = case_name + "-" + "element_data.csv"
    TestText(vert_baseline, load_text(vert_file))
    TestText(elem_baseline, load_text(elem_file))

def define_mesh_expressions(mesh_name):
    DefineScalarExpression("nid", "nodeid({})".format(mesh_name))
    DefineScalarExpression("zid", "zoneid({})".format(mesh_name))
    DefineScalarExpression("mesh_coords", "coords({})".format(mesh_name))
    DefineScalarExpression("zc_mesh_coords", "recenter(mesh_coords, \"zonal\")")
    return ("nid", "zid", "mesh_coords", "zc_mesh_coords")

# 's' for structured 'r' for rectilinear
def flatten_multi_2d_case(case):
    case_name = "multi_rect2d.silo"
    mesh_name = "mesh1"
    export_name = case_name

    # Use multi_rect3d to create all cases
    OpenDatabase(silo_data_path(case_name))
    AddPlot("Mesh", mesh_name)

    # Define some mesh based variables
    vars = define_mesh_expressions(mesh_name)

    AddOperator("Resample")
    ra = ResampleAttributes()
    ra.samplesX = 5
    ra.samplesY = 4
    ra.is3D = 0
    ra.distributedResample = 1
    SetOperatorOptions(ra)

    AddOperator("DeferExpression")
    dea = DeferExpressionAttributes()
    dea.exprs = vars
    SetOperatorOptions(dea)

    if case == 's':
        AddOperator("Transform")
        ta = TransformAttributes()
        ta.doRotate = 1
        # ta.rotateType = "Deg"
        ta.rotateAmount = 0.00001
        SetOperatorOptions(ta)
        export_name = "structured_" + case_name

    DrawPlots()

    # Create csv file
    export_dir = create_csv_output(export_name)

    DeleteAllPlots()
    CloseDatabase(silo_data_path(case_name))

    # Test text
    test_csv_output(export_dir)

def flatten_multi_2d_unstructured_case():
    case_name = "ucd2d.silo"
    mesh_name = "ucdmesh2d"
    export_name = case_name

    # Use multi_curv2d and remove cells
    OpenDatabase(silo_data_path(case_name))
    AddPlot("Mesh", mesh_name)

    # Define some mesh based variables
    vars = define_mesh_expressions(mesh_name)

    AddOperator("Threshold")
    ta = ThresholdAttributes()
    ta.listedVarNames = ("zid",)
    ta.lowerBounds = (8,)
    ta.upperBounds = (11,)
    SetOperatorOptions(ta)
    export_name = case_name

    AddOperator("DeferExpression")
    dea = DeferExpressionAttributes()
    dea.exprs = ("mesh_coords", "zc_mesh_coords")
    SetOperatorOptions(dea)

    DrawPlots()

    # Create csv file
    export_dir = create_csv_output(export_name)

    DeleteAllPlots()
    CloseDatabase(silo_data_path(case_name))

    # Test text
    test_csv_output(export_dir)

# case = 'u' for unstructured, 'r' for rectilinear, 's' for structured
def flatten_multi_3d_case(case):
    case_name = "multi_rect3d.silo"
    mesh_name = "mesh1"
    export_name = case_name
    # Use multi_rect3d to create all cases
    OpenDatabase(silo_data_path(case_name))
    AddPlot("Mesh", mesh_name)

    # Define some mesh based variables
    vars = define_mesh_expressions(mesh_name)

    AddOperator("Resample")
    ra = ResampleAttributes()
    ra.samplesX = 5
    ra.samplesY = 4
    ra.samplesZ = 3
    ra.distributedResample = 1
    SetOperatorOptions(ra)

    AddOperator("DeferExpression")
    dea = DeferExpressionAttributes()
    dea.exprs = vars
    SetOperatorOptions(dea)

    if case == 'u':
        AddOperator("Threshold")
        ta = ThresholdAttributes()
        ta.listedVarNames = ("zid",)
        ta.lowerBounds = (1,)
        SetOperatorOptions(ta)
        export_name = "unstructured_" + case_name
    elif case == 's':
        AddOperator("Transform")
        ta = TransformAttributes()
        ta.doRotate = 1
        # ta.rotateType = "Deg"
        ta.rotateAmount = 0.00001
        SetOperatorOptions(ta)
        export_name = "structured_" + case_name
    # Do nothing for rectilinear

    DrawPlots()

    # Create csv file
    export_dir = create_csv_output(export_name)

    DeleteAllPlots()
    CloseDatabase(silo_data_path(case_name))

    # Test text
    test_csv_output(export_dir)

def flatten_noise():
    case_name = "noise.silo"
    mesh_name = "Mesh"
    samples = (5, 4, 3)

    OpenDatabase(silo_data_path(case_name))
    AddPlot("Mesh", mesh_name)

    # Expose coordinates as variables
    vars = define_mesh_expressions(mesh_name)

    # Resample
    AddOperator("Resample")
    ra = ResampleAttributes()
    ra.samplesX = samples[0]
    ra.samplesY = samples[1]
    ra.samplesZ = samples[2]
    SetOperatorOptions(ra)

    AddOperator("DeferExpression")
    dea = DeferExpressionAttributes()
    dea.exprs = vars
    SetOperatorOptions(dea)

    DrawPlots()

    # Create the CSV output
    export_dir = create_csv_output(case_name)

    DeleteAllPlots()
    CloseDatabase(silo_data_path(case_name))

    # Test text
    test_csv_output(export_dir)

def partition_test_case(case_name, targets, view=None):
    # Write the original dataset
    OpenDatabase(silo_data_path(case_name))
    AddPlot("Pseudocolor", "u")
    DrawPlots()
    set_view(case_name, view)
    Test(case_name)
    DeleteAllPlots()
    CloseDatabase(silo_data_path(case_name))

    for target in targets:
        export_name = case_name + "_partition" + str(target)
        export_filename = export_name + ".cycle_000000.root"

        # Plot the original dataset
        OpenDatabase(silo_data_path(case_name))
        AddPlot("Pseudocolor", "u")
        DrawPlots()
        set_view(case_name, view)

        # Set the export database attributes.
        e = ExportDBAttributes()
        e.db_type = "Blueprint"
        e.filename = export_name
        e.variables = ("u")
        opts = GetExportOptions("Blueprint")
        opts["Operation"] = "Partition"
        opts["Partition target number of domains"] = target
        print("About to partition " + case_name + " into "
            + str(target) + " blocks.")
        ExportDatabase(e, opts)
        time.sleep(1)
        DeleteAllPlots()
        CloseDatabase(silo_data_path(case_name))

        OpenDatabase(export_filename)
        AddPlot("Pseudocolor", "mesh_topo/u")
        DrawPlots()
        set_view(case_name, view)
        Test(test_name(export_name, 0))
        DeleteAllPlots()

        AddPlot("Subset", "domains")
        DrawPlots()
        set_view(case_name, view)
        Test(test_name(export_name, 1))
        DeleteAllPlots()
        CloseDatabase(export_filename)

def partition_test_extra_options():
    dbname = "multi_rect2d.silo"
    TestSection("Extra options")
    OpenDatabase(silo_data_path(dbname))
    AddPlot("Subset", "domains(mesh1)")
    DrawPlots()
    Test("DefaultMesh")

    # Test JSON
    e0 = ExportDBAttributes()
    e0.db_type = "Blueprint"
    e0.filename = "multi_rect2d_json_target_1"
    e0.variables = ("u")
    opts0 = GetExportOptions("Blueprint")
    opts0["Operation"] = "Partition"
    opts0["Flatten / Partition extra options"] = '{"target": 1}'
    ExportDatabase(e0, opts0)
    time.sleep(1)

    # Test YAML
    e1 = ExportDBAttributes()
    e1.db_type = "Blueprint"
    e1.filename = "multi_rect2d_yaml_target_1"
    e1.variables = ("u")
    opts1 = GetExportOptions("Blueprint")
    opts1["Operation"] = "Partition"
    opts1["Flatten / Partition extra options"]  = 'target: 1'
    ExportDatabase(e1, opts1)
    time.sleep(1)

    # Test that the JSON/YAML overrides the options field
    e2 = ExportDBAttributes()
    e2.db_type = "Blueprint"
    e2.filename = "multi_rect2d_override_target_1"
    e2.variables = ("u")
    opts2 = GetExportOptions("Blueprint")
    opts2["Operation"] = "Partition"
    opts2["Partition target number of domains"] = 13
    opts2["Flatten / Partition extra options"]  = 'target: 1'
    ExportDatabase(e2, opts2)
    time.sleep(1)
    DeleteAllPlots()
    CloseDatabase(silo_data_path(dbname))

    OpenDatabase("multi_rect2d_json_target_1.cycle_000000.root")
    AddPlot("Subset", "domains")
    DrawPlots()
    Test("JSONOptions")
    DeleteAllPlots()
    CloseDatabase("multi_rect2d_json_target_1.cycle_000000.root")

    OpenDatabase("multi_rect2d_yaml_target_1.cycle_000000.root")
    AddPlot("Subset", "domains")
    DrawPlots()
    Test("YAMLOptions")
    DeleteAllPlots()
    CloseDatabase("multi_rect2d_yaml_target_1.cycle_000000.root")

    OpenDatabase("multi_rect2d_override_target_1.cycle_000000.root")
    AddPlot("Subset", "domains")
    DrawPlots()
    Test("OverrideOptions")
    DeleteAllPlots()
    CloseDatabase("multi_rect2d_override_target_1.cycle_000000.root")

def basic_test_case(case_name, varname = "d"):
    OpenDatabase(silo_data_path(case_name))
    AddPlot("Pseudocolor",varname)
    DrawPlots()
    Test("basic_" + case_name + "_input")
    # export default
    export_rfile_default = export_mesh_bp(case_name + "_default", varname)
    # export post isosurface
    AddOperator("Isosurface")
    DrawPlots()
    Test("basic_" + case_name + "_isosurface")
    export_rfile_isos = export_mesh_bp(case_name + "_isosurface", varname)
    DeleteAllPlots()
    CloseDatabase(silo_data_path(case_name))

    OpenDatabase(export_rfile_default)
    # bp var names are qualified by topo
    AddPlot("Pseudocolor","mesh_topo/" + varname)
    DrawPlots()
    Test("basic_" + case_name + "_default_exported")
    DeleteAllPlots()
    CloseDatabase(export_rfile_default)

    OpenDatabase(export_rfile_isos)
    # bp var names are qualified by topo
    AddPlot("Pseudocolor", "mesh_topo/" + varname)
    DrawPlots()
    Test("basic_" + case_name + "_isosurface_exported")
    DeleteAllPlots()
    CloseDatabase(export_rfile_isos)


def test_basic():
    basic_test_case("multi_rect3d.silo")
    basic_test_case("multi_curv3d.silo")
    basic_test_case("multi_curv2d.silo")
    basic_test_case("multi_ucd3d.silo")
    basic_test_case("multi_rect2d.silo")

def test_flatten():
    TestSection("Blueprint flatten")

    flatten_noise()
    flatten_multi_3d_case('r')
    flatten_multi_3d_case('s')
    flatten_multi_3d_case('u')

    flatten_multi_2d_case('r')
    flatten_multi_2d_case('s')
    flatten_multi_2d_unstructured_case()

def test_partition():
    TestSection("Blueprint partition")

    # Need a couple different views
    rect3d_view = GetView3D()
    rect3d_view.viewNormal = (-0.467474, 0.301847, 0.830877)
    rect3d_view.focus = (0.5, 0.5, 0.5)
    rect3d_view.viewUp = (0.140705, 0.953323, -0.267166)
    rect3d_view.viewAngle = 30
    rect3d_view.parallelScale = 0.866025
    rect3d_view.nearPlane = -1.73205
    rect3d_view.farPlane = 1.73205
    rect3d_view.imagePan = (-0.0154649, 0.027457)
    rect3d_view.imageZoom = 1.14276
    rect3d_view.perspective = 1
    rect3d_view.eyeAngle = 2
    rect3d_view.centerOfRotationSet = 0
    rect3d_view.centerOfRotation = (0.5, 0.5, 0.5)
    rect3d_view.axis3DScaleFlag = 0
    rect3d_view.axis3DScales = (1, 1, 1)
    rect3d_view.shear = (0, 0, 1)
    rect3d_view.windowValid = 1

    curv3d_view = GetView3D()
    curv3d_view.viewNormal = (-0.254971, 0.246468, 0.93501)
    curv3d_view.focus = (0, 2.5, 15)
    curv3d_view.viewUp = (0.0453522, 0.968953, -0.243049)
    curv3d_view.viewAngle = 30
    curv3d_view.parallelScale = 16.0078
    curv3d_view.nearPlane = -32.0156
    curv3d_view.farPlane = 32.0156
    curv3d_view.imagePan = (0, 0)
    curv3d_view.imageZoom = 1.21
    curv3d_view.perspective = 1
    curv3d_view.eyeAngle = 2
    curv3d_view.centerOfRotationSet = 0
    curv3d_view.centerOfRotation = (0, 2.5, 15)
    curv3d_view.axis3DScaleFlag = 0
    curv3d_view.axis3DScales = (1, 1, 1)
    curv3d_view.shear = (0, 0, 1)
    curv3d_view.windowValid = 1

    # Run 3D tests
    targets_3d = (1, 4, 19, 45)
    partition_test_case("multi_rect3d.silo",
        targets_3d, rect3d_view)
    partition_test_case("multi_curv3d.silo",
        targets_3d, curv3d_view)
    partition_test_case("multi_ucd3d.silo",
        targets_3d, curv3d_view)

    # Run 2D tests
    targets_2d = (1, 4, 7, 13, 19)
    partition_test_case("multi_curv2d.silo",
        targets_2d)
    partition_test_case("multi_rect2d.silo",
        targets_2d)

    # Test extra options
    partition_test_extra_options()

def roundtrip_mixed_topo():
    bp_mixed_topos_dir = "blueprint_v0.9.2_mixed_topo_data"
    mixed_topo_2d = data_path(pjoin(bp_mixed_topos_dir, "mixed_mesh_simple_2d_hdf5.root"))
    mixed_topo_2d_polygon = data_path(pjoin(bp_mixed_topos_dir, "mixed_mesh_polygonal_2d_hdf5.root"))
    mixed_topo_3d = data_path(pjoin(bp_mixed_topos_dir, "mixed_mesh_simple_3d_hdf5.root"))
    mixed_braid_2d = data_path(pjoin(bp_mixed_topos_dir, "braid_2d_examples_hdf5.root"))
    mixed_braid_3d = data_path(pjoin(bp_mixed_topos_dir, "braid_3d_examples_hdf5.root"))

    OpenDatabase(mixed_topo_2d)
    AddPlot("Mesh", "mesh_topo")
    AddPlot("Pseudocolor", "mesh_topo/ele_id")
    DrawPlots()
    ResetView()

    outfilename = export_mesh_bp("mixed_topo_2d", "mesh_topo/ele_id")

    save_mesh = conduit.Node()
    conduit.relay.io.blueprint.load_mesh(save_mesh, mixed_topo_2d)

    load_mesh = conduit.Node()
    conduit.relay.io.blueprint.load_mesh(load_mesh, outfilename)

    # 
    # make changes so the diff will pass
    # 

    # grab the cycle and time which were added in
    save_mesh[0]["state"]["cycle"] = load_mesh[0]["state"]["cycle"]
    save_mesh[0]["state"]["time"] = load_mesh[0]["state"]["time"]

    # remove the pts topo
    save_mesh[0]["topologies"].remove_child("pts")

    # remove the pts_id field
    save_mesh[0]["fields"].remove_child("pts_id")

    # change ele_id field to floats
    save_mesh_element_ids = save_mesh[0]["fields"]["ele_id"]["values"]
    load_mesh_element_ids = load_mesh[0]["fields"]["ele_id"]["values"]
    for i in range(0, len(save_mesh_element_ids)):
        TestValueEQ("Mixed_topo_simple_2d_export_field_vals", save_mesh_element_ids[i], load_mesh_element_ids[i])
    save_mesh[0]["fields"]["ele_id"].remove_child("values")
    load_mesh[0]["fields"]["ele_id"].remove_child("values")

    info = conduit.Node()
    diffval = load_mesh.diff(save_mesh, info)
    diff_str = info.to_yaml() if diffval else ""
    TestValueEQ("Mixed_topo_simple_2d_export", diff_str, "")
    
    DeleteAllPlots()
    CloseDatabase(mixed_topo_2d)

RequiredDatabasePlugin("Blueprint")
test_basic()
test_partition()
test_flatten()
roundtrip_mixed_topo()
Exit()
