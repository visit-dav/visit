# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  cart_proj.py
#
#  Tests:
#              plots     - pc
#              operators - CartographicProjection
#
#
#  Programmer: Cyrus Harrison
#  Date:      Tue Sep 25 08:41:19 PDT 2012
#
#  Modifications:
#
# ----------------------------------------------------------------------------


# Test 2D Case

OpenDatabase(data_path("netcdf_test_data/NASA_vegetation_lai.cdf"))

v = View2DAttributes()
v.fullFrameActivationMode = v.Off  # On, Off, Auto
SetView2D(v)


AddPlot("Pseudocolor", "lai")
DrawPlots()
ResetView()
Test("ops_cart_proj_0")

AddOperator("CartographicProjection")
cart_proj_atts = CartographicProjectionAttributes()
cart_proj_atts.projectionID = cart_proj_atts.hammer
SetOperatorOptions(cart_proj_atts)
DrawPlots()
ResetView()
Test("ops_cart_proj_1")

DeleteAllPlots()

OpenDatabase(data_path("netcdf_test_data/tas_mean_T63.nc"))

AddPlot("Pseudocolor", "tas")
DrawPlots()
ResetView()
Test("ops_cart_proj_2")

AddOperator("CartographicProjection")
cart_proj_atts = CartographicProjectionAttributes()
cart_proj_atts.projectionID = cart_proj_atts.hammer
SetOperatorOptions(cart_proj_atts)
DrawPlots()
ResetView()
Test("ops_cart_proj_3")


Exit()
