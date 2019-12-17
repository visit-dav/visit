# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

###############################################################################
# file: writescript.py
# Purpose: This function can interrogate the current state of VisIt to write out
#          a Python script that can mostly replicate the visualizations. Some
#          more work is needed but already this is interesting.
#
# Programmer: Brad Whitlock
# Creation:   Wed Mar  7 16:46:53 PST 2012
#
# Modifications:
#   Eric Brugger, Fri Jan  4 10:17:07 PST 2013
#   I replaced a call to GetCurveView with GetViewCurve, since the former
#   was incorrect. This fix was provided by Jean Favre.
#
###############################################################################

import string, sys
visit = sys.modules['visit']

def WriteScript(f):
    """Write Python code to replicate the current VisIt state to the specified file object.

Example:
    f = open('script.py', 'wt')
    WriteScript(f)
    f.close()
    """
    def object_type(obj):
        return str(type(obj))[7:-2]

    def write_state_object_atts(f, atts, name):
        lines = str(atts).split('\n')
        for line in lines:
           if line == '' or line[0] == '#':
               continue
           if '#' in line:
               pos = string.find(line, "=")
               if '"' in line[pos+2:]:
                   f.write('%s.%s\n' % (name, line))
               else:
                   f.write('%s.%s%s.%s\n' % (name, line[:pos+2], name, line[pos+2:]))
           else:
               f.write('%s.%s\n' % (name, line))

    def write_state_object(f, atts, name):
        typename = object_type(atts)
        f.write('%s = %s()\n' % (name, typename))
        write_state_object_atts(f, atts, name)

    def write_state_object_atts_diffs(f, atts, defaultatts, name):
        lines = str(atts).split('\n')
        defaultlines = str(defaultatts).split('\n')
        if len(lines) != len(defaultlines):
            write_state_object_atts(f, atts, name)
            return 1
        wroteline = 0
        for i in range(len(lines)):
            line = lines[i]
            if line == '' or line[0] == '#':
                continue
            # If the lines are the same then don't output the line.
            if line == defaultlines[i]:
                continue
            if '#' in line:
                pos = string.find(line, "=")
                if '"' in line[pos+2:]:
                    f.write('%s.%s\n' % (name, line))
                    wroteline = 1
                else:
                    f.write('%s.%s%s.%s\n' % (name, line[:pos+2], name, line[pos+2:]))
                    wroteline = 1
            else:
                f.write('%s.%s\n' % (name, line))
                wroteline = 1
        return wroteline

    def write_state_object_diffs(f, atts, defaultatts, name):
        typename = object_type(atts)
        f.write('%s = %s()\n' % (name, typename))
        return write_state_object_atts_diffs(f, atts, defaultatts, name)

    def write_color_tables(f):
        f.write('# Write color tables\n')
        # The list of built-in color table names.
        builtin_cts = ('Accent', 'Blues', 'BrBG', 'BuGn', 'BuPu', 'Dark2', 
'GnBu', 'Greens', 'Greys', 'OrRd', 'Oranges', 'PRGn', 'Paired', 'Pastel1', 
'Pastel2', 'PiYG', 'PuBu', 'PuBuGn', 'PuOr', 'PuRd', 'Purples', 'RdBu', 
'RdGy', 'RdPu', 'RdYlBu', 'RdYlGn', 'Reds', 'Set1', 'Set2', 'Set3', 
'Spectral', 'YlGn', 'YlGnBu', 'YlOrBr', 'YlOrRd', 'amino_rasmol', 
'amino_shapely', 'bluehot', 'caleblack', 'calewhite', 'contoured', 
'cpk_jmol', 'cpk_rasmol', 'difference', 'gray', 'hot', 'hot_and_cold',
'hot_desaturated', 'levels', 'orangehot', 'rainbow', 'xray')
        ctlist = visit.ColorTableNames()
        for name in ctlist:
            # Only write the color tables not in builtin_cts. Assume they are unchanged.
            if not name in builtin_cts:
                ct = visit.GetColorTable(name)
                f.write('ct = ColorControlPointList()\n')
                for i in range(ct.GetNumControlPoints()):
                    pname = "p%d" % i
                    f.write('%s = ColorControlPoint()\n' % pname)
                    f.write('%s.colors = %s\n' % (pname, str(ct.GetControlPoints(i).colors)))
                    f.write('%s.position = %s\n' % (pname, str(ct.GetControlPoints(i).position)))
                    f.write('ct.AddControlPoints(%s)\n' % pname)
                f.write('AddColorTable("%s", ct)\n' % name)

    def write_sil(f):
        silr = visit.SILRestriction()
        nsets = [0,0]
        for setid in range(silr.NumSets()):
            if silr.UsesData(setid):
                nsets[1] += 1
            else:
                nsets[0] += 1

        f.write('silr = SILRestriction()\n')
        if nsets[1] == silr.NumSets():
            f.write('silr.TurnOnAll()\n')    
        elif nsets[0] < nsets[1]:
            # More sets were on so we'll turn them all on and then turn off sets
            # that were off.
            sets = []
            for setid in range(silr.NumSets()):
                if not silr.UsesData(setid) and len(silr.MapsOut(setid)) == 0:
                    sets.append(setid)

            if len(sets) == 0:
                f.write('silr.TurnOffAll()\n')
            elif len(sets) == 1:
                f.write('silr.TurnOnAll()\n')
                f.write('silr.TurnOffSet("%s")\n' % silr.SetName(sets[0]))
            else:
                f.write('silr.SuspendCorrectnessChecking()\n')
                f.write('silr.TurnOnAll()\n')
                if 0: #For now. silr.SetIndex() is buggy. #if len(sets) < 100:
                    line = 'offSets = ('
                    linelen = len(line)
                    for i in range(len(sets)):
                        s = '"%s", ' % silr.SetName(sets[i])
                        line = line + s
                        linelen = linelen + len(s)
                        if linelen > 80:
                           line = line + '\n'
                           linelen = 0
                    line = line + ')\n'
                    f.write('%s' % line)
                    f.write('offSets = (')
                    for i in range(len(sets)):
                        f.write('"%s", ' % silr.SetName(sets[i]))
                    f.write(')\n')
                    f.write('for silSet in offSets:\n')
                    f.write('    silr.TurnOffSet(silr.SetIndex(silSet))\n')
                else:
                    line = 'offSets = ('
                    linelen = len(line)
                    for i in range(len(sets)):
                        s = '%d, ' % sets[i]
                        line = line + s
                        linelen = linelen + len(s)
                        if linelen > 80:
                           line = line + '\n'
                           linelen = 0
                    line = line + ')\n'
                    f.write('%s' % line)
                    f.write('for silSet in offSets:\n')
                    f.write('    silr.TurnOffSet(silSet)\n')
                f.write('silr.EnableCorrectnessChecking()\n')
        else:
            # More sets were off so we'll turn them all off and then turn on sets
            # that were on.
            sets = []
            for setid in range(silr.NumSets()):
                if silr.UsesData(setid) and len(silr.MapsOut(setid)) == 0:
                    sets.append(setid)

            if len(sets) == 0:
                f.write('silr.TurnOffAll()\n')
            elif len(sets) == 1:
                f.write('silr.TurnOffAll()\n')
                f.write('silr.TurnOnSet("%s")\n' % silr.SetName(sets[0]))
            else:
                f.write('silr.SuspendCorrectnessChecking()\n')
                f.write('silr.TurnOffAll()\n')
                if 0: #For now. silr.SetIndex() is buggy. #if len(sets) < 100:
                    line = 'onSets = ('
                    linelen = len(line)
                    for i in range(len(sets)):
                        s = '"%s", ' % silr.SetName(sets[i])
                        line = line + s
                        linelen = linelen + len(s)
                        if linelen > 80:
                           line = line + '\n'
                           linelen = 0
                    line = line + ')\n'
                    f.write('%s' % line)
                    f.write('for silSet in onSets:\n')
                    f.write('    silr.TurnOnSet(silr.SetIndex(silSet))\n')
                else:
                    line = 'onSets = ('
                    linelen = len(line)
                    for i in range(len(sets)):
                        s = '%d, ' % sets[i]
                        line = line + s
                        linelen = linelen + len(s)
                        if linelen > 80:
                           line = line + '\n'
                           linelen = 0
                    line = line + ')\n'
                    f.write('%s' % line)
                    f.write('for silSet in onSets:\n')
                    f.write('    silr.TurnOnSet(silSet)\n')
                f.write('silr.EnableCorrectnessChecking()\n')
        f.write('SetPlotSILRestriction(silr, 0)\n')

    def write_sil99(f):
        silr = visit.SILRestriction()
        f.write('silr = SILRestriction()\n')
        if silr.UsesAllData():
            f.write('silr.TurnOnAll()\n')
        else:
            f.write('silr.SuspendCorrectnessChecking()\n')
            onSets = []
            offSets = []
            for cat in silr.Categories():
                sets = silr.SetsInCategory(cat)
                for s in sets:
                    enabled = silr.UsesData(s)
                    if enabled:
                        onSets.append(s)
                    else:
                        offSets.append(s)
            if len(onSets) > len(offSets):
                f.write('silr.TurnOnAll()\n')
                if len(offSets) < 100:
                    # With a small number of sets, mention them by name.
                    f.write('offSets = (')
                    for s in offSets:
                        f.write('"%s", ' % silr.SetName(s))
                    f.write(')\n')
                    f.write('for setName in offSets:\n')
                    f.write('    silr.TurnOffSet(silr.SetIndex(setName))\n')
                else:
                    f.write('offSets = (')
                    for s in offSets:
                        f.write('%d, ' % s)
                    f.write(')\n')
                    f.write('for setIndex in offSets:\n')
                    f.write('    silr.TurnOffSet(setIndex)\n')
            else:
                f.write('silr.TurnOffAll()\n')
                if len(onSets) < 100:
                    # With a small number of sets, mention them by name.
                    f.write('onSets = (')
                    for s in onSets:
                        f.write('"%s", ' % silr.SetName(s))
                    f.write(')\n')
                    f.write('for setName in onSets:\n')
                    f.write('    silr.TurnOnSet(silr.SetIndex(setName))\n')
                else:
                    f.write('onSets = (')
                    for s in onSets:
                        f.write('%d, ' % s)
                    f.write(')\n')
                    f.write('for setIndex in onSets:\n')
                    f.write('    silr.TurnOnSet(setIndex)\n')
            f.write('silr.EnableCorrectnessChecking()\n')
        f.write('SetPlotSILRestriction(silr, 0)\n')

    def write_plots(f):
        f.write('# Create plots\n')
        pL = visit.GetPlotList()
        activePlots = []
        for i in range(visit.GetNumPlots()):
            plot = pL.GetPlots(i)
            if plot.activeFlag:
                activePlots = activePlots + [i]
            plotName = visit.PlotPlugins()[plot.plotType]
            visit.SetActivePlots(i)
            f.write('# Create plot %d\n' % (i+1))
            f.write('OpenDatabase("%s")\n' % plot.databaseName)
            f.write('AddPlot("%s", "%s", 0, 0)\n' % (plotName, plot.plotVar))
            atts = eval("visit." + plotName + "Attributes(1)")
            defaultatts = eval("visit." + plotName + "Attributes()")
            if write_state_object_diffs(f, atts, defaultatts, "atts"):
                f.write('SetPlotOptions(atts)\n')
            else:
                f.write('#SetPlotOptions(atts)\n')

            opIndex = 0
            for op in plot.operatorNames:
                 f.write('AddOperator("%s", 0)\n' % op)
                 opAtts = visit.GetOperatorOptions(opIndex)
                 defaultAtts = eval("visit." + object_type(opAtts) + "()")
                 if write_state_object_diffs(f, opAtts, defaultAtts, "opAtts"):
                     f.write('SetOperatorOptions(opAtts)\n')
                 else:
                     f.write('#SetOperatorOptions(opAtts)\n')
                 opIndex += 1
    
            # Set the SIL restriction
            write_sil(f)

            f.write('\n')
        if len(activePlots) > 1:
            f.write('SetActivePlots(%s)\n\n' % str(activePlots))
            visit.SetActivePlots(activePlots)
        elif len(activePlots) == 1:
            f.write('SetActivePlots(%d)\n\n' % activePlots[0])
            visit.SetActivePlots(activePlots[0])

    def set_annotation_objects(f, prefix):
        f.write('# Set annotation object properties\n')
        names = visit.GetAnnotationObjectNames()
        index = 0
        plotNames = []
        for name in names:
            obj = visit.GetAnnotationObject(name)
            objtype = object_type(obj)
            newname = prefix + "obj%03d" % index
            index = index + 1
            if objtype == 'LineObject':
                f.write('%s = CreateAnnotationObject("Line2D", "%s")\n' % (newname, name))
            elif objtype == 'Text2DObject':
                f.write('%s = CreateAnnotationObject("Text2D", "%s")\n' % (newname, name))
            elif objtype == 'ImageObject':
                f.write('%s = CreateAnnotationObject("Image", "%s")\n' % (newname, name))
            elif objtype == 'TimeSliderObject':
                f.write('%s = CreateAnnotationObject("TimeSlider", "%s")\n' % (newname, name))
            elif objtype == 'LegendAttributesObject':
                plotNames = plotNames + [name]
                continue
            write_state_object_atts(f, obj, newname)
            f.write('\n')
        index = 0 
        for name in plotNames:
            legend = prefix + "legend%03d" % index
            f.write('%s = GetAnnotationObject(GetPlotList().GetPlots(%d).plotName)\n' % (legend, index))
            write_state_object_atts(f, visit.GetAnnotationObject(name), legend)
            index += 1
        f.write('\n')

    def write_window(f, prefix):
        # Window setup
        f.write('width, height = %d, %d\n' % visit.GetWindowInformation().windowSize)
        f.write('win = GetGlobalAttributes().windows[GetGlobalAttributes().activeWindow]\n')
        f.write('ResizeWindow(win, width, height)\n')
        f.write('SetActiveWindow(win) # Synchronize\n')
        f.write('size = GetWindowInformation().windowSize\n')
        f.write('if width < size[0] or height < size[1]:\n')
        f.write('    ResizeWindow(win, width + (size[0] - width), height + (size[1] - height))\n')

        f.write('DeleteAllPlots()\n')
        f.write('for name in GetAnnotationObjectNames():\n')
        f.write('    DeleteAnnotationObject(name)\n')
        f.write('\n')

        # Write the plots.
        write_plots(f)
        f.write('DrawPlots()\n\n')

        # Set the view
        f.write('# Set the view\n')
        if visit.GetWindowInformation().viewDimension == 1:
            view = visit.GetViewCurve()
        elif visit.GetWindowInformation().viewDimension == 2:
            view = visit.GetView2D()
        else:
            view = visit.GetView3D()
        write_state_object(f, view, "view")
        if visit.GetWindowInformation().viewDimension == 1:
            f.write('SetViewCurve(view)\n')
        elif visit.GetWindowInformation().viewDimension == 2:
            f.write('SetView2D(view)\n')
        else:
            f.write('SetView3D(view)\n')
        f.write('\n');

        # Get the annotation attributes
        f.write('# Set the annotation attributes\n')
        annot = visit.GetAnnotationAttributes()
        if write_state_object_diffs(f, annot, visit.AnnotationAttributes(), "annot"):
            f.write('SetAnnotationAttributes(annot)\n\n')
        else:
            f.write('#SetAnnotationAttributes(annot)\n\n')

        set_annotation_objects(f, prefix)

    # Define expressions
    f.write('# Define expressions\n')
    expr = visit.ExpressionList(1)
    for i in range(expr.GetNumExpressions()):
        e = expr.GetExpressions(i)
        if e.fromDB == 0 and e.fromOperator == 0:
            if e.type == e.ScalarMeshVar:
                f.write('DefineScalarExpression("%s", "%s")\n' % (e.name, e.definition))
            elif e.type == e.VectorMeshVar:
                f.write('DefineVectorExpression("%s", "%s")\n' % (e.name, e.definition))
            elif e.type == e.TensorMeshVar:
                f.write('DefineTensorExpression("%s", "%s")\n' % (e.name, e.definition))
            elif e.type == e.ArrayMeshVar:
                f.write('DefineArrayExpression("%s", "%s")\n' % (e.name, e.definition))
            elif e.type == e.CurveMeshVar:
                f.write('DefineCurveExpression("%s", "%s")\n' % (e.name, e.definition))
            elif e.type == e.Mesh:
                f.write('DefineMeshExpression("%s", "%s")\n' % (e.name, e.definition))

    # Write the definitions of any non-standard color tables.
    write_color_tables(f)

    g = visit.GetGlobalAttributes()
    index = 0
    f.write('SetCloneWindowOnFirstRef(0)\n')
    for win in g.windows:
        visit.SetActiveWindow(win)
        f.write('###############################################################################\n')
        if index > 0:
            f.write('AddWindow()\n')

        write_window(f, "win%d_" % index)
        index += 1
    f.write('SetActiveWindow(GetGlobalAttributes().windows[%d])\n' % g.activeWindow)
    visit.SetActiveWindow(g.windows[g.activeWindow])
