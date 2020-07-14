# ----------------------------------------------------------------------------
#  CLASSES: nightly
#  LIMIT: 1500
#  Test Case:  PLOT3D.py 
#
#  Programmer: Kathleen Biagas
#  Date:       Sep 5, 2014
#
#  Modifications:
#    Kathleen Biagas, Wed Aug 26 09:21:15 PDT 2015
#    Removed Magnitude scalars, no longer provided by reader since VisIt
#    already automatically creates vector magnitude expressions.
#    Added tests for time-series.
#
# ----------------------------------------------------------------------------


def BasicTest(testName, testNum):
    # These are the scalars and vectors from the solution file
    scalars = ["Density", "InternalEnergy"]
    vectors = ["Momentum"]

    # These are all the reader-calculated scalars and vectors
    computedScalars = ["Pressure", "Temperature", "Enthalpy",
                       "StagnationEnergy", "Entropy", "Swirl" ]
    computedVectors = ["Velocity", "Vorticity", "PressureGradient",
                        "StrainRate"]

    AddPlot("Mesh", "mesh")
    AddPlot("Pseudocolor", scalars[0])
    DrawPlots()
    Test("%s_%s_%02d"%(testName,scalars[0],testNum))
    for s in scalars[1:]:
        ChangeActivePlotsVar(s)
        DrawPlots()
        Test("%s_%s_%02d"%(testName,s,testNum))

    if testNum == 2:
        for s in computedScalars:
            ChangeActivePlotsVar(s)
            DrawPlots()
            Test("%s_%s_%02d"%(testName,s,testNum))

    DeleteAllPlots()

    AddPlot("Vector", vectors[0])
    vecAtts = VectorAttributes()
    vecAtts.scale = 0.5
    vecAtts.useStride = 1
    SetPlotOptions(vecAtts)
    DrawPlots()
    Test("%s_%s_%02d"%(testName,vectors[0],testNum))

    if testNum == 2:
        for v in computedVectors:
            ChangeActivePlotsVar(v)
            DrawPlots()
            Test("%s_%s_%02d"%(testName,v,testNum))

    DeleteAllPlots()
    AddPlot("Subset", "domains")
    DrawPlots()
    Test("%s_subsetDomains_%02d"%(testName,testNum))

    DeleteAllPlots()

# Open using the MetaFile .vp3d
def TestMetaOpen(metaFile, testName, testStart):
    OpenDatabase(data_path("PLOT3D_test_data/%s"%metaFile))
    BasicTest(testName, testStart)
    CloseDatabase(data_path("PLOT3D_test_data/%s"%metaFile))

# Open using the grid file and openOptions
def TestOpenOptions(gridFile, testname, options, testStart):
    if len(options)> 0:
        # Grab the default options
        opts = GetDefaultFileOpenOptions("PLOT3D")
        # Set the passed options
        for k,v in options.items():
            if k in list(opts.keys()):
                opts[k] = v
        SetDefaultFileOpenOptions("PLOT3D", opts)
    OpenDatabase(data_path("PLOT3D_test_data/%s"%gridFile),0, "PLOT3D_1.0")
    BasicTest(testname, testStart)
    CloseDatabase(data_path("PLOT3D_test_data/%s"%gridFile))

# names: [dir,  vp3dname, gridname, testname, openOptions]
names = [["BluntFin", "blunt.vp3d", "blunt.x", "bluntfin", {}],
         ["DeltaWing", "delta-40.vp3d", "delta-40.x", "deltawing", {}],
         ["LiquidOxygenPost", "post.vp3d", "post.x", "post", {}],
         ["SpaceShuttleLaunchVehicle", "sslv.vp3d", "grid", "sslv", 
           {"Multi Grid": 1, "IBlanking":1, "Solution (Q) File Name":"solution"}],
         ["WingBodyTail", "wbt.vp3d", "wbtg.bin", "wbt", 
           {"Solution (Q) File Name":"wbtr.bin",
            "Multi Grid":1,
            "IBlanking":1,
            "File Format":1,
            "Big Endian":1,
            "Double Precision":0,
            "3D":1}]]


def TestTimeSeries():
    TestSection("Time Series with meta file")
    SetCreateMeshQualityExpressions(0)
    SetCreateTimeDerivativeExpressions(0)
    SetCreateVectorMagnitudeExpressions(0)
    #SetViewExtentsType("actual")
    OpenDatabase(data_path("PLOT3D_test_data/BluntFin/blunt_timeseries.vp3d"))
    md = GetMetaData(data_path("PLOT3D_test_data/BluntFin/blunt_timeseries.vp3d"))
    TestText("plot3d_timeseries_1", str(md))

    DefineScalarExpression("TD", "time(mesh) * Density")
    AddPlot("Pseudocolor", "TD")
    AddOperator("Isovolume")
    isoAtts = IsovolumeAttributes()
    isoAtts.lbound = 2000
    SetOperatorOptions(isoAtts)
    DrawPlots()

    #v = GetView3D()
    #v.viewNormal = (0, -1, 6.12323e-17)
    #v.focus = (3.32801, -0.103006, 16.13)
    #v.viewUp = (0, 6.12323e-17, 1)
    #SetView3D(v)

    #  step through time
    for t in range(0, TimeSliderGetNStates(),2):
        SetTimeSliderState(t)
        Query("Time")
        Test("plot3d_timeseries_1_%s" % GetQueryOutputValue())

    DeleteAllPlots()

    CloseDatabase(data_path("PLOT3D_test_data/BluntFin/blunt_timeseries.vp3d"))

    TestSection("Time series via Read Options") 

    # specify sub-selection of time slices.
    opts = {"Solution (Q) File Name" : r"TimeSeries/blunt_??3?.q",
            "Solution Time field accurate": 0}
    SetDefaultFileOpenOptions("PLOT3D", opts)

    OpenDatabase(data_path("PLOT3D_test_data/BluntFin/blunt.x"), 0, "PLOT3D_1.0")
    md = GetMetaData(data_path("PLOT3D_test_data/BluntFin/blunt.x"))
    TestText("plot3d_timeseries_2", str(md))

    AddPlot("Pseudocolor", "TD")
    AddOperator("Isovolume")
    SetOperatorOptions(isoAtts)
    DrawPlots()

    #SetView3D(v)
    
    #  step through time
    for t in range(0, TimeSliderGetNStates()):
        SetTimeSliderState(t)
        Query("Time")
        Test("plot3d_timeseries_2_%s" % GetQueryOutputValue())

    DeleteAllPlots()
    CloseDatabase(data_path("PLOT3D_test_data/BluntFin/blunt.x"))

def BasicTests():
    TestSection("Open using MetaFile .vp3d")
    for f in names:
        # test opening meta file
        TestMetaOpen("%s/%s"%(f[0],f[1]), f[3], 1)

    TestSection("Open using gridfile and Open Options")
    # save true defaults for resetting
    opts = GetDefaultFileOpenOptions("PLOT3D")
    for f in names:
        # test opening grid file, possibly using OpenOptions
        TestOpenOptions("%s/%s"%(f[0],f[2]), f[3], f[4], 2)
    # reset to true defaults
    SetDefaultFileOpenOptions("PLOT3D", opts)

BasicTests()
TestTimeSeries()
    
Exit()
