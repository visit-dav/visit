# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  saveformats.py
#
#  Tests:      save window formats 
#
#  Programmer: Mark C. Miller 
#  Date:       September 20, 2005 
#
#  Modifications:
#    Jeremy Meredith, Mon Apr 23 14:07:35 EDT 2007
#    Don't create a mesh plot when we're saving geometry formats, since
#    the new behavior is to separate plots into different files (as they
#    typically have different variables and geometry types).
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Cyrus Harrison, Tue Feb  2 10:55:43 PST 2010
#    Fixed problem w/ setting active window that allowed errors to propagate
#    between test cases.
#
#    Mark C. Miller, Wed Apr  7 19:02:29 PDT 2010
#    Be smarter about testing curve formats while in scalable mode.
# ----------------------------------------------------------------------------

# ----------------------------------------------------------------------------
# Function: FileExists 
#
# Purpose:
#   Tests if a file exists and, if the file is being written, waits
#   until the file size does not change for growthInterval seconds. If the
#   file exists but is of zero size, that is the same as it NOT existing
#
# waitToAppear: number of seconds to wait for the file to first appear
# growhtInterval: number of seconds between successive stats on the file 
#
# Programmer: Mark C. Miller
#             September 20, 2005
#
# ----------------------------------------------------------------------------
def FileExists(name, waitToAppear, growthInterval):
    if os.path.isfile(name) == 0:
        time.sleep(waitToAppear)
    if os.path.isfile(name) == 0:
        return 0
    curSize = os.stat(name)[ST_SIZE]
    if growthInterval == 0:
        if curSize == 0:
            return 0
    else:
            return 1
    while 1:
        time.sleep(growthInterval)
        size = os.stat(name)[ST_SIZE]
        if size == curSize:
            if curSize == 0:
                return 0
            else:
                return 1
            curSize = size

# find tif to rgb image convert utility
if 'VISIT_TEST_CONVERT' in os.environ:
    imgConverter = os.environ['VISIT_TEST_CONVERT']
elif (os.path.isfile("/usr/bin/convert")):
    imgConverter = "/usr/bin/convert"
else:
    imgConverter = "convert"

# ----------------------------------------------------------------------------
# Function: SaveFileInfo
#
# Purpose:
#   Return a string representing the appropriate extension for the
#   given file format and return bools indicating if the format supports
#   curves, images and/or geometry.
#
# Programmer: Mark C. Miller
#             September 20, 2005
#
# ----------------------------------------------------------------------------
def SaveFileInfo(fmt):
    swa = SaveWindowAttributes()
    if (fmt == swa.POSTSCRIPT):
        return ("ps", 1, 0, 0)
    elif (fmt == swa.CURVE):
        return ("curve", 1, 0, 0)
    elif (fmt == swa.ULTRA):
        return ("ultra", 1, 0, 0)
    elif (fmt == swa.BMP):
        return ("bmp", 0, 1, 0)
    elif (fmt == swa.JPEG):
        return ("jpeg", 0, 1, 0)
    elif (fmt == swa.PNG):
        return ("png", 0, 1, 0)
    elif (fmt == swa.PPM):
        return ("ppm", 0, 1, 0)
    elif (fmt == swa.RGB):
        return ("rgb", 0, 1, 0)
    elif (fmt == swa.TIFF):
        return ("tif", 0, 1, 0)
    elif (fmt == swa.STL):
        return ("stl", 0, 0, 1)
    elif (fmt == swa.OBJ):
        return ("obj", 0, 0, 1)
    elif (fmt == swa.VTK):
        return ("vtk", 0, 0, 1)
    else:
        return ("unknown", 0, 0, 0)


swa=SaveWindowAttributes()
swa.family = 0

AddWindow()
SetActiveWindow(1)

# I=Image, G=Geometry, C=Curve formats
CFormats=[swa.CURVE, swa.POSTSCRIPT, swa.ULTRA]
IFormats=[swa.BMP, swa.JPEG, swa.PNG, swa.PPM, swa.RGB, swa.TIFF]
GFormats=[swa.STL, swa.OBJ, swa.VTK]

a = AnnotationAttributes()
a.userInfoFlag = 0
a.databaseInfoFlag = 0
a.legendInfoFlag = 0
SetAnnotationAttributes(a)

def TestSaveFormat(fmt):
    SetActiveWindow(1)
    mode = ""
    result = "Failed\n"
    (ext, isC, isI, isG) = SaveFileInfo(fmt)
    swatmp = swa
    swatmp.format = fmt
    swatmp.outputToCurrentDirectory = 1
    if isI:
        swatmp.fileName = "saveformat_tmp.%s"%ext
    else:
        swatmp.fileName = "saveformat_tmp"
    SetSaveWindowAttributes(swatmp)
    try:
        SaveWindow()
    except:
        if TestEnv.params["scalable"]:
            if GetLastError() == "You cannot save non-image formats (e.g. ultra, curve, stl, etc.)" \
                                 " from a window that is currently in scalable rendering mode. You" \
                                 " may force scalable rendering to Never but if the resulting data" \
                                 " is too big for the viewer to handle, it will likely crash" \
                                 " VisIt. For 3D formats, try an export database operation instead." :
                TestText("saveformat_%s%s"%(mode,ext), "Passed\n")
                return
        TestText("saveformat_%s%s"%(mode,ext), result)
        return

    # depending on the type of format this is, try to
    # read the file we just created back into VisIt and
    # put up a plot in window 2. If that succeeds, we'll
    # say this format's save actually worked
    if isC:
        if FileExists(swatmp.fileName+"."+ext, 1, 0):
            if ext == "ps":
                result = "Passed\n" # can only test existence for ps
            else:
                SetActiveWindow(2)
                if OpenDatabase(swatmp.fileName+"."+ext):
                    AddPlot("Curve","curve")
                    if DrawPlots():
                        result = "Passed\n"
                    DeleteAllPlots()
                    CloseDatabase(swatmp.fileName+"."+ext)
    elif isI:
        if swatmp.screenCapture == 0:
            mode = "offscreen_"
        tiffFileName = "%s/saveformat_tmp.tif"%TestEnv.params["run_dir"]
        tiffFileExists = 0
        imageFileExists = FileExists(swatmp.fileName, 1, 0)
        # TODO_WINDOWS ?
        if imageFileExists:
            os.system("%s %s -compress none %s"%(imgConverter, swatmp.fileName, tiffFileName))
            tiffFileExists = FileExists(tiffFileName, 1, 0)
        if tiffFileExists:
            SetActiveWindow(2)
            if OpenDatabase(tiffFileName):
                AddPlot("Pseudocolor","red")
                if DrawPlots():
                    result = "Passed\n"
                DeleteAllPlots()
                CloseDatabase(tiffFileName)
    elif isG:
        if FileExists(swatmp.fileName+"."+ext, 1, 0):
            if ext == "stl":
                meshName = "STL_mesh"
            elif ext == "obj":
                meshName = "OBJMesh"
            elif ext == "vtk":
                meshName = "mesh"
            SetActiveWindow(2)
            if OpenDatabase(swatmp.fileName+"."+ext):
                AddPlot("Mesh",meshName)
                if DrawPlots():
                    result = "Passed\n"
                DeleteAllPlots()
                CloseDatabase(swatmp.fileName+"."+ext)
    TestText("saveformat_%s%s"%(mode,ext), result)
    SetActiveWindow(1)


TestSection("Curve Formats")
OpenDatabase(data_path("curve_test_data","c062.curve"))

AddPlot("Curve", "going_down")
DrawPlots()
for f in CFormats:
    TestSaveFormat(f)

TestSection("Image Formats via Screen Capture")
DeleteAllPlots()
CloseDatabase(data_path("curve_test_data","c062.curve"))

OpenDatabase(silo_data_path("multi_rect2d.silo"))

AddPlot("Mesh", "mesh1")
AddPlot("Pseudocolor", "d")
DrawPlots()
slider = CreateAnnotationObject("TimeSlider")
for f in IFormats:
    TestSaveFormat(f)

TestSection("Image Formats via Off Screen")
swa.screenCapture = 0
for f in IFormats:
    TestSaveFormat(f)
swa.screenCapture = 1

TestSection("Geometry Formats")
DeleteAllPlots()
CloseDatabase(silo_data_path("multi_rect2d.silo"))

OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Pseudocolor", "dx")
DrawPlots()
for f in GFormats:
    TestSaveFormat(f)

Exit()
