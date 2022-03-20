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
#    Kathleen Biagas, Wed Feb 16 13:11:57 PST 2022
#    Replaced Curve atts 'cycleColors' with 'curveColorSource'.
#
# ----------------------------------------------------------------------------

import os


ca=CurveAttributes()
ca.curveColorSource = ca.Custom
SetDefaultPlotOptions(ca)

def TestOne(index, filename, varname, meshname, absvar, emisvar, numlines, x, y, z, radius, theta, phi):
    # Because the queries we are testing output to the file system, we need to
    # delete previous outputs before running.  If we don't do this, then the
    # queries will output files to names that are continuously incrementing.
    list = os.listdir(".")
    for i in range(len(list)):
        if (list[i] == "hf0.ult"):
            os.unlink("hf0.ult")

    # Do the queries...
    OpenDatabase(filename)
    AddPlot("Pseudocolor", varname)
    DrawPlots()
    params = dict(num_lines=numlines, ray_center=(x, y, z), radius=radius, theta=theta, phi=phi, vars=(absvar, emisvar))
    Query("Hohlraum Flux", params)
    s = GetQueryOutputString()
    #v = GetQueryOutputValue()
    test_name = "hf_%d_%d" %(index,0)
    TestText(test_name, s)
    DeleteAllPlots()

    # Now test the outputs
    OpenDatabase("hf0.ult")
    ReOpenDatabase("hf0.ult")  # Flush out cached version
    AddPlot("Curve", "Hohlraum Flux")
    DrawPlots()
    ResetView()
    test1 = "hf_%d_%d" %(index,1)
    Test(test1)
    DeleteAllPlots()


DefineArrayExpression("a0", "array_compose(recenter(u), recenter(v), d)")
DefineArrayExpression("e0", "array_compose(recenter(gradient(u)[0]), recenter(gradient(u)[1]), p)")
TestOne(0, silo_data_path("rect2d.silo"), "d", "quadmesh2d", "a0", "e0", 100, 0.5,0.5,0, 0.1,0,0)
DeleteExpression("a0")
DeleteExpression("e0")

DefineArrayExpression("a1", "array_compose(<mesh_quality/volume>, <mesh_quality/taper>,  <mesh_quality/oddy>)")
DefineArrayExpression("e1", "array_compose(<mesh_quality/aspect>, <mesh_quality/shape>, <mesh_quality/skew>)")
TestOne(1, data_path("KullLite_test_data/T.pdb"),
        "mesh_quality/volume", "mesh",
        "a1", "e1", 100, 0,0,0, 0.25,90,90)
DeleteExpression("a1")
DeleteExpression("e1")

DefineArrayExpression("a2", "array_compose(<mesh_quality/mesh/jacobian>, \
                      <mesh_quality/mesh/area>, <mesh_quality/mesh/oddy>)")
DefineArrayExpression("e2", "array_compose(<mesh_quality/mesh/aspect>, \
                      <mesh_quality/mesh/shape>, <mesh_quality/mesh/skew>)")
TestOne(2, data_path("KullLite_test_data/tagtest_rz_3.pdb"),
        "mesh_quality/mesh/area", "mesh",
        "a2", "e2", 333, 0,0,1, 0.5,0,0)
DeleteExpression("a2")
DeleteExpression("e2")

TestOne(3, silo_data_path("multi_rect2d.silo"), "d", "mesh1", "d", "p", 500, 0.5,0.5,0, 1,45,45)


DefineArrayExpression("a3", "array_compose_with_bins(<mesh_quality/mesh/jacobian>,"
                            "<mesh_quality/mesh/area>, <mesh_quality/mesh/oddy>,  [0, 2, 4, 8])")
DefineArrayExpression("e3", "array_compose_with_bins(<mesh_quality/mesh/aspect>,"
                            "<mesh_quality/mesh/shape>, <mesh_quality/mesh/skew>, [0, 2, 4, 8])")
TestOne(4, data_path("KullLite_test_data/tagtest_rz_3.pdb"), "mesh_quality/mesh/area", "mesh",
        "a3", "e3", 333, 0,0,1, 0.5,0,0)
DeleteExpression("a3")
DeleteExpression("e3")






Exit()
