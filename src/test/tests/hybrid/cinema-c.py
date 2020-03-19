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
#    Kathleen Biagas, Tue Jun 11 11:44:14 PDT 2019
#    Pass '-noconfig' to generated command line in GenerateCinema.
#
# ----------------------------------------------------------------------------
import math, os, string, subprocess, zlib

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

def write_setup_plot(f):
    f.write('AddPlot("Pseudocolor", "pressure")\n')
    f.write('AddOperator("Clip")\n')
    f.write('clip = GetOperatorOptions(0)\n')
    f.write('clip.plane1Normal = (0,0,1)\n')
    f.write('clip.plane1Origin = (0,0,2.5)\n')
    f.write('SetOperatorOptions(clip)\n')
    f.write('AddPlot("Pseudocolor", "pressure", 1, 1)\n')
    f.write('clip.plane1Normal = (0,0,-1)\n')
    f.write('SetOperatorOptions(clip)\n')
    f.write('DrawPlots()\n')
    f.write('a = GetAnnotationAttributes()\n')
    f.write('a.axes3D.visible = 0\n')
    f.write('a.axes3D.bboxFlag = 0\n')
    f.write('a.axes3D.triadFlag = 0\n')
    f.write('a.legendInfoFlag = 0\n')
    f.write('a.databaseInfoFlag = 0\n')
    f.write('a.userInfoFlag = 0\n')
    f.write('a.backgroundColor=(0,0,0,255)\n')
    f.write('a.backgroundMode=a.Solid\n')
    f.write('SetAnnotationAttributes(a)\n')

def test_Z_file(testname, varname, imagefilename, domask):
    """
    Decompress a float32 buffer, make a BOV file from it, and plot it.
    """
    cbytes = open(imagefilename, "rb").read()
    fbytes = zlib.decompress(cbytes)
    datfile = testname + ".dat"
    bovfile = testname + ".bov"
    f = open(datfile, "wb")
    f.write(fbytes)
    f.close()
    imagesize = int(math.sqrt(len(fbytes)/4))
    f = open(bovfile, "wt")
    f.write("TIME: 0\n")
    f.write("DATA_FILE: %s\n" % datfile)
    f.write("DATA_SIZE: %d %d 1\n" % (imagesize, imagesize))
    f.write("DATA_FORMAT: FLOAT\n")
    f.write("VARIABLE: %s\n" % varname)
    f.write("DATA_ENDIAN: LITTLE\n")
    f.write("CENTERING: zonal\n")
    f.write("BRICK_ORIGIN: 0 0 0\n")
    f.write("BRICK_SIZE: 1 1 1\n")
    f.close()
    OpenDatabase(bovfile)
    if domask:
        # Mask out values larger than 255 so we can see the values.
        # I'd mask on Z but I don't want to do cmfe.
        DefineScalarExpression("maskvar", "if(gt(%s, 255), 0., %s)" % (varname,varname))
        AddPlot("Pseudocolor", "maskvar")
    else:
        AddPlot("Pseudocolor", varname)
    AddOperator("Transform")
    t = TransformAttributes()
    t.doScale = 1
    t.scaleY = -1
    SetOperatorOptions(t)
    DrawPlots()
    ResetView()
    v = GetView2D()
    v.viewportCoords = (0,1,0,1)
    SetView2D(v)
    Test(testname + "_" + varname)
    DeleteAllPlots()
    CloseDatabase(bovfile)

    os.unlink(datfile)
    os.unlink(bovfile)

def test_composite(testname, imagepath, scalars):
    """
    Make test images for a composite image.
    """
    img0 = os.path.join(imagepath, "image=0.png")
    img1 = os.path.join(imagepath, "image=1.Z")
    OpenDatabase(img0)
    AddPlot("Truecolor", "color")
    DrawPlots()
    ResetView()
    v = GetView2D()
    v.viewportCoords = (0,1,0,1)
    SetView2D(v)
    Test(testname + "_lum")
    DeleteAllPlots()
    CloseDatabase(img0)

    test_Z_file(testname, "z", os.path.join(imagepath, "image=1.Z"), 0)
    idx = 2
    for s in scalars:
        test_Z_file(testname, s, os.path.join(imagepath, "image=%d.Z" % idx), 1)
        idx = idx + 1

def test2(db):
    TestSection("Cinema spec C static camera")
    f = open("test2.py", "wt")
    f.write('OpenDatabase("%s")\n' % db)
    write_setup_plot(f)
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
    f.close()

    cdb = "test2.cdb"
    scalars = ("pressure", "v")
    sz = "%dx%d" % (TestEnv.params["width"], TestEnv.params["height"])
    args = ["-specification", "C", "-docomposite", "-scriptfile", "test2.py", "-output", cdb, "-format", "png", "-geometry", sz, "-camera", "static", "-var", scalars[0], "-var", scalars[1]]
    TestText("cinema_2_00", ListToString(args))

    output = GenerateCinema(args)

    files = GetFileList(cdb)
    TestText("cinema_2_01", ListToString(files))

    json = ReadTextFile(os.path.join(cdb, GetFile(files, "info.json")))
    TestText("cinema_2_02", json)

    # Test a couple of composite images
    test_composite("cinema_2_03", os.path.join(cdb, "image","time=3","vis=0"), scalars)
    test_composite("cinema_2_04", os.path.join(cdb, "image","time=3","vis=1"), scalars)


def test3(db):
    TestSection("Cinema spec C phi-theta camera")
    f = open("test3.py", "wt")
    f.write('OpenDatabase("%s")\n' % db)
    write_setup_plot(f)
    f.close()

    cdb = "test3.cdb"
    scalars = ("pressure", "v")
    sz = "%dx%d" % (TestEnv.params["width"], TestEnv.params["height"])
    args = ["-specification", "C", "-docomposite", "-scriptfile", "test3.py", "-output", cdb, "-format", "png", "-geometry", sz, "-camera", "phi-theta", "-phi", "6", "-theta", "5", "-var", scalars[0], "-var", scalars[1]]
    TestText("cinema_3_00", ListToString(args))

    output = GenerateCinema(args)

    files = GetFileList(cdb)
    TestText("cinema_3_01", ListToString(files))

    json = ReadTextFile(os.path.join(cdb, GetFile(files, "info.json")))
    TestText("cinema_3_02", json)

    # Test a couple of composite images
    test_composite("cinema_3_03", os.path.join(cdb, "image","phi=0","theta=0","time=0","vis=0"), scalars)
    test_composite("cinema_3_04", os.path.join(cdb, "image","phi=0","theta=0","time=0","vis=1"), scalars)
    test_composite("cinema_3_05", os.path.join(cdb, "image","phi=3","theta=1","time=0","vis=0"), scalars)
    test_composite("cinema_3_06", os.path.join(cdb, "image","phi=3","theta=1","time=0","vis=1"), scalars)


def MakeShortWave(incr):
    db = "short_wave.visit"
    f = open(db, "wt")
    for i in range(0, 700, incr):
        f.write(silo_data_path("wave%04d.silo" % i) + "\n")
    f.close()
    return os.path.abspath(db)

def main():
    test2(MakeShortWave(100))
    test3(MakeShortWave(500))

main()
Exit()
