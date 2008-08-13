import re

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
    plotAttrsByType = {}
    origActivePlotsList = []
    SetActiveWindow(srcWin)
    srcPlots = GetPlotList()
    for p in range(srcPlots.GetNumPlots()):

        #
        # Since we're going to be calling SetActivePlots, keep
        # a list of the ones that were orignally active in the
        # window.
        #
        if srcPlots.GetPlots(p).activeFlag == 1:
            origActivePlotsList.append(p)

        #
        # Ignore plots that are hidden.
        #
        if srcPlots.GetPlots(p).hiddenFlag == 1:
           continue

        #
        # Get this plot's type string.
        #
        plotType = PlotPlugins()[srcPlots.GetPlots(p).plotType]

        #
        # If we've already seen a plot of this type, skip it. 
        # We use only the FIRST such plot as the 'master' for
        # min/max values for other plots of that type.
        #
        if plotType in plotAttrsByType:
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
        plotAttrsByType[plotType] = attrs

    #
    # Restore the window's list of active plots
    #
    SetActivePlots(tuple(origActivePlotsList))

    #
    # Ok, now iterate over plots in all windows other than the
    # source window and set their min/max values to those of
    # the 'master(s)' identified in the loop above.
    #
    for win in range(len(globAtts.windows)):

        winID = globAtts.windows[win]

        #
        # Skip the 'master' window
        #
        if winID == srcWin:
            continue

        SetActiveWindow(winID)

        origActivePlotsList = []
        thePlots = GetPlotList()
        for p in range(thePlots.GetNumPlots()):

            #
            # Skip hidden plots
            #
            if thePlots.GetPlots(p).hiddenFlag == 1:
                continue;

            #
            # Since we're going to be calling SetActivePlots,
            # remember original setting.
            #
            if thePlots.GetPlots(p).activeFlag == 1:
                origActivePlotsList.append(p)

            #
            # Get this plot's type string.
            #
            plotType = PlotPlugins()[thePlots.GetPlots(p).plotType]

            #
            # If we don't have a 'master' for it, skip it.
            #
            if plotType not in plotAttrsByType:
                continue

            #
            # Update the min/max settings for this plot.
            #
            newAttrs = plotAttrsByType[plotType]
            SetActivePlots((p,))
            currentAttrs = GetPlotOptions()
            currentAttrs.minFlag = 1 
            currentAttrs.min     = newAttrs.min
            currentAttrs.maxFlag = 1
            currentAttrs.max     = newAttrs.max
            SetPlotOptions(currentAttrs)
	    DrawPlots()

        #
        # Restore this window's active plot list
        #
        SetActivePlots(tuple(origActivePlotsList))

    #
    # Restore the active window
    #
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
RegisterMacro("SyncMinMaxTo1", SyncMinMax1)
RegisterMacro("SyncMinMaxTo2", SyncMinMax2)
RegisterMacro("SyncMinMaxTo3", SyncMinMax3)
RegisterMacro("SyncMinMaxTo4", SyncMinMax4)

#
# Provide the equivalent of VCR step forward/backward
# buttons that also do the work to sync min/max. But,
# if we define these for all 4 windows, we'd define
# another 8 buttons. So, we do it just for window 1.
# The user will have to accept the constraint that
# window 1 will have to act as the master for these
# to be useful.
#
RegisterMacro("NextAndSyncTo1", NextAndSync1)
RegisterMacro("PrevAndSyncTo1", PrevAndSync1)
