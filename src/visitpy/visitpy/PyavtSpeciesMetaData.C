// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <PyavtSpeciesMetaData.h>
#include <ObserverToCallback.h>
#include <stdio.h>
#include <Py2and3Support.h>
#include <PyavtMatSpeciesMetaData.h>

// ****************************************************************************
// Module: PyavtSpeciesMetaData
//
// Purpose:
//   Contains species metadata attributes
//
// Note:       Autogenerated by xml2python. Do not modify by hand!
//
// Programmer: xml2python
// Creation:   omitted
//
// ****************************************************************************

//
// This struct contains the Python type information and a avtSpeciesMetaData.
//
struct avtSpeciesMetaDataObject
{
    PyObject_HEAD
    avtSpeciesMetaData *data;
    bool        owns;
    PyObject   *parent;
};

//
// Internal prototypes
//
static PyObject *NewavtSpeciesMetaData(int);
std::string
PyavtSpeciesMetaData_ToString(const avtSpeciesMetaData *atts, const char *prefix, const bool forLogging)
{
    std::string str;
    char tmpStr[1000];

    snprintf(tmpStr, 1000, "%sname = \"%s\"\n", prefix, atts->name.c_str());
    str += tmpStr;
    snprintf(tmpStr, 1000, "%soriginalName = \"%s\"\n", prefix, atts->originalName.c_str());
    str += tmpStr;
    if(atts->validVariable)
        snprintf(tmpStr, 1000, "%svalidVariable = 1\n", prefix);
    else
        snprintf(tmpStr, 1000, "%svalidVariable = 0\n", prefix);
    str += tmpStr;
    snprintf(tmpStr, 1000, "%smeshName = \"%s\"\n", prefix, atts->meshName.c_str());
    str += tmpStr;
    snprintf(tmpStr, 1000, "%smaterialName = \"%s\"\n", prefix, atts->materialName.c_str());
    str += tmpStr;
    snprintf(tmpStr, 1000, "%snumMaterials = %d\n", prefix, atts->numMaterials);
    str += tmpStr;
    { // new scope
        int index = 0;
        // Create string representation of species from atts.
        for(AttributeGroupVector::const_iterator pos = atts->GetSpecies().begin(); pos != atts->GetSpecies().end(); ++pos, ++index)
        {
            const avtMatSpeciesMetaData *current = (const avtMatSpeciesMetaData *)(*pos);
            snprintf(tmpStr, 1000, "GetSpecies(%d).", index);
            std::string objPrefix(prefix + std::string(tmpStr));
            str += PyavtMatSpeciesMetaData_ToString(current, objPrefix.c_str(), forLogging);
        }
        if(index == 0)
            str += "#species does not contain any avtMatSpeciesMetaData objects.\n";
    }
    return str;
}

static PyObject *
avtSpeciesMetaData_Notify(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;
    obj->data->Notify();
    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
avtSpeciesMetaData_SetName(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;

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

    // Set the name in the object.
    obj->data->name = cval;

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
avtSpeciesMetaData_GetName(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;
    PyObject *retval = PyString_FromString(obj->data->name.c_str());
    return retval;
}

/*static*/ PyObject *
avtSpeciesMetaData_SetOriginalName(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;

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

    // Set the originalName in the object.
    obj->data->originalName = cval;

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
avtSpeciesMetaData_GetOriginalName(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;
    PyObject *retval = PyString_FromString(obj->data->originalName.c_str());
    return retval;
}

/*static*/ PyObject *
avtSpeciesMetaData_SetValidVariable(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;

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

    // Set the validVariable in the object.
    obj->data->validVariable = cval;

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
avtSpeciesMetaData_GetValidVariable(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;
    PyObject *retval = PyInt_FromLong(obj->data->validVariable?1L:0L);
    return retval;
}

/*static*/ PyObject *
avtSpeciesMetaData_SetMeshName(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;

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

    // Set the meshName in the object.
    obj->data->meshName = cval;

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
avtSpeciesMetaData_GetMeshName(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;
    PyObject *retval = PyString_FromString(obj->data->meshName.c_str());
    return retval;
}

/*static*/ PyObject *
avtSpeciesMetaData_SetMaterialName(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;

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

    // Set the materialName in the object.
    obj->data->materialName = cval;

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
avtSpeciesMetaData_GetMaterialName(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;
    PyObject *retval = PyString_FromString(obj->data->materialName.c_str());
    return retval;
}

/*static*/ PyObject *
avtSpeciesMetaData_SetNumMaterials(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;

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
    int cval = int(val);

    if (val == -1 && PyErr_Occurred())
    {
        Py_XDECREF(packaged_args);
        PyErr_Clear();
        return PyErr_Format(PyExc_TypeError, "arg not interpretable as C++ int");
    }
    if (fabs(double(val))>1.5E-7 && fabs((double(long(cval))-double(val))/double(val))>1.5E-7)
    {
        Py_XDECREF(packaged_args);
        return PyErr_Format(PyExc_ValueError, "arg not interpretable as C++ int");
    }

    Py_XDECREF(packaged_args);

    // Set the numMaterials in the object.
    obj->data->numMaterials = cval;

    Py_INCREF(Py_None);
    return Py_None;
}

/*static*/ PyObject *
avtSpeciesMetaData_GetNumMaterials(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;
    PyObject *retval = PyInt_FromLong(long(obj->data->numMaterials));
    return retval;
}

/*static*/ PyObject *
avtSpeciesMetaData_GetSpecies(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;
    int index = -1;
    if (args == NULL)
        return PyErr_Format(PyExc_NameError, "Use .GetSpecies(int index) to get a single entry");
    if (!PyArg_ParseTuple(args, "i", &index))
        return PyErr_Format(PyExc_TypeError, "arg must be a single integer index");
    if (index < 0 || (size_t)index >= obj->data->GetSpecies().size())
        return PyErr_Format(PyExc_ValueError, "index out of range");

    // Since the new object will point to data owned by the this object,
    // we need to increment the reference count.
    Py_INCREF(self);

    PyObject *retval = PyavtMatSpeciesMetaData_Wrap(&obj->data->GetSpecies(index));
    // Set the object's parent so the reference to the parent can be decref'd
    // when the child goes out of scope.
    PyavtMatSpeciesMetaData_SetParent(retval, self);

    return retval;
}

PyObject *
avtSpeciesMetaData_GetNumSpecies(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;
    return PyInt_FromLong((long)obj->data->GetSpecies().size());
}

PyObject *
avtSpeciesMetaData_AddSpecies(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;
    PyObject *element = NULL;
    if(!PyArg_ParseTuple(args, "O", &element))
        return NULL;
    if(!PyavtMatSpeciesMetaData_Check(element))
        return PyErr_Format(PyExc_TypeError, "expected attr object of type avtMatSpeciesMetaData");
    avtMatSpeciesMetaData *newData = PyavtMatSpeciesMetaData_FromPyObject(element);
    obj->data->AddSpecies(*newData);
    obj->data->SelectSpecies();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
avtSpeciesMetaData_Remove_One_Species(PyObject *self, int index)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;
    // Remove in the AttributeGroupVector instead of calling RemoveSpecies() because we don't want to delete the object; just remove it.
    AttributeGroupVector &atts = obj->data->GetSpecies();
    AttributeGroupVector::iterator pos = atts.begin();
    // Iterate through the vector "index" times.
    for(int i = 0; i < index; ++i)
        ++pos;

    // If pos is still a valid iterator, remove that element.
    if(pos != atts.end())
    {
        // NOTE: Leak the object since other Python objects may reference it. Ideally,
        // we would put the object into some type of pool to be cleaned up later but
        // this will do for now.
        //
        // delete *pos;
        atts.erase(pos);
    }

    obj->data->SelectSpecies();
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
avtSpeciesMetaData_RemoveSpecies(PyObject *self, PyObject *args)
{
    int index = -1;
    if(!PyArg_ParseTuple(args, "i", &index))
        return PyErr_Format(PyExc_TypeError, "Expecting integer index");
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;
    if(index < 0 || index >= obj->data->GetNumSpecies())
        return PyErr_Format(PyExc_IndexError, "Index out of range");

    return avtSpeciesMetaData_Remove_One_Species(self, index);
}

PyObject *
avtSpeciesMetaData_ClearSpecies(PyObject *self, PyObject *args)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)self;
    int n = obj->data->GetNumSpecies();
    for(int i = 0; i < n; ++i)
    {
        avtSpeciesMetaData_Remove_One_Species(self, 0);
        Py_DECREF(Py_None);
    }
    Py_INCREF(Py_None);
    return Py_None;
}



PyMethodDef PyavtSpeciesMetaData_methods[AVTSPECIESMETADATA_NMETH] = {
    {"Notify", avtSpeciesMetaData_Notify, METH_VARARGS},
    {"SetName", avtSpeciesMetaData_SetName, METH_VARARGS},
    {"GetName", avtSpeciesMetaData_GetName, METH_VARARGS},
    {"SetOriginalName", avtSpeciesMetaData_SetOriginalName, METH_VARARGS},
    {"GetOriginalName", avtSpeciesMetaData_GetOriginalName, METH_VARARGS},
    {"SetValidVariable", avtSpeciesMetaData_SetValidVariable, METH_VARARGS},
    {"GetValidVariable", avtSpeciesMetaData_GetValidVariable, METH_VARARGS},
    {"SetMeshName", avtSpeciesMetaData_SetMeshName, METH_VARARGS},
    {"GetMeshName", avtSpeciesMetaData_GetMeshName, METH_VARARGS},
    {"SetMaterialName", avtSpeciesMetaData_SetMaterialName, METH_VARARGS},
    {"GetMaterialName", avtSpeciesMetaData_GetMaterialName, METH_VARARGS},
    {"SetNumMaterials", avtSpeciesMetaData_SetNumMaterials, METH_VARARGS},
    {"GetNumMaterials", avtSpeciesMetaData_GetNumMaterials, METH_VARARGS},
    {"GetSpecies", avtSpeciesMetaData_GetSpecies, METH_VARARGS},
    {"GetNumSpecies", avtSpeciesMetaData_GetNumSpecies, METH_VARARGS},
    {"AddSpecies", avtSpeciesMetaData_AddSpecies, METH_VARARGS},
    {"RemoveSpecies", avtSpeciesMetaData_RemoveSpecies, METH_VARARGS},
    {"ClearSpecies", avtSpeciesMetaData_ClearSpecies, METH_VARARGS},
    {NULL, NULL}
};

//
// Type functions
//

static void
avtSpeciesMetaData_dealloc(PyObject *v)
{
   avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)v;
   if(obj->parent != 0)
       Py_DECREF(obj->parent);
   if(obj->owns)
       delete obj->data;
}

static PyObject *avtSpeciesMetaData_richcompare(PyObject *self, PyObject *other, int op);
PyObject *
PyavtSpeciesMetaData_getattr(PyObject *self, char *name)
{
    if(strcmp(name, "name") == 0)
        return avtSpeciesMetaData_GetName(self, NULL);
    if(strcmp(name, "originalName") == 0)
        return avtSpeciesMetaData_GetOriginalName(self, NULL);
    if(strcmp(name, "validVariable") == 0)
        return avtSpeciesMetaData_GetValidVariable(self, NULL);
    if(strcmp(name, "meshName") == 0)
        return avtSpeciesMetaData_GetMeshName(self, NULL);
    if(strcmp(name, "materialName") == 0)
        return avtSpeciesMetaData_GetMaterialName(self, NULL);
    if(strcmp(name, "numMaterials") == 0)
        return avtSpeciesMetaData_GetNumMaterials(self, NULL);
    if(strcmp(name, "species") == 0)
        return avtSpeciesMetaData_GetSpecies(self, NULL);


    // Add a __dict__ answer so that dir() works
    if (!strcmp(name, "__dict__"))
    {
        PyObject *result = PyDict_New();
        for (int i = 0; PyavtSpeciesMetaData_methods[i].ml_meth; i++)
            PyDict_SetItem(result,
                PyString_FromString(PyavtSpeciesMetaData_methods[i].ml_name),
                PyString_FromString(PyavtSpeciesMetaData_methods[i].ml_name));
        return result;
    }

    return Py_FindMethod(PyavtSpeciesMetaData_methods, self, name);
}

int
PyavtSpeciesMetaData_setattr(PyObject *self, char *name, PyObject *args)
{
    PyObject NULL_PY_OBJ;
    PyObject *obj = &NULL_PY_OBJ;

    if(strcmp(name, "name") == 0)
        obj = avtSpeciesMetaData_SetName(self, args);
    else if(strcmp(name, "originalName") == 0)
        obj = avtSpeciesMetaData_SetOriginalName(self, args);
    else if(strcmp(name, "validVariable") == 0)
        obj = avtSpeciesMetaData_SetValidVariable(self, args);
    else if(strcmp(name, "meshName") == 0)
        obj = avtSpeciesMetaData_SetMeshName(self, args);
    else if(strcmp(name, "materialName") == 0)
        obj = avtSpeciesMetaData_SetMaterialName(self, args);
    else if(strcmp(name, "numMaterials") == 0)
        obj = avtSpeciesMetaData_SetNumMaterials(self, args);

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
avtSpeciesMetaData_print(PyObject *v, FILE *fp, int flags)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)v;
    fprintf(fp, "%s", PyavtSpeciesMetaData_ToString(obj->data, "",false).c_str());
    return 0;
}

PyObject *
avtSpeciesMetaData_str(PyObject *v)
{
    avtSpeciesMetaDataObject *obj = (avtSpeciesMetaDataObject *)v;
    return PyString_FromString(PyavtSpeciesMetaData_ToString(obj->data,"", false).c_str());
}

//
// The doc string for the class.
//
#if PY_MAJOR_VERSION > 2 || (PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 5)
static const char *avtSpeciesMetaData_Purpose = "Contains species metadata attributes";
#else
static char *avtSpeciesMetaData_Purpose = "Contains species metadata attributes";
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

VISIT_PY_TYPE_OBJ(avtSpeciesMetaDataType,         \
                  "avtSpeciesMetaData",           \
                  avtSpeciesMetaDataObject,       \
                  avtSpeciesMetaData_dealloc,     \
                  avtSpeciesMetaData_print,       \
                  PyavtSpeciesMetaData_getattr,   \
                  PyavtSpeciesMetaData_setattr,   \
                  avtSpeciesMetaData_str,         \
                  avtSpeciesMetaData_Purpose,     \
                  avtSpeciesMetaData_richcompare, \
                  0); /* as_number*/

//
// Helper function for comparing.
//
static PyObject *
avtSpeciesMetaData_richcompare(PyObject *self, PyObject *other, int op)
{
    // only compare against the same type 
    if ( Py_TYPE(self) != &avtSpeciesMetaDataType
         || Py_TYPE(other) != &avtSpeciesMetaDataType)
    {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }

    PyObject *res = NULL;
    avtSpeciesMetaData *a = ((avtSpeciesMetaDataObject *)self)->data;
    avtSpeciesMetaData *b = ((avtSpeciesMetaDataObject *)other)->data;

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

static avtSpeciesMetaData *defaultAtts = 0;
static avtSpeciesMetaData *currentAtts = 0;

static PyObject *
NewavtSpeciesMetaData(int useCurrent)
{
    avtSpeciesMetaDataObject *newObject;
    newObject = PyObject_NEW(avtSpeciesMetaDataObject, &avtSpeciesMetaDataType);
    if(newObject == NULL)
        return NULL;
    if(useCurrent && currentAtts != 0)
        newObject->data = new avtSpeciesMetaData(*currentAtts);
    else if(defaultAtts != 0)
        newObject->data = new avtSpeciesMetaData(*defaultAtts);
    else
        newObject->data = new avtSpeciesMetaData;
    newObject->owns = true;
    newObject->parent = 0;
    return (PyObject *)newObject;
}

static PyObject *
WrapavtSpeciesMetaData(const avtSpeciesMetaData *attr)
{
    avtSpeciesMetaDataObject *newObject;
    newObject = PyObject_NEW(avtSpeciesMetaDataObject, &avtSpeciesMetaDataType);
    if(newObject == NULL)
        return NULL;
    newObject->data = (avtSpeciesMetaData *)attr;
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
avtSpeciesMetaData_new(PyObject *self, PyObject *args)
{
    int useCurrent = 0;
    if (!PyArg_ParseTuple(args, "i", &useCurrent))
    {
        if (!PyArg_ParseTuple(args, ""))
            return NULL;
        else
            PyErr_Clear();
    }

    return (PyObject *)NewavtSpeciesMetaData(useCurrent);
}

//
// Plugin method table. These methods are added to the visitmodule's methods.
//
static PyMethodDef avtSpeciesMetaDataMethods[] = {
    {"avtSpeciesMetaData", avtSpeciesMetaData_new, METH_VARARGS},
    {NULL,      NULL}        /* Sentinel */
};

static Observer *avtSpeciesMetaDataObserver = 0;

std::string
PyavtSpeciesMetaData_GetLogString()
{
    std::string s("avtSpeciesMetaData = avtSpeciesMetaData()\n");
    if(currentAtts != 0)
        s += PyavtSpeciesMetaData_ToString(currentAtts, "avtSpeciesMetaData.", true);
    return s;
}

static void
PyavtSpeciesMetaData_CallLogRoutine(Subject *subj, void *data)
{
    typedef void (*logCallback)(const std::string &);
    logCallback cb = (logCallback)data;

    if(cb != 0)
    {
        std::string s("avtSpeciesMetaData = avtSpeciesMetaData()\n");
        s += PyavtSpeciesMetaData_ToString(currentAtts, "avtSpeciesMetaData.", true);
        cb(s);
    }
}

void
PyavtSpeciesMetaData_StartUp(avtSpeciesMetaData *subj, void *data)
{
    if(subj == 0)
        return;

    currentAtts = subj;
    PyavtSpeciesMetaData_SetDefaults(subj);

    //
    // Create the observer that will be notified when the attributes change.
    //
    if(avtSpeciesMetaDataObserver == 0)
    {
        avtSpeciesMetaDataObserver = new ObserverToCallback(subj,
            PyavtSpeciesMetaData_CallLogRoutine, (void *)data);
    }

}

void
PyavtSpeciesMetaData_CloseDown()
{
    delete defaultAtts;
    defaultAtts = 0;
    delete avtSpeciesMetaDataObserver;
    avtSpeciesMetaDataObserver = 0;
}

PyMethodDef *
PyavtSpeciesMetaData_GetMethodTable(int *nMethods)
{
    *nMethods = 1;
    return avtSpeciesMetaDataMethods;
}

bool
PyavtSpeciesMetaData_Check(PyObject *obj)
{
    return (obj->ob_type == &avtSpeciesMetaDataType);
}

avtSpeciesMetaData *
PyavtSpeciesMetaData_FromPyObject(PyObject *obj)
{
    avtSpeciesMetaDataObject *obj2 = (avtSpeciesMetaDataObject *)obj;
    return obj2->data;
}

PyObject *
PyavtSpeciesMetaData_New()
{
    return NewavtSpeciesMetaData(0);
}

PyObject *
PyavtSpeciesMetaData_Wrap(const avtSpeciesMetaData *attr)
{
    return WrapavtSpeciesMetaData(attr);
}

void
PyavtSpeciesMetaData_SetParent(PyObject *obj, PyObject *parent)
{
    avtSpeciesMetaDataObject *obj2 = (avtSpeciesMetaDataObject *)obj;
    obj2->parent = parent;
}

void
PyavtSpeciesMetaData_SetDefaults(const avtSpeciesMetaData *atts)
{
    if(defaultAtts)
        delete defaultAtts;

    defaultAtts = new avtSpeciesMetaData(*atts);
}

