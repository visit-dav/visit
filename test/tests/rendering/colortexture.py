# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  colortexture.py
#
#  Tests:      mesh      - 3D UCD
#              plots     - Pseudocolor
#
#  Defect ID:  none
#
#  Programmer: Brad Whitlock
#  Date:       Mon Sep 18 13:44:14 PST 2006
#
#  Modificatons:
#
# ----------------------------------------------------------------------------

def main():
    a = AnnotationAttributes()
    TurnOffAllAnnotations(a)
    a.foregroundColor = (255, 255, 255, 255)
    a.backgroundColor = (0, 0, 0, 255)
    SetAnnotationAttributes(a)

    OpenDatabase("../data/ucd2d.silo")
    AddPlot("Pseudocolor", "d")
    AddPlot("Mesh", "ucdmesh2d")
    DrawPlots()
    Test("colortexture_00")
    DeleteAllPlots()

    OpenDatabase("../data/globe.silo")
    AddPlot("Pseudocolor", "speed")
    pc = PseudocolorAttributes()
    pc.colorTableName = "levels"
    SetPlotOptions(pc)
    DrawPlots()

    L = GetLight(0)
    L.enabledFlag = 1
    L.type = L.Camera  # Ambient, Object, Camera
    L.direction = (-0.131, -0.685, -0.716)
    L.color = (255, 255, 255, 255)
    L.brightness = 1
    SetLight(0, L)

    v0 = View3DAttributes()
    v0.viewNormal = (-0.615147, 0.425906, 0.663475)
    v0.focus = (0, 0, 0)
    v0.viewUp = (0.235983, 0.902416, -0.360497)
    v0.viewAngle = 30
    v0.parallelScale = 17.3205
    v0.nearPlane = -34.641
    v0.farPlane = 34.641
    v0.imagePan = (0, 0)
    v0.imageZoom = 1.56642
    v0.perspective = 1
    v0.eyeAngle = 2
    v0.centerOfRotationSet = 0
    v0.centerOfRotation = (0, 0, 0)
    SetView3D(v0)
    Test("colortexture_01")

    # Turn off color texturing
    SetColorTexturingEnabled(0)
    Test("colortexture_02")

    # Turn color texturing back on.
    SetColorTexturingEnabled(1)
    pc.colorTableName = "hot"
    SetPlotOptions(pc)
    Test("colortexture_03")

    # Make sure that it works with specular highlights.
    r = GetRenderingAttributes()
    r.specularFlag = 1.
    SetRenderingAttributes(r)
    Test("colortexture_04")

main()
Exit()

