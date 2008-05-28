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
#    Changed the data directory to ../data/tecplot_test_data/tecplot_test_data, which is now
#    created from a bzipped tar file.
#
#    Brad Whitlock, Tue Jul 26 16:27:57 PST 2005
#    I added a test that has 2D data with a vector expression.
#
#    Jeremy Meredith, Wed Sep  7 12:06:04 PDT 2005
#    Allowed spaces in variable names.
#
# ----------------------------------------------------------------------------


def SetTheView():
    ResetView()
    v = GetView3D()
    v.viewNormal =  (-0.553771, 0.509362, 0.658702)
    SetView3D(v)

# Databases
OpenDatabase("../data/tecplot_test_data/2dfed.tec")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","P")
DrawPlots()
SetTheView()
Test("tecplot_01")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/3dgeom.tec")
AddPlot("Mesh","mesh")
AddPlot("Subset","Zones")
DrawPlots()
SetTheView()
Test("tecplot_02")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/T3L3CLS17u.plt")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","U")
DrawPlots()
ResetView()
v=GetView2D()
v.windowCoords=(-.01,.03,-.02,.02)
SetView2D(v)
Test("tecplot_03")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/channel.tec")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","U")
DrawPlots()
SetTheView()
Test("tecplot_04")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/crystal_plot.tec")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","t")
DrawPlots()
SetTheView()
Test("tecplot_05")
DeleteAllPlots();
CloseDatabase("../data/tecplot_test_data/crystal_plot.tec")

OpenDatabase("../data/tecplot_test_data/cube.tec")
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
#OpenDatabase("../data/tecplot_test_data/cylindrical.tec")
#AddPlot("Mesh","mesh")
#DrawPlots()
#SetTheView()
#Test("tecplot_07")
#DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/febrfeb.tec")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","Temperature")
DrawPlots()
SetTheView()
Test("tecplot_08")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/febrfep.tec")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","Temperature")
DrawPlots()
SetTheView()
Test("tecplot_09")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/fetebk.tec")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","C")
DrawPlots()
SetTheView()
Test("tecplot_10")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/fetetpt.tec")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","C")
DrawPlots()
SetTheView()
Test("tecplot_11")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/flow5.tec")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","V")
DrawPlots()
SetTheView()
Test("tecplot_12")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/human.tec")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","R")
DrawPlots()
SetTheView()
Test("tecplot_13")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/movie.tec")
AddPlot("Mesh","mesh")
AddPlot("Subset","Zones")
DrawPlots()
SetTheView()
Test("tecplot_14")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/multzn2d.tec")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","Temp")
DrawPlots()
SetTheView()
Test("tecplot_15")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/order1.tec")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","psi")
DrawPlots()
SetTheView()
Test("tecplot_16")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/sample.tp")
AddPlot("Mesh","mesh")
AddPlot("Volume","X Velocity")
v=VolumeAttributes()
v.opacityAttenuation=.05
SetPlotOptions(v)
DrawPlots()
SetTheView()
Test("tecplot_17")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/simp3dbk.tec")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","Density")
DrawPlots()
SetTheView()
Test("tecplot_18")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/simp3dpt.tec")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","Density")
DrawPlots()
SetTheView()
Test("tecplot_19")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/simpscat.tec")
AddPlot("Curve","X[M] vs/Y[M]")
DrawPlots()
SetTheView()
Test("tecplot_20")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/simpxy.tec")
AddPlot("Curve","Temperature vs/Distance")
DrawPlots()
SetTheView()
Test("tecplot_21")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/simpxy2.tec")
AddPlot("Curve","Only Zone/Temperature vs/Distance")
DrawPlots()
SetTheView()
Test("tecplot_22")
DeleteAllPlots();

OpenDatabase("../data/tecplot_test_data/symmetricFlow.plt")
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

OpenDatabase("../data/tecplot_test_data/pointmesh.tec")
AddPlot("Mesh","mesh")
ResetView()
DrawPlots()
Test("tecplot_25")
DeleteAllPlots()

Exit()
