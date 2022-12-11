# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Mark C. Miller, Fri Aug 26 14:31:48 PDT 2022
# ----------------------------------------------------------------------------

#
# Get some utils shared across many .py files used for quick recipes.
#
Source(tests_path('quickrecipes','vqr_utils.py'))

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
  except:
    TestFOA('defining expressions exception', LINE())
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
  except:
    TestFOA('pick exception', LINE())
    pass
  vqr_cleanup()
 
defining_expressions()
pick()

Exit()
