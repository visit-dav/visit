// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <PyDBOptionsAttributes.h>
#include <ObserverToCallback.h>
#include <stdio.h>

// ****************************************************************************
//  Method: PyDBOptionsAttributes_CreateDictionaryFromDBOptions
//
//  Purpose:
//     Helper that creates a Python dict of the DB options.
//
//  Programmer: Cyrus Harrison
//  Creation:   Mon May 11 14:14:38 PDT 2020
//
// ****************************************************************************
// Note: (cyrush) This would ideally be in PyDBOptionsAttributes but I
//       hit snags generating defs in headers to expose via xmltools
// ****************************************************************************
//
//  Modifications:
//    Kathleen Biagas, Tue Sep 13, 2022
//    Support MultiLineString option type.
//
// ****************************************************************************

PyObject *
PyDBOptionsAttributes_CreateDictionaryFromDBOptions(const DBOptionsAttributes &opts,
                                                    bool show_enum_opts)
{
    PyObject *dict = PyDict_New();
    for (int j=0; j<opts.GetNumberOfOptions(); j++)
    {
        // Older pythons don't support const char* in the PyDict routines,
        // so we have to copy this into a non-const string.
        char *name = new char[opts.GetName(j).length()+1];
        strcpy(name, opts.GetName(j).c_str());
        switch (opts.GetType(j))
        {
          case DBOptionsAttributes::Bool:
            PyDict_SetItemString(dict,name,PyInt_FromLong(opts.GetBool(name)));
            break;
          case DBOptionsAttributes::Int:
            PyDict_SetItemString(dict,name,PyInt_FromLong(opts.GetInt(name)));
            break;
          case DBOptionsAttributes::Float:
            PyDict_SetItemString(dict,name,PyFloat_FromDouble(opts.GetFloat(name)));
            break;
          case DBOptionsAttributes::Double:
            PyDict_SetItemString(dict,name,PyFloat_FromDouble(opts.GetDouble(name)));
            break;
          case DBOptionsAttributes::String:
            PyDict_SetItemString(dict,name,PyString_FromString(opts.GetString(name).c_str()));
            break;
          case DBOptionsAttributes::MultiLineString:
            PyDict_SetItemString(dict,name,PyString_FromString(opts.GetMultiLineString(name).c_str()));
            break;
          case DBOptionsAttributes::Enum:
                // If you modify this section, also check the Enum case in
                // FillDBOptionsFromDictionary
                int enumIndex = opts.GetEnum(name);
                stringVector enumStrings = opts.GetEnumStrings(name);
                std::string itemString(enumStrings[enumIndex]);
                if (show_enum_opts && enumStrings.size() > 1)
                {
                    itemString += " # Options are: ";
                    for (size_t i = 0; i < enumStrings.size(); ++i)
                    {
                        itemString += enumStrings[i];
                        if (i != enumStrings.size()-1)
                            itemString += ", ";
                    }
                }
                PyDict_SetItemString(dict,name,PyString_FromString(itemString.c_str()));
            break;
        }
        delete[] name;
    }
    return dict;
}

// ****************************************************************************
//  Method: PyDBOptionsAttributes_CreateDictionaryStringFromDBOptions
//
//  Purpose:
//     Create a string rep of the Python Dictionary of our opts.
//
//  Programmer: Cyrus Harrison
//  Creation:   Mon May 11 14:14:38 PDT 2020
//
// ****************************************************************************
// Note: (cyrush) This would ideally be in PyDBOptionsAttributes but I
//       hit snags generating defs in headers to expose via xmltools
// ****************************************************************************
std::string
PyDBOptionsAttributes_CreateDictionaryStringFromDBOptions(const DBOptionsAttributes &opts,
                                                          bool show_enum_opts)
{
    PyObject *py_opts_dict = PyDBOptionsAttributes_CreateDictionaryFromDBOptions(opts,
                                                                           show_enum_opts);
    PyObject *py_opts_repr = PyObject_Repr(py_opts_dict);
    const char *opts_repr_cstr = PyString_AsString(py_opts_repr);
    std::string res = std::string(opts_repr_cstr);

    Py_DECREF(py_opts_dict);
    Py_DECREF(py_opts_repr);
    return res;
}

