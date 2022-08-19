// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <string>
#include <sstream>

#include <Python.h>
#include <Py2and3Support.h>

#include <PyMapNode.h>
#include <PyVariant.h>
#include <DebugStream.h>

///////////////////////////////////////////////////////////////////////////////
//
// VisIt module helper method.
//
///////////////////////////////////////////////////////////////////////////////


// ****************************************************************************
// Method: PyMapNode_Wrap
//
// Purpose:
//   Converts a MapNode to a python dictonary.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Dec 17 15:20:25 PST 2007
//
// Modifications:
//   Eric Brugger, Fri Jan 11 09:54:04 PST 2008
//   I added some Python version specific coding to get around a problem
//   with older versions not having a const qualifier for the string argument
//   to PyDict_SetItemString.
//
// ****************************************************************************

PyObject *
PyMapNode_Wrap(const MapNode &node)
{
    if(node.Type() == MapNode::EMPTY_TYPE && node.GetNumEntries() == 0)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    // must be a variant, use variant helper
    if(node.Type() != MapNode::EMPTY_TYPE)
    {
        return PyVariant_Wrap(node);
    }

    // we have a dict with map nodes as entries
    PyObject *dict = PyDict_New();
    stringVector entry_names;
    node.GetEntryNames(entry_names);

    for(size_t i=0;i<entry_names.size();i++)
    {
        const MapNode *child_node = node.GetEntry(entry_names[i]);
        if(child_node == NULL)
            continue;
        PyObject *child = PyMapNode_Wrap(*child_node);
#if (PY_MAJOR_VERSION < 2) || ((PY_MAJOR_VERSION == 2) && (PY_MINOR_VERSION < 5))
        char *str = new char[entry_names[i].length()+1];
        strcpy(str, entry_names[i].c_str());
        PyDict_SetItemString(dict, str, child);
        delete [] str;
#else
        PyDict_SetItemString(dict, entry_names[i].c_str(), child);
#endif
    }

    return dict;
}


// ****************************************************************************
// Method: PyDict_To_MapNode
//
// Purpose:
//   Converts a python Dictionary to a MapNode.
//
// Programmer: Kathleen Bonnell
// Creation:   July 13, 2011
//
// Modifications:
//   Kathleen Biagas, Wed Sep  7 11:56:23 PDT 2011
//   Allow ints and doubles in same sequence.
//
//   Kathleen Biagas, Mon Mar 24 17:00:11 PDT 2014
//   Parse Dict.
//
//   Kathleen Biagas, Mon Jun  5 17:25:15 PDT 2017
//   Allow for PyLong.
//
//   Cyrus Harrison, Wed Dec 18 16:57:30 PST 2019
//   Support for Python 2 and Python 3
//
// ****************************************************************************

bool
PyDict_To_MapNode(PyObject *obj, MapNode &mn, std::string& errmsg)
{
    if (!PyDict_Check(obj))
        return false;

    Py_ssize_t pos = 0;
    PyObject *key = NULL;
    PyObject *value = NULL;

    while(PyDict_Next(obj, &pos, &key, &value))
    {
        std::string mkey;
        if (PyString_Check(key))
        {
            char *key_cstr = PyString_AsString(key);
            mkey = key_cstr;
            PyString_AsString_Cleanup(key_cstr);
        }
        else
        {
            return false;
        }

        if (PyTuple_Check(value) ||
            PyList_Check(value))
        {
            PyObject *item = PySequence_GetItem(value, 0);
            if (PyFloat_Check(item))
            {
                 doubleVector mval;
                 mval.push_back(PyFloat_AS_DOUBLE(item));
                 for (Py_ssize_t i = 1; i < PySequence_Size(value); ++i)
                 {
                     item = PySequence_GetItem(value, i);
                     if (PyFloat_Check(item))
                         mval.push_back(PyFloat_AS_DOUBLE(item));
                     else if (PyInt_Check(item))
                         mval.push_back((double)PyInt_AS_LONG(item));
                     else
                     {
                         debug3 << "PyDict_To_MapNode: tuples/lists must "
                                << "contain same type." << endl;
                         return false;
                     }
                 }
                 mn[mkey] = mval;
            }
            else if (PyLong_Check(item) || PyInt_Check(item))
            {
                 int ni = 1, nd = 0, no = 0;
                 for (Py_ssize_t i = 1; i < PySequence_Size(value); ++i)
                 {
                     item = PySequence_GetItem(value, i);
                     if (PyFloat_Check(item))
                         nd++;
                     else if (PyLong_Check(item))
                         ni++;
                     else if (PyInt_Check(item))
                         ni++;
                     else
                         no++;
                 }
                 if (no != 0)
                 {
                     debug3 << "PyDict_To_MapNode: tuples/lists must "
                            << "contain same type." << endl;
                     return false;
                 }
                 else if (nd != 0)
                 {
                     // process as doubleVector
                     doubleVector mval;
                     for (Py_ssize_t i = 0; i < PySequence_Size(value); ++i)
                     {
                         item = PySequence_GetItem(value, i);
                         if (PyFloat_Check(item))
                             mval.push_back(PyFloat_AS_DOUBLE(item));
                         else if (PyLong_Check(item))
                             mval.push_back((double)PyLong_AsLong(item));
                         else if (PyInt_Check(item))
                             mval.push_back((double)PyInt_AS_LONG(item));
                     }
                     mn[mkey] = mval;
                 }
                 else
                 {
                     intVector mval;
                     for (Py_ssize_t i = 0; i < PySequence_Size(value); ++i)
                     {
                         item = PySequence_GetItem(value, i);
                         if (PyLong_Check(item))
                             mval.push_back((double)PyLong_AsLong(item));
                         else if (PyInt_Check(item))
                             mval.push_back((double)PyInt_AS_LONG(item));
                     }
                     mn[mkey] = mval;
                }
            }
            else if (PyString_Check(item))
            {
                 stringVector mval;
                 char *item_cstr = PyString_AsString(item);
                 mval.push_back(std::string(item_cstr));
                 PyString_AsString_Cleanup(item_cstr);
                 for (Py_ssize_t i = 1; i < PySequence_Size(value); ++i)
                 {
                     item = PySequence_GetItem(value, i);
                     if (!PyString_Check(item))
                     {
                         debug3 << "PyDict_To_MapNode: tuples/lists must "
                                << "contain same type." << endl;
                         return false;
                     }
                     item_cstr = PyString_AsString(item);
                     mval.push_back(std::string(item_cstr));
                     PyString_AsString_Cleanup(item_cstr);
                 }
                 mn[mkey] = mval;
            }
            else
            {
                debug3 << "PyDict_To_MapNode: type "
                       << item->ob_type->tp_name
                       << " not currently implemented." << endl;
                return false;
            }
        }
        else if (PyFloat_Check(value))
        {
            mn[mkey] = (double) PyFloat_AS_DOUBLE(value);
        }
        else if (PyLong_Check(value))
        {
            mn[mkey] = (int) PyLong_AsLong(value);
        }
        else if (PyInt_Check(value))
        {
            mn[mkey] = (int) PyInt_AS_LONG(value);
        }
        else if (PyString_Check(value))
        {
            char *value_cstr = PyString_AsString(value);
            mn[mkey] = std::string(value_cstr);
            PyString_AsString_Cleanup(value_cstr);
        }
        else if (PyDict_Check(value))
        {
            MapNode tmp;
            if (PyDict_To_MapNode(value, tmp))
                mn[mkey] = tmp;
        }
        else
        {
            std::stringstream ss;

            ss << "PyDict_To_MapNode: argument named \""
                   << mkey
                   << "\" at position " 
                   << pos 
                   << " with type "
                   << value->ob_type->tp_name
                   << " not currently implemented." << endl;
            debug3 << ss.str();

            if (errmsg != PyMapNode_VoidString)
                errmsg += ss.str();

            return false;
        }
    }
    return true;
}
