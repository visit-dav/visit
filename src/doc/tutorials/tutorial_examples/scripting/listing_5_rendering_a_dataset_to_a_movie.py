###########################################
# file: VisIt Scripting Tutorial Listing 5
###########################################
#
# 1) Open VisIt
# 2) Open 'aneurysm.visit'
# 3) Paste and execute via the Commands window, or the CLI.
#

# import visit_utils, we will use it to help encode our movie
from visit_utils import *
DeleteAllPlots()

AddPlot("Pseudocolor","pressure")
DrawPlots()
 
# Set a better view
ResetView()
v = GetView3D()
v.RotateAxis(1,90)
SetView3D(v)
 
# get the number of timesteps
nts = TimeSliderGetNStates()
 
# set basic save options
swatts = SaveWindowAttributes()
#
# The 'family' option controls if visit automatically adds a frame number to 
# the rendered files. For this example we will explicitly manage the output name.
#
swatts.family = 0
#
# select PNG as the output file format
#
swatts.format = swatts.PNG 
#
# set the width of the output image
#
swatts.width = 1024 
#
# set the height of the output image
#
swatts.height = 1024

#the encoder expects file names with an integer sequence
# 0,1,2,3 .... N-1

file_idx = 0

for ts in range(0,nts,10): # look at every 10th frame
    # Change to the next timestep
    TimeSliderSetState(ts)
    #before we render the result, explicitly set the filename for this render
    swatts.fileName = "blood_flow_example_%04d.png" % file_idx
    SetSaveWindowAttributes(swatts)
    # render the image to a PNG file
    SaveWindow()
    file_idx +=1

################
# use visit_utils.encoding to encode these images into a "mp4" movie
#
# The encoder looks for a printf style pattern in the input path to identify the frames of the movie.
# The frame numbers need to start at 0. 
# 
# The encoder selects a set of decent encoding settings based on the extension of the
# the output movie file (second argument). In this case we will create a "mp4" file. 
# 
# Other supported options include ".mpg", ".mov". 
#   "mp4" is usually the best choice and plays on all most all platforms (Linux ,OSX, Windows).
#   "mpg" is lower quality, but should play on any platform.
#
# 'fdup' controls the number of times each frame is duplicated. 
#  Duplicating the frames allows you to slow the pace of the movie to something reasonable.
#
################
 
input_pattern = "blood_flow_example_%04d.png"
output_movie = "blood_flow_example.mp4"
encoding.encode(input_pattern,output_movie,fdup=4)
