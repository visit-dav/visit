#include <VisItControlInterface_V2.h>
#include <VisItDataInterface_V2.h>

#include <stdlib.h>
#include <math.h>

static const char *exportFormat = "FieldViewXDB_1.0";

static int export_visit(const char *filebase, const char **extractvars)
{
    int retval = -1;
    const char *var = NULL;
    visit_handle vars = VISIT_INVALID_HANDLE;
    if(VisIt_NameList_alloc(&vars))
    {
        VisIt_NameList_addName(vars, "default");
        while((var = *extractvars++) != NULL)
            VisIt_NameList_addName(vars, var);
        
        if(VisItExportDatabase(filebase, exportFormat, vars) == VISIT_OKAY)
            retval = 0;
        else
            retval = -2;
    }

    VisIt_NameList_free(vars);

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
            VisItSetOperatorOptionsI("originType", 0); // point intercept
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
            VisItSetOperatorOptionsI("originType", 1); // intercept
            VisItSetOperatorOptionsD("originIntercept", intercept);
            VisItSetOperatorOptionsI("axisType", axis);
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
        VisItSetPlotOptionsI("contourMethod", 1); // value
        VisItSetPlotOptionsDv("contourValue", isovalues, nisovalues);
        if(VisItDrawPlots() == VISIT_OKAY)
        {
            retval = export_visit(filebase, extractvars);
        }

        VisItDeleteActivePlots();
    }

    return retval;
}

