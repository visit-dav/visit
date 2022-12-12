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
 
setting_the_2d_view()
setting_the_3d_view()

Exit()
