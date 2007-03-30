# ----------------------------------------------------------------------------
#  Test script that shows how to change a Volume plot's attributes.
#
#  Modifications:
#    Hank Childs, Tue May  7 09:27:44 PDT 2002
#    Updated for 0.9.9.  Made it use a file in the /data directory.
#    Updated clipping planes to not cut off the dataset.
#
#    Brad Whitlock, Tue Apr 1 09:37:38 PDT 2003
#    I made it work with 1.1.3.
# 
# ----------------------------------------------------------------------------

import os

# Import math for sin().
import math, sys

# Check the version.
ScriptVersion = "1.1.3"
if(Version() != ScriptVersion):
    print "This script is for VisIt %s. It may not work with version %s" % (ScriptVersion, Version())

# define a clamping function
def clamp(v):
    r = v
    if(v < 0):
        r = 0;
    if(v > 255):
        r = 255
    return r

def fv():
    # Open the database.
    OpenDatabase('../../data/globe.silo')
    
    # Play with the transfer function. This transfer function is a sine wave 
    # that is perturbed by another higher frequency sine wave.
    f = VolumeAttributes()
    for i in range(256):
        angle = float(i) * (6.28 / 256.)
        opacity = int((math.sin(angle) * 128.) + (math.sin(5*angle) * 20.) + 128.)
        f.SetFreeformOpacity(i, clamp(opacity))
    
    # Create another transfer function. This transfer function is a parabola.
    f2 = VolumeAttributes()
    for i in range(256):
        t = float(i) / 256.
        t2 = 32. * t - 16.
        opacity = clamp(t2 * t2)
        f2.SetFreeformOpacity(i, opacity)
    
    # Add the plot and set the attributes.
    AddPlot('Volume', 'u')
    SetPlotOptions(f)
    DrawPlots()
    
    # Interpolate between the two transfer functions.
    f3 = VolumeAttributes()
    nsteps = 20
    for i in range(nsteps):
        t = float(i) / float(nsteps - 1)
        # Interpolate the transfer function using linear interpolation
        for j in range(256):
            opacity = (t * float(f2.GetFreeformOpacity(j))) + ((1.-t)*float(f.GetFreeformOpacity(j)))
            f3.SetFreeformOpacity(j, clamp(int(opacity)))
        # Set the new transfer function.
        SetPlotOptions(f3)


# Make sure we have the Volume plugin.
if 'Volume' not in PlotPlugins():
    print "This script requires the Volume plot plugin!"
elif (not os.path.isfile("../../data/globe.silo")):
    print "This script requires the file globe.silo to be built in the data directory"
else:
    fv()
