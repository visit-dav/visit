#
# file: aneurysm_part2.py
# info: Corresponds to actions taken in
# https://visit-sphinx-github-user-manual.readthedocs.io/en/develop/tutorials/Aneurysm.html
#


OpenDatabase("aneurysm.visit")
AddPlot("Mesh", "Mesh")
MeshAtts = MeshAttributes()
MeshAtts.opaqueColor = (204, 255, 255, 255)
MeshAtts.opacity = 0.25
SetPlotOptions(MeshAtts)

View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (-1, 0, 0)
View3DAtts.focus = (3.89585, 3.99132, 4.90529)
View3DAtts.viewUp = (0, 1, 0)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 2.05937
View3DAtts.nearPlane = -4.11875
View3DAtts.farPlane = 4.11875
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 1
View3DAtts.perspective = 1
View3DAtts.eyeAngle = 2
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (3.89585, 3.99132, 4.90529)
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
View3DAtts.shear = (0, 0, 1)
View3DAtts.windowValid = 1
SetView3D(View3DAtts)

AddPlot("Vector", "velocity")
VectorAtts = VectorAttributes()
VectorAtts.useStride = 1
VectorAtts.stride = 5
VectorAtts.lineStyle = VectorAtts.SOLID  # SOLID, DASH, DOT, DOTDASH
VectorAtts.lineWidth = 0
VectorAtts.scale = 0.5
VectorAtts.scaleByMagnitude = 1
VectorAtts.autoScale = 1
VectorAtts.headSize = 0.25
VectorAtts.headOn = 1
VectorAtts.colorByMag = 1
VectorAtts.useLegend = 1
VectorAtts.vectorColor = (0, 0, 0, 255)
VectorAtts.colorTableName = "Spectral"
VectorAtts.invertColorTable = 1
VectorAtts.lineStem = VectorAtts.Cylinder  # Cylinder, Line
VectorAtts.geometryQuality = VectorAtts.High  # Fast, High
SetPlotOptions(VectorAtts)
DrawPlots()

# hide before doing streamlines
HideActivePlots()

# Examining features of the Flow Field with Streamlines

# We first draw a single streamline from a seed point at the point of
# maximum velocity on a slice at the beginning of the artery

AddPlot("Pseudocolor", "velocity_magnitude")
PseudocolorAtts = PseudocolorAttributes()
PseudocolorAtts.limitsMode = PseudocolorAtts.CurrentPlot  # OriginalData, CurrentPlot
SetPlotOptions(PseudocolorAtts)

# We slice the mesh at Y=3 to display the velocity magnitude 
AddOperator("Slice")
SliceAtts = SliceAttributes()
SliceAtts.originType = SliceAtts.Point  # Point, Intercept, Percent, Zone, Node
SliceAtts.originPoint = (3, 3, 3)
SliceAtts.originIntercept = 1
SliceAtts.axisType = SliceAtts.YAxis  # XAxis, YAxis, ZAxis, Arbitrary, ThetaPhi
SliceAtts.project2d = 0
SetOperatorOptions(SliceAtts, 1)

DrawPlots()

SetQueryFloatFormat("%g")
# Query for the max value on the slice only by using use_actual_data=1
Query("Max", use_actual_data=1)
# get the x,y,z coordinates of the point. It will be used as seed point for the streamline
seed_point = GetQueryOutputObject()['max_coord']


AddPlot("Pseudocolor", "operators/IntegralCurve/velocity")

view = GetView3D()
view.viewNormal = (-1,0,0)
SetView3D(view)

iatts = IntegralCurveAttributes()
iatts.sourceType = iatts.Circle
iatts.planeOrigin = seed_point
iatts.planeNormal = (0, 1, 0)
iatts.planeUpAxis = (0, 0, 1)
iatts.radius = 0.12
iatts.sampleDensity0 = 12
iatts.dataValue = iatts.SeedPointID
iatts.integrationType = iatts.DormandPrince
iatts.issueTerminationWarnings = 0
iatts.issueStiffnessWarnings = 0
iatts.issueCriticalPointsWarnings = 0
SetOperatorOptions(iatts)

# set style options
patts = PseudocolorAttributes()
patts.colorTableName = "Reds"
patts.lineType = patts.Tube 
patts.tailStyle = patts.Spheres
patts.headStyle = patts.Cones
patts.endPointRadiusBBox = 0.02
SetPlotOptions(patts)

DrawPlots()

# Examining features of the Flow Field with Pathlines

iatts.pathlines = 1
iatts.pathlinesCMFE = iatts.CONN_CMFE  # CONN_CMFE, POS_CMFE

SetOperatorOptions(iatts)

# After this calculation is done, we can animate the Pathlines by cropping away
# portions of the curves based on advection time.

# iatts.cropValue = iatts.Time
# iatts.cropEndFlag = 1
#
# nsteps = 20 # Number of steps
# final_time = .995
# for i in range(nsteps+1):
#     iatts.cropEnd = final_time * i /nsteps
#     SetOperatorOptions(iatts)

