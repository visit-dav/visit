import os
###############################################################################
# Program: measure_mandelbulb
#
# Purpose: This program makes an isosurface through mandelbulb data and 
#          optionally a slice, saving a couple of images. From the resulting
#          timings file, we can determine how long it takes for certain 
#          operations.
#
# Running: ./visit -cli -s measure_mandelbulb.py -timing -timings-processor-stride 100
#
###############################################################################


class MemoryLogger(object):
    def __init__(self, host, fn):
        self.host = host
        self.filebase = fn
        self.samples = []

    def sample(self):
        # Sample the engine to create a starting point.
        pa = GetProcessAttributes("engine", self.host)
        #print pa.memory
        self.samples = self.samples + [pa]
        self.writeSamples()

    def writeSamples(self):
        print("writeSamples: start")
        f = open(self.filebase + ".curve", "wt")
        np0 = len(self.samples[0].memory)
        if np0 > 4096:
            np0 = 4096
        for rank in range(np0):
            f.write("# memory_rank%d\n" % rank)
            for s in range(len(self.samples)):
                np = len(self.samples[s].memory)
                if rank < np:
                    f.write("%g %g\n" % (s, self.samples[s].memory[rank]))
        f.close()
        print("writeSamples: end")

def saveImage(casename):
    s = SaveWindowAttributes()
    s.fileName = casename + "_"
    s.family = 1
    s.format = s.PNG
    s.width = 1000
    s.height = 1000 
    s.screenCapture = 0
    s.resConstraint = s.NoConstraint
    SetSaveWindowAttributes(s)
    SaveWindow()

def setView():
    v = GetView3D()
    v.SetViewNormal(0.242247, -0.911924, 0.331228)
    v.SetFocus(0, 0, 0)
    v.SetViewUp(-0.078153, 0.321946, 0.943527)
    v.SetViewAngle(30)
    v.SetParallelScale(2.07846)
    v.SetNearPlane(-4.15692)
    v.SetFarPlane(4.15692)
    v.SetImagePan(-0.00794314, 0.0116559)
    v.SetImageZoom(1.58069)
    v.SetPerspective(1)
    v.SetEyeAngle(2)
    v.SetCenterOfRotationSet(0)
    v.SetCenterOfRotation(0, 0, 0)
    v.SetAxis3DScaleFlag(0)
    v.SetAxis3DScales(1,1,1)
    v.SetShear(0,0,1)
    v.SetWindowValid(1)
    #SetView3D(v)

    v1 = GetView3D()
    v1.viewNormal = (0.446573, -0.710963, 0.543236)
    v1.focus = (-0.0318851, 4.17233e-07, 6.55651e-07)
    v1.viewUp = (-0.31605, 0.442659, 0.839145)
    v1.viewAngle = 30
    v1.parallelScale = 1.79935
    v1.nearPlane = -3.5987
    v1.farPlane = 3.5987
    v1.imagePan = (0.00854784, -0.00627101)
    v1.imageZoom = 1.47389
    v1.perspective = 1
    v1.eyeAngle = 2
    v1.centerOfRotationSet = 0
    v1.centerOfRotation = (-0.0318851, 4.17233e-07, 6.55651e-07)
    v1.axis3DScaleFlag = 0
    v1.axis3DScales = (1, 1, 1)
    v1.shear = (0, 0, 1)
    v1.windowValid = 1
    SetView3D(v1)

def setLights():
    L = GetLight(0)
    L.enabledFlag = 1
    L.type = L.Camera  # Ambient, Object, Camera
    L.direction = (-0.311, -0.137, -0.941)
    L.color = (255, 255, 255, 255)
    L.brightness = 1
    SetLight(0, L)

def setLights2():
    HideActivePlots()
    l0 = GetLight(0)
    l0.enabledFlag = 1
    l0.type = l0.Camera  # Ambient, Object, Camera
    l0.direction = (-0.311, -0.137, -0.941)
    l0.color = (255, 255, 255, 255)
    l0.brightness = 0.45
    SetLight(0, l0)

    l1 = GetLight(1)
    l1.enabledFlag = 1
    l1.type = l1.Object  # Ambient, Object, Camera
    l1.direction = (1, 0, 0)
    l1.color = (255, 0, 0, 255)
    l1.brightness = 0.33
    SetLight(1, l1)

    l2 = GetLight(2)
    l2.enabledFlag = 1
    l2.type = l2.Object  # Ambient, Object, Camera
    l2.direction = (0, -1, 0)
    l2.color = (0, 255, 0, 255)
    l2.brightness = 0.33
    SetLight(2, l2)

    l3 = GetLight(3)
    l3.enabledFlag = 1
    l3.type = l3.Object  # Ambient, Object, Camera
    l3.direction = (0, 0, -1)
    l3.color = (0, 0, 255, 255)
    l3.brightness = 0.33
    SetLight(3, l3)
    HideActivePlots()

def setRenderingAtts():
    r = GetRenderingAttributes()
    r.specularFlag = 1
    SetRenderingAttributes(r)

def setAnnotationAtts():
    a = GetAnnotationAttributes()
    a.legendInfoFlag = 0
    a.databaseInfoFlag = 0
    a.userInfoFlag = 0
    a.backgroundColor = (255,255,255,255)
    a.foregroundColor = (0,0,0,255)
    a.backgroundMode = a.Solid
    a.axes3D.visible = 0
    a.axes3D.triadFlag = 0
    a.axes3D.bboxFlag = 0
    SetAnnotationAttributes(a)

def runcase(host, datadir, casename):
    threeSlice = 0
    lights = 0

    db = os.path.join(datadir, casename, "mandelbulb.bov")

    OpenDatabase(db)

    mem = MemoryLogger(host, casename)
    mem.sample()

    AddPlot("Pseudocolor", "mandelbulb")
    pc = PseudocolorAttributes(1)
    pc.colorTableName = "RdYlBu"
    SetPlotOptions(pc)

    AddOperator("Isosurface")
    iso = IsosurfaceAttributes()
    iso.contourValue = (0.9,)
    iso.contourMethod = iso.Value
    iso.variable = "mandelbulb"
    SetOperatorOptions(iso)

    AddOperator("Transform")
    trans = TransformAttributes()
    trans.transformType = trans.Coordinate
    trans.inputCoordSys = trans.Spherical
    trans.outputCoordSys = trans.Cartesian
    SetOperatorOptions(trans)

    setView()
    setAnnotationAtts()
    setLights()
    setRenderingAtts()

    DrawPlots()

    mem.sample()

    saveImage(casename)

    mem.sample()

    if threeSlice:
        AddPlot("Pseudocolor", "mandelbulb")
        pc = PseudocolorAttributes(1)
        pc.colorTableName = "RdYlBu"
        SetPlotOptions(pc)

        AddOperator("Transform")
        trans = TransformAttributes()
        trans.transformType = trans.Coordinate
        trans.inputCoordSys = trans.Spherical
        trans.outputCoordSys = trans.Cartesian
        SetOperatorOptions(trans)

        AddOperator("ThreeSlice")
        DrawPlots()

        mem.sample()
        saveImage(casename)
        DeleteActivePlots()

    if lights: # Try out some other lights.
        DisableRedraw()
        setLights2()
        pc.colorTableName = "xray"
        SetPlotOptions(pc)
        RedrawWindow()
        saveImage(casename)
        mem.sample()

    DeleteAllPlots()
    CloseDatabase(db)

def main():
    HideToolbars()

    host = "vulcan.llnl.gov"
    datadir = host + ":/p/lscratchv/whitlocb/"
    #host = "localhost"
    #datadir = "/Users/bjw/Development/bgq_MS34/build/bin"
    cases = ("mandelbulb_1_16",
             "mandelbulb_16_256",
             "mandelbulb_32_512",
             "mandelbulb_64_1024",
             "mandelbulb_128_2048",
             "mandelbulb_256_4096",
             "mandelbulb_512_8192")

    #for c in cases:
    #    runcase(host, datadir, c)
    #runcase(host, datadir, "mandelbulb_1_16")
    #runcase(host, datadir, "mandelbulb_16_256")
    #runcase(host, datadir, "mandelbulb_32_512")
    #runcase(host, datadir, "mandelbulb_64_1024")
    #runcase(host, datadir, "mandelbulb_256_4096")
    #runcase(host, datadir, "mandelbulb_512_8192")

    #runcase(host, datadir, "mandelbulb_64_1024.100_cubed")
    #runcase(host, datadir, "mandelbulb_2048_32768.100_cubed")
    #runcase(host, datadir, "mandelbulb_4096_65536.100_cubed")
    #runcase(host, datadir, "mandelbulb_6144_98304.100_cubed")

main()
