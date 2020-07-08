# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  export_db.py
#
#  Tests:      mesh      - 3D unstructured
#              plots     - Pseudocolor
#
#  Defect ID:  '6288, '6290, '6587, '6366
#
#  Programmer: Hank Childs
#  Date:       July 1, 2005
#
#  Modifications:
#
#    Hank Childs, Thu Sep 15 16:33:48 PDT 2005
#    Add test for exporting CMFEs as secondary variables ('6587)
#
#    Kathleen Bonnell, Tue May  2 08:58:01 PDT 2006 
#    Corrected exported database names (from .visit to .vtk). 
#
#    Hank Childs, Wed Mar 28 11:33:16 PDT 2007
#    Uncommented tests for '6366, which were previously checked in, but
#    commented out.
#
#    Mark C. Miller Fri Aug  8 09:11:44 PDT 2008
#    Fixed typo of missing '()' on call to DeleteAllPlots
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to switch between Silo's HDF5 and PDB data.
#
#    Kathleen Biagas, Wed Feb 25 14:08:53 PST 2015
#    Add Mesh Plot for silo data exported as VTK, to test that mesh name
#    was properly exported.
#
#    Kathleen Biagas, Fri Feb 17, 2017
#    Update for new VTK export options.  Added test4, which tests the new
#    options.
#
#    Alister Maguire, Thu Apr 26 13:31:31 PDT 2018
#    Added test for bov compression. 
#
#    Kathleen Biagas, Wed May 16, 2018
#    Use python's gzip instead of subprocess('gunzip') so test will run
#    correctly on Windows.  Modified AssertTrue calls to have text stating
#    what is being tested.
#
#    Mark C. Miller, Wed Apr 10 10:24:32 PDT 2019
#    Add tetrahedralize test
# ----------------------------------------------------------------------------
import time
import os.path
import subprocess

def test0():
    OpenDatabase(silo_data_path("globe.silo"))
    AddPlot("Pseudocolor", "t")
    DrawPlots()

    # Set the export database attributes.
    e = ExportDBAttributes()
    e.db_type = "VTK"
    e.filename = "test_ex_db"
    ExportDatabase(e)
    time.sleep(1)

    e.variables = ("u", "v")
    e.filename = "test_ex_db2"
    e.db_type = "Silo"
    ExportDatabase(e)
    time.sleep(1)

    DeleteAllPlots()
#    CloseDatabase(silo_data_path("globe.silo"))

    OpenDatabase("test_ex_db.vtk")
    AddPlot("Pseudocolor", "t")
    DrawPlots()
    Test("export_db_01")
    DeleteAllPlots()
#    CloseDatabase("test_ex_db.vtk")

    OpenDatabase("test_ex_db2.silo")
    AddPlot("Pseudocolor", "u")
    DrawPlots()
    Test("export_db_02")
    DeleteAllPlots()
#    CloseDatabase("test_ex_db2.silo")

    OpenDatabase(silo_data_path("wave.visit"))
    DefineScalarExpression("cmfe", "conn_cmfe(coord(<%s:quadmesh>)[1], quadmesh)" % cmfe_silo_data_path("wave0020.silo"))
    AddPlot("Pseudocolor", "pressure")
    DrawPlots()
    e.variables = ("cmfe")
    e.filename = "test_ex_db3"
    e.db_type = "VTK"
    ExportDatabase(e)
    time.sleep(1)
    DeleteAllPlots()

    DeleteExpression("cmfe")
    OpenDatabase("test_ex_db3.vtk")
    AddPlot("Pseudocolor", "cmfe")
    DrawPlots()
    Test("export_db_03")

    # Ensure meshname was written by creating mesh plot with old db's meshname
    AddPlot("Mesh", "quadmesh")
    DrawPlots()
    Test("export_db_04")
    DeleteAllPlots()
#    CloseDatabase("test_ex_db3.vtk")
#    CloseDatabase(silo_data_path("wave.visit"))

def VTK_check_binary(filename):
    if (sys.version_info > (3, 0)):
        f = open(filename, "rt", encoding='utf-8', errors='ignore')
    else:
        f = open(filename, "rt")
    line = ""
    for i in (0,1,2):
        line = f.readline()
    b = "BINARY" == line[0:6]
    return b

def cleanup_files():
    exts = ("*.ply", "*.raw", "*.stl", "*.silo", "*.tec", "*.okc", "*.visit", "*.xyz", "*.obj")
    files = os.listdir(".")
    for f in files:
        for e in exts:
            pattern = e[1:]
            if f[-len(pattern):] == pattern:
                os.unlink(f)
                break

def test1():
    TestSection("Test export of some surfaces.")
    maindb = silo_data_path("multi_rect3d.silo")
    OpenDatabase(maindb)
    DefineScalarExpression("rad", "magnitude(coords(mesh1))")
    AddPlot("Pseudocolor", "u")
    AddOperator("Isosurface")
    iso = IsosurfaceAttributes()
    iso.contourValue = (0.55, 1.455)
    iso.contourMethod = iso.Value
    iso.variable = "rad"
    SetOperatorOptions(iso)
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (-0.467474, 0.301847, 0.830877)
    v.focus = (0.5, 0.5, 0.5)
    v.viewUp = (0.140705, 0.953323, -0.267166)
    v.viewAngle = 30
    v.parallelScale = 0.866025
    v.nearPlane = -1.73205
    v.farPlane = 1.73205
    v.imagePan = (-0.0154649, 0.027457)
    v.imageZoom = 1.14276
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.5, 0.5, 0.5)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)
    Test("export_db_1_00")

    # Test VTK binary
    e = ExportDBAttributes()
    e.db_type = "VTK"
    e.filename = "binary_VTK"
    opts = GetExportOptions("VTK")
    #opts['Binary format'] = 1
    opts['FileFormat'] = 1
    ExportDatabase(e, opts)
    time.sleep(1)
    line = "The binary_VTK.0.vtk file is NOT binary.\n\n"
    visitfile = " ".join(open("binary_VTK.visit").readlines())
    if VTK_check_binary("binary_VTK/binary_VTK.0.vtk"):
        line = "The binary_VTK.0.vtk file is binary.\n\n"
    s = line + visitfile
    TestText("export_db_1_01", s)

    a = GetAnnotationAttributes()

    AddWindow()
    OpenDatabase("binary_VTK.visit")
    AddPlot("Pseudocolor", "u")
    DrawPlots()
    t = CreateAnnotationObject("Text2D")
    t.text = "VTK"
    t.position = (0.01, 0.91)
    t.height = 0.07
    SetView3D(v)
    SetAnnotationAttributes(a)
    Test("export_db_1_02")
#    CloseDatabase("binary_VTK.visit")
    DeleteWindow()

    # Test exporting some surfaces in other formats and plotting the results.
    formats = {
"PLY":  ("export_PLY",  "export_PLY.ply",   "export_db_1_03", "Subset", "PLY_mesh"),
"RAW":  ("export_RAW",  "export_RAW.raw",   "export_db_1_04", "Subset", "mesh"),
"STL":  ("export_STL",  "export_STL.stl",   "export_db_1_05", "Subset", "STL_mesh"),
"Silo": ("export_Silo", "export_Silo.silo", "export_db_1_06", "Pseudocolor", "u"),
"Tecplot": ("export_Tecplot", "export_Tecplot.tec", "export_db_1_07", "Pseudocolor", "u"),
"WavefrontOBJ": ("export_OBJ", "export_OBJ.obj", "export_db_1_08", "Subset", "OBJMesh"),
"XYZ":  ("export_XYZ", "export_XYZ.xyz", "export_db_1_09", "Subset", "mesh"),
"Xmdv": ("export_Xmdv", "export_Xmdv.visit", "export_db_1_10", "Pseudocolor", "u")
}
    keys = sorted(formats.keys())
    for f in keys:
        e.db_type = f
        e.db_type_fullname = f + "_1.0"
        e.filename = formats[f][0]
        ExportDatabase(e)
        time.sleep(1)

        # Add the exported data in window 2.
        AddWindow()
        if OpenDatabase(formats[f][1]):
            md = GetMetaData(formats[f][1])
            AddPlot(formats[f][3], formats[f][4])
            DrawPlots()
        else:
            files = os.listdir(".")
            files.sort()
            s = ""
            for fn in files:
                if formats[f][0] in fn:
                    s = fn + "\n"
            TestText("files", s)
        t = CreateAnnotationObject("Text2D")
        t.text = f
        t.position = (0.01, 0.91)
        t.height = 0.07
        SetView3D(v)
        SetAnnotationAttributes(a)
        Test(formats[f][2])
        # Clean up window 2
        DeleteAllPlots()
    #    CloseDatabase(formats[f][1])
        DeleteWindow()

    # Clean up window 1
    DeleteAllPlots()
#    CloseDatabase(maindb)
    if not platform.system() == "Windows":
        cleanup_files()

def test2(writeGroupSize):
    TestSection("Test export with write groups (parallel).")
    maindb = silo_data_path("multi_rect3d.silo")
    OpenDatabase(maindb)
    DefineScalarExpression("rad", "magnitude(coords(mesh1))")
    AddPlot("Pseudocolor", "u")
    AddOperator("Isosurface")
    iso = IsosurfaceAttributes()
    iso.contourValue = (0.55, 1.455)
    iso.contourMethod = iso.Value
    iso.variable = "rad"
    SetOperatorOptions(iso)
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (-0.467474, 0.301847, 0.830877)
    v.focus = (0.5, 0.5, 0.5)
    v.viewUp = (0.140705, 0.953323, -0.267166)
    v.viewAngle = 30
    v.parallelScale = 0.866025
    v.nearPlane = -1.73205
    v.farPlane = 1.73205
    v.imagePan = (-0.0154649, 0.027457)
    v.imageZoom = 1.14276
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.5, 0.5, 0.5)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)
    Test("export_db_2_00")

    a = GetAnnotationAttributes()

    # Test exporting some surfaces in other formats and plotting the results.
    formats = {
"PLY":  ("wg_PLY",  "*.ply",   "export_db_2_01", "Subset", "PLY_mesh"),
"RAW":  ("wg_RAW",  "*.raw",   "export_db_2_02", "Subset", "mesh"),
"STL":  ("wg_STL",  "*.stl",   "export_db_2_03", "Subset", "STL_mesh"),
"Silo": ("wg_Silo", "wg_Silo.silo", "export_db_2_04", "Pseudocolor", "u"),
"Tecplot": ("wg_Tecplot", "*.tec", "export_db_2_05", "Pseudocolor", "u"),
"VTK":  ("wg_VTK", "wg_VTK.visit", "export_db_2_06", "Pseudocolor", "u"),
"WavefrontOBJ": ("wg_OBJ", "*.obj", "export_db_2_07", "Subset", "OBJMesh"),
"XYZ":  ("wg_XYZ", "*.xyz", "export_db_2_08", "Subset", "mesh"),
"Xmdv": ("wg_Xmdv", "wg_Xmdv.visit", "export_db_2_09", "Pseudocolor", "u")
}
    keys = sorted(formats.keys())
    for f in keys:
        e = ExportDBAttributes()
        e.db_type = f
        e.db_type_fullname = f + "_1.0"
        e.filename = formats[f][0]
        e.writeUsingGroups = 1
        e.groupSize = writeGroupSize
        ExportDatabase(e)
        time.sleep(1)

        # Add the exported database in window 2.
        AddWindow()
        pattern = formats[f][1]
        filelist=""
        opendbs = []
        if pattern[0] == '*':
            # Read all of the filenames
            files = sorted(os.listdir("."))
            ext = pattern[2:]
            for datafile in files:
                if datafile[-len(ext):] == ext:
                    OpenDatabase(datafile)
                    AddPlot(formats[f][3], formats[f][4])
                    opendbs = opendbs + [datafile]
                    filelist = filelist + datafile + "\n"
        else:
            if OpenDatabase(formats[f][1]):
                md = GetMetaData(formats[f][1])
                AddPlot(formats[f][3], formats[f][4])
                opendbs = opendbs + [formats[f][1]]
                filelist = filelist + formats[f][1] + "\n"
            else:
                filelist = "ERROR: " + "\n".join(os.listdir("."))
        DrawPlots()
        t = CreateAnnotationObject("Text2D")
        t.text = f
        t.position = (0.01, 0.91)
        t.height = 0.07
        SetView3D(v)
        SetAnnotationAttributes(a)
        Test(formats[f][2])
        TestText(formats[f][2] + "fn", filelist)

        # Clean up window 2
        DeleteAllPlots()
#        for db in opendbs:
#            CloseDatabase(db)
        DeleteWindow()

    # Clean up window 1
    DeleteAllPlots()
    if not platform.system() == "Windows":
        cleanup_files()
#    CloseDatabase(maindb)

def test3():
    TestSection("Test Tecplot multiblock export.")
    maindb = silo_data_path("multi_rect3d.silo")
    OpenDatabase(maindb)
    DefineScalarExpression("rad", "magnitude(coords(mesh))")
    AddPlot("Pseudocolor", "u")
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (-0.467474, 0.301847, 0.830877)
    v.focus = (0.5, 0.5, 0.5)
    v.viewUp = (0.140705, 0.953323, -0.267166)
    v.viewAngle = 30
    v.parallelScale = 0.866025
    v.nearPlane = -1.73205
    v.farPlane = 1.73205
    v.imagePan = (-0.0154649, 0.027457)
    v.imageZoom = 1.14276
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.5, 0.5, 0.5)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)
    Test("export_db_3_00")
    a = GetAnnotationAttributes()

    e = ExportDBAttributes()
    e.db_type = "Tecplot"
    e.db_type_fullname = "Tecplot_1.0"
    e.filename = "rectTecplot"
    e.writeUsingGroups = 0
    e.variables = ("default", "v", "rad")
    ExportDatabase(e)
    time.sleep(1)

    # Add the exported database in window 2
    AddWindow()
    OpenDatabase("rectTecplot.tec")
    AddPlot("Pseudocolor", "u")
    DrawPlots()
    SetAnnotationAttributes(a)
    SetView3D(v)
    Test("export_db_3_01")
    ChangeActivePlotsVar("v")
    Test("export_db_3_02")
    ChangeActivePlotsVar("rad")
    Test("export_db_3_03")

    # Make sure we have volume data
    AddOperator("ThreeSlice")
    atts = ThreeSliceAttributes()
    atts.y = 0.5
    atts.x = 0.5
    atts.z = 0.5
    SetOperatorOptions(atts)
    DrawPlots()
    Test("export_db_3_04")

    # Clean up window 2
    DeleteAllPlots()
#    CloseDatabase("rectTecplot.tec")
    DeleteWindow()

    # Back to window 1
    DeleteAllPlots()
#    CloseDatabase(maindb)

def test4():
    TestSection("Test VTK multiblock export.")
    OpenDatabase(silo_data_path("multi_rect3d.silo"))
    AddPlot("Pseudocolor", "d")
    DrawPlots()

    Test("export_db_4_01")

    e = ExportDBAttributes()
    e.db_type = "VTK"
    e.filename = "multi_rect3d_LA"
    opts = GetExportOptions("VTK")
    opts['FileFormat'] = "Legacy Ascii"
    ExportDatabase(e, opts)
    time.sleep(1)
    ReplaceDatabase("multi_rect3d_LA.visit")
    Test("export_db_4_02")

    e.filename = "multi_rect3d_LB"
    opts['FileFormat'] = "Legacy Binary"
    ExportDatabase(e, opts)
    time.sleep(1)
    ReplaceDatabase("multi_rect3d_LB.visit")
    Test("export_db_4_03")

    e.filename = "multi_rect3d_XA"
    opts['FileFormat'] = "XML Ascii"
    ExportDatabase(e, opts)
    time.sleep(1)
    ReplaceDatabase("multi_rect3d_XA.vtm")
    Test("export_db_4_04")

    e.filename = "multi_rect3d_XB"
    opts['FileFormat'] = "XML Binary"
    ExportDatabase(e, opts)
    time.sleep(1)
    ReplaceDatabase("multi_rect3d_XB.vtm")
    Test("export_db_4_05")

    DeleteAllPlots()

def test_bov():
    TestSection("Test bov export with and without compression.")
    maindb = silo_data_path("noise.silo")
    OpenDatabase(maindb)
    AddPlot("Pseudocolor", "hardyglobal")
    DrawPlots()

    #
    # test w/o compression
    #
    e = ExportDBAttributes()
    e.db_type = "BOV"
    e.db_type_fullname = "BOV_1.0"
    e.filename = "test_bov_uncompressed"
    e.variables = ("default")
    opts = GetExportOptions("BOV")
    opts["Compression"] = "None"
    ExportDatabase(e, opts)
    time.sleep(1)
    AssertTrue("test_bov_uncompressed.bov exists", os.path.isfile("test_bov_uncompressed.bov"))
    AssertTrue("test_bov_uncompressed exists", os.path.isfile("test_bov_uncompressed"))
    ReplaceDatabase("test_bov_uncompressed.bov")
    Test("export_db_5_01")

    #
    # test w gzip compression
    #
    e.filename = "test_bov_gzip"
    opts["Compression"] = "gzip"
    ExportDatabase(e, opts)
    time.sleep(1)
    AssertTrue("test_bov_gzip.bov exists", os.path.isfile("test_bov_gzip.bov"))
    AssertTrue("test_bov_gzip.gz exists", os.path.isfile("test_bov_gzip.gz"))
    with gzip.open("test_bov_gzip.gz", "rb") as f_in:
        with open("test_bov_gzip", "wb") as f_out:
            shutil.copyfileobj(f_in, f_out);
    ReplaceDatabase("test_bov_gzip.bov")
    Test("export_db_5_02")

    DeleteAllPlots()


def test_vtk_tetrahedralize():
    dbs_noext = ["ucd3d", "specmix_ucd"]
    for db_noext in dbs_noext:
        db = "%s.silo"%db_noext
        OpenDatabase(silo_data_path(db))
        AddPlot("Pseudocolor","d") # both dbs have variable 'd'
        DrawPlots()
        Query("NumZones")
        nzOrig = int(GetQueryOutputValue())

        # Test VTK binary
        e = ExportDBAttributes()
        e.db_type = "VTK"
        e.filename = "%s_ascii_VTK_tets"%db_noext
        opts = GetExportOptions("VTK")
        opts['FileFormat'] = "Legacy Ascii"
        opts['Tetrahedralize'] = True
        ExportDatabase(e, opts)
        time.sleep(1)
        DeleteAllPlots()
#        CloseDatabase(silo_data_path(db))
        OpenDatabase("%s_ascii_VTK_tets.vtk"%db_noext)
        AddPlot("Pseudocolor","d")
        DrawPlots()
        Query("NumZones")
        nzNew = int(GetQueryOutputValue())
        TestText("export_db_vtk_tets_%s"%db_noext, "Ratio of exported zone count to original is %d"%(nzNew/nzOrig))
        DeleteAllPlots()

def main():
    test0()
    test1()
    if GetEngineProperties(GetEngineList()[0]).numProcessors > 1:
        # We just use 2 processors normally so let's set the write group size 
        # to 1 so each rank will write its own data using a different write 
        # group. For certain formats, there will be 1 data file per write 
        # group, or in this case 2 output files.
        test2(1)
    test3()
    test4()
    test_bov()
    test_vtk_tetrahedralize()

main()
Exit()

