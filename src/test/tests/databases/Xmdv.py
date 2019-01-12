# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  Xmdv.py
#
#  Tests:      mesh      - 3D point mesh
#
#  Defect ID:  none
#
#  Programmer: Hank Childs
#  Date:       July 21, 2005
#
#  Modifications:
#    Brad Whitlock, Wed Aug 10 10:00:45 PDT 2005
#    Fixed due to a change in the Label plot.
#
#    Hank Childs, Tue Mar 18 15:40:57 PDT 2008
#    Reflect change in variable names.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Hank Childs, Sat Mar 13 11:13:26 PST 2010
#    Change variable names for forbidden characters.
#
# ----------------------------------------------------------------------------


# First test that we can open a Xmdv file.
OpenDatabase(data_path("okc_test_data/astronomy.okc"))

AddPlot("Pseudocolor", "classmagi")
AddOperator("Transform")
t = TransformAttributes()
t.doScale = 1
t.scaleZ = 0.03
SetOperatorOptions(t)
DrawPlots()

Test("Xmdv_01")

DefineArrayExpression("sub_arr", "array_compose(b, d)")
AddPlot("Label", "sub_arr")
L = LabelAttributes()
L.depthTestMode = L.LABEL_DT_NEVER
SetPlotOptions(L)
AddOperator("Transform")
SetOperatorOptions(t)
DrawPlots()

Test("Xmdv_02")


# This is super ugly, but I want to have something in the regression suite
# that tests reading in array variables directly from a file (for coverage
# purpose).
DeleteAllPlots()
AddPlot("Label", "all_vars")
L = LabelAttributes()
L.depthTestMode = L.LABEL_DT_NEVER
SetPlotOptions(L)
DrawPlots()
Test("Xmdv_03")
DeleteAllPlots()


# Now test the writer.

OpenDatabase(silo_data_path("rect2d.silo"))

DefineScalarExpression("uc", "recenter(u)")
DefineScalarExpression("vc", "recenter(v)")
AddPlot("Pseudocolor", "d")
DrawPlots()
e = ExportDBAttributes()
e.db_type = "Xmdv"
e.filename = "xmdv"
e.variables = ("d", "p", "uc", "vc")
ExportDatabase(e)

DeleteAllPlots()
DeleteExpression("uc")
DeleteExpression("vc")
OpenDatabase("xmdv.okc")
AddPlot("Pseudocolor", "vc")
DrawPlots()
Test("Xmdv_04")

DeleteAllPlots()
OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Vector", "vel")
DrawPlots()
e.variables = ()
e.filename = "xmdv2"
ExportDatabase(e)
DeleteAllPlots()
OpenDatabase("xmdv2.okc")
AddPlot("Pseudocolor", "vel_lb_0_rb_")
DrawPlots()
Test("Xmdv_05")


Exit()

