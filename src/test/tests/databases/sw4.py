# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  SW4.py
#
#  Tests:      mesh      - 3D curvilinear, multi domain
#                        - 3D rectilinear, single domain
#              plots     - Pseudocolor
#
#  Programmer: Eric Brugger
#  Date:       Tue Jul 21 11:15:20 PDT 2020
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create a plot from a 3Dimg file.
OpenDatabase(data_path("sw4_test_data/berk.cycle=0000.p.3Dimg"))
AddPlot("Pseudocolor", "cp")
DrawPlots()

# Set the view
v = View3DAttributes()
v.viewNormal = (-0.555957, 0.661021, -0.503948)
v.focus = (6000, 6000, 2220.93)
v.viewUp = (0.540018, -0.173664, -0.823542)
v.viewAngle = 30
v.parallelScale = 8928.79
v.nearPlane = -17857.6
v.farPlane = 17857.6
v.imagePan = (0., 0.)
v.imageZoom = 1
v.perspective = 1
v.eyeAngle = 2
SetView3D(v)

Test("sw4_00")

# Create a plot from a sw4img file.
DeleteAllPlots()
CloseDatabase(data_path("sw4_test_data/berk.cycle=0000.p.3Dimg"))

OpenDatabase(data_path("sw4_test_data/berk.cycle=0000.z=0.topo.sw4img"))
AddPlot("Pseudocolor", "topo")
DrawPlots()

Test("sw4_01")

Exit()
