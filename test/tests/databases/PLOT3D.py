# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  PLOT3D.py 
#
#  Programmer: Kathleen Biagas
#  Date:       Sep 5, 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------


def BasicTest(testName, testNum):
    # These are all the data-provided scalars and vectors
    scalars = ["Density", "InternalEnergy"]
    vectors = ["Momentum"]

    # These are all the reader-calculated scalars and vectors
    computedScalars = ["Pressure", "Temperature", "Enthalpy", "VelocityMagnitude", 
                     "StagnationEnergy", "Entropy", "Swirl", "VorticityMagnitude" ]
    computedVectors = ["Velocity", "Vorticity", "PressureGradient", "StrainRate"]

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
        for k,v in options.iteritems():
            if k in opts.keys():
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


TestSection("Open using MetaFile .vp3d")
for f in names:
    # test opening meta file
    TestMetaOpen("%s/%s"%(f[0],f[1]), f[3], 1)

TestSection("Open using gridfile and Open Options")
for f in names:
    # test opening grid file, possibly using OpenOptions
    TestOpenOptions("%s/%s"%(f[0],f[2]), f[3], f[4], 2)
    
Exit()
