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

xolotl_0D = "xolotl_test_data/network-PSI2-0D.xolotl"
xolotl_1D = "xolotl_test_data/network-PSI2-1D.xolotl"

zeroDVars = ["Helium/Vacancies", "Helium/Deuterium", "Helium/Interstitial", "Helium/Tritium"]
oneDVars = ["Helium Concentration", "Deuterium Concentration", "Interstitial Concentration", "Tritium Concentration"]

def test(var_name,tag_name):
    SetTimeSliderState(1)
    AddPlot("Pseudocolor", var_name)
    SetActivePlots(0)
    PseudocolorAtts = PseudocolorAttributes()
    PseudocolorAtts.scaling = PseudocolorAtts.Log  # Linear, Log, Skew
    PseudocolorAtts.colorTableName = "hot_desaturated"
    SetPlotOptions(PseudocolorAtts)
    DrawPlots()
    file_name = var_name.replace("/", "")
    Test(tag_name + "_" +  file_name)
    DeleteAllPlots()


TestSection("0D Example Xolot File")
OpenDatabase(data_path(xolotl_0D))
for var_name in zeroDVars:
    test(var_name,"xolotl_0D")
CloseDatabase(data_path(xolotl_0D))

TestSection("1D Example Xolotl File")
OpenDatabase(data_path(xolotl_1D))
for var_name in oneDVars:
    test(var_name,"xolotl_1D")
CloseDatabase(data_path(xolotl_1D))

Exit()
