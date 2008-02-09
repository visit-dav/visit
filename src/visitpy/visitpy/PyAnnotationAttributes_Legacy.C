#include <Python.h>
#include <visitstream.h>

#include <AnnotationAttributes.h>

//
// This module contains functions that are used by PyAnnotationAttributes to interpret
// pre 1.9 versions of the AnnotationAttributes and map the old structure into the newer,
// nested AnnotationAttributes structure.
//
// Notes:
//   This module uses functions from other generated Py* objects that are normally
//   declared static. We've had to go into Axes2D, Axes3D, ... and so on to make 
//   sure that their Set/Get methods are not static.
//

#define SET_VALUE5(A,B,C,D,E) \
        { \
        extern PyObject *A(PyObject *, PyObject*);\
        extern PyObject *B(PyObject *, PyObject*);\
        extern PyObject *C(PyObject *, PyObject*);\
        extern PyObject *D(PyObject *, PyObject*);\
        extern PyObject *E(PyObject *, PyObject*);\
        PyObject *obj[4] = {NULL,NULL,NULL,NULL}; \
        obj[0] = A(self, NULL); \
        obj[1] = B(obj[0], NULL); \
        obj[2] = C(obj[1], NULL); \
        obj[3] = D(obj[2], NULL); \
        retval = E(obj[3], tuple); \
        Py_DECREF(obj[3]); \
        Py_DECREF(obj[2]); \
        Py_DECREF(obj[1]); \
        Py_DECREF(obj[0]); }

#define SET_VALUE4(A,B,C,D) \
        { \
        extern PyObject *A(PyObject *, PyObject*);\
        extern PyObject *B(PyObject *, PyObject*);\
        extern PyObject *C(PyObject *, PyObject*);\
        extern PyObject *D(PyObject *, PyObject*);\
        PyObject *obj[3] = {NULL,NULL,NULL}; \
        obj[0] = A(self, NULL); \
        obj[1] = B(obj[0], NULL); \
        obj[2] = C(obj[1], NULL); \
        retval = D(obj[2], tuple); \
        Py_DECREF(obj[2]); \
        Py_DECREF(obj[1]); \
        Py_DECREF(obj[0]); }

#define SET_VALUE3(A,B,C) \
        { \
        extern PyObject *A(PyObject *, PyObject*);\
        extern PyObject *B(PyObject *, PyObject*);\
        extern PyObject *C(PyObject *, PyObject*);\
        PyObject *obj[2] = {NULL,NULL}; \
        obj[0] = A(self, NULL); \
        obj[1] = B(obj[0], NULL); \
        retval = C(obj[1], tuple); \
        Py_DECREF(obj[1]); \
        Py_DECREF(obj[0]); }

#define SET_VALUE2(A,B) \
        { \
        extern PyObject *A(PyObject *, PyObject*);\
        extern PyObject *B(PyObject *, PyObject*);\
        PyObject *obj = NULL; \
        obj = A(self, NULL); \
        retval = B(obj, tuple); \
        Py_DECREF(obj); }

static void
DEPRECATION_WARNING(const char *name)
{
    cerr << "Warning: The \"" << name << "\" field in AnnotationAttributes has " 
            "been deprecated but VisIt has set the equivalent field in the new " 
            "AnnotationAttributes object. Please update your script so it uses " 
            "the new AnnotationAttributes object since compatibility for the " 
            "old style of AnnotationAttributes object will be removed in a future "
            "version of VisIt. You can determine the structure of the new "
            "AnnotationAttributes object by printing it."  << endl;
}

// ****************************************************************************
// Method: AnnotationAttributes_Legacy_setattr
//
// Purpose: 
//   Maps the pre 1.9 AnnotationAttribute fields into post 1.9 AnnotationAttributes
//   fields.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 29 09:06:21 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

PyObject *
AnnotationAttributes_Legacy_setattr(PyObject *self, const char *name, PyObject *tuple)
{
    PyObject *retval = NULL;

    if(strcmp(name, "axesFlag2D") == 0)
    {
        DEPRECATION_WARNING("axesFlag2D");
        SET_VALUE2(AnnotationAttributes_GetAxes2D,
                   Axes2D_SetVisible);
    }
    else if(strcmp(name, "axesAutoSetTicks2D") == 0)
    {
        DEPRECATION_WARNING("axesAutoSetTicks2D");
        SET_VALUE2(AnnotationAttributes_GetAxes2D,
                   Axes2D_SetAutoSetTicks);
    }
    else if(strcmp(name, "labelAutoSetScaling2D") == 0)
    {
        DEPRECATION_WARNING("labelAutoSetScaling2D");
        SET_VALUE2(AnnotationAttributes_GetAxes2D,
                   Axes2D_SetAutoSetScaling);
    }
    else if(strcmp(name, "xAxisLabels2D") == 0)
    {
        DEPRECATION_WARNING("xAxisLabels2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_SetVisible);
    }
    else if(strcmp(name, "yAxisLabels2D") == 0)
    {
        DEPRECATION_WARNING("yAxisLabels2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_SetVisible);
    }
    else if(strcmp(name, "xAxisTitle2D") == 0)
    {
        DEPRECATION_WARNING("xAxisTitle2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetVisible);
    }
    else if(strcmp(name, "yAxisTitle2D") == 0)
    {
        DEPRECATION_WARNING("yAxisTitle2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetVisible);
    }
    else if(strcmp(name, "xGridLines2D") == 0)
    {
        DEPRECATION_WARNING("xGridLines2D");
        SET_VALUE3(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_SetGrid);
    }
    else if(strcmp(name, "yGridLines2D") == 0)
    {
        DEPRECATION_WARNING("yGridLines2D");
        SET_VALUE3(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_SetGrid);
    }
    else if(strcmp(name, "xMajorTickMinimum2D") == 0)
    {
        DEPRECATION_WARNING("xMajorTickMinimum2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMajorMinimum);
    }
    else if(strcmp(name, "yMajorTickMinimum2D") == 0)
    {
        DEPRECATION_WARNING("yMajorTickMinimum2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMajorMinimum);
    }
    else if(strcmp(name, "xMajorTickMaximum2D") == 0)
    {
        DEPRECATION_WARNING("xMajorTickMaximum2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMajorMaximum);
    }
    else if(strcmp(name, "yMajorTickMaximum2D") == 0)
    {
        DEPRECATION_WARNING("yMajorTickMaximum2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMajorMaximum);
    }
    else if(strcmp(name, "xMajorTickSpacing2D") == 0)
    {
        DEPRECATION_WARNING("xMajorTickSpacing2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMajorSpacing);
    }
    else if(strcmp(name, "yMajorTickSpacing2D") == 0)
    {
        DEPRECATION_WARNING("yMajorTickSpacing2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMajorSpacing);
    }
    else if(strcmp(name, "xMinorTickSpacing2D") == 0)
    {
        DEPRECATION_WARNING("xMinorTickSpacing2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMinorSpacing);
    }
    else if(strcmp(name, "yMinorTickSpacing2D") == 0)
    {
        DEPRECATION_WARNING("yMinorTickSpacing2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMinorSpacing);
    }
    else if(strcmp(name, "xLabelFontHeight2D") == 0)
    {
        DEPRECATION_WARNING("xLabelFontHeight2D");
        SET_VALUE5(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetFont,
                   FontAttributes_SetHeight);
    }
    else if(strcmp(name, "yLabelFontHeight2D") == 0)
    {
        DEPRECATION_WARNING("yLabelFontHeight2D");
        SET_VALUE5(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetFont,
                   FontAttributes_SetHeight);
    }
    else if(strcmp(name, "xTitleFontHeight2D") == 0)
    {
        DEPRECATION_WARNING("xTitleFontHeight2D");
        SET_VALUE5(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetFont,
                   FontAttributes_SetHeight);
    }
    else if(strcmp(name, "yTitleFontHeight2D") == 0)
    {
        DEPRECATION_WARNING("yTitleFontHeight2D");
        SET_VALUE5(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetFont,
                   FontAttributes_SetHeight);
    }
    else if(strcmp(name, "xLabelScaling2D") == 0)
    {
        DEPRECATION_WARNING("xLabelScaling2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_SetScaling);
    }
    else if(strcmp(name, "yLabelScaling2D") == 0)
    {
        DEPRECATION_WARNING("yLabelScaling2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_SetScaling);
    }
    else if(strcmp(name, "axesLineWidth2D") == 0)
    {
        DEPRECATION_WARNING("axesLineWidth2D");
        SET_VALUE2(AnnotationAttributes_GetAxes2D,
                   Axes2D_SetLineWidth);
    }
    else if(strcmp(name, "axesTickLocation2D") == 0)
    {
        DEPRECATION_WARNING("axesTickLocation2D");
        SET_VALUE2(AnnotationAttributes_GetAxes2D,
                   Axes2D_SetTickLocation);
    }
    else if(strcmp(name, "axesTicks2D") == 0)
    {
        DEPRECATION_WARNING("axesTicks2D");
        SET_VALUE2(AnnotationAttributes_GetAxes2D,
                   Axes2D_SetTickAxes);
    }
    else if(strcmp(name, "xAxisUserTitle2D") == 0)
    {
        DEPRECATION_WARNING("xAxisUserTitle2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetTitle);
    }
    else if(strcmp(name, "yAxisUserTitle2D") == 0)
    {
        DEPRECATION_WARNING("yAxisUserTitle2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetTitle);
    }
    else if(strcmp(name, "xAxisUserTitleFlag2D") == 0)
    {
        DEPRECATION_WARNING("xAxisUserTitleFlag2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetUserTitle);
    }
    else if(strcmp(name, "yAxisUserTitleFlag2D") == 0)
    {
        DEPRECATION_WARNING("yAxisUserTitleFlag2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetUserTitle);
    }
    else if(strcmp(name, "xAxisUserUnits2D") == 0)
    {
        DEPRECATION_WARNING("xAxisUserUnits2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetUnits);
    }
    else if(strcmp(name, "yAxisUserUnits2D") == 0)
    {
        DEPRECATION_WARNING("yAxisUserUnits2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetUnits);
    }
    else if(strcmp(name, "xAxisUserUnitsFlag2D") == 0)
    {
        DEPRECATION_WARNING("xAxisUserUnitsFlag2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetUserUnits);
    }
    else if(strcmp(name, "yAxisUserUnitsFlag2D") == 0)
    {
        DEPRECATION_WARNING("yAxisUserUnitsFlag2D");
        SET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetUserUnits);
    }
    else if(strcmp(name, "axesFlag") == 0)
    {
        DEPRECATION_WARNING("axesFlag");
        SET_VALUE2(AnnotationAttributes_GetAxes3D,
                   Axes3D_SetVisible);
    }
    else if(strcmp(name, "axesAutoSetTicks") == 0)
    {
        DEPRECATION_WARNING("axesAutoSetTicks");
        SET_VALUE2(AnnotationAttributes_GetAxes3D,
                   Axes3D_SetAutoSetTicks);
    }
    else if(strcmp(name, "labelAutoSetScaling") == 0)
    {
        DEPRECATION_WARNING("labelAutoSetScaling");
        SET_VALUE2(AnnotationAttributes_GetAxes3D,
                   Axes3D_SetAutoSetScaling);
    }
    else if(strcmp(name, "xAxisLabels") == 0)
    {
        DEPRECATION_WARNING("xAxisLabels");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_SetVisible);
    }
    else if(strcmp(name, "yAxisLabels") == 0)
    {
        DEPRECATION_WARNING("yAxisLabels");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_SetVisible);
    }
    else if(strcmp(name, "zAxisLabels") == 0)
    {
        DEPRECATION_WARNING("zAxisLabels");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_SetVisible);
    }
    else if(strcmp(name, "xAxisTitle") == 0)
    {
        DEPRECATION_WARNING("xAxisTitle");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetVisible);
    }
    else if(strcmp(name, "yAxisTitle") == 0)
    {
        DEPRECATION_WARNING("yAxisTitle");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetVisible);
    }
    else if(strcmp(name, "zAxisTitle") == 0)
    {
        DEPRECATION_WARNING("zAxisTitle");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetVisible);
    }
    else if(strcmp(name, "xGridLines") == 0)
    {
        DEPRECATION_WARNING("xGridLines");
        SET_VALUE3(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_SetGrid);
    }
    else if(strcmp(name, "yGridLines") == 0)
    {
        DEPRECATION_WARNING("yGridLines");
        SET_VALUE3(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_SetGrid);
    }
    else if(strcmp(name, "zGridLines") == 0)
    {
        DEPRECATION_WARNING("zGridLines");
        SET_VALUE3(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_SetGrid);
    }
    else if(strcmp(name, "xAxisTicks") == 0)
    {
        DEPRECATION_WARNING("xAxisTicks");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetVisible);
    }
    else if(strcmp(name, "yAxisTicks") == 0)
    {
        DEPRECATION_WARNING("yAxisTicks");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetVisible);
    }
    else if(strcmp(name, "zAxisTicks") == 0)
    {
        DEPRECATION_WARNING("zAxisTicks");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetVisible);
    }
    else if(strcmp(name, "xMajorTickMinimum") == 0)
    {
        DEPRECATION_WARNING("xMajorTickMinimum");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMajorMinimum);
    }
    else if(strcmp(name, "yMajorTickMinimum") == 0)
    {
        DEPRECATION_WARNING("yMajorTickMinimum");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMajorMinimum);
    }
    else if(strcmp(name, "zMajorTickMinimum") == 0)
    {
        DEPRECATION_WARNING("zMajorTickMinimum");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMajorMinimum);
    }
    else if(strcmp(name, "xMajorTickMaximum") == 0)
    {
        DEPRECATION_WARNING("xMajorTickMaximum");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMajorMaximum);
    }
    else if(strcmp(name, "yMajorTickMaximum") == 0)
    {
        DEPRECATION_WARNING("yMajorTickMaximum");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMajorMaximum);
    }
    else if(strcmp(name, "zMajorTickMaximum") == 0)
    {
        DEPRECATION_WARNING("zMajorTickMaximum");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMajorMaximum);
    }
    else if(strcmp(name, "xMajorTickSpacing") == 0)
    {
        DEPRECATION_WARNING("xMajorTickSpacing");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMajorSpacing);
    }
    else if(strcmp(name, "yMajorTickSpacing") == 0)
    {
        DEPRECATION_WARNING("yMajorTickSpacing");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMajorSpacing);
    }
    else if(strcmp(name, "zMajorTickSpacing") == 0)
    {
        DEPRECATION_WARNING("zMajorTickSpacing");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMajorSpacing);
    }
    else if(strcmp(name, "xMinorTickSpacing") == 0)
    {
        DEPRECATION_WARNING("xMinorTickSpacing");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMinorSpacing);
    }
    else if(strcmp(name, "yMinorTickSpacing") == 0)
    {
        DEPRECATION_WARNING("yMinorTickSpacing");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMinorSpacing);
    }
    else if(strcmp(name, "zMinorTickSpacing") == 0)
    {
        DEPRECATION_WARNING("zMinorTickSpacing");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_SetMinorSpacing);
    }
    else if(strcmp(name, "xLabelFontHeight") == 0)
    {
        DEPRECATION_WARNING("xLabelFontHeight");
        SET_VALUE5(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetFont,
                   FontAttributes_SetHeight);
    }
    else if(strcmp(name, "yLabelFontHeight") == 0)
    {
        DEPRECATION_WARNING("yLabelFontHeight");
        SET_VALUE5(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetFont,
                   FontAttributes_SetHeight);
    }
    else if(strcmp(name, "zLabelFontHeight") == 0)
    {
        DEPRECATION_WARNING("zLabelFontHeight");
        SET_VALUE5(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetFont,
                   FontAttributes_SetHeight);
    }
    else if(strcmp(name, "xTitleFontHeight") == 0)
    {
        DEPRECATION_WARNING("xTitleFontHeight");
        SET_VALUE5(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetFont,
                   FontAttributes_SetHeight);
    }
    else if(strcmp(name, "yTitleFontHeight") == 0)
    {
        DEPRECATION_WARNING("yTitleFontHeight");
        SET_VALUE5(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetFont,
                   FontAttributes_SetHeight);
    }
    else if(strcmp(name, "zTitleFontHeight") == 0)
    {
        DEPRECATION_WARNING("zTitleFontHeight");
        SET_VALUE5(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetFont,
                   FontAttributes_SetHeight);
    }
    else if(strcmp(name, "xLabelScaling") == 0)
    {
        DEPRECATION_WARNING("xLabelScaling");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_SetScaling);
    }
    else if(strcmp(name, "yLabelScaling") == 0)
    {
        DEPRECATION_WARNING("yLabelScaling");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_SetScaling);
    }
    else if(strcmp(name, "zLabelScaling") == 0)
    {
        DEPRECATION_WARNING("zLabelScaling");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_SetScaling);
    }
    else if(strcmp(name, "xAxisUserTitle") == 0)
    {
        DEPRECATION_WARNING("xAxisUserTitle");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetTitle);
    }
    else if(strcmp(name, "yAxisUserTitle") == 0)
    {
        DEPRECATION_WARNING("yAxisUserTitle");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetTitle);
    }
    else if(strcmp(name, "zAxisUserTitle") == 0)
    {
        DEPRECATION_WARNING("zAxisUserTitle");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetTitle);
    }
    else if(strcmp(name, "xAxisUserTitleFlag") == 0)
    {
        DEPRECATION_WARNING("xAxisUserTitleFlag");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetUserTitle);
    }
    else if(strcmp(name, "yAxisUserTitleFlag") == 0)
    {
        DEPRECATION_WARNING("yAxisUserTitleFlag");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetUserTitle);
    }
    else if(strcmp(name, "zAxisUserTitleFlag") == 0)
    {
        DEPRECATION_WARNING("zAxisUserTitleFlag");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetUserTitle);
    }
    else if(strcmp(name, "xAxisUserUnits") == 0)
    {
        DEPRECATION_WARNING("xAxisUserUnits");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetUnits);
    }
    else if(strcmp(name, "yAxisUserUnits") == 0)
    {
        DEPRECATION_WARNING("yAxisUserUnits");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetUnits);
    }
    else if(strcmp(name, "zAxisUserUnits") == 0)
    {
        DEPRECATION_WARNING("zAxisUserUnits");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetUnits);
    }
    else if(strcmp(name, "xAxisUserUnitsFlag") == 0)
    {
        DEPRECATION_WARNING("xAxisUserUnitsFlag");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetUserUnits);
    }
    else if(strcmp(name, "yAxisUserUnitsFlag") == 0)
    {
        DEPRECATION_WARNING("yAxisUserUnitsFlag");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetUserUnits);

    }
    else if(strcmp(name, "zAxisUserUnitsFlag") == 0)
    {
        DEPRECATION_WARNING("zAxisUserUnitsFlag");
        SET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_SetUserUnits);
    }
    else if(strcmp(name, "axesTickLocation") == 0)
    {
        DEPRECATION_WARNING("axesTickLocation");
        SET_VALUE2(AnnotationAttributes_GetAxes3D,
                   Axes3D_SetTickLocation);
    }
    else if(strcmp(name, "axesType") == 0)
    {
        DEPRECATION_WARNING("axesType");
        SET_VALUE2(AnnotationAttributes_GetAxes3D,
                   Axes3D_SetAxesType);
    }
    else if(strcmp(name, "triadFlag") == 0)
    {
        DEPRECATION_WARNING("triadFlag");
        SET_VALUE2(AnnotationAttributes_GetAxes3D,
                   Axes3D_SetTriadFlag);
    }
    else if(strcmp(name, "bboxFlag") == 0)
    {
        DEPRECATION_WARNING("bboxFlag");
        SET_VALUE2(AnnotationAttributes_GetAxes3D,
                   Axes3D_SetBboxFlag);
    }

    return retval;
}

#define GET_VALUE5(A,B,C,D,E) \
        { \
        extern PyObject *A(PyObject *, PyObject*);\
        extern PyObject *B(PyObject *, PyObject*);\
        extern PyObject *C(PyObject *, PyObject*);\
        extern PyObject *D(PyObject *, PyObject*);\
        extern PyObject *E(PyObject *, PyObject*);\
        PyObject *obj[5] = {NULL,NULL,NULL,NULL,NULL}; \
        obj[0] = A(self, NULL); \
        obj[1] = B(obj[0], NULL); \
        obj[2] = C(obj[1], NULL); \
        obj[3] = D(obj[2], NULL); \
        obj[4] = E(obj[3], NULL); \
        Py_DECREF(obj[3]); \
        Py_DECREF(obj[2]); \
        Py_DECREF(obj[1]); \
        Py_DECREF(obj[0]); \
        return obj[4]; }

#define GET_VALUE4(A,B,C,D) \
        { \
        extern PyObject *A(PyObject *, PyObject*);\
        extern PyObject *B(PyObject *, PyObject*);\
        extern PyObject *C(PyObject *, PyObject*);\
        extern PyObject *D(PyObject *, PyObject*);\
        PyObject *obj[4] = {NULL,NULL,NULL,NULL}; \
        obj[0] = A(self, NULL); \
        obj[1] = B(obj[0], NULL); \
        obj[2] = C(obj[1], NULL); \
        obj[3] = D(obj[2], NULL); \
        Py_DECREF(obj[2]); \
        Py_DECREF(obj[1]); \
        Py_DECREF(obj[0]); \
        return obj[3]; }

#define GET_VALUE3(A,B,C) \
        { \
        extern PyObject *A(PyObject *, PyObject*);\
        extern PyObject *B(PyObject *, PyObject*);\
        extern PyObject *C(PyObject *, PyObject*);\
        PyObject *obj[3] = {NULL,NULL,NULL}; \
        obj[0] = A(self, NULL); \
        obj[1] = B(obj[0], NULL); \
        obj[2] = C(obj[1], NULL); \
        Py_DECREF(obj[1]); \
        Py_DECREF(obj[0]); \
        return obj[2]; }

#define GET_VALUE2(A,B) \
        { \
        extern PyObject *A(PyObject *, PyObject*);\
        extern PyObject *B(PyObject *, PyObject*);\
        PyObject *obj[2] = {NULL,NULL}; \
        obj[0] = A(self, NULL); \
        obj[1] = B(obj[0], NULL); \
        Py_DECREF(obj[0]); \
        return obj[1]; }

// ****************************************************************************
// Method: AnnotationAttributes_Legacy_getattr
//
// Purpose: 
//   Maps the pre 1.9 AnnotationAttribute fields into post 1.9 AnnotationAttributes
//   fields.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 29 09:06:21 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

PyObject *
AnnotationAttributes_Legacy_getattr(PyObject *self, const char *name)
{
    if(strcmp(name, "axesFlag2D") == 0)
    {
        DEPRECATION_WARNING("axesFlag2D");
        GET_VALUE2(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetVisible);
    }
    else if(strcmp(name, "axesAutoSetTicks2D") == 0)
    {
        DEPRECATION_WARNING("axesAutoSetTicks2D");
        GET_VALUE2(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetAutoSetTicks);
    }
    else if(strcmp(name, "labelAutoSetScaling2D") == 0)
    {
        DEPRECATION_WARNING("labelAutoSetScaling2D");
        GET_VALUE2(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetAutoSetScaling);
    }
    else if(strcmp(name, "xAxisLabels2D") == 0)
    {
        DEPRECATION_WARNING("xAxisLabels2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetVisible);
    }
    else if(strcmp(name, "yAxisLabels2D") == 0)
    {
        DEPRECATION_WARNING("yAxisLabels2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetVisible);
    }
    else if(strcmp(name, "xAxisTitle2D") == 0)
    {
        DEPRECATION_WARNING("xAxisTitle2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetVisible);
    }
    else if(strcmp(name, "yAxisTitle2D") == 0)
    {
        DEPRECATION_WARNING("yAxisTitle2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetVisible);
    }
    else if(strcmp(name, "xGridLines2D") == 0)
    {
        DEPRECATION_WARNING("xGridLines2D");
        GET_VALUE3(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetGrid);
    }
    else if(strcmp(name, "yGridLines2D") == 0)
    {
        DEPRECATION_WARNING("yGridLines2D");
        GET_VALUE3(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetGrid);
    }
    else if(strcmp(name, "xMajorTickMinimum2D") == 0)
    {
        DEPRECATION_WARNING("xMajorTickMinimum2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMajorMinimum);
    }
    else if(strcmp(name, "yMajorTickMinimum2D") == 0)
    {
        DEPRECATION_WARNING("yMajorTickMinimum2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMajorMinimum);
    }
    else if(strcmp(name, "xMajorTickMaximum2D") == 0)
    {
        DEPRECATION_WARNING("xMajorTickMaximum2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMajorMaximum);
    }
    else if(strcmp(name, "yMajorTickMaximum2D") == 0)
    {
        DEPRECATION_WARNING("yMajorTickMaximum2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMajorMaximum);
    }
    else if(strcmp(name, "xMajorTickSpacing2D") == 0)
    {
        DEPRECATION_WARNING("xMajorTickSpacing2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMajorSpacing);
    }
    else if(strcmp(name, "yMajorTickSpacing2D") == 0)
    {
        DEPRECATION_WARNING("yMajorTickSpacing2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMajorSpacing);
    }
    else if(strcmp(name, "xMinorTickSpacing2D") == 0)
    {
        DEPRECATION_WARNING("xMinorTickSpacing2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMinorSpacing);
    }
    else if(strcmp(name, "yMinorTickSpacing2D") == 0)
    {
        DEPRECATION_WARNING("yMinorTickSpacing2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMinorSpacing);
    }
    else if(strcmp(name, "xLabelFontHeight2D") == 0)
    {
        DEPRECATION_WARNING("xLabelFontHeight2D");
        GET_VALUE5(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetFont,
                   FontAttributes_GetHeight);
    }
    else if(strcmp(name, "yLabelFontHeight2D") == 0)
    {
        DEPRECATION_WARNING("yLabelFontHeight2D");
        GET_VALUE5(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetFont,
                   FontAttributes_GetHeight);
    }
    else if(strcmp(name, "xTitleFontHeight2D") == 0)
    {
        DEPRECATION_WARNING("xTitleFontHeight2D");
        GET_VALUE5(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetFont,
                   FontAttributes_GetHeight);
    }
    else if(strcmp(name, "yTitleFontHeight2D") == 0)
    {
        DEPRECATION_WARNING("yTitleFontHeight2D");
        GET_VALUE5(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetFont,
                   FontAttributes_GetHeight);
    }
    else if(strcmp(name, "xLabelScaling2D") == 0)
    {
        DEPRECATION_WARNING("xLabelScaling2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetScaling);
    }
    else if(strcmp(name, "yLabelScaling2D") == 0)
    {
        DEPRECATION_WARNING("yLabelScaling2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetScaling);
    }
    else if(strcmp(name, "axesLineWidth2D") == 0)
    {
        DEPRECATION_WARNING("axesLineWidth2D");
        GET_VALUE2(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetLineWidth);
    }
    else if(strcmp(name, "axesTickLocation2D") == 0)
    {
        DEPRECATION_WARNING("axesTickLocation2D");
        GET_VALUE2(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetTickLocation);
    }
    else if(strcmp(name, "axesTicks2D") == 0)
    {
        DEPRECATION_WARNING("axesTicks2D");
        GET_VALUE2(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetTickAxes);
    }
    else if(strcmp(name, "xAxisUserTitle2D") == 0)
    {
        DEPRECATION_WARNING("xAxisUserTitle2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetTitle);
    }
    else if(strcmp(name, "yAxisUserTitle2D") == 0)
    {
        DEPRECATION_WARNING("yAxisUserTitle2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetTitle);
    }
    else if(strcmp(name, "xAxisUserTitleFlag2D") == 0)
    {
        DEPRECATION_WARNING("xAxisUserTitleFlag2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetUserTitle);
    }
    else if(strcmp(name, "yAxisUserTitleFlag2D") == 0)
    {
        DEPRECATION_WARNING("yAxisUserTitleFlag2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetUserTitle);
    }
    else if(strcmp(name, "xAxisUserUnits2D") == 0)
    {
        DEPRECATION_WARNING("xAxisUserUnits2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetUnits);
    }
    else if(strcmp(name, "yAxisUserUnits2D") == 0)
    {
        DEPRECATION_WARNING("yAxisUserUnits2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetUnits);
    }
    else if(strcmp(name, "xAxisUserUnitsFlag2D") == 0)
    {
        DEPRECATION_WARNING("xAxisUserUnitsFlag2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetUserUnits);
    }
    else if(strcmp(name, "yAxisUserUnitsFlag2D") == 0)
    {
        DEPRECATION_WARNING("yAxisUserUnitsFlag2D");
        GET_VALUE4(AnnotationAttributes_GetAxes2D,
                   Axes2D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetUserUnits);
    }
    else if(strcmp(name, "axesFlag") == 0)
    {
        DEPRECATION_WARNING("axesFlag");
        GET_VALUE2(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetVisible);
    }
    else if(strcmp(name, "axesAutoSetTicks") == 0)
    {
        DEPRECATION_WARNING("axesAutoSetTicks");
        GET_VALUE2(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetAutoSetTicks);
    }
    else if(strcmp(name, "labelAutoSetScaling") == 0)
    {
        DEPRECATION_WARNING("labelAutoSetScaling");
        GET_VALUE2(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetAutoSetScaling);
    }
    else if(strcmp(name, "xAxisLabels") == 0)
    {
        DEPRECATION_WARNING("xAxisLabels");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetVisible);
    }
    else if(strcmp(name, "yAxisLabels") == 0)
    {
        DEPRECATION_WARNING("yAxisLabels");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetVisible);
    }
    else if(strcmp(name, "zAxisLabels") == 0)
    {
        DEPRECATION_WARNING("zAxisLabels");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetVisible);
    }
    else if(strcmp(name, "xAxisTitle") == 0)
    {
        DEPRECATION_WARNING("xAxisTitle");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetVisible);
    }
    else if(strcmp(name, "yAxisTitle") == 0)
    {
        DEPRECATION_WARNING("yAxisTitle");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetVisible);
    }
    else if(strcmp(name, "zAxisTitle") == 0)
    {
        DEPRECATION_WARNING("zAxisTitle");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetVisible);
    }
    else if(strcmp(name, "xGridLines") == 0)
    {
        DEPRECATION_WARNING("xGridLines");
        GET_VALUE3(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetGrid);
    }
    else if(strcmp(name, "yGridLines") == 0)
    {
        DEPRECATION_WARNING("yGridLines");
        GET_VALUE3(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetGrid);
    }
    else if(strcmp(name, "zGridLines") == 0)
    {
        DEPRECATION_WARNING("zGridLines");
        GET_VALUE3(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetGrid);
    }
    else if(strcmp(name, "xAxisTicks") == 0)
    {
        DEPRECATION_WARNING("xAxisTicks");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetVisible);
    }
    else if(strcmp(name, "yAxisTicks") == 0)
    {
        DEPRECATION_WARNING("yAxisTicks");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetVisible);
    }
    else if(strcmp(name, "zAxisTicks") == 0)
    {
        DEPRECATION_WARNING("zAxisTicks");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetVisible);
    }
    else if(strcmp(name, "xMajorTickMinimum") == 0)
    {
        DEPRECATION_WARNING("xMajorTickMinimum");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMajorMinimum);
    }
    else if(strcmp(name, "yMajorTickMinimum") == 0)
    {
        DEPRECATION_WARNING("yMajorTickMinimum");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMajorMinimum);
    }
    else if(strcmp(name, "zMajorTickMinimum") == 0)
    {
        DEPRECATION_WARNING("zMajorTickMinimum");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMajorMinimum);
    }
    else if(strcmp(name, "xMajorTickMaximum") == 0)
    {
        DEPRECATION_WARNING("xMajorTickMaximum");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMajorMaximum);
    }
    else if(strcmp(name, "yMajorTickMaximum") == 0)
    {
        DEPRECATION_WARNING("yMajorTickMaximum");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMajorMaximum);
    }
    else if(strcmp(name, "zMajorTickMaximum") == 0)
    {
        DEPRECATION_WARNING("zMajorTickMaximum");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMajorMaximum);
    }
    else if(strcmp(name, "xMajorTickSpacing") == 0)
    {
        DEPRECATION_WARNING("xMajorTickSpacing");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMajorSpacing);
    }
    else if(strcmp(name, "yMajorTickSpacing") == 0)
    {
        DEPRECATION_WARNING("yMajorTickSpacing");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMajorSpacing);
    }
    else if(strcmp(name, "zMajorTickSpacing") == 0)
    {
        DEPRECATION_WARNING("zMajorTickSpacing");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMajorSpacing);
    }
    else if(strcmp(name, "xMinorTickSpacing") == 0)
    {
        DEPRECATION_WARNING("xMinorTickSpacing");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMinorSpacing);
    }
    else if(strcmp(name, "yMinorTickSpacing") == 0)
    {
        DEPRECATION_WARNING("yMinorTickSpacing");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMinorSpacing);
    }
    else if(strcmp(name, "zMinorTickSpacing") == 0)
    {
        DEPRECATION_WARNING("zMinorTickSpacing");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTickMarks,
                   AxisTickMarks_GetMinorSpacing);
    }
    else if(strcmp(name, "xLabelFontHeight") == 0)
    {
        DEPRECATION_WARNING("xLabelFontHeight");
        GET_VALUE5(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetFont,
                   FontAttributes_GetHeight);
    }
    else if(strcmp(name, "yLabelFontHeight") == 0)
    {
        DEPRECATION_WARNING("yLabelFontHeight");
        GET_VALUE5(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetFont,
                   FontAttributes_GetHeight);
    }
    else if(strcmp(name, "zLabelFontHeight") == 0)
    {
        DEPRECATION_WARNING("zLabelFontHeight");
        GET_VALUE5(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetFont,
                   FontAttributes_GetHeight);
    }
    else if(strcmp(name, "xTitleFontHeight") == 0)
    {
        DEPRECATION_WARNING("xTitleFontHeight");
        GET_VALUE5(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetFont,
                   FontAttributes_GetHeight);
    }
    else if(strcmp(name, "yTitleFontHeight") == 0)
    {
        DEPRECATION_WARNING("yTitleFontHeight");
        GET_VALUE5(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetFont,
                   FontAttributes_GetHeight);
    }
    else if(strcmp(name, "zTitleFontHeight") == 0)
    {
        DEPRECATION_WARNING("zTitleFontHeight");
        GET_VALUE5(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetFont,
                   FontAttributes_GetHeight);
    }
    else if(strcmp(name, "xLabelScaling") == 0)
    {
        DEPRECATION_WARNING("xLabelScaling");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetScaling);
    }
    else if(strcmp(name, "yLabelScaling") == 0)
    {
        DEPRECATION_WARNING("yLabelScaling");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetScaling);
    }
    else if(strcmp(name, "zLabelScaling") == 0)
    {
        DEPRECATION_WARNING("zLabelScaling");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetLabel,
                   AxisLabels_GetScaling);
    }
    else if(strcmp(name, "xAxisUserTitle") == 0)
    {
        DEPRECATION_WARNING("xAxisUserTitle");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetTitle);
    }
    else if(strcmp(name, "yAxisUserTitle") == 0)
    {
        DEPRECATION_WARNING("yAxisUserTitle");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetTitle);
    }
    else if(strcmp(name, "zAxisUserTitle") == 0)
    {
        DEPRECATION_WARNING("zAxisUserTitle");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetTitle);
    }
    else if(strcmp(name, "xAxisUserTitleFlag") == 0)
    {
        DEPRECATION_WARNING("xAxisUserTitleFlag");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetUserTitle);
    }
    else if(strcmp(name, "yAxisUserTitleFlag") == 0)
    {
        DEPRECATION_WARNING("yAxisUserTitleFlag");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetUserTitle);
    }
    else if(strcmp(name, "zAxisUserTitleFlag") == 0)
    {
        DEPRECATION_WARNING("zAxisUserTitleFlag");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetUserTitle);
    }
    else if(strcmp(name, "xAxisUserUnits") == 0)
    {
        DEPRECATION_WARNING("xAxisUserUnits");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetUnits);
    }
    else if(strcmp(name, "yAxisUserUnits") == 0)
    {
        DEPRECATION_WARNING("yAxisUserUnits");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetUnits);
    }
    else if(strcmp(name, "zAxisUserUnits") == 0)
    {
        DEPRECATION_WARNING("zAxisUserUnits");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetUnits);
    }
    else if(strcmp(name, "xAxisUserUnitsFlag") == 0)
    {
        DEPRECATION_WARNING("xAxisUserUnitsFlag");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetXAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetUserUnits);
    }
    else if(strcmp(name, "yAxisUserUnitsFlag") == 0)
    {
        DEPRECATION_WARNING("yAxisUserUnitsFlag");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetYAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetUserUnits);
    }
    else if(strcmp(name, "zAxisUserUnitsFlag") == 0)
    {
        DEPRECATION_WARNING("zAxisUserUnitsFlag");
        GET_VALUE4(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetZAxis,
                   AxisAttributes_GetTitle,
                   AxisTitles_GetUserUnits);
    }
    else if(strcmp(name, "axesTickLocation") == 0)
    {
        DEPRECATION_WARNING("axesTickLocation");
        GET_VALUE2(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetTickLocation);
    }
    else if(strcmp(name, "axesType") == 0)
    {
        DEPRECATION_WARNING("axesType");
        GET_VALUE2(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetAxesType);
    }
    else if(strcmp(name, "triadFlag") == 0)
    {
        DEPRECATION_WARNING("triadFlag");
        GET_VALUE2(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetTriadFlag);
    }
    else if(strcmp(name, "bboxFlag") == 0)
    {
        DEPRECATION_WARNING("bboxFlag");
        GET_VALUE2(AnnotationAttributes_GetAxes3D,
                   Axes3D_GetBboxFlag);
    }

    // Handle enums
    if(strcmp(name, "Off") == 0)
        return PyInt_FromLong(long(Axes2D::Off));
    else if(strcmp(name, "Bottom") == 0)
        return PyInt_FromLong(long(Axes2D::Bottom));
    else if(strcmp(name, "Left") == 0)
        return PyInt_FromLong(long(Axes2D::Left));
    else if(strcmp(name, "BottomLeft") == 0)
        return PyInt_FromLong(long(Axes2D::BottomLeft));
    else if(strcmp(name, "All") == 0)
        return PyInt_FromLong(long(Axes2D::All));

    if(strcmp(name, "Inside") == 0)
        return PyInt_FromLong(long(Axes2D::Inside));
    else if(strcmp(name, "Outside") == 0)
        return PyInt_FromLong(long(Axes2D::Outside));
    else if(strcmp(name, "Both") == 0)
        return PyInt_FromLong(long(Axes2D::Both));

    if(strcmp(name, "ClosestTriad") == 0)
        return PyInt_FromLong(long(Axes3D::ClosestTriad));
    else if(strcmp(name, "FurthestTriad") == 0)
        return PyInt_FromLong(long(Axes3D::FurthestTriad));
    else if(strcmp(name, "OutsideEdges") == 0)
        return PyInt_FromLong(long(Axes3D::OutsideEdges));
    else if(strcmp(name, "StaticTriad") == 0)
        return PyInt_FromLong(long(Axes3D::StaticTriad));
    else if(strcmp(name, "StaticEdges") == 0)
        return PyInt_FromLong(long(Axes3D::StaticEdges));

    if(strcmp(name, "TopToBottom") == 0)
        return PyInt_FromLong(long(AnnotationAttributes::TopToBottom));
    else if(strcmp(name, "BottomToTop") == 0)
        return PyInt_FromLong(long(AnnotationAttributes::BottomToTop));
    else if(strcmp(name, "LeftToRight") == 0)
        return PyInt_FromLong(long(AnnotationAttributes::LeftToRight));
    else if(strcmp(name, "RightToLeft") == 0)
        return PyInt_FromLong(long(AnnotationAttributes::RightToLeft));
    else if(strcmp(name, "Radial") == 0)
        return PyInt_FromLong(long(AnnotationAttributes::Radial));

    if(strcmp(name, "Solid") == 0)
        return PyInt_FromLong(long(AnnotationAttributes::Solid));
    else if(strcmp(name, "Gradient") == 0)
        return PyInt_FromLong(long(AnnotationAttributes::Gradient));
    else if(strcmp(name, "Image") == 0)
        return PyInt_FromLong(long(AnnotationAttributes::Image));
    else if(strcmp(name, "ImageSphere") == 0)
        return PyInt_FromLong(long(AnnotationAttributes::ImageSphere));

    if(strcmp(name, "File") == 0)
        return PyInt_FromLong(long(AnnotationAttributes::File));
    else if(strcmp(name, "Directory") == 0)
        return PyInt_FromLong(long(AnnotationAttributes::Directory));
    else if(strcmp(name, "Full") == 0)
        return PyInt_FromLong(long(AnnotationAttributes::Full));
    else if(strcmp(name, "Smart") == 0)
        return PyInt_FromLong(long(AnnotationAttributes::Smart));
    else if(strcmp(name, "SmartDirectory") == 0)
        return PyInt_FromLong(long(AnnotationAttributes::SmartDirectory));

    return NULL;
}
