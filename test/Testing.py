# ----------------------------------------------------------------------------
# Function: GetMemUsage
#
# Purpose: This function examins the process(s) whos ProcessAttributes are
# in procAtts using successive calls to ps. It sleeps 1 second between calls.
# If after 3 calls in succession, the sum of osz (virtual mem size) over all
# process(es) has not changed, it takes that as the reading of memory usage
# for the process. Note, it also, it also gets the pagesize to convert
# osz to actual byte count. This probably means this code only works on the
# suns.
#
# Programmer: Mark C. Miller
# Created:    January 31, 2005
#
# ----------------------------------------------------------------------------
#
def GetMemUsage(procAtts):
    global oszPageSize

    # if we haven't set global page size, set it
    if (oszPageSize == 0):
        pagesizeInfo = os.popen("pagesize")
        line = pagesizeInfo.readline()
        pagesizeInfo.close()
        oszPageSize = float(line)

    fmtStr = "ps -p "
    for pid in procAtts.pids:
        fmtStr = fmtStr + "%d "
    fmtStr = fmtStr + "-o osz="
    psResultsStable = 0
    osz1 = 0.0
    while (psResultsStable < 3):
        osz = 0.0
        psInfo = os.popen(fmtStr%procAtts.pids)
        line = psInfo.readline()
        while (line != ""):
            osz += float(line)
            line = psInfo.readline()
        psInfo.close()
        osz = osz * oszPageSize
        if (osz == osz1):
            psResultsStable = psResultsStable + 1
        else:
            psResultsStable = 0
        time.sleep(1)
        osz1 = osz

    return osz

# ----------------------------------------------------------------------------
# Function: GetLinesThatContain
#
# Purpose:
#  Filter out a tuple of strings so that we only return the ones that contain
#  words in the list.
#
# ----------------------------------------------------------------------------

def GetLinesThatContain(lines, words):
    retval = []
    for line in lines:
        i = 0;
        while(i < len(words)):
            if(string.find(line, words[i]) != -1):
                retval = retval + [line[:-1]]
                i = len(words)
            i = i + 1
    return retval

# ----------------------------------------------------------------------------
# Function: InformationFromLine
#
# Purpose:
#  Splits the columns from the top command and returns the values specified
#  by the indices tuple.
#
# ----------------------------------------------------------------------------

def InformationFromLine(line, indices):
    tokens = string.split(line)
    realtokens = []
    for t in tokens:
        if(t != ''):
            realtokens = realtokens + [t]
    if(len(realtokens) >= 11):
        vals = []
        for i in indices:
            if(i < len(realtokens)):
                vals = vals + [realtokens[i]]
        return vals
    else:
        return None

# ----------------------------------------------------------------------------
# Function: CreateProgramInformation
#
# Purpose:
#  Converts the information that we care about from the top command into a 
#  Python dictionary that is keyed using the program pids.
#
# ----------------------------------------------------------------------------

def CreateProgramInformation(lines, indices):
    retval = {}
    for line in lines:
        info = InformationFromLine(line, indices)
        if info != None:
            pid = info[0]
            resources = info[1:]
            retval[pid] = resources
    return retval

# ----------------------------------------------------------------------------
# Function: ReadTop
#
# Purpose:
#  Reads the output of the top command and returns a Python dictionary of
#  information for the running VisIt components.
#
# ----------------------------------------------------------------------------

def ReadTop():
    # Read the output of top.
    if(sys.platform == 'sunos5'):
        p = os.popen("top -n 20 -b")
        indices = (0, 1, 6, 10)
    elif(sys.platform == 'darwin'):
        p = os.popen("top -n 20 -l 1 -U %s" % os.environ["LOGNAME"])
        indices = (0, 1, 9, 1)
    else:
        p = os.popen("top n 1 b")
        indices = (0, 1, 5, 11)
    lines = p.readlines()
    p.close()
    programs = ("gui", "mdserver", "viewer", "cli", "engine_ser", "engine_par")
    # Get the lines from top that are for programs that we care about.
    lines = GetLinesThatContain(lines, programs)
    # Create a dictionary of information from top keyed with the pids.
    return CreateProgramInformation(lines, indices)

# ----------------------------------------------------------------------------
# Function: InterpretMemorySize
#
# Purpose:
#  Interprets the memory size from the top command in terms of Kb and Mb.
#
# ----------------------------------------------------------------------------

def InterpretMemorySize(valString):
    retval = 0
    try:
        # Get the number
        offset = 1
        if valString[-1] == "-" or valString[-1] == "+":
            offset = 2
        retval = eval(valString[:-offset])
        # Multiply by the units
        units = valString[-offset]
        if units == "K":
            retval = retval * 1000
        elif units == "M":
            retval = retval * 1000000
        elif units == "G":
            retval = retval * 1000000000
        else:
            retval = retval * 1000
    except ValueError:
        retval = 0
    return int(retval)

# ----------------------------------------------------------------------------
# Function: AddMemorySample
#
# Purpose:
#  Reads output from the top command and adds the results to the memory
#  usage data that we want to plot.
#
# ----------------------------------------------------------------------------

def AddMemorySample():
    global memoryHistory
    global sampleIndex

    try:
        # Read the information from the top command
        info = ReadTop()

        # Add the samples into the memoryHistory data.
        for k in info.keys():
            validSample = 1
            if(sys.platform != 'darwin'):
                userName = info[k][0]
                validSample = (userName == os.environ["LOGNAME"])
            # Only keep track of the data if it is for the user running the test suite.
            if validSample:
                memSizeString = info[k][1]
                procNameString = info[k][2]
                # Get the curve name
                newKey = "%s_%s" % (procNameString, k)
                # Convert the memory size to a number.
                memSize = InterpretMemorySize(memSizeString)
                # Add a new data point to the curve.
                dataPoint = (sampleIndex,memSize)
                try:
                    memoryHistory[newKey] = memoryHistory[newKey] + [dataPoint]
                except KeyError:
                    # New key
                    memoryHistory[newKey] = [dataPoint]
        # Increase the number of samples
        sampleIndex = sampleIndex + 1
    except:
        # Ignore exceptions
        return

# ----------------------------------------------------------------------------
# Function: SampleMemoryUsage
#
# Purpose:
#  Thread callback function that gathers samples until the global keepSampling
#  variable is set to 0.
#
# ----------------------------------------------------------------------------

def SampleMemoryUsage():
    global keepSampling
    global threadWorking
    threadWorking = 1
    try:
        while(keepSampling == 1):
            AddMemorySample()
    except:
        keepSampling = 0
    threadWorking = 0

# ----------------------------------------------------------------------------
# Function: BeginTrackingMemoryUsage
#
# Purpose:
#  Begins a 2nd thread that independently samples the memory usage as reported
#  by the top command while the tests execute.
#
# ----------------------------------------------------------------------------

def BeginTrackingMemoryUsage():
    global threadWorking
    thread.start_new_thread(SampleMemoryUsage, ())
    #while (threadWorking == 0): pass

# ----------------------------------------------------------------------------
# Function: ScaledValue
#
# Purpose:
#  Scales the input value using the min, max limits so it is in the range [0,1]
#
# ----------------------------------------------------------------------------

def ScaledValue(val, minval, maxval):
    retval = 0.
    valueRange = float(maxval - minval)
    if(valueRange != 0.):
        retval = (float(val) - float(minval)) / valueRange
    return retval

# ----------------------------------------------------------------------------
# Function: FinishTrackingMemoryUsage
#
# Purpose:
#  Tells the sampling thread to quit and then writes the memory usage data
#  a curve file that is plotted with VisIt and added to the html page for the
#  entire test.
#
# Modifications:
#   Brad Whitlock, Mon Mar 26 11:23:23 PDT 2007
#   Added code to delete all plots.
#
# ----------------------------------------------------------------------------

def FinishTrackingMemoryUsage(html):
    global keepSampling
    global threadWorking
    global memoryHistory
    global category
    global pyfilebase

    # Tell the 2nd thread to stop sampling and wait for it to be done.
    keepSampling = 0
    while(threadWorking == 1): pass

    #
    # Define a helper function to help in sorting the component names. We want
    # to sort alphabetically but we want all engines to be last in the list
    # too so the coloring of the memory curves will be more consistent if there
    # are parallel engines.
    #
    def sort_comp_name(x, y):
        if x[:6] == "engine":
            if y[:6] == "engine":
                return cmp(x,y)
            else:
                return 1
        else:
            if y[:6] == "engine":
                return -1
            else:
                return cmp(x,y)

    # Get the keys used in the memoryHistory but make sure that they are sorted
    sortedKeys = memoryHistory.keys()
    sortedKeys.sort(sort_comp_name)

    # Find the min, max for curve x,y for all samples.
    minSample = 100000000
    maxSample = -100000000
    minUsage = 1000000000
    maxUsage = -1000000000
    for k in sortedKeys:
        for pt in memoryHistory[k]:
             minSample = min(minSample, pt[0])
             maxSample = max(maxSample, pt[0])
             minUsage = min(minUsage, pt[1])
             maxUsage = max(maxUsage, pt[1])

    # Create the name of a curve database
    curveDB = "%s_%s_memusage.curve" % (category, pyfilebase)
    curveImage = "%s_%s_memusage" % (category, pyfilebase)

    # Now that it's done, write out a curve file.
    curveFile = open(curveDB, "wt")
    maxNSamples = 0
    for k in sortedKeys:
        curveFile.write("# %s\n" % k)
        maxNSamples = max(maxNSamples, len(memoryHistory))
        for pt in memoryHistory[k]:
            x = ScaledValue(pt[0], minSample, maxSample)
            y = ScaledValue(pt[1], minUsage, maxUsage)
            curveFile.write("%g %g\n" % (x, y))
    curveFile.close()

    # Set up a tuple of curve colors.
    curveColors = ((255,0,0,255), (0,255,0,255), (0,0,255, 255), \
(0,255, 255,255), (255,0,255,255),(255,255,0,255), (255,128,0,255))

    # Delete all of the plots in case the user left them.
    DeleteAllPlots()

    # Delete all of the annotation objects
    try:
        for annotName in GetAnnotationObjectNames():
            annot = GetAnnotationObject(annotName)
            if type(annot) != "<type 'LegendAttributesObject'>":
                annot.Delete()
    except VisItException:
        # Done deleting annotation objects.
        pass

    # Use VisIt to save plots of the memory usage.
    OpenDatabase(curveDB)
    cIndex = 0
    cX = 0.02
    cY = 0.85
    for k in sortedKeys:
        # Create a curve plot and set its attributes.
        AddPlot("Curve", k)
        c = CurveAttributes()
        if maxNSamples < 20:
            c.lineWidth = 1
            c.showPoints = 1
        else:
            c.lineWidth = 0
            c.showPoints = 0
        c.lineStyle = 0
        c.pointSize = 3
        c.color = curveColors[cIndex]
        SetPlotOptions(c)

        # Create a label for the curve
        curveLabel = CreateAnnotationObject("Text2D")
        curveLabel.text = k
        curveLabel.useForegroundForTextColor = 0
        curveLabel.textColor = curveColors[cIndex]
        curveLabel.width = float(len(k)) * 0.1 / 8.
        curveLabel.position = (cX, cY)
        curveLabel.fontBold = 1
        cY = cY - 0.04

        # Set the next color index.
        if(cIndex + 1 == len(curveColors)):
            cIndex = 0
        else:
            cIndex = cIndex + 1
    DrawPlots()

    # Set the view
    cv = ViewCurveAttributes()
    cv.rangeCoords = (0, 1.02)
    SetViewCurve(cv)

    # Set some annotation options.
    TurnOffAllAnnotations()
    a = GetAnnotationAttributes()
    a.axes2D.visible = 1
    SetAnnotationAttributes(a)

    # Make axis labels.
    xtitle = CreateAnnotationObject("Text2D")
    xtitle.text = "Samples"
    xtitle.width = 0.17
    xtitle.position=(0.4,0.05)
    ytitle = CreateAnnotationObject("Text2D")
    ytitle.text = "Mem (Mb)"
    ytitle.width = 0.17
    ytitle.position=(0.01,0.5)

    # Make the #megs label
    megsLabel = CreateAnnotationObject("Text2D")
    megsLabel.text = "%1.2g Mb" % (float(maxUsage) / 1000000.)
    megsLabel.width = 0.15
    megsLabel.position = (0.04, 0.95)

    # Save the window
    swa = GetSaveWindowAttributes()
    swa.fileName = "html/%s" % curveImage
    swa.format = swa.JPEG
    swa.family = 0
    SetSaveWindowAttributes(swa)
    SaveWindow()

    # Remove the curve database
    os.unlink(curveDB)

    # Make a link to the file in the HTML for the test case.
    html.write("<h1><a name=\"memusage\">Memory usage</a></h1><img src=\"%s.jpeg\">\n" % curveImage)

    # Clean up
    xtitle.Delete()
    ytitle.Delete()
    megsLabel.Delete()
    DeleteAllPlots()


# ----------------------------------------------------------------------------
# Function: LeakCheck 
#
# Purpose: This function iterates over the following VisIt CLI calls
#
#    ClearCacheForAllEngines()
#    ClearWindow() 
#    DrawPlots()
#    SaveWindow()
#
# and queries memory usage for both viewer and engine upon the end of each
# iteration. It then computes the difference in memory usage from one iteration
# to the next. If the difference exceeds the given threshold on more than half
# of the iterations, the test returns a failure. If the test exceeds threshold
# on less than half of the iterations 
#
# Programmer: Mark C. Miller
# Created:    January 31, 2005
#
# ----------------------------------------------------------------------------
#
def LeakCheck(file, numIters):

    global engineProcAtts
    global viewerProcAtts
    global leakHistory

    #
    # Get initial memory usage *before* iterations
    #
    eosz0 = GetMemUsage(engineProcAtts)
    vosz0 = GetMemUsage(viewerProcAtts)

    # now, loop creating and deleting the plot
    eleaks = vleaks = 0
    eleakstr = vleakstr = ""
    eamt = vamt = 0
    eosz1 = eosz0
    vosz1 = vosz0
    for i in range(numIters):
        ClearCacheForAllEngines()
	ClearWindow()
        DrawPlots()
        SaveWindow()
        eosz2 = GetMemUsage(engineProcAtts)
        vosz2 = GetMemUsage(viewerProcAtts)
	eamt = eamt + eosz2 - eosz1
	vamt = vamt + vosz2 - vosz1
	if (eosz2 - eosz1 > 0):
	    eleaks = eleaks + 1
	    eleakstr = eleakstr + "   L"
	else:
	    eleakstr = eleakstr + "  Ok"
	if (vosz2 - vosz1 > 0):
	    vleaks = vleaks + 1
	    vleakstr = vleakstr + "   L"
	else:
	    vleakstr = vleakstr + "  Ok"
        eosz1 = eosz2
	vosz1 = vosz2

    #
    # tack on the test name to the leak history entry
    #
    leakHistory = leakHistory + "For test case \"%32s\": iter = "%file
    for i in range(numIters):
        leakHistory = leakHistory + "   %d"%i
    leakHistory = leakHistory + "\n"

    #
    # tack on the engine's leak information
    #
    if (eleaks == 0):
        leakHistory = leakHistory + "    Engine DID NOT LEAK\n" 
    else:
        leakHistory = leakHistory + "    Engine leaked avg. of %8d bytes          iter = %s\n"%(int(eamt / eleaks), eleakstr)

    if (vleaks == 0):
        leakHistory = leakHistory + "    Viewer DID NOT LEAK\n" 
    else:
        leakHistory = leakHistory + "    Viewer leaked avg. of %8d bytes          iter = %s\n"%(int(vamt / vleaks), vleakstr)

    print "%s"%leakHistory

# ----------------------------------------------------------------------------
# Function: FileExists 
#
# Purpose: tests if a file exists and, if the file is being written, waits
# until the file size does not change for growthInterval seconds. If the
# file exists but is of zero size, that is the same as it NOT existing
#
# waitToAppear: number of seconds to wait for the file to first appear
# growhtInterval: number of seconds between successive stats on the file 
#
# Programmer: Mark C. Miller
#             September 20, 2005
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

# ----------------------------------------------------------------------------
# Function: SaveFileInfo
#
# Purpose: return a string representing the appropriate extension for the
# given file format and return bools indicating if the format supports
# curves, images and/or geometry
#
# Programmer: Mark C. Miller
#             September 20, 2005
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

def GenFileNames(file, ext):
    global pyfilebase
    global category
    global modeStr

    if not os.path.isdir("current/%s"%category):
        os.system("mkdir current/%s"%category)
    if not os.path.isdir("current/%s/%s"%(category,pyfilebase)):
        os.system("mkdir current/%s/%s"%(category,pyfilebase))

    if not os.path.isdir("diff/%s"%category):
        os.system("mkdir diff/%s"%category)
    if not os.path.isdir("diff/%s/%s"%(category,pyfilebase)):
        os.system("mkdir diff/%s/%s"%(category,pyfilebase))

    # create file names
    cur  = "current/%s/%s/"%(category,pyfilebase)  + file + ext
    diff = "diff/%s/%s/"%(category,pyfilebase)     + file + ext
    base = "baseline/%s/%s/"%(category,pyfilebase) + file + ext
    altbase=""
    if modeStr != "":
        altbase = "baseline/%s/%s/%s/"%(category,pyfilebase,modeStr) + file + ext
        if (os.path.isfile(altbase)):
            base = altbase

    return (cur, diff, base, altbase)


# ----------------------------------------------------------------------------
# Function: Test
#
# Purpose:
#  Write out the file, compare it to the baseline, thumbnail it,
#  and add it's data to the html
#
# Modifications:
#   Mark C. Miller, Mon Mar 29 19:37:26 PST 2004
#   Added alternate SaveWindowAttributes
#
#   Mark C. Miller, Tue Mar 30 15:48:25 PST 2004
#   Added pause for interacitve mode
#
#   Brad Whitlock, Tue Mar 30 16:38:52 PST 2004
#   Added code to sample memory.
#
#   Mark C. Miller, Mon Apr 12 16:34:50 PDT 2004
#   Added code to test against an alternate baseline if one exists
#
#   Jeremy Meredith, Tue May  4 13:26:41 PDT 2004
#   Catch exceptions from failing to open a baseline.  This can happen if
#   you make a clearcase element before putting an image into it.
#
#   Mark C. Miller, Tue Nov 28 23:13:08 PST 2006
#   Replaced maxerr, maxrms, maxpix with diffState. Added diff measure
#   indicating amount of diffs in pixels and not just count of diff pixels
#
#   Mark C. Miller, Tue Nov 28 23:50:15 PST 2006
#   Changed maxdiff to meddiff
#
#   Mark C. Miller, Wed Nov 29 08:19:52 PST 2006 
#   Changed meddiff to avgdiff
#
#   Sean Ahern, Thu Dec 20 14:48:14 EST 2007
#   Made diffState be a string so its easier to understand.
# ----------------------------------------------------------------------------

def Test(file, altSWA=0):
    global pixdifftol
    global avgdifftol
    global maxds
    global trackingMemoryUsage
    global iactive
    global usePIL
    global pyfilebase
    global category
    global skipCases

    # if interactive, pause for user
    if iactive:
	print "***********************"
	print "***********************"
	print "***********************"
	print "Saving %s"%file
        print "Hit Enter To Continue"
	print "***********************"
	print "***********************"
	print "***********************"
        next = sys.stdin.read(1)

    (cur, diff, base, altbase) = GenFileNames(file, ".tif")

    # save the window in visit
    if altSWA != 0:
        sa=altSWA
    else:
        sa=SaveWindowAttributes()
        sa.screenCapture=1
    sa.family   = 0
    sa.fileName = cur
    sa.format   = sa.TIFF
    sa
    SetSaveWindowAttributes(sa)
    SaveWindow()

    # do a leak check if requested
    if leakcheck:
       sa.fileName = "current/leakcheck.tif" 
       SetSaveWindowAttributes(sa)
       LeakCheck(file, 5)

    diffState = 'Unknown'
    skipMe = file in skipCases
    tPixs = pPixs = dPixs = 0
    dpix = davg = 0.0
    if usePIL:
        (tPixs, pPixs, dPixs, davg) = DiffUsingPIL(file, cur, diff, base, altbase)
        if pPixs != 0:
            dpix = dPixs * 100.0 / pPixs
            if dpix > pixdifftol:
                if davg > avgdifftol:
                    diffState = 'Unacceptable'
                else:
                    diffState = 'Acceptable'
            else:
                diffState = 'Acceptable'
        else:
            if dPixs != 0:
	        dpix = 1000000.0
                diffState = 'Unacceptable'
            else:
	        dpix = 0.0
                diffState = 'None'
    if skipMe:
        diffState = 'Skipped'

    WriteHTMLForOneTestImage(diffState, dpix, tPixs, pPixs, dPixs, davg, file)

    # write data to the log file if there is one
    if (os.path.isfile("log")):
        log = open("log", 'a')
        if diffState == 'None':
	    log.write("    Test case '%s' PASSED\n" % file)
        elif diffState == 'Acceptable':
            log.write("    Test case '%s' PASSED: #pix=%06d, #nonbg=%06d, #diff=%06d, ~%%diffs=%.3f, avgdiff=%3.3f\n" %
	        (file, tPixs, pPixs, dPixs, dpix, davg))
        elif diffState == 'Unacceptable':
            log.write("    Test case '%s' FAILED: #pix=%06d, #nonbg=%06d, #diff=%06d, ~%%diffs=%.3f, avgdiff=%3.3f\n" %
	        (file, tPixs, pPixs, dPixs, dpix, davg))
        elif diffState == 'Skipped':
	    log.write("    Test case '%s' SKIPPED\n" % file)
        else:
            log.write("    Test case '%s' UNKNOWN:#pix=UNK , #nonbg=UNK , #diff=UNK , ~%%diffs=UNK,  avgdiff=UNK\n")
        log.close()

    # If we're tracking memory usage, add a sample.
    if trackingMemoryUsage:
       AddMemorySample()

    # update maxmimum diff state 
    diffVals = {
        'None' :  0,
        'Acceptable' : 1,
        'Unacceptable' : 2,
        'Unknown' : 3,
        'Skipped' : 4
    }
    maxds = max(maxds, diffVals[diffState])

# ----------------------------------------------------------------------------
# Function: WriteHTMLForOneTestImage 
#
# Purpose:
#  Writes HTML stuff for a single test image 
# ----------------------------------------------------------------------------

def WriteHTMLForOneTestImage(diffState, dpix, tPixs, pPixs, dPixs, davg, file):
    global html

    # write to the html file
    color = "#ffffff"
    if diffState == 'None':             color = "#00ff00"
    elif diffState == 'Acceptable':     color = "#ffff00"
    elif diffState == 'Unacceptable':   color = "#ff0000"
    elif diffState == 'Skipped':        color = "#0000ff"
    else:                               color = "#ff00ff"
    html.write(" <tr>\n")
    html.write("  <td bgcolor=\"%s\"><a href=\"%s.html\">%s</a></td>\n" % (color, file, file))
    html.write("  <td align=center>%.2f</td>\n" % (dpix))
    html.write("  <td align=center>%.2f</td>\n" % (davg))
    if (diffState == 'Unknown'):
        html.write("  <td align=center>Not Available</td>\n")
        html.write("  <td align=center><a href=\"c_%s.jpg\" onclick='return popup(\"c_%s.jpg\",\"image\");'><img src=\"c_%s_thumb.jpg\"></a></td>\n" % (file,file,file))
        html.write("  <td align=center>Not Available</td>\n")
    elif (diffState != 'None'):
        html.write("  <td align=center><a href=\"b_%s.jpg\" onclick='return popup(\"b_%s.jpg\",\"image\");'><img src=\"b_%s_thumb.jpg\"></a></td>\n" % (file,file,file))
        html.write("  <td align=center><a href=\"c_%s.jpg\" onclick='return popup(\"c_%s.jpg\",\"image\");'><img src=\"c_%s_thumb.jpg\"></a></td>\n" % (file,file,file))
        html.write("  <td align=center><a href=\"d_%s.jpg\" onclick='return popup(\"d_%s.jpg\",\"image\");'><img src=\"d_%s_thumb.jpg\"></a></td>\n" % (file,file,file))
    else:
        html.write("  <td colspan=3 align=center><a href=\"c_%s.jpg\" onclick='return popup(\"c_%s.jpg\",\"image\");'><img src=\"c_%s_thumb.jpg\"></a></td>\n" % (file,file,file))
    html.write(" </tr>\n")

    # write the individual testcase
    testcase = open("html/%s.html" % file, 'wt')
    testcase.write("<html><head><title>Results for test case %s</title></head>\n" % file)
    testcase.write("<h1>Results for test case <i>%s</i></h1>\n" % file)
    testcase.write("<body bgcolor=\"#a080f0\">\n")
    testcase.write("<table border=5><tr><td></td></tr>\n")
    testcase.write("  <tr>\n")
    testcase.write("    <td align=center rowspan=9 bgcolor=%s>\n"%color)
    if (diffState == 'None' or diffState == 'Acceptable'):
        testcase.write("        <b><h1><i>Passed</i></h1></b>\n");
    elif (diffState == 'Unacceptable'):
        testcase.write("        <b><h1><i>Failed</i></h1></b>\n");
    elif (diffState == 'Skipped'):
        testcase.write("        <b><h1><i>Skipped</i></h1></b>\n");
    else:
        testcase.write("        <b><h1><i>Unknown</i></h1></b>\n");
    testcase.write("    </td>\n")
    testcase.write("    <td align=center>Baseline:</td>\n")
    if (diffState == 'None'):
        testcase.write("    <td>Same As Current</td>\n")
    elif (diffState == 'Unknown'):
        testcase.write("    <td>Not Available</td>\n")
    elif (diffState == 'Skipped'):
        testcase.write("    <td>Skipped</td>\n")
    else:
        testcase.write("""    <td><a href="" onMouseOver="document.b.src='c_%s.jpg'" onMouseOut="document.b.src='b_%s.jpg'"><img name="b" border=0 src="b_%s.jpg"></img></a></td>\n"""%(file,file,file))
    testcase.write("  </tr>\n")
    testcase.write("  <tr>\n")
    testcase.write("    <td align=center>Current:</td>\n")
    testcase.write("""    <td><a href="" onMouseOver="document.c.src='b_%s.jpg'" onMouseOut="document.c.src='c_%s.jpg'"><img name="c" border=0 src="c_%s.jpg"></img></a></td>\n"""%(file,file,file))
    testcase.write("  </tr>\n")
    testcase.write("  <tr>\n")
    testcase.write("    <td align=center rowspan=7>Diff Map:</td>\n")
    if (diffState == 'None'):
        testcase.write("    <td rowspan=7>No Differences</td>\n")
    elif (diffState == 'Unknown'):
        testcase.write("    <td rowspan=7>Not Available</td>\n")
    elif (diffState == 'Skipped'):
        testcase.write("    <td rowspan=7>Skipped</td>\n")
    else:
        testcase.write("    <td rowspan=7><img src=d_%s.jpg></img></td>\n" %(file))
    testcase.write("    <td align=center><i>Error Metric</i></td>\n")
    testcase.write("    <td align=center><i>Value</i></td>\n")
    testcase.write("  </tr>\n")
    testcase.write("      <tr><td>Total Pixels</td>  <td align=right>%06d</td></tr>\n"%tPixs)
    testcase.write("      <tr><td>Non-Background</td><td align=right>%06d</td></tr>\n"%pPixs)
    testcase.write("      <tr><td>Different</td>     <td align=right>%06d</td></tr>\n"%dPixs)
    testcase.write("      <tr><td>%% Diff. Pixels</td><td align=right>%f</td></tr>\n"%dpix)
    testcase.write("      <tr><td>Avg. Diff</td><td align=right>%f</td></tr>\n"%davg)
    testcase.write("      <tr></tr>\n")
    testcase.write("  </tr>\n")
    testcase.write("</table>\n")
    testcase.write("</html>\n")
    testcase.close()

# ----------------------------------------------------------------------------
# Function: GetBackgroundImage 
#
# Purpose: Returns the image of just VisIt's background without any plots
# ----------------------------------------------------------------------------

def GetBackgroundImage(file):

    notHiddenList = []
    activeList = []

    plots = ListPlots(1)
    plotInfos = string.split(plots,"#")
    for entry in plotInfos:

	if entry == "":
	    continue;

        plotParts = string.split(entry,"|")
	plotHeader = plotParts[0]
	plotInfo = plotParts[1]

	# get CLI's plot id for this plot
	plotId = string.split(plotHeader,"[")
	plotId = plotId[1]
	plotId = string.split(plotId,"]")
	plotId = int(plotId[0])

	# get this plot's active & hidden status
	words = string.split(plotInfo,";")
	hidden = -1
	active = -1
	for word in words:
	    if word == "hidden=0":
	        hidden = 0
	    elif word == "hidden=1":
	        hidden = 1
            elif word == "active=0":
	        active = 0
            elif word == "active=1":
	        active = 1

	if active == 1:
	    activeList.append(plotId)

	# if this plot is NOT hidden, hide it
        if hidden == 0:
	    SetActivePlots((plotId))
	    HideActivePlots()
	    notHiddenList.append(plotId)

    # ok, all non-hidden plots have been hidden. So,
    # now save the background image
    oldSWA = SaveWindowAttributes()
    tmpSWA = SaveWindowAttributes() 
    tmpSWA.family   = 0
    tmpSWA.fileName = "current/" + file + "_bg.tif" 
    tmpSWA.format   = tmpSWA.TIFF
    tmpSWA.screenCapture = 1
    SetSaveWindowAttributes(tmpSWA)
    SaveWindow()
    bkimage = Image.open(tmpSWA.fileName)

    # ok, now restore everything to the way it was
    # before we got here
    SetSaveWindowAttributes(oldSWA)
    SetActivePlots(tuple(notHiddenList))
    HideActivePlots()
    SetActivePlots(tuple(activeList))

    return bkimage

# ----------------------------------------------------------------------------
# Function: DiffUsingPIL 
#
# Purpose:
#  Diffs test results using PIL, outputs HTML, makes jpeg images,
#
# Modifications:
#    Jeremy Meredith, Tue Jun  7 12:14:11 PDT 2005
#    Fixed error reporting for missing baseline images.
#
# ----------------------------------------------------------------------------

def DiffUsingPIL(file, cur, diff, baseline, altbase):

    # open it using PIL Image
    newimg = Image.open(cur)
    size = newimg.size;

    # open the baseline image
    try:
        if (os.path.isfile(altbase)):
            oldimg = Image.open(altbase)
            if (size != oldimg.size):
                print "Error: Baseline and current images are different sizes... resizing baseline to compensate"
                oldimg = oldimg.resize(size, Image.BICUBIC)
        elif (os.path.isfile(baseline)):
            oldimg = Image.open(baseline)
            if (size != oldimg.size):
                print "Error: Baseline and current images are different sizes... resizing baseline to compensate"
                oldimg = oldimg.resize(size, Image.BICUBIC)
        else:
            print "Warning: No baseline image: ",baseline
            oldimg = Image.open('nobaseline.pnm')
            oldimg = oldimg.resize(size, Image.BICUBIC)
    except:
        oldimg = Image.open('nobaseline.pnm')
        print "Warning: Defective baseline image: ",baseline
        oldimg = oldimg.resize(size, Image.BICUBIC)
        

    # create the difference image
    diffimg = ImageChops.difference(oldimg, newimg)
    diffimg = diffimg.convert("L", (0.3333333, 0.3333333, 0.3333333, 0))

    # get some statistics
    dstat   = ImageStat.Stat(diffimg)
    dmin    = dstat.extrema[0][0]
    dmax    = dstat.extrema[0][1]
    dmean   = dstat.mean[0]
    dmedian = dstat.median[0]
    drms    = dstat.rms[0]
    dstddev = dstat.stddev[0]

    plotpixels = 0
    diffpixels = 0
    size = newimg.size
    totpixels = size[0] * size[1]

    mdiffimg = diffimg.copy()
    if (dmax > 0 and dmax != dmin):

        # brighten the difference image before we save it
        map = []
        map.append(0)
        for i in range(1,256): map.append(255)
        mdiffimg = mdiffimg.point(map)

	annotAtts = GetAnnotationAttributes()

	if (annotAtts.backgroundMode != 0 or 
	    annotAtts.backgroundColor != (255, 255, 255, 255)):

	    # we have to be really smart if we don't have a constant color
	    # background
	    backimg = GetBackgroundImage(file)

	    # now, scan over pixels in oldimg counting how many non-background
	    # pixels there are and how many diffs there are
	    for col in range(0,size[0]):
	        for row in range(0, size[1]):
	            newpixel = newimg.getpixel((col,row))
		    oldpixel = oldimg.getpixel((col,row))
	            backpixel = backimg.getpixel((col,row))
		    diffpixel = mdiffimg.getpixel((col,row))
		    if oldpixel != backpixel:
		        plotpixels = plotpixels + 1
                    if diffpixel == 255:
		        diffpixels = diffpixels + 1

        else:

            mdstat   = ImageStat.Stat(mdiffimg)
            oldimgm = oldimg.convert("L", (0.3333333, 0.3333333, 0.3333333, 0))
            map1 = []
            for i in range(0,254): map1.append(255)
            map1.append(0)
            map1.append(0)
	    oldimgm = oldimgm.point(map1)
            mbstat   = ImageStat.Stat(oldimgm)
	    diffpixels = int(mdstat.sum[0]/255)
	    plotpixels = int(mbstat.sum[0]/255)

    mdiffimg.save(diff)

    # thumbnail size (w,h)
    thumbsize = (100,100)

    # create thumbnails and save jpegs
    newthumb    = newimg.resize(   thumbsize, Image.BICUBIC)
    newthumb.save(   "html/c_%s_thumb.jpg"%file);
    newimg.save(   "html/c_%s.jpg"%file, quality=90);
    if (dmax != 0):
        oldthumb    = oldimg.resize(   thumbsize, Image.BICUBIC)
        diffthumb   = mdiffimg.resize(  thumbsize, Image.BICUBIC)
        oldthumb.save(   "html/b_%s_thumb.jpg"%file);
        diffthumb.save(  "html/d_%s_thumb.jpg"%file);
        oldimg.save(   "html/b_%s.jpg"%file, quality=90);
        mdiffimg.save(  "html/d_%s.jpg"%file, quality=90);

    return (totpixels, plotpixels, diffpixels, dmean)

# ----------------------------------------------------------------------------
# Function: FilterTestText
#
# Purpose:
#  Filters words from the test text before it gets saved.
#
# Modifications:
#   Mark C. Miller, Tue Jan 29 18:57:45 PST 2008
#   Moved code to filter VISIT_TOP_DIR to top of routine to ensure it is
#   executed always, not just in the case where numdifftol is zero. I also
#   fixed cases where a floating point number occuring at the end of a
#   sentence ending in a period ('.') was not getting correctly interpreted
#   and filtered.
#
#   Mark C. Miller, Tue Jan 29 19:37:54 PST 2008
#   Adjusted case with the absolute value of the base values are very near
#   zero (e.g. less than square of numdifftol), to switch to absolute
#   diffs.
#
#   Mark C. Miller, Thu Jan 31 17:51:21 PST 2008
#   Modified the algorithm in a subtle way. Since the string.replace() calls
#   are applied repeatedly over the entire input string, it was possible for
#   an earlier replace to be corrupted by a later replace. The new algorithm
#   uses a two-step replace process. As numbers are found in the input string,
#   they are compared for magnitude of numerial difference to their counter
#   parts in the baseline string. If the difference is BELOW threshold, we aim
#   to replace the 'current' numerical value in inText with the baseline value
#   in baseText. This has the effect of preventing the numerical difference
#   from causing a REAL difference when the two strings are diff'd later on.
#   If the difference is NOT below threshold, we skip this replacement. That
#   has the effect of causing a REAL difference when the two strings are
#   diff'd later. When the replacement is performed (e.g. the numerical 
#   difference is below threshold), we perform the replacement in two steps.
#   In the first pass over the string, we replace each current value with
#   a unique replacement 'tag.' The string we use must be unique over all
#   words in inText. In the second pass, we replace each of these replacement
#   tags with the actual baseline string thereby making that word in the
#   string identical to the baseline result and effectively eliminating it
#   from effecting the text difference.
#
#   Mark C. Miller, Tue Mar  4 18:35:45 PST 2008
#   Fixed some issues with the replace algorithm. Changed how near-zero
#   diffs are handled back to 'ordinary' relative diff. Made it more graceful
#   if it is unable to import PIL. Made text diff'ing proceed without PIL.
#
#   Mark C. Miller, Tue Mar  4 19:53:19 PST 2008
#   Discovered that string managment was taking a non-trivial portion of
#   total test time for text-oriented tests. So, found a better way to handle
#   the replacements using string slicing. Now, replacements are handled as
#   we march word-for-word through the strings.
#
#   Mark C. Miller, Thu Mar  6 09:39:43 PST 2008
#   Made logic for relative diff clearer. Switched to use min operation in
#   denominator and switch order of min/abs there.
# ----------------------------------------------------------------------------

def FilterTestText(inText, baseText):
    global numdifftol

    #
    # We have to filter out the absolute path information we might see in
    # this string. runtest passes the value for visitTopDir here.
    #
    inText = string.replace(inText, "%s/data/"%visitTopDir, "VISIT_TOP_DIR/data/")
    inText = string.replace(inText, "%s/test/"%visitTopDir, "VISIT_TOP_DIR/test/")

    #
    # Only consider doing any string substitution if numerical diff threshold
    # is non-zero
    #
    if numdifftol != 0.0:

	# this is to support using VisIt tests to test Silo updates
        if silo == 1:
            tmpText = string.replace(inText, "/view/visit_VOBowner_testsilo", "")      
	else:
            tmpText = string.replace(inText, "/view/visit_VOBowner_testopt", "")      


	#
	# Break the strings into words. Pass over words looking for words that
	# form numbers. Whenever we have numbers, compute their difference
	# and compare it to threshold. If its above threshold, do nothing.
	# The strings will wind up triggering a text difference. If its below
	# threshold, eliminate the word from effecting text difference by
	# setting it identical to corresponding baseline word.
	#
	baseWords = string.split(baseText)
	inWords = string.split(tmpText)
	outText=""
        transTab = string.maketrans(string.digits, string.digits)
        inStart = 0
	for w in range(len(baseWords)):
            try:
		inWordT = string.translate(inWords[w], transTab, '><,()')
		baseWordT = string.translate(baseWords[w], transTab, '><,()')
		if inWordT.count(".") == 2 and inWordT.endswith(".") or \
		   baseWordT.count(".") == 2 and baseWordT.endswith("."):
		    inWordT = inWordT.rstrip(".")
		    baseWordT = baseWordT.rstrip(".")
                inStart = string.find(tmpText, inWordT, inStart)

		#
		# Attempt to convert this word to a number. Exception indicates
		# it wasn't a number and we can move on to next word
		#
	        inVal = string.atof(inWordT)
	        baseVal = string.atof(baseWordT)

                #
                # Compute a relative difference measure for these two numbers
                # This logic was taken from www.math.utah.edu/~beebe/software/ndiff
                #
                if inVal == baseVal:
                    valDiff = 0
                elif inVal == 0 and baseVal != 0:
                    valDiff = numdifftol # treat as above threshold 
                elif inVal != 0 and baseVal == 0:
                    valDiff = numdifftol # treat as above threshold 
                else:
                    valDiff = abs(inVal - baseVal) / min(abs(inVal), abs(baseVal))

                #
                # We want to ignore diffs that are deemed below threshold given
                # the relative diff. measure above. To affect this, we need to
                # replace the numbers in the input text that differ with their
                # cooresponding numbers in the baseline text. This will have the
                # effect of making the HTML difference ignore this value.
                # So, we do this replace only if the diff is non-zero and less
                # than threshold.
                #
                if valDiff > 0 and valDiff < numdifftol:
                    tmpText = tmpText[:inStart] + baseWordT + tmpText[inStart+len(inWordT):]

                inStart = inStart + len(inWordT)

	    #
	    # This word wasn't a number, move on
	    #
            except ValueError:
                # ignore exceptions
                pass

        return tmpText

    else:
        if silo == 1:
            return string.replace(inText, "/view/visit_VOBowner_testsilo", "")
	else:
            return string.replace(inText, "/view/visit_VOBowner_test", "")


# ----------------------------------------------------------------------------
# Function: TestText
#
# Purpose:
#  Write out text to file, diff it with the baseline, and add it's data to 
#  the html
#
# Modifications:
#   Brad Whitlock, Tue Mar 30 16:39:43 PST 2004
#   Added code to sample memory.
#
#   Mark C. Miller, Tue May 25 14:29:40 PDT 2004
#   Added code to support interactive mode
#
#   Hank Childs, Sun Mar 27 14:34:30 PST 2005
#   Fix typo with testing mode specific baselines.
#
#   Jeremy Meredith, Tue Jun  7 12:09:12 PDT 2005
#   Added support for missing baseline text files again.
#
#   Brad Whitlock, Mon Nov 21 13:41:19 PST 2005
#   I made sure that it uses the mode-specific baseline if one exists.
#
# ----------------------------------------------------------------------------
def TestText(file, inText):
    global html
    global maxds
    global trackingMemoryUsage
    global skipCases

    # if interactive, pause for user
    if iactive:
        print "***********************"
        print "***********************"
        print "***********************"
        print "Saving %s"%file
        print "Hit Any Key To Continue"
        print "***********************"
        print "***********************"
        print "***********************"
        next = sys.stdin.read(1)

    # create file names
    (cur, diff, base, altbase) = GenFileNames(file, ".txt")

    if (os.path.isfile(base)):
        O = os.open(base, os.O_RDONLY)
        baseText = os.read(O,100000000)
        os.close(O)
    else:
        print "Warning: No baseline text file: ",base
        base = "notext.txt"
        baseText = ""

    # Filter out unwanted text
    inText = FilterTestText(inText, baseText)

    # save the current text output 
    O = open(cur, 'w')
    O.write(inText)
    O.close()

    nchanges = nlines = 0

    # diff the baseline and current text files
    d = HtmlDiff.Differencer(base, cur)
    (nchanges, nlines) = d.Difference("html/%s.html"%file, file)

    # save the diff output 
    diffComm = "diff " + base + " " + cur
    diffOut = commands.getoutput(diffComm)
    O = open(diff, 'w')
    O.write(diffOut)
    O.close()

    # did the test fail? 
    failed = (nchanges > 0)
    skipMe = file in skipCases

    # write data to the log file if there is one
    if (os.path.isfile("log")):
        log = open("log", 'a')
        if failed:
	    if skipMe: log.write("    Test case '%s' SKIPPED\n" % file) 
	    else: log.write("    Test case '%s' FAILED\n" % file) 
        else:
	    if (nchanges < 0): log.write("    Test case '%s' UNKNOWN\n" % file) 
	    else: log.write("    Test case '%s' PASSED\n" % file) 
        log.close()

    # write to the html file
    color = "#00ff00"
    if (failed):
        if skipMe: color = "#0000ff"
	else: color = "#ff0000"
    else:
        if (nchanges < 0): color = "#00ffff"
    html.write(" <tr>\n")
    html.write("  <td bgcolor=\"%s\"><a href=\"%s.html\">%s</a></td>\n" % (color, file, file))
    html.write("  <td colspan=5 align=center>%d modifications totalling %d lines</td>\n" % (nchanges,nlines))
    html.write(" </tr>\n")

    # If we're tracking memory usage, add a sample.
    if trackingMemoryUsage:
       AddMemorySample()

    # set error codes 
    if failed:
        if skipMe: maxds = 4
	else: maxds = 2

# ----------------------------------------------------------------------------
# Function: TestSection
#
# Purpose:
#  Write a section header into the results table so it is easier to understand
#  the results for a large test.
#
# ----------------------------------------------------------------------------

def TestSection(sectionName):
    global html
    html.write(" <tr>\n")
    html.write("  <td colspan=6 align=center bgcolor=\"#0000ff\"><font color=\"#ffffff\"><b>%s</b></font></td>\n" % sectionName)
    html.write(" </tr>\n")

# ----------------------------------------------------------------------------
# Function: Exit
#
# Purpose:
#  Exit with the appropriate error code.  Must be called at end of test cases.
# ----------------------------------------------------------------------------

def Exit():
    global html
    global maxds
    global trackingMemoryUsage
    global leakHistory
    if purify == 1:
        ClearCacheForAllEngines()
    html.write("</table>\n")
    if trackingMemoryUsage == 1:
        try:
            FinishTrackingMemoryUsage(html)
        except:
            # We had an error finishing up the memory graph. Don't let it ruin our results.
            html.write("<i>FinishTrackingMemoryUsage encountered an error. No memory graph was created.</i>\n")
            pass
    html.write("</body>\n")
    html.write("</html>\n")
    if leakcheck:
	words = string.split(pyfilename,".")
	tmpfilename = words[0] + "_leaks"
	TestText(tmpfilename, leakHistory)
	leakHistory=""
    html.close()
    if (iactive == 0):
	if (maxds == 0):         sys.exit(111)
        if (maxds == 1):         sys.exit(112)
        if (maxds == 2):         sys.exit(113)
        if (maxds == 4):         sys.exit(119)
        sys.exit(114)


def TurnOnAllAnnotations(givenAtts=0):
    if (givenAtts == 0):
        a = AnnotationAttributes()
    else:
        a = givenAtts
    a.axes2D.visible = 1
    a.axes3D.visible = 1
    a.axes3D.triadFlag = 1
    a.axes3D.bboxFlag = 1
    a.userInfoFlag = 0
    a.databaseInfoFlag = 1
    a.legendInfoFlag = 1
    SetAnnotationAttributes(a)

def TurnOffAllAnnotations(givenAtts=0):
    if (givenAtts == 0):
        a = AnnotationAttributes()
    else:
        a = givenAtts
    a.axes2D.visible = 0
    a.axes3D.visible = 0
    a.axes3D.triadFlag = 0
    a.axes3D.bboxFlag = 0
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    a.legendInfoFlag = 0
    SetAnnotationAttributes(a)

# ----------------------------------------------------------------------------
#       Code to help in the VisIt test suite
#
#  Programmer: Jeremy Meredith
#  Date:       April 17, 2002
#
#  Modifications:
#    Hank Childs, Fri May 24 08:43:58 PDT 2002
#    Renamed SaveImageAtts to SaveWindowAtts.
#
#    Jeremy Meredith, Fri Jul 19 17:27:10 PDT 2002
#    Added python coloring code.  Added a third exit code for small errors.
#    Added code to write each individual test case to its own html file
#    and each test script to its own as well, and changed the formatting.
#
#    Jeremy Meredith, Thu Aug 29 15:10:45 PDT 2002
#    Improved the log file writing.
#
#    Jeremy Meredith, Fri Sep 13 17:11:48 PDT 2002
#    Made it brighten the difference image before saving to make it
#    easier to spot differences.  Made the per test case statistics
#    be in float format instead of decimal.
#
#    Hank Childs, Wed Nov 20 14:58:17 PST 2002
#    Explicitly test for divide by zero error.
#
#    Kathleen Bonnell, Fri Jun  6 12:09:41 PDT 2003  
#    Added TestText.
#
#    Jeremy Meredith, Mon Jun  9 17:53:36 PDT 2003
#    Moved the colorize-python code into its own module.
#    Added more advanced differencing to the textual comparisons.
#
#    Jeremy Meredith, Thu Jul 24 09:52:21 PDT 2003
#    Stopped saving baseline images.  It was messing things up.  Instead,
#    I made a default baseline image that clearly states "No Baseline Image".
#
#    Jeremy Meredith, Mon Aug 11 17:46:22 PDT 2003
#    Upped the quality level on output images to 90% (from the default of 75%)
#
#    Jeremy Meredith, Mon Aug 18 15:19:01 PDT 2003
#    Added timings.
#
#    Brad Whitlock, Mon Mar 22 13:51:17 PST 2004
#    Added TestSection.
#
#    Mark C. Miller, Tue Mar 30 15:48:25 PST 2004
#    Added new global, iactive, for interactive mode
#
#    Brad Whitlock, Tue Mar 30 15:56:38 PST 2004
#    I added code to create a graph of memory usage as the test runs.
#
#    Brad Whitlock, Fri Apr 2 10:00:09 PDT 2004
#    I fixed the memory tracking code so it should work when run in the
#    nightly test suite. Previously, it was failing because os.getlogin()
#    was throwing an exception, which prevented us from processing
#    any of the output from top. That resulted in empty memory plots.
#
#    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
#    Changed scalable rendering controls to use activation mode
#
#    Jeremy Meredith, Thu Oct 21 13:24:51 PDT 2004
#    Difference images now are monochrome instead of grayscale.
#
#    Mark C. Miller, Mon Nov 29 18:52:41 PST 2004
#    Added code to do differences based upon check sums, if available
#    Made it so thumb and full size images for baseline and diff are NOT
#    generated in cases where test passes
#
#    Mark C. Miller, Mon Feb 14 20:24:23 PST 2005
#    Added code to deal with where to find ImageMagick convert utility
#    Removed code to re-define some CLI functions for HDF5 test mode
#
#    Mark C. Miller, Tue May 10 19:53:08 PDT 2005
#    Made it smarter about measuring differences
#
#    Mark C. Miller, Mon Jan 23 16:11:59 PST 2006
#    Changed default SaveWindowAttributes to NOT use screen capture. So,
#    had to explicitly invoke it here.
#
#    Mark C. Miller, Sat Feb 11 12:07:27 PST 2006
#    Force save in screen capture for background image
#
#    Mark C. Miller, Tue Nov 21 09:06:25 PST 2006
#    Changed code to remove userInfo from annotations to
#    SetDefaultAnnotationAttributes so it will take effect in all windows.
#    Re-organized code in this file to place functions at top and main 
#    execution lines at bottom
#
#    Mark C. Miller, Mon Nov 27 12:44:37 PST 2006
#    Work around bug in calling SetDefaultAnnotationAttributes by also
#    calling SetAnnotationAttributes
#
#    Mark C. Miller, hu Feb  8 17:13:14 PST 2007
#    Added population of 'silo' mode and logic to FilterTestText to
#    deal with test view used for silo's tests
#
#    Mark C. Miller, Tue Jan 29 16:37:53 PST 2008
#    Removed 'optimized' mode. Added -numdifftol command line option
#    and numdifftol global tolerance for relative numerical differences.
# ----------------------------------------------------------------------------

import string, sys, time, os, commands, thread, HtmlDiff, HtmlPython
from stat import *

#############################################################################
#   Argument/Environment Processin 
#############################################################################

# global indicating the maximum difference over all images
maxds = 0
avgdifftol = 0.0
pixdifftol = 0
numdifftol = 0.0

# Some globals needed for plotting the memory usage of the test.
keepSampling = 1
threadWorking = 0
memoryHistory = {}
trackingMemoryUsage = 1
usePIL = 1
iactive = 0
sampleIndex = 0
checkSumMap = {}
oszPageSize = 0

# Process some command line arguments.
for arg in sys.argv:
    if (arg == "-notrackmem"): 
        trackingMemoryUsage = 0
    if (arg == "-noPIL"):
        usePIL = 0
    if (arg == "-interactive"):
        iactive = 1
    subargs = string.split(arg,"=")
    if (subargs[0] == "-pixdiff"):
        pixdifftol = float(subargs[1])
    if (subargs[0] == "-avgdiff"):
        avgdifftol = float(subargs[1])
    if (subargs[0] == "-numdiff"):
        numdifftol = float(subargs[1])

# find our file name
visitTopDir = os.environ['VISIT_TOP_DIR']
pyfilename = os.environ['VISIT_TEST_NAME']
pyfilebase = pyfilename[:-3]
category = os.environ['VISIT_TEST_CATEGORY']
modes    = string.split(os.environ['VISIT_TEST_MODES'],",")
skipCases = string.split(os.environ['VISIT_TEST_SKIP_CASES'],",")

if usePIL:
    try:
        from PIL import Image, ImageChops, ImageStat
    except:
        print "WARNING: unable to import modules from PIL" 
        usePIL = 0
else:
    if os.path.isfile("baseline/cksums.txt"):
        cksumFile = open("baseline/cksums.txt","r")
        line = cksumFile.readline()
        while (line != ""):
            words = string.split(line)
            checkSumMap[words[2]] = words[0]
            line = cksumFile.readline()
        cksumFile.close()
    else:
        print "WARNING: baseline/cksums.txt not available"

# parse modes for various possible modes
serial = 1
scalable = 0
parallel = 0
purify = 0
leakcheck = 0
silo = 0
leakHistory=""
modeStr=""
for mode in modes:
   if modeStr == "":
      modeStr = mode
   else:
      if mode != "":
          modeStr = modeStr + "_" + mode
   if mode == "scalable":
      scalable = 1
   if mode == "parallel":
      parallel = 1
      serial = 0
   if mode == "purify":
      purify = 1
   if mode == "leakcheck":
      leakcheck = 1
   if mode == "silo":
      silo = 1

# find tif to rgb image convert utility
if os.environ.has_key('VISIT_TEST_CONVERT'):
    imgConverter = os.environ['VISIT_TEST_CONVERT']
elif (os.path.isfile("/usr/bin/convert")):
    imgConverter = "/usr/bin/convert"
else:
    imgConverter = "convert"

#############################################################################
#   VisIt CLI Commands 
#############################################################################

# remove the user name
annot = AnnotationAttributes()
annot.userInfoFlag = 0
SetDefaultAnnotationAttributes(annot)
SetAnnotationAttributes(annot)

# set scalable rendering mode if desired
if scalable:
   ra = GetRenderingAttributes()
   ra.scalableActivationMode = ra.Always
   SetRenderingAttributes(ra)
else:
   ra = GetRenderingAttributes()
   ra.scalableActivationMode = ra.Never
   SetRenderingAttributes(ra)

# start parallel engine if parallel
haveParallelEngine = 1L
if parallel:
   haveParallelEngine = OpenComputeEngine("localhost", ("-np", "2"))
   if haveParallelEngine == 0L:
      diffState='Unknown'
      Exit()
else:
   OpenComputeEngine("localhost")

# get unix process attributes for engine and viewer
engineProcAtts=GetProcessAttributes("engine")
viewerProcAtts=GetProcessAttributes("viewer")
epids = open("engine_pids.txt", 'wt')
for p in engineProcAtts.pids:
    epids.write("%d\n"%int(p))
epids.close()

# Automatically turn off all annotations
# This is to prevent new tests getting committed that
# are unnecessarily dependent on annotations.
TurnOffAllAnnotations()

# set up our html output
html = open("html/%s_%s.html" % (category, pyfilebase), 'wt')
html.write("<SCRIPT TYPE=\"text/javascript\">\n")
html.write("<!--\n")
html.write("function popup(mylink, name)\n")
html.write("{\n")
html.write("if (! window.focus)return true;\n")
html.write("var href;\n")
html.write("if (typeof(mylink) == 'string')\n")
html.write("   href=mylink;\n")
html.write("else\n")
html.write("   href=mylink.href;\n")
html.write("window.open(href,name,'width=500,height=500,scrollbars=no');\n")
html.write("return false;\n")
html.write("}\n")
html.write("//-->\n")
html.write("</SCRIPT>\n")
html.write("<html><head><title>Results for %s/%s</title></head>\n" % (category,pyfilename))
html.write("<body bgcolor=\"#a0a0f0\">\n")
html.write("<H1>Results of VisIt Regression Test - <a href=%s_%s_py.html>%s/%s</a></H1>\n" % (category,pyfilebase,category,pyfilename))
memLinkString = ""
if trackingMemoryUsage == 1:
    memLinkString = "  <a href=\"#memusage\">(Memory usage)</a>"
html.write("<H2><a href=%s_%s_timings.html>(Full Timings)</a>%s</H2>\n" % (category,pyfilebase, memLinkString))
html.write("<table border>\n")
html.write(" <tr>\n")
html.write("  <td rowspan=2><b><i>Test Case</b></i></td>\n")
html.write("  <td colspan=2 align=center><b><i>Errors</b></i></td>\n")
html.write("  <td colspan=3 align=center><b><i>Images</b></i></td>\n")
html.write(" </tr>\n")
html.write(" <tr>\n")
html.write("  <td>%Diffs</td>\n")
html.write("  <td>Maximum</td>\n")
html.write("  <td>Baseline</td>\n")
html.write("  <td>Current</td>\n")
html.write("  <td>Diff Map</td>\n")
#html.write("  <td>All</td>\n")
html.write(" </tr>\n")
html.write("\n")

# colorize the source file, and write to an html file
HtmlPython.ColorizePython(category, pyfilename, pyfilebase)

# add test file info to log file
if (os.path.isfile("log")):
    log = open("log", 'a')
    log.write("\n")
    log.write(" - - - - - - - - - - - - - - -\n")
    log.write("  Test script %s\n" % pyfilename)
    log.write("\n")
    log.close()

#
# If we want to test the memory usage of the test then begin testing now.
#
if trackingMemoryUsage == 1:
    try:
        BeginTrackingMemoryUsage()
    except:
        trackingMemoryUsage = 0
