# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Mark C. Miller, Fri Aug 26 14:31:48 PDT 2022
# ----------------------------------------------------------------------------

#
# Get some utils shared across many .py files used for quick recipes.
#
Source(tests_path('quickrecipes','vqr_utils.py'))

def adding_operators():
 
  OpenDatabase(silo_data_path("globe.silo"))
  try:
    # adding operators {
    # Names of all available operator plugins as a python tuple
    x = OperatorPlugins()

    # print(x) will produce something like...
    # ('AMRStitchCell', 'AxisAlignedSlice4D', 'BoundaryOp', 'Box', 'CartographicProjection', 'Clip',
    #  'Cone', 'ConnectedComponents', 'CoordSwap', 'CreateBonds', 'Cylinder', 'DataBinning', 'DeferExpression',
    #  'Displace', 'DualMesh', 'Edge', 'Elevate', 'EllipsoidSlice', 'Explode', 'ExternalSurface',
    #  'Extrude', 'FFT', 'Flux', 'IndexSelect', 'IntegralCurve', 'InverseGhostZone', 'Isosurface',
    #  'Isovolume', 'LCS', 'Lagrangian', 'LimitCycle', 'Lineout', 'ModelFit', 'MultiresControl', 'OnionPeel',
    #  'PersistentParticles', 'Poincare', 'Project', 'RadialResample', 'Reflect', 'Remap', 'Replicate',
    #  'Resample', 'Revolve', 'SPHResample', 'Slice', 'Smooth', 'SphereSlice', 'StatisticalTrends',
    #  'SubdivideQuads', 'SurfaceNormal', 'Tessellate', 'ThreeSlice', 'Threshold', 'Transform',
    #  'TriangulateRegularPoints', 'Tube')
    print(x)

    # We need at least one plot that we can add operators to
    AddPlot("Pseudocolor", "dx")
    AddPlot("Mesh","mesh1")

    # Add Isovolume and Slice operators using whatever their default attributes are.
    # The non-zero 2nd arg means to add the operator to all plots. A value of 0 or
    # if the 2nd argument is not present means to add the operator only to the
    # *active* plots (usually just the last plot added).
    AddOperator("Isovolume", 1)
    AddOperator("Slice", 1)
    DrawPlots()
    # adding operators }
    TestValueEQ('adding operators error message',GetLastError(),'')
    TestPOA('adding operators exceptions')
  except:
    TestFOA('adding operators exception', LINE())
    pass
  vqr_cleanup()
  CloseDatabase(silo_data_path("globe.silo"))
 
def setting_operator_attributes():
 
  try:
    # setting operator attributes {

OpenDatabase("/usr/local/visit/data/noise.silo")
AddPlot("Pseudocolor", "hardyglobal")
AddOperator("Slice")
s = SliceAttributes()
s.originType = s.Percent
s.project2d = 0
SetOperatorOptions(s)
DrawPlots()

nSteps = 20
for axis in (0,1,2):
  s.axisType = axis
  for step in range(nSteps):
    t = float(step) / float(nSteps - 1)
    s.originPercent = t * 100.
    SetOperatorOptions(s)
    SaveWindow()


    # setting operator attributes }
    TestValueEQ('setting operator attributes error message',GetLastError(),'')
    TestPOA('setting operator attributes exceptions')
  except:
    TestFOA('setting operator attributes exception', LINE())
    pass
  vqr_cleanup()
  CloseDatabase(silo_data_path("globe.silo"))
 
adding_operators()
setting_operator_attributes()

Exit()
