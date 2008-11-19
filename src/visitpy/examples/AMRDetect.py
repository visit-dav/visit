###############################################################################
#
# Purpose: Use VisIt CLI to automatically detect and compute AMR mesh
#          configuration given a multi-domain mesh of pieces. This code will
#          compute the parent-child hierarchy, refinement ratios and logical
#          extents of the mesh pieces for both rectilinear and curvilinear
#          meshes. 
#
#          This is very experimental and the code is not at all well
#          structured.
#
# Usage: visit -cli -nowin -s AMRDetect.py -db_name <db-name> -mesh_name <mesh_name>
#
# Programmer: Mark C. Miller
# Date:       Tue Oct  7 10:01:50 PDT 2008
# 
# Modifications:
#
#   Mark C. Miller, Mon Oct 27 22:05:58 PDT 2008
#   Fixed some bugs due to typos introduced by cut-paste-modify errors.
#   Changed domain rank compuation to average zone area (or volume in 3D)
#   instead of whole domain. Changed algorithm using spatial extents for
#   computing overlap candidates to use <= instead of just < operator.
#   Note that there is still an issue with assigning domains to levels when
#   some coarse domains have no children while other coarse domains have
#   children. The algorithm needs to be modified to first assign domains
#   with children to levels and then assign UNassigned domains which are
#   neighbors of assigned domains to the same level.
#
##############################################################################
import string, re, sys, copy

meshName=""
db=""

try:
    i = 1
    while i < len(sys.argv):
        if sys.argv[i] == "-db_name":
            i = i + 1
            db = sys.argv[i]
        elif sys.argv[i] == "-mesh_name":
            i = i + 1
            meshName = sys.argv[i]
        i = i + 1
except:
    print "Usage: AMRDetect.py -db_name <db-name> -mesh_name <mesh_name>"
    sys.exit(1)

if db == "" or meshName == "":
    print "Usage: AMRDetect.py -db_name <db-name> -mesh_name <mesh_name>"
    sys.exit(1)

def GetMeshLogicalDims(metadata, meshname):
    dim = -1
    for mi in range(metadata.GetNumMeshes()):
        if metadata.GetMeshes(mi).name == meshname:
            dim = metadata.GetMeshes(mi).topologicalDimension
            break
    return dim 

def MeshIsRectilinear(meshname):
    md = GetMetaData(db)
    for mi in range(md.GetNumMeshes()):
        if md.GetMeshes(mi).name == meshname:
            return md.GetMeshes(mi).meshType == \
	           md.GetMeshes(mi).AVT_RECTILINEAR_MESH

def SetHiddenPlotState(plotVarName, hiddenState):
    pl = GetPlotList()
    for p in range(pl.GetNumPlots()):
        plot = pl.GetPlots(p)
        if plot.plotVar == plotVarName:
            if plot.hiddenFlag != hiddenState:
	        SetActivePlots((p,))
		HideActivePlots()
		DrawPlots()
		return

def SwitchPlotsTo(plotVarName):
    SetHiddenPlotState(meshName, 1)
    SetHiddenPlotState("one", 1)
    SetHiddenPlotState("overlapMap", 1)
    SetHiddenPlotState("maxe", 1)
    SetHiddenPlotState(plotVarName, 0)

OpenDatabase(db)
md = GetMetaData(db)
AddPlot("Mesh", meshName) # plot 0
DefineScalarExpression("one","cell_constant(%s, 1.0)"%meshName)
AddPlot("Pseudocolor", "one") # plot 1
DefineScalarExpression("maxe","max_edge_length(%s)"%meshName)
AddPlot("Pseudocolor", "maxe") # plot 2
DrawPlots()

DefineScalarExpression("overlapMap", "pos_cmfe(<%s_onedom_tmp.silo:one>,%s,0.0)"%(meshName,meshName))

meshDim = GetMeshLogicalDims(md, meshName)

silr = SILRestriction()
silr.TurnOffAll()
SetPlotSILRestriction(silr)

SuppressQueryOutputOn()
SuppressMessages(3)
pa = GetPickAttributes()
pa.showNodeDomainLogicalCoords = 1
pa.showNodeBlockLogicalCoords = 1
pa.showZoneId = 1
pa.showZoneDomainLogicalCoords = 0
pa.showZoneBlockLogicalCoords = 0
pa.showTimeStep = 0
pa.showMeshName = 0
pa.displayPickLetter = 0
pa.timePreserveCoord = 0
SetPickAttributes(pa)

exopts = GetExportOptions("Silo")
exopts['Single File'] = 1
exdb = ExportDBAttributes()
exdb.db_type = "Silo"
exdb.filename = "%s_onedom_tmp"%meshName
exdb.variables = ("one")

def LinToLogId(nDomId, entId, isNodeId):
    global meshDim
    global logSizes

    if meshDim == 2:
        NumZonesX = logSizes[nDomId][0]
        NumNodesX = NumZonesX+1
        if isNodeId:
            J = entId / NumNodesX
            I = entId % NumNodesX
        else:
            J = entId / NumZonesX
            I = entId % NumZonesX
        K = 0
    else:
        NumZonesX = logSizes[nDomId][0]
        NumZonesY = logSizes[nDomId][1]
        NumNodesX = NumZonesX+1
        NumNodesY = NumZonesY+1 
        if isNodeId:
            K = entId / (NumNodesX * NumNodesY)
            R = entId % (NumNodesX * NumNodesY)
            J = R / NumNodesX
            I = R % NumNodesX
        else:
            K = entId / (NumZonesX * NumZonesY)
            R = entId % (NumZonesX * NumZonesY)
            J = R / NumZonesX
            I = R % NumZonesX

    return (I,J,K)

def LogToLinId(nDomId, logId, isNodeId):
    global meshDim
    global logSizes

    if meshDim == 2:
        I,J = logId
        NumZonesX = logSizes[nDomId][0]
        NumNodesX = NumZonesX+1
        if isNodeId:
            theId = J*NumNodesX + I
        else:
            theId = J*NumZonesX + I
    else:
        I,J,K = logId
        NumZonesX = logSizes[nDomId][0]
        NumZonesY = logSizes[nDomId][1]
        NumNodesX = NumZonesX+1
        NumNodesY = NumZonesY+1 
        if isNodeId:
            theId = K*NumNodesX*NumNodesY + J*NumNodesX + I
        else:
            theId = K*NumZonesX*NumZonesY + J*NumZonesX + I

    return theId

def GetCornerPointsForZone(nDomId, vDomId, zoneId):
    global meshDim
    global logSizes

    cPoints = []
    nodeIds = ()
    if meshDim == 2:
        NumZonesX = logSizes[nDomId][0]
        J = zoneId / NumZonesX
        I = zoneId % NumZonesX
        NumNodesX = NumZonesX+1
        nodeIds = (J*NumNodesX+I,J*NumNodesX+(I+1),(J+1)*NumNodesX+I,(J+1)*NumNodesX+(I+1))
    else:
        NumZonesX = logSizes[nDomId][0]
        NumZonesY = logSizes[nDomId][1]
        K = zoneId / (NumZonesX * NumZonesY)
        R = zoneId % (NumZonesX * NumZonesY)
        J = R / NumZonesX
        I = R % NumZonesX
        NumNodesX = NumZonesX+1
        NumNodesY = NumZonesY+1 
        nodeIds = (K*NumNodesX*NumNodesY+J*NumNodesX+I, K*NumNodesX*NumNodesY+J*NumNodesX+(I+1), \
                   K*NumNodesX*NumNodesY+(J+1)*NumNodesX+I, (K+1)*NumNodesX*NumNodesY+J*NumNodesX+I, \
                   (K+1)*NumNodesX*NumNodesY+(J+1)*NumNodesX+(I+1))
    for nodeId in nodeIds:
        PickByNode(nodeId, vDomId)
        s = GetPickOutput()
        vals = re.search("Point: <([0-9eE+-\.]*), ([0-9eE+-\.]*)(, ([0-9eE+-\.]*))?>",s)
        if meshDim == 2:
            cPoints.append((float(vals.group(1)), float(vals.group(2))))
        else:
            cPoints.append((float(vals.group(1)), float(vals.group(2)), float(vals.group(4))))

    return cPoints


# Ensure just the mesh plot is visible and active
logSizes = []
spatExtents = []
domRankMap = [] 
n2vDomMap = []
nDomId = 0
SwitchPlotsTo(meshName)
for d in silr.SetsInCategory("domains"):

    # Turn on just this domain
    silr.TurnOnSet(d)
    SetPlotSILRestriction(silr, 1)

    # Get this domain's logical extents by doing a node-pick
    # on the highest numbered node
    Query("NumZones","actual")
    zoneCount = int(GetQueryOutputValue())
    Query("NumNodes","actual")
    v = int(GetQueryOutputValue())
    print "Domain %d, nodeId %d"%(d, v-1)
    PickByNode(v-1,d)
    s = GetPickOutput()
    vals = re.search("Node: *([0-9]*) domain <([0-9]*), ([0-9]*)(, ([0-9]*))?>", s)
    if meshDim == 2:
        print "Logical dimensions = %d, %d"%(int(vals.group(2)), int(vals.group(3)))
	logSizes.append((int(vals.group(2)), int(vals.group(3))))
    else:
        print "Logical dimensions = %d, %d, %d"%(int(vals.group(2)), int(vals.group(3)), int(vals.group(5)))
	logSizes.append((int(vals.group(2)), int(vals.group(3)), int(vals.group(5))))
    Query("SpatialExtents","actual")
    s = GetQueryOutputString()
    vals = re.search("The actual extents are \(([0-9eE+-\.]*), ([0-9eE+-\.]*), ([0-9eE+-\.]*), ([0-9eE+-\.]*)(, ([0-9eE+-\.]*), ([0-9eE+-\.]*))?\)$", s)
    if meshDim == 2:
        print "Spatial Extents = %f, %f, %f, %f"%(float(vals.group(1)), float(vals.group(2)),
	    float(vals.group(3)), float(vals.group(4)))
        spatExtents.append((float(vals.group(1)), float(vals.group(2)),
	                    float(vals.group(3)), float(vals.group(4))))
    else:
        print "Spatial Extents = %f, %f, %f, %f, %f, %f"%(float(vals.group(1)), float(vals.group(2)),
	    float(vals.group(3)), float(vals.group(4)), float(vals.group(6)), float(vals.group(7)))
        spatExtents.append((float(vals.group(1)), float(vals.group(2)),
	                    float(vals.group(3)), float(vals.group(4)),
			    float(vals.group(6)), float(vals.group(7))))

    if meshDim == 2:
        Query("2D area")
        rank = float(GetQueryOutputValue()/zoneCount)
    else:
        Query("Volume")
        rank = float(GetQueryOutputValue()/zoneCount)
    domRankMap.append((nDomId,rank))
    print "Rank = %f"%rank

    # Turn this domain back off
    print "\n"
    silr.TurnOffSet(d)
    n2vDomMap.append(d)
    nDomId = nDomId + 1

def n2vDomId(nDomId):
    global n2vDomMap
    if nDomId >= len(n2vDomMap):
        raise
    return n2vDomMap[nDomId]

def v2nDomId(vDomId):
    global n2vDomMap
    return n2vDomMap.index(vDomId)

def CompareDomRanks(a, b):
    if a[1] < b[1]:
        return -1
    elif a[1] == b[1]:
        return 0
    else:
        return 1

#
# Sort domain ids in descending order of rank 
# 
domRankMap.sort(CompareDomRanks)
domRankMap.reverse()
domOrder = []
for dr in domRankMap:
    domOrder.append(dr[0])
domRankOrder = tuple(domOrder)
print "Domain rank order...", 
for dr in domRankOrder:
    print "%d"%n2vDomId(dr),
print ""

#
# Loop, using spatial extents, to determine which domains
# MAY overlap. Because we do it in rank order from the
# LARGEST domain to the smallest, we are finding, for each
# domain, the other domains which MAY be its children.
#

domDescendants = []
d = 1
for d1 in domRankOrder: 

    exts1 = spatExtents[d1]
    descendants = []

    for d2 in domRankOrder[d:]: 

	#
	# If dom d2 does not overlap spatially with d1, then
	# we can skip it.
	#
        exts2 = spatExtents[d2]
	if exts1[1] <= exts2[0] or \
	   exts2[1] <= exts1[0] or \
	   exts1[3] <= exts2[2] or \
	   exts2[3] <= exts1[2] or (meshDim == 3 and \
	  (exts1[5] <= exts2[4] or \
	   exts2[5] <= exts1[4])):
	   continue

        #
	# If we get here, spatial extents suggest d2 MAY
	# overlap with d1.
	#
        descendants.append(d2)

    domDescendants.append(tuple(descendants))
    d = d + 1

def PrintDescendantsLists(domDescendants):
    d = 0
    mode = 0
    for d1 in domRankOrder: 
        if len(domDescendants[d]) == 0:
            if mode == 0:
                mode = 1
                run = 0
                print "Empty: %d"%n2vDomId(d1),
            else:
                if run >= 40:
                    run = 0
                    print "\n", "    "
                print "%d"%n2vDomId(d1),
                run = run + 1
        else:
            if mode == 1:
                mode = 0
                print ""
            print "Candidate descendants of domain %d"%n2vDomId(d1)
            print "    ",
            run = 0
            for desc in domDescendants[d]:
                if run >= 40:
                    run = 0
                    print "\n", "    "
                print n2vDomId(desc), 
                run = run + 1
            print ""
        d = d + 1
    print ""
    print ""
    print ""

print "************************************************************"
print "************************************************************"
print "Candidate descendants lists as determined by spatial extents"
print "************************************************************"
print "************************************************************"
PrintDescendantsLists(domDescendants)

#
# If the mesh is NOT rectilinear, then the spatial bounding box
# work above serves only to determine CANDIDATES for overlap.
# Whether two domains do indeed overlap will have to be determined
# using pos_cmfe. We compute a 'classifier variable' that is
# a constant value of one everywhere on a specific domain.
# We then use pos_cmfe to map this single domain's 'one' onto
# the whole mesh (really just the domains of the mesh it MAY
# overlap with). If we get a maximum value for any given domain
# greater than a half (0.5), then we conclude that the two domains
# do indeed overlap. Otherwise, they do not and we eliminate
# the domain from the list of candidates.
#
pickMap = {}
if not MeshIsRectilinear(meshName):
    print ""
    print ""
    print "**********************************************"
    print "**********************************************"
    print "Computing zone-by-zone overlaps using pos_cmfe"
    print "**********************************************"
    print "**********************************************"

    domRankOrderRev = list(domRankOrder)
    domRankOrderRev.reverse()
    AddPlot("Pseudocolor","overlapMap")
    for d1 in domRankOrderRev:

        # Form list of doms d1 appears as a descendant in
	d1Parents = []
	d = 0
        for descList in domDescendants:
	    if d1 in descList:
	        d1Parents.append(domRankOrder[d])
            d = d + 1
        if len(d1Parents) == 0:
	    continue

        # Export dom d1 to its own database
        SwitchPlotsTo("one")
	silr.TurnOffAll()
	silr.TurnOnSet(n2vDomId(d1))
	SetPlotSILRestriction(silr)
	ExportDatabase(exdb, exopts)
	ClearCacheForAllEngines()

	# 
        print "Checking domain %d against "%n2vDomId(d1),
        SwitchPlotsTo("overlapMap")
	silr.TurnOffAll()
	for d in d1Parents:
            print "%d"%n2vDomId(d),
	    silr.TurnOnSet(n2vDomId(d))
	    SetPlotSILRestriction(silr)
	    Query("Max","actual")
	    v = GetQueryOutputValue()
	    if v < 0.5: # remove this domain from the descendants
		didx = list(domRankOrder).index(d);
		dlist = list(domDescendants[didx])
		dlist.remove(d1)
		domDescendants[didx] = tuple(dlist)
            else: # gather info to help compute ratios
                s = GetQueryOutputString()
                vals = re.search("\(zone ([0-9]*) in domain [0-9]* at coord <([0-9eE+-\.]*), ([0-9eE+-\.]*)(, ([0-9eE+-\.]*))?>\)",s)
                key = (d, d1)
                if meshDim == 2:
                    val = (int(vals.group(1)), (float(vals.group(2)), float(vals.group(3))))
                else:
                    val = (int(vals.group(1)), (float(vals.group(2)), float(vals.group(3)), float(vals.group(5))))
                pickMap[key] = val 
	    silr.TurnOffSet(n2vDomId(d))
        print ""

    print "**********************************************************"
    print "**********************************************************"
    print "Candidate descendants as modifed by pos_cmfe overlap tests"
    print "**********************************************************"
    print "**********************************************************"
    PrintDescendantsLists(domDescendants)

def IsADeepDescendantOf(dDom, pDom, depth):
    global domDescendants
    global domRankOrder

    pDomDescListIdx = list(domRankOrder).index(pDom)
    pDomDescList = domDescendants[pDomDescListIdx]

    if len(pDomDescList) == 0:
        return False

    if depth > 0 and dDom in pDomDescList:
        return True

    for cpDom in pDomDescList:
        if IsADeepDescendantOf(dDom, cpDom, depth+1):
	    return True

    return False

#
# At this point, we know which domains are DESCENDANTS of
# which domains. But, we really only want the IMMEDIATE children 
# of each. So, we need to eliminate from each domain's list of
# descendants, those that appear in other domain's list of 
# descendants.
#
print "****************************"
print "****************************"
print "Eliminating deep descendants"
print "****************************"
print "****************************"
domChildren = []
for d in domRankOrder:
    didx = list(domRankOrder).index(d)
    children = list(domDescendants[didx])
    children_copy = list(domDescendants[didx])
    for c in children:
	if IsADeepDescendantOf(c, d, 0):
	    children_copy.remove(c)
    domChildren.append(tuple(children_copy))

print "**************************"
print "**************************"
print "Final descendants lists..."
print "**************************"
print "**************************"
PrintDescendantsLists(domChildren)

def MaxHeight(dDom):
    global domDescendants
    global domRankOrder

    dDomDescListIdx = list(domRankOrder).index(dDom)
    dDomDescList = domDescendants[dDomDescListIdx]

    if len(dDomDescList) == 0:
        return 0 

    hMax = 0
    for ddDom in dDomDescList:
        hMax = max(hMax, MaxHeight(ddDom))
    return hMax+1


#
# Now, determine the height of each domain above the 'bottom' of
# the hierarchy. This will translate to 'levels' grouping in VisIt. 
#
domHeights = []
for d in domRankOrder:
    domHeights.append(MaxHeight(d))
print domHeights


def ComputeRectilinearRatios():
    global domRankOrder
    global domDescendants
    global meshDim

    domRatios = []

    for d in domRankOrder:
        didx = list(domRankOrder).index(d)
        dChildList = domChildren[didx]
        ratios = []
        for dc in dChildList:
            dSizeX = (spatExtents[d][1] - spatExtents[d][0]) / logSizes[d][0]
            dSizeY = (spatExtents[d][3] - spatExtents[d][2]) / logSizes[d][1]
            dcSizeX = (spatExtents[dc][1] - spatExtents[dc][0]) / logSizes[dc][0]
            dcSizeY = (spatExtents[dc][3] - spatExtents[dc][2]) / logSizes[dc][1]
            ratioX = int(round(dSizeX / dcSizeX))
            ratioY = int(round(dSizeY / dcSizeY))
            if meshDim == 3:
                dSizeZ = (spatExtents[d][5] - spatExtents[d][4]) / logSizes[d][2]
                dcSizeZ = (spatExtents[dc][5] - spatExtents[dc][4]) / logSizes[dc][2]
                ratioZ = int(round(dSizeZ / dcSizeZ))
                ratios.append((ratioX, ratioY, ratioZ))
            else:
                ratios.append((ratioX, ratioY))
        domRatios.append(tuple(ratios))

    return domRatios

def ComputeCurvilinearRatios():
    global domRankOrder
    global domDescendants
    global meshDim

    domRatios = []
    SwitchPlotsTo(meshName)
    silr.TurnOffAll()
    SetPlotSILRestriction(silr)
    for d in domRankOrder:

        print "Working on domain", n2vDomId(d)
        didx = list(domRankOrder).index(d)
        dChildList = domChildren[didx]
        ratios = []
        silr.TurnOnSet(n2vDomId(d))
        SetPlotSILRestriction(silr)

        for dc in dChildList:

            key = (d, dc)
            val = pickMap[key]
            pickPoint = val[1]

            ZonePick(pickPoint)
            s = GetPickOutput()
            if re.search("Chosen pick did not intersect surface.",s) != None:
                print "Missed ZonePick at", pickPoint
                continue
            vals = re.search("\nZone: *([0-9]*)\n",s)
            zoneId = int(vals.group(1))
            print "At pick point", pickPoint, "got zone id", zoneId
            dCornerPoints = GetCornerPointsForZone(d,n2vDomId(d),zoneId)

            silr.TurnOffSet(n2vDomId(d))
            silr.TurnOnSet(n2vDomId(dc))
            SetPlotSILRestriction(silr)

            ZonePick(pickPoint)
            s = GetPickOutput()
            if re.search("Chosen pick did not intersect surface.",s) != None:
                print "Missed ZonePick at", pickPoint
                continue
            vals = re.search("\nZone: *([0-9]*)\n",s)
            zoneId = int(vals.group(1))
            print "At pick point", pickPoint, "got zone id", zoneId
            dcCornerPoints = GetCornerPointsForZone(dc,n2vDomId(dc),zoneId)

            dSizeX  =  dCornerPoints[1][0] -  dCornerPoints[0][0]
            dcSizeX = dcCornerPoints[1][0] - dcCornerPoints[0][0]
            dSizeY =   dCornerPoints[2][1] -  dCornerPoints[0][1]
            dcSizeY = dcCornerPoints[2][1] - dcCornerPoints[0][1]
            ratioX = int(round(dSizeX / dcSizeX))
            ratioY = int(round(dSizeY / dcSizeY))
            if meshDim == 3:
                dSizeZ  =  dCornerPoints[3][2] -  dCornerPoints[0][2]
                dcSizeZ = dcCornerPoints[3][2] - dcCornerPoints[0][2]
                ratioZ = int(round(dSizeZ / dcSizeZ))
                ratios.append((ratioX, ratioY, ratioZ))
            else:
                ratios.append((ratioX, ratioY))

            silr.TurnOffSet(n2vDomId(dc))
            silr.TurnOnSet(n2vDomId(d))
            SetPlotSILRestriction(silr)

        silr.TurnOffSet(n2vDomId(d))
        domRatios.append(tuple(ratios))

    return domRatios

def ComputeRatios():
    if MeshIsRectilinear(meshName):
        return ComputeRectilinearRatios()
    else:
        return ComputeCurvilinearRatios()

print "*********************************************"
print "*********************************************"
print "Determining parent/child refinement ratios..."
print "*********************************************"
print "*********************************************"
domRatios = ComputeRatios()
print domRatios

#
# See if the ratios are same for all parent-child of the
# same height pairing.
#
levelRatios = {}
for d in domRankOrder:

    didx = list(domRankOrder).index(d)
    dChildList = domChildren[didx]
    dHeight = domHeights[didx]

    for dc in dChildList:

        dcidx = list(domRankOrder).index(dc)
        dccidx = list(dChildList).index(dc)
        dcHeight = domHeights[dcidx]
	ratios = domRatios[didx][dccidx]
	hKey = (dHeight, dcHeight)
	if levelRatios.has_key(hKey):
	    if levelRatios[hKey] != ratios:
                levelRatios[hKey] = 0
        else:
            levelRatios[hKey] = ratios

levelRatiosAreGood = 1
for lr in levelRatios:
    if levelRatios[lr] == 0:
        levelRatiosAreGood = 0
	break
if levelRatiosAreGood:
    print "We have constant ratios for each level"
    for lr in levelRatios:
        print "for height pairing", lr, "ratios =", levelRatios[lr]

def Qf(f):
    g = f * 10**6
    g = round(g)
    g = g / 10**6
    return g
    
def QfStr(fstr):
    return Qf(float(fstr))
    
#
# Returns sets of points for N,S,E,W,F,B domain boundaries
#
def ComputeDomainBoundaryPointSets(nDomId):
    global meshDim
    global logSizes

    cPoints = []
    nodeIds = ()
    if meshDim == 2:
        NumZonesX = logSizes[nDomId][0]
        NumZonesY = logSizes[nDomId][1]
        NumNodesX = NumZonesX+1
	NumNodesY = NumZonesY+1
	Xoff = NumNodesX-1
	Yoff = (NumNodesY-1)*NumNodesX
	nodeIds = (       0,        Xoff,
	           Yoff + 0, Yoff + Xoff)
    else:
        NumZonesX = logSizes[nDomId][0]
        NumZonesY = logSizes[nDomId][1]
	NumZonesZ = logSizes[nDomId][2]
        NumNodesX = NumZonesX+1
        NumNodesY = NumZonesY+1 
	NumNodesZ = NumZonesZ+1
	Xoff = NumNodesX-1
	Yoff = (NumNodesY-1)*NumNodesX
	Zoff = (NumNodesZ-1)*NumNodesX*NumNodesY
	nodeIds = (              0,               Xoff,
	                  Yoff + 0,        Yoff + Xoff,
	           Zoff        + 0, Zoff        + Xoff,
	           Zoff + Yoff + 0, Zoff + Yoff + Xoff)
    for nodeId in nodeIds:
        PickByNode(nodeId, n2vDomId(nDomId))
        s = GetPickOutput()
        vals = re.search("Point: <([0-9eE+-\.]*), ([0-9eE+-\.]*)(, ([0-9eE+-\.]*))?>",s)
        if meshDim == 2:
            cPoints.append((QfStr(vals.group(1)), QfStr(vals.group(2))))
        else:
            cPoints.append((QfStr(vals.group(1)), QfStr(vals.group(2)), QfStr(vals.group(4))))

    #
    # Ok, now form the point sets for each of the N,S,E,W,F,B boundaries
    #
    pSets = []
    if meshDim == 2:
        pSets.append(set([cPoints[2],cPoints[3]])) # N
        pSets.append(set([cPoints[0],cPoints[1]])) # S
        pSets.append(set([cPoints[1],cPoints[3]])) # E
        pSets.append(set([cPoints[0],cPoints[2]])) # W
    else:
        pSets.append(set([cPoints[2],cPoints[3],cPoints[6],cPoints[7]])) # N
        pSets.append(set([cPoints[0],cPoints[1],cPoints[4],cPoints[5]])) # S
        pSets.append(set([cPoints[1],cPoints[3],cPoints[5],cPoints[7]])) # E
        pSets.append(set([cPoints[0],cPoints[2],cPoints[4],cPoints[6]])) # W
        pSets.append(set([cPoints[4],cPoints[5],cPoints[6],cPoints[7]])) # F
        pSets.append(set([cPoints[0],cPoints[1],cPoints[2],cPoints[3]])) # B

    return pSets

#
# If there is more than one patch at the max height, we need to
# assemble them into a coherent, logical indexing space. Hopefully,
# if there are multiple top-level patches, their number is still
# small relative to the total. That is because we wind up doing
# an O(n^2) search over top-level doms here.
#
logExtents = []
for i in range(len(domRankOrder)):
    logExtents.append(0)
maxHeight = 0
for h in domHeights:
    maxHeight = max(h,maxHeight)
highestDoms = []
for i in range(len(domHeights)):
    if domHeights[i] == maxHeight:
	highestDoms.append(domRankOrder[i])
if len(highestDoms) > 1:
    print "***************************************************************"
    print "***************************************************************"
    print "Assembling top-level patches into coherent, logical index space"
    print "***************************************************************"
    print "***************************************************************"

    SwitchPlotsTo(meshName)
    silr.TurnOffAll()
    SetPlotSILRestriction(silr)
    for i in range(len(highestDoms)):

        silr.TurnOnSet(n2vDomId(highestDoms[i]))
	SetPlotSILRestriction(silr)

        # compute corner points of dom and replace entry
	# in highestDoms with full dom info
	domInfo = (highestDoms[i], ComputeDomainBoundaryPointSets(highestDoms[i]), [-1,-1,-1,-1,-1,-1])
        silr.TurnOffSet(n2vDomId(highestDoms[i]))
	highestDoms[i] = domInfo

    for i in range(len(highestDoms)):

	infoI = highestDoms[i]
	domI = infoI[0]

        for j in range(len(highestDoms)):
	    if j == i:
	        continue
        
	    infoJ = highestDoms[j]
	    domJ = infoJ[0]

	    NorthI = infoI[1][0]
	    SouthI = infoI[1][1]
	    EastI  = infoI[1][2]
	    WestI  = infoI[1][3]
	    NorthJ = infoJ[1][0]
	    SouthJ = infoJ[1][1]
	    EastJ  = infoJ[1][2]
	    WestJ  = infoJ[1][3]

	    if NorthI == SouthJ:
	        infoI[2][0] = domJ
		infoJ[2][1] = domI
            if SouthI == NorthJ:
		infoI[2][1] = domJ
		infoJ[2][0] = domI
            if EastI == WestJ:
		infoI[2][2] = domJ
		infoJ[2][3] = domI
            if WestI == EastJ:
		infoI[2][3] = domJ
		infoJ[2][2] = domI

            if meshDim == 3:
	        FrontI = infoI[1][4]
		BackI  = infoI[1][5]
		FrontJ = infoJ[1][4]
		BackJ  = infoJ[1][5]

		if FrontI == BackJ:
		    infoI[2][4] = domJ
		    infoJ[2][5] = domI
                if BackI == FrontJ:
		    infoI[2][5] = domJ
		    infoJ[2][4] = domI

    print "Top-level domain neighbor relationships..."
    for i in range(len(highestDoms)):
        print highestDoms[i][0], highestDoms[i][2]

    #
    # Find the south-most, west-most, back-most
    # by simply walking that way from an arbitrary
    # domain.
    #
    hDom = highestDoms[0][0]
    hDomList = []
    for i in range(len(highestDoms)):
        hDomList.append(highestDoms[i][0])
    done = False
    while not done:
        hDom = hDomList.index(hDom)
        if highestDoms[hDom][2][1] != -1:
	    hDom = highestDoms[hDom][2][1]
        elif highestDoms[hDom][2][3] != -1:
	    hDom = highestDoms[hDom][2][3]
        elif meshDim == 3 and highestDoms[hDom][2][5] != -1:
	    hDom = highestDoms[hDom][2][5]
        else:
	    done = True

    swbMostDom = highestDoms[hDom][0]
    print "The top-level, south-most, west-most, back-most domain is", n2vDomId(swbMostDom)
    if meshDim == 2:
        logExtents[swbMostDom] = [0, logSizes[swbMostDom][0]-1, 0, logSizes[swbMostDom][1]-1, 0, 0]
    else:
        logExtents[swbMostDom] = [0, logSizes[swbMostDom][0]-1, 0, logSizes[swbMostDom][1]-1,
	                          0, logSizes[swbMostDom][2]-1]

    extents = [0,0,0,0,0,0]
    extents[0] = 0
    extents[1] = logSizes[swbMostDom][0]-1
    extents[2] = 0
    extents[3] = logSizes[swbMostDom][1]-1
    extents[4] = 0
    if meshDim == 3:
        extents[5] = logSizes[swbMostDom][2]-1
    fbDone = False
    fDom = swbMostDom
    while not fbDone:
        fDomi = hDomList.index(fDom)
	nxtfDom = -1
	if meshDim == 3:
	    nxtfDom = highestDoms[fDomi][2][4]

	nsDone = False
	nDom = fDom
	while not nsDone:
            nDomi = hDomList.index(nDom)
	    nxtnDom = highestDoms[nDomi][2][0]

	    ewDone = False
	    eDom = nDom
	    while not ewDone:
	        eDomi = hDomList.index(eDom)
		nxteDom = highestDoms[eDomi][2][2]

                eDomIdx = list(domRankOrder).index(eDom)
		logExtents[eDomIdx] = copy.copy(extents)
		if nxteDom != -1:
		    eDom = nxteDom
		    extents[0] = extents[0] + logSizes[nxteDom][0]
		    extents[1] = extents[1] + logSizes[nxteDom][0]
		else:
		    ewDone = True

	    extents[0] = 0
	    extents[1] = logSizes[swbMostDom][0]-1
	    if nxtnDom != -1:
	        nDom = nxtnDom
	        extents[2] = extents[2] + logSizes[nxtnDom][1]
	        extents[3] = extents[3] + logSizes[nxtnDom][1]
	    else:
	        nsDone = True

	extents[2] = 0
	extents[3] = logSizes[swbMostDom][1]-1
        if nxtfDom != -1:
	    fDom = nxtfDom
	    extents[4] = extents[4] + logSizes[nxtfDom][2]
	    extents[5] = extents[5] + logSizes[nxtfDom][2]
	else:
	    fbDone = True

    print "Logical extents are..."
    print logExtents

    # Walk east as far as possible, summing logical extents
    hDom = swbMostDom
    lxSize = logSizes[hDom][0]
    done = False
    while not done:
        hDom = hDomList.index(hDom)
	if highestDoms[hDom][2][2] != -1:
	    hDom = highestDoms[hDom][2][2]
	    lxSize = lxSize + logSizes[hDom][0]
        else:
	    done = True

    # Walk as far north as possible
    hDom = swbMostDom
    lySize = logSizes[hDom][1]
    done = False
    while not done:
        hDom = hDomList.index(hDom)
	if highestDoms[hDom][2][0] != -1:
	    hDom = highestDoms[hDom][2][0]
	    lySize = lySize + logSizes[hDom][1]
        else:
	    done = True

    # Walk as far front as possible
    lzSize = 0
    if meshDim == 3:
        hDom = swbMostDom
        lzSize = logSizes[hDom][2]
        done = False
        while not done:
            hDom = hDomList.index(hDom)
	    if highestDoms[hDom][2][4] != -1:
	        hDom = highestDoms[hDom][2][4]
	        lzSize = lzSize + logSizes[hDom][2]
            else:
	        done = True

    print "Top-level logical extents are", lxSize, lySize, lzSize

else: # if len(highestDoms) > 1:
    hDom = highestDoms[0]
    if meshDim == 2:
        logExtents[hDom] = [0, logSizes[hDom][0]-1, 0, logSizes[hDom][1]-1, 0, 0]
    else:
        logExtents[hDom] = [0, logSizes[hDom][0]-1, 0, logSizes[hDom][1]-1,
	                          0, logSizes[hDom][2]-1]

def ClampPointToDomainExtents(p, d):
    global spatExetnts
    global meshDim

    ptmp = list(p)
    xEpsilon = (spatExtents[d][1] - spatExtents[d][0]) / logSizes[d][0] / 10**4
    if ptmp[0] <= spatExtents[d][0]:
        ptmp[0] = spatExtents[d][0] + xEpsilon
    if ptmp[0] >= spatExtents[d][1]:
        ptmp[0] = spatExtents[d][1] - xEpsilon
    yEpsilon = (spatExtents[d][3] - spatExtents[d][2]) / logSizes[d][1] / 10**4
    if ptmp[1] <= spatExtents[d][2]:
        ptmp[1] = spatExtents[d][2] + yEpsilon
    if ptmp[1] >= spatExtents[d][3]:
        ptmp[1] = spatExtents[d][3] - yEpsilon
    if meshDim == 3:
        zEpsilon = (spatExtents[d][5] - spatExtents[d][4]) / logSizes[d][2] / 10**4
        if ptmp[2] <= spatExtents[d][4]:
            ptmp[2] = spatExtents[d][4] + zEpsilon
        if ptmp[2] >= spatExtents[d][5]:
            ptmp[2] = spatExtents[d][5] - zEpsilon

    return tuple(ptmp)


#
# Determine logical extents of patches.
# Above, we computed logical extents of top-level patches. Now,
# we iterate layer by layer downwards, computing logical extents
# of patches in terms of their parents.
#
SwitchPlotsTo(meshName)
currentHeight = maxHeight
while currentHeight >= 0:
    print "Doms at height", currentHeight, "...",
    for i in range(len(domHeights)):
        if domHeights[i] != currentHeight:
            continue
        print n2vDomId(domRankOrder[i]),
    print ""

    for i in range(len(domHeights)):
        if domHeights[i] != currentHeight:
            continue

        #
        # We continue only if we haven't already evaluated the
        # logical extents for this domain
        #
        if logExtents[i] != 0:
            continue

        #
        # Form list of doms d1 appears as a descendant in
        #
        d1 = domRankOrder[i]
        d1Parents = []
        d = 0
        for childList in domChildren:
            if d1 in childList:
                d1Parents.append(domRankOrder[d])
            d = d + 1
        if len(d1Parents) == 0:
            continue
        print "Parents of dom", n2vDomId(d1), "are",
        for d in d1Parents:
            print n2vDomId(d),
        print ""

        #
        # Get coords of first and last node of this domain
        #        
        node0Coords = ()
        nodeNCoords = ()
        silr.TurnOffAll()
        silr.TurnOnSet(n2vDomId(d1))
        SetPlotSILRestriction(silr)
        PickByNode(0, n2vDomId(d1))
        s = GetPickOutput()
        vals = re.search("Point: <([0-9eE+-\.]*), ([0-9eE+-\.]*)(, ([0-9eE+-\.]*))?>",s)
        if meshDim == 2:
            node0Coords = (QfStr(vals.group(1)), QfStr(vals.group(2)))
        else:
            node0Coords = (QfStr(vals.group(1)), QfStr(vals.group(2)), QfStr(vals.group(4)))

        nodeN = (logSizes[d1][0]+1) * (logSizes[d1][1]+1)
        if meshDim == 3:
            nodeN = nodeN * (logSizes[d1][2]+1)
        nodeN = nodeN - 1
        PickByNode(nodeN, n2vDomId(d1))
        s = GetPickOutput()
        vals = re.search("Point: <([0-9eE+-\.]*), ([0-9eE+-\.]*)(, ([0-9eE+-\.]*))?>",s)
        if meshDim == 2:
            nodeNCoords = (QfStr(vals.group(1)), QfStr(vals.group(2)))
        else:
            nodeNCoords = (QfStr(vals.group(1)), QfStr(vals.group(2)), QfStr(vals.group(4)))
        silr.TurnOffSet(n2vDomId(d1))
        print "coords to pick at are..."
        print "    ", node0Coords
        print "    ", nodeNCoords

        #
        # Get logical coords of node in parent nearest the points found above.
        # Typically, these occure in the SAME parent but can be in different
        # parents.
        #
        havePickedNode0 = False
        havePickedNodeN = False
        ratio = levelRatios[(currentHeight+1,currentHeight)]
        iextents = [0,0,0,0,0,0]
        for d in d1Parents:
            silr.TurnOnSet(n2vDomId(d))
            SetPlotSILRestriction(silr)
            didx = list(domRankOrder).index(d)
            dextents = logExtents[didx]
            if not havePickedNode0:
                p0 = copy.copy(node0Coords)
                p0 = ClampPointToDomainExtents(p0, d)
                NodePick(p0)
                s = GetPickOutput()
                vals = re.search("Node: *([0-9]*) domain <([0-9]*), ([0-9]*)(, ([0-9]*))?>", s)
                if vals != None:
                    nodeId = int(vals.group(1))
                    havePickedNode0 = True
                    logId = LinToLogId(d, nodeId, True)
                    if meshDim == 2:
                        iextents[0] = (dextents[0] + logId[0]) * ratio[0]
                        iextents[2] = (dextents[2] + logId[1]) * ratio[1]
                    else:
                        iextents[0] = (dextents[0] + logId[0]) * ratio[0]
                        iextents[2] = (dextents[2] + logId[1]) * ratio[1]
                        iextents[4] = (dextents[4] + logId[2]) * ratio[2]
                    print "got node 0 pick", s, nodeId, logId, dextents, iextents
            if not havePickedNodeN:
                pN = copy.copy(nodeNCoords)
                print "pN=",pN
                pN = ClampPointToDomainExtents(pN, d)
                print "pN, again",pN
                NodePick(pN)
                s = GetPickOutput()
                vals = re.search("Node: *([0-9]*) domain <([0-9]*), ([0-9]*)(, ([0-9]*))?>", s)
                if vals != None:
                    nodeId = int(vals.group(1))
                    havePickedNodeN = True
                    logId = LinToLogId(d, nodeId, True)
                    if meshDim == 2:
                        iextents[1] = (dextents[0] + logId[0]) * ratio[0] - 1
                        iextents[3] = (dextents[2] + logId[1]) * ratio[1] - 1
                    else:
                        iextents[1] = (dextents[0] + logId[0]) * ratio[0] - 1
                        iextents[3] = (dextents[2] + logId[1]) * ratio[1] - 1
                        iextents[5] = (dextents[4] + logId[2]) * ratio[2] - 1
                    print "got node N pick", s, nodeId, logId, dextents, iextents
            silr.TurnOffSet(n2vDomId(d))
            if havePickedNode0 and havePickedNodeN:
                break

        logExtents[i] = copy.copy(iextents)

    currentHeight = currentHeight - 1

xmlOut = open("%s_config.xml"%meshName, "w")
xmlOut.write("<AMRDecomp ")
xmlOut.write("meshName=\"%s\" "%meshName)
xmlOut.write("numDims=\"%d\" "%meshDim)
xmlOut.write("numLevels=\"%d\" "%(maxHeight+1))
xmlOut.write("numPatches=\"%d\" "%len(domRankOrder))
xmlOut.write(">\n")

currentHeight = maxHeight
while currentHeight >= 0:
    domsAtHeight = []
    for i in range(len(domHeights)):
        if domHeights[i] != currentHeight:
            continue
        domsAtHeight.append(i)
    xmlOut.write("    <Level ")
    xmlOut.write("level=\"%d\" "%(maxHeight - currentHeight))
    xmlOut.write("numPatches=\"%d\" "%len(domsAtHeight))
    if currentHeight == maxHeight:
        ratios = (1, 1, 1)
    else:
        ratios = levelRatios[(currentHeight+1,currentHeight)]
    if meshDim == 2:
        xmlOut.write("ratios=\"%d %d\" "%(ratios[0],ratios[1]))
    else:
        xmlOut.write("ratios=\"%d %d %d\" "%(ratios[0],ratios[1], ratios[2]))
    xmlOut.write(">\n")
    xmlOut.write("        ")
    for i in range(len(domsAtHeight)): 
        xmlOut.write("%d "%domsAtHeight[i])
        if (i+1) % 20 == 0:
            xmlOut.write("\n        ")
    xmlOut.write("\n")
    xmlOut.write("    </Level>\n")
    currentHeight = currentHeight - 1

for d in range(len(domRankOrder)):
    didx = list(domRankOrder).index(d)
    xmlOut.write("    <Patch ")
    xmlOut.write("iDx=\"%d\" "%d)
    xmlOut.write("vId=\"%d\" "%n2vDomId(d))
    xmlOut.write("level=\"%d\" "%(maxHeight - domHeights[d]))
    xmlOut.write("rank=\"%f\" "%domRankMap[didx][1])
    xmlOut.write("numChildren=\"%d\" "%len(domChildren[didx]))
    if meshDim == 2:
        xmlOut.write("logSize=\"%d %d\" "%(logSizes[d][0], logSizes[d][1]))
        xmlOut.write("logExtents=\"%d %d %d %d\" "%(logExtents[didx][0], logExtents[didx][1],
                                                    logExtents[didx][2], logExtents[didx][3]))
        xmlOut.write("spatExtents=\"%f %f %f %f\" "%(spatExtents[d][0], spatExtents[d][1],
                                               spatExtents[d][2], spatExtents[d][3]))
    else:
        xmlOut.write("logSize=\"%d %d %d\" "%(logSizes[d][0], logSizes[d][1], logSizes[d][2]))
        xmlOut.write("logExtents=\"%d %d %d %d %d %d\" "%(logExtents[didx][0], logExtents[didx][1],
                                                          logExtents[didx][2], logExtents[didx][3],
                                                          logExtents[didx][4], logExtents[didx][5]))
        xmlOut.write("spatExtents=\"%f %f %f %f %f %f\" "%(spatExtents[d][0], spatExtents[d][1],
                                                           spatExtents[d][2], spatExtents[d][3],
                                                           spatExtents[d][4], spatExtents[d][5]))
    xmlOut.write(">\n")
    xmlOut.write("        ")
    for i in range(len(domChildren[didx])):
        xmlOut.write("%d "%domChildren[didx][i])
        if (i+1) % 20 == 0:
            xmlOut.write("\n        ")
    xmlOut.write("\n")
    xmlOut.write("    </Patch>\n")

xmlOut.write("</AMRDecomp>\n")
xmlOut.close()
sys.exit(0)
