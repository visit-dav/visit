import os, sys, inspect

try:
    import pyparsing
except:
    raise VisItException("LoadUltra requires PyParsing to be installed in python's site-packages.")

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

#-----------------------------------------------------------------------------
# Creates cmfe with the 2 curves (c1 being the 'active db')
#
# Modifications:
#   Kathleen Bonnell, Tue Feb 22 15:45:58 PST 2011
#   Take into account expressions attached to the curve
#-----------------------------------------------------------------------------

def createCurveCMFE(c1, c2):
    var1 = "<%s>" % c1.getUseVar()
    var2 = "<%s:%s>" % (c2.fileName, c2.getUseVar()) 
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
        print(menuList) 
    elif t.cmd == 'lst' and selectedList.curves != []:
        print(selectedList)
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
#
# Modifications:
#   Kathleen Bonnell, Tue Feb 22 16:36:52 PST 2011
#   Make use of new curve method 'getUseVar'
#-----------------------------------------------------------------------------
def ultra_doOp_percurve(curve, t):
    currentPlot = setAsActivePlot(curve.plotId)
    if (currentPlot == -1):
        return 

    curve.modified = True

    usevar = "<%s>"%curve.getUseVar()
    if t.cmd in mathOpsNoArg:
        newvarDef = "%s(%s)" % (t.cmd, usevar)
        if t.cmd == "ln" or t.cmd == "log10":
            newvarDef = "%s(%s, -1.0e38)" % (t.cmd, usevar)
        createAndApplyExpression(curve, newvarDef, t.cmd)

    elif t.cmd in mathOpsXNoArg:
        usecmd = mathOpsNoArg[mathOpsXNoArg.index(t.cmd)]
        newvarDef = "curve_domain(%s, %s(coord(%s)[0]))"  % \
                                 (usevar, usecmd, usevar)
        if usecmd == "ln" or t.cmd == "log10":
            newvarDef = "curve_domain(%s, %s(coord(%s)[0], -1.0e38))"  % \
                                 (usevar, usecmd, usevar)
        createAndApplyExpression(curve, newvarDef, t.cmd)
        ResetView()


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
        secondVar = t.arg
        if t.cmd == 'divy':
            op = "/"
        elif t.cmd == 'dy':
            op = "+"
        elif t.cmd == 'my':
            op = "*"
        elif t.cmd == 'powr':
            op = "^"
        elif t.cmd == 'powa':
            op = "^"
            secondVar = usevar
            usevar = t.arg
        else:
            return 
        newvarDef = "%s %s %s" % (usevar, op, secondVar)
        createAndApplyExpression(curve, newvarDef, t.cmd)

    elif t.cmd in mathOpsXArg:
        newvarDef = ""
        op = ""
        secondVar = t.arg
        if t.cmd == 'divx':
            op = "/"
        elif t.cmd == 'dx':
            op = "+"
        elif t.cmd == 'mx':
            op = "*"
        elif t.cmd == 'powrx':
            op = "^"
        elif t.cmd == 'powax':
            op = "^"
            secondVar = usevar
            usevar = t.arg
        else:
            return 
        newvarDef = "curve_domain(%s, coord(%s)[0] %s %s)" % \
                                 (usevar, usevar, op, secondVar)
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
#
# Modifications:
#   Kathleen Bonnell, Tue Feb 22 14:56:42 PST 2011
#   Activate the DB of the first curve, as it is the base for the cmfe.
#   Stop processing if AddPlot returns an error. 
#   Take into acount expressions that have been applied to a curve.
# 
#-----------------------------------------------------------------------------
def ultra_docmfemath(cl, t):
    if len(cl) < 2:
        print("Usage error: %s curve-list" % t.cmd)
        return 
    curve1 = cl[0]
    # when building the cmfe, we want the first curve to be the active DB
    ActivateDatabase(curve1.fileName)
    fullExpression = "<%s>" % curve1.getUseVar()
    exprName = "%s" % curve1.id
    for i in range(1, len(cl)):
        curve = cl[i]
        exprName += "%s%s" % (t.cmd, curve.id)
        if (curve.getUseVar() not in  curve.expressions):
            cmfe = createCurveCMFE(curve1, curve)
        else:
            cmfe = "<%s>" % curve.getUseVar()
        fullExpression += " %s %s" %(t.cmd, cmfe)
    DefineCurveExpression(exprName, fullExpression)
    newCurve = PlotCurveItem(exprName)
    newCurve.expressions.append(exprName)
    # don't want to do anything more if AddPlot returns an error
    if not AddPlot("Curve", newCurve.var):
        return
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
#
# Modifications:
#   Kathleen Bonnell, Tue Feb 22 15:04:37 PST 2011
#   Ensure the selected curve's DB is active (if not in need of opening).
#
#-----------------------------------------------------------------------------

def ultra_multiCurveNum(t):
    if t.cmd == 'select' and menuList.curves != []: 
        cl =  menuList.slice(t.clist)
        for curve in cl:
            if not curve.fileName in openedDBList:
                openedDBList.append(curve.fileName)
                OpenDatabase(curve.fileName)
            else:
                ActivateDatabase(curve.fileName)
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
    elif t.cmd in mathOpsArg or t.cmd in mathOpsXArg:
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
            print("  %d" % a.axes2D.visible)
        elif t.arg == "on" or t.arg == "off":
            a.axes2D.visible = toggleToInt(t.arg)
            SetAnnotationAttributes(a)

    elif t.cmd == "grid":
        a = GetAnnotationAttributes()
        if not t.arg:
            print("  %d" % a.axes2D.xAxis.grid) 
        elif t.arg == "on" or t.arg == "off":
            a.axes2D.xAxis.grid = toggleToInt(t.arg)
            a.axes2D.yAxis.grid = toggleToInt(t.arg)
            SetAnnotationAttributes(a)

    elif t.cmd == "x-log-scale":
        v = GetViewCurve()
        if not t.arg:
            print("  %d" % v.domainScale)
        elif t.arg == "on":
            v.domainScale = v.LOG
            SetViewCurve(v)
        elif t.arg == "off":
            v.domainScale = v.LINEAR
            SetViewCurve(v)

    elif t.cmd == "y-log-scale":
        v = GetViewCurve()
        if not t.arg:
            print("  %d" % v.rangeScale)
        elif t.arg == "on":
            v.rangeScale = v.LOG
            SetViewCurve(v)
        elif t.arg == "off":
            v.rangeScale = v.LINEAR
            SetViewCurve(v)

    elif t.cmd == "data-id":
        if not t.arg:
            print("  %d" % selectedList.__class__.data_id)
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
    # type not currently supported, always saves as ascii
    #stype = "ascii"
    #if t.type:
    #    stype = t.type
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
            s = GetPlotInformation()['Curve']
            FILE.write("# %s\n"%curve.var) 
            for i in range(0, len(s), 2):
                FILE.write("%13.6e %13.6e\n"%(s[i], s[i+1]))
    FILE.close()

#-----------------------------------------------------------------------------
# Execution of wrapper
#
# Modifications:
#   Kathleen Bonnell, Tue Feb 22 14:56:42 PST 2011
#   Remove exception from message printed on error. 
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
            cmd = input('U-> ')
        
        if cmd != '': 
            if cmd.split()[0] == 'runscript':
                ultrascript.parseFile(cmd.split()[1])
                continue
            try:
                ulRes = ultracommand.parseString(cmd)
            except ParseException as err: 
                # the actual exception is not useful for users 
                #print 'Exception (%s) while parsing command: %s' %(err,cmd)
                print('Exception while parsing command: %s' %(cmd))
                UltraUsage(cmd.split()[0])
                continue




runUltraWrapper()
