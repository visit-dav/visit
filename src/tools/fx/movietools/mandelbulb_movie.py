import os

# This function rotates about an axis. For simplicity, we just select a 
# coordinate axis. Of course, you could define viewNormal, viewUp, viewFocus
# to define a different axis.
def RotateAxis(axis, angle):
    import math
 
    v1 = [0.,0.,0.]
    v2 = [0.,0.,0.]
    v3 = [0.,0.,0.]
    t1 = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
    t2 = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
    m1 = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
    m2 = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
    r = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
    ma = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
    mb = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
    mc = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
    rM = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
    viewNormal = [0.,0.,0.]
    viewUp = [0.,0.,0.]
    viewFocus = [0.,0.,0.]
    view3d = GetView3D()
 
    #
    # Calculate the rotation matrix in screen coordinates.
    #
    angleRadians = math.radians(angle)
    if axis == 0:
        r[0]  = 1.
        r[1]  = 0.
        r[2]  = 0.
        r[3]  = 0.
        r[4]  = 0.
        r[5]  = math.cos(angleRadians)
        r[6]  = - math.sin(angleRadians)
        r[7]  = 0.
        r[8]  = 0.
        r[9]  = math.sin(angleRadians)
        r[10] = math.cos(angleRadians)
        r[11] = 0.
        r[12] = 0.
        r[13] = 0.
        r[14] = 0.
        r[15] = 1.
    elif axis == 1:
        r[0]  = math.cos(angleRadians)
        r[1]  = 0.
        r[2]  = math.sin(angleRadians)
        r[3]  = 0.
        r[4]  = 0.
        r[5]  = 1.
        r[6]  = 0.
        r[7]  = 0.
        r[8]  = - math.sin(angleRadians)
        r[9]  = 0.
        r[10]  = math.cos(angleRadians)
        r[11] = 0.
        r[12] = 0.
        r[13] = 0.
        r[14] = 0.
        r[15] = 1.
    elif axis == 2:
        r[0]  = math.cos(angleRadians)
        r[1]  = - math.sin(angleRadians)
        r[2]  = 0.
        r[3]  = 0.
        r[4]  = math.sin(angleRadians)
        r[5]  = math.cos(angleRadians)
        r[6]  = 0.
        r[7]  = 0.
        r[8]  = 0.
        r[9]  = 0.
        r[10]  = 1.
        r[11] = 0.
        r[12] = 0.
        r[13] = 0.
        r[14] = 0.
        r[15] = 1.
 
    #
    # Calculate the matrix to rotate from object coordinates to screen
    # coordinates and its inverse.
    #
    v1[0] = view3d.GetViewNormal()[0]
    v1[1] = view3d.GetViewNormal()[1]
    v1[2] = view3d.GetViewNormal()[2]
 
    v2[0] = view3d.GetViewUp()[0]
    v2[1] = view3d.GetViewUp()[1]
    v2[2] = view3d.GetViewUp()[2]
 
    v3[0] =   v2[1]*v1[2] - v2[2]*v1[1]
    v3[1] = - v2[0]*v1[2] + v2[2]*v1[0]
    v3[2] =   v2[0]*v1[1] - v2[1]*v1[0]
 
    m1[0]  = v3[0]
    m1[1]  = v2[0]
    m1[2]  = v1[0]
    m1[3]  = 0.
    m1[4]  = v3[1]
    m1[5]  = v2[1]
    m1[6]  = v1[1]
    m1[7]  = 0.
    m1[8]  = v3[2]
    m1[9]  = v2[2]
    m1[10] = v1[2]
    m1[11] = 0.
    m1[12] = 0.
    m1[13] = 0.
    m1[14] = 0.
    m1[15] = 1.
 
    m2[0]  = m1[0]
    m2[1]  = m1[4]
    m2[2]  = m1[8]
    m2[3]  = m1[12]
    m2[4]  = m1[1]
    m2[5]  = m1[5]
    m2[6]  = m1[9]
    m2[7]  = m1[13]
    m2[8]  = m1[2]
    m2[9]  = m1[6]
    m2[10] = m1[10]
    m2[11] = m1[14]
    m2[12] = m1[3]
    m2[13] = m1[7]
    m2[14] = m1[11]
    m2[15] = m1[15]
 
    #
    # Calculate the translation to the center of rotation (and its
    # inverse).
    #
    t1[0]  = 1.
    t1[1]  = 0.
    t1[2]  = 0.
    t1[3]  = 0.
    t1[4]  = 0.
    t1[5]  = 1.
    t1[6]  = 0.
    t1[7]  = 0.
    t1[8]  = 0.
    t1[9]  = 0.
    t1[10] = 1.
    t1[11] = 0.
    t1[12] = -view3d.GetCenterOfRotation()[0]
    t1[13] = -view3d.GetCenterOfRotation()[1]
    t1[14] = -view3d.GetCenterOfRotation()[2]
    t1[15] = 1.
 
    t2[0]  = 1.
    t2[1]  = 0.
    t2[2]  = 0.
    t2[3]  = 0.
    t2[4]  = 0.
    t2[5]  = 1.
    t2[6]  = 0.
    t2[7]  = 0.
    t2[8]  = 0.
    t2[9]  = 0.
    t2[10] = 1.
    t2[11] = 0.
    t2[12] = view3d.GetCenterOfRotation()[0]
    t2[13] = view3d.GetCenterOfRotation()[1]
    t2[14] = view3d.GetCenterOfRotation()[2]
    t2[15] = 1.
 
    #
    # Form the composite transformation matrix t1 X m1 X r X m2 X t2.
    #
    ma[0]  = t1[0]*m1[0]  + t1[1]*m1[4]  + t1[2]*m1[8]   + t1[3]*m1[12]
    ma[1]  = t1[0]*m1[1]  + t1[1]*m1[5]  + t1[2]*m1[9]   + t1[3]*m1[13]
    ma[2]  = t1[0]*m1[2]  + t1[1]*m1[6]  + t1[2]*m1[10]  + t1[3]*m1[14]
    ma[3]  = t1[0]*m1[3]  + t1[1]*m1[7]  + t1[2]*m1[11]  + t1[3]*m1[15]
    ma[4]  = t1[4]*m1[0]  + t1[5]*m1[4]  + t1[6]*m1[8]   + t1[7]*m1[12]
    ma[5]  = t1[4]*m1[1]  + t1[5]*m1[5]  + t1[6]*m1[9]   + t1[7]*m1[13]
    ma[6]  = t1[4]*m1[2]  + t1[5]*m1[6]  + t1[6]*m1[10]  + t1[7]*m1[14]
    ma[7]  = t1[4]*m1[3]  + t1[5]*m1[7]  + t1[6]*m1[11]  + t1[7]*m1[15]
    ma[8]  = t1[8]*m1[0]  + t1[9]*m1[4]  + t1[10]*m1[8]  + t1[11]*m1[12]
    ma[9]  = t1[8]*m1[1]  + t1[9]*m1[5]  + t1[10]*m1[9]  + t1[11]*m1[13]
    ma[10] = t1[8]*m1[2]  + t1[9]*m1[6]  + t1[10]*m1[10] + t1[11]*m1[14]
    ma[11] = t1[8]*m1[3]  + t1[9]*m1[7]  + t1[10]*m1[11] + t1[11]*m1[15]
    ma[12] = t1[12]*m1[0] + t1[13]*m1[4] + t1[14]*m1[8]  + t1[15]*m1[12]
    ma[13] = t1[12]*m1[1] + t1[13]*m1[5] + t1[14]*m1[9]  + t1[15]*m1[13]
    ma[14] = t1[12]*m1[2] + t1[13]*m1[6] + t1[14]*m1[10] + t1[15]*m1[14]
    ma[15] = t1[12]*m1[3] + t1[13]*m1[7] + t1[14]*m1[11] + t1[15]*m1[15]
 
    mb[0]  = ma[0]*r[0]  + ma[1]*r[4]  + ma[2]*r[8]   + ma[3]*r[12]
    mb[1]  = ma[0]*r[1]  + ma[1]*r[5]  + ma[2]*r[9]   + ma[3]*r[13]
    mb[2]  = ma[0]*r[2]  + ma[1]*r[6]  + ma[2]*r[10]  + ma[3]*r[14]
    mb[3]  = ma[0]*r[3]  + ma[1]*r[7]  + ma[2]*r[11]  + ma[3]*r[15]
    mb[4]  = ma[4]*r[0]  + ma[5]*r[4]  + ma[6]*r[8]   + ma[7]*r[12]
    mb[5]  = ma[4]*r[1]  + ma[5]*r[5]  + ma[6]*r[9]   + ma[7]*r[13]
    mb[6]  = ma[4]*r[2]  + ma[5]*r[6]  + ma[6]*r[10]  + ma[7]*r[14]
    mb[7]  = ma[4]*r[3]  + ma[5]*r[7]  + ma[6]*r[11]  + ma[7]*r[15]
    mb[8]  = ma[8]*r[0]  + ma[9]*r[4]  + ma[10]*r[8]  + ma[11]*r[12]
    mb[9]  = ma[8]*r[1]  + ma[9]*r[5]  + ma[10]*r[9]  + ma[11]*r[13]
    mb[10] = ma[8]*r[2]  + ma[9]*r[6]  + ma[10]*r[10] + ma[11]*r[14]
    mb[11] = ma[8]*r[3]  + ma[9]*r[7]  + ma[10]*r[11] + ma[11]*r[15]
    mb[12] = ma[12]*r[0] + ma[13]*r[4] + ma[14]*r[8]  + ma[15]*r[12]
    mb[13] = ma[12]*r[1] + ma[13]*r[5] + ma[14]*r[9]  + ma[15]*r[13]
    mb[14] = ma[12]*r[2] + ma[13]*r[6] + ma[14]*r[10] + ma[15]*r[14]
    mb[15] = ma[12]*r[3] + ma[13]*r[7] + ma[14]*r[11] + ma[15]*r[15]
 
    mc[0]  = mb[0]*m2[0]  + mb[1]*m2[4]  + mb[2]*m2[8]   + mb[3]*m2[12]
    mc[1]  = mb[0]*m2[1]  + mb[1]*m2[5]  + mb[2]*m2[9]   + mb[3]*m2[13]
    mc[2]  = mb[0]*m2[2]  + mb[1]*m2[6]  + mb[2]*m2[10]  + mb[3]*m2[14]
    mc[3]  = mb[0]*m2[3]  + mb[1]*m2[7]  + mb[2]*m2[11]  + mb[3]*m2[15]
    mc[4]  = mb[4]*m2[0]  + mb[5]*m2[4]  + mb[6]*m2[8]   + mb[7]*m2[12]
    mc[5]  = mb[4]*m2[1]  + mb[5]*m2[5]  + mb[6]*m2[9]   + mb[7]*m2[13]
    mc[6]  = mb[4]*m2[2]  + mb[5]*m2[6]  + mb[6]*m2[10]  + mb[7]*m2[14]
    mc[7]  = mb[4]*m2[3]  + mb[5]*m2[7]  + mb[6]*m2[11]  + mb[7]*m2[15]
    mc[8]  = mb[8]*m2[0]  + mb[9]*m2[4]  + mb[10]*m2[8]  + mb[11]*m2[12]
    mc[9]  = mb[8]*m2[1]  + mb[9]*m2[5]  + mb[10]*m2[9]  + mb[11]*m2[13]
    mc[10] = mb[8]*m2[2]  + mb[9]*m2[6]  + mb[10]*m2[10] + mb[11]*m2[14]
    mc[11] = mb[8]*m2[3]  + mb[9]*m2[7]  + mb[10]*m2[11] + mb[11]*m2[15]
    mc[12] = mb[12]*m2[0] + mb[13]*m2[4] + mb[14]*m2[8]  + mb[15]*m2[12]
    mc[13] = mb[12]*m2[1] + mb[13]*m2[5] + mb[14]*m2[9]  + mb[15]*m2[13]
    mc[14] = mb[12]*m2[2] + mb[13]*m2[6] + mb[14]*m2[10] + mb[15]*m2[14]
    mc[15] = mb[12]*m2[3] + mb[13]*m2[7] + mb[14]*m2[11] + mb[15]*m2[15]
 
    rM[0]  = mc[0]*t2[0]  + mc[1]*t2[4]  + mc[2]*t2[8]   + mc[3]*t2[12]
    rM[1]  = mc[0]*t2[1]  + mc[1]*t2[5]  + mc[2]*t2[9]   + mc[3]*t2[13]
    rM[2]  = mc[0]*t2[2]  + mc[1]*t2[6]  + mc[2]*t2[10]  + mc[3]*t2[14]
    rM[3]  = mc[0]*t2[3]  + mc[1]*t2[7]  + mc[2]*t2[11]  + mc[3]*t2[15]
    rM[4]  = mc[4]*t2[0]  + mc[5]*t2[4]  + mc[6]*t2[8]   + mc[7]*t2[12]
    rM[5]  = mc[4]*t2[1]  + mc[5]*t2[5]  + mc[6]*t2[9]   + mc[7]*t2[13]
    rM[6]  = mc[4]*t2[2]  + mc[5]*t2[6]  + mc[6]*t2[10]  + mc[7]*t2[14]
    rM[7]  = mc[4]*t2[3]  + mc[5]*t2[7]  + mc[6]*t2[11]  + mc[7]*t2[15]
    rM[8]  = mc[8]*t2[0]  + mc[9]*t2[4]  + mc[10]*t2[8]  + mc[11]*t2[12]
    rM[9]  = mc[8]*t2[1]  + mc[9]*t2[5]  + mc[10]*t2[9]  + mc[11]*t2[13]
    rM[10] = mc[8]*t2[2]  + mc[9]*t2[6]  + mc[10]*t2[10] + mc[11]*t2[14]
    rM[11] = mc[8]*t2[3]  + mc[9]*t2[7]  + mc[10]*t2[11] + mc[11]*t2[15]
    rM[12] = mc[12]*t2[0] + mc[13]*t2[4] + mc[14]*t2[8]  + mc[15]*t2[12]
    rM[13] = mc[12]*t2[1] + mc[13]*t2[5] + mc[14]*t2[9]  + mc[15]*t2[13]
    rM[14] = mc[12]*t2[2] + mc[13]*t2[6] + mc[14]*t2[10] + mc[15]*t2[14]
    rM[15] = mc[12]*t2[3] + mc[13]*t2[7] + mc[14]*t2[11] + mc[15]*t2[15]
 
    #
    # Calculate the new view normal and view up.
    #
    viewNormal[0] = view3d.GetViewNormal()[0] * rM[0] +\
                    view3d.GetViewNormal()[1] * rM[4] +\
                    view3d.GetViewNormal()[2] * rM[8]
    viewNormal[1] = view3d.GetViewNormal()[0] * rM[1] +\
                    view3d.GetViewNormal()[1] * rM[5] +\
                    view3d.GetViewNormal()[2] * rM[9]
    viewNormal[2] = view3d.GetViewNormal()[0] * rM[2] +\
                    view3d.GetViewNormal()[1] * rM[6] +\
                    view3d.GetViewNormal()[2] * rM[10]
    dist = math.sqrt(viewNormal[0]*viewNormal[0] +\
                     viewNormal[1]*viewNormal[1] +\
                     viewNormal[2]*viewNormal[2])
    viewNormal[0] /= dist
    viewNormal[1] /= dist
    viewNormal[2] /= dist 
    view3d.viewNormal = tuple(viewNormal)
 
    viewUp[0] = view3d.GetViewUp()[0] * rM[0] +\
                view3d.GetViewUp()[1] * rM[4] +\
                view3d.GetViewUp()[2] * rM[8]
    viewUp[1] = view3d.GetViewUp()[0] * rM[1] +\
                view3d.GetViewUp()[1] * rM[5] +\
                view3d.GetViewUp()[2] * rM[9]
    viewUp[2] = view3d.GetViewUp()[0] * rM[2] +\
                view3d.GetViewUp()[1] * rM[6] +\
                view3d.GetViewUp()[2] * rM[10]
    dist = math.sqrt(viewUp[0]*viewUp[0] + viewUp[1]*viewUp[1] +\
                viewUp[2]*viewUp[2])
    viewUp[0] /= dist
    viewUp[1] /= dist
    viewUp[2] /= dist 
    view3d.viewUp = tuple(viewUp)
 
    if (view3d.GetCenterOfRotationSet()):
        viewFocus[0] = view3d.GetFocus()[0] * rM[0]  +\
                       view3d.GetFocus()[1] * rM[4]  +\
                       view3d.GetFocus()[2] * rM[8]  +\
                       rM[12]
        viewFocus[1] = view3d.GetFocus()[0] * rM[1]  +\
                       view3d.GetFocus()[1] * rM[5]  +\
                       view3d.GetFocus()[2] * rM[9]  +\
                       rM[13]
        viewFocus[2] = view3d.GetFocus()[0] * rM[2]  +\
                       view3d.GetFocus()[1] * rM[6]  +\
                       view3d.GetFocus()[2] * rM[10] +\
                       rM[14]
        view3d.focus = tuple(viewFocus)
 
    SetView3D(view3d)

def saveImage(casename, index):
    DrawPlots()

    s = SaveWindowAttributes()
    s.fileName = casename + "%04d" % index
    s.family = 0
    s.format = s.JPEG
    s.width = 1920 #1024
    s.height = 1080 #768 
    s.screenCapture = 0
    s.quality = 100
    s.resConstraint = s.NoConstraint
    SetSaveWindowAttributes(s)
    SaveWindow()

def setView():
    v = GetView3D()
    v.viewNormal = (0.326913, 0.533941, 0.779766)
    v.focus = (0, 0, 0)
    v.viewUp = (-0.206157, 0.845521, -0.492537)
    v.viewAngle = 30
    v.parallelScale = 2.07846
    v.nearPlane = -4.15692
    v.farPlane = 4.15692
    v.imagePan = (0.00815442, 0.00850481)
    v.imageZoom = 2.06902
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)


def setLights():
    L = GetLight(0)
    L.enabledFlag = 1
    L.type = L.Camera  # Ambient, Object, Camera
    L.direction = (-0.311, -0.137, -0.941)
    L.color = (255, 255, 255, 255)
    L.brightness = 1
    SetLight(0, L)

def setRenderingAtts():
    r = GetRenderingAttributes()
    r.specularFlag = 1
    r.specularCoeff = 0.24
    r.doShadowing = 1
    r.shadowStrength = 0.5
    SetRenderingAttributes(r)

def setAnnotationAtts():
    a = GetAnnotationAttributes()
    a.legendInfoFlag = 1
    a.databaseInfoFlag = 0
    a.userInfoFlag = 0
    a.foregroundColor = (255,255,255,255)
    a.backgroundColor = (0,0,0,255)
    a.backgroundMode = a.Gradient
    a.gradientBackgroundStyle = a.TopToBottom  # TopToBottom, BottomToTop, LeftToRight, RightToLeft, Radial
    a.gradientColor1 = (25, 0, 59, 255)
    a.gradientColor2 = (0, 0, 0, 255)
    a.axes3D.visible = 0
    a.axes3D.triadFlag = 1
    a.axes3D.bboxFlag = 0
    SetAnnotationAttributes(a)

def setup(db, imgname):
    # Add the first plot.
    AddPlot("Pseudocolor", "mandelbulb")
    pc = PseudocolorAttributes(1)
    pc.colorTableName = "Spectral"
    pc.minFlag = 1
    pc.maxFlag = 1
    pc.min = 0
    pc.max = 20
    SetPlotOptions(pc)

    AddOperator("Isosurface")
    iso = IsosurfaceAttributes()
    iso.contourValue = (1.1,)
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

    # Add the slice plot.
    AddPlot("Pseudocolor", "mandelbulb")
    pc.colorTableName = "Spectral"
    pc.legendFlag = 0
    SetPlotOptions(pc)

    AddOperator("Transform")
    trans = TransformAttributes()
    trans.transformType = trans.Coordinate
    trans.inputCoordSys = trans.Spherical
    trans.outputCoordSys = trans.Cartesian
    SetOperatorOptions(trans)

    startPercent = 10
    endPercent = 90
    AddOperator("Slice")
    s = SliceAttributes()
    s.originType = s.Percent
    s.originPercent = startPercent
    s.axisType = s.XAxis
    s.project2d = 0
    SetOperatorOptions(s)

    setView()
    setAnnotationAtts()
    setLights()
    setRenderingAtts()

    DrawPlots()

def slices(imgname, nsteps, index):
    repeat = 0

    startPercent = 10
    endPercent = 90

    iso = IsosurfaceAttributes()
    iso.contourValue = (1.1,)
    iso.contourMethod = iso.Value
    iso.variable = "mandelbulb"

    # X slices
    s = SliceAttributes(1)
    s.originType = s.Percent
    s.originPercent = startPercent
    s.axisType = s.XAxis
    s.project2d = 0
    s.axisType = s.XAxis
    for i in range(nsteps):
        t = float(i) / float(nsteps - 1)
        pct = (1.-t)*startPercent + t*endPercent
        s.originPercent = pct
        SetOperatorOptions(s)
        setView()
        saveImage(imgname, index)
        index = index + 1
    # Repeat.
    if repeat:
        for i in range(10):
            saveImage(imgname, index)
            index = index + 1

    # Y slices
    s.axisType = s.YAxis
    for i in range(nsteps):
        t = float(i) / float(nsteps - 1)
        pct = (1.-t)*startPercent + t*endPercent
        s.originPercent = pct
        SetOperatorOptions(s)
        setView()
        saveImage(imgname, index)
        index = index + 1
    # Repeat.
    if repeat:
        for i in range(10):
            saveImage(imgname, index)
            index = index + 1

    # Z slices
    s.axisType = s.ZAxis
    for i in range(nsteps):
        t = float(i) / float(nsteps - 1)
        pct = (1.-t)*startPercent + t*endPercent
        s.originPercent = pct
        SetOperatorOptions(s)
        setView()
        saveImage(imgname, index)
        index = index + 1

    # Delete the slice plot
    DeleteActivePlots()

    # Repeat.
    if repeat:
        for i in range(10):
            saveImage(imgname, index)
            index = index + 1

    # ISO slices
    contourStart = 1.1
    contourEnd = 20
    for i in range(nsteps):
        t = float(i) / float(nsteps - 1)
        cont = (1.-t)*contourStart + t*contourEnd
        iso.contourValue = (cont,)
        SetOperatorOptions(iso)
        setView()
        saveImage(imgname, index)
        index = index + 1

    # Repeat.
    if repeat:
        for i in range(10):
            saveImage(imgname, index)
            index = index + 1

    # ISO slices (back to start)
    for i in range(nsteps):
        t = float(i) / float(nsteps - 1)
        cont = t*contourStart + (1.-t)*contourEnd
        iso.contourValue = (cont,)
        SetOperatorOptions(iso)
        setView()
        saveImage(imgname, index)
        index = index + 1

    return index

def rotations(imgname, nsteps, index):
    # Rotate about Y
    v = GetView3D()
    Nrotations = 1.
    for i in range(nsteps):
        # Restore to the initial view
        SetView3D(v)
        t = float(i) / float(nsteps-1)
        angle = t * 360.
        # Rotate to the new view
        RotateAxis(1, angle * Nrotations)
        saveImage(imgname, index)
        index = index + 1
    return index

def makeframes(imgname):
    vulcan = 1
    nsteps = 100
    imgname = "image"

    if vulcan:
        host = "vulcan.llnl.gov"
        db = host + ":" + "/p/lscratchv/whitlocb/mandelbulb_64_1024.100_cubed/mandelbulb.bov"

        OpenComputeEngine(host, ("-nn", "64", "-np", "1024", "-l", "srun", "-p", "psmall", "-t", "480", "-b", "sspwork", "-dir", "/g/g17/whitlocb/install", "-fixed-buffer-sockets"))
    else:
        db = "/Users/bjw/Development/MAIN/trunk/build/data/mandelbulb.bov"
        OpenComputeEngine("localhost", ("-np", "8"))
    OpenDatabase(db)

    index = 0
    setup(imgname, nsteps)
    index = slices(imgname, nsteps, index)
    index = rotations(imgname, nsteps, index)

    DeleteAllPlots()
    CloseDatabase(db)

def encode(imgname):
    from visit_utils import encoding
    encoding.encode(imgname + "%04d.jpeg", "output.mpg")

def main():
    imgname = "image"
    makeframes(imgname)
    encode(imgname)

main()
