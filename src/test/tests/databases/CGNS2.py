

datapath = data_path("CGNS_test_data")

# Draw antialiased lines
r = GetRenderingAttributes()
r.antialiasing = 1
SetRenderingAttributes(r)


OpenDatabase(pjoin(datapath,"delta.cgns"))
AddPlot("Mesh", "Zone___1")
DrawPlots()
v0 = View3DAttributes()
v0.viewNormal = (-0.673597, -0.70478, 0.222605)
v0.focus = (0.5, 0.827196, 0.0589651)
v0.viewUp = (0.150257, 0.164313, 0.974897)
v0.viewAngle = 30
v0.parallelScale = 2.30431
v0.nearPlane = -4.60862
v0.farPlane = 4.60862
v0.imagePan = (0.0171482, 0.0494526)
v0.imageZoom = 1.09899
v0.perspective = 1
v0.eyeAngle = 2
v0.centerOfRotationSet = 0
v0.centerOfRotation = (0.5, 0.827196, 0.0589651)
SetView3D(v0)

Exit()
