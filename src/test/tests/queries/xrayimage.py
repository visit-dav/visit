# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  xrayimage.py
#  Tests:      queries     - xray imagel
#
#  Programmer: Eric Brugger
#  Date:       July 13, 2010
#
#  Modifications:
#    Kathleen Bonnell, Thu Jul 14 10:44:55 PDT 2011
#    Change most of code to use python dictionary to pass query parameters.
#    First call to 'Query' still tests old-style argument passing.
#    Second call to 'Query' creates a Python dictionary from scratch and
#    uses that.  Prior to third call to Query, retrieve default dictionary via
#    GetQueryParameters.  All subsequent calls to Query modify that dictionary 
#    object as necessary and pass it.
#
#    Kathleen Biagas, Wed Oct 17 14:25:05 PDT 2012
#    Show usage of new 'up_vector' parameter.
# 
#    Justin Privitera, Tue Jun 14 10:02:21 PDT 2022
#    Change tests to reflect new ability to send output directory to query.
# 
#    Justin Privitera, Wed Jun 15 16:43:34 PDT 2022
#    Added tests for new blueprint output.
# 
#    Justin Privitera, Wed Jul 20 13:54:06 PDT 2022
#    Added query output msg tests and tests for query errors.
# 
#    Justin Privitera, Thu Sep  8 16:29:06 PDT 2022
#    Added new tests for blueprint output metadata.
# 
#    Justin Privitera, Tue Sep 27 10:52:59 PDT 2022
#    Changed names of most output files to reflect new naming conventions.
#    Added tests for filenames, all output types, and result messages.
# 
#    Justin Privitera, Fri Sep 30 15:54:40 PDT 2022
#    Changed location of temp output files.
#    os.remove is gone.
#    tmp/baddir is gone, replaced.
#    These changes were made so the tests no longer crash on windows.
# 
#    Justin Privitera, Tue Nov 15 14:54:35 PST 2022
#    Added new tests for additional blueprint output metadata as well as
#    imaging plane topologies.
# 
#    Justin Privitera, Tue Nov 22 14:56:04 PST 2022
#    Updated numbering on later tests.
#    Reorganized blueprint tests so they use a function. That function also
#    uses new and old query calls, doubling the number of blueprint tests.
#    It also tests energy group bin output for hdf5.
# 
#    Justin Privitera, Mon Nov 28 15:38:25 PST 2022
#    Renamed energy group bins to energy group bounds.
# 
#    Justin Privitera, Wed Nov 30 10:41:17 PST 2022
#    Added tests for always positive detector height and width in blueprint
#    metadata.
# 
#    Justin Privitera, Wed Nov 30 17:43:48 PST 2022
#    Added tests for piping the units through the query.
# 
#    Justin Privitera, Thu Dec  1 15:29:48 PST 2022
#    Tests for new location of units in blueprint output.
# 
#    Justin Privitera, Wed Dec  7 16:16:16 PST 2022
#    Added tests for the blueprint ray output.
# 
#    Justin Privitera, Mon Dec 12 13:28:55 PST 2022
#    Reworked some of the blueprint output tests to reflect changes in 
#    output metadata.
#    Added new tests for path length and spatial extent images.
# 
#    Justin Privitera, Tue Feb 14 17:02:43 PST 2023
#    Change all metadata to use pot_hole_case instead of camelCase.
#    Add test for image_topo_order_of_domain_variables.
#
#    Justin Privitera, Wed Oct 12 11:38:11 PDT 2022
#    Changed output type for many tests since bmp output type is removed.
#
# ----------------------------------------------------------------------------

import os
import conduit
import conduit.blueprint
import conduit.relay 

#
# Test a single block structured grid with scalars.
#
OpenDatabase(silo_data_path("curv3d.silo"))

AddPlot("Pseudocolor", "d")
DrawPlots()

# old style argument passing
Query("XRay Image", 1, ".", 1, 0.0, 2.5, 10.0, 0, 0, 10., 10., 300, 300, ("d", "p"))


if not os.path.isdir(out_path("current","queries")):
    os.mkdir(out_path("current","queries"))
out_base = out_path("current","queries","xrayimage")
if not os.path.isdir(out_base):
    os.mkdir(out_base)

os.rename("output.png", out_path(out_base,"xrayimage00.png"))
Test("xrayimage00", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage01", s)

#
# Test a multi block structured grid with an array variable.
#
DefineScalarExpression("d1", 'recenter(d, "zonal")')
DefineScalarExpression("p1", 'recenter(p, "zonal")')

DefineArrayExpression("da", "array_compose(d1,d1)")
DefineArrayExpression("pa", "array_compose(p1,p1)")

DeleteAllPlots()

OpenDatabase(silo_data_path("multi_curv3d.silo"))

AddPlot("Pseudocolor", "d")
DrawPlots()

#create our own dictionary
params = dict(output_type=1, output_dir=".", divide_emis_by_absorb=1, origin=(0.0, 2.5, 10.0), up_vector=(0, 1, 0), theta=0, phi=0, width = 10., height=10., image_size=(300, 300), vars=("da", "pa"))
Query("XRay Image", params)

os.rename("output.00.png", out_path(out_base,"xrayimage02.png"))
os.rename("output.01.png", out_path(out_base,"xrayimage03.png"))

Test("xrayimage02", 0, 1)
Test("xrayimage03", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage04", s)

#
# Test a 2d structured grid with scalars.
#
DeleteAllPlots()

OpenDatabase(silo_data_path("curv2d.silo"))

AddPlot("Pseudocolor", "d")
DrawPlots()

#retreive default query parameters
params = GetQueryParameters("XRay Image")
#modify as necessary
params['image_size'] = (300, 300)
params['divide_emis_by_absorb'] = 1
params['width'] = 10.
params['height'] = 10.
params['vars'] = ("d", "p")
Query("XRay Image", params)

os.rename("output.png", out_path(out_base,"xrayimage05.png"))
Test("xrayimage05", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage06", s)

params['theta'] = 90
params['phi'] =  0
Query("XRay Image", params)

os.rename("output.png", out_path(out_base,"xrayimage07.png"))
Test("xrayimage07", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage08", s)

#
# Test an unstructured grid with scalars.
#
DefineScalarExpression("u1", 'recenter(((u+10.)*0.01), "zonal")')
DefineScalarExpression("v1", 'recenter(((v+10.)*0.01*matvf(mat1,1)), "zonal")')
DefineScalarExpression("v2", 'recenter(((v+10.)*0.01*matvf(mat1,2)), "zonal")')
DefineScalarExpression("v3", 'recenter(((v+10.)*0.01*matvf(mat1,3)), "zonal")')
DefineScalarExpression("v4", 'recenter(((v+10.)*0.01*matvf(mat1,4)), "zonal")')
DefineScalarExpression("w1", 'recenter(((w+10.)*0.01), "zonal")')

DeleteAllPlots()

OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Pseudocolor", "u")
DrawPlots()

# Do tets.
params['theta'] = 0
params['phi'] = 0
params['width'] = 1.
params['height'] = 1.
params['vars'] = ("w1", "v1")
Query("XRay Image", params)

os.rename("output.png", out_path(out_base,"xrayimage09.png"))
Test("xrayimage09", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage10", s)

params['theta'] = 90
params['width'] = 4.
params['height'] = 4.

Query("XRay Image", params)

os.rename("output.png", out_path(out_base,"xrayimage11.png"))
Test("xrayimage11", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage12", s)

# Do pyramids.
params['theta'] = 0
params['vars'] = ("w1", "v2")
Query("XRay Image", params) 

os.rename("output.png", out_path(out_base,"xrayimage13.png"))
Test("xrayimage13", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage14", s)

params['theta'] = 90
Query("XRay Image", params)

os.rename("output.png", out_path(out_base,"xrayimage15.png"))
Test("xrayimage15", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage16", s)

# Do wedges.
params['theta'] = 0
params['width'] = 8.
params['height'] = 8.
params['vars'] = ("w1", "v3")
Query("XRay Image", params)

os.rename("output.png", out_path(out_base,"xrayimage17.png"))
Test("xrayimage17", 0, 1)


s = GetQueryOutputString()
TestText("xrayimage18", s)

params['theta'] = 90
params['width'] = 20.
params['height'] = 20.
Query("XRay Image", params)

os.rename("output.png", out_path(out_base,"xrayimage19.png"))
Test("xrayimage19", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage20", s)

# Do hexes.
params['theta'] = 0
params['vars'] = ("w1", "v4")
Query("XRay Image", params)

os.rename("output.png", out_path(out_base,"xrayimage21.png"))
Test("xrayimage21", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage22", s)

params['theta'] = 90
Query("XRay Image", params)

os.rename("output.png", out_path(out_base,"xrayimage23.png"))
Test("xrayimage23", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage24", s)

#
# Test with perspective.
#
DeleteAllPlots()

OpenDatabase(silo_data_path("multi_curv3d.silo"))

AddPlot("Pseudocolor", "d")
DrawPlots()

params = dict(output_type="png", output_dir=".", divide_emis_by_absorb=1, focus=(0.0, 2.5, 15.0), view_up=(0., 1., 0.), normal=(0., 0., 1.), view_angle=30., parallel_scale = 16.0078, near_plane = -32.0156, far_plane = 32.0156, image_pan=(0., 0.), image_zoom = 2.4, perspective = 1, image_size=(300, 300), vars=("d", "p"))
Query("XRay Image", params)

os.rename("output.png", out_path(out_base,"xrayimage25.png"))

Test("xrayimage25", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage26", s)

#
# Test with background intensities.
#
DefineArrayExpression("wa", "array_compose(w1,w1)")
DefineArrayExpression("va", "array_compose(v4,v4)")

DeleteAllPlots()

OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Pseudocolor", "u")
DrawPlots()

params = dict(output_type="png", output_dir=".", divide_emis_by_absorb=1, theta=90., phi=0., width=20., height=20., image_size=(300, 300), vars=("wa", "va"), background_intensities=(0.05, 0.1))
Query("XRay Image", params)

os.rename("output.00.png", out_path(out_base,"xrayimage27.png"))
os.rename("output.01.png", out_path(out_base,"xrayimage28.png"))

Test("xrayimage27", 0, 1)
Test("xrayimage28", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage29", s)

DeleteAllPlots()

# 
# test setting output directory
# 
outdir_set = pjoin(TestEnv.params["run_dir"], "testdir")
if not os.path.isdir(outdir_set):
    os.mkdir(outdir_set)

OpenDatabase(silo_data_path("curv3d.silo"))

AddPlot("Pseudocolor", "d")
DrawPlots()

# old style argument passing
Query("XRay Image", "png", outdir_set, 1, 0.0, 2.5, 10.0, 0, 0, 10., 10., 300, 300, ("d", "p"))
os.rename(outdir_set + "/output.png", out_path(out_base, "xrayimage30.png"))
Test("xrayimage30", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage31", s)
DeleteAllPlots()
CloseDatabase(silo_data_path("curv3d.silo"))

#
# test blueprint output
#

conduit_dir_hdf5 = pjoin(outdir_set, "hdf5")
if not os.path.isdir(conduit_dir_hdf5):
    os.mkdir(conduit_dir_hdf5)
conduit_dir_json = pjoin(outdir_set, "json")
if not os.path.isdir(conduit_dir_json):
    os.mkdir(conduit_dir_json)
conduit_dir_yaml = pjoin(outdir_set, "yaml")
if not os.path.isdir(conduit_dir_yaml):
    os.mkdir(conduit_dir_yaml)
conduit_dir_imaging_planes0 = pjoin(outdir_set, "imaging_planes0")
if not os.path.isdir(conduit_dir_imaging_planes0):
    os.mkdir(conduit_dir_imaging_planes0)
conduit_dir_imaging_planes1 = pjoin(outdir_set, "imaging_planes1")
if not os.path.isdir(conduit_dir_imaging_planes1):
    os.mkdir(conduit_dir_imaging_planes1)
conduit_dir_detector_dims = pjoin(outdir_set, "detector_dims")
if not os.path.isdir(conduit_dir_detector_dims):
    os.mkdir(conduit_dir_detector_dims)

def setup_bp_test():
    OpenDatabase(silo_data_path("curv3d.silo"))
    AddPlot("Pseudocolor", "d")
    DrawPlots()

def test_bp_state_xray_view(testname, xrayout):
    normalx = xrayout["domain_000000/state/xray_view/normal/x"]
    normaly = xrayout["domain_000000/state/xray_view/normal/y"]
    normalz = xrayout["domain_000000/state/xray_view/normal/z"]
    TestValueEQ(testname + "_view_normal", [normalx, normaly, normalz], [0,0,1])
    
    focusx = xrayout["domain_000000/state/xray_view/focus/x"]
    focusy = xrayout["domain_000000/state/xray_view/focus/y"]
    focusz = xrayout["domain_000000/state/xray_view/focus/z"]
    TestValueEQ(testname + "_view_focus", [focusx, focusy, focusz], [0,2.5,10])
    
    view_upx = xrayout["domain_000000/state/xray_view/view_up/x"]
    view_upy = xrayout["domain_000000/state/xray_view/view_up/y"]
    view_upz = xrayout["domain_000000/state/xray_view/view_up/z"]
    TestValueEQ(testname + "_view_view_up", [view_upx, view_upy, view_upz], [0,1,0])
    
    view_angle = xrayout["domain_000000/state/xray_view/view_angle"]
    TestValueEQ(testname + "_view_view_angle", view_angle, 30)
    
    parallel_scale = xrayout["domain_000000/state/xray_view/parallel_scale"]
    TestValueEQ(testname + "_view_parallel_scale", parallel_scale, 5)
    
    near_plane = xrayout["domain_000000/state/xray_view/near_plane"]
    TestValueEQ(testname + "_view_near_plane", near_plane, -100)
    
    far_plane = xrayout["domain_000000/state/xray_view/far_plane"]
    TestValueEQ(testname + "_view_far_plane", far_plane, 100)
    
    image_panx = xrayout["domain_000000/state/xray_view/image_pan/x"]
    image_pany = xrayout["domain_000000/state/xray_view/image_pan/y"]
    TestValueEQ(testname + "_view_image_pan", [image_panx, image_pany], [0,0])
    
    image_zoom = xrayout["domain_000000/state/xray_view/image_zoom"]
    TestValueEQ(testname + "_view_image_zoom", image_zoom, 1)
    
    perspective = xrayout["domain_000000/state/xray_view/perspective"]
    TestValueEQ(testname + "_view_perspective", perspective, 0)

    perspective_str = xrayout["domain_000000/state/xray_view/perspective_str"]
    TestValueEQ(testname + "_view_perspective_str", perspective_str, "parallel")

UNITS_OFF = 0
UNITS_ON = 1

def test_bp_state_xray_query(testname, xrayout, units):
    divide_emis_by_absorb = xrayout["domain_000000/state/xray_query/divide_emis_by_absorb"]
    TestValueEQ(testname + "_query_divide_emis_by_absorb", divide_emis_by_absorb, 1)
    
    divide_emis_by_absorb_str = xrayout["domain_000000/state/xray_query/divide_emis_by_absorb_str"]
    TestValueEQ(testname + "_query_divide_emis_by_absorb_str", divide_emis_by_absorb_str, "yes")
    
    num_x_pixels = xrayout["domain_000000/state/xray_query/num_x_pixels"]
    TestValueEQ(testname + "_query_num_x_pixels", num_x_pixels, 300)
    
    num_y_pixels = xrayout["domain_000000/state/xray_query/num_y_pixels"]
    TestValueEQ(testname + "_query_num_y_pixels", num_y_pixels, 200)
    
    num_bins = xrayout["domain_000000/state/xray_query/num_bins"]
    TestValueEQ(testname + "_query_num_bins", num_bins, 1)
    
    abs_var_name = xrayout["domain_000000/state/xray_query/abs_var_name"]
    TestValueEQ(testname + "_query_abs_var_name", abs_var_name, "d")
    
    emis_var_name = xrayout["domain_000000/state/xray_query/emis_var_name"]
    TestValueEQ(testname + "_query_emis_var_name", emis_var_name, "p")

    abs_units = xrayout["domain_000000/state/xray_query/abs_units"]
    emis_units = xrayout["domain_000000/state/xray_query/emis_units"]

    if (units == UNITS_ON):
        TestValueEQ(testname + "_query_abs_units", abs_units, "abs units")
        TestValueEQ(testname + "_query_emis_units", emis_units, "emis units")
    else:
        TestValueEQ(testname + "_query_abs_units", abs_units, "no units provided")
        TestValueEQ(testname + "_query_emis_units", emis_units, "no units provided")

def test_bp_state_xray_data(testname, xrayout):
    detector_width = xrayout["domain_000000/state/xray_data/detector_width"]
    TestValueEQ(testname + "_data_detector_width", detector_width, 15)

    detector_height = xrayout["domain_000000/state/xray_data/detector_height"]
    TestValueEQ(testname + "_data_detector_height", detector_height, 10)
    
    intensity_max = xrayout["domain_000000/state/xray_data/intensity_max"]
    TestValueEQ(testname + "_data_intensity_max", intensity_max, 0.24153)
    
    intensity_min = xrayout["domain_000000/state/xray_data/intensity_min"]
    TestValueEQ(testname + "_data_intensity_min", intensity_min, 0)
    
    path_length_max = xrayout["domain_000000/state/xray_data/path_length_max"]
    TestValueEQ(testname + "_data_path_length_max", path_length_max, 148.67099)
    
    path_length_min = xrayout["domain_000000/state/xray_data/path_length_min"]
    TestValueEQ(testname + "_data_path_length_min", path_length_min, 0)

    image_topo_order_of_domain_variables = xrayout["domain_000000/state/xray_data/image_topo_order_of_domain_variables"]
    TestValueEQ(testname + "_data_image_topo_order_of_domain_variables", image_topo_order_of_domain_variables, "xyz")

NO_ENERGY_GROUP_BOUNDS = 0
ENERGY_GROUP_BOUNDS_MISMATCH = 1
ENERGY_GROUP_BOUNDS = 2

def test_bp_data(testname, conduit_db, bin_state = NO_ENERGY_GROUP_BOUNDS, units = UNITS_OFF):
    xrayout = conduit.Node()
    conduit.relay.io.blueprint.load_mesh(xrayout, conduit_db)

    # test metadata

    time = xrayout["domain_000000/state/time"]
    TestValueEQ(testname + "_Time", time, 4.8)
    
    cycle = xrayout["domain_000000/state/cycle"]
    TestValueEQ(testname + "_Cycle", cycle, 48)

    test_bp_state_xray_view(testname, xrayout)
    test_bp_state_xray_query(testname, xrayout, units)
    test_bp_state_xray_data(testname, xrayout)

    # test data embedded within the meshes

    intensityUnits = xrayout["domain_000000/fields/intensities/units"]
    pathLengthUnits = xrayout["domain_000000/fields/path_length/units"]

    if (units == UNITS_ON):
        TestValueEQ(testname + "_IntensityUnits", intensityUnits, "intensity units")
        TestValueEQ(testname + "_PathLengthUnits", pathLengthUnits, "path length metadata")
    else:
        TestValueEQ(testname + "_IntensityUnits", intensityUnits, "no units provided")
        TestValueEQ(testname + "_PathLengthUnits", pathLengthUnits, "no info provided")

    spatial_coords_x = xrayout["domain_000000/coordsets/spatial_coords/values/x"]
    spatial_coords_y = xrayout["domain_000000/coordsets/spatial_coords/values/y"]
    energy_group_bounds = xrayout["domain_000000/coordsets/spatial_coords/values/z"]
    energy_group_info = xrayout["domain_000000/coordsets/spatial_coords/info"]
    TestValueEQ(testname + "_data_SpatialExtents0", [spatial_coords_x[0], spatial_coords_y[0]], [0.0, 0.0])
    TestValueEQ(testname + "_data_SpatialExtents1", [spatial_coords_x[1], spatial_coords_y[1]], [0.05, 0.05])
    TestValueEQ(testname + "_data_SpatialExtents2", [spatial_coords_x[2], spatial_coords_y[2]], [0.1, 0.1])
    TestValueEQ(testname + "_data_SpatialExtents3", [spatial_coords_x[-1], spatial_coords_y[-1]], [15.0, 10.0])

    if (bin_state == NO_ENERGY_GROUP_BOUNDS):
        TestValueEQ(testname + "_data_EnergyGroupInfo", energy_group_info, "Energy group bounds not provided.")
        TestValueEQ(testname + "_data_EnergyGroupBounds", [energy_group_bounds[0], energy_group_bounds[1]], [0, 1])
    elif (bin_state == ENERGY_GROUP_BOUNDS_MISMATCH):
        baseline_string = "Energy group bounds size mismatch: provided 3 bounds, but 2 in query results."
        TestValueEQ(testname + "_data_EnergyGroupInfo", energy_group_info, baseline_string)
        TestValueEQ(testname + "_data_EnergyGroupBounds", [energy_group_bounds[0], energy_group_bounds[1]], [0, 1])
    elif (bin_state == ENERGY_GROUP_BOUNDS):
        TestValueEQ(testname + "_data_EnergyGroupBounds", [energy_group_bounds[0], energy_group_bounds[1]], [3.7, 4.2])

    xunits = xrayout["domain_000000/coordsets/spatial_coords/units/x"]
    yunits = xrayout["domain_000000/coordsets/spatial_coords/units/y"]
    zunits = xrayout["domain_000000/coordsets/spatial_coords/units/z"]

    if (units == UNITS_ON):
        TestValueEQ(testname + "_data_XUnits", xunits, "cm")
        TestValueEQ(testname + "_data_YUnits", yunits, "cm")
        TestValueEQ(testname + "_data_ZUnits", zunits, "kev")
    else:
        TestValueEQ(testname + "_data_XUnits", xunits, "no units provided")
        TestValueEQ(testname + "_data_YUnits", yunits, "no units provided")
        TestValueEQ(testname + "_data_ZUnits", zunits, "no units provided")

    xlabel = xrayout["domain_000000/coordsets/spatial_coords/labels/x"];
    ylabel = xrayout["domain_000000/coordsets/spatial_coords/labels/y"];
    zlabel = xrayout["domain_000000/coordsets/spatial_coords/labels/z"];
    TestValueEQ(testname + "_data_XLabels", xlabel, "width")
    TestValueEQ(testname + "_data_YLabels", ylabel, "height")
    TestValueEQ(testname + "_data_ZLabels", zlabel, "energy_group")

    xlabel = xrayout["domain_000000/coordsets/spatial_energy_reduced_coords/labels/x"];
    ylabel = xrayout["domain_000000/coordsets/spatial_energy_reduced_coords/labels/y"];
    TestValueEQ(testname + "_data_ser_XLabels", xlabel, "width")
    TestValueEQ(testname + "_data_ser_YLabels", ylabel, "height")

def blueprint_test(output_type, outdir, testtextnumber, testname, hdf5 = False):
    for i in range(0, 2):
        setup_bp_test()

        # run query and test the output message
        if (i == 0):
            Query("XRay Image", output_type, outdir, 1, 0.0, 2.5, 10.0, 0, 0, 10., 10., 300, 200, ("d", "p"))
        elif (i == 1):
            params = dict() # GetQueryParameters("XRay Image")
            params["output_type"] = output_type
            params["output_dir"] = outdir
            params["divide_emis_by_absorb"] = 1;
            params["origin"] = (0.0, 2.5, 10.0);
            params["theta"] = 0;
            params["phi"] = 0;
            params["width"] = 10.;
            params["height"] = 10.;
            params["image_size"] = (300, 200)
            params["vars"] = ("d", "p")
            params["spatial_units"] = "cm";
            params["energy_units"] = "kev";
            params["abs_units"] = "abs units";
            params["emis_units"] = "emis units";
            params["intensity_units"] = "intensity units";
            params["path_length_info"] = "path length metadata";
            Query("XRay Image", params)
        s = GetQueryOutputString()
        TestText("xrayimage" + str(testtextnumber + i), s)
        DeleteAllPlots()
        CloseDatabase(silo_data_path("curv3d.silo"))

        # test opening the bp output and visualizing in visit
        conduit_db = pjoin(outdir, "output.root")
        OpenDatabase(conduit_db)
        AddPlot("Pseudocolor", "mesh_image_topo/intensities")
        DrawPlots()
        Test(testname + "_image_topo_intensities" + str(i))
        DeleteAllPlots()

        AddPlot("Pseudocolor", "mesh_image_topo/path_length")
        DrawPlots()
        Test(testname + "_image_topo_path_length" + str(i))
        DeleteAllPlots()

        AddPlot("Pseudocolor", "mesh_spatial_topo/intensities_spatial")
        DrawPlots()
        Test(testname + "_spatial_topo_intensities" + str(i))
        DeleteAllPlots()

        AddPlot("Pseudocolor", "mesh_spatial_topo/path_length_spatial")
        DrawPlots()
        Test(testname + "_spatial_topo_path_length" + str(i))
        DeleteAllPlots()

        AddPlot("Pseudocolor", "mesh_spatial_energy_reduced_topo/intensities_spatial_energy_reduced")
        DrawPlots()
        Test(testname + "_spatial_energy_reduced_topo_intensities" + str(i))
        DeleteAllPlots()

        AddPlot("Pseudocolor", "mesh_spatial_energy_reduced_topo/path_length_spatial_energy_reduced")
        DrawPlots()
        Test(testname + "_spatial_energy_reduced_topo_path_length" + str(i))
        DeleteAllPlots()

        CloseDatabase(conduit_db)

    if (hdf5):
        units = UNITS_OFF if i == 0 else UNITS_ON
        test_bp_data(testname + str(i), conduit_db, bin_state=NO_ENERGY_GROUP_BOUNDS, units=units) # no bounds
        setup_bp_test()
        Query("XRay Image", output_type, outdir, 1, 0.0, 2.5, 10.0, 0, 0, 10., 10., 300, 200, ("d", "p"), [1,2,3])
        test_bp_data(testname + str(i), conduit_db, bin_state=ENERGY_GROUP_BOUNDS_MISMATCH, units=UNITS_OFF) # bounds mismatch
        Query("XRay Image", output_type, outdir, 1, 0.0, 2.5, 10.0, 0, 0, 10., 10., 300, 200, ("d", "p"), [3.7, 4.2])
        test_bp_data(testname + str(i), conduit_db, bin_state=ENERGY_GROUP_BOUNDS, units=UNITS_OFF) # bounds
        DeleteAllPlots()
        CloseDatabase(silo_data_path("curv3d.silo"))

blueprint_test("hdf5", conduit_dir_hdf5, 32, "Blueprint_HDF5_X_Ray_Output", hdf5=True)
blueprint_test("json", conduit_dir_json, 34, "Blueprint_JSON_X_Ray_Output", hdf5=False)
blueprint_test("yaml", conduit_dir_yaml, 36, "Blueprint_YAML_X_Ray_Output", hdf5=False)

# test detector height and width are always positive in blueprint output

setup_bp_test()

params = dict()
params["image_size"] = (400, 300)
params["output_type"] = "hdf5"
params["output_dir"] = conduit_dir_detector_dims
params["focus"] = (0., 2.5, 10.)
params["perspective"] = 1
params["near_plane"] = -50.
params["far_plane"] = 50.
params["vars"] = ("d", "p")
params["energy_group_bounds"] = [3.7, 4.2];
params["parallel_scale"] = 5.
Query("XRay Image", params)

DeleteAllPlots()
CloseDatabase(silo_data_path("curv3d.silo"))

conduit_db = pjoin(conduit_dir_detector_dims, "output.root")
xrayout = conduit.Node()
conduit.relay.io.blueprint.load_mesh(xrayout, conduit_db)

detector_width = xrayout["domain_000000/state/xray_data/detector_width"]
TestValueEQ("Blueprint_Positive_Detector_width", detector_width, 22.3932263237838)

detector_height = xrayout["domain_000000/state/xray_data/detector_height"]
TestValueEQ("Blueprint_Positive_Detector_height", detector_height, 16.7949192423103)

# test imaging plane topos and ray output

def test_imaging_planes_and_rays():
    for i in range(0, 2):
        setup_bp_test()

        params = dict()
        params["image_size"] = (400, 300)
        params["output_dir"] = conduit_dir_imaging_planes0 if i == 0 else conduit_dir_imaging_planes1
        params["output_type"] = "hdf5"
        params["focus"] = (0., 2.5, 10.)
        params["perspective"] = 1
        params["near_plane"] = -50.
        params["far_plane"] = 50.
        params["vars"] = ("d", "p")
        params["parallel_scale"] = 5.
        Query("XRay Image", params)

        conduit_db = pjoin(conduit_dir_imaging_planes0 if i == 0 else conduit_dir_imaging_planes1, "output.root")

        OpenDatabase(conduit_db)

        AddPlot("Pseudocolor", "mesh_far_plane_topo/far_plane_field", 1, 1)
        AddPlot("Pseudocolor", "mesh_view_plane_topo/view_plane_field", 1, 1)
        AddPlot("Pseudocolor", "mesh_near_plane_topo/near_plane_field", 1, 1)
        if i == 0:
            AddPlot("Pseudocolor", "mesh_ray_corners_topo/ray_corners_field", 1, 1)
        else:
            AddPlot("Pseudocolor", "mesh_ray_topo/ray_field", 1, 1)
        DrawPlots()

        SetActivePlots(4)
        PseudocolorAtts = PseudocolorAttributes()
        PseudocolorAtts.invertColorTable = 1
        SetPlotOptions(PseudocolorAtts)

        View3DAtts = View3DAttributes()
        View3DAtts.viewNormal = (-0.519145, 0.199692, -0.831031)
        View3DAtts.focus = (0, 2.5, 10)
        View3DAtts.viewUp = (-0.0954901, 0.952683, 0.288577)
        View3DAtts.viewAngle = 30
        View3DAtts.parallelScale = 58.6531
        View3DAtts.nearPlane = -117.306
        View3DAtts.farPlane = 117.306
        SetView3D(View3DAtts)

        Test("Blueprint_HDF5_Imaging_Planes" + str(i))

        DeleteAllPlots()
        CloseDatabase(silo_data_path("curv3d.silo"))
        CloseDatabase(conduit_db)

test_imaging_planes_and_rays()

# 
# test catching failures
# 

# write to dir that does not exist

dir_dne = pjoin(outdir_set, "doesnotexist")
if os.path.isdir(dir_dne):
    os.rmdir(dir_dne)

setup_bp_test()

Query("XRay Image", "hdf5", dir_dne, 1, 0.0, 2.5, 10.0, 0, 0, 10., 10., 300, 300, ("d", "p"))
s = GetQueryOutputString()
TestText("xrayimage38", s)
DeleteAllPlots()
CloseDatabase(silo_data_path("curv3d.silo"))

# os.chmod does not work on windows
if not platform.system() == "Windows":
    # write to dir w/ read only permissions

    outdir_bad = pjoin(outdir_set, "baddir")
    if not os.path.isdir(outdir_bad):
        os.mkdir(outdir_bad)
    os.chmod(outdir_bad, 0o444)

    OpenDatabase(silo_data_path("curv3d.silo"))
    AddPlot("Pseudocolor", "d")
    DrawPlots()

    Query("XRay Image", "hdf5", outdir_bad, 1, 0.0, 2.5, 10.0, 0, 0, 10., 10., 300, 300, ("d", "p"))
    s = GetQueryOutputString()
    # strip out two lines that make the test machine dependent
    s = '\n'.join([line if line[:4] != "file" else '' for line in s.split('\n')])
    s = '\n'.join([line if line[:4] != "line" else '' for line in s.split('\n')])
    TestText("xrayimage39", s)
    DeleteAllPlots()
    CloseDatabase(silo_data_path("curv3d.silo"))

# 
# Test filenames and output types
# 

setup_bp_test()

DefineScalarExpression("d1", 'recenter(d, "zonal")')
DefineScalarExpression("p1", 'recenter(p, "zonal")')

DefineArrayExpression("da", "array_compose(d1,d1)")
DefineArrayExpression("pa", "array_compose(p1,p1)")

def query_variety(otype, scheme, thevars, outdir):
    SetQueryFloatFormat("%g")
    Query("XRay Image", 
        background_intensity=0, 
        divide_emis_by_absorb=0, 
        far_plane=20, 
        filename_scheme=scheme, 
        family_files=0, # this is to test that family_files is ignored when filename_scheme is set
        focus=(0, 0, 0), 
        image_pan=(0, 0), 
        image_size=(3, 3), 
        image_zoom=1, 
        near_plane=-20, 
        normal=(0, 0, 1), 
        output_dir=outdir, 
        output_ray_bounds=0, 
        output_type=otype, 
        parallel_scale=10, 
        perspective=0, 
        view_angle=30, 
        view_up=(0, 1, 0), 
        vars=thevars)
    return GetQueryOutputString()

def query_family_backwards_compat(family, thevars, outdir):
    SetQueryFloatFormat("%g")
    Query("XRay Image", 
        background_intensity=0, 
        divide_emis_by_absorb=0, 
        far_plane=20, 
        family_files=family,
        focus=(0, 0, 0), 
        image_pan=(0, 0), 
        image_size=(3, 3), 
        image_zoom=1, 
        near_plane=-20, 
        normal=(0, 0, 1), 
        output_dir=outdir, 
        output_ray_bounds=0, 
        output_type="png", 
        parallel_scale=10, 
        perspective=0, 
        view_angle=30, 
        view_up=(0, 1, 0), 
        vars=thevars)
    return GetQueryOutputString()

output_types = ["jpeg", "png", "tif", "bof", "bov", "json", "hdf5", "yaml"]
filename_schemes = ["family", "family", "cycle", "none"]
family_options = [0, 1]
vars_options = [("d", "p"), ("da", "pa")]

info = ""
for i in range(0, len(output_types)):
    outdir_set_otype = outdir_set + "_" + output_types[i]
    if not os.path.isdir(outdir_set_otype):
        os.mkdir(outdir_set_otype)
    if output_types[i] == "jpeg":
        # create a dummy file to test the file familying
        open(outdir_set_otype + "/output.0000.jpg", 'w').close()
    info = ""
    for j in range(0, len(filename_schemes)):
        for k in range(0, len(vars_options)):
            info += query_variety(output_types[i], filename_schemes[j], vars_options[k], outdir_set_otype)
    info += str(os.listdir(outdir_set_otype))
    TestText("Test_filenames_for_" + output_types[i] + "_outputs", info)

# test backwards compatibility with family_files option
for i in range(0, len(family_options)):
    outdir_set_family = outdir_set + "_family_" + str(family_options[i])
    if not os.path.isdir(outdir_set_family):
        os.mkdir(outdir_set_family)
    info = ""
    for j in range(0, len(vars_options)):
        info += query_family_backwards_compat(family_options[i], vars_options[j], outdir_set_family)
    info += str(os.listdir(outdir_set_family))
    TestText("Test_filenames_for_family" + str(family_options[i]) + "_outputs", info)

#
# Test that we get decent error messages for common cases
#
import numpy

nporig = numpy.array([0.0, 2.5, 10.0])

params = dict(output_type=1, output_dir=".", divide_emis_by_absorb=1, \
    origin=nporig, up_vector=(0, 1, 0), theta=0, phi=0, \
    width = 10., height=10., image_size=(300, 300), vars=("da", "pa"))
try:
    Query("XRay Image", params)
except (visit.VisItException, VisItException) as e:
    if '"origin"' in e.args[0] and "position 4" in e.args[0] and "type numpy.ndarray" in e.args[0]:
        TestPOA('detect and warn numpy array as query param')
    else:
        TestFOA('detect and warn numpy array as query param', LINE())
    pass
except:
    TestFOA('detect and warn numpy array as query param', LINE())
    pass

params = dict(output_type=1, output_dir=".", divide_emis_by_absorb=1, \
    origin=nporig.tolist(), up_vector=(0, 1, 0), theta=0, phi=0, \
    width = 10., height=10., image_size=(300, 300), vars=("da", "pa"))
try:
    Query("XRay Image", params)
    TestPOA('numpy array converted to list works as query param')
except:
    TestFOA('numpy array converted to list works as query param', LINE())
    pass

Exit()
