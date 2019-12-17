# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  cinema-a.py
#
#  Tests:      Tests various aspects of "visit -cinema".
#
#  Notes:      
#
#  Programmer: Brad Whitlock
#  Date:       Thu Feb 15 16:37:20 PST 2018
#
#  Modifications:
#    Kathleen Biagas, Friday December 14, 2018
#    Fixes for running on Windows: Use abs_path for short_wave.visit, and cdb,
#    use 'repr(db)' when writing script file to preserve '\' escapes.
#    Replace forward-slash with back-slash in pattern.
#
#    Kathleen Biagas, Tue Jun 11 11:44:14 PDT 2019
#    Pass '-noconfig' to generated command line in GenerateCinema.
#
# ----------------------------------------------------------------------------
import os, string, subprocess

def GenerateCinema(cinemaArgs):
    if TestEnv.params["parallel"]:
        args = [TestEnv.params["visit_bin"], "-noconfig", "-cinema", "-np", "2", "-l", TestEnv.params["parallel_launch"]] + cinemaArgs
    else:
        args = [TestEnv.params["visit_bin"], "-noconfig", "-cinema"] + cinemaArgs
    p = subprocess.check_output(args)
    return p

def GetFileList(path0):
    def gfl(path):
        outfiles = []
        files = os.listdir(path)
        for f in sorted(files):
            fpath = os.path.join(path, f)
            if os.path.isdir(fpath):
                outfiles = outfiles + gfl(fpath)
            else:
                outfiles = outfiles + [fpath[lpath0+1:]]
        return outfiles
    lpath0 = len(path0)
    return gfl(path0)

def ListToString(files):
    s = ""
    for f in files:
        s += f
        s += "\n"
    return s

def GetFile(manyfilenames, filename):
    for f in manyfilenames:
        if string.find(f, filename) != -1:
            return f
    return ""

def ReadTextFile(filename):
    lines = open(filename, "rt").readlines()
    s = ""
    for line in lines:
        s += line
    return s

def test0(db):
    TestSection("Cinema spec A static camera")
    f = open("test0.py", "wt")
    f.write("OpenDatabase(%s)\n" % repr(db))
    f.write('AddPlot("Pseudocolor", "pressure")\n')
    f.write('DrawPlots()\n')
    f.write('v = GetView3D()\n')
    f.write('v.viewNormal = (-0.569392, 0.672931, 0.472183)\n')
    f.write('v.focus = (5, 0.353448, 2.5)\n')
    f.write('v.viewUp = (0.562941, 0.737756, -0.372577)\n')
    f.write('v.viewAngle = 30\n')
    f.write('v.parallelScale = 5.6009\n')
    f.write('v.nearPlane = -11.2018\n')
    f.write('v.farPlane = 11.2018\n')
    f.write('v.imagePan = (0.0431021, 0.0442006)\n')
    f.write('v.imageZoom = 1.06444\n')
    f.write('v.perspective = 1\n')
    f.write('v.eyeAngle = 2\n')
    f.write('v.centerOfRotationSet = 0\n')
    f.write('v.centerOfRotation = (5, 0.353448, 2.5)\n')
    f.write('v.axis3DScaleFlag = 0\n')
    f.write('v.axis3DScales = (1, 1, 1)\n')
    f.write('v.shear = (0, 0, 1)\n')
    f.write('v.windowValid = 1\n')
    f.write('SetView3D(v)\n')
    f.write('a = GetAnnotationAttributes()\n')
    f.write('a.axes3D.visible = 0\n')
    f.write('a.axes3D.bboxFlag = 0\n')
    f.write('a.axes3D.triadFlag = 0\n')
    f.write('a.legendInfoFlag = 0\n')
    f.write('a.databaseInfoFlag = 0\n')
    f.write('a.userInfoFlag = 0\n')
    f.write('SetAnnotationAttributes(a)\n')
    f.close()

    cdb = abs_path(TestEnv.params["run_dir"], "test0.cdb")
    sz = "%dx%d" % (TestEnv.params["width"], TestEnv.params["height"])
    args = ["-specification", "A", "-scriptfile", "test0.py", "-output", cdb, "-format", "png", "-geometry", sz, "-camera", "static"]
    TestText("cinema_0_00", ListToString(args))

    output = GenerateCinema(args)

    files = GetFileList(cdb)
    TestText("cinema_0_01", ListToString(files))

    json = ReadTextFile(os.path.join(cdb, GetFile(files, "info.json")))
    TestText("cinema_0_02", json)

    firstimg = os.path.join(cdb, GetFile(files, ".png"))
    OpenDatabase(firstimg)
    AddPlot("Truecolor", "color")
    DrawPlots()
    v = GetView2D()
    v.viewportCoords = (0,1,0,1)
    SetView2D(v)
    Test("cinema_0_03")
    DeleteAllPlots()
    CloseDatabase(firstimg)

def test1(db):
    TestSection("Cinema spec A phi-theta camera")
    f = open("test1.py", "wt")
    f.write("OpenDatabase(%s)\n" % repr(db))
    f.write('AddPlot("Pseudocolor", "pressure")\n')
    f.write('DrawPlots()\n')
    f.write('a = GetAnnotationAttributes()\n')
    f.write('a.axes3D.visible = 0\n')
    f.write('a.axes3D.bboxFlag = 0\n')
    f.write('a.axes3D.triadFlag = 0\n')
    f.write('a.legendInfoFlag = 0\n')
    f.write('a.databaseInfoFlag = 0\n')
    f.write('a.userInfoFlag = 0\n')
    f.write('SetAnnotationAttributes(a)\n')
    f.close()

    cdb = abs_path(TestEnv.params["run_dir"], "test1.cdb")

    sz = "%dx%d" % (TestEnv.params["width"], TestEnv.params["height"])
    args = ["-specification", "A", "-scriptfile", "test1.py", "-output", cdb, "-format", "png", "-geometry", sz, "-camera", "phi-theta", "-phi", "6", "-theta", "5", "-stride", "3"]
    TestText("cinema_1_00", ListToString(args))

    output = GenerateCinema(args)

    files = GetFileList(cdb)
    TestText("cinema_1_01", ListToString(files))

    json = ReadTextFile(os.path.join(cdb, GetFile(files, "info.json")))
    TestText("cinema_1_02", json)

    # Use the JSON to come up with some filenames to plot. We will vary phi.
    params = eval(json)
    theta_values = [str(x) for x in params["arguments"]["theta"]["values"]]
    phi_values = [str(x) for x in  params["arguments"]["phi"]["values"]]
    time_values = params["arguments"]["time"]["values"]

    theta = theta_values[len(theta_values)/4]
    time = time_values[0]
    i = 3
    for phi in phi_values:
        pattern = params["name_pattern"]
        if sys.platform.startswith("win"):
            pattern = string.replace(pattern, "/", "\\")
        name = string.replace(pattern, "{phi}", phi)
        name = string.replace(name, "{theta}", theta)
        name = string.replace(name, "{time}", time)

        img = os.path.join(cdb, GetFile(files, name))
        OpenDatabase(img)
        AddPlot("Truecolor", "color")
        DrawPlots()
        v = GetView2D()
        v.viewportCoords = (0,1,0,1)
        SetView2D(v)
        Test("cinema_1_%02d" % i)
        DeleteAllPlots()
        CloseDatabase(img)
        i = i + 1

def MakeShortWave():
    db = abs_path("short_wave.visit")
    f = open(db, "wt")
    for i in range(0, 700, 100):
        f.write(silo_data_path("wave%04d.silo" % i) + "\n")
    f.close()
    return os.path.abspath(db)

def main():
    db = MakeShortWave()
    test0(db)
    test1(db)

    os.unlink(db)

main()
Exit()
