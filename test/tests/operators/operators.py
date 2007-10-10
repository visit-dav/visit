# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  operators.py
#
#  Tests:      mesh      - 3D unstructured, single domain
#              plots     - pc
#              operators - slice, clip, onionpeel
#              selection - none
#
#  Defect ID:  none
#
#  Programmer: Jeremy Meredith
#  Date:       April 17, 2002
#
#  Modifications:
#
#    Hank Childs, Wed Jul 24 10:21:07 PDT 2002
#    Remove threshold tests in favor of its own file.
#
#    Hank Childs. Tue Nov 19 15:45:12 PST 2002
#    Change Aslice to Slice for slicing interface change.
#
#    Eric Brugger, Fri Jan 17 16:53:32 PST 2003
#    Change Erase to Clip for interface change.
#
#    Eric Brugger, Thu May  8 12:47:54 PDT 2003
#    Change the setting of one of the Slice attributes because of an
#    interface change.
#
#    Kathleen Bonnell, Thu Apr 29 07:40:58 PDT 2004 
#    Default slice atts have changed, update accordingly. 
#    
# ----------------------------------------------------------------------------

# Turn off all annotation
a = AnnotationAttributes()
a.axesFlag2D = 0
a.axesFlag = 0
a.triadFlag = 0
a.bboxFlag = 0
a.userInfoFlag = 0
a.databaseInfoFlag = 0
a.legendInfoFlag = 0
a.backgroundMode = 0
a.foregroundColor = (0, 0, 0, 255)
a.backgroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)

OpenDatabase("../data/globe.silo")

AddPlot("Pseudocolor", "u")
DrawPlots()

v = GetView3D()
v.SetViewNormal(-0.528889, 0.367702, 0.7649)
v.SetViewUp(0.176641, 0.929226, -0.324558)
v.SetParallelScale(17.3205)
v.SetPerspective(1)
SetView3D(v)

# Slice
slice = SliceAttributes()

RemoveAllOperators()
slice.originType = slice.Point
slice.SetOriginPoint(1,2,3)
slice.SetNormal(1,-1,0)
slice.SetUpAxis(0,0,1)
slice.project2d = 1
SetDefaultOperatorOptions(slice)
AddOperator("Slice")

Test("ops_sliceop01")

RemoveAllOperators()
slice.project2d = 0
SetDefaultOperatorOptions(slice)
AddOperator("Slice")

Test("ops_sliceop02")

# Clip
clip = ClipAttributes()

RemoveAllOperators()
clip.plane1Status = 1
clip.SetPlane1Origin(1,2,3)
clip.SetPlane1Normal(-1,1,0)
clip.funcType = 0
SetDefaultOperatorOptions(clip)
AddOperator("Clip")

Test("ops_clip01")

RemoveAllOperators()
clip.SetCenter(0,0,10)
clip.radius = 9
clip.funcType = 1
SetDefaultOperatorOptions(clip)
AddOperator("Clip")

Test("ops_clip02")

clip = ClipAttributes()
clip.plane1Status = 1
clip.SetPlane1Origin(1,2,3)
clip.SetPlane1Normal(-1,1,0)
clip.plane2Status = 1
clip.SetPlane2Origin(0,0,0)
clip.SetPlane2Normal(-1,0,0)
clip.funcType = 0
SetDefaultOperatorOptions(clip)
AddOperator("Clip")

Test("ops_clip03")

# OnionPeel
onion = OnionPeelAttributes()

RemoveAllOperators()
onion.adjacencyType = 0
onion.index = 30
onion.requestedLayer = 3
SetDefaultOperatorOptions(onion)
AddOperator("OnionPeel")

Test("ops_onion01")

RemoveAllOperators()
onion.adjacencyType = 1
onion.requestedLayer = 7
SetDefaultOperatorOptions(onion)
AddOperator("OnionPeel")

Test("ops_onion02")

Exit()
