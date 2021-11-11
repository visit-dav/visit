import time
import os.path

def Exit():
    print("Exiting test...")
    return

def RequiredDatabasePlugin(name):
    print("This test requires database plugin " + name)
    return

def TestSection(name):
    print("---- " + name + " ----")
    return

def Test(name):
    s = SaveWindowAttributes()
    s.fileName = name
    SetSaveWindowAttributes(s)
    SaveWindow()
    return

def TestText(name):
    print("Testing text file")

def set_view(case_name, view=None):
    if "2d" in case_name:
        ResetView()
    else:
        SetView3D(view)

def silo_data_path(name):
    retval = os.path.join("/mnt/data/il/VisIt/VisItClass/data", name)
    # print(retval)
    return retval

def test_name(case, i):
    return case + "_" + str(i) + "_"

def flatten_test_case(case_name, vars):
    OpenDatabase(silo_data_path(case_name))
    AddPlot("Pseudocolor", "u")
    DrawPlots()

    # Set the export database attributes.
    export_name = case_name + "_flatten"
    export_dir  = export_name + ".csv"
    e = ExportDBAttributes()
    e.db_type = "Blueprint"
    e.filename = export_name
    e.variables = vars
    opts = GetExportOptions("Blueprint")
    opts["Operation"] = "FlattenCSV"
    ExportDatabase(e, opts)
    time.sleep(1)

    DeleteAllPlots()
    CloseDatabase(silo_data_path(case_name))

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

def test_flatten():
    TestSection("Blueprint flatten")



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

def main():
    RequiredDatabasePlugin("Blueprint")
    test_partition()


main()
Exit()
