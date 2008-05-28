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
    Query("Chord Length Distribution (individual)", 15000, 100, 0, limit)
    Query("Chord Length Distribution (aggregate)", 15000, 100, 0, limit)
    Query("Ray Length Distribution (individual)", 15000, 100, 0, limit)
    Query("Ray Length Distribution (aggregate)", 15000, 100, 0, limit)
    Query("Mass Distribution", 15000, 100, 0, limit)
    Query("Distance From Boundary", 15000, 100, 0, limit)
    Query("Line Scan Transform", 15000, 100, 0, limit)
    DeleteAllPlots()
    
    # Now test the outputs
    OpenDatabase("cld_i0.ult")
    ReOpenDatabase("cld_i0.ult")  # Flush out cached version
    AddPlot("Curve", "Chord length distribution <individual>")
    DrawPlots()
    ResetView()
    test1 = "ls_%d_%d" %(index,1)
    Test(test1)
    Query("Integrate")
    text = GetQueryOutputString()
    test2 = "ls_%d_%d" %(index,2)
    TestText(test2, text)
    Query("Expected Value")
    text = GetQueryOutputString()
    test3 = "ls_%d_%d" %(index,3)
    TestText(test3, text)
    DeleteAllPlots()
    OpenDatabase("md0.ult")
    ReOpenDatabase("md0.ult")  # Flush out cached version
    AddPlot("Curve", "Mass distribution")
    DrawPlots()
    ResetView()
    test4 = "ls_%d_%d" %(index,4)
    Test(test4)
    Query("Integrate")
    text = GetQueryOutputString()
    test5 = "ls_%d_%d" %(index,5)
    TestText(test5, text)
    Query("Expected Value")
    text = GetQueryOutputString()
    test6 = "ls_%d_%d" %(index,6)
    TestText(test6, text)
    DeleteAllPlots()
    OpenDatabase("dfb0.ult")
    ReOpenDatabase("dfb0.ult")  # Flush out cached version
    AddPlot("Curve", "Mass distribution")
    DrawPlots()
    ResetView()
    test7 = "ls_%d_%d" %(index,7)
    Test(test7)
    Query("Integrate")
    text = GetQueryOutputString()
    test8 = "ls_%d_%d" %(index,8)
    TestText(test8, text)
    Query("Expected Value")
    text = GetQueryOutputString()
    test9 = "ls_%d_%d" %(index,9)
    TestText(test9, text)
    DeleteAllPlots()
    OpenDatabase("cld_a0.ult")
    ReOpenDatabase("cld_a0.ult")  # Flush out cached version
    AddPlot("Curve", "Chord length distribution <aggregate>")
    DrawPlots()
    ResetView()
    test1 = "ls_%d_%d" %(index,10)
    Test(test1)
    Query("Integrate")
    text = GetQueryOutputString()
    test2 = "ls_%d_%d" %(index,11)
    TestText(test2, text)
    Query("Expected Value")
    text = GetQueryOutputString()
    test3 = "ls_%d_%d" %(index,12)
    TestText(test3, text)
    DeleteAllPlots()
    OpenDatabase("rld_i0.ult")
    ReOpenDatabase("rld_i0.ult")  # Flush out cached version
    AddPlot("Curve", "Ray length distribution <individual>")
    DrawPlots()
    ResetView()
    test1 = "ls_%d_%d" %(index,13)
    Test(test1)
    Query("Integrate")
    text = GetQueryOutputString()
    test2 = "ls_%d_%d" %(index,14)
    TestText(test2, text)
    Query("Expected Value")
    text = GetQueryOutputString()
    test3 = "ls_%d_%d" %(index,15)
    TestText(test3, text)
    DeleteAllPlots()
    OpenDatabase("rld_a0.ult")
    ReOpenDatabase("rld_a0.ult")  # Flush out cached version
    AddPlot("Curve", "Ray length distribution <aggregate>")
    DrawPlots()
    ResetView()
    test1 = "ls_%d_%d" %(index,16)
    Test(test1)
    Query("Integrate")
    text = GetQueryOutputString()
    test2 = "ls_%d_%d" %(index,17)
    TestText(test2, text)
    Query("Expected Value")
    text = GetQueryOutputString()
    test3 = "ls_%d_%d" %(index,18)
    TestText(test3, text)
    DeleteAllPlots()
    OpenDatabase("lst0.ult")
    ReOpenDatabase("lst0.ult")  # Flush out cached version
    AddPlot("Curve", "Line Scan Transform")
    DrawPlots()
    ResetView()
    test1 = "ls_%d_%d" %(index,19)
    Test(test1)
    Query("Integrate")
    text = GetQueryOutputString()
    test2 = "ls_%d_%d" %(index,20)
    TestText(test2, text)
    Query("Expected Value")
    text = GetQueryOutputString()
    test3 = "ls_%d_%d" %(index,21)
    TestText(test3, text)
    DeleteAllPlots()


TestOne(0, "../data/rect2d.silo", "d", "quadmesh2d", 0.5)
TestOne(1, "../data/multi_rect2d.silo", "d", "mesh1", 0.5)
TestOne(2, "../data/KullLite_test_data/T.pdb", "mesh_quality/volume", "mesh", 0.2)
TestOne(3, "../data/KullLite_test_data/tagtest_rz_3.pdb", "mesh_quality/mesh/area", "mesh", 4.0)

Exit()
