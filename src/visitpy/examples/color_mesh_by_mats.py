###############################################################################
# Function: MatForMesh
#
# Purpose:  Return a material object for a given mesh
#
# Programmer: Mark C. Miller
# Date:       Wed Jul 18 18:12:28 PDT 2007
#
###############################################################################
def MatForMesh(metadata, meshname):
    for i in range(metadata.GetNumMaterials()):
        if metadata.GetMaterials(i).meshName == meshname:
            return metadata.GetMaterials(i)

###############################################################################
# Function: ColorMeshByMats 
#
# Purpose:  Example macro to make a 'mesh' plot that colors the mesh by
#           material. Since the mesh plot does not permit us to color each 
#           edge that is drawn using different colors, we wind up having to
#           instantiate several mesh plots, one for each material, and setting
#           the SIL for each plot to a given material and then setting the
#           color for the mesh plot to match what the FilledBoundary plot
#           would have used for color for each material. 
#
# Programmer: Mark C. Miller, Wed Jan 21 21:47:33 PST 2009
#
###############################################################################
def ColorMeshByMats():
    global meshPlotIds
    global matVarName
    global fbLegendPlotId 

    #
    # Things go much faster if you first disable re-draws and then re-enable
    # at the end.
    #
    DisableRedraw()

    #
    # Find an active, non-hidden mesh plot
    #
    pl = GetPlotList()
    plot = 0
    for p in range(pl.GetNumPlots()):
        plot = pl.GetPlots(p)
        if plot.activeFlag == 1 and plot.hiddenFlag == 0 and \
           PlotPlugins()[plot.plotType] == "Mesh":
               break

    #
    # If we can't find an active, non-hidden mesh plot, return early
    #
    if plot == 0:
        print "ColorMeshByMats requires an active, non-hidden mesh plot to work from"
        return

    #
    # Ok, get the material for this mesh
    #
    md = GetMetaData(plot.databaseName)
    matmd = MatForMesh(md, plot.plotVar)

    #
    # Clear global information we're managing
    #
    matVarName = matmd.name
    if fbLegendPlotId >= 0:
        SetActivePlots((fbLegendPlotId,))
        DeleteActivePlots()
    fbLegendPlotId = -1
    if len(meshPlotIds) > 0:
        SetActivePlots(tuple(meshPlotIds))
        DeleteActivePlots()
    meshPlotIds = []

    #
    # Add a FilledBoundary plot for this material. We're going to remove
    # remove it momentarily. We're using it to get the colors to color
    # the mesh by so that the mesh colors will match the colors that the
    # material (FilledBoundary) plot would have produced.
    #
    AddPlot("FilledBoundary", matmd.name)
    fbatts = FilledBoundaryAttributes()
    DeleteActivePlots()

    #
    # Hide the original mesh plot we started from
    #
    SetActivePlots((p,))
    HideActivePlots()

    #
    # Ok, now iterate, adding mesh plots, setting their SIL's and colors
    #
    for m in range(matmd.numMaterials):
        matName = matmd.materialNames[m]

        #
        # Find the index of this material in fbatts so we can see what
        # color a FilledBoundary plot would have used for this material.
        #
        bn = fbatts.GetBoundaryNames()
        for i in range(len(bn)):
            if bn[i] == matName:
                break
        color = fbatts.GetMultiColor()[i]

        # Add the mesh plot and set its fg color 
        AddPlot("Mesh", plot.plotVar)
        ma = MeshAttributes()
        ma.meshColor = color
        ma.foregroundFlag = 0
        ma.legendFlag = 0
        ma.opaqueMode = ma.On
        SetPlotOptions(ma)

        # Set the mesh plot's SIL restriction to just current material
        silr = SILRestriction()
        silr.TurnOffAll()
        for s in range(silr.NumSets()):
            if silr.SetName(s) == matName:
                silr.TurnOnSet(s)
                break
        SetPlotSILRestriction(silr)

        # Update our global list of plot ids
        meshPlotIds.append(GetPlotList().GetNumPlots()-1)

    DrawPlots()
    RedrawWindow()


###############################################################################
# Function: ChangeLineThickness 
#
# Purpose:  Increase or decrease the thickness of lines being used by all the
#           mesh plots. Two different macro buttons are registered for this.
#           One to call it with '1' as val and the other with '-1'.
#
# Programmer: Mark C. Miller, Wed Jan 21 21:47:33 PST 2009
#
###############################################################################
def ChangeLineThickness(val):
    global meshPlotIds

    DisableRedraw()
    for p in range(len(meshPlotIds)):
        SetActivePlots((meshPlotIds[p],))
        ma = GetPlotOptions() 
        ma.lineWidth = ma.lineWidth + val
        if ma.lineWidth < 0:
            ma.lineWidth = 0
        if ma.lineWidth > 9:
            ma.lineWidth = 9 
        SetPlotOptions(ma)
    RedrawWindow()

def IncLineThickness():
    ChangeLineThickness(1)

def DecLineThickness():
    ChangeLineThickness(-1)

###############################################################################
# Function: SetOpaque 
#
# Purpose:  Turn the opaque mode of the mesh plots either on or off. Two
#           different macro buttons are registered for this. One to call it
#           to turn opaque mode on and the other off.
#
# Programmer: Mark C. Miller, Wed Jan 21 21:47:33 PST 2009
#
###############################################################################
def SetOpaque(val):
    global meshPlotIds

    DisableRedraw()
    for p in range(len(meshPlotIds)):
        SetActivePlots((meshPlotIds[p],))
        ma = GetPlotOptions()
        ma.opaqueMode = val
        SetPlotOptions(ma)
    RedrawWindow()

def OpaqueOn():
    ma = MeshAttributes()
    SetOpaque(ma.On)

def OpaqueOff():
    ma = MeshAttributes()
    SetOpaque(ma.Off)

###############################################################################
# Function: LegendOn 
#
# Purpose:  Fudge a sort of 'legend' for these mesh plots by making a 
#           FilledBoundary plot. We don't really want to plot the
#           FilledBoundary as it could obscure what the user is trying to do.
#           But, if we hide it, it's legend will disappear to. So, we set its
#           transparency to zero. I think VisIt's internal infrastructure is
#           designed to handle this situation efficiently. In any case, we
#           also set the FilledBoundary plot to 'drawInternal' so that we
#           get an entry in the legend for all the materials not just the
#           ones that might have been visible in the FilledBoundary plot.
#           This may not be the best thing to do.
#
#           If the FilledBoundary plot we're using as a surrugate here does
#           not already exist, create it. Otherwise, just unhide it.
#           
#
# Programmer: Mark C. Miller, Wed Jan 21 21:47:33 PST 2009
#
###############################################################################
def LegendOn():
    global fbLegendPlotId
    global matVarName

    DisableRedraw()
    if fbLegendPlotId < 0:
        AddPlot("FilledBoundary", matVarName)
        fbatts = FilledBoundaryAttributes()
        fbatts.drawInternal = 1
        fbatts.opacity = 0
        SetPlotOptions(fbatts)
        DrawPlots()
        fbLegendPlotId = GetPlotList().GetNumPlots()-1
    else:
        plot = GetPlotList().GetPlots(fbLegendPlotId)
        if plot.hiddenFlag == 1:
            SetActivePlots((fbLegendPlotId,))
            HideActivePlots()
    RedrawWindow()

def LegendOff():
    global fbLegendPlotId

    DisableRedraw()
    if fbLegendPlotId >= 0:   
        plot = GetPlotList().GetPlots(fbLegendPlotId)
        if plot.hiddenFlag == 0:
            SetActivePlots((fbLegendPlotId,))
            HideActivePlots()
    RedrawWindow()

#
# Maintain some global information to support the various
# operations of this set of macros. 
#
meshPlotIds = []
fbLegendPlotId = -1
matVarName = ""

#
# Delay to avoid race condition between GUI/Viewer. Without
# this, often the macro buttons don't appear in Controls->Macros
# without the user having to hit the 'Update Macros' button.
#
i = 0
for f in range(1000000):
    i = i + 1
RegisterMacro("ColorMeshByMats", ColorMeshByMats)
RegisterMacro("ColorMeshByMats++", IncLineThickness)
RegisterMacro("ColorMeshByMats--", DecLineThickness)
RegisterMacro("ColorMeshByMatsOpqOn", OpaqueOn)
RegisterMacro("ColorMeshByMatsOpqOff", OpaqueOff)
RegisterMacro("ColorMeshByMatsLgndOn", LegendOn)
RegisterMacro("ColorMeshByMatsLgndOff", LegendOff)
