# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  uintah.py
#
#  Programmer: Eric Brugger
#  Date:       Tue Jan  4 13:05:47 PST 2022
#
#  Modifications:
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("Uintah")

def test_particle():
    TestSection("Particle data")
    OpenDatabase(pjoin(data_path("uintah_test_data"), "index.xml"))
    AddPlot("Pseudocolor", "p.particleID/*")
    pc = PseudocolorAttributes()
    pc.pointType = pc.Point
    pc.pointSizePixels = 40
    SetPlotOptions(pc)
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (0, -1, 0)
    v.focus = (0.01, 0.01, 0.015)
    v.viewUp = (0, 0, 1)
    v.viewAngle = 30
    v.parallelScale = 0.0206155
    v.nearPlane = -0.0412311
    v.farPlane = 0.0412311
    v.imagePan = (0, 0)
    v.imageZoom = 1
    v.perspective = 0
    SetView3D(v)
    Test("Particle_00")

    ChangeActivePlotsVar("p.particleID/0")
    Test("Particle_01")

    ChangeActivePlotsVar("p.particleID/1")
    Test("Particle_02")

    ChangeActivePlotsVar("p.mass/*")
    Test("Particle_03")

    ChangeActivePlotsVar("p.mass/0")
    Test("Particle_04")

    ChangeActivePlotsVar("p.mass/1")
    Test("Particle_05")

    PickAtts = GetPickAttributes()
    PickAtts.variables = ("p.particleID/*")
    SetPickAttributes(PickAtts)
    SetQueryOutputToObject()
    p = NodePick(0, 0)
    id = p['p.particleID/*']
    TestValueEQ("Particle ID", id, 281474976710656., 0)

def main():
    test_particle()

main()
Exit()

