import string
from pyparsing import *

            
#-----------------------------------------------------------------------------
# Operations/commands
#-----------------------------------------------------------------------------
mathOpsNoArg = ["sin", "cos", "tan", "atan", "asin", "acos", "abs", "ln", \
                "exp", "log10", "sqr", "sqrt", "sinh", "cosh", "tanh"]
mathOpsXNoArg =["%sx" %el for el in mathOpsNoArg]

mathOpsLongName = ["sine", "cosine", "tangent", "ArcTan", "ArcSin", "ArcCos", "absolute value", "natural log", "exponential", "base 10 log", "square", "square root", "hyperbolic sine", "hyperbolic cosine", "hyperbolic tangent"]


cmfeOps = ["+", "-", "*", "/"]
cmfeOpsLongName = ["sum", "difference", "product", "quotient"]

mathOpsArg = ["dy", "divy", "ymin", "ymax", "my"]
mathOpsXArg = ["%s" %el.replace('y', 'x') for el in mathOpsArg]

mathOpsArg +=["powr", "powa"]
mathOpsXArg += ["powrx", "powax"]

mathOpsArgLongName = ["Shift", "Divide", "", "", "Scale", "", ""]

opsArg = ["lnwidth"]
togglePlotOps = ["axis", "data-id", "grid", "x-log-scale", "y-log-scale"]
rangePlotOps = ["domain", "range"]
saveOps = ["save", "wrt"]
singletonOps = ["menu", "lst", "erase", "quit", "end"]

lnStyleOptions = ["solid", "dotted", "dashed", "dotdashed"]



#-----------------------------------------------------------------------------
# create the help/usage dictionary
#-----------------------------------------------------------------------------
cmdHelp = {}


def AddMathOps():
    """ Adds math operators to the help dictionary
    """
    tempDict = dict(zip(mathOpsNoArg, mathOpsLongName) + \
                    zip(mathOpsXNoArg, mathOpsLongName))
    for cmd in mathOpsNoArg + mathOpsXNoArg:
        if cmd in mathOpsNoArg:
            yc = "y"
        else:
            yc = "x"
        usg = "Usage: %s <curve-list>" %cmd
        proc = "Procedure: Take %s of %s values of curves" %(tempDict[cmd], yc)
        cmdHelp[cmd] = [proc, usg]


    tempDict = dict(zip(mathOpsArg, mathOpsArgLongName) + \
                    zip(mathOpsXArg, mathOpsArgLongName))

    for cmd in mathOpsArg + mathOpsXArg:
        if cmd in mathOpsArg:
            yc = "y"
        else:
            yc = "x"
        if cmd == "ymin" or cmd == "ymax" or cmd == "xmin" or cmd == "xmax":
            if cmd == "ymin" or cmd == "xmin":
                log = "<"   
            else: 
                log = ">"   
            usg = "Usage: %s <curve-list> <limit>" %cmd
            proc = "Procedure: Filter out point in curves whose %s-values"%yc
            proc += " %s limit." % log

        elif cmd == "powr" or cmd == "powrx": 
            usg = "Usage: %s <curve-list> <a>" %cmd
            proc = "Procedure: Raise %s values of curves to a power, %s=%s^a"\
                   %(yc,yc,yc) 
        elif cmd == "powa" or cmd == "powax":
            usg = "Usage: %s <curve-list> <a>" %cmd
            proc = "Procedure: Raise a to the power of the %s values of" %yc
            proc += " curves, %s=a^%s" %(yc,yc) 
        else:
            usg = "Usage: %s <curve-list> <value>" %cmd
            proc = "Procedure: %s %s values of curves by a constant" \
                    %(tempDict[cmd], yc)
        cmdHelp[cmd] = [proc, usg]

    tempDict = dict(zip(cmfeOps, cmfeOpsLongName)) 
    for cmd in cmfeOps:
        usg = "Usage: %s <curve-list>" %cmd
        proc = "Procedure: Take %s of curves" % tempDict[cmd]
        cmdHelp[cmd] = [proc, usg]

def AddToggleOps():
    """ Adds plot operators that are toggles to the help dictionary
    """
    for cmd in togglePlotOps:
        usg = "Usage: %s on | off"%cmd
        proc = "Variable: "
        if   cmd == "axis":
            proc += "Controls drawing the axes"
        elif cmd == "data-id":
            proc += "Controls display of curve identifiers on graph"
        elif cmd == "grid":
            proc += "Controls display of grid lines in graph"
        elif cmd == "x-log-scale":
            proc += "Controls log scale on x axis"
        elif cmd == "y-log-scale":
            proc += "Controls log scale on y axis"
        cmdHelp[cmd] = [proc, usg]

def AddRangeOps():
    """ Adds plot operators that require ranges to the help dictionary
    """
    for cmd in rangePlotOps:
        usg = "Usage: %s <low-lim> <high-lim> or %s de "%(cmd, cmd)
        proc = "Procedure: Set the %s for plotting"%cmd
        cmdHelp[cmd] = [proc, usg]

def CreateHelp():
    # groupd ids
    cmdHelp["io"] = ["",""]
    cmdHelp["ma"] = ["",""]
    cmdHelp["m+"] = ["",""]
    cmdHelp["ie"] = ["",""]
    #cmdHelp["ic"] = ["",""]
    cmdHelp["ce"] = ["",""]
    cmdHelp["cp"] = ["",""]
    cmdHelp["cc"] = ["",""]
    #individual commands
    cmdHelp["select"] = ["Procedure: Select curves from the menu for plotting"\
                         ,"Usage: select <list-of-menu-numbers>"]
    cmdHelp["menu"] = ["Macro: List the available curves", \
                       "Usage: menu"]
    cmdHelp["lst"] = ["Macro: Display curves in list",\
                      "Usage: lst"]
    cmdHelp["erase"] = ["Macro: Erases all curves on the screen",\
                        "Usage: erase"]
    cmdHelp["del"] = ["Procedure: Delete curves from list", \
                      "Usage: del <curve-list>"]
    cmdHelp["end"] = ["Procedure: Exit ULTRA command wrapper", \
                      "Usage: end"]
    cmdHelp["quit"] = ["Procedure: Exit ULTRA command wrapper", \
                      "Usage: quit"]
    cmdHelp["rd"] = ["Macro: Read curve data file", \
                      "Usage: rd <\"file-name\">"]
    cmdHelp["lnstyle"] = ["Procedure: Set the line styles of curves", \
             "Usage: lnstyle <curve-list> solid | dotted | dashed |dotdashed"]
    cmdHelp["lnwidth"] = ["Procedure: Set the line widths of curves", \
             "Usage: lnstyle <curve-list> <width-number>"]
    cmdHelp["save"] = ["Macro: Save curves to file", \
             "Usage: save [<type>] <\"file-name\"> <curve-list>"]
    cmdHelp["wrt"] = ["Macro: Save curves to file", \
             "Usage: wrt [<type>] <\"file-name\"> <curve-list>"]
    AddMathOps()
    AddToggleOps()
    AddRangeOps()

def UltraUsage(cmd):
    if cmd in cmdHelp:
        print cmdHelp[cmd][1]
    else:
        print "%s not valid command or not currently supported"%cmd

def UltraHelp(cmd):
    if cmd == "":
        print " io  - I/O Commands"
        print " ma  - Math Operations Which Do Not Generate a New Curve"
        print " m+  - Math Operations Which Do Generate a New Curve"
        print " ie  - Environmental Inquiry Commands"
        #print " ic  - Curve Inquiry Commands"
        print " ce  - Environemtal Control Commands"
        print " cp  - Plot Control Commands"
        print " cc  - Curve Control Commands"
        #print " cn  - Constants"
        #print " var - Variables"
        #print " sy  - Synonyms"
        print " To list the member of a group, enter:    help <group-id>"
        #print " To list the member of all groups, enter: help all"
        print " For information about a command, enter:  help <command-name>"
    elif cmd == "io":
        print " I/O Commands"
        print "   rd save"
    elif cmd == "ma":
        print " Math Operations Which Do Not Generate a New Curve:"
        a = mathOpsNoArg + mathOpsXNoArg + mathOpsArg + mathOpsXArg
        a.sort() 
        print "   ", string.join(a)
    elif cmd == "m+":
        print " Math Operations Which Do Generate a New Curve:"
        a = cmfeOps
        a.sort() 
        print "   ", string.join(a)
    elif cmd == "ie":
        print " Environment Inquiry Commands:"
        print "   lst menu"
    elif cmd == "ce":
        print " Environment Control Commands:"
        print "   end erase quit"
    elif cmd == "cp":
        print " Plot Control Commands:"
        a = togglePlotOps + rangePlotOps
        a.sort() 
        print "   ", string.join(a)
    elif cmd == "cc":
        print " Curve Control Commands:"
        print "   del lnstyle lnwidth select"
    elif cmd in cmdHelp:
        print "    %s" %cmdHelp[cmd][0]
        print "    %s" %cmdHelp[cmd][1]
    

CreateHelp()

#-----------------------------------------------------------------------------
# Grammar
#-----------------------------------------------------------------------------

fileName = quotedString.setParseAction(removeQuotes) | \
           dblQuotedString.setParseAction(removeQuotes)

signedInt = Optional("-") + Word(nums)
decimalPart = "." + Word(nums)
real = Combine(signedInt + Optional(decimalPart)).setParseAction(lambda t: float(t[0]))

# curves and curve lists
curveItemNum   = Word("123456789",max=2)
curveSelectNum = Combine(curveItemNum + ':' + curveItemNum) | curveItemNum
curveListNum   = OneOrMore(curveSelectNum)

curveItemAlpha   = Word(srange("[A-Z]"),exact=1)
curveSelectAlpha = Combine(curveItemAlpha + ':' + curveItemAlpha) | \
                   curveItemAlpha 
curveListAlpha   = OneOrMore(curveSelectAlpha)

toggle =  oneOf("""on off""")
limits = Group(real + real)
default = oneOf(""" de """)
rangeOption = limits | default

#commands
unaryFileCommand = oneOf("""rd """).setResultsName("cmd")

singletonCommand = oneOf(singletonOps).setResultsName("cmd")


# reserved for future use
# unaryCurveCommand = oneOf("""fft normalize""")
# binaryCurveCommand = oneOf("""diff-measure""")

#operates on 'menu' curves
multiCurveCommandNum = oneOf("""select""").setResultsName("cmd")

#operates on 'selected' curves
multiCurveCommandAlpha = oneOf("""del """    + \
                               " ".join(\
                                        mathOpsNoArg + \
                                        mathOpsXNoArg + \
                                        cmfeOps  + \
                                        mathOpsArg + \
                                        mathOpsXArg + \
                                        opsArg      \
                                        ) \
                               ).setResultsName("cmd") 

lnstylecmd = oneOf("""lnstyle""").setResultsName("cmd") + \
             curveListAlpha.setResultsName("clist") + \
             oneOf(lnStyleOptions).setResultsName("arg")


#ops
fileOp = unaryFileCommand + fileName.setResultsName("path")

# reserved for future use
# unaryCurveOp = unaryCurveCommand + curveItemAlpha
# binaryCurveOp = unaryCurveCommand + curveItemAlpha + curveItemAlpha 

multiCurveOpNum = multiCurveCommandNum   + \
                  curveListNum.setResultsName("clist")


multiCurveOpAlpha = multiCurveCommandAlpha + \
                    curveListAlpha.setResultsName("clist") + \
                    Optional(real).setResultsName("arg") 


# ops on plots
plotOp = (oneOf(togglePlotOps).setResultsName("cmd") + \
          Optional(toggle).setResultsName("arg")) | \
          (oneOf(rangePlotOps).setResultsName("cmd")  + \
          Optional(rangeOption).setResultsName("arg")) 

#save
saveCmd = oneOf(saveOps).setResultsName("cmd") + \
          Optional(oneOf("""ascii pdb""")).setResultsName("type") + \
          fileName.setResultsName("file") + \
          curveListAlpha.setResultsName("clist")

helpCmd = oneOf("""help""").setResultsName("cmd") + \
             Optional(oneOf(cmdHelp.keys())).setResultsName("arg")

# head
ultracommand = lnstylecmd | multiCurveOpAlpha | multiCurveOpNum | fileOp | \
               singletonCommand  | plotOp | saveCmd | helpCmd


ultrascript = ZeroOrMore(ultracommand)
#  end of grammar

