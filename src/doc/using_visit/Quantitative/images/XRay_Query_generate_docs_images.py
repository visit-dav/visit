import conduit
import os
import sys

path_to_data_file = "../" # CHANGE ME
data_file = path_to_data_file + "testdata/silo_hdf5_test_data/curv3d.silo"

shouldisave = True

IMAGE_VIEW       = 0
SPATIAL_VIEW     = 1
ALT_SPATIAL_VIEW = 2
SIM_VIEW         = 3
ALT_SIM_VIEW     = 4
SIDE_VIEW        = 5
ALT_SIDE_VIEW    = 6

image_num = 0

def save_image(name):
	global image_num
	if shouldisave:
		image_filename = "visit" + str(image_num).zfill(4) + ".png"
		while os.path.exists(image_filename):
			image_num += 1
			image_filename = "visit" + str(image_num).zfill(4) + ".png"
		else:
			SaveWindow()
			new_filename = "XRay_Query_" + name + ".png"
			os.rename(image_filename, new_filename)
			print("Renamed " + image_filename + " to " + new_filename)
			image_num += 1

def set_view(view):
	View3DAtts = View3DAttributes()
	if view == IMAGE_VIEW:
		View3DAtts.viewNormal = (0.616352, 0.284876, 0.734136)
		View3DAtts.focus = (200, 150, 1.5)
		View3DAtts.viewUp = (0.202105, 0.95827, 0.202169)
		View3DAtts.parallelScale = 250
		View3DAtts.nearPlane = -500
		View3DAtts.farPlane = 500
		View3DAtts.centerOfRotation = (200, 150, 1.5)
		View3DAtts.windowValid = 1
	elif view == SPATIAL_VIEW:
		View3DAtts.viewNormal = (0.616352, 0.284876, 0.734136)
		View3DAtts.focus = (4.5, 3.3, 2)
		View3DAtts.viewUp = (0.202105, 0.95827, 0.202169)
		View3DAtts.parallelScale = 5.7
		View3DAtts.nearPlane = -11
		View3DAtts.farPlane = 11
		View3DAtts.centerOfRotation = (4.5, 3.3, 2)
		View3DAtts.windowValid = 1
	elif view == ALT_SPATIAL_VIEW:
		View3DAtts.viewNormal = (0.616352, 0.284876, 0.734136)
		View3DAtts.focus = (8, 6, 2)
		View3DAtts.viewUp = (0.202105, 0.95827, 0.202169)
		View3DAtts.parallelScale = 10
		View3DAtts.nearPlane = -20
		View3DAtts.farPlane = 20
		View3DAtts.centerOfRotation = (8, 6, 2)
		View3DAtts.windowValid = 1
	elif view == SIM_VIEW:
		View3DAtts.viewNormal = (-0.616352, 0.284876, -0.734136)
		View3DAtts.focus = (0, 2.5, 10)
		View3DAtts.viewUp = (0.202105, 0.95827, 0.202169)
		View3DAtts.parallelScale = 37.4109
		View3DAtts.nearPlane = -74.8218
		View3DAtts.farPlane = 74.8218
		View3DAtts.centerOfRotation = (0, 2.5, 10)
		View3DAtts.windowValid = 1
	elif view == ALT_SIM_VIEW:
		View3DAtts.viewNormal = (-0.478743, 0.181535, 0.858982)
		View3DAtts.focus = (0, 2.5, 10)
		View3DAtts.viewUp = (-0.127902, 0.953531, -0.272801)
		View3DAtts.parallelScale = 58.6531
		View3DAtts.nearPlane = -117.306
		View3DAtts.farPlane = 117.306
		View3DAtts.centerOfRotation = (0, 2.5, 10)
		View3DAtts.windowValid = 1
	elif view == SIDE_VIEW:
		View3DAtts.viewNormal = (0.478743, 0.181535, -0.858982)
		View3DAtts.focus = (0, 2.5, 10)
		View3DAtts.viewUp = (-0.127902, 0.953531, -0.272801)
		View3DAtts.parallelScale = 58.6531
		View3DAtts.nearPlane = -117.306
		View3DAtts.farPlane = 117.306
		View3DAtts.imageZoom = 1.4641
	elif view == ALT_SIDE_VIEW:
		View3DAtts.viewNormal = (0.840, 0.383, -0.385)
		View3DAtts.focus = (0, 2.5, 15)
		View3DAtts.viewUp = (-0.0651, 0.775, 0.628)
		View3DAtts.parallelScale = 20
		View3DAtts.nearPlane = -15
		View3DAtts.farPlane = 15
	else:
		print("invalid view selected. Setting view to default")
	SetView3D(View3DAtts)

def turn_off_annotations():
	# Logging for SetAnnotationObjectOptions is not implemented yet.
	AnnotationAtts = AnnotationAttributes()
	AnnotationAtts.axes2D.visible = 0
	AnnotationAtts.axes3D.visible = 0
	AnnotationAtts.axes3D.triadFlag = 0
	AnnotationAtts.axes3D.bboxFlag = 0
	AnnotationAtts.userInfoFlag = 0
	AnnotationAtts.legendInfoFlag = 0
	AnnotationAtts.databaseInfoFlag = 0
	SetAnnotationAttributes(AnnotationAtts)

def turn_on_2d_annotations():
	AnnotationAtts = GetAnnotationAttributes()
	AnnotationAtts.axes2D.visible = 1
	SetAnnotationAttributes(AnnotationAtts)

def visualize_imaging_planes():
	AddPlot("Pseudocolor", "mesh_near_plane_topo/near_plane_field")
	AddPlot("Pseudocolor", "mesh_view_plane_topo/view_plane_field")
	AddPlot("Pseudocolor", "mesh_far_plane_topo/far_plane_field")
	DrawPlots()

def color_imaging_planes():
	# Make the plot of the near plane active
	SetActivePlots(1)
	PseudocolorAtts = PseudocolorAttributes()
	# We invert the color table so that it is a different color from the far plane
	PseudocolorAtts.invertColorTable = 1
	SetPlotOptions(PseudocolorAtts)

	# Make the plot of the view plane active
	SetActivePlots(2)
	PseudocolorAtts = PseudocolorAttributes()
	PseudocolorAtts.colorTableName = "Oranges"
	PseudocolorAtts.invertColorTable = 1
	PseudocolorAtts.opacityType = PseudocolorAtts.Constant  # ColorTable, FullyOpaque, Constant, Ramp, VariableRange
	# We lower the opacity so that the view plane does not obstruct our view of anything.
	PseudocolorAtts.opacity = 0.7
	SetPlotOptions(PseudocolorAtts)

	# leave the far plane as is

def visualize_ray_field():
	AddPlot("Pseudocolor", "mesh_ray_topo/ray_field")
	DrawPlots()

def adjust_opacity():
	PseudocolorAtts = PseudocolorAttributes()
	PseudocolorAtts.opacityType = PseudocolorAtts.Constant  # ColorTable, FullyOpaque, Constant, Ramp, VariableRange
	PseudocolorAtts.opacity = 0.5
	SetPlotOptions(PseudocolorAtts)

def visualize_ray_corners():
	AddPlot("Mesh", "mesh_ray_corners_topo")
	DrawPlots()
	MeshAtts = MeshAttributes()
	MeshAtts.lineWidth = 1
	SetPlotOptions(MeshAtts)

def imaging_planes_and_rays(res, direction):
	view = SIM_VIEW
	if direction == "side":
		view = ALT_SIM_VIEW

	visualize_imaging_planes()
	set_view(view)
	save_image("imaging_planes_uncolored_" + res + "_" + direction)

	color_imaging_planes()
	save_image("imaging_planes_" + res + "_" + direction)

	visualize_ray_corners()
	save_image("imaging_planes_and_ray_corners_" + res + "_" + direction)

	# hide ray corners
	SetActivePlots(4)
	HideActivePlots()

	visualize_ray_field()
	save_image("imaging_planes_and_rays_" + res + "_" + direction)

	# show ray corners
	SetActivePlots(4)
	HideActivePlots()
	save_image("imaging_planes_rays_and_ray_corners" + res + "_" + direction)
	# hide ray corners
	SetActivePlots(4)
	HideActivePlots()

	SetActivePlots(5)
	adjust_opacity()
	save_image("imaging_planes_and_transparent_rays_" + res + "_" + direction)

	# show ray corners
	SetActivePlots(4)
	HideActivePlots()
	save_image("imaging_planes_transparent_rays_and_ray_corners_" + res + "_" + direction)

	DeleteAllPlots()

# whichone is either "intensities" or "path_length"
def visualize_image_topo(whichone):
	ResetView()
	AddPlot("Pseudocolor", "mesh_image_topo/" + whichone)
	DrawPlots()
	set_view(IMAGE_VIEW)

def change_to_xray():
	# Make sure the plot you want to change the color of is active
	PseudocolorAtts = PseudocolorAttributes()
	PseudocolorAtts.colorTableName = "xray"
	SetPlotOptions(PseudocolorAtts)

# whichone is either "intensities" or "path_length"
def visualize_spatial_topo(whichone, direction):
	view = SPATIAL_VIEW
	if direction == "side":
		view = ALT_SPATIAL_VIEW
	ResetView()
	AddPlot("Pseudocolor", "mesh_spatial_topo/" + whichone + "_spatial")
	DrawPlots()
	set_view(view)

def visualize_spatial_energy_reduced_topo(whichone):
	ResetView()
	AddPlot("Pseudocolor", "mesh_spatial_energy_reduced_topo/" + whichone + "_spatial_energy_reduced")
	DrawPlots()

def visualize_spectra_curves(whichone):
	ResetView()
	AddPlot("Curve", "mesh_spectra_topo/" + whichone + "_spectra")
	CurveAtts = CurveAttributes()
	CurveAtts.showLegend = 0
	CurveAtts.showLabels = 0
	SetPlotOptions(CurveAtts)
	DrawPlots()

def bonus_topos(direction):
	turn_on_2d_annotations() # we want these plots to have axes and labels

	# spatial energy reduced
	visualize_spatial_energy_reduced_topo("intensities")
	save_image("spatial_energy_reduced_intensities_" + direction)
	change_to_xray()
	save_image("spatial_energy_reduced_intensities_xray_" + direction)
	DeleteAllPlots() # Make sure we have a clean slate for ensuing visualizations.
	visualize_spatial_energy_reduced_topo("path_length")
	save_image("spatial_energy_reduced_path_length_" + direction)
	change_to_xray()
	save_image("spatial_energy_reduced_path_length_xray_" + direction)
	DeleteAllPlots() # Make sure we have a clean slate for ensuing visualizations.

	# spectra curves
	visualize_spectra_curves("intensities")
	save_image("spectra_intensities_" + direction)
	DeleteAllPlots() # Make sure we have a clean slate for ensuing visualizations.
	visualize_spectra_curves("path_length")
	save_image("spectra_path_length_" + direction)
	DeleteAllPlots() # Make sure we have a clean slate for ensuing visualizations.

	turn_off_annotations()

def image_topos(direction):
	# image topo fields
	visualize_image_topo("intensities")
	save_image("image_intensities_" + direction)
	change_to_xray()
	save_image("image_intensities_xray_" + direction)
	DeleteAllPlots() # Make sure we have a clean slate for ensuing visualizations.
	visualize_image_topo("path_length")
	save_image("image_path_length_" + direction)
	change_to_xray()
	save_image("image_path_length_xray_" + direction)
	DeleteAllPlots() # Make sure we have a clean slate for ensuing visualizations.
	
	# spatial topo fields
	visualize_spatial_topo("intensities", direction)
	save_image("spatial_intensities_" + direction)
	change_to_xray()
	save_image("spatial_intensities_xray_" + direction)
	DeleteAllPlots() # Make sure we have a clean slate for ensuing visualizations.
	visualize_spatial_topo("path_length", direction)
	save_image("spatial_path_length_" + direction)
	change_to_xray()
	save_image("spatial_path_length_xray_" + direction)
	DeleteAllPlots() # Make sure we have a clean slate for ensuing visualizations.

def setup():
	OpenDatabase(data_file)

	AddPlot("Pseudocolor", "d")
	DrawPlots()

	DefineScalarExpression("d1", 'd')
	DefineScalarExpression("p1", 'p')
	DefineScalarExpression("d2", 'd1 * 2')
	DefineScalarExpression("p2", 'p1 * 2')
	DefineScalarExpression("d3", 'd1 * 4')
	DefineScalarExpression("p3", 'p1 * 4')
	DefineScalarExpression("d4", 'd1 * 3')
	DefineScalarExpression("p4", 'p1 * 3')
	# DefineScalarExpression("d5", 'd1 * 8')
	# DefineScalarExpression("p5", 'p1 * 8')
	# DefineScalarExpression("d6", 'd1 * 2')
	# DefineScalarExpression("p6", 'p1 * 2')
	DefineArrayExpression("da", "array_compose(d1,d2,d3,d4)")
	DefineArrayExpression("pa", "array_compose(p1,p2,p3,p4)")
	# DefineArrayExpression("da", "array_compose(d1,d2,d3,d4,d5,d6)")
	# DefineArrayExpression("pa", "array_compose(p1,p2,p3,p4,p5,p6)")

def call_query(image_size = (400, 300), side_view = False):
	# params = GetQueryParameters("XRay Image")
	params = dict()

	# standard args
	params["vars"] = ("da", "pa")
	# params["background_intensity"] = 0
	# params["background_intensities"] = 0
	# params["divide_emis_by_absorb"] = 0
	params["image_size"] = image_size
	# params["debug_ray"] = -1
	# params["output_ray_bounds"] = 0
	params["energy_group_bounds"] = [0.0, 2.0, 6.0, 8.0, 9.0, 10.0, 12.0]

	# filename, directory, and output type choices
	params["output_dir"] = "."
	params["filename_scheme"] = "family" # "none", "family", or "cycle" 
	params["output_type"] = "hdf5"

	# units
	params["spatial_units"] = "cm"
	params["energy_units"] = "kev"
	params["abs_units"] = "cm^2/g"
	params["emis_units"] = "GJ/cm^2/ster/ns/keV"
	params["intensity_units"] = "intensity units"
	params["path_length_info"] = "path length metadata"

	# # simple camera spec
	# params["width"] = 1
	# params["height"] = 1
	# params["origin"] = (0., 2.5, 10.)
	# params["theta"] = 0
	# params["phi"] = 0
	# params["up_vector"] = (0, 1, 0)

	# complete camera spec - look straight on
	# params["normal"] = (0,0,1)
	params["focus"] = (0., 2.5, 10.)
	# params["view_up"] = (0,1,0)
	# params["view_angle"] = 30
	params["parallel_scale"] = 10.
	params["near_plane"] = -25.
	params["far_plane"] = 25.
	# params["image_pan"] = (0,0)
	# params["image_zoom"] = 1
	params["perspective"] = 1 # 0 parallel, 1 perspective

	if side_view:
		# tweak params so look from the side
		params['normal'] = (0.840, 0.383, -0.385)
		params['view_up'] = (-0.0651, 0.775, 0.628)
		params["near_plane"] = -15.
		params["far_plane"] = 15.

	Query("XRay Image", params)

def fetch_coordset_dims(data):
    cset_vals = data.fetch_existing("coordsets/image_coords/values")
    # fetch rectilinear coordset dims
    nx = cset_vals.fetch_existing("x").dtype().number_of_elements()
    ny = cset_vals.fetch_existing("y").dtype().number_of_elements()
    nz = cset_vals.fetch_existing("z").dtype().number_of_elements()
    return nx, ny, nz

def fetch_topology_dims(data):
    # rectilinear topology have one less element in each 
    # logical dim than the coordset
    cnx, cny, cnz = fetch_coordset_dims(data)
    return cnx-1, cny-1, cnz-1

def fetch_reshaped_field_values(data,field_name):
    nx, ny, nz = fetch_topology_dims(data)
    vals = data.fetch_existing("fields/" + field_name + "/values").value()
    return vals.reshape(nz,ny,nx)

def test_reshape():
	# read xray blueprint output
	n = conduit.Node()
	conduit.relay.io.blueprint.load_mesh(n,"output.0000.root")
	# the node with our data is domain 0 -- or the first child
	data = n[0]
	intensity_vals = fetch_reshaped_field_values(data,"intensities")

	print(data["fields/intensities/values"])
	print(intensity_vals.shape)
	print(intensity_vals)

def conduit_processing():
	xrayout = conduit.Node()

	conduit.relay.io.blueprint.load_mesh(xrayout, "output.0000.root")

	print(xrayout["domain_000000"])
	# print(xrayout["domain_000000"].to_yaml())

	# test_reshape()

def run_imaging_planes_and_rays(res, file, direction):
	# Make sure that the input mesh is visible
	ActivateDatabase(data_file)
	AddPlot("Pseudocolor", "d")
	DrawPlots()
	# Then we want to go back to the output file and visualize the imaging planes
	OpenDatabase(file) # low-res data
	imaging_planes_and_rays(res, direction)
	CloseDatabase(file)

##########################

def main():
	setup()
	turn_off_annotations()
	call_query(image_size = (400, 300))
	out_400x300 = "output.0000.root"
	
	# generate low res data so it is easy to visualize rays
	call_query(image_size = (40, 30))
	out_40x30 = "output.0001.root"
	call_query(image_size = (20, 15))
	out_20x15 = "output.0002.root"
	call_query(image_size = (8, 6))
	out_8x6 = "output.0003.root"
	# we won't need to make multiple resolution datasets anymore once the index select operator works properly
	# https://github.com/visit-dav/visit/issues/18422

	# generate two side views
	call_query(image_size = (400, 300), side_view = True)
	out_side_400x300 = "output.0004.root"
	call_query(image_size = (40, 30), side_view = True)
	out_side_40x30 = "output.0005.root"

	DeleteAllPlots()

	# conduit_processing()
	
	# generate hi-res image and bonus topo images for FRONT view
	OpenDatabase(out_400x300)
	image_topos(direction = "front")
	bonus_topos(direction = "front")
	CloseDatabase(out_400x300)
	
	# generate hi-res image and bonus topo images for SIDE view
	OpenDatabase(out_side_400x300)
	image_topos(direction = "side")
	bonus_topos(direction = "side")
	CloseDatabase(out_side_400x300)
	
	# generate input mesh images for all views
	ActivateDatabase(data_file)
	AddPlot("Pseudocolor", "d")
	DrawPlots()
	save_image("input_mesh_front")
	set_view(SIDE_VIEW)
	save_image("input_mesh_side")
	set_view(SIM_VIEW)
	save_image("input_mesh_sim")
	set_view(ALT_SIDE_VIEW)
	save_image("input_mesh_alt_side")
	DeleteAllPlots()

	# generate imaging planes and rays images for multiple resolutions for FRONT view
	run_imaging_planes_and_rays(res = "400x300", file = out_400x300, direction = "front")
	run_imaging_planes_and_rays(res = "40x30", file = out_40x30, direction = "front")
	run_imaging_planes_and_rays(res = "20x15", file = out_20x15, direction = "front")
	run_imaging_planes_and_rays(res = "8x6", file = out_8x6, direction = "front")

	# generate imaging planes and rays images for multiple resolutions for SIDE view
	run_imaging_planes_and_rays(res = "400x300", file = out_side_400x300, direction = "side")
	run_imaging_planes_and_rays(res = "40x30", file = out_side_40x30, direction = "side")

main()
