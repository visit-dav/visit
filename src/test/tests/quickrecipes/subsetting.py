# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Mark C. Miller, Fri Aug 26 14:31:48 PDT 2022
# ----------------------------------------------------------------------------

#
# Get some utils shared across many .py files used for quick recipes.
#
Source(tests_path('quickrecipes','vqr_utils.py'))

def turning_off_domains():
 
  try:
    # turning off domains {
    OpenDatabase("~juanita/silo/stuff/multi_rect2d.silo")
    AddPlot("Pseudocolor", "d")
    DrawPlots()

    # Turning off every other domain
    d = GetDomains()
    i = 0
    for dom in d:
        if i%2:
            TurnDomainsOff(dom)
        i += 1

    # Turn all domains off
    TurnDomainsOff()

    # Turn on domains 3,5,7
    TurnDomainsOn((d[3], d[5], d[7]))

    # turning off domains }
    TestValueEQ('turning off domains error message',GetLastError(),'')
    TestPOA('turning off domains exceptions')
  except Exception as inst:
    TestFOA('turning off domains exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()
 
def turning_off_materials():
 
  try:
    # turning off materials {
    OpenDatabase("~juanita/silo/stuff/multi_rect2d.silo")
    AddPlot("FilledBoundary", "mat1")
    DrawPlots()
    # Get the material names 
    GetMaterials()
    # GetMaterials() will return a tuple of material names such as
    #     ('1', '2', '3')
    # Turn off material with name "2"
    TurnMaterialsOff("2")
    # turning off materials }
    TestValueEQ('turning off materials error message',GetLastError(),'')
    TestPOA('turning off materials exceptions')
  except Exception as inst:
    TestFOA('turning off domains exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()
 
turning_off_domains()
turning_off_materials()

Exit()
