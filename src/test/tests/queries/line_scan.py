# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  line_scan.py
#  Tests:      queries     - mass distribution, chord length distribution
#
#  Defect ID:  '7474 + '7476
#
#  Programmer: Hank Childs
#  Date:       August 23, 2006
#
#  Modifications:
# 
#    Hank Childs, Sun Aug 27 16:52:07 PDT 2006
#    Added testing for expected value, mass from boundary.
#
#    Dave Bremer, Fri Sep  8 11:44:22 PDT 2006
#    Added testing for the line scan transform.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Kathleen Biagas, Thu Jul 14 10:44:55 PDT 2011
#    Use named arguments. 
#
# ----------------------------------------------------------------------------

import os

ca=CurveAttributes()
ca.cycleColors = 0
SetDefaultPlotOptions(ca)

def TestOne(index, filename, varname, meshname, limit):
    # Because the queries we are testing output to the file system, we need to
    # delete previous outputs before running.  If we don't do this, then the 
    # queries will output files to names that are continuously incrementing.
    list = os.listdir(".")
    for i in range(len(list)):
        if (list[i] == "cld_i0.ult"):
            os.unlink("cld_i0.ult")
        if (list[i] == "cld_a0.ult"):
            os.unlink("cld_a0.ult")
        if (list[i] == "rld_i0.ult"):
            os.unlink("rld_i0.ult")
        if (list[i] == "rld_a0.ult"):
            os.unlink("rld_a0.ult")
        if (list[i] == "md0.ult"):
            os.unlink("md0.ult")
        if (list[i] == "dfb0.ult"):
            os.unlink("dfb0.ult")
        if (list[i] == "lst0.ult"):
            os.unlink("lst0.ult")

    # Set up a purely 2D data set...
    randname = "rand%d" %(index)
    defn = "rand(%s)" %(meshname)
    DefineScalarExpression(randname, defn)
    OpenDatabase(filename)
    AddPlot("Pseudocolor", varname)
    AddOperator("Isovolume")
    iso_atts = IsovolumeAttributes()
    iso_atts.ubound = 0.3
    iso_atts.variable = randname
    SetOperatorOptions(iso_atts)
    DrawPlots()

    # Do the queries...
    Query("Chord Length Distribution - individual", num_lines=15000, num_bins=100, min=0, max=limit)
    Query("Chord Length Distribution - aggregate", num_lines=15000, num_bins=100, min=0, max=limit)
    Query("Ray Length Distribution - individual", num_lines=15000, num_bins=100, min=0, max=limit)
    Query("Ray Length Distribution - aggregate", num_lines=15000, num_bins=100, min=0, max=limit)
    Query("Mass Distribution", num_lines=15000, num_bins=100, min=0, max=limit)
    Query("Distance From Boundary", num_lines=15000, num_bins=100, min=0, max=limit)
    Query("Line Scan Transform", num_lines=15000, num_bins=100, min=0, max=limit)
    DeleteAllPlots()
    
    # Now test the outputs
    OpenDatabase("cld_i0.ult")
    ReOpenDatabase("cld_i0.ult")  # Flush out cached version
    AddPlot("Curve", "Chord length distribution - individual")
    DrawPlots()
    ResetView()
    test1 = "ls_%d_%d" %(index,1)
    Test(test1)
    Query("Integrate")
    test2 = "ls_%d_%d" %(index,2)
    TestValueEQ(test2, GetQueryOutputValue(), baseVals[test2])
    Query("Expected Value")
    test3 = "ls_%d_%d" %(index,3)
    TestValueEQ(test3, GetQueryOutputValue(), baseVals[test3])
    DeleteAllPlots()
    OpenDatabase("md0.ult")
    ReOpenDatabase("md0.ult")  # Flush out cached version
    AddPlot("Curve", "Mass distribution")
    DrawPlots()
    ResetView()
    test4 = "ls_%d_%d" %(index,4)
    Test(test4)
    Query("Integrate")
    test5 = "ls_%d_%d" %(index,5)
    TestValueEQ(test5, GetQueryOutputValue(), baseVals[test5])
    Query("Expected Value")
    test6 = "ls_%d_%d" %(index,6)
    TestValueEQ(test6, GetQueryOutputValue(), baseVals[test6])
    DeleteAllPlots()
    OpenDatabase("dfb0.ult")
    ReOpenDatabase("dfb0.ult")  # Flush out cached version
    AddPlot("Curve", "Mass distribution")
    DrawPlots()
    ResetView()
    test7 = "ls_%d_%d" %(index,7)
    Test(test7)
    Query("Integrate")
    test8 = "ls_%d_%d" %(index,8)
    TestValueEQ(test8, GetQueryOutputValue(), baseVals[test8])
    Query("Expected Value")
    test9 = "ls_%d_%d" %(index,9)
    TestValueEQ(test9, GetQueryOutputValue(), baseVals[test9])
    DeleteAllPlots()
    OpenDatabase("cld_a0.ult")
    ReOpenDatabase("cld_a0.ult")  # Flush out cached version
    AddPlot("Curve", "Chord length distribution - aggregate")
    DrawPlots()
    ResetView()
    test1 = "ls_%d_%d" %(index,10)
    Test(test1)
    Query("Integrate")
    test2 = "ls_%d_%d" %(index,11)
    TestValueEQ(test2, GetQueryOutputValue(), baseVals[test2])
    Query("Expected Value")
    test3 = "ls_%d_%d" %(index,12)
    TestValueEQ(test3, GetQueryOutputValue(), baseVals[test3])
    DeleteAllPlots()
    OpenDatabase("rld_i0.ult")
    ReOpenDatabase("rld_i0.ult")  # Flush out cached version
    AddPlot("Curve", "Ray length distribution - individual")
    DrawPlots()
    ResetView()
    test1 = "ls_%d_%d" %(index,13)
    Test(test1)
    Query("Integrate")
    test2 = "ls_%d_%d" %(index,14)
    TestValueEQ(test2, GetQueryOutputValue(), baseVals[test2])
    Query("Expected Value")
    test3 = "ls_%d_%d" %(index,15)
    TestValueEQ(test3, GetQueryOutputValue(), baseVals[test3])
    DeleteAllPlots()
    OpenDatabase("rld_a0.ult")
    ReOpenDatabase("rld_a0.ult")  # Flush out cached version
    AddPlot("Curve", "Ray length distribution - aggregate")
    DrawPlots()
    ResetView()
    test1 = "ls_%d_%d" %(index,16)
    Test(test1)
    Query("Integrate")
    test2 = "ls_%d_%d" %(index,17)
    TestValueEQ(test2, GetQueryOutputValue(), baseVals[test2])
    Query("Expected Value")
    test3 = "ls_%d_%d" %(index,18)
    TestValueEQ(test3, GetQueryOutputValue(), baseVals[test3])
    DeleteAllPlots()
    OpenDatabase("lst0.ult")
    ReOpenDatabase("lst0.ult")  # Flush out cached version
    AddPlot("Curve", "Line Scan Transform")
    DrawPlots()
    ResetView()
    test1 = "ls_%d_%d" %(index,19)
    Test(test1)
    Query("Integrate")
    test2 = "ls_%d_%d" %(index,20)
    TestValueEQ(test2, GetQueryOutputValue(), baseVals[test2])
    Query("Expected Value")
    test3 = "ls_%d_%d" %(index,21)
    TestValueEQ(test3, GetQueryOutputValue(), baseVals[test3])
    DeleteAllPlots()

#
# Expected results for each test
#
baseVals = {
    "ls_0_2":1,
    "ls_0_3":0.0312789,
    "ls_0_5":0.125232,
    "ls_0_6":0.00631335,
    "ls_0_8":0.125232,
    "ls_0_9":0.00316435,
    "ls_1_11":1,
    "ls_0_11":1,
    "ls_0_12":0.182223,
    "ls_0_14":1,
    "ls_0_15":0.0141507,
    "ls_0_17":1,
    "ls_0_18":0.0572263,
    "ls_0_20":0.164641,
    "ls_0_21":0.00609974,
    "ls_1_2":1,
    "ls_1_3":0.0322331,
    "ls_1_5":0.142894,
    "ls_1_6":0.00773154,
    "ls_1_8":0.142894,
    "ls_1_9":0.00387523,
    "ls_1_12":0.20811,
    "ls_1_14":0.999999,
    "ls_1_15":0.0145833,
    "ls_1_17":1,
    "ls_1_18":0.0642869,
    "ls_1_20":0.18530,
    "ls_1_21":0.00830585,
    "ls_2_2":1,
    "ls_2_3":0.0266496,
    "ls_2_5":3.38562e-06,
    "ls_2_6":1.87224e-07,
    "ls_2_8":3.38562e-06,
    "ls_2_9":9.37578e-08,
    "ls_2_11":1,
    "ls_3_11":1,
    "ls_2_12":0.0860585,
    "ls_2_14":1,
    "ls_2_15":0.001,
    "ls_2_17":1,
    "ls_2_18":0.001,
    "ls_2_20":0.0798091,
    "ls_2_21":0.00218212,
    "ls_3_2":1,
    "ls_3_3":1.08223,
    "ls_3_5":1.11039,
    "ls_3_6":1.53097,
    "ls_3_8":1.11039,
    "ls_3_9":0.765742,
    "ls_3_12":1.25808,
    "ls_3_14":0.999999,
    "ls_3_15":0.160933,
    "ls_3_17":0.999999,
    "ls_3_18":0.178107,
    "ls_3_20":1.25822,
    "ls_3_21":0.9419}

TestOne(0, silo_data_path("rect2d.silo") , "d", "quadmesh2d", 0.5)
TestOne(1, silo_data_path("multi_rect2d.silo") , "d", "mesh1", 0.5)
TestOne(2, data_path("KullLite_test_data/T.pdb"), "mesh_quality/volume", "mesh", 0.2)
TestOne(3, data_path("KullLite_test_data/tagtest_rz_3.pdb"), "mesh_quality/mesh/area", "mesh", 4.0)

Exit()
