# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  locus.py
#
#  Purpose:
#      Identifies the locus of points of maximum value from inside a sphere.
#      Tests repeated lineout capabilities, as well as the VisIt writer.
#
#  Programmer: Hank Childs
#  Date:       April 4, 2005
#
#  Modifications:
#    Kathleen Bonnell, Thu Jul 14 10:28:41 PDT 2005
#    Save tmp.ultra to 'current' directory.
# ----------------------------------------------------------------------------

import visit_writer

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'

# BEGIN USER MODIFIABLE SECTION

database = "../data/globe.silo"
origin = (0,0,0)
radius = 10 
variable = "recenter(t) + 10*u"
isExpression = 1
startTheta = 0
thetaStep = 36
nThetaSteps = 11
startPhi = 0
phiStep = 18
nPhiSteps = 11

# END USER MODIFIABLE SECTION

if (isExpression):
   DefineScalarExpression("locus", variable)
OpenDatabase(database)
AddPlot("Pseudocolor", "locus")
DrawPlots()

import math

sw = GetSaveWindowAttributes()
sw.format = sw.ULTRA
sw.family = 0
sw.fileName = "current/tmp"
SetSaveWindowAttributes(sw)

import string
def GetNum(line):
   a = line.split(' ')
   pos = string.atof(a[0])
   max = string.atof(a[1])
   return [pos, max]


# Opens up the file "tmp.ultra" and gets the maximum position.
def GetMaximumPosition():
   uf = open("current/tmp.ultra", "r")
   line = uf.readline()  # Remove header
   line = uf.readline()
   num = GetNum(line)
   pos = num[0]
   max = num[1]
   while(1):
      line = uf.readline()
      if (line == ""):
          break
      num = GetNum(line)
      if (num[1] > max):
         pos = num[0]
         max = num[1]
   return [pos, max]


vals = []
pos = []
points = []
for j in range(nPhiSteps):
    phi = startPhi + j*phiStep
    phi_rads = phi*2.*math.pi / 360.
    for i in range(nThetaSteps):
        theta = startTheta + i*thetaStep
        theta_rads = theta*2.*math.pi / 360.
        PX = radius * math.cos(theta_rads) * math.sin(phi_rads)
        PY = radius * math.sin(theta_rads) * math.sin(phi_rads)
        PZ = radius * math.cos(phi_rads)
        PX2 = PX + origin[0]
        PY2 = PY + origin[1]
        PZ2 = PZ + origin[2]
        Lineout(origin, (PX2, PY2, PZ2))
        SetActiveWindow(2)
        SaveWindow()
        rv = GetMaximumPosition()
        pos = pos + [rv[0]]
        vals = vals + [rv[1]]
        prop = rv[0] / radius
        FX = origin[0] + (PX)*prop
        FY = origin[1] + (PY)*prop
        FZ = origin[2] + (PZ)*prop
        points = points + [FX, FY, FZ]
        DeleteAllPlots()
        SetActiveWindow(1)
        
dims = (nThetaSteps, nPhiSteps, 1)

visit_writer.WriteCurvilinearMesh("locus", 0, dims, points, [["max", 1, 1, vals]])
DeleteAllPlots()
OpenDatabase("locus.vtk")
AddPlot("Pseudocolor", "max")
DrawPlots()
Test("locus_01")

Exit()



