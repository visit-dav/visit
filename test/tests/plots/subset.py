# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  subset.py
#
#  Tests:      meshes    - 2D curvilinear, multiple domain
#              plots     - subset 
#
#  Defect ID:  '6105, '6762
#
#  Programmer: Hank Childs
#  Date:       March 31, 2005
#
#  Modifications:
#
#    Hank Childs, Tue Nov 15 07:28:43 PST 2005
#    Added test for subset plus macro expression.
#
# ----------------------------------------------------------------------------

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'

OpenDatabase("../data/multi_curv2d.silo")

AddPlot("Subset", "domains")
DrawPlots()

v = GetView2D()
v.viewportCoords = (0, 1, 0, 1)
v.windowCoords = (-5, 5, 0, 5)
SetView2D(v)

Test("subset_01")

s = SubsetAttributes()
s.wireframe = 1
SetPlotOptions(s)
Test("subset_02")

i = IsovolumeAttributes()
i.variable = "d"
i.lbound = 2.7
i.ubound = 3.6
SetDefaultOperatorOptions(i)
AddOperator("Isovolume")
Test("subset_03")

s.wireframe = 0
SetPlotOptions(s)
Test("subset_04")

RemoveLastOperator()
op = OnionPeelAttributes()
op.categoryName = "domains"
op.subsetName = "domain3"
op.index = (100)
SetDefaultOperatorOptions(op)
AddOperator("OnionPeel")

v.windowCoords = (-0.7, -0.3, 3.1, 3.25)
SetView2D(v)

Test("subset_05")

# This is a different wireframe path than the other tests, because
# we have special handling for single cell wireframes.
# This corresponds to ticket '6105.
s.wireframe = 1
SetPlotOptions(s)
Test("subset_06")

DeleteAllPlots()
OpenDatabase("../data/boxlib_test_data/2D/plt0822/Header")
AddPlot("Subset", "levels")
AddOperator("Isovolume")
DefineScalarExpression("vort", "curl(vel)")
iso_atts = IsovolumeAttributes()
iso_atts.variable = "vort"
iso_atts.lbound = -10000000
iso_atts.ubound = 0
SetOperatorOptions(iso_atts)
DrawPlots()
ResetView()
Test("subset_07")

Exit()
