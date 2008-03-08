/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef GENERATE_PYTHON_H
#define GENERATE_PYTHON_H

#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include "Field.h"
#include <qregexp.h>

using std::vector;

#define GENERATOR_NAME "xml2python"

inline char toupper(char c)
{
    if (c>='a' && c<='z')
    {
        c = char(int(c) + int('A')-('a'));
    }
    return c;
}

// ****************************************************************************
//  File:  GenerateAtts
//
//  Purpose:
//    Contains a set of classes which override the default implementation
//    to create an attribute subject.
//
//  Note: This file overrides --
//    FieldFactory
//    Field
//    Attribute
//    Enum
//    Plugin
//
//  Programmer:  Brad Whitlock
//  Creation:    August 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Apr 19 15:25:46 PDT 2002
//    Added constants, includes, and a codefile.
//
//    Jeremy Meredith, Fri May 31 13:41:21 PDT 2002
//    Made enumerated types be real enums.
//
//    Jeremy Meredith, Fri Jul 12 16:28:40 PDT 2002
//    Added my own toupper() since some implementations don't have one.
//
//    Jeremy Meredith, Tue Aug 27 14:32:50 PDT 2002
//    Added mfiles, dbtype, and libs.  Allowed NULL atts.
//
//    Jeremy Meredith, Thu Oct 17 15:58:29 PDT 2002
//    Added some enhancements for the XML editor.
//
//    Brad Whitlock, Thu Dec 12 09:58:30 PDT 2002
//    I added ucharVector support.
//
//    Brad Whitlock, Fri Jun 20 10:55:22 PDT 2003
//    I fixed a bug in the code generation for setattr that caused bad code
//    to be generated when the first field is internal.
//
//    Kathleen Bonnell, Fri Jun 27 14:49:20 PDT 2003 
//    I made code generation for Enum's getattr obey the internal flag.
//
//    Brad Whitlock, Thu Sep 11 11:24:28 PDT 2003
//    I rewrote the code to set colors so it handles doubles and ints.
//
//    Jeremy Meredith, Tue Sep 23 17:09:31 PDT 2003
//    Made haswriter be a bool.
//
//    Jeremy Meredith, Wed Nov  5 13:28:03 PST 2003
//    Added ability to disable plugins by default.
//
//    Brad Whitlock, Thu Mar 18 15:02:35 PST 2004
//    I made it generate the str() method that can cast the extension object
//    into a string so we can use them in the test suite.
//
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Brad Whitlock, Fri Jul 30 16:29:12 PST 2004
//    Fixed the size of a buffer used by SNPRINTF when writing out enum code.
//
//    Brad Whitlock, Wed Dec 8 15:55:24 PST 2004
//    Added support for variable names.
//
//    Brad Whitlock, Wed Apr 6 11:55:26 PDT 2005
//    Added support for generating code to set/get ColorAttributeList.
//
//    Hank Childs, Tue May 24 10:19:40 PDT 2005
//    Added hasoptions.
//
//    Brad Whitlock, Thu Jan 5 13:33:40 PST 2006
//    Changed string representation coding and removed most print coding by
//    making it use the string coding.
//
//    Hank Childs, Fri Jun  9 09:50:37 PDT 2006
//    Added copyright string.
//
//    Brad Whitlock, Tue Nov 21 10:24:37 PDT 2006
//    Added support for line style symbols.
//
//    Cyrus Harrison, Wed Mar  7 09:52:17 PST 2007
//    Allow for engine-specific code in a plugin's source files.
//
//    Cyrus Harrison, Tue May 29 13:24:42 PDT 2007
//    Fixed code gen problem with LineStyle.
//
//    Brad Whitlock, Tue Jan 29 11:18:05 PDT 2008
//    Improved the accessibility of nested AttributeSubjects.
//
//    Brad Whitlock, Thu Feb 28 16:29:56 PST 2008
//    Made use of base classes. Added support for code file.
//
// ****************************************************************************

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class PythonGeneratorField : public virtual Field
{
  protected:
    QString generatorName;
  public:
    PythonGeneratorField(const QString &t, const QString &n, const QString &l)
        : Field(t,n,l), generatorName(GENERATOR_NAME)
    {
    }

    virtual ~PythonGeneratorField()
    {
    }

    QString MethodNameGet() const
    {
        QString tmp("Get");
        tmp += Name;
        return tmp;
    }

    QString MethodNameSet() const
    {
        QString tmp("Set");
        tmp += Name;
        return tmp;
    }

    virtual void WriteIncludedHeaders(ostream &c)
    {
        // Write the list of include files that the object needs.
    }

    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    // NOT IMPLEMENTED!!!" << endl;
        c << "    // name=" << name << ", type=" << type << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    // NOT IMPLEMENTED!!!" << endl;
        c << "    // name=" << name << ", type=" << type << endl;
        c << "    PyObject *retval = NULL;" << endl;
    }

    // Whether a Set method should be created. Most objects will allow the set 
    // method to be created but att and attVector do not allow it for all types.
    virtual bool ProvidesSetMethod() const { return true; }

    virtual void WriteAdditionalMethods(ostream &c, const QString &className)
    {
        // Do nothing.
    }

    virtual std::vector<QString> AdditionalMethodNames(const QString &className)
    {
        std::vector<QString> ret;
        return ret;
    }

    virtual void WriteSetMethod(ostream &c, const QString &className)
    {
        c << "static PyObject *" << endl;
        c << className << "_" << MethodNameSet() << "(PyObject *self, PyObject *args)" << endl;
        c << "{" << endl;
        c << "    " << className << "Object *obj = ("<<className<<"Object *)self;" << endl;
        c << endl;
        WriteSetMethodBody(c, className);
        c << endl;
        c << "    Py_INCREF(Py_None);" << endl;
        c << "    return Py_None;" << endl;
        c << "}" << endl;
        c << endl;
    }

    virtual void WriteGetMethod(ostream &c, const QString &className)
    {
        c << "static PyObject *" << endl;
        c << className << "_" << MethodNameGet() << "(PyObject *self, PyObject *args)" << endl;
        c << "{" << endl;
        c << "    " << className << "Object *obj = ("<<className<<"Object *)self;" << endl;
        WriteGetMethodBody(c, className);
        c << "    return retval;" << endl;
        c << "}" << endl;
        c << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    //" << name << endl;
    }

    virtual void WritePyObjectMethodTable(ostream &c, const QString &className)
    {
        // Do not add any methods if the field is internal.
        if(internal)
            return;

        if(ProvidesSetMethod())
            c << "    {\"" << MethodNameSet() << "\", " << className << "_" << MethodNameSet() << ", METH_VARARGS}," << endl;

        c << "    {\"" << MethodNameGet() << "\", " << className << "_" << MethodNameGet() << ", METH_VARARGS}," << endl;

        // Write any additional methods that may go along with this field.
        std::vector<QString> additionalMethods(AdditionalMethodNames(className));
        for(int i = 0; i < additionalMethods.size(); i += 2)
            c << "    {\"" << additionalMethods[i] << "\", " << additionalMethods[i+1] << ", METH_VARARGS}," << endl;
    }

    virtual void WriteGetAttr(ostream &c, const QString &className)
    {
        // Do not add any methods if the field is internal.
        if(internal)
            return;

        c << "    if(strcmp(name, \"" << name << "\") == 0)" << endl;
        c << "        return " << className << "_" << MethodNameGet() << "(self, NULL);" << endl;
    }

    virtual bool HasSetAttr()
    {
        return true;
    }

    virtual void WriteSetAttr(ostream &c, const QString &className, bool first)
    {
        if(internal)
            return;

        if(ProvidesSetMethod())
        {
            if(first)
                c << "    if(strcmp(name, \"" << name << "\") == 0)" << endl;
            else
                c << "    else if(strcmp(name, \"" << name << "\") == 0)" << endl;

            c << "        obj = "<<className<<"_"<<MethodNameSet()<<"(self, tuple);" << endl;
        }
    }
};

//
// ------------------------------------ Int -----------------------------------
//
class AttsGeneratorInt : public virtual Int , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorInt(const QString &n, const QString &l)
        : Int(n,l), PythonGeneratorField("int",n,l), Field("int",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    int ival;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &ival))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Set the " << name << " in the object." << endl;
        if(accessType == AccessPublic)
            c << "    obj->data->" << name << " = (" << type << ")ival;" << endl;
        else
            c << "    obj->data->" << MethodNameSet() << "((" << type << ")ival);" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    PyObject *retval = PyInt_FromLong(long(obj->data->";
        if(accessType == AccessPublic)
            c << name;
        else
            c << MethodNameGet()<<"()";
        c << "));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = %d\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ");" << endl;
        c << "    str += tmpStr;" << endl;
    }
};


//
// -------------------------------- IntArray --------------------------------
//
class AttsGeneratorIntArray : public virtual IntArray , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorIntArray(const QString &s, const QString &n, const QString &l)
        : IntArray(s,n,l), PythonGeneratorField("intArray",n,l), Field("intArray",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    int *ivals = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"";
        int i;
        for(i = 0; i < length; ++i)
            c << "i";
        c << "\", ";
        for(i = 0; i < length; ++i)
        {
            c << "&ivals[" << i << "]";
            if(i < length - 1)
                c << ", ";
        }
        c << "))" << endl;
        c << "    {" << endl;
        c << "        PyObject     *tuple;" << endl;
        c << "        if(!PyArg_ParseTuple(args, \"O\", &tuple))" << endl;
        c << "            return NULL;" << endl;
        c << endl;
        c << "        if(PyTuple_Check(tuple))" << endl;
        c << "        {" << endl;
        c << "            if(PyTuple_Size(tuple) != " << length << ")" << endl;
        c << "                return NULL;" << endl;
        c << endl;
        c << "            PyErr_Clear();" << endl;
        c << "            for(int i = 0; i < PyTuple_Size(tuple); ++i)" << endl;
        c << "            {" << endl;
        c << "                PyObject *item = PyTuple_GET_ITEM(tuple, i);" << endl;
        c << "                if(PyFloat_Check(item))" << endl;
        c << "                    ivals[i] = int(PyFloat_AS_DOUBLE(item));" << endl;
        c << "                else if(PyInt_Check(item))" << endl;
        c << "                    ivals[i] = int(PyInt_AS_LONG(item));" << endl;
        c << "                else if(PyLong_Check(item))" << endl;
        c << "                    ivals[i] = int(PyLong_AsDouble(item));" << endl;
        c << "                else" << endl;
        c << "                    ivals[i] = 0;" << endl;
        c << "            }" << endl;
        c << "        }" << endl;
        c << "        else" << endl;
        c << "            return NULL;" << endl;
        c << "    }" << endl;
        c << endl;
        c << "    // Mark the " << name << " in the object as modified." << endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << endl;
        else
            c << "    obj->data->Select" << Name << "();" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << endl;
        c << "    PyObject *retval = PyTuple_New(" << length << ");" << endl;
        c << "    const int *" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << ";" << endl;
        else
            c << MethodNameGet() << "();" << endl;
        c << "    for(int i = 0; i < " << length << "; ++i)" << endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyInt_FromLong(long(" << name << "[i])));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    {   const int *" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << endl;
        c << "        str += tmpStr;" << endl;
        c << "        for(int i = 0; i < " << length << "; ++i)" << endl;
        c << "        {" << endl;
        c << "            SNPRINTF(tmpStr, 1000, \"%d\", " << name << "[i]);" << endl;
        c << "            str += tmpStr;" << endl;
        c << "            if(i < " << length - 1 << ")" << endl;
        c << "            {" << endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << endl;
        c << "                str += tmpStr;" << endl;
        c << "            }" << endl;
        c << "        }" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << endl;
        c << "        str += tmpStr;" << endl;
        c << "    }" << endl;
    }

};


//
// -------------------------------- IntVector --------------------------------
//
class AttsGeneratorIntVector : public virtual IntVector , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorIntVector(const QString &n, const QString &l)
        : IntVector(n,l), PythonGeneratorField("intVector",n,l), Field("intVector",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    intVector  &vec = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "    PyObject   *tuple;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"O\", &tuple))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    if(PyTuple_Check(tuple))" << endl;
        c << "    {" << endl;
        c << "        vec.resize(PyTuple_Size(tuple));" << endl;
        c << "        for(int i = 0; i < PyTuple_Size(tuple); ++i)" << endl;
        c << "        {" << endl;
        c << "            PyObject *item = PyTuple_GET_ITEM(tuple, i);" << endl;
        c << "            if(PyFloat_Check(item))" << endl;
        c << "                vec[i] = int(PyFloat_AS_DOUBLE(item));" << endl;
        c << "            else if(PyInt_Check(item))" << endl;
        c << "                vec[i] = int(PyInt_AS_LONG(item));" << endl;
        c << "            else if(PyLong_Check(item))" << endl;
        c << "                vec[i] = int(PyLong_AsLong(item));" << endl;
        c << "            else" << endl;
        c << "                vec[i] = 0;" << endl;
        c << "        }" << endl;
        c << "    }" << endl;
        c << "    else if(PyFloat_Check(tuple))" << endl;
        c << "    {" << endl;
        c << "        vec.resize(1);" << endl;
        c << "        vec[0] = int(PyFloat_AS_DOUBLE(tuple));" << endl;
        c << "    }" << endl;
        c << "    else if(PyInt_Check(tuple))" << endl;
        c << "    {" << endl;
        c << "        vec.resize(1);" << endl;
        c << "        vec[0] = int(PyInt_AS_LONG(tuple));" << endl;
        c << "    }" << endl;
        c << "    else if(PyLong_Check(tuple))" << endl;
        c << "    {" << endl;
        c << "        vec.resize(1);" << endl;
        c << "        vec[0] = int(PyLong_AsLong(tuple));" << endl;
        c << "    }" << endl;
        c << "    else" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Mark the "<<name<<" in the object as modified." << endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << endl;
        else
            c << "    obj->data->Select"<<Name<<"();" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << endl;
        c << "    const intVector &" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << ";" << endl;
        else
            c << MethodNameGet() << "();" << endl;
        c << "    PyObject *retval = PyTuple_New(" << name << ".size());" << endl;
        c << "    for(int i = 0; i < "<<name<<".size(); ++i)" << endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyInt_FromLong(long(" << name << "[i])));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    {   const intVector &" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << endl;
        c << "        str += tmpStr;" << endl;
        c << "        for(int i = 0; i < " << name << ".size(); ++i)" << endl;
        c << "        {" << endl;
        c << "            SNPRINTF(tmpStr, 1000, \"%d\", " << name << "[i]);" << endl;
        c << "            str += tmpStr;" << endl;
        c << "            if(i < " << name << ".size() - 1)" << endl;
        c << "            {" << endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << endl;
        c << "                str += tmpStr;" << endl;
        c << "            }" << endl;
        c << "        }" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << endl;
        c << "        str += tmpStr;" << endl;
        c << "    }" << endl;
    }
};


// 
// ----------------------------------- Bool -----------------------------------
//
class AttsGeneratorBool : public virtual Bool , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorBool(const QString &n, const QString &l)
        : Bool(n,l), PythonGeneratorField("bool",n,l), Field("bool",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    int ival;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &ival))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Set the " << name << " in the object." << endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = ";
        else
            c << MethodNameSet();
        c << "(ival != 0);" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    PyObject *retval = PyInt_FromLong(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet()<<"()";
        c << "?1L:0L);" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    if(atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ")" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = 1\\n\", prefix);" << endl;
        c << "    else" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = 0\\n\", prefix);" << endl;
        c << "    str += tmpStr;" << endl;
    }
};


//
// ----------------------------------- Float ----------------------------------
//
class AttsGeneratorFloat : public virtual Float , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorFloat(const QString &n, const QString &l)
        : Float(n,l), PythonGeneratorField("float",n,l), Field("float",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    float fval;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"f\", &fval))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Set the " << name << " in the object." << endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = fval;" << endl;
        else
            c << MethodNameSet() << "(fval);" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    PyObject *retval = PyFloat_FromDouble(double(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << ";" << endl;
        else
            c <<MethodNameGet()<<"()";
        c << "));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = %g\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ");" << endl;
        c << "    str += tmpStr;" << endl;
    }
};


//
// -------------------------------- FloatArray -------------------------------
//
class AttsGeneratorFloatArray : public virtual FloatArray , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorFloatArray(const QString &s, const QString &n, const QString &l)
        : FloatArray(s,n,l), PythonGeneratorField("floatArray",n,l), Field("floatArray",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    float *fvals = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"";
        int i;
        for(i = 0; i < length; ++i)
            c << "f";
        c << "\", ";
        for(i = 0; i < length; ++i)
        {
            c << "&fvals[" << i << "]";
            if(i < length - 1)
                c << ", ";
        }
        c << "))" << endl;
        c << "    {" << endl;
        c << "        PyObject     *tuple;" << endl;
        c << "        if(!PyArg_ParseTuple(args, \"O\", &tuple))" << endl;
        c << "            return NULL;" << endl;
        c << endl;
        c << "        if(PyTuple_Check(tuple))" << endl;
        c << "        {" << endl;
        c << "            if(PyTuple_Size(tuple) != " << length << ")" << endl;
        c << "                return NULL;" << endl;
        c << endl;
        c << "            PyErr_Clear();" << endl;
        c << "            for(int i = 0; i < PyTuple_Size(tuple); ++i)" << endl;
        c << "            {" << endl;
        c << "                PyObject *item = PyTuple_GET_ITEM(tuple, i);" << endl;
        c << "                if(PyFloat_Check(item))" << endl;
        c << "                    fvals[i] = float(PyFloat_AS_DOUBLE(item));" << endl;
        c << "                else if(PyInt_Check(item))" << endl;
        c << "                    fvals[i] = float(PyInt_AS_LONG(item));" << endl;
        c << "                else if(PyLong_Check(item))" << endl;
        c << "                    fvals[i] = float(PyLong_AsDouble(item));" << endl;
        c << "                else" << endl;
        c << "                    fvals[i] = 0.;" << endl;
        c << "            }" << endl;
        c << "        }" << endl;
        c << "        else" << endl;
        c << "            return NULL;" << endl;
        c << "    }" << endl;
        c << endl;
        c << "    // Mark the " << name << " in the object as modified." << endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << endl;
        else
            c << "    obj->data->Select" << Name << "();" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << endl;
        c << "    PyObject *retval = PyTuple_New(" << length << ");" << endl;
        c << "    const float *" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << ";" << endl;
        else
            c << MethodNameGet() << "();" << endl;
        c << "    for(int i = 0; i < " << length << "; ++i)" << endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyFloat_FromDouble(double(" << name << "[i])));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    {   const float *" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << endl;
        c << "        str += tmpStr;" << endl;
        c << "        for(int i = 0; i < " << length << "; ++i)" << endl;
        c << "        {" << endl;
        c << "            SNPRINTF(tmpStr, 1000, \"%g\", " << name << "[i]);" << endl;
        c << "            str += tmpStr;" << endl;
        c << "            if(i < " << length - 1 << ")" << endl;
        c << "            {" << endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << endl;
        c << "                str += tmpStr;" << endl;
        c << "            }" << endl;
        c << "        }" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << endl;
        c << "        str += tmpStr;" << endl;
        c << "    }" << endl;
    }
};


//
// ---------------------------------- Double ----------------------------------
//
class AttsGeneratorDouble : public virtual Double , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorDouble(const QString &n, const QString &l)
        : Double(n,l), PythonGeneratorField("double",n,l), Field("double",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    double dval;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"d\", &dval))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Set the " << name << " in the object." << endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = dval;" << endl;
        else
            c << MethodNameSet() << "(dval);" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    PyObject *retval = PyFloat_FromDouble(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ");" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = %g\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ");" << endl;
        c << "    str += tmpStr;" << endl;
    }
};


//
// -------------------------------- DoubleArray -------------------------------
//
class AttsGeneratorDoubleArray : public virtual DoubleArray , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorDoubleArray(const QString &s, const QString &n, const QString &l)
        : DoubleArray(s,n,l), PythonGeneratorField("doubleArray",n,l), Field("doubleArray",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    double *dvals = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"";
        int i;
        for(i = 0; i < length; ++i)
            c << "d";
        c << "\", ";
        for(i = 0; i < length; ++i)
        {
            c << "&dvals[" << i << "]";
            if(i < length - 1)
                c << ", ";
        }
        c << "))" << endl;
        c << "    {" << endl;
        c << "        PyObject     *tuple;" << endl;
        c << "        if(!PyArg_ParseTuple(args, \"O\", &tuple))" << endl;
        c << "            return NULL;" << endl;
        c << endl;
        c << "        if(PyTuple_Check(tuple))" << endl;
        c << "        {" << endl;
        c << "            if(PyTuple_Size(tuple) != " << length << ")" << endl;
        c << "                return NULL;" << endl;
        c << endl;
        c << "            PyErr_Clear();" << endl;
        c << "            for(int i = 0; i < PyTuple_Size(tuple); ++i)" << endl;
        c << "            {" << endl;
        c << "                PyObject *item = PyTuple_GET_ITEM(tuple, i);" << endl;
        c << "                if(PyFloat_Check(item))" << endl;
        c << "                    dvals[i] = PyFloat_AS_DOUBLE(item);" << endl;
        c << "                else if(PyInt_Check(item))" << endl;
        c << "                    dvals[i] = double(PyInt_AS_LONG(item));" << endl;
        c << "                else if(PyLong_Check(item))" << endl;
        c << "                    dvals[i] = PyLong_AsDouble(item);" << endl;
        c << "                else" << endl;
        c << "                    dvals[i] = 0.;" << endl;
        c << "            }" << endl;
        c << "        }" << endl;
        c << "        else" << endl;
        c << "            return NULL;" << endl;
        c << "    }" << endl;
        c << endl;
        c << "    // Mark the " << name << " in the object as modified." << endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << endl;
        else
            c << "    obj->data->Select" << Name << "();" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << endl;
        c << "    PyObject *retval = PyTuple_New(" << length << ");" << endl;
        c << "    const double *" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ";" << endl;
        c << "    for(int i = 0; i < " << length << "; ++i)" << endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyFloat_FromDouble(" << name << "[i]));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    {   const double *" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << endl;
        c << "        str += tmpStr;" << endl;
        c << "        for(int i = 0; i < " << length << "; ++i)" << endl;
        c << "        {" << endl;
        c << "            SNPRINTF(tmpStr, 1000, \"%g\", " << name << "[i]);" << endl;
        c << "            str += tmpStr;" << endl;
        c << "            if(i < " << length - 1 << ")" << endl;
        c << "            {" << endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << endl;
        c << "                str += tmpStr;" << endl;
        c << "            }" << endl;
        c << "        }" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << endl;
        c << "        str += tmpStr;" << endl;
        c << "    }" << endl;
    }
};


//
// ------------------------------- DoubleVector -------------------------------
//
class AttsGeneratorDoubleVector : public virtual DoubleVector , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorDoubleVector(const QString &n, const QString &l)
        : DoubleVector(n,l), PythonGeneratorField("doubleVector",n,l), Field("doubleVector",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    doubleVector  &vec = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "    PyObject     *tuple;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"O\", &tuple))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    if(PyTuple_Check(tuple))" << endl;
        c << "    {" << endl;
        c << "        vec.resize(PyTuple_Size(tuple));" << endl;
        c << "        for(int i = 0; i < PyTuple_Size(tuple); ++i)" << endl;
        c << "        {" << endl;
        c << "            PyObject *item = PyTuple_GET_ITEM(tuple, i);" << endl;
        c << "            if(PyFloat_Check(item))" << endl;
        c << "                vec[i] = PyFloat_AS_DOUBLE(item);" << endl;
        c << "            else if(PyInt_Check(item))" << endl;
        c << "                vec[i] = double(PyInt_AS_LONG(item));" << endl;
        c << "            else if(PyLong_Check(item))" << endl;
        c << "                vec[i] = PyLong_AsDouble(item);" << endl;
        c << "            else" << endl;
        c << "                vec[i] = 0.;" << endl;
        c << "        }" << endl;
        c << "    }" << endl;
        c << "    else if(PyFloat_Check(tuple))" << endl;
        c << "    {" << endl;
        c << "        vec.resize(1);" << endl;
        c << "        vec[0] = PyFloat_AS_DOUBLE(tuple);" << endl;
        c << "    }" << endl;
        c << "    else if(PyInt_Check(tuple))" << endl;
        c << "    {" << endl;
        c << "        vec.resize(1);" << endl;
        c << "        vec[0] = double(PyInt_AS_LONG(tuple));" << endl;
        c << "    }" << endl;
        c << "    else if(PyLong_Check(tuple))" << endl;
        c << "    {" << endl;
        c << "        vec.resize(1);" << endl;
        c << "        vec[0] = PyLong_AsDouble(tuple);" << endl;
        c << "    }" << endl;
        c << "    else" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Mark the "<<name<<" in the object as modified." << endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << endl;
        else
            c << "    obj->data->Select"<<Name<<"();" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << endl;
        c << "    const doubleVector &" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ";" << endl;
        c << "    PyObject *retval = PyTuple_New(" << name << ".size());" << endl;
        c << "    for(int i = 0; i < "<<name<<".size(); ++i)" << endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyFloat_FromDouble(" << name << "[i]));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    {   const doubleVector &" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << endl;
        c << "        str += tmpStr;" << endl;
        c << "        for(int i = 0; i < " << name << ".size(); ++i)" << endl;
        c << "        {" << endl;
        c << "            SNPRINTF(tmpStr, 1000, \"%g\", " << name << "[i]);" << endl;
        c << "            str += tmpStr;" << endl;
        c << "            if(i < " << name << ".size() - 1)" << endl;
        c << "            {" << endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << endl;
        c << "                str += tmpStr;" << endl;
        c << "            }" << endl;
        c << "        }" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << endl;
        c << "        str += tmpStr;" << endl;
        c << "    }" << endl;
    }
};


//
// ----------------------------------- UChar ----------------------------------
//
class AttsGeneratorUChar : public virtual UChar , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorUChar(const QString &n, const QString &l)
        : UChar(n,l), PythonGeneratorField("uchar",n,l), Field("uchar",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    unsigned char uval;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"c\", &uval))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Set the " << name << " in the object." << endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = uval;" << endl;
        else
            c << MethodNameSet() << "(uval);" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    PyObject *retval = PyInt_FromLong(long(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c <<"));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = %d\\n\", prefix, int(atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << "));" << endl;
        c << "    str += tmpStr;" << endl;
    }
};


//
// -------------------------------- UCharArray --------------------------------
//
class AttsGeneratorUCharArray : public virtual UCharArray , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorUCharArray(const QString &s, const QString &n, const QString &l)
        : UCharArray(s,n,l), PythonGeneratorField("ucharArray",n,l), Field("ucharArray",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    unsigned char *cvals = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"";
        int i;
        for(i = 0; i < length; ++i)
            c << "c";
        c << "\", ";
        for(i = 0; i < length; ++i)
        {
            c << "&cvals[" << i << "]";
            if(i < length - 1)
                c << ", ";
        }
        c << "))" << endl;
        c << "    {" << endl;
        c << "        PyObject     *tuple;" << endl;
        c << "        if(!PyArg_ParseTuple(args, \"O\", &tuple))" << endl;
        c << "            return NULL;" << endl;
        c << endl;
        c << "        if(PyTuple_Check(tuple))" << endl;
        c << "        {" << endl;
        c << "            if(PyTuple_Size(tuple) != " << length << ")" << endl;
        c << "                return NULL;" << endl;
        c << endl;
        c << "            PyErr_Clear();" << endl;
        c << "            for(int i = 0; i < PyTuple_Size(tuple); ++i)" << endl;
        c << "            {" << endl;
        c << "                int c;" << endl;
        c << "                PyObject *item = PyTuple_GET_ITEM(tuple, i);" << endl;
        c << "                if(PyFloat_Check(item))" << endl;
        c << "                    c = int(PyFloat_AS_DOUBLE(item));" << endl;
        c << "                else if(PyInt_Check(item))" << endl;
        c << "                    c = int(PyInt_AS_LONG(item));" << endl;
        c << "                else if(PyLong_Check(item))" << endl;
        c << "                    c = int(PyLong_AsDouble(item));" << endl;
        c << "                else" << endl;
        c << "                    c = 0;" << endl;
        c << endl;
        c << "                if(c < 0) c = 0;" << endl;
        c << "                if(c > 255) c = 255;" << endl;
        c << "                cvals[i] = (unsigned char)(c);" << endl;
        c << "            }" << endl;
        c << "        }" << endl;
        c << "        else" << endl;
        c << "            return NULL;" << endl;
        c << "    }" << endl;
        c << endl;
        c << "    // Mark the " << name << " in the object as modified." << endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << endl;
        else
            c << "    obj->data->Select" << Name << "();" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << endl;
        c << "    PyObject *retval = PyTuple_New(" << length << ");" << endl;
        c << "    const unsigned char *" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ";" << endl;
        c << "    for(int i = 0; i < " << length << "; ++i)" << endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyInt_FromLong(long(" << name << "[i])));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    {   const unsigned char *" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << endl;
        c << "        str += tmpStr;" << endl;
        c << "        for(int i = 0; i < " << length << "; ++i)" << endl;
        c << "        {" << endl;
        c << "            SNPRINTF(tmpStr, 1000, \"%d\", int(" << name << "[i]));" << endl;
        c << "            str += tmpStr;" << endl;
        c << "            if(i < " << length - 1 << ")" << endl;
        c << "            {" << endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << endl;
        c << "                str += tmpStr;" << endl;
        c << "            }" << endl;
        c << "        }" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << endl;
        c << "        str += tmpStr;" << endl;
        c << "    }" << endl;
    }
};


//
// ------------------------------- UCharVector -------------------------------
//
class AttsGeneratorUCharVector : public virtual UCharVector , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorUCharVector(const QString &n, const QString &l)
        : UCharVector(n,l), PythonGeneratorField("ucharVector",n,l), Field("ucharVector",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    unsignedCharVector  &vec = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "    PyObject     *tuple;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"O\", &tuple))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    if(PyTuple_Check(tuple))" << endl;
        c << "    {" << endl;
        c << "        vec.resize(PyTuple_Size(tuple));" << endl;
        c << "        for(int i = 0; i < PyTuple_Size(tuple); ++i)" << endl;
        c << "        {" << endl;
        c << "            int c;" << endl;
        c << "            PyObject *item = PyTuple_GET_ITEM(tuple, i);" << endl;
        c << "            if(PyFloat_Check(item))" << endl;
        c << "                c = int(PyFloat_AS_DOUBLE(item));" << endl;
        c << "            else if(PyInt_Check(item))" << endl;
        c << "                c = int(PyInt_AS_LONG(item));" << endl;
        c << "            else if(PyLong_Check(item))" << endl;
        c << "                c = int(PyLong_AsDouble(item));" << endl;
        c << "            else" << endl;
        c << "                c = 0;" << endl;
        c << endl;
        c << "            if(c < 0) c = 0;" << endl;
        c << "            if(c > 255) c = 255;" << endl;
        c << "            vec[i] = (unsigned char)(c);" << endl;
        c << "        }" << endl;
        c << "    }" << endl;
        c << "    else if(PyFloat_Check(tuple))" << endl;
        c << "    {" << endl;
        c << "        vec.resize(1);" << endl;
        c << "        int c = int(PyFloat_AS_DOUBLE(tuple));" << endl;
        c << "        if(c < 0) c = 0;" << endl;
        c << "        if(c > 255) c = 255;" << endl;
        c << "        vec[0] = (unsigned char)(c);" << endl;
        c << "    }" << endl;
        c << "    else if(PyInt_Check(tuple))" << endl;
        c << "    {" << endl;
        c << "        vec.resize(1);" << endl;
        c << "        int c = int(PyInt_AS_LONG(tuple));" << endl;
        c << "        if(c < 0) c = 0;" << endl;
        c << "        if(c > 255) c = 255;" << endl;
        c << "        vec[0] = (unsigned char)(c);" << endl;
        c << "    }" << endl;
        c << "    else if(PyLong_Check(tuple))" << endl;
        c << "    {" << endl;
        c << "        vec.resize(1);" << endl;
        c << "        int c = PyLong_AsLong(tuple);" << endl;
        c << "        if(c < 0) c = 0;" << endl;
        c << "        if(c > 255) c = 255;" << endl;
        c << "        vec[0] = (unsigned char)(c);" << endl;
        c << "    }" << endl;
        c << "    else" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Mark the "<<name<<" in the object as modified." << endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << endl;
        else
            c << "    obj->data->Select"<<Name<<"();" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << endl;
        c << "    const unsignedCharVector &" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ";" << endl;
        c << "    PyObject *retval = PyTuple_New(" << name << ".size());" << endl;
        c << "    for(int i = 0; i < "<<name<<".size(); ++i)" << endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyInt_FromLong(long(" << name << "[i])));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    {   const unsignedCharVector &" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << endl;
        c << "        str += tmpStr;" << endl;
        c << "        for(int i = 0; i < " << name << ".size(); ++i)" << endl;
        c << "        {" << endl;
        c << "            SNPRINTF(tmpStr, 1000, \"%d\", int(" << name << "[i]));" << endl;
        c << "            str += tmpStr;" << endl;
        c << "            if(i < " << name << ".size() - 1)" << endl;
        c << "            {" << endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << endl;
        c << "                str += tmpStr;" << endl;
        c << "            }" << endl;
        c << "        }" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << endl;
        c << "        str += tmpStr;" << endl;
        c << "    }" << endl;
    }
};

//
// ---------------------------------- String ----------------------------------
//
class AttsGeneratorString : public virtual String , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorString(const QString &n, const QString &l)
        : String(n,l), PythonGeneratorField("string",n,l), Field("string",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    char *str;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"s\", &str))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Set the " << name << " in the object." << endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = std::string(str);" << endl;
        else
            c << MethodNameSet() << "(std::string(str));" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    PyObject *retval = PyString_FromString(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ".c_str());" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = \\\"%s\\\"\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ".c_str());" << endl;
        c << "    str += tmpStr;" << endl;
    }
};


//
// ------------------------------- StringVector -------------------------------
//
class AttsGeneratorStringVector : public virtual StringVector , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorStringVector(const QString &n, const QString &l)
        : StringVector(n,l), PythonGeneratorField("stringVector",n,l), Field("stringVector",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    stringVector  &vec = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "    PyObject     *tuple;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"O\", &tuple))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    if(PyTuple_Check(tuple))" << endl;
        c << "    {" << endl;
        c << "        vec.resize(PyTuple_Size(tuple));" << endl;
        c << "        for(int i = 0; i < PyTuple_Size(tuple); ++i)" << endl;
        c << "        {" << endl;
        c << "            PyObject *item = PyTuple_GET_ITEM(tuple, i);" << endl;
        c << "            if(PyString_Check(item))" << endl;
        c << "                vec[i] = std::string(PyString_AS_STRING(item));" << endl;
        c << "            else" << endl;
        c << "                vec[i] = std::string(\"\");" << endl;
        c << "        }" << endl;
        c << "    }" << endl;
        c << "    else if(PyString_Check(tuple))" << endl;
        c << "    {" << endl;
        c << "        vec.resize(1);" << endl;
        c << "        vec[0] = std::string(PyString_AS_STRING(tuple));" << endl;
        c << "    }" << endl;
        c << "    else" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Mark the "<<name<<" in the object as modified." << endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << endl;
        else
            c << "    obj->data->Select"<<Name<<"();" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << endl;
        c << "    const stringVector &" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ";" << endl;
        c << "    PyObject *retval = PyTuple_New(" << name << ".size());" << endl;
        c << "    for(int i = 0; i < "<<name<<".size(); ++i)" << endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyString_FromString(" << name << "[i].c_str()));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    {   const stringVector &" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << endl;
        c << "        str += tmpStr;" << endl;
        c << "        for(int i = 0; i < " << name << ".size(); ++i)" << endl;
        c << "        {" << endl;
        c << "            SNPRINTF(tmpStr, 1000, \"\\\"%s\\\"\", " << name << "[i].c_str());" << endl;
        c << "            str += tmpStr;" << endl;
        c << "            if(i < " << name << ".size() - 1)" << endl;
        c << "            {" << endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << endl;
        c << "                str += tmpStr;" << endl;
        c << "            }" << endl;
        c << "        }" << endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << endl;
        c << "        str += tmpStr;" << endl;
        c << "    }" << endl;
    }
};


//
// -------------------------------- ColorTable --------------------------------
//
class AttsGeneratorColorTable : public virtual ColorTable , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorColorTable(const QString &n, const QString &l)
        : ColorTable(n,l), PythonGeneratorField("colortable",n,l), Field("colortable",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    char *str;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"s\", &str))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Set the " << name << " in the object." << endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = std::string(str);" << endl;
        else
            c << MethodNameSet() << "(std::string(str));" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    PyObject *retval = PyString_FromString(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c <<".c_str());" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = \\\"%s\\\"\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ".c_str());" << endl;
        c << "    str += tmpStr;" << endl;
    }
};


//
// ----------------------------------- Color ----------------------------------
//
class AttsGeneratorColor : public virtual Color , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorColor(const QString &n, const QString &l)
        : Color(n,l), PythonGeneratorField("color",n,l), Field("color",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    int c[4];" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"iiii\", &c[0], &c[1], &c[2], &c[3]))" << endl;
        c << "    {" << endl;
        c << "        c[3] = 255;" << endl;
        c << "        if(!PyArg_ParseTuple(args, \"iii\", &c[0], &c[1], &c[2]))" << endl;
        c << "        {" << endl;
        c << "            double dr, dg, db, da;" << endl;
        c << "            if(PyArg_ParseTuple(args, \"dddd\", &dr, &dg, &db, &da))" << endl;
        c << "            {" << endl;
        c << "                c[0] = int(dr);" << endl;
        c << "                c[1] = int(dg);" << endl;
        c << "                c[2] = int(db);" << endl;
        c << "                c[3] = int(da);" << endl;
        c << "            }" << endl;
        c << "            else if(PyArg_ParseTuple(args, \"ddd\", &dr, &dg, &db))" << endl;
        c << "            {" << endl;
        c << "                c[0] = int(dr);" << endl;
        c << "                c[1] = int(dg);" << endl;
        c << "                c[2] = int(db);" << endl;
        c << "                c[3] = 255;" << endl;
        c << "            }" << endl;
        c << "            else" << endl;
        c << "            {" << endl;
        c << "                PyObject *tuple = NULL;" << endl;
        c << "                if(!PyArg_ParseTuple(args, \"O\", &tuple))" << endl;
        c << "                    return NULL;" << endl;
        c << endl;
        c << "                if(!PyTuple_Check(tuple))" << endl;
        c << "                    return NULL;" << endl;
        c << endl;
        c << "                // Make sure that the tuple is the right size." << endl;
        c << "                if(PyTuple_Size(tuple) < 3 || PyTuple_Size(tuple) > 4)" << endl;
        c << "                    return NULL;" << endl;
        c << endl;
        c << "                // Make sure that all elements in the tuple are ints." << endl;
        c << "                for(int i = 0; i < PyTuple_Size(tuple); ++i)" << endl;
        c << "                {" << endl;
        c << "                    PyObject *item = PyTuple_GET_ITEM(tuple, i);" << endl;
        c << "                    if(PyInt_Check(item))" << endl;
        c << "                        c[i] = int(PyInt_AS_LONG(PyTuple_GET_ITEM(tuple, i)));" << endl;
        c << "                    else if(PyFloat_Check(item))" << endl;
        c << "                        c[i] = int(PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(tuple, i)));" << endl;
        c << "                    else" << endl;
        c << "                        return NULL;" << endl;
        c << "                }" << endl;
        c << "            }" << endl;
        c << "        }" << endl;
        c << "        PyErr_Clear();" << endl;
        c << "    }" << endl;
        c << endl;
        c << "    // Set the " << name << " in the object." << endl;
        c << "    ColorAttribute ca(c[0], c[1], c[2], c[3]);" << endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = ca;" << endl;
        else
            c << MethodNameSet() << "(ca);" << endl;
    }

    virtual void WriteIncludedHeaders(ostream &c)
    {
        // Write the list of include files that the object needs.
        c << "#include <ColorAttribute.h>" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << endl;
        c << "    PyObject *retval = PyTuple_New(4);" << endl;
        c << "    const unsigned char *" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ".GetColor();" << endl;
        c << "    PyTuple_SET_ITEM(retval, 0, PyInt_FromLong(long(" << name << "[0])));" << endl;
        c << "    PyTuple_SET_ITEM(retval, 1, PyInt_FromLong(long(" << name << "[1])));" << endl;
        c << "    PyTuple_SET_ITEM(retval, 2, PyInt_FromLong(long(" << name << "[2])));" << endl;
        c << "    PyTuple_SET_ITEM(retval, 3, PyInt_FromLong(long(" << name << "[3])));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    const unsigned char *" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ".GetColor();" << endl;
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = (%d, %d, %d, %d)\\n\", prefix, int("<<name<<"[0]), int("<<name<<"[1]), int("<<name<<"[2]), int("<<name<<"[3]));" << endl;
        c << "    str += tmpStr;" << endl;
    }
};


//
// --------------------------------- LineStyle --------------------------------
//
class AttsGeneratorLineStyle : public virtual LineStyle , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorLineStyle(const QString &n, const QString &l)
        : LineStyle(n,l), PythonGeneratorField("linestyle",n,l), Field("linestyle",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    int ival;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &ival))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Set the " << name << " in the object." << endl;
        c << "    if(ival >= 0 && ival <= 3)" << endl;
        c << "        obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = ival;" << endl;
        else
            c << MethodNameSet() << "(ival);" << endl;
        c << "    else" << endl;
        c << "    {" << endl;
        c << "        fprintf(stderr, \"An invalid  value was given. \"" << endl;
        c << "                        \"Valid values are in the range of [0,3]. \"" << endl;
        c << "                        \"You can also use the following names: \"" << endl;
        c << "                        \"\\\"SOLID\\\", \\\"DASH\\\", \\\"DOT\\\", \\\"DOTDASH\\\"\\n\");" << endl;
        c << "        return NULL;" << endl;
        c << "    }" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    PyObject *retval = PyInt_FromLong(long(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c <<"));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    const char *" << name << "_values[] = {\"SOLID\", \"DASH\", \"DOT\", \"DOTDASH\"};" << endl;
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = ";
        c << "%s%s  # SOLID, DASH, DOT, DOTDASH\\n\", prefix, prefix, " << name << "_values[atts->";

        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << "]);" << endl;
        c << "    str += tmpStr;" << endl;
    }

    virtual void WriteGetAttr(ostream &c, const QString &classname)
    {
        if (internal)
            return;

        c << "    if(strcmp(name, \"" << name << "\") == 0)" << endl;
        c << "        return " << classname << "_" << MethodNameGet() << "(self, NULL);" << endl;
        c << "    if(strcmp(name, \"SOLID\") == 0)" << endl;
        c << "        return PyInt_FromLong(long(0));" << endl;
        c << "    else if(strcmp(name, \"DASH\") == 0)" << endl;
        c << "        return PyInt_FromLong(long(1));" << endl;
        c << "    else if(strcmp(name, \"DOT\") == 0)" << endl;
        c << "        return PyInt_FromLong(long(2));" << endl;
        c << "    else if(strcmp(name, \"DOTDASH\") == 0)" << endl;
        c << "        return PyInt_FromLong(long(3));" << endl;
        c << endl;
    }
};


//
// --------------------------------- LineWidth --------------------------------
//
class AttsGeneratorLineWidth : public virtual LineWidth , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorLineWidth(const QString &n, const QString &l)
        : LineWidth(n,l), PythonGeneratorField("linewidth",n,l), Field("linewidth",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    int ival;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &ival))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Set the " << name << " in the object." << endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = ival;" << endl;
        else
            c << MethodNameSet() << "(ival);" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    PyObject *retval = PyInt_FromLong(long(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c <<"));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = %d\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ");" << endl;
        c << "    str += tmpStr;" << endl;
    }
};


//
// --------------------------------- Opacity ----------------------------------
//
class AttsGeneratorOpacity : public virtual Opacity , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorOpacity(const QString &n, const QString &l)
        : Opacity(n,l), PythonGeneratorField("opacity",n,l), Field("opacity",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    double dval;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"d\", &dval))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Set the " << name << " in the object." << endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = dval;" << endl;
        else
            c << MethodNameSet() << "(dval);" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    PyObject *retval = PyFloat_FromDouble(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c <<");" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = %g\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ");" << endl;
        c << "    str += tmpStr;" << endl;
    }
};


//
// -------------------------------- VariableName --------------------------------
//
class AttsGeneratorVariableName : public virtual VariableName , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorVariableName(const QString &n, const QString &l)
        : VariableName(n,l), PythonGeneratorField("variablename",n,l), Field("variablename",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    char *str;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"s\", &str))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Set the " << name << " in the object." << endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = std::string(str);" << endl;
        else
            c << MethodNameSet() << "(std::string(str));" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    PyObject *retval = PyString_FromString(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c<<".c_str());" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = \\\"%s\\\"\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ".c_str());" << endl;
        c << "    str += tmpStr;" << endl;
    }
};


//
// ------------------------------------ Att -----------------------------------
//
// Modifications:
//
//   Hank Childs, Wed Dec 14 10:42:09 PST 2005
//   Added print statement so that users will be redirected to get/set methods.
//
// ----------------------------------------------------------------------------

class AttsGeneratorAtt : public virtual Att , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorAtt(const QString &t, const QString &n, const QString &l)
        : Att(t,n,l), PythonGeneratorField("att",n,l), Field("att",n,l) { cerr << "**** AttType=" << AttType << endl;}

    virtual void WriteIncludedHeaders(ostream &c)
    {
        // Write the list of include files that the object needs.
        c << "#include <Py" << attType << ".h>" << endl;
    }

    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        if(AttType == "ColorAttributeList")
        {
            c << "    PyObject *pyobj = NULL;" << endl;
            c << "    ColorAttributeList &cL = obj->data->";
            if(accessType == Field::AccessPublic)
                c << name;
            else
                c << MethodNameGet() << "()";
            c << ";" << endl;
            c << "    int index = 0;" << endl;
            c << "    int c[4] = {0,0,0,255};" << endl;
            c << "    bool setTheColor = true;" << endl;
            c << endl;
            c << "    if(!PyArg_ParseTuple(args, \"iiiii\", &index, &c[0], &c[1], &c[2], &c[3]))" << endl;
            c << "    {" << endl;
            c << "        if(!PyArg_ParseTuple(args, \"iiii\", &index, &c[0], &c[1], &c[2]))" << endl;
            c << "        {" << endl;
            c << "            double dr, dg, db, da;" << endl;
            c << "            if(PyArg_ParseTuple(args, \"idddd\", &index, &dr, &dg, &db, &da))" << endl;
            c << "            {" << endl;
            c << "                c[0] = int(dr);" << endl;
            c << "                c[1] = int(dg);" << endl;
            c << "                c[2] = int(db);" << endl;
            c << "                c[3] = int(da);" << endl;
            c << "            }" << endl;
            c << "            else if(PyArg_ParseTuple(args, \"iddd\", &index, &dr, &dg, &db))" << endl;
            c << "            {" << endl;
            c << "                c[0] = int(dr);" << endl;
            c << "                c[1] = int(dg);" << endl;
            c << "                c[2] = int(db);" << endl;
            c << "                c[3] = 255;" << endl;
            c << "            }" << endl;
            c << "            else" << endl;
            c << "            {" << endl;
            c << "                if(!PyArg_ParseTuple(args, \"iO\", &index, &pyobj))" << endl;
            c << "                {" << endl;
            c << "                    if(PyArg_ParseTuple(args, \"O\", &pyobj))" << endl;
            c << "                    {" << endl;
            c << "                        setTheColor = false;" << endl;
            c << "                        if(PyTuple_Check(pyobj))" << endl;
            c << "                        {" << endl;
            c << "                            // Make sure that the tuple is the right size." << endl;
            c << "                            if(PyTuple_Size(pyobj) < cL.GetNumColors())" << endl;
            c << "                                return NULL;" << endl;
            c << endl;
            c << "                            // Make sure that the tuple is the right size." << endl;
            c << "                            bool badInput = false;" << endl;
            c << "                            int i, *C = new int[4 * cL.GetNumColors()];" << endl;
            c << "                            for(i = 0; i < PyTuple_Size(pyobj) && !badInput; ++i)" << endl;
            c << "                            {" << endl;
            c << "                                PyObject *item = PyTuple_GET_ITEM(pyobj, i);" << endl;
            c << "                                if(PyTuple_Check(item) &&" << endl;
            c << "                                   PyTuple_Size(item) == 3 || PyTuple_Size(item) == 4)" << endl;
            c << "                                {" << endl;
            c << "                                    C[i*4] = 0;" << endl;
            c << "                                    C[i*4+1] = 0;" << endl;
            c << "                                    C[i*4+2] = 0;" << endl;
            c << "                                    C[i*4+3] = 255;" << endl;
            c << "                                    for(int j = 0; j < PyTuple_Size(item) && !badInput; ++j)" << endl;
            c << "                                    {" << endl;
            c << "                                        PyObject *colorcomp = PyTuple_GET_ITEM(item, j);" << endl;
            c << "                                        if(PyInt_Check(colorcomp))" << endl;
            c << "                                           C[i*4+j] = int(PyInt_AS_LONG(colorcomp));" << endl;
            c << "                                        else if(PyFloat_Check(colorcomp))" << endl;
            c << "                                           C[i*4+j] = int(PyFloat_AS_DOUBLE(colorcomp));" << endl;
            c << "                                        else" << endl;
            c << "                                           badInput = true;" << endl;
            c << "                                    }" << endl;
            c << "                                }" << endl;
            c << "                                else" << endl;
            c << "                                    badInput = true;" << endl;
            c << "                            }" << endl;
            c << endl;
            c << "                            if(badInput)" << endl;
            c << "                            {" << endl;
            c << "                                delete [] C;" << endl;
            c << "                                return NULL;" << endl;
            c << "                            }" << endl;
            c << endl;
            c << "                            for(i = 0; i < cL.GetNumColors(); ++i)" << endl;
            c << "                                cL[i].SetRgba(C[i*4], C[i*4+1], C[i*4+2], C[i*4+3]);" << endl;
            c << "                            delete [] C;" << endl;
            c << "                        }" << endl;
            c << "                        else if(PyList_Check(pyobj))" << endl;
            c << "                        {" << endl;
            c << "                            // Make sure that the list is the right size." << endl;
            c << "                            if(PyList_Size(pyobj) < cL.GetNumColors())" << endl;
            c << "                                return NULL;" << endl;
            c << endl;
            c << "                            // Make sure that the tuple is the right size." << endl;
            c << "                            bool badInput = false;" << endl;
            c << "                            int i, *C = new int[4 * cL.GetNumColors()];" << endl;
            c << "                            for(i = 0; i < PyList_Size(pyobj) && !badInput; ++i)" << endl;
            c << "                            {" << endl;
            c << "                                PyObject *item = PyList_GET_ITEM(pyobj, i);" << endl;
            c << "                                if(PyTuple_Check(item) &&" << endl;
            c << "                                   PyTuple_Size(item) == 3 || PyTuple_Size(item) == 4)" << endl;
            c << "                                {" << endl;
            c << "                                    C[i*4] = 0;" << endl;
            c << "                                    C[i*4+1] = 0;" << endl;
            c << "                                    C[i*4+2] = 0;" << endl;
            c << "                                    C[i*4+3] = 255;" << endl;
            c << "                                    for(int j = 0; j < PyTuple_Size(item) && !badInput; ++j)" << endl;
            c << "                                    {" << endl;
            c << "                                        PyObject *colorcomp = PyTuple_GET_ITEM(item, j);" << endl;
            c << "                                        if(PyInt_Check(colorcomp))" << endl;
            c << "                                           C[i*4+j] = int(PyInt_AS_LONG(colorcomp));" << endl;
            c << "                                        else if(PyFloat_Check(colorcomp))" << endl;
            c << "                                           C[i*4+j] = int(PyFloat_AS_DOUBLE(colorcomp));" << endl;
            c << "                                        else" << endl;
            c << "                                           badInput = true;" << endl;
            c << "                                    }" << endl;
            c << "                                }" << endl;
            c << "                                else" << endl;
            c << "                                    badInput = true;" << endl;
            c << "                            }" << endl;
            c << endl;
            c << "                            if(badInput)" << endl;
            c << "                            {" << endl;
            c << "                                delete [] C;" << endl;
            c << "                                return NULL;" << endl;
            c << "                            }" << endl;
            c << endl;
            c << "                            for(i = 0; i < cL.GetNumColors(); ++i)" << endl;
            c << "                                cL[i].SetRgba(C[i*4], C[i*4+1], C[i*4+2], C[i*4+3]);" << endl;
            c << endl;
            c << "                            delete [] C;" << endl;
            c << "                        }" << endl;
            c << "                        else" << endl;
            c << "                            return NULL;" << endl;
            c << "                    }" << endl;
            c << "                }" << endl;
            c << "                else" << endl;
            c << "                {" << endl;
            c << "                    if(!PyTuple_Check(pyobj))" << endl;
            c << "                        return NULL;" << endl;
            c << endl;
            c << "                    // Make sure that the tuple is the right size." << endl;
            c << "                    if(PyTuple_Size(pyobj) < 3 || PyTuple_Size(pyobj) > 4)" << endl;
            c << "                        return NULL;" << endl;
            c << endl;
            c << "                    // Make sure that all elements in the tuple are ints." << endl;
            c << "                    for(int i = 0; i < PyTuple_Size(pyobj); ++i)" << endl;
            c << "                    {" << endl;
            c << "                        PyObject *item = PyTuple_GET_ITEM(pyobj, i);" << endl;
            c << "                        if(PyInt_Check(item))" << endl;
            c << "                            c[i] = int(PyInt_AS_LONG(PyTuple_GET_ITEM(pyobj, i)));" << endl;
            c << "                        else if(PyFloat_Check(item))" << endl;
            c << "                            c[i] = int(PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(pyobj, i)));" << endl;
            c << "                        else" << endl;
            c << "                            return NULL;" << endl;
            c << "                    }" << endl;
            c << "                }" << endl;
            c << "            }" << endl;
            c << "        }" << endl;
            c << "        PyErr_Clear();" << endl;
            c << "    }" << endl;
            c << endl;
            c << "    if(index < 0 || index >= cL.GetNumColors())" << endl;
            c << "        return NULL;" << endl;
            c << endl;
            c << "    // Set the color in the object." << endl;
            c << "    if(setTheColor)" << endl;
            c << "        cL[index] = ColorAttribute(c[0], c[1], c[2], c[3]);" << endl;
            c << "    cL.SelectColors();" << endl;
            c << "    obj->data->Select" << Name << "();" << endl;
        }
        else
        {
            c << "    PyObject *newValue = NULL;" << endl;
            c << "    if(!PyArg_ParseTuple(args, \"O\", &newValue))" << endl;
            c << "        return NULL;" << endl;
            c << "    if(!Py" << AttType << "_Check(newValue))" << endl;
            c << "    {" << endl;
            c << "        VisItErrorFunc(\"The " << name << " field can only be set with " << AttType << " objects.\");" << endl;
            c << "        return NULL;" << endl;
            c << "    }" << endl;
            c << endl;
            c << "    obj->data->Set" << Name << "(*Py" << AttType << "_FromPyObject(newValue));" << endl;
        }
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        if(AttType == "ColorAttributeList")
        {
            c << "    PyObject *retval = NULL;" << endl;
            c << "    ColorAttributeList &cL = obj->data->Get" << Name << "();" << endl;
            c << endl;
            c << "    int index = 0;" << endl;
            c << "    if(PyArg_ParseTuple(args, \"i\", &index))" << endl;
            c << "    {" << endl;
            c << "        if(index < 0 || index >= cL.GetNumColors())" << endl;
            c << "            return NULL;" << endl;
            c << endl;
            c << "        // Allocate a tuple the with enough entries to hold the singleColor." << endl;
            c << "        retval = PyTuple_New(4);" << endl;
            c << "        const unsigned char *c = cL.GetColors(index).GetColor();" << endl;
            c << "        PyTuple_SET_ITEM(retval, 0, PyInt_FromLong(long(c[0])));" << endl;
            c << "        PyTuple_SET_ITEM(retval, 1, PyInt_FromLong(long(c[1])));" << endl;
            c << "        PyTuple_SET_ITEM(retval, 2, PyInt_FromLong(long(c[2])));" << endl;
            c << "        PyTuple_SET_ITEM(retval, 3, PyInt_FromLong(long(c[3])));" << endl;
            c << "    }" << endl;
            c << "    else" << endl;
            c << "    {" << endl;
            c << "        PyErr_Clear();" << endl;
            c << endl;
            c << "        // Return the whole thing." << endl;
            c << "        retval = PyList_New(cL.GetNumColors());" << endl;
            c << "        for(int i = 0; i < cL.GetNumColors(); ++i)" << endl;
            c << "        {" << endl;
            c << "            const unsigned char *c = cL.GetColors(i).GetColor();" << endl;
            c << endl;
            c << "            PyObject *t = PyTuple_New(4);" << endl;
            c << "            PyTuple_SET_ITEM(t, 0, PyInt_FromLong(long(c[0])));" << endl;
            c << "            PyTuple_SET_ITEM(t, 1, PyInt_FromLong(long(c[1])));" << endl;
            c << "            PyTuple_SET_ITEM(t, 2, PyInt_FromLong(long(c[2])));" << endl;
            c << "            PyTuple_SET_ITEM(t, 3, PyInt_FromLong(long(c[3])));" << endl;
            c << endl;
            c << "            PyList_SET_ITEM(retval, i, t);" << endl;
            c << "        }" << endl;
            c << "    }" << endl;
        }
        else
        {
            c << "    // Since the new object will point to data owned by this object," << endl;
            c << "    // we need to increment the reference count." << endl;
            c << "    Py_INCREF(self);" << endl;
            c << endl;
            c << "    PyObject *retval = Py" << attType << "_Wrap(";
            if(accessType == Field::AccessPublic)
                c << "&obj->data->" << name << ");" << endl;
            else
                c << "&obj->data->Get" << Name << "());" << endl;
            c << "    // Set the object's parent so the reference to the parent can be decref'd" << endl;
            c << "    // when the child goes out of scope." << endl;
            c << "    Py" << attType << "_SetParent(retval, self);" << endl;
            c << endl;
        }
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        if(AttType == "ColorAttributeList")
        {
            c << "    { const ColorAttributeList &cL = atts->";
            if(accessType == Field::AccessPublic)
                c << name;
            else
                c << MethodNameGet() << "()";
            c << ";" << endl;
            c << "        const char *comment = (prefix==0 || strcmp(prefix,\"\")==0) ? \"# \" : \"\";" << endl;
            c << "        for(int i = 0; i < cL.GetNumColors(); ++i)" << endl;
            c << "        {" << endl;
            c << "            const unsigned char *c = cL[i].GetColor();" << endl;
            c << "            SNPRINTF(tmpStr, 1000, \"%s%sSet" << Name << "(%d, (%d, %d, %d, %d))\\n\"," << endl;
            c << "                     comment, prefix, i, int(c[0]), int(c[1]), int(c[2]), int(c[3]));" << endl;
            c << "            str += tmpStr;" << endl;
            c << "        }" << endl;
            c << "    }" << endl;
        }
        else
        {
            c << "    { // new scope" << endl;
            c << "        std::string objPrefix(prefix);" << endl;
            c << "        objPrefix += \"" << name << ".\";" << endl;
            c << "        str += Py" << attType << "_ToString(";
            if(accessType == Field::AccessPublic)
                c << "atts->" << name;
            else
                c << "&atts->" << MethodNameGet() << "()";
            c << ", objPrefix.c_str());" << endl;
            c << "    }" << endl;
        }
    }
};


//
// --------------------------------- AttVector --------------------------------
//
class AttsGeneratorAttVector : public virtual AttVector , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorAttVector(const QString &t, const QString &n, const QString &l)
        : AttVector(t,n,l), PythonGeneratorField("attVector",n,l), Field("attVector",n,l) { }

    virtual void WriteSetAttr(ostream &c, const QString &className, bool first)
    {
        // Not implemented yet!
    }

    virtual bool HasSetAttr()
    {
        return false;
    }

    virtual void WriteIncludedHeaders(ostream &c)
    {
        // Write the list of include files that the object needs.
        c << "#include <Py" << attType << ".h>" << endl;
    }

    // Do not allow set methods. Make the user use the Get### method to return a reference
    // to the object.
    virtual bool ProvidesSetMethod() const { return false; }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    int index;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &index))" << endl;
        c << "        return NULL;" << endl;
        c << "    if(index < 0 || index >= obj->data->Get" << Name << "().size())" << endl;
        c << "    {" << endl;
        c << "        char msg[200];" << endl;
        c << "        if(obj->data->Get" << Name << "().size() == 0)" << endl;
        c << "            SNPRINTF(msg, 200, \"The index is invalid because " << name << " is empty.\");" << endl;
        c << "        else" << endl;
        c << "            SNPRINTF(msg, 200, \"The index is invalid. Use index values in: [0, %d).\", obj->data->Get" << Name << "().size());" << endl;
        c << "        PyErr_SetString(PyExc_IndexError, msg);" << endl;
        c << "        return NULL;" << endl;
        c << "    }" << endl;
        c << endl;
        c << "    // Since the new object will point to data owned by the this object," << endl;
        c << "    // we need to increment the reference count." << endl;
        c << "    Py_INCREF(self);" << endl;
        c << endl;
        c << "    PyObject *retval = Py" << attType << "_Wrap(&obj->data->Get" << Name << "(index));" << endl;
        c << "    // Set the object's parent so the reference to the parent can be decref'd" << endl;
        c << "    // when the child goes out of scope." << endl;
        c << "    Py" << attType << "_SetParent(retval, self);" << endl;
        c << endl;
    }

    virtual void WriteAdditionalMethods(ostream &c, const QString &className)
    {
        c << "PyObject *" << endl;
        c << className << "_GetNum" << Name << "(PyObject *self, PyObject *args)" << endl;
        c << "{" << endl;
        c << "    " << className << "Object *obj = ("<<className<<"Object *)self;" << endl;
        c << "    return PyInt_FromLong((long)obj->data->Get" << Name << "().size());" << endl;
        c << "}" << endl;
        c << endl;

        c << "PyObject *" << endl;
        c << className << "_Add" << Name << "(PyObject *self, PyObject *args)" << endl;
        c << "{" << endl;
        c << "    " << className << "Object *obj = ("<<className<<"Object *)self;" << endl;
        c << "    PyObject *element = NULL;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"O\", &element))" << endl;
        c << "        return NULL;" << endl;
        c << "    if(!Py" << attType << "_Check(element))" << endl; 
        c << "    {" << endl;
        c << "        char msg[400];" << endl;
        c << "        SNPRINTF(msg, 400, \"The Add" << Name << " method only accepts " << attType << " objects.\");" << endl;
        c << "        PyErr_SetString(PyExc_TypeError, msg);" << endl;
        c << "        return NULL;" << endl;
        c << "    }" << endl;
        c << "    " << attType << " *newData = Py" << attType << "_FromPyObject(element);" << endl;
        if(accessType != Field::AccessPublic)
        {
            c << "    obj->data->Add" << Name << "(*newData);" << endl;
            c << "    obj->data->Select" << Name << "();" << endl;
        }
        else
        {
            c << "    obj->data->" << name << ".push_back(new " << attType << "(*newData));" << endl;
        }
        c << "    Py_INCREF(Py_None);" << endl;
        c << "    return Py_None;" << endl;
        c << "}" << endl;
        c << endl;

        c << "static PyObject *" << endl;
        c << className << "_Remove_One_" << Name << "(PyObject *self, int index)" << endl;
        c << "{" << endl;
        c << "    " << className << "Object *obj = ("<<className<<"Object *)self;" << endl;
        c << "    // Remove in the AttributeGroupVector instead of calling Remove" << Name
          << "() because we don't want to delete the object; just remove it." << endl;
        if(accessType == Field::AccessPublic)
            c << "    AttributeGroupVector &atts = obj->data->" << name << ";" << endl;
        else
            c << "    AttributeGroupVector &atts = obj->data->Get" << Name << "();" << endl;
        c << "    AttributeGroupVector::iterator pos = atts.begin();" << endl;
        c << "    // Iterate through the vector \"index\" times. " << endl;
        c << "    for(int i = 0; i < index; ++i)" << endl;
        c << "        ++pos;" << endl;
        c << endl;
        c << "    // If pos is still a valid iterator, remove that element." << endl;
        c << "    if(pos != atts.end())" << endl;
        c << "    {" << endl;
        c << "        // NOTE: Leak the object since other Python objects may reference it. Ideally," << endl;
        c << "        // we would put the object into some type of pool to be cleaned up later but" << endl;
        c << "        // this will do for now." << endl;
        c << "        //" << endl;
        c << "        // delete *pos;" << endl;
        c << "        atts.erase(pos);" << endl;
        c << "    }" << endl;
        c << endl;
        if(accessType != Field::AccessPublic)
            c << "    obj->data->Select" << Name << "();" << endl;
        c << "    Py_INCREF(Py_None);" << endl;
        c << "    return Py_None;" << endl;
        c << "}" << endl;
        c << endl;

        c << "PyObject *" << endl;
        c << className << "_Remove" << Name << "(PyObject *self, PyObject *args)" << endl;
        c << "{" << endl;
        c << "    int index;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &index))" << endl;
        c << "        return NULL;" << endl;
        c << "    " << className << "Object *obj = ("<<className<<"Object *)self;" << endl;
        if(accessType == Field::AccessPublic)
            c << "    if(index < 0 || index >= obj->data->" << name << ".size())" << endl;
        else
            c << "    if(index < 0 || index >= obj->data->GetNum" << Name << "())" << endl;
        c << "    {" << endl;
        c << "        PyErr_SetString(PyExc_IndexError, \"Index out of range\");" << endl;
        c << "        return NULL;" << endl;
        c << "    }" << endl;
        c << endl;
        c << "    return " << className << "_Remove_One_" << Name << "(self, index);" << endl;
        c << "}" << endl;
        c << endl;

        c << "PyObject *" << endl;
        c << className << "_Clear" << Name << "(PyObject *self, PyObject *args)" << endl;
        c << "{" << endl;
        c << "    " << className << "Object *obj = ("<<className<<"Object *)self;" << endl;
        if(accessType == Field::AccessPublic)
            c << "    int n = obj->data->" << name << ".size();" << endl;
        else
            c << "    int n = obj->data->GetNum" << Name << "();" << endl; 
        c << "    for(int i = 0; i < n; ++i)" << endl;
        c << "    {" << endl;
        c << "        " << className << "_Remove_One_" << Name << "(self, 0);" << endl;
        c << "        Py_DECREF(Py_None);" << endl;
        c << "    }" << endl;
        c << "    Py_INCREF(Py_None);" << endl;
        c << "    return Py_None;" << endl;
        c << "}" << endl;
        c << endl;
    }

    virtual std::vector<QString> AdditionalMethodNames(const QString &className)
    {
        std::vector<QString> nameFunc;

        nameFunc.push_back(QString("GetNum") + Name);
        nameFunc.push_back(className + QString("_GetNum") + Name);

        nameFunc.push_back(QString("Add") + Name);
        nameFunc.push_back(className + QString("_Add") + Name);

        nameFunc.push_back(QString("Remove") + Name);
        nameFunc.push_back(className + QString("_Remove") + Name);

        nameFunc.push_back(QString("Clear") + Name);
        nameFunc.push_back(className + QString("_Clear") + Name);

        return nameFunc;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    { // new scope" << endl;
        c << "        int index = 0;" << endl;
        c << "        // Create string representation of " << name << " from atts." << endl;
        if(accessType == Field::AccessPublic)
            c << "        for(AttributeGroupVector::const_iterator pos = atts->" << name << ".begin(); pos != atts->" << name << ".end(); ++pos, ++index)" << endl;
        else
            c << "        for(AttributeGroupVector::const_iterator pos = atts->" << MethodNameGet() << "().begin(); pos != atts->" << MethodNameGet() << "().end(); ++pos, ++index)" << endl;
        c << "        {" << endl;
        c << "            const " << attType << " *current" << " = (const " << attType << " *)(*pos);" << endl;
        c << "            SNPRINTF(tmpStr, 1000, \"Get" << Name << "(%d).\", index);" << endl;
        c << "            std::string objPrefix(prefix + std::string(tmpStr));" << endl;
        c << "            str += Py" << attType << "_ToString(current, objPrefix.c_str());" << endl;
        c << "        }" << endl;
        c << "        if(index == 0)" << endl;
        c << "            str += \"#" << name << " does not contain any " << attType << " objects.\\n\";" << endl;
        c << "    }" << endl;
    }
};


//
// ----------------------------------- Enum -----------------------------------
//
class PythonGeneratorEnum : public virtual Enum , public virtual PythonGeneratorField
{
  public:
    PythonGeneratorEnum(const QString &t, const QString &n, const QString &l)
        : Enum(t,n,l), PythonGeneratorField("enum",n,l), Field("enum",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    int ival;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &ival))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Set the " << name << " in the object." << endl;
        c << "    if(ival >= 0 && ival < " << enumType->values.size() << ")" << endl;
        c << "        obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = " << GetCPPName(true,className) << "(ival);" << endl;
        else
            c << MethodNameSet() << "(" << GetCPPName(true,className) << "(ival));" << endl;
        c << "    else" << endl;
        c << "    {" << endl;
        c << "        fprintf(stderr, \"An invalid " << name << " value was given. \"" << endl;
        c << "                        \"Valid values are in the range of [0," << enumType->values.size()-1 << "]. \"" << endl;
        c << "                        \"You can also use the following names: \"" << endl;
        c << "                        \"";
        for(int i = 0; i < enumType->values.size(); ++i)
        {
            c << enumType->values[i];
            if(i < enumType->values.size() - 1)
                c << ", ";
        }
        c << ".\");" << endl;
        c << "        return NULL;" << endl;
        c << "    }"<< endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    PyObject *retval = PyInt_FromLong(long(obj->data->"<<MethodNameGet()<<"()));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        // Create a string that shows the possible values.
        c << "    const char *" << name << "_names = \"";
        for(int j = 0; j < enumType->values.size(); ++j)
        {
            c << enumType->values[j];
            if(j < enumType->values.size() - 1)
                c << ", ";
        }
        c << "\";" << endl;

        for(int i = 0; i < enumType->values.size(); ++i)
        {
            c << "    ";
            if(i == 0)
            {
                c << "if(atts->";
                if(accessType == Field::AccessPublic)
                    c << name;
                else
                    c << MethodNameGet() << "()";
                c << " == " << classname << "::" << enumType->values[i] << ")" << endl;
            }
            else if(i < enumType->values.size() - 1)
            {
                c << "else if";
                c << "(atts->";
                if(accessType == Field::AccessPublic)
                    c << name;
                else
                    c << MethodNameGet() << "()";
                c << " == " << classname << "::" << enumType->values[i] << ")" << endl;
            }
            else
                c << "else" << endl;
            c << "    {" << endl;
            c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = %s" << enumType->values[i] << "  # %s\\n\", prefix, prefix, " << name << "_names);" << endl;
            c << "        str += tmpStr;" << endl;
            c << "    }" << endl;
        }
        c << endl;
    }

    virtual void WriteGetAttr(ostream &c, const QString &classname)
    {
        if (internal)
            return;

        c << "    if(strcmp(name, \"" << name << "\") == 0)" << endl;
        c << "        return " << classname << "_" << MethodNameGet() << "(self, NULL);" << endl;

        for(int i = 0; i < enumType->values.size(); ++i)
        {
            c << "    ";
            if(i == 0)
                c << "if";
            else
                c << "else if";

            c << "(strcmp(name, \"";
            c << enumType->values[i];
            c << "\") == 0)" << endl;
            c << "        return PyInt_FromLong(long(";
            c << classname << "::" << enumType->values[i];
            c << "));" << endl;
        }
        c << endl;
    }
};

#define AVT_GENERATOR_METHODS \
    virtual void WriteIncludedHeaders(ostream &c) \
    { \
        c << "#include <avtTypes.h>" << endl; \
    } \
    virtual void WriteSetMethodBody(ostream &c, const QString &className) \
    { \
        c << "    int ival;" << endl; \
        c << "    if(!PyArg_ParseTuple(args, \"i\", &ival))" << endl; \
        c << "        return NULL;" << endl; \
        c << endl; \
        QString T(type); T.replace("Field", "");\
        if(accessType == AccessPublic) \
            c << "    obj->data->" << name << " = (" << T << ")ival;" << endl; \
        else \
            c << "    obj->data->" << MethodNameSet() << "((" << T << ")ival);" << endl; \
    } \
    virtual void WriteGetMethodBody(ostream &c, const QString &className) \
    { \
        c << "    PyObject *retval = PyInt_FromLong(long(obj->data->"; \
        if(accessType == AccessPublic) \
            c << name; \
        else \
            c << MethodNameGet()<<"()"; \
        c << "));" << endl; \
    } \
    virtual void StringRepresentation(ostream &c, const QString &classname) \
    { \
        c << "    const char *" << name << "_names = \""; \
        int values_size = 0; \
        const char **values = GetSymbols(values_size); \
        for(int j = 0; j < values_size; ++j) \
        { \
            c << values[j]; \
            if(j < values_size - 1) \
                c << ", "; \
        } \
        c << "\";" << endl; \
        for(int i = 0; i < values_size; ++i) \
        { \
            c << "    "; \
            if(i == 0) \
            { \
                c << "if(atts->"; \
                if(accessType == Field::AccessPublic) \
                    c << name; \
                else \
                    c << MethodNameGet() << "()"; \
                c << " == " << values[i] << ")" << endl; \
            } \
            else if(i < values_size - 1) \
            { \
                c << "else if"; \
                c << "(atts->"; \
                if(accessType == Field::AccessPublic) \
                    c << name; \
                else \
                    c << MethodNameGet() << "()"; \
                c << " == " << values[i] << ")" << endl; \
            } \
            else \
                c << "else" << endl; \
            c << "    {" << endl; \
            c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = %s" << values[i] << "  # %s\\n\", prefix, prefix, " << name << "_names);" << endl; \
            c << "        str += tmpStr;" << endl; \
            c << "    }" << endl; \
        } \
        c << endl; \
    }\
    virtual void WriteGetAttr(ostream &c, const QString &classname)\
    {\
        if (internal)\
            return;\
        c << "    if(strcmp(name, \"" << name << "\") == 0)" << endl;\
        c << "        return " << classname << "_" << MethodNameGet() << "(self, NULL);" << endl;\
        int values_size = 0;\
        const char **values = GetSymbols(values_size);\
        for(int i = 0; i < values_size; ++i)\
        {\
            c << "    ";\
            if(i == 0)\
                c << "if";\
            else\
                c << "else if";\
            c << "(strcmp(name, \"";\
            c << values[i];\
            c << "\") == 0)" << endl;\
            c << "        return PyInt_FromLong(long(";\
            c << values[i];\
            c << "));" << endl;\
        }\
        c << endl;\
    }

//
// ----------------------------------- avtCentering -----------------------------------
//
class AttsGeneratoravtCentering : public virtual PythonGeneratorField, public virtual avtCenteringField
{
  public:
    AttsGeneratoravtCentering(const QString &n, const QString &l)
        : avtCenteringField(n,l), PythonGeneratorField("avtCentering",n,l), Field("avtCentering",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtGhostType -----------------------------------
//
class AttsGeneratoravtGhostType : public virtual PythonGeneratorField, public virtual avtGhostTypeField
{
  public:
    AttsGeneratoravtGhostType(const QString &n, const QString &l)
        : avtGhostTypeField(n,l), PythonGeneratorField("avtGhostType",n,l), Field("avtGhostType",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtSubsetType -----------------------------------
//
class AttsGeneratoravtSubsetType : public virtual PythonGeneratorField, public virtual avtSubsetTypeField
{
  public:
    AttsGeneratoravtSubsetType(const QString &n, const QString &l)
        : avtSubsetTypeField(n,l), PythonGeneratorField("int",n,l), Field("int",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtVarType -----------------------------------
//
class AttsGeneratoravtVarType : public virtual PythonGeneratorField, public virtual avtVarTypeField
{
  public:
    AttsGeneratoravtVarType(const QString &n, const QString &l)
        : avtVarTypeField(n,l), PythonGeneratorField("avtVarType",n,l), Field("avtVarTypeField",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtMeshType -----------------------------------
//
class AttsGeneratoravtMeshType : public virtual PythonGeneratorField, public virtual avtMeshTypeField
{
  public:
    AttsGeneratoravtMeshType(const QString &n, const QString &l)
        : avtMeshTypeField(n,l), PythonGeneratorField("avtMeshType",n,l), Field("avtMeshTypeField",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtExtentType -----------------------------------
//
class AttsGeneratoravtExtentType : public virtual PythonGeneratorField, public virtual avtExtentTypeField
{
  public:
    AttsGeneratoravtExtentType(const QString &n, const QString &l)
        : avtExtentTypeField(n,l), PythonGeneratorField("avtExentType",n,l), Field("avtExtentType",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtMeshCoordType -----------------------------------
//
class AttsGeneratoravtMeshCoordType : public virtual PythonGeneratorField, public virtual avtMeshCoordTypeField
{
  public:
    AttsGeneratoravtMeshCoordType(const QString &n, const QString &l)
        : avtMeshCoordTypeField(n,l), PythonGeneratorField("avtMeshCoordType",n,l), Field("avtMeshCoordType",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- LoadBalanceScheme -----------------------------------
//
class AttsGeneratorLoadBalanceScheme : public virtual PythonGeneratorField, public virtual LoadBalanceSchemeField
{
  public:
    AttsGeneratorLoadBalanceScheme(const QString &n, const QString &l)
        : LoadBalanceSchemeField(n,l), PythonGeneratorField("LoadBalanceScheme",n,l), Field("LoadBalanceScheme",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// --------------------------------- ScaleMode --------------------------------
//
class AttsGeneratorScaleMode : public virtual ScaleMode , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorScaleMode(const QString &n, const QString &l)
        : ScaleMode(n,l), PythonGeneratorField("scalemode",n,l), Field("scalemode",n,l) { }
    virtual void WriteSetMethodBody(ostream &c, const QString &className)
    {
        c << "    int ival;" << endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &ival))" << endl;
        c << "        return NULL;" << endl;
        c << endl;
        c << "    // Set the " << name << " in the object." << endl;
        c << "    if(ival >= 0 && ival <= 1)" << endl;
        c << "        obj->data->" << MethodNameSet() << "(ival);" << endl;
        c << "    else" << endl;
        c << "    {" << endl;
        c << "        fprintf(stderr, \"An invalid  value was given. \"" << endl;
        c << "                        \"Valid values are in the range of [0,1]. \"" << endl;
        c << "                        \"You can also use the following names: \"" << endl;
        c << "                        \"\\\"LINEAR\\\", \\\"LOG\\\"\\n\");" << endl;
        c << "        return NULL;" << endl;
        c << "    }" << endl;
    }

    virtual void WriteGetMethodBody(ostream &c, const QString &className)
    {
        c << "    PyObject *retval = PyInt_FromLong(long(obj->data->"<<MethodNameGet()<<"()));" << endl;
    }

    virtual void StringRepresentation(ostream &c, const QString &classname)
    {
        c << "    const char *" << name << "_values[] = {\"LINEAR\", \"LOG\"};" << endl;
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = %s  # LINEAR, LOG\\n\", prefix, " << name << "_values[atts->" << MethodNameGet() << "()]);" << endl;
        c << "    str += tmpStr;" << endl;
    }

    virtual void WriteGetAttr(ostream &c, const QString &classname)
    {
        if (internal)
            return;

        c << "    if(strcmp(name, \"" << name << "\") == 0)" << endl;
        c << "        return " << classname << "_" << MethodNameGet() << "(self, NULL);" << endl;
        c << "    if(strcmp(name, \"LINEAR\") == 0)" << endl;
        c << "        return PyInt_FromLong(long(0));" << endl;
        c << "    else if(strcmp(name, \"LOG\") == 0)" << endl;
        c << "        return PyInt_FromLong(long(1));" << endl;
        c << endl;
    }
};


// ----------------------------------------------------------------------------
// Modifications:
//    Brad Whitlock, Wed Dec 8 15:55:08 PST 2004
//    Added support for variable names.
//
//    Kathleen Bonnell, Thu Mar 22 16:58:23 PDT 2007 
//    Added scalemode.
//
// ----------------------------------------------------------------------------
class PythonFieldFactory
{
  public:
    static PythonGeneratorField *createField(const QString &name,
                                           const QString &type,
                                           const QString &subtype,
                                           const QString &length,
                                           const QString &label)
    {
        PythonGeneratorField *f = NULL;
        if      (type.isNull())          throw QString().sprintf("Field %s was specified with no type.",name.latin1());
        else if (type == "int")          f = new AttsGeneratorInt(name,label);
        else if (type == "intArray")     f = new AttsGeneratorIntArray(length,name,label);
        else if (type == "intVector")    f = new AttsGeneratorIntVector(name,label);
        else if (type == "bool")         f = new AttsGeneratorBool(name,label);
        else if (type == "float")        f = new AttsGeneratorFloat(name,label);
        else if (type == "floatArray")   f = new AttsGeneratorFloatArray(length,name,label);
        else if (type == "double")       f = new AttsGeneratorDouble(name,label);
        else if (type == "doubleArray")  f = new AttsGeneratorDoubleArray(length,name,label);
        else if (type == "doubleVector") f = new AttsGeneratorDoubleVector(name,label);
        else if (type == "uchar")        f = new AttsGeneratorUChar(name,label);
        else if (type == "ucharArray")   f = new AttsGeneratorUCharArray(length,name,label);
        else if (type == "ucharVector")  f = new AttsGeneratorUCharVector(name,label);
        else if (type == "string")       f = new AttsGeneratorString(name,label);
        else if (type == "stringVector") f = new AttsGeneratorStringVector(name,label);
        else if (type == "colortable")   f = new AttsGeneratorColorTable(name,label);
        else if (type == "color")        f = new AttsGeneratorColor(name,label);
        else if (type == "opacity")      f = new AttsGeneratorOpacity(name,label);
        else if (type == "linestyle")    f = new AttsGeneratorLineStyle(name,label);
        else if (type == "linewidth")    f = new AttsGeneratorLineWidth(name,label);
        else if (type == "variablename") f = new AttsGeneratorVariableName(name,label);
        else if (type == "att")          f = new AttsGeneratorAtt(subtype,name,label);
        else if (type == "attVector")    f = new AttsGeneratorAttVector(subtype,name,label);
        else if (type == "enum")         f = new PythonGeneratorEnum(subtype, name, label);
        else if (type == "scalemode")    f = new AttsGeneratorScaleMode(name,label);

        // Special built-in AVT enums
        else if (type == "avtCentering")      f = new AttsGeneratoravtCentering(name, label);
        else if (type == "avtVarType")        f = new AttsGeneratoravtVarType(name, label);
        else if (type == "avtSubsetType")     f = new AttsGeneratoravtSubsetType(name, label);
        else if (type == "avtExtentType")     f = new AttsGeneratoravtExtentType(name, label);
        else if (type == "avtMeshType")       f = new AttsGeneratoravtMeshType(name, label);
        else if (type == "avtGhostType")      f = new AttsGeneratoravtGhostType(name, label);
        else if (type == "avtMeshCoordType")  f = new AttsGeneratoravtMeshCoordType(name, label);
        else if (type == "LoadBalanceScheme") f = new AttsGeneratorLoadBalanceScheme(name, label);

        if (!f)
            throw QString().sprintf("PythonFieldFactory: unknown type for field %s: %s",name.latin1(),type.latin1());

        return f;
    }
};

// ----------------------------------------------------------------------------
//  Modifications:
//    Brad Whitlock, Thu Feb 28 16:29:20 PST 2008
//    Made it use a base class.
//
// ----------------------------------------------------------------------------
#include <GeneratorBase.h>

class PythonGeneratorAttribute : public GeneratorBase
{
  public:
    vector<PythonGeneratorField*> fields;
    bool visitpy_api;
  public:
    PythonGeneratorAttribute(const QString &n, const QString &p, const QString &f,
                           const QString &e, const QString &ei)
        : GeneratorBase(n,p,f,e,ei, GENERATOR_NAME), fields()
    {
        visitpy_api = true;
    }

    virtual ~PythonGeneratorAttribute()
    {
        for (int i = 0; i < fields.size(); ++i)
            delete fields[i];
        fields.clear();
    }

    void DisableVISITPY() { visitpy_api = false; }

    void PrintFunction(ostream &out, const QString &f)
    {
        for (int i=0; i<functions.size(); i++)
            if (functions[i]->name == f)
                out << functions[i]->def;
    }

    void Print(ostream &out)
    {
        out << "    Attribute: " << name << " (" << purpose << ")" << endl;
        int i;
        for (i=0; i<fields.size(); i++)
            fields[i]->Print(out);
        for (i=0; i<functions.size(); i++)
            functions[i]->Print(out, generatorName);
    }

    void WriteHeader(ostream &h)
    {
        h << copyright_str.c_str() << endl;
        h << "#ifndef PY_" << name.upper() << "_H" << endl;
        h << "#define PY_" << name.upper() << "_H" << endl;
        h << "#include <Python.h>" << endl;
        h << "#include <"<<name<<".h>" << endl;
        QString api(""), api2(" ");
        if(visitpy_api)
        {
             h << "#include <visitpy_exports.h>" << endl;
             api = "VISITPY_API";
             api2 = "VISITPY_API ";
        }
        h << endl;
        h << "//" << endl;
        h << "// Functions exposed to the VisIt module." << endl;
        h << "//" << endl;
        h << "void "<<api2<<"          Py"<<name<<"_StartUp("<<name<<" *subj, void *data);" << endl;
        h << "void "<<api2<<"          Py"<<name<<"_CloseDown();" << endl;
        h << api << "PyMethodDef * "<<"  Py"<<name<<"_GetMethodTable(int *nMethods);" << endl;
        h << "bool "<<api2<<"          Py"<<name<<"_Check(PyObject *obj);" << endl;
        h << api << name << " * "<<" Py"<<name<<"_FromPyObject(PyObject *obj);" << endl;
        h << api << "PyObject * "<<"     Py"<<name<<"_New();" << endl;
        h << api << "PyObject * "<<"     Py"<<name<<"_Wrap(const " << name << " *attr);" << endl;
        h << "void "<<api2<<"          Py"<<name<<"_SetParent(PyObject *obj, PyObject *parent);" << endl;
        h << "void "<<api2<<"          Py"<<name<<"_SetDefaults(const "<<name<<" *atts);" << endl;
        h << "std::string "<<api2<<"   Py"<<name<<"_GetLogString();" << endl;
        h << "std::string "<<api2<<"   Py"<<name<<"_ToString(const " << name << " *, const char *);" << endl;
        h << endl;
        h << "#endif" << endl;
        h << endl;
    }

    void WriteIncludedHeaders(ostream &c)
    {
        c << "#include <snprintf.h>" << endl;

        // Write the headers that are needed.
        for(int i = 0; i < fields.size(); ++i)
            fields[i]->WriteIncludedHeaders(c);
    }

    void WriteHeaderComment(ostream &c)
    {
        c << "// ****************************************************************************" << endl;
        c << "// Module: Py" << name << endl;
        c << "//" << endl;
        c << "// Purpose: " << endl;
        c << "//   " << purpose << endl;
        c << "//" << endl;
        c << "// Note:       Autogenerated by xml2python. Do not modify by hand!" << endl;
        c << "//" << endl;
        c << "// Programmer: xml2python" << endl;
        c << "// Creation:   " << CurrentTime() << endl;       
        c << "//" << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
    }

    void WritePyObjectStruct(ostream &c)
    {
        c << "//" << endl;
        c << "// This struct contains the Python type information and a "<<name<<"." << endl;
        c << "//" << endl;
        c << "struct "<<name<<"Object" << endl;
        c << "{" << endl;
        c << "    PyObject_HEAD" << endl;
        c << "    "<<name<<" *data;" << endl;
        c << "    bool        owns;" << endl;
        c << "    PyObject   *parent;" << endl;
        c << "};" << endl;
        c << endl;
    }

    void WriteInternalPrototypes(ostream &c)
    {
        c << "//" << endl;
        c << "// Internal prototypes" << endl;
        c << "//" << endl;
        c << "static PyObject *New"<<name<<"(int);" << endl;
        c << endl;
    }

    void WritePyObjectMethods(ostream &c)
    {
        // Write the Notify method.
        c << "static PyObject *" << endl;
        c << name << "_Notify(PyObject *self, PyObject *args)" << endl;
        c << "{" << endl;
        c << "    " << name << "Object *obj = ("<<name<<"Object *)self;" << endl;
        c << "    obj->data->Notify();" << endl;
        c << "    Py_INCREF(Py_None);" << endl;
        c << "    return Py_None;" << endl;
        c << "}" << endl;
        c << endl;

        // Write the rest of the methods.
        for(int i = 0; i < fields.size(); ++i)
        {
            // Do not add any methods if the field is internal.
            if(fields[i]->internal)
                continue;

            if(fields[i]->ProvidesSetMethod())
            {
                QString sName(name + "_" + fields[i]->MethodNameSet());
                if(HasFunction(sName))
                {
                    PrintFunction(c, sName);
                    c << endl;
                }
                else
                    fields[i]->WriteSetMethod(c, name);
            }

            QString gName(name + "_" + fields[i]->MethodNameGet());
            if(HasFunction(gName))
            {
                PrintFunction(c, gName);
                c << endl;
            }
            else
                fields[i]->WriteGetMethod(c, name);

            fields[i]->WriteAdditionalMethods(c, name);
        }
        c << endl;
    }

    void WriteUserDefinedFunctions(ostream &c)
    { 
        for(int i = 0; i < functions.size(); ++i)
        {
            if(functions[i]->target == generatorName &&
               functions[i]->user)
                c << functions[i]->def << endl;
        }
    }

    void WritePyObjectMethodTable(ostream &c)
    {
        c << endl;
        c << "static struct PyMethodDef "<<name<<"_methods[] = {" << endl;
        c << "    {\"Notify\", " << name << "_Notify, METH_VARARGS}," << endl;
        for(int i = 0; i < fields.size(); ++i)
        {
            fields[i]->WritePyObjectMethodTable(c, name);
        }
        for(int i = 0; i < functions.size(); ++i)
        {
            if(functions[i]->target == generatorName &&
               functions[i]->user)
                c << functions[i]->decl << ", " << endl;
        }
        c << "    {NULL, NULL}" << endl;
        c << "};" << endl;
        c << endl;
    }

    void WriteGetAttrFunction(ostream &c)
    {
        QString mName(name + "_getattr");
        if(HasFunction(mName))
        {
            PrintFunction(c, mName);
            c << endl;
            return;
        }

        c << "static PyObject *" << endl;
        c << mName << "(PyObject *self, char *name)" << endl;
        c << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(c, mName, 0);
        for(int i = 0; i < fields.size(); ++i)
            fields[i]->WriteGetAttr(c, name);
        c << endl;
        if(HasCode(mName, 1))
            PrintCode(c, mName, 1);
        c << "    return Py_FindMethod("<<name<<"_methods, self, name);" << endl;
        c << "}" << endl;
        c << endl;
    }

    void WriteSetAttrFunction(ostream &c)
    {
        QString mName(name + "_setattr");
        if(HasFunction(mName))
        {
            PrintFunction(c, mName);
            c << endl;
            return;
        }

        c << "static int" << endl;
        c << mName << "(PyObject *self, char *name, PyObject *args)" << endl;
        c << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(c, mName, 0);
        c << "    // Create a tuple to contain the arguments since all of the Set" << endl;
        c << "    // functions expect a tuple." << endl;
        c << "    PyObject *tuple = PyTuple_New(1);" << endl;
        c << "    PyTuple_SET_ITEM(tuple, 0, args);" << endl;
        c << "    Py_INCREF(args);" << endl;
        c << "    PyObject *obj = NULL;" << endl;
        c << endl;

        // Figure out the first field that can write a _setattr method.
        int i, index = 0;
        for(i = 0; i < fields.size(); ++i)
        {
            if(fields[i]->HasSetAttr() && !fields[i]->internal)
            {
                index = i;
                break;
            }
        }

        for(i = index; i < fields.size(); ++i)
            fields[i]->WriteSetAttr(c, name, i == index);
        c << endl;

        if(HasCode(mName, 1))
            PrintCode(c, mName, 1);

        c << "    if(obj != NULL)" << endl;
        c << "        Py_DECREF(obj);" << endl;
        c << endl;
        c << "    Py_DECREF(tuple);" << endl;
        c << "    return (obj != NULL) ? 0 : -1;" << endl;
        c << "}" << endl;
        c << endl;
    }

    void WritePrintFunction(ostream &c)
    {
        QString mName(name + "_print");
        if(HasFunction(mName))
        {
            PrintFunction(c, mName);
            c << endl;
            return;
        }

        c << "static int" << endl;
        c << mName << "(PyObject *v, FILE *fp, int flags)" << endl;
        c << "{" << endl;
        c << "    "<<name<<"Object *obj = ("<<name<<"Object *)v;" << endl;
        if(HasCode(mName, 0))
            PrintCode(c, mName, 0);
        c << "    fprintf(fp, \"%s\", Py" << name << "_ToString(obj->data, \"\").c_str());" << endl;
        if(HasCode(mName, 1))
            PrintCode(c, mName, 1);
        c << "    return 0;" << endl;
        c << "}" << endl;
        c << endl;
    }

    void WriteToStringFunction(ostream &c)
    {
        QString mName(QString("Py") + name + "_ToString");
        if(HasFunction(mName))
        {
            PrintFunction(c, mName);
            c << endl;
            return;
        }

        c << "std::string" << endl;
        c << mName << "(const "<<name<<" *atts, const char *prefix)" << endl;
        c << "{" << endl;
        c << "    std::string str; " << endl;
        c << "    char tmpStr[1000]; " << endl;
        c << endl;
        if(HasCode(mName, 0))
            PrintCode(c, mName, 0);
        for(int i = 0; i < fields.size(); ++i)
        {
            if(!fields[i]->internal)
                fields[i]->StringRepresentation(c, name);
        }
        if(HasCode(mName, 1))
            PrintCode(c, mName, 1);
        c << "    return str;" << endl;
        c << "}" << endl << endl;
    }

    void WriteStringRepresentationFunction(ostream &c)
    {
        QString mName(name + "_str");
        if(HasFunction(mName))
        {
            PrintFunction(c, mName);
            c << endl;
            return;
        }

        c << "PyObject *" << endl;
        c << mName << "(PyObject *v)" << endl;
        c << "{" << endl;
        c << "    "<<name<<"Object *obj = ("<<name<<"Object *)v;" << endl;
        c << "    return PyString_FromString(Py" << name << "_ToString(obj->data,\"\").c_str());" << endl;
        c << "}" << endl << endl;
    }


    void WriteTypeFunctions(ostream &c)
    {
        c << "//" << endl;
        c << "// Type functions" << endl;
        c << "//" << endl;
        c << endl;

        c << "static void" << endl;
        c << name << "_dealloc(PyObject *v)" << endl;
        c << "{" << endl;
        c << "   " << name << "Object *obj = (" << name << "Object *)v;" << endl;
        c << "   if(obj->parent != 0)" << endl;
        c << "       Py_DECREF(obj->parent);" << endl;
        c << "   if(obj->owns)" << endl;
        c << "       delete obj->data;" << endl;
        c << "}" << endl;
        c << endl;

        c << "static int" << endl;
        c << name << "_compare(PyObject *v, PyObject *w)" << endl;
        c << "{" << endl;
        c << "    "<<name<<" *a = (("<<name<<"Object *)v)->data;" << endl;
        c << "    "<<name<<" *b = (("<<name<<"Object *)w)->data;" << endl;
        c << "    return (*a == *b) ? 0 : -1;" << endl;
        c << "}" << endl;
        c << endl;

        // Write the getattr function
        WriteGetAttrFunction(c);

        // Write the setattr function
        WriteSetAttrFunction(c);

        // Write the print function
        WritePrintFunction(c);

        // Write the str function.
        WriteStringRepresentationFunction(c);

        c << "//" << endl;
        c << "// The doc string for the class." << endl;
        c << "//" << endl;
        c << "static char *" << name << "_Purpose = \"" << purpose << "\";" << endl;
        c << endl;

        c << "//" << endl;
        c << "// The type description structure" << endl;
        c << "//" << endl;
        c << "static PyTypeObject "<<name<<"Type =" << endl;
        c << "{" << endl;
        c << "    //" << endl;
        c << "    // Type header" << endl;
        c << "    //" << endl;
        c << "    PyObject_HEAD_INIT(&PyType_Type)" << endl;
        c << "    0,                                   // ob_size" << endl;
        c << "    \""<<name<<"\",                    // tp_name" << endl;
        c << "    sizeof("<<name<<"Object),        // tp_basicsize" << endl;
        c << "    0,                                   // tp_itemsize" << endl;
        c << "    //" << endl;
        c << "    // Standard methods" << endl;
        c << "    //" << endl;
        c << "    (destructor)"<<name<<"_dealloc,  // tp_dealloc" << endl;
        c << "    (printfunc)"<<name<<"_print,     // tp_print" << endl;
        c << "    (getattrfunc)"<<name<<"_getattr, // tp_getattr" << endl;
        c << "    (setattrfunc)"<<name<<"_setattr, // tp_setattr" << endl;
        c << "    (cmpfunc)"<<name<<"_compare,     // tp_compare" << endl;
        c << "    (reprfunc)0,                         // tp_repr" << endl;
        c << "    //" << endl;
        c << "    // Type categories" << endl;
        c << "    //" << endl;
        c << "    0,                                   // tp_as_number" << endl;
        c << "    0,                                   // tp_as_sequence" << endl;
        c << "    0,                                   // tp_as_mapping" << endl;
        c << "    //" << endl;
        c << "    // More methods" << endl;
        c << "    //" << endl;
        c << "    0,                                   // tp_hash" << endl;
        c << "    0,                                   // tp_call" << endl;
        c << "    (reprfunc)" << name << "_str,        // tp_str" << endl;
        c << "    0,                                   // tp_getattro" << endl;
        c << "    0,                                   // tp_setattro" << endl;
        c << "    0,                                   // tp_as_buffer" << endl;
        c << "    Py_TPFLAGS_CHECKTYPES,               // tp_flags" << endl;
        c << "    "<<name<<"_Purpose,              // tp_doc" << endl;
        c << "    0,                                   // tp_traverse" << endl;
        c << "    0,                                   // tp_clear" << endl;
        c << "    0,                                   // tp_richcompare" << endl;
        c << "    0                                    // tp_weaklistoffset" << endl;
        c << "};" << endl;
        c << endl;
        c << "//" << endl;
        c << "// Helper functions for object allocation." << endl;
        c << "//" << endl;
        c << endl;
        c << "static "<<name<<" *defaultAtts = 0;" << endl;
        c << "static "<<name<<" *currentAtts = 0;" << endl;
        c << endl;
        c << "static PyObject *" << endl;
        c << "New"<<name<<"(int useCurrent)" << endl;
        c << "{" << endl;
        c << "    "<<name<<"Object *newObject;" << endl;
        c << "    newObject = PyObject_NEW("<<name<<"Object, &"<<name<<"Type);" << endl;
        c << "    if(newObject == NULL)" << endl;
        c << "        return NULL;" << endl;
        c << "    if(useCurrent && currentAtts != 0)" << endl;
        c << "        newObject->data = new "<<name<<"(*currentAtts);" << endl;
        c << "    else if(defaultAtts != 0)" << endl;
        c << "        newObject->data = new "<<name<<"(*defaultAtts);" << endl;
        c << "    else" << endl;
        c << "        newObject->data = new "<<name<<";" << endl;
        c << "    newObject->owns = true;" << endl;
        c << "    newObject->parent = 0;" << endl;
        c << "    return (PyObject *)newObject;" << endl;
        c << "}" << endl;
        c << endl;
        c << "static PyObject *" << endl;
        c << "Wrap"<<name<<"(const " << name << " *attr)" << endl;
        c << "{" << endl;
        c << "    "<<name<<"Object *newObject;" << endl;
        c << "    newObject = PyObject_NEW("<<name<<"Object, &"<<name<<"Type);" << endl;
        c << "    if(newObject == NULL)" << endl;
        c << "        return NULL;" << endl;
        c << "    newObject->data = ("<<name<< " *)attr;" << endl;
        c << "    newObject->owns = false;" << endl;
        c << "    newObject->parent = 0;" << endl;
        c << "    return (PyObject *)newObject;" << endl;
        c << "}" << endl;
        c << endl;
    }

    void WriteCallLogRoutineMethod(ostream &c)
    {
        QString shortName(name);
        shortName.replace(QRegExp("Attributes"), "Atts");

        QString GetLogString(QString("Py") + name + "_GetLogString");
        if(HasFunction(GetLogString))
            PrintFunction(c, GetLogString);
        else
        { 
            c << "std::string" << endl;
            c << "Py" << name << "_GetLogString()" << endl;
            c << "{" << endl;
            c << "    std::string s(\"" << shortName << " = " << name << "()\\n\");" << endl;
            c << "    if(currentAtts != 0)" << endl;
            c << "        s += Py" << name << "_ToString(currentAtts, \"" << shortName << ".\");" << endl;
            c << "    return s;" << endl;
            c << "}" << endl;
        }
        c << endl;

        QString CallLogRoutine(QString("Py") + name + "_CallLogRoutine");
        if(HasFunction(CallLogRoutine))
            PrintFunction(c, CallLogRoutine);
        else
        { 
            c << "static void" << endl;
            c << CallLogRoutine << "(Subject *subj, void *data)" << endl;
            c << "{" << endl;
            c << "    "<<name<<" *atts = ("<<name<<" *)subj;" << endl;
            if(HasCode(CallLogRoutine, 0))
                PrintCode(c, CallLogRoutine, 0);
            c << "    typedef void (*logCallback)(const std::string &);" << endl;
            c << "    logCallback cb = (logCallback)data;" << endl;
            c << endl;
            c << "    if(cb != 0)" << endl;
            c << "    {" << endl;
            c << "        std::string s(\"" << shortName << " = " << name << "()\\n\");" << endl;
            c << "        s += Py" << name << "_ToString(currentAtts, \"" << shortName << ".\");" << endl;
            c << "        cb(s);" << endl;
            c << "    }" << endl;
            if(HasCode(CallLogRoutine, 1))
                PrintCode(c, CallLogRoutine, 1);
            c << "}" << endl;
        }
    }

    void WriteExposedInterface(ostream &c)
    {
        c << "///////////////////////////////////////////////////////////////////////////////" << endl;
        c << "//" << endl;
        c << "// Interface that is exposed to the VisIt module." << endl;
        c << "//" << endl;
        c << "///////////////////////////////////////////////////////////////////////////////" << endl;
        c << endl;

        QString Py_new(name + "_new");
        if(HasFunction(Py_new))
            PrintFunction(c, Py_new);
        else
        { 
            c << "PyObject *" << endl;
            c << Py_new << "(PyObject *self, PyObject *args)" << endl;
            c << "{" << endl;
            if(HasCode(Py_new, 0))
                PrintCode(c, Py_new, 0);
            c << "    int useCurrent = 0;" << endl;
            c << "    if (!PyArg_ParseTuple(args, \"i\", &useCurrent))" << endl;
            c << "    {" << endl;
            c << "        if (!PyArg_ParseTuple(args, \"\"))" << endl;
            c << "            return NULL;" << endl;
            c << "        else" << endl;
            c << "            PyErr_Clear();" << endl;
            c << "    }" << endl;
            c << endl;
            if(HasCode(Py_new, 1))
                PrintCode(c, Py_new, 1);
            c << "    return (PyObject *)New"<<name<<"(useCurrent);" << endl;
            c << "}" << endl;
        }
        c << endl;

        c << "//" << endl;
        c << "// Plugin method table. These methods are added to the visitmodule's methods." << endl;
        c << "//" << endl;
        c << "static PyMethodDef "<<name<<"Methods[] = {" << endl;
        c << "    {\""<<name<<"\", "<<name<<"_new, METH_VARARGS}," << endl;
        c << "    {NULL,      NULL}        /* Sentinel */" << endl;
        c << "};" << endl;
        c << endl;
        c << "static Observer *"<<name<<"Observer = 0;" << endl;
        c << endl;
        WriteCallLogRoutineMethod(c);
        c << endl;

        QString StartUp(QString("Py") + name + "_StartUp");
        if(HasFunction(StartUp))
            PrintFunction(c, StartUp);
        else
        { 
            c << "void" << endl;
            c << StartUp << "("<<name<<" *subj, void *data)" << endl;
            c << "{" << endl;
            c << "    if(subj == 0)" << endl;
            c << "        return;" << endl;
            c << endl;
            if(HasCode(StartUp, 0))
                PrintCode(c, StartUp, 0);
            c << "    currentAtts = subj;" << endl;
            c << "    Py" << name << "_SetDefaults(subj);" << endl;
            c << endl;
            c << "    //" << endl;
            c << "    // Create the observer that will be notified when the attributes change." << endl;
            c << "    //" << endl;
            c << "    if("<<name<<"Observer == 0)" << endl;
            c << "    {" << endl;
            c << "        "<<name<<"Observer = new ObserverToCallback(subj," << endl;
            c << "            Py"<<name<<"_CallLogRoutine, (void *)data);" << endl;
            c << "    }" << endl;
            c << endl;
            if(HasCode(StartUp, 1))
                PrintCode(c, StartUp, 1);
            c << "}" << endl;
        }
        c << endl;

        QString CloseDown(QString("Py") + name + "_CloseDown");
        if(HasFunction(CloseDown))
            PrintFunction(c, CloseDown);
        else
        { 
            c << "void" << endl;
            c << CloseDown << "()" << endl;
            c << "{" << endl;
            if(HasCode(CloseDown, 0))
                PrintCode(c, CloseDown, 0);
            c << "    delete defaultAtts;" << endl;
            c << "    defaultAtts = 0;" << endl;
            c << "    delete "<<name<<"Observer;" << endl;
            c << "    "<<name<<"Observer = 0;" << endl;
            if(HasCode(CloseDown, 1))
                PrintCode(c, CloseDown, 1);
            c << "}" << endl;
        }
        c << endl;

        c << "PyMethodDef *" << endl;
        c << "Py"<<name<<"_GetMethodTable(int *nMethods)" << endl;
        c << "{" << endl;
        c << "    *nMethods = 1;" << endl;
        c << "    return "<<name<<"Methods;" << endl;
        c << "}" << endl;
        c << endl;
        c << "bool" << endl;
        c << "Py"<<name<<"_Check(PyObject *obj)" << endl;
        c << "{" << endl;
        c << "    return (obj->ob_type == &"<<name<<"Type);" << endl;
        c << "}" << endl;
        c << endl;

        QString FromPyObject(QString("Py") + name + "_FromPyObject");
        if(HasFunction(FromPyObject))
            PrintFunction(c, FromPyObject);
        else
        { 
            c << name << " *" << endl;
            c << FromPyObject << "(PyObject *obj)" << endl;
            c << "{" << endl;
            c << "    "<<name<<"Object *obj2 = ("<<name<<"Object *)obj;" << endl;
            c << "    return obj2->data;" << endl;
            c << "}" << endl;
        }
        c << endl;

        QString PyNew(QString("Py") + name + "_New");
        if(HasFunction(PyNew))
            PrintFunction(c, PyNew);
        else
        { 
            c << "PyObject *" << endl;
            c << PyNew << "()" << endl;
            c << "{" << endl;
            c << "    return New"<<name<<"(0);" << endl;
            c << "}" << endl;
        }
        c << endl;

        QString Wrap(QString("Py") + name + "_Wrap");
        if(HasFunction(Wrap))
            PrintFunction(c, Wrap);
        else
        { 
            c << "PyObject *" << endl;
            c << Wrap << "(const " << name << " *attr)" << endl;
            c << "{" << endl;
            c << "    return Wrap"<<name<<"(attr);" << endl;
            c << "}" << endl;
        }
        c << endl;

        QString SetParent(QString("Py") + name + "_SetParent");
        if(HasFunction(SetParent))
            PrintFunction(c, SetParent);
        else
        { 
            c << "void" << endl;
            c << SetParent << "(PyObject *obj, PyObject *parent)" << endl;
            c << "{" << endl;
            c << "    "<<name<<"Object *obj2 = ("<<name<<"Object *)obj;" << endl;
            if(HasCode(SetParent, 0))
                PrintCode(c, SetParent, 0);
            c << "    obj2->parent = parent;" << endl;
            if(HasCode(SetParent, 1))
                PrintCode(c, SetParent, 1);
            c << "}" << endl;
        }
        c << endl;
        
        QString SetDefaults(QString("Py") + name + "_SetDefaults");
        if(HasFunction(SetDefaults))
            PrintFunction(c, SetDefaults);
        else
        {    
            c << "void" << endl;
            c << SetDefaults << "(const " << name << " *atts)" << endl;
            c << "{" << endl;
            if(HasCode(SetDefaults, 0))
                PrintCode(c, SetDefaults, 0);
            c << "    if(defaultAtts)" << endl;
            c << "        delete defaultAtts;" << endl;
            c << endl;
            c << "    defaultAtts = new " << name << "(*atts);" << endl;
            if(HasCode(SetDefaults, 1))
                PrintCode(c, SetDefaults, 1);
            c << "}" << endl;
        }
        c << endl;
    }

    void WriteSource(ostream &c)
    {
        c << copyright_str.c_str() << endl;
        c << "#include <Py" << name << ".h>" << endl;
        c << "#include <ObserverToCallback.h>" << endl;
        WriteIncludedHeaders(c);
        c << endl;
        c << "extern void VisItErrorFunc(const char *);" << endl;
        c << endl;
        WriteHeaderComment(c);
        WritePyObjectStruct(c);
        WriteInternalPrototypes(c);
        WriteToStringFunction(c);
        WritePyObjectMethods(c);
        WriteUserDefinedFunctions(c);
        WritePyObjectMethodTable(c);
        WriteTypeFunctions(c);
        WriteExposedInterface(c);
    }
};

// ----------------------------------------------------------------------------
//
// Modifications:
//
//  Kathleen Bonnell, Thu Jul  5 17:53:43 PDT 2007
//  Added WriteHeader, so that visitpy_api could be disabled.
//
//  Hank Childs, Thu Jan 10 14:33:30 PST 2008
//  Added filenames, specifiedFilenames.
//
//   Brad Whitlock, Thu Feb 28 16:26:46 PST 2008
//   Made it use a base class.
//
// ----------------------------------------------------------------------------
#include <PluginBase.h>

class PythonGeneratorPlugin : public PluginBase
{
  public:
    PythonGeneratorAttribute *atts;
  public:
    PythonGeneratorPlugin(const QString &n,const QString &l,const QString &t,
        const QString &vt,const QString &dt, const QString &v, const QString &ifile,
        bool hw, bool ho, bool onlyengine, bool noengine) : 
        PluginBase(n,l,t,vt,dt,v,ifile,hw,ho,onlyengine,noengine), atts(NULL)
    {
    }

    void Print(ostream &out)
    {
        out << "Plugin: "<<name<<" (\""<<label<<"\", type="<<type<<") -- version "<<version<< endl;
        if (atts)
        {
            atts->DisableVISITPY();
            atts->Print(out);
        }
    }

    void WriteHeader(ostream &h)
    {
        if (atts)
        {
            atts->DisableVISITPY();
            atts->WriteHeader(h);
        }
    }
};


// ----------------------------------------------------------------------------
//                           Override default types
// ----------------------------------------------------------------------------
#define FieldFactory PythonFieldFactory
#define Field        PythonGeneratorField
#define Attribute    PythonGeneratorAttribute
#define Enum         PythonGeneratorEnum
#define Plugin       PythonGeneratorPlugin

#endif
