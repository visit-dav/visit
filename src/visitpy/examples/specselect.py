# ----------------------------------------------------------------------------
#  Test script that changes animates an isosurface by changing its iso-value.
#
#  Modifications:
#    Hank Childs, Tue May  7 09:27:44 PDT 2002
#    Updated for 0.9.9.  Made it use a file in the /data directory.
#    Also update view setting methods to meet new interface.
#
#    Brad Whitlock, Tue Apr 1 10:07:57 PDT 2003
#    Updated for 1.1.3.
#
#    Eric Brugger, Tue Sep  2 12:06:22 PDT 2003
#    I updated the script for 1.2.1.
#
# ----------------------------------------------------------------------------

import os

# Check the version.
ScriptVersion = "1.2.1"
if(Version() != ScriptVersion):
    print "This script is for VisIt %s. It may not work with version %s" % (ScriptVersion, Version())

# Define a function to print out the categories and the sets in them.
def specselect():
    # Open a database and do a plot.
    OpenDatabase('../../data/specmix_quad.silo')
    AddPlot('Pseudocolor', 'Species')
    
    # Do species selection
    silr = SILRestriction()
    sets = silr.SetsInCategory('Species')
    # Iterate through the SIL restriction and turn off all species except
    # species 1 for each material.
    for s_index in sets:
        speciesNumber = silr.SetName(s_index)[-1:]
        if(speciesNumber == '1'):
            silr.TurnOnSet(s_index)
        else:
            silr.TurnOffSet(s_index)
    SetPlotSILRestriction(silr)
    print silr
    
    # Draw the species selected plot.
    DrawPlots()

if(not os.path.isfile("../../data/specmix_quad.silo")):
    print "This script requires the file specmix_quad.silo to be built in the data directory"
else:
    specselect()
