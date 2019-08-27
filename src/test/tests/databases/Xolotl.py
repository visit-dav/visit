# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  Xolotl.py
#
#  Tests:      Xolotl hdf5 files
#
#  Programmer: James Kress
#  Date:       Mon Aug 26 13:00:15 EDT 2019
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("Xolotl")

from os.path import join as pjoin

xolotl_0D = "xolotl_network-PSI2-0D.xolotl"
xolotl_1D = "xolotl_network-PSI2-1D.xolotl"

zeroDVars = ["Helium/Vacancies", "Helium/Deuterium", "Helium/Interstitial", "Helium/Tritium"]
oneDVars = ["Helium Concentration", "Deuterium Concentration", "Interstitial Concentration", "Tritium Concentration"]

def test(var_name,tag_name):
    SetTimeSliderState(1)
    AddPlot("Pseudocolor", var_name)
    SetActivePlots(0)
    PseudocolorAtts = PseudocolorAttributes()
    PseudocolorAtts.scaling = PseudocolorAtts.Log  # Linear, Log, Skew
    PseudocolorAtts.colorTableName = "Oranges"
    SetPlotOptions(PseudocolorAtts)
    DrawPlots()
    Test(tag_name + "_" +  var_name)
    DeleteAllPlots()


TestSection("0D Example Xolot File")
OpenDatabase(xolotl_0D)
for var_name in zeroDVars:
    test(var_name,"xolotl_0D")
CloseDatabase(xolotl_0D)

TestSection("1D Example Xolotl File")
OpenDatabase(xolotl_1D)
for var_name in oneDVars:
    test(var_name,"xolotl_1D")
CloseDatabase(Xolotl_1D)

Exit()
