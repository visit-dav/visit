import os, sys, inspect

#make the ultrawrapper modules available for import
sys.path.append(os.path.dirname(os.path.realpath(inspect.getfile(inspect.currentframe()))))

from UltraCurve import *
from UltraGrammar import *


class UltraQuit(object):
    """ a class to hold ultra's exit status """

    def __init__(self):
        self.timeToQuit = 0

#-----------------------------------------------------------------------------
# convenience methods 
#-----------------------------------------------------------------------------
def getShownPlots():
    rv = ()
    pl = GetPlotList()
    for i in range(GetNumPlots()):
        if not pl.GetPlots(i).hiddenFlag:
           rv += i,
    return rv

def getHiddenPlots():
    rv = ()
    pl = GetPlotList()
    for i in range(GetNumPlots()):
        if pl.GetPlots(i).hiddenFlag:
           rv += i,
    return rv

def makeAllPlotsActive():
    ap = ()
    for i in range(GetNumPlots()):
        ap += i, 
    SetActivePlots(ap)

def setAsActivePlot(curvePlotId):
    pl = GetPlotList()
    for i in range(GetNumPlots()):
        if pl.GetPlots(i).id == curvePlotId:
            SetActivePlots(i)
            return i
    return -1
  
def delAllExpr(curve):
    for i in range(len(curve.expressions)):
        DeleteExpression(curve.expressions[i])

def getBoxOperatorId(id):
    plot = GetPlotList().GetPlots(id)
    for i in range(len(plot.operatorNames)):
        if (plot.operatorNames[i] == "Box"):
            return i
    return -1


def createCurveCMFE(c1, c2):
    var1 = "<%s>" % (c1.var) 
    var2 = "<%s:%s>" % (c2.fileName, c2.var) 
    cmfe = "curve_cmfe(%s,%s)" % (var2, var1)
    return cmfe

    
def toggleToInt(t):
    if t == "off":
        return 0
    else:
        return 1
            
#-----------------------------------------------------------------------------
# Some globals
#-----------------------------------------------------------------------------

menuList     = CurveList()
selectedList = CurveList()

quitTime = UltraQuit()

openedDBList = []

plotHiddenState = []

#-----------------------------------------------------------------------------
# parse actions 
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# Read in curves 
#-----------------------------------------------------------------------------

def ultra_rd(s):
    t = LongFileName(s)
    md = GetMetaData(t)
    for i in range (md.GetNumCurves()):
        c = md.GetCurves(i)
        e = c.minSpatialExtents, c.maxSpatialExtents, \
            c.minDataExtents, c.maxDataExtents
        curve = MenuCurveItem(c.name, e, t)
        menuList.add(curve)

#-----------------------------------------------------------------------------
# handle file ops 
#-----------------------------------------------------------------------------

def ultra_file(t):
    if t.cmd == 'rd':
        ultra_rd(t.path)

#-----------------------------------------------------------------------------
# Handle singleton ops, like 'menu' and 'lst' 
#-----------------------------------------------------------------------------

def ultra_singleton(t):
    if t.cmd == 'menu' and menuList.curves != []:
        print menuList 
    elif t.cmd == 'lst' and selectedList.curves != []:
        print selectedList
    elif t.cmd == 'erase' and selectedList.curves != []:
        DeleteAllPlots() 
        for curve in selectedList.curves:
            delAllExpr(curve)
        selectedList.removeAll()
    elif (t.cmd == 'quit') or (t.cmd == 'end'):
        DeleteAllPlots()
        for curve in selectedList.curves:
            delAllExpr(curve)
        selectedList.removeAll()
        menuList.removeAll()
        quitTime.timeToQuit = 1

def createAndApplyExpression(curve, expDef, cmd):
    numE = len(curve.expressions)
    newvarName = "%s_%02d" % (curve.id, numE)
    DefineCurveExpression(newvarName, expDef)
    curve.expressions.append(newvarName)
    ChangeActivePlotsVar(newvarName)
    ResetView()
    SuppressQueryOutputOn()
    Query("SpatialExtents", "actual")
    curve.extents = GetQueryOutputValue()

#-----------------------------------------------------------------------------
# Handle regular math ops
#-----------------------------------------------------------------------------
def ultra_doOp_percurve(curve, t):
    currentPlot = setAsActivePlot(curve.plotId)
    if (currentPlot == -1):
        return 

    curve.modified = True

    # which var should be used for new exressions?
    numE = len(curve.expressions)
    if numE == 0:
        usevar = curve.var 
    else:
        usevar = curve.expressions[numE-1]

    if t.cmd in mathOpsNoArg:
        newvarDef = "%s(<%s>)" % (t.cmd, usevar)
        createAndApplyExpression(curve, newvarDef, t.cmd)

    elif t.cmd in mathOpsXNoArg:
        usecmd = mathOpsNoArg[mathOpsXNoArg.index(t.cmd)]
        newvarDef = "curve_domain(<%s>, %s(coord(<%s>)[0]))"  % \
                                 (usevar, usecmd, usevar)
        createAndApplyExpression(curve, newvarDef, t.cmd)


    elif t.cmd == 'ymin' or t.cmd == 'xmin':
        # need to make 'curve' the active plot
        currentPlot = setAsActivePlot(curve.plotId)
        # need to find out if 'box' has already been applied
        # and use its attributes instead of default to start
        boxId = getBoxOperatorId(currentPlot)
        if boxId == -1:
            AddOperator("Box")
            boxAtts = BoxAttributes()
            if t.cmd == 'ymin':
                boxAtts.minx = curve.extents[0]
            else :
                boxAtts.miny = curve.extents[2]
            boxAtts.maxx = curve.extents[1]
            boxAtts.maxy = curve.extents[3]
        else:
            boxAtts = GetOperatorOptions(boxId)
        if t.cmd == 'ymin':
            boxAtts.miny = float(t.arg)
        else: 
            boxAtts.minx = float(t.arg)
        SetOperatorOptions(boxAtts)
        ResetView() 

    elif t.cmd == 'ymax' or t.cmd == 'xmax':
        currentPlot = setAsActivePlot(curve.plotId)
        if (currentPlot == -1):
            return 
        # need to find out if 'box' has already been applied
        # and use its attributes instead of default to start
        boxId = getBoxOperatorId(currentPlot)
        if boxId == -1:
            AddOperator("Box")
            boxAtts = BoxAttributes()
            boxAtts.minx = curve.extents[0]
            boxAtts.miny = curve.extents[2]
            if t.cmd == 'ymax':
                boxAtts.maxx = curve.extents[1]
            else:
                boxAtts.maxy = curve.extents[3]
        else:
            boxAtts = GetOperatorOptions(boxId)
        if t.cmd == 'ymax':
            boxAtts.maxy = float(t.arg)
        else:
            boxAtts.maxx = float(t.arg)
        SetOperatorOptions(boxAtts)
        ResetView() 

    elif t.cmd in mathOpsArg:
        newvarDef = ""
        op = ""
        if t.cmd == 'divy':
            op = "/"
        elif t.cmd == 'dy':
            op = "+"
        elif t.cmd == 'my':
            op = "*"
        else:
            return 
        newvarDef = "<%s> %s %s" % (usevar, op, t.arg)
        createAndApplyExpression(curve, newvarDef, t.cmd)

    elif t.cmd in mathOpsXArg:
        newvarDef = "curve_domain(<%s>, %s(coord(<%s>)[0]))"  % \
                                 (usevar, usecmd, usevar)
        newvarDef = ""
        op = ""
        if t.cmd == 'divx':
            op = "/"
        elif t.cmd == 'dx':
            op = "+"
        elif t.cmd == 'mx':
            op = "*"
        else:
            return 
        newvarDef = "curve_domain(<%s>, coord(<%s>)[0] %s %s" % \
                                 (usevar, usevar, op, t.arg)
        createAndApplyExpression(curve, newvarDef, t.cmd)

    elif t.cmd == 'lnwidth':
        currentPlot = setAsActivePlot(curve.plotId)
        if (currentPlot == -1):
            return 
        ca = CurveAttributes(1)
        ca.lineWidth = int(t.arg)
        SetPlotOptions(ca)
    elif t.cmd == 'lnstyle':
        currentPlot = setAsActivePlot(curve.plotId)
        if (currentPlot == -1):
            return 
        ca = CurveAttributes(1)
        if (t.arg == "solid"):
            ca.lineStyle = ca.SOLID
        elif (t.arg == "dotted"):
            ca.lineStyle = ca.DOT
        elif (t.arg == "dashed"):
            ca.lineStyle = ca.DASH
        elif (t.arg == "dotdashed"):
            ca.lineStyle = ca.DOTDASH
        SetPlotOptions(ca)

#-----------------------------------------------------------------------------
# Handle math ops requiring cmfe's
#-----------------------------------------------------------------------------
def ultra_docmfemath(cl, t):
    if len(cl) < 2:
        print "Usage error: %s curve-list" % t.cmd
        return 
    curve1 = cl[0]
    fullExpression = "<%s>" % curve1.var
    exprName = "%s" % curve1.id
    for i in range(1, len(cl)):
        curve = cl[i]
        exprName += "%s%s" % (t.cmd, curve.id)
        if (curve.var not in  curve.expressions):
            cmfe = createCurveCMFE(curve1, curve)
        else:
            cmfe = "<%s>" % curve.var
        fullExpression += " %s %s" %(t.cmd, cmfe)
    DefineCurveExpression(exprName, fullExpression)
    newCurve = PlotCurveItem(exprName)
    newCurve.expressions.append(exprName)
    AddPlot("Curve", newCurve.var)
    n = GetNumPlots()
    newCurve.plotId = GetPlotList().GetPlots(n-1).id
    selectedList.add(newCurve)
    ca = CurveAttributes(1)
    ca.designator = newCurve.id
    SetPlotOptions(ca)
    DrawPlots()
    SuppressQueryOutputOn()
    Query("SpatialExtents", "actual")
    newCurve.extents = GetQueryOutputValue()


#-----------------------------------------------------------------------------
# Handle ops on Menu curves
#-----------------------------------------------------------------------------

def ultra_multiCurveNum(t):
    if t.cmd == 'select' and menuList.curves != []: 
        cl =  menuList.slice(t.clist)
        for curve in cl:
            if not curve.fileName in openedDBList:
                openedDBList.append(curve.fileName)
                OpenDatabase(curve.fileName)
            AddPlot("Curve", curve.var)
            n = GetNumPlots()
            c2 = PlotCurveItem(curve.var, curve.extents, curve.fileName)
            c2.plotId = GetPlotList().GetPlots(n-1).id
            selectedList.add(c2)
            ca = CurveAttributes(1)
            ca.designator = c2.id
            SetPlotOptions(ca)
        DrawPlots()


#-----------------------------------------------------------------------------
# Ops on selected (plotted) curves 
#-----------------------------------------------------------------------------

def ultra_multiCurveAlpha(t):
    if len(selectedList.curves) == 0:
        return 

    cl =  selectedList.slice(t.clist)
    if  len(cl) == 0:
        return

    if t.cmd == 'del':
        for curve in cl:
            selectedList.remove(curve)
            if setAsActivePlot(curve.plotId) >= 0:
                DeleteActivePlots()
            delAllExpr(curve)
    elif t.cmd in mathOpsNoArg:
        for curve in cl:
            ultra_doOp_percurve(curve, t) 
    elif t.cmd in mathOpsXNoArg:
        for curve in cl:
            ultra_doOp_percurve(curve, t) 
    elif t.cmd in cmfeOps:
        ultra_docmfemath(cl, t)
    elif t.cmd in mathOpsArg:
        if not t.arg:
            UltraUsage(cmd)
        else:
            for curve in cl:
                ultra_doOp_percurve(curve, t)
    elif t.cmd in opsArg or t.cmd == "lnstyle":
        if not t.arg:
            UltraUsage(cmd)
        else:
            for curve in cl:
                ultra_doOp_percurve(curve, t)

#-----------------------------------------------------------------------------
# Plot control operations 
#-----------------------------------------------------------------------------
def ultra_plotControl_toggleOps(t):

    if t.cmd == "axis":
        a = GetAnnotationAttributes()
        if not t.arg:
            print "  %d" % a.axes2D.visible
        elif t.arg == "on" or t.arg == "off":
            a.axes2D.visible = toggleToInt(t.arg)
            SetAnnotationAttributes(a)

    elif t.cmd == "grid":
        a = GetAnnotationAttributes()
        if not t.arg:
            print "  %d" % a.axes2D.xAxis.grid 
        elif t.arg == "on" or t.arg == "off":
            a.axes2D.xAxis.grid = toggleToInt(t.arg)
            a.axes2D.yAxis.grid = toggleToInt(t.arg)
            SetAnnotationAttributes(a)

    elif t.cmd == "x-log-scale":
        v = GetViewCurve()
        if not t.arg:
            print "  %d" % v.domainScale
        elif t.arg == "on":
            v.domainScale = v.LOG
            SetViewCurve(v)
        elif t.arg == "off":
            v.domainScale = v.LINEAR
            SetViewCurve(v)

    elif t.cmd == "y-log-scale":
        v = GetViewCurve()
        if not t.arg:
            print "  %d" % v.rangeScale
        elif t.arg == "on":
            v.rangeScale = v.LOG
            SetViewCurve(v)
        elif t.arg == "off":
            v.rangeScale = v.LINEAR
            SetViewCurve(v)

    elif t.cmd == "data-id":
        if not t.arg:
            print "  %d" % selectedList.__class__.data_id
            return
        elif t.arg != "on" and t.arg != "off":
            return

        tval = toggleToInt(t.arg)
        selectedList.__class__.data_id = tval
        # first want to hide all plots, modify the curve atts for all
        # plots (individually so as to only overwrite the 'showLabels' att),
        # then unhide them.  However, we need to take into account the fact
        # that some plots may already be hidden.  

        # get a list of visible plots
        shownPlots = getShownPlots()
        SetActivePlots(shownPlots)

        # this toggles the 'hidden' state, as we are only applying it
        # to shown plots, they will now be hidden.
        HideActivePlots()
        for i in range(GetNumPlots()):
           SetActivePlots(i)
           ca = CurveAttributes(1)
           ca.showLabels = tval
           SetPlotOptions(ca)
        SetActivePlots(shownPlots)
        # toggle back to 'on' the hidden state of the previously shown plots
        HideActivePlots()

def ultra_plotControl_rangeOps(t):
    if not t.arg:
        return

    elif t.cmd == "domain":
        v = GetViewCurve()
        if t.arg == "de":
           # keep range as it is, but reset domain
           r = v.rangeCoords
           shownPlots = getShownPlots()
           SetActivePlots(shownPlots)
           HideActivePlots()
           ResetView()
           v = GetViewCurve()
           v.rangeCoords = r
           SetViewCurve(v)
           HideActivePlots()
        elif len(t.arg) == 2:
           v.domainCoords = (t.arg[0], t.arg[1])
           SetViewCurve(v)
    elif t.cmd == "range":
        v = GetViewCurve()
        if t.arg == "de":
           # keep domain as it is, but reset domain
           d = v.domainCoords
           shownPlots = getShownPlots()
           SetActivePlots(shownPlots)
           HideActivePlots()
           ResetView()
           v = GetViewCurve()
           v.domainCoords = d
           SetViewCurve(v)
           HideActivePlots()
        elif len(t.arg) == 2:
            v.rangeCoords = (t.arg[0], t.arg[1])
            SetViewCurve(v)
        

def ultra_plotControl(t):
    if t.cmd in togglePlotOps:
        ultra_plotControl_toggleOps(t) 
    elif t.cmd in rangePlotOps:
        ultra_plotControl_rangeOps(t)

#-----------------------------------------------------------------------------
# Write out curves to ultra files
#-----------------------------------------------------------------------------
           
def ultra_saveCurve(t):
    stype = "ascii"
    if t.type:
        stype = t.type
    if len(selectedList.curves) == 0:
        return 
    cl =  selectedList.slice(t.clist)
    if  len(cl) == 0:
        return
   
    FILE = open(t.file, "w") 
    for curve in cl:
        currentPlot = setAsActivePlot(curve.plotId)
        if (currentPlot != -1):
            # This is how we need to do it for VisIt 2.0
            #s = GetPlotInformation()['Curve']
            # This is how we need to do it for VisIt 1.12
            s = GetOutputArray()
            FILE.write("# %s\n"%curve.var) 
            for i in range(0, len(s), 2):
                FILE.write("%13.6e %13.6e\n"%(s[i], s[i+1]))
    FILE.close()

#-----------------------------------------------------------------------------
# Execution of wrapper
#-----------------------------------------------------------------------------
def runUltraWrapper():
    quitTime.timeToQuit = 0

    #define parse actions for grammar

    singletonCommand.setParseAction(lambda t: ultra_singleton(t))
    fileOp.setParseAction(lambda t: ultra_file(t))
    multiCurveOpNum.setParseAction(lambda t: ultra_multiCurveNum(t))
    multiCurveOpAlpha.setParseAction(lambda t: ultra_multiCurveAlpha(t))
    lnstylecmd.setParseAction(lambda t: ultra_multiCurveAlpha(t))
    plotOp.setParseAction(lambda t: ultra_plotControl(t))
    saveCmd.setParseAction(lambda t: ultra_saveCurve(t))
    helpCmd.setParseAction(lambda t: UltraHelp(t.arg)) 

    while True:
        if quitTime.timeToQuit == 1:
            break
        if (GetUltraScript() != ""):
            cmd = "runscript %s" %GetUltraScript()
            SetUltraScript("")
        else: 
            cmd = raw_input('U-> ')
        if cmd.split()[0] == 'runscript':
            ultrascript.parseFile(cmd.split()[1])
            continue
        try:
            ulRes = ultracommand.parseString(cmd)
        except ParseException, err: 
            #print 'Exception (%s) while parsing command: %s' %(err,cmd)
            UltraUsage(cmd.split()[0])
            continue




runUltraWrapper()
