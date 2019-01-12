import re

def SaveActivePlots(activePlots):
    thePlots = GetPlotList()
    for p in range(thePlots.GetNumPlots()):
        if thePlots.GetPlots(p).activeFlag == 1:
            activePlots.append(p)

def RestoreActivePlots(activePlots):
    SetActivePlots(tuple(activePlots))

###############################################################################
# Function: UnsynchMinMax 
#
# Purpose:  As best as possible, undo whatever min/max changes SyncMinMax has
#           done. 
#
# Programmer: Mark C. Miller, Tue Aug 12 11:04:58 PDT 2008
#
###############################################################################
def UnsyncMinMax():
    """Undo whatever min/max settings changes SyncMinMax did.

    UnsyncMinMax does its best to undo whatever changes to plots SyncMinMax
    does. However, if the user changes things between invokations of
    SyncMinMax and UnsyncMinMax, it can be possible to confuse this operation
    and prevent it from correctly undoing everything.
    """

    global plotsChangedSinceLastUnsync

    globAtts = GetGlobalAttributes()
    origActiveWindowID = globAtts.windows[globAtts.activeWindow]

    #
    # Iterate over all windows represented in plotsChangedSinceLastUnsync
    #
    for winID in plotsChangedSinceLastUnsync.keys():
        try:
            plotInfos = plotsChangedSinceLastUnsync[winID]
            SetActiveWindow(winID)
            origActivePlotList = []
            SaveActivePlots(origActivePlotList)
            for pi in range(len(plotInfos)):
                plotIndex = plotInfos[pi][0]
                plotOrigAttrs = plotInfos[pi][1]
                plotSetAttrs = plotInfos[pi][2]

                #
                # Get the current attributes of the plot. If the only
                # difference between the plots current attributes and
                # the original attributes before SyncMinMax changed
                # them is the min/max settings, then we'll set back to
                # the original
                #
                SetActivePlots((plotIndex,))
                attrs = GetPlotOptions()
                if attrs != plotOrigAttrs:
                    savedMinFlag = plotOrigAttrs.minFlag
                    savedMaxFlag = plotOrigAttrs.maxFlag
                    savedMin     = plotOrigAttrs.min
                    savedMax     = plotOrigAttrs.max
                    plotOrigAttrs.minFlag = 1
                    plotOrigAttrs.maxFlag = 1
                    plotOrigAttrs.min = attrs.min
                    plotOrigAttrs.max = attrs.max
                    if attrs == plotOrigAttrs:
                        plotOrigAttrs.minFlag = savedMinFlag
                        plotOrigAttrs.maxFlag = savedMaxFlag
                        plotOrigAttrs.min = savedMin
                        plotOrigAttrs.max = savedMax
                        SetPlotOptions(plotOrigAttrs)
                
            #
            # Restore the active plot list
            #
            RestoreActivePlots(origActivePlotsList)

        except:
            continue

    #
    # Restore the active window
    #
    SetActiveWindow(origActiveWindowID)

    #
    # Clear the list of plots we've changed
    #
    plotsChangedSinceLastUnsync = {}
    

###############################################################################
# Function: SyncMinMax 
#
# Purpose:  Set Min/Max values on all plots in all windows with those obtained
#           form equivalent plots in the source window.
#
# Programmer: Mark C. Miller, Tue Aug 12 11:04:58 PDT 2008
#
###############################################################################
def SyncMinMax(srcWin):
    """Synchronize all plot's min/max settings to those of specified window ID.

    SyncMinMax will find all non-hidden plots in the specified window and for
    each plot that supports the explicit setting of its min/max values (the
    Pseudocolor plot does this, for example), find equivalent plots in all
    other windows and force those plots to use the same min/max. This makes
    it easier to compare subtlely different results in different windows.

    SyncMinMax is designed to work when there are multiple plots in each 
    window. However, if there are multiple plots of the same type (e.g.
    two Pseudocolor plots) in the source window, it will use only the
    FIRST it finds there to set all equivalent plots min/max values.
    """

    #
    # Iterate over all plots in srcWin finding the first of each 'type'
    # that has min/max settings and record those settings,
    #
    globAtts = GetGlobalAttributes()
    origActiveWindowID = globAtts.windows[globAtts.activeWindow]
    plotAttrsByKey = {}
    SetActiveWindow(srcWin)
    origActivePlotsList = []
    SaveActivePlots(origActivePlotsList)
    srcPlots = GetPlotList()
    for p in range(srcPlots.GetNumPlots()):
        #
        # Ignore plots that are hidden.
        #
        if srcPlots.GetPlots(p).hiddenFlag == 1:
           continue

        #
        # Form a 'key' for this plot. If its the only one of its
        # type, just use the type. Otherwise combine type with
        # variable name.
        #
        plotKey = PlotPlugins()[srcPlots.GetPlots(p).plotType]
        for q in range(srcPlots.GetNumPlots()):
            if q == p: continue
            if PlotPlugins()[srcPlots.GetPlots(q).plotType] == plotKey:
                plotKey = plotKey + ":" + srcPlots.GetPlots(p).plotVar
                break
         
        #
        # If we've already seen a plot with this key, skip it. 
        # We use only the FIRST such plot as the 'master' for
        # min/max values for other plots of that type.
        #
        if plotKey in plotAttrsByKey:
            continue

        #
        # Get the attributes for this plot type.
        #
        SetActivePlots((p,))
        attrs = GetPlotOptions()

        #
        # See if the attributes for this plot type has controls
        # for setting min/max manually. If it doesn't, skip it.
        # We won't be able to adjust its equivalent plots in other
        # windows without this feature.
        #
        if not (hasattr(attrs, "min") and hasattr(attrs, "max") and \
                hasattr(attrs, "minFlag") and hasattr(attrs, "maxFlag")):
            continue

        #
        # If the min/max controls are manually set explicitly, use
        # those. Note that they will already exist in attrs 'min'
        # and 'max' members. Otherwise, query the plot for its
        # min/max values. Note that GetQueryOutputValue doesn't give
        # numbers that are same as PC plot's legend. So, we parse
        # values from the query output string instead.
        #
        if attrs.minFlag == 0 or attrs.maxFlag == 0:
	    SuppressQueryOutputOn()
            Query("MinMax")
            qo = GetQueryOutputString()
	    SuppressQueryOutputOff()
            if attrs.minFlag == 0:
                # stuff min results into our attrs object
	        mininfo = re.search("Min = ([0-9e.+-]* )",qo)
		if mininfo == None:
	            mininfo = re.search("Min .*\n.*= ([0-9e.+-]* )",qo)
		if mininfo != None:
                    attrs.min = float(mininfo.group(1)) 
            if attrs.maxFlag == 0:
                # stuff max results into our attrs object
	        maxinfo = re.search("Max = ([0-9e.+-]* )",qo)
		if maxinfo == None:
	            maxinfo = re.search("Max .*\n.*= ([0-9e.+-]* )",qo)
		if maxinfo != None:
                    attrs.max = float(maxinfo.group(1))

        #
        # If we get here, we've got a non-hidden plot that supports
        # manual setting of min/max values and this is the FIRST such
        # plot of this type we've seen. So, save its attrs. We really
        # only need the min/max values so saving all the attrs is
        # way overkill. But, its convenient.
        #
        plotAttrsByKey[plotKey] = attrs

    RestoreActivePlots(origActivePlotsList)

    #
    # Ok, now iterate over plots in all windows other than the
    # source window and set their min/max values to those of
    # the 'master(s)' identified in the loop above.
    #
    global plotsChangedSinceLastUnsync
    for win in range(len(globAtts.windows)):

        #
        # Get window ID for call to SetActiveWindow
        #
        winID = globAtts.windows[win]

        #
        # Skip the 'master' window
        #
        if winID == srcWin:
            continue

        SetActiveWindow(winID)
        origActivePlotsList = []
        SaveActivePlots(origActivePlotsList)
        thePlots = GetPlotList()
        for p in range(thePlots.GetNumPlots()):
            #
            # Skip hidden plots
            #
            if thePlots.GetPlots(p).hiddenFlag == 1:
                continue;

            #
            # Form a 'key' for this plot. If its the only one of its
            # type, just use the type. Otherwise combine type with
            # variable name.
            #
            plotKey = PlotPlugins()[thePlots.GetPlots(p).plotType]
            for q in range(thePlots.GetNumPlots()):
                if q == p: continue
                if PlotPlugins()[thePlots.GetPlots(q).plotType] == plotKey:
                    plotKey = plotKey + ":" + thePlots.GetPlots(p).plotVar
                    break

            #
            # If we don't have a 'master' for it, skip it.
            #
            if plotKey not in plotAttrsByKey:
                continue

            #
            # Update the min/max settings for this plot.
            #
            newAttrs = plotAttrsByKey[plotKey]
            SetActivePlots((p,))
            currentAttrs = GetPlotOptions()
            if currentAttrs.minFlag != 1 or currentAttrs.min != newAttrs.min or \
               currentAttrs.maxFlag != 1 or currentAttrs.max != newAttrs.max:
                origAttrs = GetPlotOptions()
                currentAttrs.minFlag = 1 
                currentAttrs.min     = newAttrs.min
                currentAttrs.maxFlag = 1
                currentAttrs.max     = newAttrs.max
                SetPlotOptions(currentAttrs)

                #
                # Record the fact that we fiddled with this
                # plots extents so that we can later unsync them
                # if we need to.
                #
                if winID not in plotsChangedSinceLastUnsync:
                    plotsChangedSinceLastUnsync[winID] = []
                plotsChangedSinceLastUnsync[winID].append((p,origAttrs,currentAttrs))

        RestoreActivePlots(origActivePlotsList)

    SetActiveWindow(origActiveWindowID)

###############################################################################
#
# Purpose: Provide alternative VCR step controls that ensure all plots
#          min/max settings in all windows are sync'd with the plot(s) in a
#          given window (1). Several types of plots support setting of min/max
#          values. This code is designed to find any that do in the 'master'
#          window and then push those values out to the equivalent plots in
#          other windows.
#
# Programmer: Mark C. Miller, Tue Aug 12 11:03:00 PDT 2008
#
##############################################################################

#
# Define the python functions we'll bind to macro buttons
#
def SyncMinMax1():
    SyncMinMax(1)

def SyncMinMax2():
    SyncMinMax(2)

def SyncMinMax3():
    SyncMinMax(3)

def SyncMinMax4():
    SyncMinMax(4)

def NextAndSync1():
    TimeSliderNextState()
    SyncMinMax(1)

def PrevAndSync1():
    TimeSliderPreviousState()
    SyncMinMax(1)

#
# Delay to avoid race condition between GUI/Viewer. Without
# this, often the macro buttons don't appear in Controls->Macros
# without the user having to hit the 'Update Macros' button.
#
i = 0
for f in range(1000000):
    i = i + 1

#
# Create a set of useful buttons in Controls->Macros window.
# We assume no more than 4 windows and, furthermore, all
# have ids 1...4. We can't do much better than this.
#
RegisterMacro("SyncMinMaxToWin1", SyncMinMax1)
RegisterMacro("SyncMinMaxToWin2", SyncMinMax2)
RegisterMacro("SyncMinMaxToWin3", SyncMinMax3)
RegisterMacro("SyncMinMaxToWin4", SyncMinMax4)
RegisterMacro("UndoSyncMinMax", UnsyncMinMax)

#
# Provide the equivalent of VCR step forward/backward
# buttons that also do the work to sync min/max. But,
# if we define these for all 4 windows, we'd define
# another 8 buttons. So, we do it just for window 1.
# The user will have to accept the constraint that
# window 1 will have to act as the master for these
# to be useful.
#
RegisterMacro("NextAndSyncToWin1", NextAndSync1)
RegisterMacro("PrevAndSyncToWin1", PrevAndSync1)

plotsChangedSinceLastUnsync = {}
