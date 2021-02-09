# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  tecplot.py
#
#  Tests:      TecPlot files
#
#  Programmer: Jeremy Meredith
#  Date:       November 17, 2004
#
#  Modifications:
#    Hank Childs, Wed Feb 16 07:34:07 PST 2005
#    Rename variables that have unsupported characters.
#
#    Brad Whitlock, Thu Mar 31 17:12:06 PST 2005
#    Changed the data directory to ../data/tecplot_test_data/tecplot_test_data,
#    which is now created from a bzipped tar file.
#
#    Brad Whitlock, Tue Jul 26 16:27:57 PST 2005
#    I added a test that has 2D data with a vector expression.
#
#    Jeremy Meredith, Wed Sep  7 12:06:04 PDT 2005
#    Allowed spaces in variable names.
#
#    Jeremy Meredith, Mon Jul  7 14:20:12 EDT 2008
#    The tecplot reader now successfully parses x/y/z coordinate variables
#    from the simpscat.tec file, so I changed the plot to be a pseudocolor
#    plot instead of a curve plot.
#
#    Hank Childs, Sat Mar 13 11:13:26 PST 2010
#    Change variable names for forbidden characters.
#
#    Jeremy Meredith, Fri May 20 09:37:14 EDT 2011
#    Change subset mesh variable name since files have more than one mesh now.
#
#    Kathleen Biagas, Wed Aug 28 09:04:00 MST 2019
#    Turn off cycling of colors for all Curve plot tests.  Set the colors
#    individually to match current baseline results.
#
# ----------------------------------------------------------------------------


def SetTheView():
    ResetView()
    v = GetView3D()
    v.viewNormal =  (-0.553771, 0.509362, 0.658702)
    SetView3D(v)

# Databases
OpenDatabase(data_path("tecplot_test_data/2dfed.tec"))

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","P")
DrawPlots()
SetTheView()
Test("tecplot_01")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/3dgeom.tec"))

AddPlot("Mesh","mesh")
AddPlot("Subset","Zones(mesh)")
DrawPlots()
SetTheView()
Test("tecplot_02")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/T3L3CLS17u.plt"))

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","U")
DrawPlots()
ResetView()
v=GetView2D()
v.windowCoords=(-.01,.03,-.02,.02)
SetView2D(v)
Test("tecplot_03")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/channel.tec"))

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","U")
DrawPlots()
SetTheView()
Test("tecplot_04")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/crystal_plot.tec"))

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","t")
DrawPlots()
SetTheView()
Test("tecplot_05")
DeleteAllPlots();
CloseDatabase(data_path("tecplot_test_data/crystal_plot.tec"))


OpenDatabase(data_path("tecplot_test_data/cube.tec"))

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","R")
DrawPlots()
SetTheView()
Test("tecplot_06")
DeleteAllPlots();

# We must skip this one, as it has column-major
# ordering instead of row-major, and we are
# expecting row-major.  There is nothing in the
# tecplot file format to specify which type it is,
# so this must wait until we have options for
# the database plugins.  See '2440.
##OpenDatabase(data_path("tecplot_test_data/cylindrical.tec"))

#AddPlot("Mesh","mesh")
#DrawPlots()
#SetTheView()
#Test("tecplot_07")
#DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/febrfeb.tec"))

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","Temperature")
DrawPlots()
SetTheView()
Test("tecplot_08")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/febrfep.tec"))

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","Temperature")
DrawPlots()
SetTheView()
Test("tecplot_09")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/fetebk.tec"))

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","C")
DrawPlots()
SetTheView()
Test("tecplot_10")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/fetetpt.tec"))

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","C")
DrawPlots()
SetTheView()
Test("tecplot_11")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/flow5.tec"))

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","V")
DrawPlots()
SetTheView()
Test("tecplot_12")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/human.tec"))

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","R")
DrawPlots()
SetTheView()
Test("tecplot_13")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/movie.tec"))

AddPlot("Mesh","mesh")
AddPlot("Subset","Zones(mesh)")
DrawPlots()
SetTheView()
Test("tecplot_14")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/multzn2d.tec"))

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","Temp")
DrawPlots()
SetTheView()
Test("tecplot_15")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/order1.tec"))

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","psi")
DrawPlots()
SetTheView()
Test("tecplot_16")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/sample.tp"))

AddPlot("Mesh","mesh")
AddPlot("Volume","X Velocity")
v=VolumeAttributes()
SetPlotOptions(v)
DrawPlots()
SetTheView()
Test("tecplot_17")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/simp3dbk.tec"))

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","Density")
DrawPlots()
SetTheView()
Test("tecplot_18")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/simp3dpt.tec"))

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","Density")
DrawPlots()
SetTheView()
Test("tecplot_19")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/simpscat.tec"))

AddPlot("Pseudocolor","T_lb_K_rb_")
p = PseudocolorAttributes()
p.pointSizePixels = 6
SetPlotOptions(p)
AddOperator("Threshold")
t = ThresholdAttributes()
t.outputMeshType = 1
SetOperatorOptions(t)
DrawPlots()
ResetView()
Test("tecplot_20")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/simpxy.tec"))

AddPlot("Curve","Temperature vs/Distance")
curveAtts = CurveAttributes()
curveAtts.curveColorSource = curveAtts.Custom
curveAtts.curveColor = (255, 0, 0, 255)
SetPlotOptions(curveAtts)
DrawPlots()
SetTheView()
Test("tecplot_21")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/simpxy2.tec"))

AddPlot("Curve","Only Zone/Temperature vs/Distance")
curveAtts.curveColor = (0, 255, 0, 255)
SetPlotOptions(curveAtts)
DrawPlots()
SetTheView()
Test("tecplot_22")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/symmetricFlow.plt"))

AddPlot("Pseudocolor","VELOCITY")
DrawPlots()
ResetView()
Test("tecplot_23")
DeleteAllPlots();
AddPlot("Vector", "VEL")
v = VectorAttributes()
v.useStride = 1
SetPlotOptions(v)
DrawPlots()
v = View2DAttributes()
v.windowCoords = (53.9908, 74.1434, 66.5995, 88.2732)
v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
v.fullFrameActivationMode = v.Off  # On, Off, Auto
v.fullFrameAutoThreshold = 100
SetView2D(v)
Test("tecplot_24")
DeleteAllPlots();

OpenDatabase(data_path("tecplot_test_data/pointmesh.tec"))

AddPlot("Mesh","mesh")
ResetView()
DrawPlots()
Test("tecplot_25")
DeleteAllPlots()

DeleteAllPlots();
CloseDatabase(data_path("tecplot_test_data/pointmesh.tec"))

# binary file containing both node and cell data in block format
OpenDatabase(data_path("tecplot_test_data/two_triangles_node_and_cell.plt"))

AddPlot("Pseudocolor","nodal_field")
ResetView()
DrawPlots()
Test("tecplot_26")
DeleteAllPlots()

AddPlot("Pseudocolor","cell_field")
ResetView()
DrawPlots()
Test("tecplot_27")
DeleteAllPlots()

Exit()
