extract_options_exportFormat = "FieldViewXDB_1.0"
extract_options_writeUsingGroups = 0;
extract_options_writeGroupSize = 1;

def extract_set_options(fmt, writeUsingGroups, groupSize):
    extract_options_exportFormat = fmt
    extract_options_writeUsingGroups = writeUsingGroups
    extract_options_writeGroupSize = groupSize

def extract_err(err):
    ret = ""
    if err == 0:
        ret = "Success"
    elif err == -1:
        ret = "Could not create plot and draw it."
    elif err == -2:
        ret = "Export failed."
    else:
        ret = "?"
    return ret

def export_visit(filebase, extractvars):
    retval = -1
    opts = VISIT_INVALID_HANDLE;
    hvars = VisIt_NameList_alloc()
    if hvars != VISIT_INVALID_HANDLE:
        VisIt_NameList_addName(hvars, "default")
        for v in extractvars:
            VisIt_NameList_addName(hvars, v)

        opts = VisIt_OptionList_alloc()
        if opts != VISIT_INVALID_HANDLE:
            VisIt_OptionList_setValueI(opts, VISIT_EXPORT_WRITE_USING_GROUPS,
                                       extract_options_writeUsingGroups)
            VisIt_OptionList_setValueI(opts, VISIT_EXPORT_GROUP_SIZE,
                                       extract_options_writeGroupSize)

        if VisItExportDatabaseWithOptions(filebase, extract_options_exportFormat,
                                          hvars, opts) == VISIT_OKAY:
            retval = 0
        else:
            retval = -2

        if opts != VISIT_INVALID_HANDLE:
            VisIt_OptionList_free(opts)

        VisIt_NameList_free(hvars)

    return retval


def extract_slice_origin_normal(filebase, origin, normal, extractvars):
    retval = -1
    if len(filebase) == 0 or len(origin) != 3 or len(normal) != 3 or len(extractvars) == 0:
        return retval;

    if VisItAddPlot("Pseudocolor", extractvars[0]) == VISIT_OKAY:
        if VisItAddOperator("Slice", 0) == VISIT_OKAY:
            VisItSetOperatorOptionsI("axisType", 3) # arbitrary 
            VisItSetOperatorOptionsI("originType", 0) # point intercept 
            VisItSetOperatorOptionsDv("originPoint", origin, 3)
            VisItSetOperatorOptionsDv("normal", normal, 3)
            VisItSetOperatorOptionsB("project2d", 0)

            if VisItDrawPlots() == VISIT_OKAY:
                retval = export_visit(filebase, extractvars)

        VisItDeleteActivePlots()

    return retval

def extract_slice_3v(filebase, v0, v1, v2, extractvars):
    origin = [0,0,0]
    normal = [0,0,0]
    vec1 = [0,0,0]
    vec2 = [0,0,0]

    origin[0] = v0[0]
    origin[1] = v0[1]
    origin[2] = v0[2]

    vec1[0] = v1[0] - v0[0]
    vec1[1] = v1[1] - v0[1]
    vec1[2] = v1[2] - v0[2]
    mag = math.sqrt(vec1[0]*vec1[0] + vec1[1]*vec1[1] + vec1[2]*vec1[2])
    if mag > 0.:
        vec1[0] /= mag
        vec1[1] /= mag
        vec1[2] /= mag

    vec2[0] = v2[0] - v0[0]
    vec2[1] = v2[1] - v0[1]
    vec2[2] = v2[2] - v0[2]
    mag = math.sqrt(vec2[0]*vec2[0] + vec2[1]*vec2[1] + vec2[2]*vec2[2])
    if mag > 0.:
        vec2[0] /= mag
        vec2[1] /= mag
        vec2[2] /= mag

    normal[0] = vec1[1]*vec2[2] - vec1[2]*vec2[1]
    normal[1] = vec1[2]*vec2[0] - vec1[0]*vec2[2]
    normal[2] = vec1[0]*vec2[1] - vec1[1]*vec2[0]

    return extract_slice_origin_normal(filebase, origin, normal, extractvars)

def extract_slice(filebase, axis, intercept, extractvars):
    retval = -1
    if len(filebase) == 0 or len(extractvars) == 0:
        return retval

    if VisItAddPlot("Pseudocolor", extractvars[0]) == VISIT_OKAY:
        if VisItAddOperator("Slice", 0) == VISIT_OKAY:
            VisItSetOperatorOptionsI("axisType", axis)
            VisItSetOperatorOptionsI("originType", 1) # intercept 
            VisItSetOperatorOptionsD("originIntercept", intercept)
            VisItSetOperatorOptionsB("project2d", 0)

            if VisItDrawPlots() == VISIT_OKAY:
                retval = export_visit(filebase, extractvars)

        VisItDeleteActivePlots()

    return retval

def extract_iso(filebase, isovar, isovalues, extractvars):
    retval = -1
    if len(filebase) == 0 or len(isovar) == 0 or len(isovalues) == 0 or len(extractvars) == 0:
         return -1

    if VisItAddPlot("Contour", isovar) == VISIT_OKAY:
        VisItSetPlotOptionsI("contourMethod", 1) # value
        VisItSetPlotOptionsDv("contourValue", isovalues, nisovalues)
        if VisItDrawPlots() == VISIT_OKAY:
            retval = export_visit(filebase, extractvars)

        VisItDeleteActivePlots()

    return retval


