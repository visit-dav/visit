###########################################
# file: VisIt Scripting Tutorial Listing 4
###########################################
#
# 1) Open VisIt
# 2) Open 'example.silo'
# 3) Paste and execute via the Commands window, or the CLI.
#

# import visit_utils, we will use it to help encode our movie
from visit_utils import *
 
# Set a better view
ResetView()
v = GetView3D()
v.RotateAxis(0,44)
v.RotateAxis(1,-23)
SetView3D(v)
 
# Disable annotations
aatts = AnnotationAttributes()
aatts.axes3D.visible = 0
aatts.axes3D.triadFlag = 0
aatts.axes3D.bboxFlag = 0
aatts.userInfoFlag = 0
aatts.databaseInfoFlag = 0
aatts.legendInfoFlag = 0
SetAnnotationAttributes(aatts)
 
# Set basic save options
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
 

 
####
# Crop streamlines to render them at increasing time values over 50 steps
####
iatts.cropValue = iatts.Time 
iatts.cropEndFlag = 1
iatts.cropBeginFlag = 1
iatts.cropBegin = 0
for ts in range(0,50):
    # set the integral curve attributes to change the where we crop the streamlines
    iatts.cropEnd = (ts + 1) * .5
    
    # update streamline attributes and draw the plot
    SetOperatorOptions(iatts)
    DrawPlots()
    #before we render the result, explicitly set the filename for this render
    swatts.fileName = "streamline_crop_example_%04d.png" % ts
    SetSaveWindowAttributes(swatts)
    # render the image to a PNG file
    SaveWindow()
 
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
 
input_pattern = "streamline_crop_example_%04d.png"
output_movie = "streamline_crop_example.mp4"
encoding.encode(input_pattern,output_movie,fdup=4)