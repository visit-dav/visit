// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <PyTimeSliderObject.h>
#include <ObserverToCallback.h>
#include <ColorAttribute.h>
#include <AnnotationObject.h>

// CUSTOM:
#include <Py2and3Support.h>

// Functions that we need in visitmodule.C
extern void UpdateAnnotationHelper(AnnotationObject *);
extern bool DeleteAnnotationObjectHelper(AnnotationObject *);

// ****************************************************************************
// Module: PyTimeSliderObject
//
// Purpose:
//   This class is a Python wrapper arround AnnotationObject that makes it
//   look like a Time slider object.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 3 14:34:25 PST 2003
//
// ****************************************************************************

//
// This struct contains the Python type information and a TimeSliderObject.
//
struct TimeSliderObjectObject
{
    PyObject_HEAD
    AnnotationObject *data;
    bool owns;
};

//
// Internal prototypes
//
static PyObject *NewTimeSliderObject();

static PyObject *
TimeSliderObject_SetVisible(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    int ival;
    if(!PyArg_ParseTuple(args, "i", &ival))
        return NULL;

    // Set the visible in the object.
    obj->data->SetVisible(ival != 0);
/* CUSTOM */
    UpdateAnnotationHelper(obj->data);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
TimeSliderObject_GetVisible(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;
    PyObject *retval = PyInt_FromLong(obj->data->GetVisible()?1L:0L);
    return retval;
}

static PyObject *
TimeSliderObject_SetActive(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    int ival;
    if(!PyArg_ParseTuple(args, "i", &ival))
        return NULL;

    // Set the active in the object.
    obj->data->SetActive(ival != 0);
/* CUSTOM */
    UpdateAnnotationHelper(obj->data);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
TimeSliderObject_GetActive(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;
    PyObject *retval = PyInt_FromLong(obj->data->GetActive()?1L:0L);
    return retval;
}

static PyObject *
TimeSliderObject_SetPosition(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    double *dvals = obj->data->GetPosition();
    if(!PyArg_ParseTuple(args, "dd", &dvals[0], &dvals[1]))
    {
        PyObject     *tuple;
        if(!PyArg_ParseTuple(args, "O", &tuple))
            return NULL;

        if(PyTuple_Check(tuple))
        {
            if(PyTuple_Size(tuple) != 2)
                return NULL;

            PyErr_Clear();
            for(int i = 0; i < PyTuple_Size(tuple); ++i)
            {
                PyObject *item = PyTuple_GET_ITEM(tuple, i);
                if(PyFloat_Check(item))
                    dvals[i] = (PyFloat_AS_DOUBLE(item));
                else if(PyInt_Check(item))
                    dvals[i] = double(PyInt_AS_LONG(item));
                else if(PyLong_Check(item))
                    dvals[i] = (PyLong_AsDouble(item));
                else
                    dvals[i] = 0.;
            }
        }
        else
            return NULL;
    }

    // Mark the position in the object as modified.
    obj->data->SelectPosition();
/* CUSTOM */
    UpdateAnnotationHelper(obj->data);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
TimeSliderObject_GetPosition(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;
    // Allocate a tuple the with enough entries to hold the position.
    PyObject *retval = PyTuple_New(2);
    const double *position = obj->data->GetPosition();
    for(int i = 0; i < 2; ++i)
        PyTuple_SET_ITEM(retval, i, PyFloat_FromDouble(double(position[i])));
    return retval;
}

static PyObject *
TimeSliderObject_SetWidth(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    double dval;
    if(!PyArg_ParseTuple(args, "d", &dval))
        return NULL;

    // Set the width in the object.
/*CUSTOM*/
    double *pos2 = obj->data->GetPosition2();
    pos2[0] = dval;
    obj->data->SelectPosition2();
    UpdateAnnotationHelper(obj->data);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
TimeSliderObject_GetWidth(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;
/*CUSTOM*/
    double *pos2 = obj->data->GetPosition2();
    PyObject *retval = PyFloat_FromDouble(double(pos2[0]));
    return retval;
}

static PyObject *
TimeSliderObject_SetHeight(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    double dval;
    if(!PyArg_ParseTuple(args, "d", &dval))
        return NULL;

    // Set the height in the object.
/*CUSTOM*/
    double *pos2 = obj->data->GetPosition2();
    pos2[1] = dval;
    obj->data->SelectPosition2();
    UpdateAnnotationHelper(obj->data);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
TimeSliderObject_GetHeight(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;
/*CUSTOM*/
    double *pos2 = obj->data->GetPosition2();
    PyObject *retval = PyFloat_FromDouble(double(pos2[1]));
    return retval;
}

static PyObject *
TimeSliderObject_SetTextColor(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    int c[4];
    if(!PyArg_ParseTuple(args, "iiii", &c[0], &c[1], &c[2], &c[3]))
    {
        c[3] = 255;
        if(!PyArg_ParseTuple(args, "iii", &c[0], &c[1], &c[2]))
        {
            double dr, dg, db, da;
            if(PyArg_ParseTuple(args, "dddd", &dr, &dg, &db, &da))
            {
                c[0] = int(dr);
                c[1] = int(dg);
                c[2] = int(db);
                c[3] = int(da);
            }
            else if(PyArg_ParseTuple(args, "ddd", &dr, &dg, &db))
            {
                c[0] = int(dr);
                c[1] = int(dg);
                c[2] = int(db);
                c[3] = 255;
            }
            else
            {
                PyObject *tuple = NULL;
                if(!PyArg_ParseTuple(args, "O", &tuple))
                    return NULL;

                if(!PyTuple_Check(tuple))
                    return NULL;

                // Make sure that the tuple is the right size.
                if(PyTuple_Size(tuple) < 3 || PyTuple_Size(tuple) > 4)
                    return NULL;

                // Make sure that all elements in the tuple are ints.
                for(int i = 0; i < PyTuple_Size(tuple); ++i)
                {
                    PyObject *item = PyTuple_GET_ITEM(tuple, i);
                    if(PyInt_Check(item))
                        c[i] = int(PyInt_AS_LONG(PyTuple_GET_ITEM(tuple, i)));
                    else if(PyFloat_Check(item))
                        c[i] = int(PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(tuple, i)));
                    else
                        return NULL;
                }
            }
        }
        PyErr_Clear();
    }

    // Set the textColor in the object.
    ColorAttribute ca(c[0], c[1], c[2], c[3]);
    obj->data->SetTextColor(ca);
    UpdateAnnotationHelper(obj->data);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
TimeSliderObject_GetTextColor(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;
    // Allocate a tuple the with enough entries to hold the textColor.
    PyObject *retval = PyTuple_New(4);
    const unsigned char *textColor = obj->data->GetTextColor().GetColor();
    PyTuple_SET_ITEM(retval, 0, PyInt_FromLong(long(textColor[0])));
    PyTuple_SET_ITEM(retval, 1, PyInt_FromLong(long(textColor[1])));
    PyTuple_SET_ITEM(retval, 2, PyInt_FromLong(long(textColor[2])));
    PyTuple_SET_ITEM(retval, 3, PyInt_FromLong(long(textColor[3])));
    return retval;
}

static PyObject *
TimeSliderObject_SetUseForegroundForTextColor(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    int ival;
    if(!PyArg_ParseTuple(args, "i", &ival))
        return NULL;

    // Set the useForegroundForTextColor in the object.
    obj->data->SetUseForegroundForTextColor(ival != 0);
/* CUSTOM */
    UpdateAnnotationHelper(obj->data);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
TimeSliderObject_GetUseForegroundForTextColor(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;
    PyObject *retval = PyInt_FromLong(obj->data->GetUseForegroundForTextColor()?1L:0L);
    return retval;
}

static PyObject *
TimeSliderObject_SetStartColor(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    int c[4];
    if(!PyArg_ParseTuple(args, "iiii", &c[0], &c[1], &c[2], &c[3]))
    {
        c[3] = 255;
        if(!PyArg_ParseTuple(args, "iii", &c[0], &c[1], &c[2]))
        {
            double dr, dg, db, da;
            if(PyArg_ParseTuple(args, "dddd", &dr, &dg, &db, &da))
            {
                c[0] = int(dr);
                c[1] = int(dg);
                c[2] = int(db);
                c[3] = int(da);
            }
            else if(PyArg_ParseTuple(args, "ddd", &dr, &dg, &db))
            {
                c[0] = int(dr);
                c[1] = int(dg);
                c[2] = int(db);
                c[3] = 255;
            }
            else
            {
                PyObject *tuple = NULL;
                if(!PyArg_ParseTuple(args, "O", &tuple))
                    return NULL;

                if(!PyTuple_Check(tuple))
                    return NULL;

                // Make sure that the tuple is the right size.
                if(PyTuple_Size(tuple) < 3 || PyTuple_Size(tuple) > 4)
                    return NULL;

                // Make sure that all elements in the tuple are ints.
                for(int i = 0; i < PyTuple_Size(tuple); ++i)
                {
                    PyObject *item = PyTuple_GET_ITEM(tuple, i);
                    if(PyInt_Check(item))
                        c[i] = int(PyInt_AS_LONG(PyTuple_GET_ITEM(tuple, i)));
                    else if(PyFloat_Check(item))
                        c[i] = int(PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(tuple, i)));
                    else
                        return NULL;
                }
            }
        }
        PyErr_Clear();
    }

    // Set the startColor in the object.
    ColorAttribute ca(c[0], c[1], c[2], c[3]);
/*CUSTOM*/
    obj->data->SetColor1(ca);
    UpdateAnnotationHelper(obj->data);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
TimeSliderObject_GetStartColor(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;
    // Allocate a tuple the with enough entries to hold the startColor.
    PyObject *retval = PyTuple_New(4);
/*CUSTOM*/
    const unsigned char *startColor = obj->data->GetColor1().GetColor();
    PyTuple_SET_ITEM(retval, 0, PyInt_FromLong(long(startColor[0])));
    PyTuple_SET_ITEM(retval, 1, PyInt_FromLong(long(startColor[1])));
    PyTuple_SET_ITEM(retval, 2, PyInt_FromLong(long(startColor[2])));
    PyTuple_SET_ITEM(retval, 3, PyInt_FromLong(long(startColor[3])));
    return retval;
}

static PyObject *
TimeSliderObject_SetEndColor(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    int c[4];
    if(!PyArg_ParseTuple(args, "iiii", &c[0], &c[1], &c[2], &c[3]))
    {
        c[3] = 255;
        if(!PyArg_ParseTuple(args, "iii", &c[0], &c[1], &c[2]))
        {
            double dr, dg, db, da;
            if(PyArg_ParseTuple(args, "dddd", &dr, &dg, &db, &da))
            {
                c[0] = int(dr);
                c[1] = int(dg);
                c[2] = int(db);
                c[3] = int(da);
            }
            else if(PyArg_ParseTuple(args, "ddd", &dr, &dg, &db))
            {
                c[0] = int(dr);
                c[1] = int(dg);
                c[2] = int(db);
                c[3] = 255;
            }
            else
            {
                PyObject *tuple = NULL;
                if(!PyArg_ParseTuple(args, "O", &tuple))
                    return NULL;

                if(!PyTuple_Check(tuple))
                    return NULL;

                // Make sure that the tuple is the right size.
                if(PyTuple_Size(tuple) < 3 || PyTuple_Size(tuple) > 4)
                    return NULL;

                // Make sure that all elements in the tuple are ints.
                for(int i = 0; i < PyTuple_Size(tuple); ++i)
                {
                    PyObject *item = PyTuple_GET_ITEM(tuple, i);
                    if(PyInt_Check(item))
                        c[i] = int(PyInt_AS_LONG(PyTuple_GET_ITEM(tuple, i)));
                    else if(PyFloat_Check(item))
                        c[i] = int(PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(tuple, i)));
                    else
                        return NULL;
                }
            }
        }
        PyErr_Clear();
    }

    // Set the endColor in the object.
    ColorAttribute ca(c[0], c[1], c[2], c[3]);
/*CUSTOM*/
    obj->data->SetColor2(ca);
    UpdateAnnotationHelper(obj->data);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
TimeSliderObject_GetEndColor(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;
    // Allocate a tuple the with enough entries to hold the endColor.
    PyObject *retval = PyTuple_New(4);
/*CUSTOM*/
    const unsigned char *endColor = obj->data->GetColor2().GetColor();
    PyTuple_SET_ITEM(retval, 0, PyInt_FromLong(long(endColor[0])));
    PyTuple_SET_ITEM(retval, 1, PyInt_FromLong(long(endColor[1])));
    PyTuple_SET_ITEM(retval, 2, PyInt_FromLong(long(endColor[2])));
    PyTuple_SET_ITEM(retval, 3, PyInt_FromLong(long(endColor[3])));
    return retval;
}

static PyObject *
TimeSliderObject_SetText(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    char *str;
    if(!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    // Set the text in the object.
/*CUSTOM*/
    stringVector s(obj->data->GetText());
    if(s.size() > 1)
    {
        s[0] = str;
    }
    else
    {
        s.clear();
        s.push_back(str);
        s.push_back("%%g");
    }
    obj->data->SetText(s);
    UpdateAnnotationHelper(obj->data);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
TimeSliderObject_GetText(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;
/*CUSTOM*/
    const stringVector &s = obj->data->GetText();
    PyObject *retval = PyString_FromString(s.size() > 0 ? s[0].c_str(): "");
    return retval;
}

static PyObject *
TimeSliderObject_SetTimeFormatString(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    char *str;
    if(!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    // Set the text in the object.
/*CUSTOM*/
    stringVector s(obj->data->GetText());
    if(s.size() > 1)
    {
        s[1] = str;
    }
    else
    {
        s.clear();
        s.push_back("");
        s.push_back(str);
    }
    obj->data->SetText(s);
    UpdateAnnotationHelper(obj->data);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
TimeSliderObject_GetTimeFormatString(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;
/*CUSTOM*/
    const stringVector &s = obj->data->GetText();
    PyObject *retval = PyString_FromString(s.size() > 1 ? s[1].c_str(): "%%g");
    return retval;
}

static PyObject *
TimeSliderObject_SetTimeDisplay(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    int ival;
    if(!PyArg_ParseTuple(args, "i", &ival))
        return NULL;

    // Set the timeDisplay in the object.
    if(ival >= 0 && ival < 4)
    {
/*CUSTOM*/
        int val = (obj->data->GetIntAttribute1() & (~12)) | ((ival & 3) << 2);
        obj->data->SetIntAttribute1(val);
        UpdateAnnotationHelper(obj->data);
    }
    else
    {
        fprintf(stderr, "An invalid timeDisplay value was given. "
                        "Valid values are in the range of [0,2]. "
                        "You can also use the following names: "
                        "AllFrames, FramesForPlot, StatesForPlot, "
                        "UserSpecified.");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
TimeSliderObject_GetTimeDisplay(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;
/*CUSTOM*/
    int timeDisplay = ((obj->data->GetIntAttribute1() >> 2) & 3);
    PyObject *retval = PyInt_FromLong(long(timeDisplay));
    return retval;
}

static PyObject *
TimeSliderObject_SetPercentComplete(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    double dval;
    if(!PyArg_ParseTuple(args, "d", &dval))
        return NULL;

    // Set the timeDisplay in the object.
    if(dval >= 0. && dval <= 100.)
    {
/*CUSTOM*/
        double percent = dval * 0.01;
        obj->data->SetDoubleAttribute1(percent);
        UpdateAnnotationHelper(obj->data);
    }
    else
    {
        fprintf(stderr, "An invalid percentComplete value was given. "
                        "Valid values are in the range of [0,100].");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
TimeSliderObject_GetPercentComplete(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;
/*CUSTOM*/
    double percent = obj->data->GetDoubleAttribute1() * 100.;
    PyObject *retval = PyInt_FromLong(long(percent));
    return retval;
}

static PyObject *
TimeSliderObject_SetRounded(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    int ival;
    if(!PyArg_ParseTuple(args, "i", &ival))
        return NULL;

    // Set the rounded in the object.
/*CUSTOM*/
    int v = (obj->data->GetIntAttribute1() & (~1)) | (ival?1:0);
    obj->data->SetIntAttribute1(v);
    UpdateAnnotationHelper(obj->data);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
TimeSliderObject_GetRounded(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;
/*CUSTOM*/
    int rounded = obj->data->GetIntAttribute1() & 1;
    PyObject *retval = PyInt_FromLong(rounded?1L:0L);
    return retval;
}

static PyObject *
TimeSliderObject_SetShaded(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    int ival;
    if(!PyArg_ParseTuple(args, "i", &ival))
        return NULL;

    // Set the shaded in the object.
/*CUSTOM*/
    int v = (obj->data->GetIntAttribute1() & (~2)) | ((ival?1:0) << 1);
    obj->data->SetIntAttribute1(v);
    UpdateAnnotationHelper(obj->data);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
TimeSliderObject_GetShaded(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;
/*CUSTOM*/
    int shaded = obj->data->GetIntAttribute1() & 2;
    PyObject *retval = PyInt_FromLong(shaded?1L:0L);
    return retval;
}


/*CUSTOM*/
static PyObject *
TimeSliderObject_Delete(PyObject *self, PyObject *args)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)self;

    // Tell the VisIt module to decrement the reference count for the
    // AnnotationObject pointed to by obj->data. If there are no more
    // references, this object will own it.
    obj->owns = DeleteAnnotationObjectHelper(obj->data);

    // Decrement this object's reference count.
    Py_DECREF(self);

    Py_INCREF(Py_None);
    return Py_None;
}

static struct PyMethodDef TimeSliderObject_methods[] = {
    {"SetVisible", TimeSliderObject_SetVisible, METH_VARARGS},
    {"GetVisible", TimeSliderObject_GetVisible, METH_VARARGS},
    {"SetActive", TimeSliderObject_SetActive, METH_VARARGS},
    {"GetActive", TimeSliderObject_GetActive, METH_VARARGS},
    {"SetPosition", TimeSliderObject_SetPosition, METH_VARARGS},
    {"GetPosition", TimeSliderObject_GetPosition, METH_VARARGS},
    {"SetWidth", TimeSliderObject_SetWidth, METH_VARARGS},
    {"GetWidth", TimeSliderObject_GetWidth, METH_VARARGS},
    {"SetHeight", TimeSliderObject_SetHeight, METH_VARARGS},
    {"GetHeight", TimeSliderObject_GetHeight, METH_VARARGS},
    {"SetTextColor", TimeSliderObject_SetTextColor, METH_VARARGS},
    {"GetTextColor", TimeSliderObject_GetTextColor, METH_VARARGS},
    {"SetUseForegroundForTextColor", TimeSliderObject_SetUseForegroundForTextColor, METH_VARARGS},
    {"GetUseForegroundForTextColor", TimeSliderObject_GetUseForegroundForTextColor, METH_VARARGS},
    {"SetStartColor", TimeSliderObject_SetStartColor, METH_VARARGS},
    {"GetStartColor", TimeSliderObject_GetStartColor, METH_VARARGS},
    {"SetEndColor", TimeSliderObject_SetEndColor, METH_VARARGS},
    {"GetEndColor", TimeSliderObject_GetEndColor, METH_VARARGS},
    {"SetText", TimeSliderObject_SetText, METH_VARARGS},
    {"GetText", TimeSliderObject_GetText, METH_VARARGS},
    {"SetTimeDisplay", TimeSliderObject_SetTimeDisplay, METH_VARARGS},
    {"GetTimeDisplay", TimeSliderObject_GetTimeDisplay, METH_VARARGS},
    {"SetTimeFormatString", TimeSliderObject_SetTimeFormatString, METH_VARARGS},
    {"GetTimeFormatString", TimeSliderObject_GetTimeFormatString, METH_VARARGS},
    {"SetPercentComplete", TimeSliderObject_SetPercentComplete, METH_VARARGS},
    {"GetPercentComplete", TimeSliderObject_GetPercentComplete, METH_VARARGS},
    {"SetRounded", TimeSliderObject_SetRounded, METH_VARARGS},
    {"GetRounded", TimeSliderObject_GetRounded, METH_VARARGS},
    {"SetShaded", TimeSliderObject_SetShaded, METH_VARARGS},
    {"GetShaded", TimeSliderObject_GetShaded, METH_VARARGS},
    {"Delete", TimeSliderObject_Delete, METH_VARARGS},
    {NULL, NULL}
};

//
// Type functions
//

static void
TimeSliderObject_dealloc(PyObject *v)
{
   TimeSliderObjectObject *obj = (TimeSliderObjectObject *)v;
   if(obj->owns)
       delete obj->data;
}

// static int
// TimeSliderObject_compare(PyObject *v, PyObject *w)
// {
//     AnnotationObject *a = ((TimeSliderObjectObject *)v)->data;
//     AnnotationObject *b = ((TimeSliderObjectObject *)w)->data;
//     return (*a == *b) ? 0 : -1;
// }

static PyObject *
TimeSliderObject_getattr(PyObject *self, char *name)
{
    if(strcmp(name, "visible") == 0)
        return TimeSliderObject_GetVisible(self, NULL);
    if(strcmp(name, "active") == 0)
        return TimeSliderObject_GetActive(self, NULL);
    if(strcmp(name, "position") == 0)
        return TimeSliderObject_GetPosition(self, NULL);
    if(strcmp(name, "width") == 0)
        return TimeSliderObject_GetWidth(self, NULL);
    if(strcmp(name, "height") == 0)
        return TimeSliderObject_GetHeight(self, NULL);
    if(strcmp(name, "textColor") == 0)
        return TimeSliderObject_GetTextColor(self, NULL);
    if(strcmp(name, "useForegroundForTextColor") == 0)
        return TimeSliderObject_GetUseForegroundForTextColor(self, NULL);
    if(strcmp(name, "startColor") == 0)
        return TimeSliderObject_GetStartColor(self, NULL);
    if(strcmp(name, "endColor") == 0)
        return TimeSliderObject_GetEndColor(self, NULL);
    if(strcmp(name, "text") == 0)
        return TimeSliderObject_GetText(self, NULL);
    if(strcmp(name, "timeDisplay") == 0)
        return TimeSliderObject_GetTimeDisplay(self, NULL);
    if(strcmp(name, "timeFormatString") == 0)
        return TimeSliderObject_GetTimeFormatString(self, NULL);
    if(strcmp(name, "AllFrames") == 0)
        return PyInt_FromLong(0);
    else if(strcmp(name, "FramesForPlot") == 0)
        return PyInt_FromLong(1);
    else if(strcmp(name, "StatesForPlot") == 0)
        return PyInt_FromLong(2);
    else if(strcmp(name, "UserSpecified") == 0)
        return PyInt_FromLong(3);
    if(strcmp(name, "percentComplete") == 0)
        return TimeSliderObject_GetPercentComplete(self, NULL);

    if(strcmp(name, "rounded") == 0)
        return TimeSliderObject_GetRounded(self, NULL);
    if(strcmp(name, "shaded") == 0)
        return TimeSliderObject_GetShaded(self, NULL);

    return Py_FindMethod(TimeSliderObject_methods, self, name);
}

static int
TimeSliderObject_setattr(PyObject *self, char *name, PyObject *args)
{
    // Create a tuple to contain the arguments since all of the Set
    // functions expect a tuple.
    PyObject *tuple = PyTuple_New(1);
    PyTuple_SET_ITEM(tuple, 0, args);
    Py_INCREF(args);
    bool retval = false;

    if(strcmp(name, "visible") == 0)
        retval = (TimeSliderObject_SetVisible(self, tuple) != NULL);
    else if(strcmp(name, "active") == 0)
        retval = (TimeSliderObject_SetActive(self, tuple) != NULL);
    else if(strcmp(name, "position") == 0)
        retval = (TimeSliderObject_SetPosition(self, tuple) != NULL);
    else if(strcmp(name, "width") == 0)
        retval = (TimeSliderObject_SetWidth(self, tuple) != NULL);
    else if(strcmp(name, "height") == 0)
        retval = (TimeSliderObject_SetHeight(self, tuple) != NULL);
    else if(strcmp(name, "textColor") == 0)
        retval = (TimeSliderObject_SetTextColor(self, tuple) != NULL);
    else if(strcmp(name, "useForegroundForTextColor") == 0)
        retval = (TimeSliderObject_SetUseForegroundForTextColor(self, tuple) != NULL);
    else if(strcmp(name, "startColor") == 0)
        retval = (TimeSliderObject_SetStartColor(self, tuple) != NULL);
    else if(strcmp(name, "endColor") == 0)
        retval = (TimeSliderObject_SetEndColor(self, tuple) != NULL);
    else if(strcmp(name, "text") == 0)
        retval = (TimeSliderObject_SetText(self, tuple) != NULL);
    else if(strcmp(name, "timeDisplay") == 0)
        retval = (TimeSliderObject_SetTimeDisplay(self, tuple) != NULL);
    else if(strcmp(name, "timeFormatString") == 0)
        retval = (TimeSliderObject_SetTimeFormatString(self, tuple) != NULL);
    else if(strcmp(name, "percentComplete") == 0)
        retval = (TimeSliderObject_SetPercentComplete(self, tuple) != NULL);
    else if(strcmp(name, "rounded") == 0)
        retval = (TimeSliderObject_SetRounded(self, tuple) != NULL);
    else if(strcmp(name, "shaded") == 0)
        retval = (TimeSliderObject_SetShaded(self, tuple) != NULL);

    Py_DECREF(tuple);
    return retval ? 0 : -1;
}

static std::string
PyTimeSliderObject_ToString(PyObject *v)
{
    TimeSliderObjectObject *obj = (TimeSliderObjectObject *)v;
    stringVector const &s = obj->data->GetText();
    size_t textLen = s.size() > 0 ? s[0].length() : 0;
    size_t memLen = textLen + 512;
    char *memStr = new char[memLen];
    size_t memIdx = 0;

    if(obj->data->GetVisible())
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "visible = 1\n");
    else
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "visible = 0\n");
    if(obj->data->GetActive())
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "active = 1\n");
    else
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "active = 0\n");
    {   const double *position = obj->data->GetPosition();
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "position = (");
        for(int i = 0; i < 2; ++i)
        {
            memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "%g", position[i]);
            if(i < 1)
                memIdx += snprintf(&memStr[memIdx], memLen-memIdx, ", ");
        }
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, ")\n");
    }
/*CUSTOM*/
    const double *position2 = obj->data->GetPosition2();
    memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "width = %g\n", position2[0]);
    memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "height = %g\n", position2[1]);
    const unsigned char *textColor = obj->data->GetTextColor().GetColor();
    memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "textColor = (%d, %d, %d, %d)\n", int(textColor[0]), int(textColor[1]), int(textColor[2]), int(textColor[3]));
    if(obj->data->GetUseForegroundForTextColor())
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "useForegroundForTextColor = 1\n");
    else
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "useForegroundForTextColor = 0\n");
/*CUSTOM*/
    const unsigned char *startColor = obj->data->GetColor1().GetColor();
    memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "startColor = (%d, %d, %d, %d)\n", int(startColor[0]), int(startColor[1]), int(startColor[2]), int(startColor[3]));
/*CUSTOM*/
    const unsigned char *endColor = obj->data->GetColor2().GetColor();
    memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "endColor = (%d, %d, %d, %d)\n", int(endColor[0]), int(endColor[1]), int(endColor[2]), int(endColor[3]));
/*CUSTOM*/
    memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "text = \"%s\"\n", s.size() > 0 ? s[0].c_str() : "");
    memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "timeFormatString = \"%s\"\n", s.size() > 1 ? s[1].c_str() : "%%g");

/*CUSTOM*/
    int timeDisplay = ((obj->data->GetIntAttribute1() >> 2) & 3);
    const char *timeDisplay_names = "AllFrames, FramesForPlot, StatesForPlot, UserSpecified";
    if(timeDisplay == 0)
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "timeDisplay = AllFrames  # %s\n", timeDisplay_names);
    else if(timeDisplay == 1)
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "timeDisplay = FramesForPlot  # %s\n", timeDisplay_names);
    else if(timeDisplay == 2)
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "timeDisplay = StatesForPlot  # %s\n", timeDisplay_names);
    else
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "timeDisplay = UserSpecified  # %s\n", timeDisplay_names);
    memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "percentComplete = %g\n", obj->data->GetDoubleAttribute1() * 100.);
/*CUSTOM*/
    int rounded = obj->data->GetIntAttribute1() & 1;
    if(rounded)
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "rounded = 1\n");
    else
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "rounded = 0\n");
/*CUSTOM*/
    int shaded = obj->data->GetIntAttribute1() & 2;
    if(shaded)
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "shaded = 1\n");
    else
        memIdx += snprintf(&memStr[memIdx], memLen-memIdx, "shaded = 0\n");

    memStr[memIdx] = '\0';
    std::string retval = memStr;
    delete [] memStr;
    return retval;
}

static int
TimeSliderObject_print(PyObject *v, FILE *fp, int flags)
{
    fprintf(fp, "%s", PyTimeSliderObject_ToString(v).c_str());
    return 0;
}

PyObject *
TimeSliderObject_str(PyObject *v)
{
    return PyString_FromString(PyTimeSliderObject_ToString(v).c_str());
}

//
// The doc string for the class.
//
#if PY_MAJOR_VERSION > 2 || (PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 5)
static const char *TimeSliderObject_Purpose = "This class defines a general set of attributes that are used to set the attributes for all annotation objects.";
#else
static char *TimeSliderObject_Purpose = "This class defines a general set of attributes that are used to set the attributes for all annotation objects.";
#endif

// CUSTOM
static PyObject *TimeSliderObject_richcompare(PyObject *self, PyObject *other, int op);

// CUSTOM

//
// Python Type Struct Def Macro from Py2and3Support.h
//
//         VISIT_PY_TYPE_OBJ( VPY_TYPE,
//                            VPY_NAME,
//                            VPY_OBJECT,
//                            VPY_DEALLOC,
//                            VPY_PRINT,
//                            VPY_GETATTR,
//                            VPY_SETATTR,
//                            VPY_STR,
//                            VPY_PURPOSE,
//                            VPY_RICHCOMP,
//                            VPY_AS_NUMBER)

//
// The type description structure
//
VISIT_PY_TYPE_OBJ( TimeSliderObjectType,              \
                   "TimeSliderObject",                \
                   TimeSliderObjectObject,            \
                   TimeSliderObject_dealloc,          \
                   TimeSliderObject_print,            \
                   TimeSliderObject_getattr,          \
                   TimeSliderObject_setattr,          \
                   TimeSliderObject_str,              \
                   TimeSliderObject_Purpose,          \
                   TimeSliderObject_richcompare,      \
                   0); /* as_number*/

// CUSTOM
static PyObject *
TimeSliderObject_richcompare(PyObject *self, PyObject *other, int op)
{
    // only compare against the same type
    if ( Py_TYPE(self) != Py_TYPE(other)
         || Py_TYPE(self) != &TimeSliderObjectType)
    {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }

    PyObject *res = NULL;
    AnnotationObject *a = ((TimeSliderObjectObject *)self)->data;
    AnnotationObject *b = ((TimeSliderObjectObject *)other)->data;

    switch (op)
    {
       case Py_EQ:
           res = (*a == *b) ? Py_True : Py_False;
           break;
       case Py_NE:
           res = (*a != *b) ? Py_True : Py_False;
           break;
       default:
           res = Py_NotImplemented;
           break;
    }

    Py_INCREF(res);
    return res;
}


//
// Helper functions for object allocation.
//

static PyObject *
NewTimeSliderObject()
{
    TimeSliderObjectObject *newObject;
    newObject = PyObject_NEW(TimeSliderObjectObject, &TimeSliderObjectType);
    if(newObject == NULL)
        return NULL;
    newObject->data = new AnnotationObject;
    newObject->data->SetObjectType(AnnotationObject::TimeSlider);
    newObject->owns = true;
    return (PyObject *)newObject;
}

static PyObject *
WrapTimeSliderObject(AnnotationObject *annot)
{
    TimeSliderObjectObject *newObject;
    newObject = PyObject_NEW(TimeSliderObjectObject, &TimeSliderObjectType);
    if(newObject == NULL)
        return NULL;
    newObject->data = annot;
    newObject->owns = false;
    return (PyObject *)newObject;
}

///////////////////////////////////////////////////////////////////////////////
//
// Interface that is exposed to the VisIt module.
//
///////////////////////////////////////////////////////////////////////////////

bool
PyTimeSliderObject_Check(PyObject *obj)
{
    return (obj->ob_type == &TimeSliderObjectType);
}

AnnotationObject *
PyTimeSliderObject_FromPyObject(PyObject *obj)
{
    TimeSliderObjectObject *obj2 = (TimeSliderObjectObject *)obj;
    return obj2->data;
}

PyObject *
PyTimeSliderObject_NewPyObject()
{
    return NewTimeSliderObject();
}

PyObject *
PyTimeSliderObject_WrapPyObject(AnnotationObject *attr)
{
    return WrapTimeSliderObject(attr);
}

