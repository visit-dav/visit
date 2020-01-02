#
# This is vdbck, like fsck for a VisIt Database
#
# This tool is designed to iterate over all the objects in a database
# and validate that VisIt can process them by doing plots involving
# the objects. It reports cases that fail. This is intended to be a
# way for users to sanity check that they are creating a database
# VisIt is capable of reading or indicate which object(s) in the database
# VisIt encounters errors processing.
#
# Mark C. Miller, 01Apr17
#
import sys, string, atexit
import argparse
from os import path as ospath

# TO DO:
#     3. html output of results
#     4. dump results to different dirs
#
# Codify the set of database objects we're going to iterate on.
# For each object, we need to know the name of a suitable plot,
# the name of the list in metadata, the name of the expression
# type for expressions.
#
visitDbObjectMap = {
    "Mesh":{
        "plotName":"Mesh",
        "mdName":"Meshes",
        "exName":"Mesh"
    },
    "Scalar":{
        "plotName":"Pseudocolor",
        "mdName":"Scalars",
        "exName":"ScalarMeshVar"
    },
    "Vector":{
        "plotName":"Vector",
        "mdName":"Vectors",
        "exName":"VectorMeshVar"
    },
    "Tensor":{
        "plotName":"Tensor",
        "mdName":"Tensors",
        "exName":"TensorMeshVar"
    },
    "Symmetric Tensor":{
        "plotName":"Tensor",
        "mdName":"SymmTensors",
        "exName":"SymmetricTensorMeshVar"
    },
    "Array":{
        "plotName":"Label",
        "mdName":"Arrays",
        "exName":"ArrayMeshVar"
    },
    "Label":{
        "plotName":"Label",
        "mdName":"Labels",
        "exName":""
    },
    "Material":{
        "plotName":"FilledBoundary",
        "mdName":"Materials",
        "exName":"Material"
    },
    "Curve":{
        "plotName":"Curve",
        "mdName":"Curves",
        "exName":"CurveMeshVar"
    }
}

class lastError:
     def __init__(self):
         self.lastMsg = ""
     def message(self):
         if self.lastMsg == GetLastError():
             return ""
         else:
             self.lastMsg = GetLastError()
             return self.lastMsg

lastErrorCache = lastError()

class phaseStack:
     def __init__(self):
         self.items = []

     def isEmpty(self):
         return self.items == []

     def push(self, item):
         self.items.append(item)

     def pop(self):
         return self.items.pop()

     def peek(self):
         return self.items[len(self.items)-1]

     def size(self):
         return len(self.items)

log = []
logPhaseStack = phaseStack()
badMeshNames = []

def isTrue(testVal):
    if testVal:
        return True
    return False

def isFalse(testVal):
    if not testVal:
        return True
    return False


def OutputLog():
    if clArgs.no_verbose:
        return
    f = open("check_visit_db.out","w")
    for line in log:
        f.write(line)
    f.close()

def Fail():
    OutputLog()
    sys.exit(1)

def logIndentLen():
    return int(4*logPhaseStack.size())

def logIndent():
    return " "*logIndentLen()

def StartPhase(phaseName):
    if clArgs.no_verbose:
        return
    log.append("%s%s\n"%(logIndent(),"*"*(120-logIndentLen())))
    log.append("%s%s\n"%(logIndent(),phaseName))
    log.append("%s%s\n"%(logIndent(),"*"*(120-logIndentLen())))
    logPhaseStack.push(phaseName)

def EndPhase():
    if clArgs.no_verbose:
        return
    logPhaseStack.pop()

def LogStatus(objName, retVal, errMsg, statusStr, failSeverity):
    if clArgs.no_verbose and statusStr == "PASSED":
        return
    log.append("%s%s %s %s %s %s\n"%(
        logIndent(),
        '{:20s}'.format(objName),
        '{:30s}'.format(str(retVal).replace("\n"," ")[:30]),
        '{:30s}'.format(errMsg),
        '{:10s}'.format(statusStr),
        '{:10s}'.format(failSeverity)))

def doVisItCLIOp(func, args, passFunc, failSeverity):
    retVal = None
    try:
        retVal = func(*args)
        errMsg = lastErrorCache.message()
        if passFunc(retVal):
            LogStatus(func.__name__, retVal, errMsg, "PASSED", failSeverity)
        else:
            LogStatus(func.__name__, retVal, errMsg, "FAILED", failSeverity)
            if failSeverity == "Fatal":
                Fail()
    except:
        LogStatus(func.__name__, retVal, errMsg, "EXCEPTION", failSeverity)
        if failSeverity == "Fatal":
            Fail()
    return retVal

def doVisItPlot(plotName, plotVar):
    retVal = 0
    try:
        DeleteAllPlots()
        AddPlot(plotName, plotVar)
        doVisItCLIOp(DrawPlots, (), isTrue, "NonFatal")
        ResetView()
        SaveWindow()
        LogStatus(plotName, "", plotVar, "PASSED", "NonFatal")
    except:
        LogStatus(plotName, "", plotVar, "WARNING", "NonFatal")
        retVal = 1
    return retVal
    
#
# Report all metadata objects that are not valide
#
def checkNotValidVars(md):
    retVal = 0
    StartPhase("Invalid variables")
    for oClass in visitDbObjectMap:
        StartPhase("Invalid %s variables"%oClass)
        mdName = visitDbObjectMap[oClass]["mdName"]
        numObjsInClass = getattr(md, "GetNum%s"%mdName)()
        for i in range(numObjsInClass):
            objMd = getattr(md, "Get%s"%mdName)(i)
            if not objMd.validVariable:
                retVal = 1
                LogStatus(objMd.name, "", "Invalid %s"%oClass, "WARNING", "NonFatal")
                if oClass == "Mesh":
                    badMeshNames.append(objMd.name)
        EndPhase()
    EndPhase()
    return retVal
       
#
# Test all variables taking care to avoid invalid variables
#
def checkVarsInDatabase(md):
    retVal = 0
    StartPhase("Plotting variables")
    for oClass in visitDbObjectMap:
        StartPhase("Plotting %s variables"%oClass)
        plotName = visitDbObjectMap[oClass]["plotName"]
        mdName = visitDbObjectMap[oClass]["mdName"]
        numObjsInClass = getattr(md, "GetNum%s"%mdName)()
        for i in range(numObjsInClass):
            objMd = getattr(md, "Get%s"%mdName)(i)
            if not objMd.validVariable:
                continue
            if doVisItPlot(plotName, objMd.name) != 0:
                retVal = 1
                if oClass == "Mesh":
                    badMeshNames.append(objMd.name)
        EndPhase()
    EndPhase()
    return retVal

#
# Test all expressions taking care to skip auto-expressions
#
def checkExprsInDatabase(md):
    retVal = 0
    exprList = md.GetExprList()
    if not exprList.GetNumExpressions():
        return
    StartPhase("Plotting expressions")
    for oClass in visitDbObjectMap:
        StartPhase("Plotting %s expressions"%oClass)
        plotName = visitDbObjectMap[oClass]["plotName"]
        exprTypeName = visitDbObjectMap[oClass]["exName"]
        if not exprTypeName:
            EndPhase()
            continue
        exprType = getattr(md.GetExprList().GetExpressions(0),exprTypeName)
        for i in range(exprList.GetNumExpressions()):
            expr = exprList.GetExpressions(i)
            if expr.GetAutoExpression():
                continue
            if expr.GetType() != exprType:
                continue
            if doVisItPlot(plotName, expr.GetName()) != 0:
                retVal = 1
        EndPhase()
    EndPhase()
    return retVal

def checkSILsInDatabase(md):
    retVal = 0
    StartPhase("Subset defined by SILs")
    numMeshes = md.GetNumMeshes()
    for i in range(numMeshes):
        meshMd = md.GetMeshes(i)
        if not meshMd.validVariable:
            continue
        if meshMd.GetName() in badMeshNames:
            continue
        DeleteAllPlots()
        AddPlot("Mesh", meshMd.GetName())
        DrawPlots()
        ResetView()
        silr = SILRestriction()
        cats = silr.Categories()
        print(cats)
        for c in cats:
            catSets = silr.SetsInCategory(c)
            if len(catSets) <= 1:
                LogStatus(c, "", "", "SKIPPED", "NonFatal")
                continue
            aset = silr.SetsInCategory(c)[0]
            silr.TurnOffSet(aset)
            try:
                SetPlotSILRestriction(silr)
                SaveWindow()
                silr.TurnOnAll()
                LogStatus(c, "", silr.SetName(aset), "PASSED", "NonFatal")
                SetPlotSILRestriction(silr)
            except:
                retVal = 1
                LogStatus(c, "", silr.SetName(aset), "FAILED", "NonFatal")
    EndPhase()
    return retVal

def checkMdDiffs(md0, md1):
    retVal = 0
    StartPhase("Metadata time variation")
    for oClass in visitDbObjectMap:
        mdName = visitDbObjectMap[oClass]["mdName"]
        numObjs0 = getattr(md0, "GetNum%s"%mdName)()
        numObjs1 = getattr(md1, "GetNum%s"%mdName)()
        getObjs0 = getattr(md0, "Get%s"%mdName)
        getObjs1 = getattr(md1, "Get%s"%mdName)
        objNames0 = set(getObjs0(i).GetName() for i in range(numObjs0) if getObjs0(i).GetValidVariable())
        objNames1 = set(getObjs1(i).GetName() for i in range(numObjs1) if getObjs1(i).GetValidVariable())
        if objNames0 != objNames1:
            retVal = 1
            LogStatus(oClass, "", "", "FAILED", "NonFatal")
    EndPhase()
    return retVal

def checkFile(dbName):
    try:
        fp = open(dbName)
        fp.close()
    except:
        LogStatus("File check", "", "", "FAILED", "Fatal")
        Fail()

def checkDatabase(dbName, timeStates):
    nerrors = 0
    allTimeStates = False
    initTimeState = 0
    baseDbName = ospath.basename(dbName)

    if timeStates[0] == -1:
        allTimeStates = True
    else:
        initTimeState = timeStates[0]

    #
    # Confirm file is readable
    #
    checkFile(dbName)
    StartPhase("Database \"%s\""%ospath.basename(dbName))

    #
    # Get database metadata
    #
    md = doVisItCLIOp(GetMetaData, (dbName, initTimeState), isTrue, "Fatal")
    moreThanOneTimeState = False
    if len(md.GetTimes()) > 1:
        moreThanOneTimeState = True
    if allTimeStates:
        timeStates = list(md.GetTimes())

    #
    # Open the database
    #
    doVisItCLIOp(OpenDatabase, (dbName, initTimeState), isTrue, "Fatal")

    for t in timeStates:

        if moreThanOneTimeState:
            StartPhase("Time state %d"%t)
            ReOpenDatabase(dbName)
            TimeSliderSetState(t)

        newMd = doVisItCLIOp(GetMetaData, (dbName, t), isTrue, "NonFatal")

        #
        # Report differences in Metadata
        #
        nerrors = nerrors + checkMdDiffs(md, newMd)

        #
        # Report any invalid vars in md
        #
        nerrors = nerrors + checkNotValidVars(md)

        #
        # Test plotting all the variables in the database
        #
        nerrors = nerrors + checkVarsInDatabase(md)

        #
        # Test plotting all expressions in the database
        #
        nerrors = nerrors + checkExprsInDatabase(md)

        #
        # Test SIL Restrictions
        #
        nerrors = nerrors + checkSILsInDatabase(md)

        md = newMd
        if moreThanOneTimeState:
            EndPhase() # time states
        else:
            break

    EndPhase() # Database

    OutputLog()

    if nerrors > 0:
        sys.exit(1)
    sys.exit(0)

#
# Main
#
atexit.register(OutputLog)

clParser = argparse.ArgumentParser(description="VisIt Database Consistency Check.")

clParser.add_argument("dbpath", type=str,
    help="The name of a VisIt database to check")

clParser.add_argument("-t", "--time-states", type=int, nargs="*", default=[0], metavar="I",
    help="Time state indices to check. -1=All.")

clParser.add_argument("--no-verbose", action="store_true",
    help="Turn off verbose reporting and report only failures.")

clParser.add_argument("--no-images", action="store_true",
    help="Turn off keeping of saved images as they are produced.")

clArgs = clParser.parse_args()

swa = GetSaveWindowAttributes()
swa.fileName = "vdb_check_image"
if clArgs.no_images:
    swa.outputDirectory = "/dev"
    swa.outputToCurrentDirectory = 0
    swa.family = 0
    swa.fileName = "null"
SetSaveWindowAttributes(swa)

checkDatabase(clArgs.dbpath,clArgs.time_states)
