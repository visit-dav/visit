# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  OpenPMD.py
#
#  Tests:      openPMD hdf5 files
#
#  Programmer: Dave Pugmire
#  Date:       Mon Oct 28 13:00:15 EDT 2019
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("OpenPMD")

openPMDFile = 'openpmd_test_data/data00000500.opmd'

vars = ['Fields/rho']

def test(var, tagName) :
    SetTimeSliderState(1)
    AddPlot("Contour", var)
    SetActivePlots(0)
    ContourAtts = ContourAttributes()
    ContourAtts.minFlag = 1
    ContourAtts.min = 1
    ContourAtts.scaling = ContourAtts.Log
    SetPlotOptions(ContourAtts)
    DrawPlots()
    fileName = var.replace("/", "")
    Test(tagName + "_" +  fileName)
    DeleteAllPlots()


TestSection("3D Example openPMD File")
OpenDatabase(data_path(openPMDFile))
for var in vars:
    test(var,"openPMD_3D")
CloseDatabase(data_path(openPMDFile))

Exit()
