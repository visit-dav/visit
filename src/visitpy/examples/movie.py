# *****************************************************************************
# Script: movie.py
#
# Purpose:
#     Creates a dummy movie.  The purpose of this script is to demonstrate
#     some of the scripting capabilities of VisIt.
#
# Programmer: Hank Childs
# Creation:   April 24, 2003
#
# Modifications:
#
#   Hank Childs, Wed Jun 25 09:30:59 PDT 2003
#   Previously referring to different versions of wave.visit.  Now only refer
#   to one.
#
# *****************************************************************************

import os
import sys
if(not os.path.isfile("/usr/gapps/visit/data/wave.visit")):
    print "This script requires the file wave.visit to be built in the data directory"
    sys.exit()

#
# Tell VisIt what files you want to look at.
#
# In the GUI, VisIt will automatically recognize file naming conventions and
# make "virtual databases" out of them.  In the CLI, this functionality does
# not exist.  However, you can tell VisIt which files you are interested in
# using a ".visit file".  This is just a text file that has all of the files
# you want to look at.
#
# % cat wave.visit
# wave0000.silo
# wave0010.silo
# ...
# wave0700.silo
# %
#
OpenDatabase("/usr/gapps/visit/data/wave.visit")

#
# Add a transparent plot of pressure.
#
AddPlot("Pseudocolor", "pressure")
pc_atts = PseudocolorAttributes()
pc_atts.minFlag = 1 # Just indicates that we are going to set a min.
pc_atts.min = 0.    # This is the actual minimum.
pc_atts.maxFlag = 1 # Just indicates that we are going to set a max.
pc_atts.max = 1.    # This is the actual maximum.
pc_atts.opacity = 0.33
SetPlotOptions(pc_atts)

#
# Add a contour plot and scale it a little bit bigger in every dimension
# (this is an excuse to show how operators are applied)
#
AddPlot("Contour", "pressure")
AddOperator("Transform")
trans_atts = TransformAttributes()
trans_atts.doScale = 1
trans_atts.scaleOrigin = (0, 0.5, 2.5)
trans_atts.scaleY = 1.2
trans_atts.scaleZ = 1.05
SetOperatorOptions(trans_atts)
DrawPlots()

#
# This will set the name of prefix of the TIFF files saved out.
#
sw_atts = SaveWindowAttributes()
sw_atts.fileName = "wave"
SetSaveWindowAttributes(sw_atts)

#
# The following code will turn off all of the annotations.
#
annot_atts = AnnotationAttributes()
annot_atts.legendInfoFlag = 0
annot_atts.databaseInfoFlag = 0
annot_atts.userInfoFlag = 0
annot_atts.axesFlag = 0
annot_atts.bboxFlag = 0

# Solid colors -- ignore fourth component of tuple
# annot_atts.foregroundColor = (255, 255, 255, 255)
# annot_atts.backgroundColor = (0, 0, 0, 255)

# Gradient background -- still ignore fourth component of tuple.
annot_atts.backgroundMode = 1
annot_atts.gradientColor1 = (0, 0, 0, 255)
annot_atts.gradientColor2 = (255, 255, 255, 255)

SetAnnotationAttributes(annot_atts)

#
# As we animate through time, we will also be moving the camera position.
# The camera position will move across a cubic spline.  You must have at
# least four views to create the spline.
#
# These views were created by using the CLI.  I would rotate to a position
# I liked and then say "v = GetView3D()".  I would then print "v" to the
# screen (you can do this by saying "v").  I then copied and pasted the views
# into this script as v0, v1, v2, v3.  Note that you have to create an object
# each time to do this (ie "v0 = ViewAttributes()").
#
v0 = ViewAttributes()
v0.viewNormal = (-0.418219, 0.345837, 0.839934)
v0.focus = (5, 0.353448, 2.5)
v0.viewUp = (0.229871, 0.934881, -0.270473)
v0.viewAngle = 30
v0.setScale = 1
v0.parallelScale = 5.6009
v0.nearPlane = -11.2018
v0.farPlane = 11.2018
v0.perspective = 1
v0.windowCoords = (0, 1, 0, 1)
v0.viewportCoords = (0.2, 0.95, 0.15, 0.95)

v1 = ViewAttributes()
v1.viewNormal = (-0.00916236, 0.453467, 0.891226)
v1.focus = (5, 0.353448, 2.5)
v1.viewUp = (-0.1004, 0.886343, -0.452014)
v1.viewAngle = 30
v1.setScale = 1
v1.parallelScale = 5.6009
v1.nearPlane = -11.2018
v1.farPlane = 11.2018
v1.perspective = 1
v1.windowCoords = (0, 1, 0, 1)
v1.viewportCoords = (0.2, 0.95, 0.15, 0.95)

v2 = ViewAttributes()
v2.viewNormal = (0.443099, 0.463564, 0.767315)
v2.focus = (5, 0.353448, 2.5)
v2.viewUp = (-0.296768, 0.883516, -0.362393)
v2.viewAngle = 30
v2.setScale = 1
v2.parallelScale = 5.6009
v2.nearPlane = -11.2018
v2.farPlane = 11.2018
v2.perspective = 1
v2.windowCoords = (0, 1, 0, 1)
v2.viewportCoords = (0.2, 0.95, 0.15, 0.95)

v3 = ViewAttributes()
v3.viewNormal = (0.810397, 0.504527, 0.29784)
v3.focus = (5, 0.353448, 2.5)
v3.viewUp = (-0.482793, 0.863074, -0.14837)
v3.viewAngle = 30
v3.setScale = 1
v3.parallelScale = 5.6009
v3.nearPlane = -11.2018
v3.farPlane = 11.2018
v3.perspective = 1
v3.windowCoords = (0, 1, 0, 1)
v3.viewportCoords = (0.2, 0.95, 0.15, 0.95)

# Iterate through the time sequence.
frames_per_timestep=2
nframes = frames_per_timestep*GetDatabaseNStates()
vpts = (v0, v1, v2, v3)  # This could be an arbitrarily large number of views
x=[]
for i in range(4):
  x = x + [float(i) / float(3.)]

for i in range(0, nframes-1):
    # Only advance in time one in every "frames_per_timestep"
    if ((i % frames_per_timestep) == 0):
        AnimationSetFrame(i/frames_per_timestep)
    
    t = float(i) / float(nframes - 1)
    v = EvalCubicSpline(t, x, vpts)
    SetView3D(v)
    SaveWindow()

