###############################################################################
#
# Program: writeback.py
#
# Purpose: Connects to a running "writeback" simulation program and uses its
#          data to create a plot, modify the plot, save the modified data
#          back to the simulation, and plot that modified data from the
#          simulation.
#
# Notes:   Run from "visit -cli -s writeback.py". You could also, 
#          "import visit" and prepend the VisIt calls with "visit." if you 
#          use the VisIt module from a regular Python shell.
#
# Programmer: Brad Whitlock
# Date: Fri Nov 3 09:15:30 PDT 2006
#
# Modifications:
#
###############################################################################
import os, sys

def GetMostRecentSIMFile():
    cwd = os.path.abspath(os.curdir)
    os.chdir(os.getenv("HOME") + "/.visit/simulations")
    simpath = os.path.abspath(os.curdir)
    files = os.listdir(".")
    files.sort()
    simFiles = []
    for f in files:
        if f[-5:] == ".sim1":
            simFiles = simFiles + [f]
    os.chdir(cwd)
    return simpath + "/" + simFiles[-1]


# Open the simulation.
simDB = GetMostRecentSIMFile()
OpenDatabase(simDB)

# OpenDatabase, in the case of a simulation, seems to return once VisIt has
# connected to the simulation. Unfortunately, the metadata from the simulation
# may not have been returned at the end of OpenDatabase. We will have to
# make OpenDatabase block until the metadata has been returned for simulations
# to work 100% as expected. In the meantime, insert a long pause.
os.system("sleep 10")

# Add a mesh plot
AddPlot("Mesh", "mesh")

# Keep only the region of the mesh plot within a sphere.
c = ClipAttributes()
c.funcType = c.Sphere
c.center = (4., 2.5, 0.)
c.radius = 2
c.sphereInverse = 1
AddOperator("Clip")
SetOperatorOptions(c)

# Draw the plot.
DrawPlots()

# Write the clipped data back to the simulation as a mesh called "OBJ".
e = ExportDBAttributes()
e.db_type = "SimV1"
e.variables = ()
e.filename = "OBJ"
ExportDatabase(e)

# Reopen the database so VisIt gets revised metadata about the contents
# of the simulation. This is a kludge that we will hopefully be able to
# avoid in later versions of VisIt. We'll have to provide a mechanism
# for the simulation to push its new metadata back to VisIt when the
# simulation accepts new data from VisIt. Not hard - just not done yet.
ReOpenDatabase(simDB)

# Now make a plot of the new "OBJ" object that came from VisIt. Note that
# it looks the same as the previous plot but it required no operators
# to make it look that way. That's because "OBJ" is a new mesh within
# the simulation.
DeleteAllPlots()
AddPlot("Mesh", "OBJ")
DrawPlots()
SaveWindow()

# Quit out.
sys.exit(0)
