#include <VisItControlInterface_V2.h>
#include <VisItDataInterface_V2.h>

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static char extract_options_exportFormat[30] = {
'F','i','e','l','d','V','i','e','w','X','D','B','_','1','.','0',
'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};

static int extract_options_writeUsingGroups = 0;
static int extract_options_writeGroupSize = 1;

void extract_set_options(const char *fmt, int writeUsingGroups, int groupSize)
{
    strncpy(extract_options_exportFormat, fmt, 30);
    extract_options_writeUsingGroups = writeUsingGroups;
    extract_options_writeGroupSize = groupSize;
}

const char *
extract_err(int err)
{
    const char *ret;
    switch(err)
    {
    case 0:
        ret = "Success";
        break;
    case -1:
        ret = "Could not create plot and draw it.";
        break;
    case -2:
        ret = "Export failed.";
        break;
    default:
        ret = "?";
        break;
    }
    return ret;
}

static int export_visit(const char *filebase, const char **extractvars)
{
    int retval = -1;
    const char *var = NULL;
    visit_handle vars = VISIT_INVALID_HANDLE, opts = VISIT_INVALID_HANDLE;
    if(VisIt_NameList_alloc(&vars))
    {
        VisIt_NameList_addName(vars, "default");
        while((var = *extractvars++) != NULL)
            VisIt_NameList_addName(vars, var);

        if(VisIt_OptionList_alloc(&opts))
        {
            VisIt_OptionList_setValueI(opts, VISIT_EXPORT_WRITE_USING_GROUPS,
                                       extract_options_writeUsingGroups);
            VisIt_OptionList_setValueI(opts, VISIT_EXPORT_GROUP_SIZE,
                                       extract_options_writeGroupSize);
        }

        if(VisItExportDatabaseWithOptions(filebase, extract_options_exportFormat,
                                          vars, opts) == VISIT_OKAY)
            retval = 0;
        else
            retval = -2;

        if(opts != VISIT_INVALID_HANDLE)
            VisIt_OptionList_free(opts);
    }

    VisIt_NameList_free(vars);

    return retval;
}

int extract_slice_origin_normal(const char *filebase,
                                const double *origin,
                                const double *normal,
                                const char **extractvars)
{
    int retval = -1;
    if(filebase == NULL || origin == NULL || normal == NULL || extractvars == NULL)
        return retval;

    if(VisItAddPlot("Pseudocolor", extractvars[0]) == VISIT_OKAY)
    {
        if(VisItAddOperator("Slice", 0) == VISIT_OKAY)
        {
            VisItSetOperatorOptionsI("axisType", 3); /* arbitrary */
            VisItSetOperatorOptionsI("originType", 0); /* point intercept */
            VisItSetOperatorOptionsDv("originPoint", origin, 3);
            VisItSetOperatorOptionsDv("normal", normal, 3);
            VisItSetOperatorOptionsB("project2d", 0);


            if(VisItDrawPlots() == VISIT_OKAY)
            {
                retval = export_visit(filebase, extractvars);
            }
        }

        VisItDeleteActivePlots();
    }

    return retval;
}

int extract_slice_3v(const char *filebase, 
                     const double *v0, const double *v1, const double *v2,
                     const char **extractvars)
{
    double origin[3], normal[3], vec1[3], vec2[3], mag;

    origin[0] = v0[0];
    origin[1] = v0[1];
    origin[2] = v0[2];

    vec1[0] = v1[0] - v0[0];
    vec1[1] = v1[1] - v0[1];
    vec1[2] = v1[2] - v0[2];
    mag = sqrt(vec1[0]*vec1[0] + vec1[1]*vec1[1] + vec1[2]*vec1[2]);
    if(mag > 0.)
    {
        vec1[0] /= mag;
        vec1[1] /= mag;
        vec1[2] /= mag;
    }

    vec2[0] = v2[0] - v0[0];
    vec2[1] = v2[1] - v0[1];
    vec2[2] = v2[2] - v0[2];
    mag = sqrt(vec2[0]*vec2[0] + vec2[1]*vec2[1] + vec2[2]*vec2[2]);
    if(mag > 0.)
    {
        vec2[0] /= mag;
        vec2[1] /= mag;
        vec2[2] /= mag;
    }

    normal[0] = vec1[1]*vec2[2] - vec1[2]*vec2[1];
    normal[1] = vec1[2]*vec2[0] - vec1[0]*vec2[2];
    normal[2] = vec1[0]*vec2[1] - vec1[1]*vec2[0];

    return extract_slice_origin_normal(filebase, origin, normal, extractvars);
}

int extract_slice(const char *filebase, int axis, double intercept, 
    const char **extractvars)
{
    int retval = -1;
    if(filebase == NULL || extractvars == NULL)
        return retval;

    if(VisItAddPlot("Pseudocolor", extractvars[0]) == VISIT_OKAY)
    {
        if(VisItAddOperator("Slice", 0) == VISIT_OKAY)
        {
            VisItSetOperatorOptionsI("axisType", axis);
            VisItSetOperatorOptionsI("originType", 1); /* intercept */
            VisItSetOperatorOptionsD("originIntercept", intercept);
            VisItSetOperatorOptionsB("project2d", 0);

            if(VisItDrawPlots() == VISIT_OKAY)
            {
                retval = export_visit(filebase, extractvars);
            }
        }

        VisItDeleteActivePlots();
    }

    return retval;
}

int extract_iso(const char *filebase, const char *isovar,
                const double *isovalues, int nisovalues,
                const char **extractvars)
{
    int retval = -1;
    if(filebase == NULL || isovar == NULL || isovalues == NULL || extractvars == NULL)
         return -1;

    if(VisItAddPlot("Contour", isovar) == VISIT_OKAY)
    {
        VisItSetPlotOptionsI("contourMethod", 1); /* value */
        VisItSetPlotOptionsDv("contourValue", isovalues, nisovalues);
        if(VisItDrawPlots() == VISIT_OKAY)
        {
            retval = export_visit(filebase, extractvars);
        }

        VisItDeleteActivePlots();
    }

    return retval;
}

