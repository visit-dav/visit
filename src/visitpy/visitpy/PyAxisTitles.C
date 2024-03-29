// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <PyAxisTitles.h>
#include <ObserverToCallback.h>
#include <stdio.h>
#include <Py2and3Support.h>
#include <PyFontAttributes.h>

// ****************************************************************************
// Module: PyAxisTitles
//
// Purpose:
//   Contains the title properties for one axis.
//
// Note:       Autogenerated by xml2python. Do not modify by hand!
//
// Programmer: xml2python
// Creation:   omitted
//
// ****************************************************************************

//
// This struct contains the Python type information and a AxisTitles.
//
struct AxisTitlesObject
{
    PyObject_HEAD
    AxisTitles *data;
    bool        owns;
    PyObject   *parent;
};

//
// Internal prototypes
//
static PyObject *NewAxisTitles(int);
std::string
PyAxisTitles_ToString(const AxisTitles *atts, const char *prefix, const bool forLogging)
{
    std::string str;
    char tmpStr[1000];

    if(atts->GetVisible())
        snprintf(tmpStr, 1000, "%svisible = 1\n", prefix);
    else
        snprintf(tmpStr, 1000, "%svisible = 0\n", prefix);
    str += tmpStr;
    { // new scope
        std::string objPrefix(prefix);
        objPrefix += "font.";
        str += PyFontAttributes_ToString(&atts->GetFont(), objPrefix.c_str(), forLogging);
    }
    if(atts->GetUserTitle())
        snprintf(tmpStr, 1000, "%suserTitle = 1\n", prefix);
    else
        snprintf(tmpStr, 1000, "%suserTitle = 0\n", prefix);
    str += tmpStr;
    if(atts->GetUserUnits())
        snprintf(tmpStr, 1000, "%suserUnits = 1\n", prefix);
    else
        snprintf(tmpStr, 1000, "%suserUnits = 0\n", prefix);
    str += tmpStr;
    snprintf(tmpStr, 1000, "%stitle = \"%s\"\n", prefix, atts->GetTitle().c_str());
    str += tmpStr;
    snprintf(tmpStr, 1000, "%sunits = \"%s\"\n", prefix, atts->GetUnits().c_str());
    str += tmpStr;
    return str;
}

static PyObject *
AxisTitles_Notify(PyObject *self, PyObject *args)
{
    AxisTitlesObject *obj = (AxisTitlesObject *)self;
    obj->data->Notify();
    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
AxisTitles_SetVisible(PyObject *self, PyObject *args)
{
    AxisTitlesObject *obj = (AxisTitlesObject *)self;

    PyObject *packaged_args = 0;

    // Handle args packaged into a tuple of size one
    // if we think the unpackaged args matches our needs
    if (PySequence_Check(args) && PySequence_Size(args) == 1)
    {
        packaged_args = PySequence_GetItem(args, 0);
        if (PyNumber_Check(packaged_args))
            args = packaged_args;
    }

    if (PySequence_Check(args))
    {
        Py_XDECREF(packaged_args);
        return PyErr_Format(PyExc_TypeError, "expecting a single number arg");
    }

    if (!PyNumber_Check(args))
    {
        Py_XDECREF(packaged_args);
        return PyErr_Format(PyExc_TypeError, "arg is not a number type");
    }

    long val = PyLong_AsLong(args);
    bool cval = bool(val);

    if (val == -1 && PyErr_Occurred())
    {
        Py_XDECREF(packaged_args);
        PyErr_Clear();
        return PyErr_Format(PyExc_TypeError, "arg not interpretable as C++ bool");
    }
    if (fabs(double(val))>1.5E-7 && fabs((double(long(cval))-double(val))/double(val))>1.5E-7)
    {
        Py_XDECREF(packaged_args);
        return PyErr_Format(PyExc_ValueError, "arg not interpretable as C++ bool");
    }

    Py_XDECREF(packaged_args);

    // Set the visible in the object.
    obj->data->SetVisible(cval);

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
AxisTitles_GetVisible(PyObject *self, PyObject *args)
{
    AxisTitlesObject *obj = (AxisTitlesObject *)self;
    PyObject *retval = PyInt_FromLong(obj->data->GetVisible()?1L:0L);
    return retval;
}

/*static*/ PyObject *
AxisTitles_SetFont(PyObject *self, PyObject *args)
{
    AxisTitlesObject *obj = (AxisTitlesObject *)self;

    PyObject *newValue = NULL;
    if(!PyArg_ParseTuple(args, "O", &newValue))
        return NULL;
    if(!PyFontAttributes_Check(newValue))
        return PyErr_Format(PyExc_TypeError, "Field font can be set only with FontAttributes objects");

    obj->data->SetFont(*PyFontAttributes_FromPyObject(newValue));

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
AxisTitles_GetFont(PyObject *self, PyObject *args)
{
    AxisTitlesObject *obj = (AxisTitlesObject *)self;
    // Since the new object will point to data owned by this object,
    // we need to increment the reference count.
    Py_INCREF(self);

    PyObject *retval = PyFontAttributes_Wrap(&obj->data->GetFont());
    // Set the object's parent so the reference to the parent can be decref'd
    // when the child goes out of scope.
    PyFontAttributes_SetParent(retval, self);

    return retval;
}

/*static*/ PyObject *
AxisTitles_SetUserTitle(PyObject *self, PyObject *args)
{
    AxisTitlesObject *obj = (AxisTitlesObject *)self;

    PyObject *packaged_args = 0;

    // Handle args packaged into a tuple of size one
    // if we think the unpackaged args matches our needs
    if (PySequence_Check(args) && PySequence_Size(args) == 1)
    {
        packaged_args = PySequence_GetItem(args, 0);
        if (PyNumber_Check(packaged_args))
            args = packaged_args;
    }

    if (PySequence_Check(args))
    {
        Py_XDECREF(packaged_args);
        return PyErr_Format(PyExc_TypeError, "expecting a single number arg");
    }

    if (!PyNumber_Check(args))
    {
        Py_XDECREF(packaged_args);
        return PyErr_Format(PyExc_TypeError, "arg is not a number type");
    }

    long val = PyLong_AsLong(args);
    bool cval = bool(val);

    if (val == -1 && PyErr_Occurred())
    {
        Py_XDECREF(packaged_args);
        PyErr_Clear();
        return PyErr_Format(PyExc_TypeError, "arg not interpretable as C++ bool");
    }
    if (fabs(double(val))>1.5E-7 && fabs((double(long(cval))-double(val))/double(val))>1.5E-7)
    {
        Py_XDECREF(packaged_args);
        return PyErr_Format(PyExc_ValueError, "arg not interpretable as C++ bool");
    }

    Py_XDECREF(packaged_args);

    // Set the userTitle in the object.
    obj->data->SetUserTitle(cval);

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
AxisTitles_GetUserTitle(PyObject *self, PyObject *args)
{
    AxisTitlesObject *obj = (AxisTitlesObject *)self;
    PyObject *retval = PyInt_FromLong(obj->data->GetUserTitle()?1L:0L);
    return retval;
}

/*static*/ PyObject *
AxisTitles_SetUserUnits(PyObject *self, PyObject *args)
{
    AxisTitlesObject *obj = (AxisTitlesObject *)self;

    PyObject *packaged_args = 0;

    // Handle args packaged into a tuple of size one
    // if we think the unpackaged args matches our needs
    if (PySequence_Check(args) && PySequence_Size(args) == 1)
    {
        packaged_args = PySequence_GetItem(args, 0);
        if (PyNumber_Check(packaged_args))
            args = packaged_args;
    }

    if (PySequence_Check(args))
    {
        Py_XDECREF(packaged_args);
        return PyErr_Format(PyExc_TypeError, "expecting a single number arg");
    }

    if (!PyNumber_Check(args))
    {
        Py_XDECREF(packaged_args);
        return PyErr_Format(PyExc_TypeError, "arg is not a number type");
    }

    long val = PyLong_AsLong(args);
    bool cval = bool(val);

    if (val == -1 && PyErr_Occurred())
    {
        Py_XDECREF(packaged_args);
        PyErr_Clear();
        return PyErr_Format(PyExc_TypeError, "arg not interpretable as C++ bool");
    }
    if (fabs(double(val))>1.5E-7 && fabs((double(long(cval))-double(val))/double(val))>1.5E-7)
    {
        Py_XDECREF(packaged_args);
        return PyErr_Format(PyExc_ValueError, "arg not interpretable as C++ bool");
    }

    Py_XDECREF(packaged_args);

    // Set the userUnits in the object.
    obj->data->SetUserUnits(cval);

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
AxisTitles_GetUserUnits(PyObject *self, PyObject *args)
{
    AxisTitlesObject *obj = (AxisTitlesObject *)self;
    PyObject *retval = PyInt_FromLong(obj->data->GetUserUnits()?1L:0L);
    return retval;
}

/*static*/ PyObject *
AxisTitles_SetTitle(PyObject *self, PyObject *args)
{
    AxisTitlesObject *obj = (AxisTitlesObject *)self;

    PyObject *packaged_args = 0;

    // Handle args packaged as first member of a tuple of size one
    // if we think the unpackaged args matches our needs
    if (PySequence_Check(args) && PySequence_Size(args) == 1)
    {
        packaged_args = PySequence_GetItem(args, 0);
        if (PyUnicode_Check(packaged_args))
            args = packaged_args;
    }

    if (!PyUnicode_Check(args))
    {
        Py_XDECREF(packaged_args);
        return PyErr_Format(PyExc_TypeError, "arg is not a unicode string");
    }

    char const *val = PyUnicode_AsUTF8(args);
    std::string cval = std::string(val);

    if (val == 0 && PyErr_Occurred())
    {
        Py_XDECREF(packaged_args);
        PyErr_Clear();
        return PyErr_Format(PyExc_TypeError, "arg not interpretable as utf8 string");
    }

    Py_XDECREF(packaged_args);

    // Set the title in the object.
    obj->data->SetTitle(cval);

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
AxisTitles_GetTitle(PyObject *self, PyObject *args)
{
    AxisTitlesObject *obj = (AxisTitlesObject *)self;
    PyObject *retval = PyString_FromString(obj->data->GetTitle().c_str());
    return retval;
}

/*static*/ PyObject *
AxisTitles_SetUnits(PyObject *self, PyObject *args)
{
    AxisTitlesObject *obj = (AxisTitlesObject *)self;

    PyObject *packaged_args = 0;

    // Handle args packaged as first member of a tuple of size one
    // if we think the unpackaged args matches our needs
    if (PySequence_Check(args) && PySequence_Size(args) == 1)
    {
        packaged_args = PySequence_GetItem(args, 0);
        if (PyUnicode_Check(packaged_args))
            args = packaged_args;
    }

    if (!PyUnicode_Check(args))
    {
        Py_XDECREF(packaged_args);
        return PyErr_Format(PyExc_TypeError, "arg is not a unicode string");
    }

    char const *val = PyUnicode_AsUTF8(args);
    std::string cval = std::string(val);

    if (val == 0 && PyErr_Occurred())
    {
        Py_XDECREF(packaged_args);
        PyErr_Clear();
        return PyErr_Format(PyExc_TypeError, "arg not interpretable as utf8 string");
    }

    Py_XDECREF(packaged_args);

    // Set the units in the object.
    obj->data->SetUnits(cval);

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
AxisTitles_GetUnits(PyObject *self, PyObject *args)
{
    AxisTitlesObject *obj = (AxisTitlesObject *)self;
    PyObject *retval = PyString_FromString(obj->data->GetUnits().c_str());
    return retval;
}



PyMethodDef PyAxisTitles_methods[AXISTITLES_NMETH] = {
    {"Notify", AxisTitles_Notify, METH_VARARGS},
    {"SetVisible", AxisTitles_SetVisible, METH_VARARGS},
    {"GetVisible", AxisTitles_GetVisible, METH_VARARGS},
    {"SetFont", AxisTitles_SetFont, METH_VARARGS},
    {"GetFont", AxisTitles_GetFont, METH_VARARGS},
    {"SetUserTitle", AxisTitles_SetUserTitle, METH_VARARGS},
    {"GetUserTitle", AxisTitles_GetUserTitle, METH_VARARGS},
    {"SetUserUnits", AxisTitles_SetUserUnits, METH_VARARGS},
    {"GetUserUnits", AxisTitles_GetUserUnits, METH_VARARGS},
    {"SetTitle", AxisTitles_SetTitle, METH_VARARGS},
    {"GetTitle", AxisTitles_GetTitle, METH_VARARGS},
    {"SetUnits", AxisTitles_SetUnits, METH_VARARGS},
    {"GetUnits", AxisTitles_GetUnits, METH_VARARGS},
    {NULL, NULL}
};

//
// Type functions
//

static void
AxisTitles_dealloc(PyObject *v)
{
   AxisTitlesObject *obj = (AxisTitlesObject *)v;
   if(obj->parent != 0)
       Py_DECREF(obj->parent);
   if(obj->owns)
       delete obj->data;
}

static PyObject *AxisTitles_richcompare(PyObject *self, PyObject *other, int op);
PyObject *
PyAxisTitles_getattr(PyObject *self, char *name)
{
    if(strcmp(name, "visible") == 0)
        return AxisTitles_GetVisible(self, NULL);
    if(strcmp(name, "font") == 0)
        return AxisTitles_GetFont(self, NULL);
    if(strcmp(name, "userTitle") == 0)
        return AxisTitles_GetUserTitle(self, NULL);
    if(strcmp(name, "userUnits") == 0)
        return AxisTitles_GetUserUnits(self, NULL);
    if(strcmp(name, "title") == 0)
        return AxisTitles_GetTitle(self, NULL);
    if(strcmp(name, "units") == 0)
        return AxisTitles_GetUnits(self, NULL);


    // Add a __dict__ answer so that dir() works
    if (!strcmp(name, "__dict__"))
    {
        PyObject *result = PyDict_New();
        for (int i = 0; PyAxisTitles_methods[i].ml_meth; i++)
            PyDict_SetItem(result,
                PyString_FromString(PyAxisTitles_methods[i].ml_name),
                PyString_FromString(PyAxisTitles_methods[i].ml_name));
        return result;
    }

    return Py_FindMethod(PyAxisTitles_methods, self, name);
}

int
PyAxisTitles_setattr(PyObject *self, char *name, PyObject *args)
{
    PyObject NULL_PY_OBJ;
    PyObject *obj = &NULL_PY_OBJ;

    if(strcmp(name, "visible") == 0)
        obj = AxisTitles_SetVisible(self, args);
    else if(strcmp(name, "font") == 0)
        obj = AxisTitles_SetFont(self, args);
    else if(strcmp(name, "userTitle") == 0)
        obj = AxisTitles_SetUserTitle(self, args);
    else if(strcmp(name, "userUnits") == 0)
        obj = AxisTitles_SetUserUnits(self, args);
    else if(strcmp(name, "title") == 0)
        obj = AxisTitles_SetTitle(self, args);
    else if(strcmp(name, "units") == 0)
        obj = AxisTitles_SetUnits(self, args);

    if (obj != NULL && obj != &NULL_PY_OBJ)
        Py_DECREF(obj);

    if (obj == &NULL_PY_OBJ)
    {
        obj = NULL;
        PyErr_Format(PyExc_NameError, "name '%s' is not defined", name);
    }
    else if (obj == NULL && !PyErr_Occurred())
        PyErr_Format(PyExc_RuntimeError, "unknown problem with '%s'", name);

    return (obj != NULL) ? 0 : -1;
}

static int
AxisTitles_print(PyObject *v, FILE *fp, int flags)
{
    AxisTitlesObject *obj = (AxisTitlesObject *)v;
    fprintf(fp, "%s", PyAxisTitles_ToString(obj->data, "",false).c_str());
    return 0;
}

PyObject *
AxisTitles_str(PyObject *v)
{
    AxisTitlesObject *obj = (AxisTitlesObject *)v;
    return PyString_FromString(PyAxisTitles_ToString(obj->data,"", false).c_str());
}

//
// The doc string for the class.
//
#if PY_MAJOR_VERSION > 2 || (PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 5)
static const char *AxisTitles_Purpose = "Contains the title properties for one axis.";
#else
static char *AxisTitles_Purpose = "Contains the title properties for one axis.";
#endif

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

VISIT_PY_TYPE_OBJ(AxisTitlesType,         \
                  "AxisTitles",           \
                  AxisTitlesObject,       \
                  AxisTitles_dealloc,     \
                  AxisTitles_print,       \
                  PyAxisTitles_getattr,   \
                  PyAxisTitles_setattr,   \
                  AxisTitles_str,         \
                  AxisTitles_Purpose,     \
                  AxisTitles_richcompare, \
                  0); /* as_number*/

//
// Helper function for comparing.
//
static PyObject *
AxisTitles_richcompare(PyObject *self, PyObject *other, int op)
{
    // only compare against the same type 
    if ( Py_TYPE(self) != &AxisTitlesType
         || Py_TYPE(other) != &AxisTitlesType)
    {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }

    PyObject *res = NULL;
    AxisTitles *a = ((AxisTitlesObject *)self)->data;
    AxisTitles *b = ((AxisTitlesObject *)other)->data;

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

static AxisTitles *defaultAtts = 0;
static AxisTitles *currentAtts = 0;

static PyObject *
NewAxisTitles(int useCurrent)
{
    AxisTitlesObject *newObject;
    newObject = PyObject_NEW(AxisTitlesObject, &AxisTitlesType);
    if(newObject == NULL)
        return NULL;
    if(useCurrent && currentAtts != 0)
        newObject->data = new AxisTitles(*currentAtts);
    else if(defaultAtts != 0)
        newObject->data = new AxisTitles(*defaultAtts);
    else
        newObject->data = new AxisTitles;
    newObject->owns = true;
    newObject->parent = 0;
    return (PyObject *)newObject;
}

static PyObject *
WrapAxisTitles(const AxisTitles *attr)
{
    AxisTitlesObject *newObject;
    newObject = PyObject_NEW(AxisTitlesObject, &AxisTitlesType);
    if(newObject == NULL)
        return NULL;
    newObject->data = (AxisTitles *)attr;
    newObject->owns = false;
    newObject->parent = 0;
    return (PyObject *)newObject;
}

///////////////////////////////////////////////////////////////////////////////
//
// Interface that is exposed to the VisIt module.
//
///////////////////////////////////////////////////////////////////////////////

PyObject *
AxisTitles_new(PyObject *self, PyObject *args)
{
    int useCurrent = 0;
    if (!PyArg_ParseTuple(args, "i", &useCurrent))
    {
        if (!PyArg_ParseTuple(args, ""))
            return NULL;
        else
            PyErr_Clear();
    }

    return (PyObject *)NewAxisTitles(useCurrent);
}

//
// Plugin method table. These methods are added to the visitmodule's methods.
//
static PyMethodDef AxisTitlesMethods[] = {
    {"AxisTitles", AxisTitles_new, METH_VARARGS},
    {NULL,      NULL}        /* Sentinel */
};

static Observer *AxisTitlesObserver = 0;

std::string
PyAxisTitles_GetLogString()
{
    std::string s("AxisTitles = AxisTitles()\n");
    if(currentAtts != 0)
        s += PyAxisTitles_ToString(currentAtts, "AxisTitles.", true);
    return s;
}

static void
PyAxisTitles_CallLogRoutine(Subject *subj, void *data)
{
    typedef void (*logCallback)(const std::string &);
    logCallback cb = (logCallback)data;

    if(cb != 0)
    {
        std::string s("AxisTitles = AxisTitles()\n");
        s += PyAxisTitles_ToString(currentAtts, "AxisTitles.", true);
        cb(s);
    }
}

void
PyAxisTitles_StartUp(AxisTitles *subj, void *data)
{
    if(subj == 0)
        return;

    currentAtts = subj;
    PyAxisTitles_SetDefaults(subj);

    //
    // Create the observer that will be notified when the attributes change.
    //
    if(AxisTitlesObserver == 0)
    {
        AxisTitlesObserver = new ObserverToCallback(subj,
            PyAxisTitles_CallLogRoutine, (void *)data);
    }

}

void
PyAxisTitles_CloseDown()
{
    delete defaultAtts;
    defaultAtts = 0;
    delete AxisTitlesObserver;
    AxisTitlesObserver = 0;
}

PyMethodDef *
PyAxisTitles_GetMethodTable(int *nMethods)
{
    *nMethods = 1;
    return AxisTitlesMethods;
}

bool
PyAxisTitles_Check(PyObject *obj)
{
    return (obj->ob_type == &AxisTitlesType);
}

AxisTitles *
PyAxisTitles_FromPyObject(PyObject *obj)
{
    AxisTitlesObject *obj2 = (AxisTitlesObject *)obj;
    return obj2->data;
}

PyObject *
PyAxisTitles_New()
{
    return NewAxisTitles(0);
}

PyObject *
PyAxisTitles_Wrap(const AxisTitles *attr)
{
    return WrapAxisTitles(attr);
}

void
PyAxisTitles_SetParent(PyObject *obj, PyObject *parent)
{
    AxisTitlesObject *obj2 = (AxisTitlesObject *)obj;
    obj2->parent = parent;
}

void
PyAxisTitles_SetDefaults(const AxisTitles *atts)
{
    if(defaultAtts)
        delete defaultAtts;

    defaultAtts = new AxisTitles(*atts);
}

