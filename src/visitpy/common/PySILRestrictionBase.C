// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <PySILRestrictionBase.h>
#include <VisItException.h>
#include <avtSILRestrictionTraverser.h>
#include <Py2and3Support.h>


// ****************************************************************************
//  File: PySILRestrictionBase.C
//
//  Modifications:
//  Cyrus Harrison, Wed Feb 10 10:13:30 PST 2010
//   NOTE: I needed wrap SILRestrictions without any viewer proxy
//         dependancies for contract access in Python Filters. To accomplish
//         this I moved most of the functionality from PySILRestriction.{h,C}
//         into PySILRestrictionBase.{h,C} and left the visitmodule specific
//         pieces in PySILRestriction.{h,C}.
//
// ****************************************************************************


// ****************************************************************************
//  Notes:   I wanted the data member to be an actual SIL restriction (as
//           opposed to a pointer to a SIL restriction).  The problem is that
//           Python allocates the memory as C-memory, meaning that no
//           constructors are called.  The ref_ptr then believes that it has
//           been initialized properly (with uninitialized memory) and when you
//           try to assign over the uninitialized memory, it tries to free up
//           its "old reference", which causes a core.
//
//  Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Changed pointer to SIL restriction to reference counted pointer to a SIL
//   restriction.  This is consistent with handling elsewhere in the code.
//
// ****************************************************************************
struct PySILRestrictionObject
{
    PyObject_HEAD
    avtSILRestriction_p *silr;
};

//
// SILRestriction methods.
//

// forward declare b/c we need to use in type def and need to use the type in this func
static PyObject *SILRestriction_richcompare(PyObject *self, PyObject *other, int op);

// ****************************************************************************
// Function: SILRestriction_Categories
//
// Purpose:
//   Returns a tuple that contains the set categories that come out of the
//   top level set.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 14:54:44 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Changed pointer to SIL restriction to reference counted pointer to a SIL
//   restriction.  This is consistent with handling elsewhere in the code.
//
// ****************************************************************************

static PyObject *
SILRestriction_Categories(PyObject *self, PyObject *args)
{
    PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
    avtSILRestriction_p silr = *(obj->silr);
    avtSILSet_p current = silr->GetSILSet(silr->GetTopSet());
    const std::vector<int> &mapsOut = current->GetMapsOut();

    // Allocate a tuple the with enough entries to hold the Collection name list.
    PyObject *retval = PyTuple_New(mapsOut.size());

    for(size_t i = 0; i < mapsOut.size(); ++i)
    {
        int cIndex = mapsOut[i];
        avtSILCollection_p collection = silr->GetSILCollection(cIndex);

        PyObject *dval = PyString_FromString(collection->GetCategory().c_str());
        if(dval == NULL)
            continue;
        PyTuple_SET_ITEM(retval, i, dval);
    }

    return retval;
}

// ****************************************************************************
// Function: SILRestriction_NumCategories
//
// Purpose:
//   Returns the number of categories coming from the top-level set.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 14:55:13 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Changed pointer to SIL restriction to reference counted pointer to a SIL
//   restriction.  This is consistent with handling elsewhere in the code.
//
// ****************************************************************************

static PyObject *
SILRestriction_NumCategories(PyObject *self, PyObject *args)
{
    PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
    avtSILRestriction_p silr = *(obj->silr);
    avtSILSet_p current = silr->GetSILSet(silr->GetTopSet());
    const std::vector<int> &mapsOut = current->GetMapsOut();
    PyObject *retval = PyLong_FromLong((long)mapsOut.size());
    return retval;
}

// ****************************************************************************
// Function: SILRestriction_NumSets
//
// Purpose:
//   Returns the number of sets in the SIL restriction.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 14:55:13 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Changed pointer to SIL restriction to reference counted pointer to a SIL
//   restriction.  This is consistent with handling elsewhere in the code.
//
// ****************************************************************************

static PyObject *
SILRestriction_NumSets(PyObject *self, PyObject *args)
{
    PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
    avtSILRestriction_p silr = *(obj->silr);
    PyObject *retval = PyLong_FromLong((long)silr->GetNumSets());
    return retval;
}

// ****************************************************************************
// Function: SILRestriction_SetIndex
//
// Purpose:
//   Returns the index of the set in the SIL restriction given the set's name.
//   If an error occurs, we throw a VisIt python exception.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 14:55:13 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Changed pointer to SIL restriction to reference counted pointer to a SIL
//   restriction.  This is consistent with handling elsewhere in the code.
//
// ****************************************************************************

static PyObject *
SILRestriction_SetIndex(PyObject *self, PyObject *args)
{
    PyObject *retval = NULL;
    char *setName;
    if(!PyArg_ParseTuple(args, "s", &setName))
        return PyErr_Format(PyExc_TypeError, "Expecting scalar string for name of set");

    TRY
    {
        PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
        avtSILRestriction_p silr = *(obj->silr);
        retval = PyLong_FromLong((long)silr->GetSetIndex(setName));
    }
    CATCH2(VisItException, e)
    {
        Py_XDECREF(retval);
        CATCH_RETURN2(1, PyErr_Format(PyExc_RuntimeError, "Internal VisIt exception \"%s\"", e.Message().c_str()));
    }
    ENDTRY

    return retval;
}

// ****************************************************************************
// Function: SILRestriction_SetName
//
// Purpose:
//   Returns the set name of the i'th set index.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 14:55:13 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Changed pointer to SIL restriction to reference counted pointer to a SIL
//   restriction.  This is consistent with handling elsewhere in the code.
//
// ****************************************************************************

static PyObject *
SILRestriction_SetName(PyObject *self, PyObject *args)
{
    PyObject *retval = NULL;
    int setNumber;
    int outOfRangeMax = -1;
    if(!PyArg_ParseTuple(args, "i", &setNumber))
        return PyErr_Format(PyExc_TypeError, "Expecting integer set index");

    TRY
    {
        PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
        avtSILRestriction_p silr = *(obj->silr);
        if (0 <= setNumber && setNumber < silr->GetNumSets())
            retval = PyString_FromString(silr->GetSILSet(setNumber)->GetName().c_str());
        else
            outOfRangeMax = silr->GetNumSets();
    }
    CATCH2(VisItException, e)
    {
        Py_XDECREF(retval);
        CATCH_RETURN2(1, PyErr_Format(PyExc_RuntimeError, "Internal VisIt exception \"%s\"", e.Message().c_str()));
    }
    ENDTRY

    if (outOfRangeMax != -1)
        return PyErr_Format(PyExc_ValueError, "Set integer index %d out of range [0,%d]", setNumber, outOfRangeMax);

    return retval;
}

// ****************************************************************************
// Function: SILRestriction_MapsOut
//
// Purpose:
//   Return a tuple of subsets for the specified set.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Wed May  2 13:12:05 PDT 2012
//
// Modifications:
//
// ****************************************************************************

static PyObject *
SILRestriction_MapsOut(PyObject *self, PyObject *args)
{
    int setNumber;
    int outOfRangeMax = -1;
    if(!PyArg_ParseTuple(args, "i", &setNumber))
        return PyErr_Format(PyExc_TypeError, "Expecing integer set index");

    PyObject *retval = NULL;
    TRY
    {
        PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
        avtSILRestriction_p silr = *(obj->silr);
        if (0 <= setNumber && setNumber < silr->GetNumSets())
        {
            const std::vector<int> &subsets = silr->GetSILSet(setNumber)->GetMapsOut();
            retval = PyTuple_New(int(subsets.size()));
            for(size_t i = 0; i < subsets.size(); ++i)
                PyTuple_SET_ITEM(retval, i, PyInt_FromLong(subsets[i]));
        }
        else
            outOfRangeMax = silr->GetNumSets();
    }
    CATCH2(VisItException, e)
    {
        Py_XDECREF(retval);
        CATCH_RETURN2(1, PyErr_Format(PyExc_RuntimeError, "Internal VisIt exception \"%s\"", e.Message().c_str()));
    }
    ENDTRY

    if (outOfRangeMax != -1)
        return PyErr_Format(PyExc_ValueError, "Set integer index %d out of range [0,%d]", setNumber, outOfRangeMax);

    return retval;
}

// ****************************************************************************
// Function: SILRestriction_SetsInCategory
//
// Purpose:
//   Returns a tuple of set indices that represent the sets that belong to
//   the named category.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 14:55:13 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Changed pointer to SIL restriction to reference counted pointer to a SIL
//   restriction.  This is consistent with handling elsewhere in the code.
//
//   Dave Bremer, Thu Jan 31 17:52:55 PST 2008
//   Fixed a bug in which a vector<int> &sets is requested from an
//   avtSILCollection, but the collection goes out of scope and its vector
//   of sets is deleted before this method is done using them.
//
//   Hank Childs, Tue Dec 15 13:40:40 PST 2009
//   Adapt to new SIL interface.
//
// ****************************************************************************

static PyObject *
SILRestriction_SetsInCategory(PyObject *self, PyObject *args)
{
    PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
    avtSILRestriction_p silr = *(obj->silr);

    // Get the Collection name.
    char *collectionName;
    if(!PyArg_ParseTuple(args, "s", &collectionName))
        return PyErr_Format(PyExc_TypeError, "Expecting string collection name");

    // Search through the Collection list for the specified Collection.
    int i, collectionIndex = -1;
    for(i = 0; i < silr->GetNumCollections(); ++i)
    {
        avtSILCollection_p collection = silr->GetSILCollection(i);
        if(collection->GetCategory() == std::string(collectionName))
        {
            collectionIndex = i;
            break;
        }
    }
    if(collectionIndex == -1)
    {
        return PyErr_Format(PyExc_ValueError, "Invalid collection name");
    }

    // Get the subset list.
    avtSILCollection_p col = silr->GetSILCollection(collectionIndex);
    int numSubsets = col->GetNumberOfSubsets();

    // Allocate a tuple the with enough entries to hold the Collection
    // subset list.
    PyObject *retval = PyTuple_New(numSubsets);
    for(i = 0; i < numSubsets; ++i)
    {
        PyObject *dval = PyLong_FromLong((long)col->GetSubset(i));
        if(dval == NULL)
            continue;
        PyTuple_SET_ITEM(retval, i, dval);
    }

    return retval;
}

// ****************************************************************************
// Function: SILRestriction_TopSet
//
// Purpose:
//   Returns the top set index.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 14:55:13 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Changed pointer to SIL restriction to reference counted pointer to a SIL
//   restriction.  This is consistent with handling elsewhere in the code.
//
// ****************************************************************************

static PyObject *
SILRestriction_TopSet(PyObject *self, PyObject *args)
{
    PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
    avtSILRestriction_p silr = *(obj->silr);
    PyObject *retval = PyLong_FromLong((long)silr->GetTopSet());
    return retval;
}

// ****************************************************************************
// Function: SILRestriction_TurnOnAll
//
// Purpose:
//   Turns on all sets in the SIL restriction.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 14:55:13 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Changed pointer to SIL restriction to reference counted pointer to a SIL
//   restriction.  This is consistent with handling elsewhere in the code.
//
// ****************************************************************************

static PyObject *
SILRestriction_TurnOnAll(PyObject *self, PyObject *args)
{
    TRY
    {
        PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
        avtSILRestriction_p silr = *(obj->silr);
        silr->TurnOnAll();
    }
    CATCH2(VisItException, e)
    {
        CATCH_RETURN2(1, PyErr_Format(PyExc_RuntimeError, "Internal VisIt exception \"%s\"", e.Message().c_str()));
    }
    ENDTRY

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: SILRestriction_TurnOffAll
//
// Purpose:
//   Turns off all sets in the SIL restriction.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 14:55:13 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Changed pointer to SIL restriction to reference counted pointer to a SIL
//   restriction.  This is consistent with handling elsewhere in the code.
//
// ****************************************************************************

static PyObject *
SILRestriction_TurnOffAll(PyObject *self, PyObject *args)
{
    TRY
    {
        PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
        avtSILRestriction_p silr = *(obj->silr);
        silr->TurnOffAll();
    }
    CATCH2(VisItException, e)
    {
        CATCH_RETURN2(1, PyErr_Format(PyExc_RuntimeError, "Internal VisIt exception \"%s\"", e.Message().c_str()));
    }
    ENDTRY

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: SILRestriction_TurnOnSet
//
// Purpose:
//   Turns on a set that is specified by a set index.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 14:55:13 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Changed pointer to SIL restriction to reference counted pointer to a SIL
//   restriction.  This is consistent with handling elsewhere in the code.
//
// ****************************************************************************

static PyObject *
SILRestriction_TurnOnSet(PyObject *self, PyObject *args)
{
    int setNumber;
    int outOfRangeMax = -1;
    if(!PyArg_ParseTuple(args, "i", &setNumber))
        return PyErr_Format(PyExc_TypeError, "Expecting integer set index");

    TRY
    {
        PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
        avtSILRestriction_p silr = *(obj->silr);
        if (0 <= setNumber && setNumber < silr->GetNumSets())
            silr->TurnOnSet(setNumber);
        else
            outOfRangeMax = silr->GetNumSets();
    }
    CATCH2(VisItException, e)
    {
        CATCH_RETURN2(1, PyErr_Format(PyExc_RuntimeError, "Internal VisIt exception \"%s\"", e.Message().c_str()));
    }
    ENDTRY

    if (outOfRangeMax != -1)
        return PyErr_Format(PyExc_ValueError, "Set integer index %d out of range [0,%d]", setNumber, outOfRangeMax);

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: SILRestriction_TurnOffSet
//
// Purpose:
//   Turns off the set specified by the set index.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 14:55:13 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Changed pointer to SIL restriction to reference counted pointer to a SIL
//   restriction.  This is consistent with handling elsewhere in the code.
//
// ****************************************************************************

static PyObject *
SILRestriction_TurnOffSet(PyObject *self, PyObject *args)
{
    int setNumber;
    int outOfRangeMax = -1;
    if(!PyArg_ParseTuple(args, "i", &setNumber))
        return PyErr_Format(PyExc_TypeError, "Expecting integer set index");

    TRY
    {
        PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
        avtSILRestriction_p silr = *(obj->silr);
        if (0 <= setNumber && setNumber < silr->GetNumSets())
            silr->TurnOffSet(setNumber);
        else
            outOfRangeMax = silr->GetNumSets();
    }
    CATCH2(VisItException, e)
    {
        CATCH_RETURN2(1, PyErr_Format(PyExc_RuntimeError, "Internal VisIt exception \"%s\"", e.Message().c_str()));
    }
    ENDTRY

    if (outOfRangeMax != -1)
        return PyErr_Format(PyExc_ValueError, "Set integer index %d out of range [0,%d]", setNumber, outOfRangeMax);

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: SILRestriction_TurnSet
//
// Purpose:
//   Turns the specified set index on or off.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 14:55:13 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Changed pointer to SIL restriction to reference counted pointer to a SIL
//   restriction.  This is consistent with handling elsewhere in the code.
//
// ****************************************************************************

static PyObject *
SILRestriction_TurnSet(PyObject *self, PyObject *args)
{
    int setNumber, onOff;
    int outOfRangeMax = -1;
    if(!PyArg_ParseTuple(args, "ii", &setNumber, &onOff))
        return PyErr_Format(PyExc_TypeError, "Expecting integer set index and integer state (0/1=off/on) pair");

    TRY
    {
        PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
        avtSILRestriction_p silr = *(obj->silr);
        if (0 <= setNumber && setNumber < silr->GetNumSets())
        {
            if (onOff)
                silr->TurnOnSet(setNumber);
            else
                silr->TurnOffSet(setNumber);
        }
        else
           outOfRangeMax = silr->GetNumSets();
    }
    CATCH2(VisItException, e)
    {
        CATCH_RETURN2(1, PyErr_Format(PyExc_RuntimeError, "Internal VisIt exception \"%s\"", e.Message().c_str()));
    }
    ENDTRY

    if (outOfRangeMax != -1)
        return PyErr_Format(PyExc_ValueError, "Set integer index %d out of range [0,%d]", setNumber, outOfRangeMax);

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: SILRestriction_UsesAllData
//
// Purpose:
//   Returns whether or not all sets are on.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 14:55:13 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Used a SIL Restriction Traverser to meet the new interface.
//
// ****************************************************************************

static PyObject *
SILRestriction_UsesAllData(PyObject *self, PyObject *args)
{
    PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
    avtSILRestrictionTraverser trav(*(obj->silr));
    PyObject *retval = PyLong_FromLong((long)(trav.UsesAllData()?1:0));

    return retval;
}

// ****************************************************************************
// Function: SILRestriction_UsesData
//
// Purpose:
//   Returns whether or not the specified set is being used.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 14:55:13 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Used a SIL Restriction Traverser to meet the new interface.
//
// ****************************************************************************

static PyObject *
SILRestriction_UsesData(PyObject *self, PyObject *args)
{
    PyObject *retval = NULL;
    int outOfRangeMax = -1;
    int setNumber;
    if(!PyArg_ParseTuple(args, "i", &setNumber))
        return PyErr_Format(PyExc_TypeError, "Expecting integet set index");

    TRY
    {
        PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
        avtSILRestriction_p silr = *(obj->silr);
        avtSILRestrictionTraverser trav(silr);
        if (0 <= setNumber && setNumber < silr->GetNumSets())
            retval = PyLong_FromLong((long)(trav.UsesData(setNumber)?1:0));
        else
            outOfRangeMax = silr->GetNumSets();
    }
    CATCH2(VisItException, e)
    {
        Py_XDECREF(retval);
        CATCH_RETURN2(1, PyErr_Format(PyExc_RuntimeError, "Internal VisIt exception \"%s\"", e.Message().c_str()));
    }
    ENDTRY

    if (outOfRangeMax != -1)
        return PyErr_Format(PyExc_ValueError, "Set integer index %d out of range [0,%d]", setNumber, outOfRangeMax);

    return retval;
}

// ****************************************************************************
// Function: SILRestriction_Wholes
//
// Purpose:
//   Returns a tuple of whole set indices.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 14:55:13 PST 2001
//
// Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Changed pointer to SIL restriction to reference counted pointer to a SIL
//   restriction.  This is consistent with handling elsewhere in the code.
//
// ****************************************************************************

static PyObject *
SILRestriction_Wholes(PyObject *self, PyObject *args)
{
    PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
    avtSILRestriction_p silr = *(obj->silr);

    // Allocate a tuple the with enough entries to hold the wholes.
    PyObject *retval = PyTuple_New(silr->GetWholes().size());

    for(size_t i = 0; i < silr->GetWholes().size(); ++i)
    {
        PyObject *dval = PyLong_FromLong((long)silr->GetWholes()[i]);
        if(dval == NULL)
            continue;
        PyTuple_SET_ITEM(retval, i, dval);
    }

    return retval;
}

// ****************************************************************************
// Method: SILRestriction_SuspendCorrectnessChecking
//
// Purpose:
//   Suspends SILR correctness checking.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 9 18:10:12 PST 2006
//
// Modifications:
//
// ****************************************************************************

static PyObject *
SILRestriction_SuspendCorrectnessChecking(PyObject *self, PyObject *args)
{
    PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
    avtSILRestriction_p silr = *(obj->silr);
    silr->SuspendCorrectnessChecking();

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Method: SILRestriction_EnableCorrectnessChecking
//
// Purpose:
//   Enables SILR correctness checking.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 9 18:10:12 PST 2006
//
// Modifications:
//
// ****************************************************************************

static PyObject *
SILRestriction_EnableCorrectnessChecking(PyObject *self, PyObject *args)
{
    PySILRestrictionObject *obj = (PySILRestrictionObject *)self;
    avtSILRestriction_p silr = *(obj->silr);
    silr->EnableCorrectnessChecking();

    Py_INCREF(Py_None);
    return Py_None;
}

static struct PyMethodDef SILRestriction_methods[] = {
    {"Categories",       SILRestriction_Categories, METH_VARARGS},
    {"MapsOut",          SILRestriction_MapsOut, METH_VARARGS},
    {"NumCategories",    SILRestriction_NumCategories, METH_VARARGS},
    {"NumSets",          SILRestriction_NumSets, METH_VARARGS},
    {"SetIndex",         SILRestriction_SetIndex, METH_VARARGS},
    {"SetName",          SILRestriction_SetName, METH_VARARGS},
    {"SetsInCategory",   SILRestriction_SetsInCategory, METH_VARARGS},
    {"TopSet",           SILRestriction_TopSet, METH_VARARGS},
    {"TurnOnAll",        SILRestriction_TurnOnAll, METH_VARARGS},
    {"TurnOffAll",       SILRestriction_TurnOffAll, METH_VARARGS},
    {"TurnOnSet",        SILRestriction_TurnOnSet, METH_VARARGS},
    {"TurnOffSet",       SILRestriction_TurnOffSet, METH_VARARGS},
    {"TurnSet",          SILRestriction_TurnSet, METH_VARARGS},
    {"UsesAllData",      SILRestriction_UsesAllData, METH_VARARGS},
    {"UsesData",         SILRestriction_UsesData, METH_VARARGS},
    {"Wholes",           SILRestriction_Wholes, METH_VARARGS},
    {"SuspendCorrectnessChecking", SILRestriction_SuspendCorrectnessChecking, METH_VARARGS},
    {"EnableCorrectnessChecking", SILRestriction_EnableCorrectnessChecking, METH_VARARGS},
    {NULL, NULL}
};

//
// Type functions
//

// ****************************************************************************
// Function: SILRestriction_dealloc
//
// Purpose:
//   Destructor for PySILRestriction.
//
//
// Programmer: TODO
// Creation:   TODO
//
//  Modifications:
//
//    Hank Childs, Mon Dec  2 14:08:29 PST 2002
//    Do not delete the SIL restriction, since it is reference counted.
//
// ****************************************************************************
static void
SILRestriction_dealloc(PyObject *v)
{
    // SIL restriction is a ref ptr, so it will clean itself up.
}

// ****************************************************************************
// Function: SILRestriction_compare
//
// Purpose:
//   Comparison function for PySILRestriction.
//
//
// Programmer: TODO
// Creation:   TODO
//
//  Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Used a SIL Restriction Traverser to meet the new interface.
//
// ****************************************************************************

static int
SILRestriction_compare(PyObject *v, PyObject *w)
{
    avtSILRestriction_p a = *(((PySILRestrictionObject *)v)->silr);
    avtSILRestrictionTraverser trav(a);
    avtSILRestriction_p b = *(((PySILRestrictionObject *)w)->silr);
    return trav.Equal(b) ? 0 : -1;
}

// ****************************************************************************
// Function: SILRestriction_getattr
//
// Purpose:
//   Attribute fetch for PySILRestriction.
//
//
// Programmer: TODO
// Creation:   TODO
//
// Modifications:
//
// ****************************************************************************

static PyObject *
SILRestriction_getattr(PyObject *self, char *name)
{
    // Add a __dict__ answer so that dir() works
    if (!strcmp(name, "__dict__"))
    {
        PyObject *result = PyDict_New();
        for (int i = 0; SILRestriction_methods[i].ml_meth; i++)
            PyDict_SetItem(result,
                PyString_FromString(SILRestriction_methods[i].ml_name),
                PyString_FromString(SILRestriction_methods[i].ml_name));
        return result;
    }

    return Py_FindMethod(SILRestriction_methods, self, name);
}


// ****************************************************************************
// Function: SILRestriction_print
//
// Purpose:
//   Print method for SILRestriction.
//
//
// Programmer: TODO
// Creation:   TODO
//
// Modifications:
//
// ****************************************************************************

static int
SILRestriction_print(PyObject *v, FILE *fp, int flags)
{
    PySILRestrictionObject *obj = (PySILRestrictionObject *)v;
    avtSILRestriction_p silr = *(obj->silr);
    silr->Print(cout);
    return 0;
}

#if PY_MAJOR_VERSION > 2 || (PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 5)
static const char *SILRestriction_Purpose = "This class contains attributes used to restrict the subset inclusion lattice of a plot.";
#else
static char *SILRestriction_Purpose = "This class contains attributes used to restrict the subset inclusion lattice of a plot.";
#endif

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
VISIT_PY_TYPE_OBJ(PySILRestrictionType,        \
                  "SILRestriction",            \
                  PySILRestrictionObject,      \
                  SILRestriction_dealloc,      \
                  SILRestriction_print,        \
                  SILRestriction_getattr,      \
                  0,                           \
                  0,                           \
                  SILRestriction_Purpose,      \
                  SILRestriction_richcompare,  \
                  0); /* as_number*/

// ****************************************************************************
// Function: SILRestriction_richcompare
//
// Purpose:
//   Comparison function for PySILRestriction.
//
//
// Programmer: TODO
// Creation:   TODO
//
//  Modifications:
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Used a SIL Restriction Traverser to meet the new interface.
//
// ****************************************************************************
static PyObject *
SILRestriction_richcompare(PyObject *self, PyObject *other, int op)
{
    // only compare against the same type
    if ( Py_TYPE(self) != Py_TYPE(other)
         || Py_TYPE(self) != &PySILRestrictionType)
    {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }

    PyObject *res = NULL;

    avtSILRestriction_p self_silr = *(((PySILRestrictionObject *)self)->silr);
    avtSILRestrictionTraverser trav(self_silr);
    avtSILRestriction_p other_silr = *(((PySILRestrictionObject *)other)->silr);
    bool silr_equal = trav.Equal(other_silr);

    switch (op)
    {
       case Py_EQ:
           res = silr_equal ? Py_True : Py_False;
           break;
       case Py_NE:
           res = !silr_equal ? Py_True : Py_False;
           break;
       default:
           res = Py_NotImplemented;
           break;
    }

    Py_INCREF(res);
    return res;
}



///////////////////////////////////////////////////////////////////////////////
//
// Main Python/C Interface
//
///////////////////////////////////////////////////////////////////////////////


// ****************************************************************************
// Function: PySILRestriction_Wrap
//
// Purpose:
//   Create wrap avtSILRestriction_p into a python sil restriction object.
//
//
// Programmer: Cyrus Harrison
// Creation:   Wed Feb 10 09:28:43 PST 2010
//
// Modifications:
//
// ****************************************************************************

PyObject *
PySILRestriction_Wrap(avtSILRestriction_p restriction)
{
    PySILRestrictionObject *res;
    res = PyObject_NEW(PySILRestrictionObject, &PySILRestrictionType);
    if(res  == NULL)
        return NULL;
    res->silr = new avtSILRestriction_p;
    // set the restriction
    *(res->silr) = restriction;
    return (PyObject *)res;
}


// ****************************************************************************
// Function: PySILRestriction_Check
//
// Purpose:
//   Check if given python object is an instance of PySILRestriction.
//
//
// Programmer: TODO
// Creation:   TODO
//
// Modifications:
//
// ****************************************************************************

bool
PySILRestriction_Check(PyObject *obj)
{
    return (obj->ob_type == &PySILRestrictionType);
}

// ****************************************************************************
// Function: PySILRestriction_FromPyObject
//
// Purpose:
//   Obtain the avtSILRestriction from a python sil restriction object.
//
//
// Programmer: TODO
// Creation:   TODO
//
// Modifications:
//
// ****************************************************************************

avtSILRestriction_p
PySILRestriction_FromPyObject(PyObject *obj)
{
    PySILRestrictionObject *obj2 = (PySILRestrictionObject *)obj;
    return *(obj2->silr);
}


