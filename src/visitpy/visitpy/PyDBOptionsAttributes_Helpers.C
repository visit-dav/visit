// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <PyDBOptionsAttributes.h>
#include <ObserverToCallback.h>
#include <stdio.h>

#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>

namespace
{
bool
SetDictionaryItem(PyObject *dict, const char *name, PyObject *value)
{
    if (value == NULL)
        return false;

    int status = PyDict_SetItemString(dict, name, value);
    Py_DECREF(value);
    return status == 0;
}

std::string
PythonStringLiteral(const std::string &value)
{
    static const char hex[] = "0123456789abcdef";
    std::string result("'");

    for (size_t i = 0; i < value.size(); ++i)
    {
        unsigned char c = static_cast<unsigned char>(value[i]);
        switch (c)
        {
          case '\\': result += "\\\\"; break;
          case '\'': result += "\\'";  break;
          case '\n': result += "\\n";  break;
          case '\r': result += "\\r";  break;
          case '\t': result += "\\t";  break;
          default:
            if (c < 0x20 || c == 0x7f)
            {
                result += "\\x";
                result += hex[(c >> 4) & 0xf];
                result += hex[c & 0xf];
            }
            else
                result += static_cast<char>(c);
            break;
        }
    }

    result += "'";
    return result;
}

std::string
PythonFloatLiteral(double value, int precision)
{
    if (std::isnan(value))
        return "float('nan')";
    if (std::isinf(value))
        return value < 0. ? "-float('inf')" : "float('inf')";

    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << std::setprecision(precision) << value;

    std::string result(out.str());
    if (result.find_first_of(".eE") == std::string::npos)
        result += ".0";
    return result;
}

bool
DBOptionValueToPythonLiteral(const DBOptionsAttributes &opts, int index,
                             bool show_enum_opts, std::string &value)
{
    const std::string name(opts.GetName(index));
    switch (opts.GetType(index))
    {
      case DBOptionsAttributes::Bool:
        value = opts.GetBool(name) ? "1" : "0";
        break;
      case DBOptionsAttributes::Int:
        value = std::to_string(opts.GetInt(name));
        break;
      case DBOptionsAttributes::Float:
        value = PythonFloatLiteral(
            opts.GetFloat(name), std::numeric_limits<float>::max_digits10);
        break;
      case DBOptionsAttributes::Double:
        value = PythonFloatLiteral(
            opts.GetDouble(name),
            std::numeric_limits<double>::max_digits10);
        break;
      case DBOptionsAttributes::String:
        value = PythonStringLiteral(opts.GetString(name));
        break;
      case DBOptionsAttributes::Color:
        {
            int red, green, blue, alpha;
            opts.GetColor(name, red, green, blue, alpha);
            std::ostringstream color;
            color << "(" << red << ", " << green << ", " << blue
                  << ", " << alpha << ")";
            value = color.str();
        }
        break;
      case DBOptionsAttributes::MultiLineString:
        value = PythonStringLiteral(opts.GetMultiLineString(name));
        break;
      case DBOptionsAttributes::Enum:
        {
            int enumIndex = opts.GetEnum(name);
            stringVector enumStrings = opts.GetEnumStrings(name);
            if (enumIndex >= 0 &&
                static_cast<size_t>(enumIndex) < enumStrings.size())
            {
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
                value = PythonStringLiteral(itemString);
            }
            else
                value = std::to_string(enumIndex);
        }
        break;
      default:
        return false;
    }

    return true;
}
}

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
//    Justin Privitera, Mon Jul 20 16:40:22 PDT 2026
//    Now handles the new DBOptionsAttributes::Color option type. For color
//    options, instead of ignoring them or failing, it reads the RGBA
//    components from DBOptionsAttributes and inserts a Python 4-tuple 
//    (r, g, b, a) into the returned dictionary.
//
//    Kathleen Biagas, Wed Sep 23, 2026
//    Check enum bounds and release temporary Python objects.
//
// ****************************************************************************

PyObject *
PyDBOptionsAttributes_CreateDictionaryFromDBOptions(const DBOptionsAttributes &opts,
                                                    bool show_enum_opts)
{
    PyObject *dict = PyDict_New();
    if (dict == NULL)
        return NULL;

    for (int j=0; j<opts.GetNumberOfOptions(); j++)
    {
        // Older pythons don't support const char* in the PyDict routines,
        // so we have to copy this into a non-const string.
        char *name = new char[opts.GetName(j).length()+1];
        strcpy(name, opts.GetName(j).c_str());
        bool itemAdded = false;
        switch (opts.GetType(j))
        {
          case DBOptionsAttributes::Bool:
            itemAdded = SetDictionaryItem(dict, name,
                                          PyInt_FromLong(opts.GetBool(name)));
            break;
          case DBOptionsAttributes::Int:
            itemAdded = SetDictionaryItem(dict, name,
                                          PyInt_FromLong(opts.GetInt(name)));
            break;
          case DBOptionsAttributes::Float:
            itemAdded = SetDictionaryItem(dict, name,
                                          PyFloat_FromDouble(opts.GetFloat(name)));
            break;
          case DBOptionsAttributes::Double:
            itemAdded = SetDictionaryItem(dict, name,
                                          PyFloat_FromDouble(opts.GetDouble(name)));
            break;
          case DBOptionsAttributes::String:
            itemAdded = SetDictionaryItem(
                dict, name, PyString_FromString(opts.GetString(name).c_str()));
            break;
          case DBOptionsAttributes::Color:
            {
                int red, green, blue, alpha;
                opts.GetColor(name, red, green, blue, alpha);
                PyObject *tuple = Py_BuildValue("(iiii)",
                                                red, green, blue, alpha);
                itemAdded = SetDictionaryItem(dict, name, tuple);
            }
            break;
          case DBOptionsAttributes::MultiLineString:
            itemAdded = SetDictionaryItem(
                dict, name,
                PyString_FromString(opts.GetMultiLineString(name).c_str()));
            break;
          case DBOptionsAttributes::Enum:
            {
                // If you modify this section, also check the Enum case in
                // FillDBOptionsFromDictionary
                int enumIndex = opts.GetEnum(name);
                stringVector enumStrings = opts.GetEnumStrings(name);
                if (enumIndex >= 0 &&
                    static_cast<size_t>(enumIndex) < enumStrings.size())
                {
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
                    itemAdded = SetDictionaryItem(
                        dict, name, PyString_FromString(itemString.c_str()));
                }
                else
                    itemAdded = SetDictionaryItem(dict, name,
                                                  PyInt_FromLong(enumIndex));
            }
            break;
        }
        delete[] name;

        if (!itemAdded)
        {
            Py_DECREF(dict);
            return NULL;
        }
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
//
//  Modifications:
//    Kathleen Biagas, Wed Sep 23, 2026
//    Build the representation without the Python C API. This function can be
//    called by command recording without the GIL.
//
// ****************************************************************************
std::string
PyDBOptionsAttributes_CreateDictionaryStringFromDBOptions(const DBOptionsAttributes &opts,
                                                          bool show_enum_opts)
{
    // This helper is called while translating viewer actions into CLI
    // commands. That callback does not necessarily hold Python's GIL, so do
    // not use the Python C API here.
    std::ostringstream result;
    result.imbue(std::locale::classic());
    result << "{";

    bool first = true;
    for (int j = 0; j < opts.GetNumberOfOptions(); ++j)
    {
        const std::string name(opts.GetName(j));
        std::string value;
        if (!DBOptionValueToPythonLiteral(opts, j, show_enum_opts, value))
            continue;
        if (!first)
            result << ", ";
        result << PythonStringLiteral(name) << ": " << value;
        first = false;
    }

    result << "}";
    return result.str();
}

// ****************************************************************************
//  Method: PyDBOptionsAttributes_CreateDictionaryAssignmentsFromDBOptions
//
//  Purpose:
//     Create Python assignment statements for a dictionary of DB options.
//
// ****************************************************************************
std::string
PyDBOptionsAttributes_CreateDictionaryAssignmentsFromDBOptions(
    const DBOptionsAttributes &opts, const std::string &dict_name,
    bool show_enum_opts)
{
    std::ostringstream result;
    result.imbue(std::locale::classic());

    for (int j = 0; j < opts.GetNumberOfOptions(); ++j)
    {
        std::string value;
        if (!DBOptionValueToPythonLiteral(opts, j, show_enum_opts, value))
            continue;

        result << dict_name << "[" << PythonStringLiteral(opts.GetName(j))
               << "] = " << value << "\n";
    }

    return result.str();
}
