# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Mark C. Miller, Fri Aug 26 14:31:48 PDT 2022
# ----------------------------------------------------------------------------

#
# Get some utils shared across many .py files used for quick recipes.
#
Source(tests_path('quickrecipes','vqr_utils.py.inc'))

def defining_expressions():
 
  try:
    # defining expressions {
    # Creating a new expression
    OpenDatabase("~juanita/silo/stuff/noise.silo")
    AddPlot("Pseudocolor", "hardyglobal")
    DrawPlots()
    DefineScalarExpression("newvar", "sin(hardyglobal) + cos(shepardglobal)")
    ChangeActivePlotsVar("newvar")
    # defining expressions }
    TestValueEQ('defining expressions error message',GetLastError(),'')
    TestPOA('defining expressions exceptions')
  except Exception as inst:
    TestFOA('defining expressions exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()
 
def pick():
 
  try:
    # pick {
    OpenDatabase("~juanita/silo/stuff/noise.silo")
    AddPlot("Pseudocolor", "hgslice")
    DrawPlots()
    s = []
    # Pick by a node id
    PickByNode(300)
    s = s + [GetPickOutput()]
    # Pick by a cell id
    PickByZone(250)
    s = s + [GetPickOutput()]
    # Pick on a cell using a 3d point
    Pick((-2., 2., 0.))
    s = s + [GetPickOutput()]
    # Pick on the node closest to (-2,2,0)
    NodePick((-2,2,0))
    s = s + [GetPickOutput()]
    # Print all pick results
    print(s)
    # Will produce output somewhat like...
    # ['\nA:  noise.silo\nMesh2D \nPoint: <-10, -7.55102>\nNode:...
    #  '\nD:  noise.silo\nMesh2D \nPoint: <-1.83673, 1.83673>\nNode:...
    #  ...\nhgslice:  <nodal> = 4.04322\n\n']
    # pick }
    TestValueIN('pick string',s[0],'hgslice:  <nodal> = 2.472')
    TestValueIN('pick string',s[3],'hgslice:  <nodal> = 4.043')
    TestValueEQ('pick error message',GetLastError(),'')
    TestPOA('pick exceptions')
  except Exception as inst:
    TestFOA('pick exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

def lineout1():
 
  try:
    # lineout 1 {
    p0 = (-5,-3, 0)
    p1 = ( 5, 8, 0)
    OpenDatabase("~juanita/silo/stuff/noise.silo")
    AddPlot("Pseudocolor", "hardyglobal")
    DrawPlots()
    Lineout(p0, p1)
    # Specify 65 sample points
    Lineout(p0, p1, 65)
    # Do three variables ("default" is "hardyglobal")
    Lineout(p0, p1, ("default", "airVf", "radial"))
    # lineout 1 }
    TestValueEQ('lineout 1 error message',GetLastError(),'')
    TestPOA('lineout 1 exceptions')
  except Exception as inst:
    TestFOA('lineout 1 exception "%s"'%str(inst), LINE())
    pass
  # No cleanup because next method uses results from this
  # DeleteWindow()
  # vqr_cleanup()

def lineout2():
 
  try:
    # lineout 2 {
    # Set active window to one containing Lineout curve plots (typically #2)
    SetActiveWindow(2)
    # Get array of x,y pairs for first curve plot in window
    SetActivePlots(0)
    hg_vals = GetPlotInformation()["Curve"]
    # Get array of x,y pairs for second curve plot in window
    SetActivePlots(1)
    avf_vals = GetPlotInformation()["Curve"]
    # Get array of x,y pairs for third curve plot in window
    SetActivePlots(2)
    rad_vals = GetPlotInformation()["Curve"]

    # Write it as CSV data to a file
    for i in range(int(len(hg_vals) / 2)):
        idx = i*2+1 # take only y-values in each array
        print("%g,%g,%g" % (hg_vals[idx], avf_vals[idx], rad_vals[idx]))
    # lineout 2 }
    TestValueEQ('lineout 2 error message',GetLastError(),'')
    TestPOA('lineout 2 exceptions')
  except Exception as inst:
    TestFOA('lineout 2 exception "%s"'%str(inst), LINE())
    pass
  DeleteWindow()
  vqr_cleanup()
 
def query():
 
  try:
    # query {
    OpenDatabase("~juanita/silo/stuff/noise.silo")
    AddPlot("Pseudocolor", "hardyglobal")
    DrawPlots()
    Query("NumNodes")
    print("The float value is: %g" % GetQueryOutputValue())
    Query("NumNodes")
    # query }
    TestValueEQ('query error message',GetLastError(),'')
    TestPOA('query exceptions')
  except Exception as inst:
    TestFOA('query exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

def finding_the_min_and_max():
 
  try:
    # finding the min and max {
    # Define a helper function to get node/zone id's from query string.
    def GetMinMaxIds(qstr):
        import string
        s = qstr.split(' ')
        retval = []
        nextGood = 0
        idType = 0
        for token in s:
            if token == "(zone" or token == "(cell":
                idType = 1
                nextGood = 1
                continue
            elif token == "(node":
                idType = 0
                nextGood = 1
                continue
            if nextGood == 1:
                nextGood = 0
                retval = retval + [(idType, int(token))]
        return retval

    # Set up a plot
    OpenDatabase("~juanita/silo/stuff/noise.silo")
    AddPlot("Pseudocolor", "hgslice")
    DrawPlots()
    Query("MinMax")

    # Do picks on the ids that were returned by MinMax.
    for ids in GetMinMaxIds(GetQueryOutputString()):
        idType = ids[0]
        id = ids[1]
        if idType == 0:
            PickByNode(id)
        else:
            PickByZone(id)
    # finding the min and max }
    TestValueEQ('finding the min and max error message',GetLastError(),'')
    TestPOA('finding the min and max exceptions')
  except Exception as inst:
    TestFOA('finding the min and max exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()
 
defining_expressions()
pick()
lineout1()
lineout2()
query()
finding_the_min_and_max()

Exit()
