# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Mark C. Miller, Fri Aug 26 14:31:48 PDT 2022
# ----------------------------------------------------------------------------

#
# Get some utils shared across many .py files used for quick recipes.
#
Source(tests_path('quickrecipes','vqr_utils.py'))

def setting_the_2d_view():
 
  try:
    # setting the 2d view {
    OpenDatabase("~juanita/silo/stuff/noise.silo")
    AddPlot("Pseudocolor", "hgslice")
    AddPlot("Mesh", "Mesh2D")
    AddPlot("Label", "hgslice")
    DrawPlots()
    print("The current view is:", GetView2D())
    # Get an initialized 2D view object.
    # Note that DrawPlots() must be executed prior to getting
    # the view to ensure current view parameters are obtained
    v = GetView2D()
    v.windowCoords = (-7.67964, -3.21856, 2.66766, 7.87724)
    SetView2D(v)
    # setting the 2d view }
    TestValueEQ('setting the 2d view error message',GetLastError(),'')
    TestPOA('setting the 2d view exceptions')
  except Exception as inst:
    TestFOA('setting the 2d view exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

def setting_the_3d_view():
 
  try:
    # setting the 3d view {
    OpenDatabase("~juanita/silo/stuff/noise.silo")
    AddPlot("Pseudocolor", "hardyglobal")
    AddPlot("Mesh", "Mesh")
    DrawPlots()
    # Note that DrawPlots() must be executed prior to getting
    # the view to ensure current view parameters are obtained
    v = GetView3D()
    print("The view is: ", v)
    v.viewNormal = (-0.571619, 0.405393, 0.713378)
    v.viewUp = (0.308049, 0.911853, -0.271346)
    SetView3D(v)
    # setting the 3d view }
    TestValueEQ('setting the 3d view error message',GetLastError(),'')
    TestPOA('setting the 3d view exceptions')
  except Exception as inst:
    TestFOA('setting the 3d view exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

def flying_around_plots(): 
 
  try:
    # flying around plots {
    OpenDatabase("~juanita/silo/stuff/globe.silo")
    AddPlot("Pseudocolor", "u")
    DrawPlots()

    # Create the control points for the views.
    c0 = View3DAttributes()
    c0.viewNormal = (0, 0, 1)
    c0.focus = (0, 0, 0)
    c0.viewUp = (0, 1, 0)
    c0.viewAngle = 30
    c0.parallelScale = 17.3205
    c0.nearPlane = 17.3205
    c0.farPlane = 81.9615
    c0.perspective = 1

    c1 = View3DAttributes()
    c1.viewNormal = (-0.499159, 0.475135, 0.724629)
    c1.focus = (0, 0, 0)
    c1.viewUp = (0.196284, 0.876524, -0.439521)
    c1.viewAngle = 30
    c1.parallelScale = 14.0932
    c1.nearPlane = 15.276
    c1.farPlane = 69.917
    c1.perspective = 1

    c2 = View3DAttributes()
    c2.viewNormal = (-0.522881, 0.831168, -0.189092)
    c2.focus = (0, 0, 0)
    c2.viewUp = (0.783763, 0.556011, 0.27671)
    c2.viewAngle = 30
    c2.parallelScale = 11.3107
    c2.nearPlane = 14.8914
    c2.farPlane = 59.5324
    c2.perspective = 1

    c3 = View3DAttributes()
    c3.viewNormal = (-0.438771, 0.523661, -0.730246)
    c3.focus = (0, 0, 0)
    c3.viewUp = (-0.0199911, 0.80676, 0.590541)
    c3.viewAngle = 30
    c3.parallelScale = 8.28257
    c3.nearPlane = 3.5905
    c3.farPlane = 48.2315
    c3.perspective = 1

    c4 = View3DAttributes()
    c4.viewNormal = (0.286142, -0.342802, -0.894768)
    c4.focus = (0, 0, 0)
    c4.viewUp = (-0.0382056, 0.928989, -0.36813)
    c4.viewAngle = 30
    c4.parallelScale = 10.4152
    c4.nearPlane = 1.5495
    c4.farPlane = 56.1905
    c4.perspective = 1

    c5 = View3DAttributes()
    c5.viewNormal = (0.974296, -0.223599, -0.0274086)
    c5.focus = (0, 0, 0)
    c5.viewUp = (0.222245, 0.97394, -0.0452541)
    c5.viewAngle = 30
    c5.parallelScale = 1.1052
    c5.nearPlane = 24.1248
    c5.farPlane = 58.7658
    c5.perspective = 1

    # Make the last point loop around to the first
    c6 = c0

    # Create a tuple of camera values and x values. The x values
    # determine where in [0,1] the control points occur.
    cpts = (c0, c1, c2, c3, c4, c5, c6)
    x=[]
    for i in range(7):
        x = x + [float(i) / float(6.)]

    # Animate the view using EvalCubicSpline.
    nsteps = 100
    for i in range(nsteps):
        t = float(i) / float(nsteps - 1)
        c = EvalCubicSpline(t, x, cpts)
        c.nearPlane = -34.461
        c.farPlane = 34.461
        SetView3D(c)
    # flying around plots }
    TestValueEQ('flying around plots error message',GetLastError(),'')
    TestPOA('flying around plots exceptions')
  except Exception as inst:
    TestFOA('flying around plots exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()
 
setting_the_2d_view()
setting_the_3d_view()
flying_around_plots()

Exit()
