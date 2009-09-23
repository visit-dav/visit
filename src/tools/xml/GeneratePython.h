/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#include <QTextStream>

#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include "Field.h"
#include <QRegExp>

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
//    Added my own t.toUpper() since some implementations don't have one.
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
//    Kathleen Bonnell, Mon Mar 10 16:27:24 PDT 2008 
//    For enum fields, to get around Windows compiler limitations for nested
//    blocks and string-literals, modified: 
//    StringRepresentation ==> to use case instead of if-else, to use 
//    adjacent strings instead of single long string;
//    WriteGetAttr =>  changed else-if to simple if as 'then' is a return;
//    WriteSetMethodBody ==> use adjacent strings instead of single long string
//
//    Jeremy Meredith, Thu Aug  7 14:34:01 EDT 2008
//    Reorder constructor initializers to be the correct order.
//    Use const char* for string literals.
//
//    Mark C. Miller, Wed Aug 26 10:53:51 PDT 2009
//    Added logic to count methods WITHOUT writing them.
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

    virtual void WriteIncludedHeaders(QTextStream &c)
    {
        // Write the list of include files that the object needs.
    }

    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    // NOT IMPLEMENTED!!!" << Endl;
        c << "    // name=" << name << ", type=" << type << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    // NOT IMPLEMENTED!!!" << Endl;
        c << "    // name=" << name << ", type=" << type << Endl;
        c << "    PyObject *retval = NULL;" << Endl;
    }

    // Whether a Set method should be created. Most objects will allow the set 
    // method to be created but att and attVector do not allow it for all types.
    virtual bool ProvidesSetMethod() const { return true; }

    virtual void WriteAdditionalMethods(QTextStream &c, const QString &className)
    {
        // Do nothing.
    }

    virtual std::vector<QString> AdditionalMethodNames(const QString &className)
    {
        std::vector<QString> ret;
        return ret;
    }

    virtual void WriteSetMethod(QTextStream &c, const QString &className)
    {
        c << "/*static*/ PyObject *" << Endl;
        c << className << "_" << MethodNameSet() << "(PyObject *self, PyObject *args)" << Endl;
        c << "{" << Endl;
        c << "    " << className << "Object *obj = ("<<className<<"Object *)self;" << Endl;
        c << Endl;
        WriteSetMethodBody(c, className);
        c << Endl;
        c << "    Py_INCREF(Py_None);" << Endl;
        c << "    return Py_None;" << Endl;
        c << "}" << Endl;
        c << Endl;
    }

    virtual void WriteGetMethod(QTextStream &c, const QString &className)
    {
        c << "/*static*/ PyObject *" << Endl;
        c << className << "_" << MethodNameGet() << "(PyObject *self, PyObject *args)" << Endl;
        c << "{" << Endl;
        c << "    " << className << "Object *obj = ("<<className<<"Object *)self;" << Endl;
        WriteGetMethodBody(c, className);
        c << "    return retval;" << Endl;
        c << "}" << Endl;
        c << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    //" << name << Endl;
    }

    virtual int WritePyObjectMethodTable(QTextStream &c, const QString &className, bool countOnly = false)
    {
        // Do not add any methods if the field is internal.
        if(internal)
            return 0;

        int methCnt = 0;

        if(ProvidesSetMethod())
        {
            if (!countOnly)
                c << "    {\"" << MethodNameSet() << "\", " << className << "_" << MethodNameSet() << ", METH_VARARGS}," << Endl;
            methCnt++;
        }

        if (!countOnly)
            c << "    {\"" << MethodNameGet() << "\", " << className << "_" << MethodNameGet() << ", METH_VARARGS}," << Endl;
        methCnt++;

        // Write any additional methods that may go along with this field.
        std::vector<QString> additionalMethods(AdditionalMethodNames(className));
        for(size_t i = 0; i < additionalMethods.size(); i += 2)
        {
            if (!countOnly)
                c << "    {\"" << additionalMethods[i] << "\", " << additionalMethods[i+1] << ", METH_VARARGS}," << Endl;
            methCnt++;
        }

        return methCnt;
    }

    virtual void WriteGetAttr(QTextStream &c, const QString &className)
    {
        // Do not add any methods if the field is internal.
        if(internal)
            return;

        c << "    if(strcmp(name, \"" << name << "\") == 0)" << Endl;
        c << "        return " << className << "_" << MethodNameGet() << "(self, NULL);" << Endl;
    }

    virtual bool HasSetAttr()
    {
        return true;
    }

    virtual void WriteSetAttr(QTextStream &c, const QString &className, bool first)
    {
        if(internal)
            return;

        if(ProvidesSetMethod())
        {
            if(first)
                c << "    if(strcmp(name, \"" << name << "\") == 0)" << Endl;
            else
                c << "    else if(strcmp(name, \"" << name << "\") == 0)" << Endl;

            c << "        obj = "<<className<<"_"<<MethodNameSet()<<"(self, tuple);" << Endl;
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
        : Field("int",n,l), Int(n,l), PythonGeneratorField("int",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    int ival;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &ival))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Set the " << name << " in the object." << Endl;
        if(accessType == AccessPublic)
            c << "    obj->data->" << name << " = (" << type << ")ival;" << Endl;
        else
            c << "    obj->data->" << MethodNameSet() << "((" << type << ")ival);" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    PyObject *retval = PyInt_FromLong(long(obj->data->";
        if(accessType == AccessPublic)
            c << name;
        else
            c << MethodNameGet()<<"()";
        c << "));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = %d\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ");" << Endl;
        c << "    str += tmpStr;" << Endl;
    }
};


//
// -------------------------------- IntArray --------------------------------
//
class AttsGeneratorIntArray : public virtual IntArray , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorIntArray(const QString &s, const QString &n, const QString &l)
        : Field("intArray",n,l), IntArray(s,n,l), PythonGeneratorField("intArray",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    int *ivals = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
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
        c << "))" << Endl;
        c << "    {" << Endl;
        c << "        PyObject     *tuple;" << Endl;
        c << "        if(!PyArg_ParseTuple(args, \"O\", &tuple))" << Endl;
        c << "            return NULL;" << Endl;
        c << Endl;
        c << "        if(PyTuple_Check(tuple))" << Endl;
        c << "        {" << Endl;
        c << "            if(PyTuple_Size(tuple) != " << length << ")" << Endl;
        c << "                return NULL;" << Endl;
        c << Endl;
        c << "            PyErr_Clear();" << Endl;
        c << "            for(int i = 0; i < PyTuple_Size(tuple); ++i)" << Endl;
        c << "            {" << Endl;
        c << "                PyObject *item = PyTuple_GET_ITEM(tuple, i);" << Endl;
        c << "                if(PyFloat_Check(item))" << Endl;
        c << "                    ivals[i] = int(PyFloat_AS_DOUBLE(item));" << Endl;
        c << "                else if(PyInt_Check(item))" << Endl;
        c << "                    ivals[i] = int(PyInt_AS_LONG(item));" << Endl;
        c << "                else if(PyLong_Check(item))" << Endl;
        c << "                    ivals[i] = int(PyLong_AsDouble(item));" << Endl;
        c << "                else" << Endl;
        c << "                    ivals[i] = 0;" << Endl;
        c << "            }" << Endl;
        c << "        }" << Endl;
        c << "        else" << Endl;
        c << "            return NULL;" << Endl;
        c << "    }" << Endl;
        c << Endl;
        c << "    // Mark the " << name << " in the object as modified." << Endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << Endl;
        else
            c << "    obj->data->Select" << Name << "();" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << Endl;
        c << "    PyObject *retval = PyTuple_New(" << length << ");" << Endl;
        c << "    const int *" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << ";" << Endl;
        else
            c << MethodNameGet() << "();" << Endl;
        c << "    for(int i = 0; i < " << length << "; ++i)" << Endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyInt_FromLong(long(" << name << "[i])));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    {   const int *" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "        for(int i = 0; i < " << length << "; ++i)" << Endl;
        c << "        {" << Endl;
        c << "            SNPRINTF(tmpStr, 1000, \"%d\", " << name << "[i]);" << Endl;
        c << "            str += tmpStr;" << Endl;
        c << "            if(i < " << length - 1 << ")" << Endl;
        c << "            {" << Endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << Endl;
        c << "                str += tmpStr;" << Endl;
        c << "            }" << Endl;
        c << "        }" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "    }" << Endl;
    }

};


//
// -------------------------------- IntVector --------------------------------
//
class AttsGeneratorIntVector : public virtual IntVector , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorIntVector(const QString &n, const QString &l)
        : Field("intVector",n,l), IntVector(n,l), PythonGeneratorField("intVector",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    intVector  &vec = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
        c << "    PyObject   *tuple;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"O\", &tuple))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    if(PyTuple_Check(tuple))" << Endl;
        c << "    {" << Endl;
        c << "        vec.resize(PyTuple_Size(tuple));" << Endl;
        c << "        for(int i = 0; i < PyTuple_Size(tuple); ++i)" << Endl;
        c << "        {" << Endl;
        c << "            PyObject *item = PyTuple_GET_ITEM(tuple, i);" << Endl;
        c << "            if(PyFloat_Check(item))" << Endl;
        c << "                vec[i] = int(PyFloat_AS_DOUBLE(item));" << Endl;
        c << "            else if(PyInt_Check(item))" << Endl;
        c << "                vec[i] = int(PyInt_AS_LONG(item));" << Endl;
        c << "            else if(PyLong_Check(item))" << Endl;
        c << "                vec[i] = int(PyLong_AsLong(item));" << Endl;
        c << "            else" << Endl;
        c << "                vec[i] = 0;" << Endl;
        c << "        }" << Endl;
        c << "    }" << Endl;
        c << "    else if(PyFloat_Check(tuple))" << Endl;
        c << "    {" << Endl;
        c << "        vec.resize(1);" << Endl;
        c << "        vec[0] = int(PyFloat_AS_DOUBLE(tuple));" << Endl;
        c << "    }" << Endl;
        c << "    else if(PyInt_Check(tuple))" << Endl;
        c << "    {" << Endl;
        c << "        vec.resize(1);" << Endl;
        c << "        vec[0] = int(PyInt_AS_LONG(tuple));" << Endl;
        c << "    }" << Endl;
        c << "    else if(PyLong_Check(tuple))" << Endl;
        c << "    {" << Endl;
        c << "        vec.resize(1);" << Endl;
        c << "        vec[0] = int(PyLong_AsLong(tuple));" << Endl;
        c << "    }" << Endl;
        c << "    else" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Mark the "<<name<<" in the object as modified." << Endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << Endl;
        else
            c << "    obj->data->Select"<<Name<<"();" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << Endl;
        c << "    const intVector &" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << ";" << Endl;
        else
            c << MethodNameGet() << "();" << Endl;
        c << "    PyObject *retval = PyTuple_New(" << name << ".size());" << Endl;
        c << "    for(size_t i = 0; i < "<<name<<".size(); ++i)" << Endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyInt_FromLong(long(" << name << "[i])));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    {   const intVector &" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "        for(size_t i = 0; i < " << name << ".size(); ++i)" << Endl;
        c << "        {" << Endl;
        c << "            SNPRINTF(tmpStr, 1000, \"%d\", " << name << "[i]);" << Endl;
        c << "            str += tmpStr;" << Endl;
        c << "            if(i < " << name << ".size() - 1)" << Endl;
        c << "            {" << Endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << Endl;
        c << "                str += tmpStr;" << Endl;
        c << "            }" << Endl;
        c << "        }" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "    }" << Endl;
    }
};


// 
// ----------------------------------- Bool -----------------------------------
//
class AttsGeneratorBool : public virtual Bool , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorBool(const QString &n, const QString &l)
        : Field("bool",n,l), Bool(n,l), PythonGeneratorField("bool",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    int ival;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &ival))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Set the " << name << " in the object." << Endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = ";
        else
            c << MethodNameSet();
        c << "(ival != 0);" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    PyObject *retval = PyInt_FromLong(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet()<<"()";
        c << "?1L:0L);" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    if(atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ")" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = 1\\n\", prefix);" << Endl;
        c << "    else" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = 0\\n\", prefix);" << Endl;
        c << "    str += tmpStr;" << Endl;
    }
};


//
// ----------------------------------- Float ----------------------------------
//
class AttsGeneratorFloat : public virtual Float , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorFloat(const QString &n, const QString &l)
        : Field("float",n,l), Float(n,l), PythonGeneratorField("float",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    float fval;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"f\", &fval))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Set the " << name << " in the object." << Endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = fval;" << Endl;
        else
            c << MethodNameSet() << "(fval);" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    PyObject *retval = PyFloat_FromDouble(double(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << ";" << Endl;
        else
            c <<MethodNameGet()<<"()";
        c << "));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = %g\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ");" << Endl;
        c << "    str += tmpStr;" << Endl;
    }
};


//
// -------------------------------- FloatArray -------------------------------
//
class AttsGeneratorFloatArray : public virtual FloatArray , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorFloatArray(const QString &s, const QString &n, const QString &l)
        : Field("floatArray",n,l), FloatArray(s,n,l), PythonGeneratorField("floatArray",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    float *fvals = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
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
        c << "))" << Endl;
        c << "    {" << Endl;
        c << "        PyObject     *tuple;" << Endl;
        c << "        if(!PyArg_ParseTuple(args, \"O\", &tuple))" << Endl;
        c << "            return NULL;" << Endl;
        c << Endl;
        c << "        if(PyTuple_Check(tuple))" << Endl;
        c << "        {" << Endl;
        c << "            if(PyTuple_Size(tuple) != " << length << ")" << Endl;
        c << "                return NULL;" << Endl;
        c << Endl;
        c << "            PyErr_Clear();" << Endl;
        c << "            for(int i = 0; i < PyTuple_Size(tuple); ++i)" << Endl;
        c << "            {" << Endl;
        c << "                PyObject *item = PyTuple_GET_ITEM(tuple, i);" << Endl;
        c << "                if(PyFloat_Check(item))" << Endl;
        c << "                    fvals[i] = float(PyFloat_AS_DOUBLE(item));" << Endl;
        c << "                else if(PyInt_Check(item))" << Endl;
        c << "                    fvals[i] = float(PyInt_AS_LONG(item));" << Endl;
        c << "                else if(PyLong_Check(item))" << Endl;
        c << "                    fvals[i] = float(PyLong_AsDouble(item));" << Endl;
        c << "                else" << Endl;
        c << "                    fvals[i] = 0.;" << Endl;
        c << "            }" << Endl;
        c << "        }" << Endl;
        c << "        else" << Endl;
        c << "            return NULL;" << Endl;
        c << "    }" << Endl;
        c << Endl;
        c << "    // Mark the " << name << " in the object as modified." << Endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << Endl;
        else
            c << "    obj->data->Select" << Name << "();" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << Endl;
        c << "    PyObject *retval = PyTuple_New(" << length << ");" << Endl;
        c << "    const float *" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << ";" << Endl;
        else
            c << MethodNameGet() << "();" << Endl;
        c << "    for(int i = 0; i < " << length << "; ++i)" << Endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyFloat_FromDouble(double(" << name << "[i])));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    {   const float *" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "        for(int i = 0; i < " << length << "; ++i)" << Endl;
        c << "        {" << Endl;
        c << "            SNPRINTF(tmpStr, 1000, \"%g\", " << name << "[i]);" << Endl;
        c << "            str += tmpStr;" << Endl;
        c << "            if(i < " << length - 1 << ")" << Endl;
        c << "            {" << Endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << Endl;
        c << "                str += tmpStr;" << Endl;
        c << "            }" << Endl;
        c << "        }" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "    }" << Endl;
    }
};


//
// ---------------------------------- Double ----------------------------------
//
class AttsGeneratorDouble : public virtual Double , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorDouble(const QString &n, const QString &l)
        : Field("double",n,l), Double(n,l), PythonGeneratorField("double",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    double dval;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"d\", &dval))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Set the " << name << " in the object." << Endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = dval;" << Endl;
        else
            c << MethodNameSet() << "(dval);" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    PyObject *retval = PyFloat_FromDouble(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ");" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = %g\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ");" << Endl;
        c << "    str += tmpStr;" << Endl;
    }
};


//
// -------------------------------- DoubleArray -------------------------------
//
class AttsGeneratorDoubleArray : public virtual DoubleArray , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorDoubleArray(const QString &s, const QString &n, const QString &l)
        : Field("doubleArray",n,l), DoubleArray(s,n,l), PythonGeneratorField("doubleArray",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    double *dvals = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
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
        c << "))" << Endl;
        c << "    {" << Endl;
        c << "        PyObject     *tuple;" << Endl;
        c << "        if(!PyArg_ParseTuple(args, \"O\", &tuple))" << Endl;
        c << "            return NULL;" << Endl;
        c << Endl;
        c << "        if(PyTuple_Check(tuple))" << Endl;
        c << "        {" << Endl;
        c << "            if(PyTuple_Size(tuple) != " << length << ")" << Endl;
        c << "                return NULL;" << Endl;
        c << Endl;
        c << "            PyErr_Clear();" << Endl;
        c << "            for(int i = 0; i < PyTuple_Size(tuple); ++i)" << Endl;
        c << "            {" << Endl;
        c << "                PyObject *item = PyTuple_GET_ITEM(tuple, i);" << Endl;
        c << "                if(PyFloat_Check(item))" << Endl;
        c << "                    dvals[i] = PyFloat_AS_DOUBLE(item);" << Endl;
        c << "                else if(PyInt_Check(item))" << Endl;
        c << "                    dvals[i] = double(PyInt_AS_LONG(item));" << Endl;
        c << "                else if(PyLong_Check(item))" << Endl;
        c << "                    dvals[i] = PyLong_AsDouble(item);" << Endl;
        c << "                else" << Endl;
        c << "                    dvals[i] = 0.;" << Endl;
        c << "            }" << Endl;
        c << "        }" << Endl;
        c << "        else" << Endl;
        c << "            return NULL;" << Endl;
        c << "    }" << Endl;
        c << Endl;
        c << "    // Mark the " << name << " in the object as modified." << Endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << Endl;
        else
            c << "    obj->data->Select" << Name << "();" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << Endl;
        c << "    PyObject *retval = PyTuple_New(" << length << ");" << Endl;
        c << "    const double *" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ";" << Endl;
        c << "    for(int i = 0; i < " << length << "; ++i)" << Endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyFloat_FromDouble(" << name << "[i]));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    {   const double *" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "        for(int i = 0; i < " << length << "; ++i)" << Endl;
        c << "        {" << Endl;
        c << "            SNPRINTF(tmpStr, 1000, \"%g\", " << name << "[i]);" << Endl;
        c << "            str += tmpStr;" << Endl;
        c << "            if(i < " << length - 1 << ")" << Endl;
        c << "            {" << Endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << Endl;
        c << "                str += tmpStr;" << Endl;
        c << "            }" << Endl;
        c << "        }" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "    }" << Endl;
    }
};


//
// ------------------------------- DoubleVector -------------------------------
//
class AttsGeneratorDoubleVector : public virtual DoubleVector , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorDoubleVector(const QString &n, const QString &l)
        : Field("doubleVector",n,l), DoubleVector(n,l), PythonGeneratorField("doubleVector",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    doubleVector  &vec = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
        c << "    PyObject     *tuple;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"O\", &tuple))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    if(PyTuple_Check(tuple))" << Endl;
        c << "    {" << Endl;
        c << "        vec.resize(PyTuple_Size(tuple));" << Endl;
        c << "        for(int i = 0; i < PyTuple_Size(tuple); ++i)" << Endl;
        c << "        {" << Endl;
        c << "            PyObject *item = PyTuple_GET_ITEM(tuple, i);" << Endl;
        c << "            if(PyFloat_Check(item))" << Endl;
        c << "                vec[i] = PyFloat_AS_DOUBLE(item);" << Endl;
        c << "            else if(PyInt_Check(item))" << Endl;
        c << "                vec[i] = double(PyInt_AS_LONG(item));" << Endl;
        c << "            else if(PyLong_Check(item))" << Endl;
        c << "                vec[i] = PyLong_AsDouble(item);" << Endl;
        c << "            else" << Endl;
        c << "                vec[i] = 0.;" << Endl;
        c << "        }" << Endl;
        c << "    }" << Endl;
        c << "    else if(PyFloat_Check(tuple))" << Endl;
        c << "    {" << Endl;
        c << "        vec.resize(1);" << Endl;
        c << "        vec[0] = PyFloat_AS_DOUBLE(tuple);" << Endl;
        c << "    }" << Endl;
        c << "    else if(PyInt_Check(tuple))" << Endl;
        c << "    {" << Endl;
        c << "        vec.resize(1);" << Endl;
        c << "        vec[0] = double(PyInt_AS_LONG(tuple));" << Endl;
        c << "    }" << Endl;
        c << "    else if(PyLong_Check(tuple))" << Endl;
        c << "    {" << Endl;
        c << "        vec.resize(1);" << Endl;
        c << "        vec[0] = PyLong_AsDouble(tuple);" << Endl;
        c << "    }" << Endl;
        c << "    else" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Mark the "<<name<<" in the object as modified." << Endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << Endl;
        else
            c << "    obj->data->Select"<<Name<<"();" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << Endl;
        c << "    const doubleVector &" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ";" << Endl;
        c << "    PyObject *retval = PyTuple_New(" << name << ".size());" << Endl;
        c << "    for(size_t i = 0; i < "<<name<<".size(); ++i)" << Endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyFloat_FromDouble(" << name << "[i]));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    {   const doubleVector &" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "        for(size_t i = 0; i < " << name << ".size(); ++i)" << Endl;
        c << "        {" << Endl;
        c << "            SNPRINTF(tmpStr, 1000, \"%g\", " << name << "[i]);" << Endl;
        c << "            str += tmpStr;" << Endl;
        c << "            if(i < " << name << ".size() - 1)" << Endl;
        c << "            {" << Endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << Endl;
        c << "                str += tmpStr;" << Endl;
        c << "            }" << Endl;
        c << "        }" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "    }" << Endl;
    }
};


//
// ----------------------------------- UChar ----------------------------------
//
class AttsGeneratorUChar : public virtual UChar , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorUChar(const QString &n, const QString &l)
        : Field("uchar",n,l), UChar(n,l), PythonGeneratorField("uchar",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    unsigned char uval;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"c\", &uval))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Set the " << name << " in the object." << Endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = uval;" << Endl;
        else
            c << MethodNameSet() << "(uval);" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    PyObject *retval = PyInt_FromLong(long(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c <<"));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = %d\\n\", prefix, int(atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << "));" << Endl;
        c << "    str += tmpStr;" << Endl;
    }
};


//
// -------------------------------- UCharArray --------------------------------
//
class AttsGeneratorUCharArray : public virtual UCharArray , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorUCharArray(const QString &s, const QString &n, const QString &l)
        : Field("ucharArray",n,l), UCharArray(s,n,l), PythonGeneratorField("ucharArray",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    unsigned char *cvals = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
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
        c << "))" << Endl;
        c << "    {" << Endl;
        c << "        PyObject     *tuple;" << Endl;
        c << "        if(!PyArg_ParseTuple(args, \"O\", &tuple))" << Endl;
        c << "            return NULL;" << Endl;
        c << Endl;
        c << "        if(PyTuple_Check(tuple))" << Endl;
        c << "        {" << Endl;
        c << "            if(PyTuple_Size(tuple) != " << length << ")" << Endl;
        c << "                return NULL;" << Endl;
        c << Endl;
        c << "            PyErr_Clear();" << Endl;
        c << "            for(int i = 0; i < PyTuple_Size(tuple); ++i)" << Endl;
        c << "            {" << Endl;
        c << "                int c;" << Endl;
        c << "                PyObject *item = PyTuple_GET_ITEM(tuple, i);" << Endl;
        c << "                if(PyFloat_Check(item))" << Endl;
        c << "                    c = int(PyFloat_AS_DOUBLE(item));" << Endl;
        c << "                else if(PyInt_Check(item))" << Endl;
        c << "                    c = int(PyInt_AS_LONG(item));" << Endl;
        c << "                else if(PyLong_Check(item))" << Endl;
        c << "                    c = int(PyLong_AsDouble(item));" << Endl;
        c << "                else" << Endl;
        c << "                    c = 0;" << Endl;
        c << Endl;
        c << "                if(c < 0) c = 0;" << Endl;
        c << "                if(c > 255) c = 255;" << Endl;
        c << "                cvals[i] = (unsigned char)(c);" << Endl;
        c << "            }" << Endl;
        c << "        }" << Endl;
        c << "        else" << Endl;
        c << "            return NULL;" << Endl;
        c << "    }" << Endl;
        c << Endl;
        c << "    // Mark the " << name << " in the object as modified." << Endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << Endl;
        else
            c << "    obj->data->Select" << Name << "();" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << Endl;
        c << "    PyObject *retval = PyTuple_New(" << length << ");" << Endl;
        c << "    const unsigned char *" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ";" << Endl;
        c << "    for(int i = 0; i < " << length << "; ++i)" << Endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyInt_FromLong(long(" << name << "[i])));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    {   const unsigned char *" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "        for(int i = 0; i < " << length << "; ++i)" << Endl;
        c << "        {" << Endl;
        c << "            SNPRINTF(tmpStr, 1000, \"%d\", int(" << name << "[i]));" << Endl;
        c << "            str += tmpStr;" << Endl;
        c << "            if(i < " << length - 1 << ")" << Endl;
        c << "            {" << Endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << Endl;
        c << "                str += tmpStr;" << Endl;
        c << "            }" << Endl;
        c << "        }" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "    }" << Endl;
    }
};


//
// ------------------------------- UCharVector -------------------------------
//
class AttsGeneratorUCharVector : public virtual UCharVector , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorUCharVector(const QString &n, const QString &l)
        : Field("ucharVector",n,l), UCharVector(n,l), PythonGeneratorField("ucharVector",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    unsignedCharVector  &vec = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
        c << "    PyObject     *tuple;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"O\", &tuple))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    if(PyTuple_Check(tuple))" << Endl;
        c << "    {" << Endl;
        c << "        vec.resize(PyTuple_Size(tuple));" << Endl;
        c << "        for(int i = 0; i < PyTuple_Size(tuple); ++i)" << Endl;
        c << "        {" << Endl;
        c << "            int c;" << Endl;
        c << "            PyObject *item = PyTuple_GET_ITEM(tuple, i);" << Endl;
        c << "            if(PyFloat_Check(item))" << Endl;
        c << "                c = int(PyFloat_AS_DOUBLE(item));" << Endl;
        c << "            else if(PyInt_Check(item))" << Endl;
        c << "                c = int(PyInt_AS_LONG(item));" << Endl;
        c << "            else if(PyLong_Check(item))" << Endl;
        c << "                c = int(PyLong_AsDouble(item));" << Endl;
        c << "            else" << Endl;
        c << "                c = 0;" << Endl;
        c << Endl;
        c << "            if(c < 0) c = 0;" << Endl;
        c << "            if(c > 255) c = 255;" << Endl;
        c << "            vec[i] = (unsigned char)(c);" << Endl;
        c << "        }" << Endl;
        c << "    }" << Endl;
        c << "    else if(PyFloat_Check(tuple))" << Endl;
        c << "    {" << Endl;
        c << "        vec.resize(1);" << Endl;
        c << "        int c = int(PyFloat_AS_DOUBLE(tuple));" << Endl;
        c << "        if(c < 0) c = 0;" << Endl;
        c << "        if(c > 255) c = 255;" << Endl;
        c << "        vec[0] = (unsigned char)(c);" << Endl;
        c << "    }" << Endl;
        c << "    else if(PyInt_Check(tuple))" << Endl;
        c << "    {" << Endl;
        c << "        vec.resize(1);" << Endl;
        c << "        int c = int(PyInt_AS_LONG(tuple));" << Endl;
        c << "        if(c < 0) c = 0;" << Endl;
        c << "        if(c > 255) c = 255;" << Endl;
        c << "        vec[0] = (unsigned char)(c);" << Endl;
        c << "    }" << Endl;
        c << "    else if(PyLong_Check(tuple))" << Endl;
        c << "    {" << Endl;
        c << "        vec.resize(1);" << Endl;
        c << "        int c = PyLong_AsLong(tuple);" << Endl;
        c << "        if(c < 0) c = 0;" << Endl;
        c << "        if(c > 255) c = 255;" << Endl;
        c << "        vec[0] = (unsigned char)(c);" << Endl;
        c << "    }" << Endl;
        c << "    else" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Mark the "<<name<<" in the object as modified." << Endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << Endl;
        else
            c << "    obj->data->Select"<<Name<<"();" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << Endl;
        c << "    const unsignedCharVector &" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ";" << Endl;
        c << "    PyObject *retval = PyTuple_New(" << name << ".size());" << Endl;
        c << "    for(size_t i = 0; i < "<<name<<".size(); ++i)" << Endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyInt_FromLong(long(" << name << "[i])));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    {   const unsignedCharVector &" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "        for(size_t i = 0; i < " << name << ".size(); ++i)" << Endl;
        c << "        {" << Endl;
        c << "            SNPRINTF(tmpStr, 1000, \"%d\", int(" << name << "[i]));" << Endl;
        c << "            str += tmpStr;" << Endl;
        c << "            if(i < " << name << ".size() - 1)" << Endl;
        c << "            {" << Endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << Endl;
        c << "                str += tmpStr;" << Endl;
        c << "            }" << Endl;
        c << "        }" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "    }" << Endl;
    }
};

//
// ---------------------------------- String ----------------------------------
//
class AttsGeneratorString : public virtual String , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorString(const QString &n, const QString &l)
        : Field("string",n,l), String(n,l), PythonGeneratorField("string",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    char *str;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"s\", &str))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Set the " << name << " in the object." << Endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = std::string(str);" << Endl;
        else
            c << MethodNameSet() << "(std::string(str));" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    PyObject *retval = PyString_FromString(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ".c_str());" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = \\\"%s\\\"\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ".c_str());" << Endl;
        c << "    str += tmpStr;" << Endl;
    }
};


//
// ------------------------------- StringVector -------------------------------
//
class AttsGeneratorStringVector : public virtual StringVector , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorStringVector(const QString &n, const QString &l)
        : Field("stringVector",n,l), StringVector(n,l), PythonGeneratorField("stringVector",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    stringVector  &vec = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
        c << "    PyObject     *tuple;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"O\", &tuple))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    if(PyTuple_Check(tuple))" << Endl;
        c << "    {" << Endl;
        c << "        vec.resize(PyTuple_Size(tuple));" << Endl;
        c << "        for(int i = 0; i < PyTuple_Size(tuple); ++i)" << Endl;
        c << "        {" << Endl;
        c << "            PyObject *item = PyTuple_GET_ITEM(tuple, i);" << Endl;
        c << "            if(PyString_Check(item))" << Endl;
        c << "                vec[i] = std::string(PyString_AS_STRING(item));" << Endl;
        c << "            else" << Endl;
        c << "                vec[i] = std::string(\"\");" << Endl;
        c << "        }" << Endl;
        c << "    }" << Endl;
        c << "    else if(PyString_Check(tuple))" << Endl;
        c << "    {" << Endl;
        c << "        vec.resize(1);" << Endl;
        c << "        vec[0] = std::string(PyString_AS_STRING(tuple));" << Endl;
        c << "    }" << Endl;
        c << "    else" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Mark the "<<name<<" in the object as modified." << Endl;
        if(accessType == Field::AccessPublic)
            c << "    obj->data->SelectAll();" << Endl;
        else
            c << "    obj->data->Select"<<Name<<"();" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << Endl;
        c << "    const stringVector &" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ";" << Endl;
        c << "    PyObject *retval = PyTuple_New(" << name << ".size());" << Endl;
        c << "    for(size_t i = 0; i < "<<name<<".size(); ++i)" << Endl;
        c << "        PyTuple_SET_ITEM(retval, i, PyString_FromString(" << name << "[i].c_str()));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    {   const stringVector &" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ";" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = (\", prefix);" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "        for(size_t i = 0; i < " << name << ".size(); ++i)" << Endl;
        c << "        {" << Endl;
        c << "            SNPRINTF(tmpStr, 1000, \"\\\"%s\\\"\", " << name << "[i].c_str());" << Endl;
        c << "            str += tmpStr;" << Endl;
        c << "            if(i < " << name << ".size() - 1)" << Endl;
        c << "            {" << Endl;
        c << "                SNPRINTF(tmpStr, 1000, \", \");" << Endl;
        c << "                str += tmpStr;" << Endl;
        c << "            }" << Endl;
        c << "        }" << Endl;
        c << "        SNPRINTF(tmpStr, 1000, \")\\n\");" << Endl;
        c << "        str += tmpStr;" << Endl;
        c << "    }" << Endl;
    }
};


//
// -------------------------------- ColorTable --------------------------------
//
class AttsGeneratorColorTable : public virtual ColorTable , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorColorTable(const QString &n, const QString &l)
        : Field("colortable",n,l), ColorTable(n,l), PythonGeneratorField("colortable",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    char *str;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"s\", &str))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Set the " << name << " in the object." << Endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = std::string(str);" << Endl;
        else
            c << MethodNameSet() << "(std::string(str));" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    PyObject *retval = PyString_FromString(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c <<".c_str());" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = \\\"%s\\\"\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ".c_str());" << Endl;
        c << "    str += tmpStr;" << Endl;
    }
};


//
// ----------------------------------- Color ----------------------------------
//
class AttsGeneratorColor : public virtual Color , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorColor(const QString &n, const QString &l)
        : Field("color",n,l), Color(n,l), PythonGeneratorField("color",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    int c[4];" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"iiii\", &c[0], &c[1], &c[2], &c[3]))" << Endl;
        c << "    {" << Endl;
        c << "        c[3] = 255;" << Endl;
        c << "        if(!PyArg_ParseTuple(args, \"iii\", &c[0], &c[1], &c[2]))" << Endl;
        c << "        {" << Endl;
        c << "            double dr, dg, db, da;" << Endl;
        c << "            if(PyArg_ParseTuple(args, \"dddd\", &dr, &dg, &db, &da))" << Endl;
        c << "            {" << Endl;
        c << "                c[0] = int(dr);" << Endl;
        c << "                c[1] = int(dg);" << Endl;
        c << "                c[2] = int(db);" << Endl;
        c << "                c[3] = int(da);" << Endl;
        c << "            }" << Endl;
        c << "            else if(PyArg_ParseTuple(args, \"ddd\", &dr, &dg, &db))" << Endl;
        c << "            {" << Endl;
        c << "                c[0] = int(dr);" << Endl;
        c << "                c[1] = int(dg);" << Endl;
        c << "                c[2] = int(db);" << Endl;
        c << "                c[3] = 255;" << Endl;
        c << "            }" << Endl;
        c << "            else" << Endl;
        c << "            {" << Endl;
        c << "                PyObject *tuple = NULL;" << Endl;
        c << "                if(!PyArg_ParseTuple(args, \"O\", &tuple))" << Endl;
        c << "                    return NULL;" << Endl;
        c << Endl;
        c << "                if(!PyTuple_Check(tuple))" << Endl;
        c << "                    return NULL;" << Endl;
        c << Endl;
        c << "                // Make sure that the tuple is the right size." << Endl;
        c << "                if(PyTuple_Size(tuple) < 3 || PyTuple_Size(tuple) > 4)" << Endl;
        c << "                    return NULL;" << Endl;
        c << Endl;
        c << "                // Make sure that all elements in the tuple are ints." << Endl;
        c << "                for(int i = 0; i < PyTuple_Size(tuple); ++i)" << Endl;
        c << "                {" << Endl;
        c << "                    PyObject *item = PyTuple_GET_ITEM(tuple, i);" << Endl;
        c << "                    if(PyInt_Check(item))" << Endl;
        c << "                        c[i] = int(PyInt_AS_LONG(PyTuple_GET_ITEM(tuple, i)));" << Endl;
        c << "                    else if(PyFloat_Check(item))" << Endl;
        c << "                        c[i] = int(PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(tuple, i)));" << Endl;
        c << "                    else" << Endl;
        c << "                        return NULL;" << Endl;
        c << "                }" << Endl;
        c << "            }" << Endl;
        c << "        }" << Endl;
        c << "        PyErr_Clear();" << Endl;
        c << "    }" << Endl;
        c << Endl;
        c << "    // Set the " << name << " in the object." << Endl;
        c << "    ColorAttribute ca(c[0], c[1], c[2], c[3]);" << Endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = ca;" << Endl;
        else
            c << MethodNameSet() << "(ca);" << Endl;
    }

    virtual void WriteIncludedHeaders(QTextStream &c)
    {
        // Write the list of include files that the object needs.
        c << "#include <ColorAttribute.h>" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    // Allocate a tuple the with enough entries to hold the " << name << "." << Endl;
        c << "    PyObject *retval = PyTuple_New(4);" << Endl;
        c << "    const unsigned char *" << name << " = obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c << ".GetColor();" << Endl;
        c << "    PyTuple_SET_ITEM(retval, 0, PyInt_FromLong(long(" << name << "[0])));" << Endl;
        c << "    PyTuple_SET_ITEM(retval, 1, PyInt_FromLong(long(" << name << "[1])));" << Endl;
        c << "    PyTuple_SET_ITEM(retval, 2, PyInt_FromLong(long(" << name << "[2])));" << Endl;
        c << "    PyTuple_SET_ITEM(retval, 3, PyInt_FromLong(long(" << name << "[3])));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    const unsigned char *" << name << " = atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ".GetColor();" << Endl;
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = (%d, %d, %d, %d)\\n\", prefix, int("<<name<<"[0]), int("<<name<<"[1]), int("<<name<<"[2]), int("<<name<<"[3]));" << Endl;
        c << "    str += tmpStr;" << Endl;
    }
};


//
// --------------------------------- LineStyle --------------------------------
//
class AttsGeneratorLineStyle : public virtual LineStyle , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorLineStyle(const QString &n, const QString &l)
        : Field("linestyle",n,l), LineStyle(n,l), PythonGeneratorField("linestyle",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    int ival;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &ival))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Set the " << name << " in the object." << Endl;
        c << "    if(ival >= 0 && ival <= 3)" << Endl;
        c << "        obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = ival;" << Endl;
        else
            c << MethodNameSet() << "(ival);" << Endl;
        c << "    else" << Endl;
        c << "    {" << Endl;
        c << "        fprintf(stderr, \"An invalid  value was given. \"" << Endl;
        c << "                        \"Valid values are in the range of [0,3]. \"" << Endl;
        c << "                        \"You can also use the following names: \"" << Endl;
        c << "                        \"\\\"SOLID\\\", \\\"DASH\\\", \\\"DOT\\\", \\\"DOTDASH\\\"\\n\");" << Endl;
        c << "        return NULL;" << Endl;
        c << "    }" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    PyObject *retval = PyInt_FromLong(long(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c <<"));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    const char *" << name << "_values[] = {\"SOLID\", \"DASH\", \"DOT\", \"DOTDASH\"};" << Endl;
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = ";
        c << "%s%s  # SOLID, DASH, DOT, DOTDASH\\n\", prefix, prefix, " << name << "_values[atts->";

        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << "]);" << Endl;
        c << "    str += tmpStr;" << Endl;
    }

    virtual void WriteGetAttr(QTextStream &c, const QString &classname)
    {
        if (internal)
            return;

        c << "    if(strcmp(name, \"" << name << "\") == 0)" << Endl;
        c << "        return " << classname << "_" << MethodNameGet() << "(self, NULL);" << Endl;
        c << "    if(strcmp(name, \"SOLID\") == 0)" << Endl;
        c << "        return PyInt_FromLong(long(0));" << Endl;
        c << "    else if(strcmp(name, \"DASH\") == 0)" << Endl;
        c << "        return PyInt_FromLong(long(1));" << Endl;
        c << "    else if(strcmp(name, \"DOT\") == 0)" << Endl;
        c << "        return PyInt_FromLong(long(2));" << Endl;
        c << "    else if(strcmp(name, \"DOTDASH\") == 0)" << Endl;
        c << "        return PyInt_FromLong(long(3));" << Endl;
        c << Endl;
    }
};


//
// --------------------------------- LineWidth --------------------------------
//
class AttsGeneratorLineWidth : public virtual LineWidth , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorLineWidth(const QString &n, const QString &l)
        : Field("linewidth",n,l), LineWidth(n,l), PythonGeneratorField("linewidth",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    int ival;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &ival))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Set the " << name << " in the object." << Endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = ival;" << Endl;
        else
            c << MethodNameSet() << "(ival);" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    PyObject *retval = PyInt_FromLong(long(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c <<"));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = %d\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ");" << Endl;
        c << "    str += tmpStr;" << Endl;
    }
};


//
// --------------------------------- Opacity ----------------------------------
//
class AttsGeneratorOpacity : public virtual Opacity , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorOpacity(const QString &n, const QString &l)
        : Field("opacity",n,l), Opacity(n,l), PythonGeneratorField("opacity",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    double dval;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"d\", &dval))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Set the " << name << " in the object." << Endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = dval;" << Endl;
        else
            c << MethodNameSet() << "(dval);" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    PyObject *retval = PyFloat_FromDouble(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c <<");" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = %g\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ");" << Endl;
        c << "    str += tmpStr;" << Endl;
    }
};


//
// -------------------------------- VariableName --------------------------------
//
class AttsGeneratorVariableName : public virtual VariableName , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorVariableName(const QString &n, const QString &l)
        : Field("variablename",n,l), VariableName(n,l), PythonGeneratorField("variablename",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    char *str;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"s\", &str))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Set the " << name << " in the object." << Endl;
        c << "    obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = std::string(str);" << Endl;
        else
            c << MethodNameSet() << "(std::string(str));" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    PyObject *retval = PyString_FromString(obj->data->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c <<MethodNameGet()<<"()";
        c<<".c_str());" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = \\\"%s\\\"\\n\", prefix, atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ".c_str());" << Endl;
        c << "    str += tmpStr;" << Endl;
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
//   Mark C. Miller, Mon Mar 31 13:59:27 PDT 2008
//   Fixed typo where 'AttType' was used for some code generation in which
//   'attType' should have been used.
//
// ----------------------------------------------------------------------------

class AttsGeneratorAtt : public virtual Att , public virtual PythonGeneratorField
{
  public:
    AttsGeneratorAtt(const QString &t, const QString &n, const QString &l)
        : Field("att",n,l), Att(t,n,l), PythonGeneratorField("att",n,l) { /*cerr << "**** AttType=" << AttType << endl;*/}

    virtual void WriteIncludedHeaders(QTextStream &c)
    {
        // Write the list of include files that the object needs.
        c << "#include <Py" << attType << ".h>" << Endl;
    }

    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        if(AttType == "ColorAttributeList")
        {
            c << "    PyObject *pyobj = NULL;" << Endl;
            c << "    ColorAttributeList &cL = obj->data->";
            if(accessType == Field::AccessPublic)
                c << name;
            else
                c << MethodNameGet() << "()";
            c << ";" << Endl;
            c << "    int index = 0;" << Endl;
            c << "    int c[4] = {0,0,0,255};" << Endl;
            c << "    bool setTheColor = true;" << Endl;
            c << Endl;
            c << "    if(!PyArg_ParseTuple(args, \"iiiii\", &index, &c[0], &c[1], &c[2], &c[3]))" << Endl;
            c << "    {" << Endl;
            c << "        if(!PyArg_ParseTuple(args, \"iiii\", &index, &c[0], &c[1], &c[2]))" << Endl;
            c << "        {" << Endl;
            c << "            double dr, dg, db, da;" << Endl;
            c << "            if(PyArg_ParseTuple(args, \"idddd\", &index, &dr, &dg, &db, &da))" << Endl;
            c << "            {" << Endl;
            c << "                c[0] = int(dr);" << Endl;
            c << "                c[1] = int(dg);" << Endl;
            c << "                c[2] = int(db);" << Endl;
            c << "                c[3] = int(da);" << Endl;
            c << "            }" << Endl;
            c << "            else if(PyArg_ParseTuple(args, \"iddd\", &index, &dr, &dg, &db))" << Endl;
            c << "            {" << Endl;
            c << "                c[0] = int(dr);" << Endl;
            c << "                c[1] = int(dg);" << Endl;
            c << "                c[2] = int(db);" << Endl;
            c << "                c[3] = 255;" << Endl;
            c << "            }" << Endl;
            c << "            else" << Endl;
            c << "            {" << Endl;
            c << "                if(!PyArg_ParseTuple(args, \"iO\", &index, &pyobj))" << Endl;
            c << "                {" << Endl;
            c << "                    if(PyArg_ParseTuple(args, \"O\", &pyobj))" << Endl;
            c << "                    {" << Endl;
            c << "                        setTheColor = false;" << Endl;
            c << "                        if(PyTuple_Check(pyobj))" << Endl;
            c << "                        {" << Endl;
            c << "                            // Make sure that the tuple is the right size." << Endl;
            c << "                            if(PyTuple_Size(pyobj) < cL.GetNumColors())" << Endl;
            c << "                                return NULL;" << Endl;
            c << Endl;
            c << "                            // Make sure that the tuple is the right size." << Endl;
            c << "                            bool badInput = false;" << Endl;
            c << "                            int *C = new int[4 * cL.GetNumColors()];" << Endl;
            c << "                            for(int i = 0; i < PyTuple_Size(pyobj) && !badInput; ++i)" << Endl;
            c << "                            {" << Endl;
            c << "                                PyObject *item = PyTuple_GET_ITEM(pyobj, i);" << Endl;
            c << "                                if(PyTuple_Check(item) &&" << Endl;
            c << "                                   PyTuple_Size(item) == 3 || PyTuple_Size(item) == 4)" << Endl;
            c << "                                {" << Endl;
            c << "                                    C[i*4] = 0;" << Endl;
            c << "                                    C[i*4+1] = 0;" << Endl;
            c << "                                    C[i*4+2] = 0;" << Endl;
            c << "                                    C[i*4+3] = 255;" << Endl;
            c << "                                    for(int j = 0; j < PyTuple_Size(item) && !badInput; ++j)" << Endl;
            c << "                                    {" << Endl;
            c << "                                        PyObject *colorcomp = PyTuple_GET_ITEM(item, j);" << Endl;
            c << "                                        if(PyInt_Check(colorcomp))" << Endl;
            c << "                                           C[i*4+j] = int(PyInt_AS_LONG(colorcomp));" << Endl;
            c << "                                        else if(PyFloat_Check(colorcomp))" << Endl;
            c << "                                           C[i*4+j] = int(PyFloat_AS_DOUBLE(colorcomp));" << Endl;
            c << "                                        else" << Endl;
            c << "                                           badInput = true;" << Endl;
            c << "                                    }" << Endl;
            c << "                                }" << Endl;
            c << "                                else" << Endl;
            c << "                                    badInput = true;" << Endl;
            c << "                            }" << Endl;
            c << Endl;
            c << "                            if(badInput)" << Endl;
            c << "                            {" << Endl;
            c << "                                delete [] C;" << Endl;
            c << "                                return NULL;" << Endl;
            c << "                            }" << Endl;
            c << Endl;
            c << "                            for(int i = 0; i < cL.GetNumColors(); ++i)" << Endl;
            c << "                                cL[i].SetRgba(C[i*4], C[i*4+1], C[i*4+2], C[i*4+3]);" << Endl;
            c << "                            delete [] C;" << Endl;
            c << "                        }" << Endl;
            c << "                        else if(PyList_Check(pyobj))" << Endl;
            c << "                        {" << Endl;
            c << "                            // Make sure that the list is the right size." << Endl;
            c << "                            if(PyList_Size(pyobj) < cL.GetNumColors())" << Endl;
            c << "                                return NULL;" << Endl;
            c << Endl;
            c << "                            // Make sure that the tuple is the right size." << Endl;
            c << "                            bool badInput = false;" << Endl;
            c << "                            int *C = new int[4 * cL.GetNumColors()];" << Endl;
            c << "                            for(int i = 0; i < PyList_Size(pyobj) && !badInput; ++i)" << Endl;
            c << "                            {" << Endl;
            c << "                                PyObject *item = PyList_GET_ITEM(pyobj, i);" << Endl;
            c << "                                if(PyTuple_Check(item) &&" << Endl;
            c << "                                   PyTuple_Size(item) == 3 || PyTuple_Size(item) == 4)" << Endl;
            c << "                                {" << Endl;
            c << "                                    C[i*4] = 0;" << Endl;
            c << "                                    C[i*4+1] = 0;" << Endl;
            c << "                                    C[i*4+2] = 0;" << Endl;
            c << "                                    C[i*4+3] = 255;" << Endl;
            c << "                                    for(int j = 0; j < PyTuple_Size(item) && !badInput; ++j)" << Endl;
            c << "                                    {" << Endl;
            c << "                                        PyObject *colorcomp = PyTuple_GET_ITEM(item, j);" << Endl;
            c << "                                        if(PyInt_Check(colorcomp))" << Endl;
            c << "                                           C[i*4+j] = int(PyInt_AS_LONG(colorcomp));" << Endl;
            c << "                                        else if(PyFloat_Check(colorcomp))" << Endl;
            c << "                                           C[i*4+j] = int(PyFloat_AS_DOUBLE(colorcomp));" << Endl;
            c << "                                        else" << Endl;
            c << "                                           badInput = true;" << Endl;
            c << "                                    }" << Endl;
            c << "                                }" << Endl;
            c << "                                else" << Endl;
            c << "                                    badInput = true;" << Endl;
            c << "                            }" << Endl;
            c << Endl;
            c << "                            if(badInput)" << Endl;
            c << "                            {" << Endl;
            c << "                                delete [] C;" << Endl;
            c << "                                return NULL;" << Endl;
            c << "                            }" << Endl;
            c << Endl;
            c << "                            for(int i = 0; i < cL.GetNumColors(); ++i)" << Endl;
            c << "                                cL[i].SetRgba(C[i*4], C[i*4+1], C[i*4+2], C[i*4+3]);" << Endl;
            c << Endl;
            c << "                            delete [] C;" << Endl;
            c << "                        }" << Endl;
            c << "                        else" << Endl;
            c << "                            return NULL;" << Endl;
            c << "                    }" << Endl;
            c << "                }" << Endl;
            c << "                else" << Endl;
            c << "                {" << Endl;
            c << "                    if(!PyTuple_Check(pyobj))" << Endl;
            c << "                        return NULL;" << Endl;
            c << Endl;
            c << "                    // Make sure that the tuple is the right size." << Endl;
            c << "                    if(PyTuple_Size(pyobj) < 3 || PyTuple_Size(pyobj) > 4)" << Endl;
            c << "                        return NULL;" << Endl;
            c << Endl;
            c << "                    // Make sure that all elements in the tuple are ints." << Endl;
            c << "                    for(int i = 0; i < PyTuple_Size(pyobj); ++i)" << Endl;
            c << "                    {" << Endl;
            c << "                        PyObject *item = PyTuple_GET_ITEM(pyobj, i);" << Endl;
            c << "                        if(PyInt_Check(item))" << Endl;
            c << "                            c[i] = int(PyInt_AS_LONG(PyTuple_GET_ITEM(pyobj, i)));" << Endl;
            c << "                        else if(PyFloat_Check(item))" << Endl;
            c << "                            c[i] = int(PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(pyobj, i)));" << Endl;
            c << "                        else" << Endl;
            c << "                            return NULL;" << Endl;
            c << "                    }" << Endl;
            c << "                }" << Endl;
            c << "            }" << Endl;
            c << "        }" << Endl;
            c << "        PyErr_Clear();" << Endl;
            c << "    }" << Endl;
            c << Endl;
            c << "    if(index < 0 || index >= cL.GetNumColors())" << Endl;
            c << "        return NULL;" << Endl;
            c << Endl;
            c << "    // Set the color in the object." << Endl;
            c << "    if(setTheColor)" << Endl;
            c << "        cL[index] = ColorAttribute(c[0], c[1], c[2], c[3]);" << Endl;
            c << "    cL.SelectColors();" << Endl;
            c << "    obj->data->Select" << Name << "();" << Endl;
        }
        else
        {
            c << "    PyObject *newValue = NULL;" << Endl;
            c << "    if(!PyArg_ParseTuple(args, \"O\", &newValue))" << Endl;
            c << "        return NULL;" << Endl;
            c << "    if(!Py" << attType << "_Check(newValue))" << Endl;
            c << "    {" << Endl;
            c << "        fprintf(stderr, \"The " << name << " field can only be set with " << attType << " objects.\\n\");" << Endl;
            c << "        return NULL;" << Endl;
            c << "    }" << Endl;
            c << Endl;
            c << "    obj->data->Set" << Name << "(*Py" << attType << "_FromPyObject(newValue));" << Endl;
        }
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        if(AttType == "ColorAttributeList")
        {
            c << "    PyObject *retval = NULL;" << Endl;
            c << "    ColorAttributeList &cL = obj->data->Get" << Name << "();" << Endl;
            c << Endl;
            c << "    int index = 0;" << Endl;
            c << "    if(PyArg_ParseTuple(args, \"i\", &index))" << Endl;
            c << "    {" << Endl;
            c << "        if(index < 0 || index >= cL.GetNumColors())" << Endl;
            c << "            return NULL;" << Endl;
            c << Endl;
            c << "        // Allocate a tuple the with enough entries to hold the singleColor." << Endl;
            c << "        retval = PyTuple_New(4);" << Endl;
            c << "        const unsigned char *c = cL.GetColors(index).GetColor();" << Endl;
            c << "        PyTuple_SET_ITEM(retval, 0, PyInt_FromLong(long(c[0])));" << Endl;
            c << "        PyTuple_SET_ITEM(retval, 1, PyInt_FromLong(long(c[1])));" << Endl;
            c << "        PyTuple_SET_ITEM(retval, 2, PyInt_FromLong(long(c[2])));" << Endl;
            c << "        PyTuple_SET_ITEM(retval, 3, PyInt_FromLong(long(c[3])));" << Endl;
            c << "    }" << Endl;
            c << "    else" << Endl;
            c << "    {" << Endl;
            c << "        PyErr_Clear();" << Endl;
            c << Endl;
            c << "        // Return the whole thing." << Endl;
            c << "        retval = PyList_New(cL.GetNumColors());" << Endl;
            c << "        for(int i = 0; i < cL.GetNumColors(); ++i)" << Endl;
            c << "        {" << Endl;
            c << "            const unsigned char *c = cL.GetColors(i).GetColor();" << Endl;
            c << Endl;
            c << "            PyObject *t = PyTuple_New(4);" << Endl;
            c << "            PyTuple_SET_ITEM(t, 0, PyInt_FromLong(long(c[0])));" << Endl;
            c << "            PyTuple_SET_ITEM(t, 1, PyInt_FromLong(long(c[1])));" << Endl;
            c << "            PyTuple_SET_ITEM(t, 2, PyInt_FromLong(long(c[2])));" << Endl;
            c << "            PyTuple_SET_ITEM(t, 3, PyInt_FromLong(long(c[3])));" << Endl;
            c << Endl;
            c << "            PyList_SET_ITEM(retval, i, t);" << Endl;
            c << "        }" << Endl;
            c << "    }" << Endl;
        }
        else
        {
            c << "    // Since the new object will point to data owned by this object," << Endl;
            c << "    // we need to increment the reference count." << Endl;
            c << "    Py_INCREF(self);" << Endl;
            c << Endl;
            c << "    PyObject *retval = Py" << attType << "_Wrap(";
            if(accessType == Field::AccessPublic)
                c << "&obj->data->" << name << ");" << Endl;
            else
                c << "&obj->data->Get" << Name << "());" << Endl;
            c << "    // Set the object's parent so the reference to the parent can be decref'd" << Endl;
            c << "    // when the child goes out of scope." << Endl;
            c << "    Py" << attType << "_SetParent(retval, self);" << Endl;
            c << Endl;
        }
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        if(AttType == "ColorAttributeList")
        {
            c << "    { const ColorAttributeList &cL = atts->";
            if(accessType == Field::AccessPublic)
                c << name;
            else
                c << MethodNameGet() << "()";
            c << ";" << Endl;
            c << "        const char *comment = (prefix==0 || strcmp(prefix,\"\")==0) ? \"# \" : \"\";" << Endl;
            c << "        for(int i = 0; i < cL.GetNumColors(); ++i)" << Endl;
            c << "        {" << Endl;
            c << "            const unsigned char *c = cL[i].GetColor();" << Endl;
            c << "            SNPRINTF(tmpStr, 1000, \"%s%sSet" << Name << "(%d, (%d, %d, %d, %d))\\n\"," << Endl;
            c << "                     comment, prefix, i, int(c[0]), int(c[1]), int(c[2]), int(c[3]));" << Endl;
            c << "            str += tmpStr;" << Endl;
            c << "        }" << Endl;
            c << "    }" << Endl;
        }
        else
        {
            c << "    { // new scope" << Endl;
            c << "        std::string objPrefix(prefix);" << Endl;
            c << "        objPrefix += \"" << name << ".\";" << Endl;
            c << "        str += Py" << attType << "_ToString(";
            if(accessType == Field::AccessPublic)
                c << "atts->" << name;
            else
                c << "&atts->" << MethodNameGet() << "()";
            c << ", objPrefix.c_str());" << Endl;
            c << "    }" << Endl;
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
        : Field("attVector",n,l), AttVector(t,n,l), PythonGeneratorField("attVector",n,l) { }

    virtual void WriteSetAttr(QTextStream &c, const QString &className, bool first)
    {
        // Not implemented yet!
    }

    virtual bool HasSetAttr()
    {
        return false;
    }

    virtual void WriteIncludedHeaders(QTextStream &c)
    {
        // Write the list of include files that the object needs.
        c << "#include <Py" << attType << ".h>" << Endl;
    }

    // Do not allow set methods. Make the user use the Get### method to return a reference
    // to the object.
    virtual bool ProvidesSetMethod() const { return false; }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    int index;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &index))" << Endl;
        c << "        return NULL;" << Endl;
        c << "    if(index < 0 || index >= obj->data->Get" << Name << "().size())" << Endl;
        c << "    {" << Endl;
        c << "        char msg[200];" << Endl;
        c << "        if(obj->data->Get" << Name << "().size() == 0)" << Endl;
        c << "            SNPRINTF(msg, 200, \"The index is invalid because " << name << " is empty.\");" << Endl;
        c << "        else" << Endl;
        c << "            SNPRINTF(msg, 200, \"The index is invalid. Use index values in: [0, %ld).\", obj->data->Get" << Name << "().size());" << Endl;
        c << "        PyErr_SetString(PyExc_IndexError, msg);" << Endl;
        c << "        return NULL;" << Endl;
        c << "    }" << Endl;
        c << Endl;
        c << "    // Since the new object will point to data owned by the this object," << Endl;
        c << "    // we need to increment the reference count." << Endl;
        c << "    Py_INCREF(self);" << Endl;
        c << Endl;
        c << "    PyObject *retval = Py" << attType << "_Wrap(&obj->data->Get" << Name << "(index));" << Endl;
        c << "    // Set the object's parent so the reference to the parent can be decref'd" << Endl;
        c << "    // when the child goes out of scope." << Endl;
        c << "    Py" << attType << "_SetParent(retval, self);" << Endl;
        c << Endl;
    }

    virtual void WriteAdditionalMethods(QTextStream &c, const QString &className)
    {
        c << "PyObject *" << Endl;
        c << className << "_GetNum" << Name << "(PyObject *self, PyObject *args)" << Endl;
        c << "{" << Endl;
        c << "    " << className << "Object *obj = ("<<className<<"Object *)self;" << Endl;
        c << "    return PyInt_FromLong((long)obj->data->Get" << Name << "().size());" << Endl;
        c << "}" << Endl;
        c << Endl;

        c << "PyObject *" << Endl;
        c << className << "_Add" << Name << "(PyObject *self, PyObject *args)" << Endl;
        c << "{" << Endl;
        c << "    " << className << "Object *obj = ("<<className<<"Object *)self;" << Endl;
        c << "    PyObject *element = NULL;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"O\", &element))" << Endl;
        c << "        return NULL;" << Endl;
        c << "    if(!Py" << attType << "_Check(element))" << Endl; 
        c << "    {" << Endl;
        c << "        char msg[400];" << Endl;
        c << "        SNPRINTF(msg, 400, \"The Add" << Name << " method only accepts " << attType << " objects.\");" << Endl;
        c << "        PyErr_SetString(PyExc_TypeError, msg);" << Endl;
        c << "        return NULL;" << Endl;
        c << "    }" << Endl;
        c << "    " << attType << " *newData = Py" << attType << "_FromPyObject(element);" << Endl;
        if(accessType != Field::AccessPublic)
        {
            c << "    obj->data->Add" << Name << "(*newData);" << Endl;
            c << "    obj->data->Select" << Name << "();" << Endl;
        }
        else
        {
            c << "    obj->data->" << name << ".push_back(new " << attType << "(*newData));" << Endl;
        }
        c << "    Py_INCREF(Py_None);" << Endl;
        c << "    return Py_None;" << Endl;
        c << "}" << Endl;
        c << Endl;

        c << "static PyObject *" << Endl;
        c << className << "_Remove_One_" << Name << "(PyObject *self, int index)" << Endl;
        c << "{" << Endl;
        c << "    " << className << "Object *obj = ("<<className<<"Object *)self;" << Endl;
        c << "    // Remove in the AttributeGroupVector instead of calling Remove" << Name
          << "() because we don't want to delete the object; just remove it." << Endl;
        if(accessType == Field::AccessPublic)
            c << "    AttributeGroupVector &atts = obj->data->" << name << ";" << Endl;
        else
            c << "    AttributeGroupVector &atts = obj->data->Get" << Name << "();" << Endl;
        c << "    AttributeGroupVector::iterator pos = atts.begin();" << Endl;
        c << "    // Iterate through the vector \"index\" times. " << Endl;
        c << "    for(int i = 0; i < index; ++i)" << Endl;
        c << "        ++pos;" << Endl;
        c << Endl;
        c << "    // If pos is still a valid iterator, remove that element." << Endl;
        c << "    if(pos != atts.end())" << Endl;
        c << "    {" << Endl;
        c << "        // NOTE: Leak the object since other Python objects may reference it. Ideally," << Endl;
        c << "        // we would put the object into some type of pool to be cleaned up later but" << Endl;
        c << "        // this will do for now." << Endl;
        c << "        //" << Endl;
        c << "        // delete *pos;" << Endl;
        c << "        atts.erase(pos);" << Endl;
        c << "    }" << Endl;
        c << Endl;
        if(accessType != Field::AccessPublic)
            c << "    obj->data->Select" << Name << "();" << Endl;
        c << "    Py_INCREF(Py_None);" << Endl;
        c << "    return Py_None;" << Endl;
        c << "}" << Endl;
        c << Endl;

        c << "PyObject *" << Endl;
        c << className << "_Remove" << Name << "(PyObject *self, PyObject *args)" << Endl;
        c << "{" << Endl;
        c << "    int index;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &index))" << Endl;
        c << "        return NULL;" << Endl;
        c << "    " << className << "Object *obj = ("<<className<<"Object *)self;" << Endl;
        if(accessType == Field::AccessPublic)
            c << "    if(index < 0 || index >= obj->data->" << name << ".size())" << Endl;
        else
            c << "    if(index < 0 || index >= obj->data->GetNum" << Name << "())" << Endl;
        c << "    {" << Endl;
        c << "        PyErr_SetString(PyExc_IndexError, \"Index out of range\");" << Endl;
        c << "        return NULL;" << Endl;
        c << "    }" << Endl;
        c << Endl;
        c << "    return " << className << "_Remove_One_" << Name << "(self, index);" << Endl;
        c << "}" << Endl;
        c << Endl;

        c << "PyObject *" << Endl;
        c << className << "_Clear" << Name << "(PyObject *self, PyObject *args)" << Endl;
        c << "{" << Endl;
        c << "    " << className << "Object *obj = ("<<className<<"Object *)self;" << Endl;
        if(accessType == Field::AccessPublic)
            c << "    int n = obj->data->" << name << ".size();" << Endl;
        else
            c << "    int n = obj->data->GetNum" << Name << "();" << Endl; 
        c << "    for(int i = 0; i < n; ++i)" << Endl;
        c << "    {" << Endl;
        c << "        " << className << "_Remove_One_" << Name << "(self, 0);" << Endl;
        c << "        Py_DECREF(Py_None);" << Endl;
        c << "    }" << Endl;
        c << "    Py_INCREF(Py_None);" << Endl;
        c << "    return Py_None;" << Endl;
        c << "}" << Endl;
        c << Endl;
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

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    { // new scope" << Endl;
        c << "        int index = 0;" << Endl;
        c << "        // Create string representation of " << name << " from atts." << Endl;
        if(accessType == Field::AccessPublic)
            c << "        for(AttributeGroupVector::const_iterator pos = atts->" << name << ".begin(); pos != atts->" << name << ".end(); ++pos, ++index)" << Endl;
        else
            c << "        for(AttributeGroupVector::const_iterator pos = atts->" << MethodNameGet() << "().begin(); pos != atts->" << MethodNameGet() << "().end(); ++pos, ++index)" << Endl;
        c << "        {" << Endl;
        c << "            const " << attType << " *current" << " = (const " << attType << " *)(*pos);" << Endl;
        c << "            SNPRINTF(tmpStr, 1000, \"Get" << Name << "(%d).\", index);" << Endl;
        c << "            std::string objPrefix(prefix + std::string(tmpStr));" << Endl;
        c << "            str += Py" << attType << "_ToString(current, objPrefix.c_str());" << Endl;
        c << "        }" << Endl;
        c << "        if(index == 0)" << Endl;
        c << "            str += \"#" << name << " does not contain any " << attType << " objects.\\n\";" << Endl;
        c << "    }" << Endl;
    }
};


//
// ----------------------------------- Enum -----------------------------------
//
//    Mark C. Miller, Wed Aug 19 15:25:14 PDT 2009
//    Added missing logic for accesType in WriteGetMethodBody
class PythonGeneratorEnum : public virtual Enum , public virtual PythonGeneratorField
{
  public:
    PythonGeneratorEnum(const QString &t, const QString &n, const QString &l)
        : Field("enum",n,l), Enum(t,n,l), PythonGeneratorField("enum",n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    int ival;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &ival))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Set the " << name << " in the object." << Endl;
        c << "    if(ival >= 0 && ival < " << enumType->values.size() << ")" << Endl;
        c << "        obj->data->";
        if(accessType == Field::AccessPublic)
            c << name << " = " << GetCPPName(true,className) << "(ival);" << Endl;
        else
            c << MethodNameSet() << "(" << GetCPPName(true,className) << "(ival));" << Endl;
        c << "    else" << Endl;
        c << "    {" << Endl;
        c << "        fprintf(stderr, \"An invalid " << name 
          << " value was given. \"" << Endl;
        c << "                        \"Valid values are in the range of [0," 
          << enumType->values.size()-1 << "]. \"" << Endl;
        c << "                        \"You can also use the following names: \"" 
          << Endl;
        c << "                        \"";
        for(size_t i = 0; i < enumType->values.size(); ++i)
        {
            c << enumType->values[i];
            if(i < enumType->values.size() - 1)
                c << ", ";
            if(i  > 0 && i%4==0)
                c << "\"\n                        \"";
        }
        c << ".\");" << Endl;
        c << "        return NULL;" << Endl;
        c << "    }"<< Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    PyObject *retval = PyInt_FromLong(long(obj->data->";
        if(accessType == AccessPublic)
            c << name;
        else
            c << MethodNameGet()<<"()";
        c << "));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        // Create a string that shows the possible values.
        c << "    const char *" << name << "_names = \"";
        for(size_t j = 0; j < enumType->values.size(); ++j)
        {
            c << enumType->values[j];
            if(j < enumType->values.size() - 1)
                c << ", ";
            if(j  > 0 && j%4==0 && j != enumType->values.size()-1)
            {
                c << "\"\n        \"";
            }
        }
        c << "\";" << Endl;

        c << "    switch (atts->";
        if(accessType == Field::AccessPublic)
            c << name;
        else
            c << MethodNameGet() << "()";
        c << ")\n    {\n";

        for(size_t i = 0; i < enumType->values.size(); ++i)
        {
            c << "      case " << classname << "::" << enumType->values[i] << ":\n";
            c << "          SNPRINTF(tmpStr, 1000, \"%s" << name << " = %s" 
              << enumType->values[i] << "  # %s\\n\", prefix, prefix, " 
              << name << "_names);" << Endl;
            c << "          str += tmpStr;" << Endl;
            c << "          break;" << Endl;
        }
        c << "      default:" << Endl;
        c << "          break;\n    }" << Endl;

        c << Endl;
    }

    virtual void WriteGetAttr(QTextStream &c, const QString &classname)
    {
        if (internal)
            return;

        c << "    if(strcmp(name, \"" << name << "\") == 0)" << Endl;
        c << "        return " << classname << "_" << MethodNameGet() 
          << "(self, NULL);" << Endl;

        for(size_t i = 0; i < enumType->values.size(); ++i)
        {
            c << "    ";
            c << "if";
            c << "(strcmp(name, \"";
            c << enumType->values[i];
            c << "\") == 0)" << Endl;
            c << "        return PyInt_FromLong(long(";
            c << classname << "::" << enumType->values[i];
            c << "));" << Endl;
        }
        c << Endl;
    }
};

#define AVT_GENERATOR_METHODS \
    virtual void WriteIncludedHeaders(QTextStream &c) \
    { \
        c << "#include <avtTypes.h>" << Endl; \
    } \
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className) \
    { \
        c << "    int ival;" << Endl; \
        c << "    if(!PyArg_ParseTuple(args, \"i\", &ival))" << Endl; \
        c << "        return NULL;" << Endl; \
        c << Endl; \
        QString T(type); T.replace("Field", "");\
        if(accessType == AccessPublic) \
            c << "    obj->data->" << name << " = (" << T << ")ival;" << Endl; \
        else \
            c << "    obj->data->" << MethodNameSet() << "((" << T << ")ival);" << Endl; \
    } \
    virtual void WriteGetMethodBody(QTextStream &c, const QString &className) \
    { \
        c << "    PyObject *retval = PyInt_FromLong(long(obj->data->"; \
        if(accessType == AccessPublic) \
            c << name; \
        else \
            c << MethodNameGet()<<"()"; \
        c << "));" << Endl; \
    } \
    virtual void StringRepresentation(QTextStream &c, const QString &classname) \
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
        c << "\";" << Endl; \
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
                c << " == " << values[i] << ")" << Endl; \
            } \
            else if(i < values_size - 1) \
            { \
                c << "else if"; \
                c << "(atts->"; \
                if(accessType == Field::AccessPublic) \
                    c << name; \
                else \
                    c << MethodNameGet() << "()"; \
                c << " == " << values[i] << ")" << Endl; \
            } \
            else \
                c << "else" << Endl; \
            c << "    {" << Endl; \
            c << "        SNPRINTF(tmpStr, 1000, \"%s" << name << " = %s" << values[i] << "  # %s\\n\", prefix, prefix, " << name << "_names);" << Endl; \
            c << "        str += tmpStr;" << Endl; \
            c << "    }" << Endl; \
        } \
        c << Endl; \
    }\
    virtual void WriteGetAttr(QTextStream &c, const QString &classname)\
    {\
        if (internal)\
            return;\
        c << "    if(strcmp(name, \"" << name << "\") == 0)" << Endl;\
        c << "        return " << classname << "_" << MethodNameGet() << "(self, NULL);" << Endl;\
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
            c << "\") == 0)" << Endl;\
            c << "        return PyInt_FromLong(long(";\
            c << values[i];\
            c << "));" << Endl;\
        }\
        c << Endl;\
    }

//
// ----------------------------------- avtCentering -----------------------------------
//
class AttsGeneratoravtCentering : public virtual PythonGeneratorField, public virtual avtCenteringField
{
  public:
    AttsGeneratoravtCentering(const QString &n, const QString &l)
        : Field("avtCentering",n,l), PythonGeneratorField("avtCentering",n,l), avtCenteringField(n,l)
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
        : Field("avtGhostType",n,l), PythonGeneratorField("avtGhostType",n,l), avtGhostTypeField(n,l)
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
        : Field("int",n,l), PythonGeneratorField("int",n,l), avtSubsetTypeField(n,l)
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
        : Field("avtVarTypeField",n,l), PythonGeneratorField("avtVarType",n,l), avtVarTypeField(n,l)
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
        : Field("avtMeshTypeField",n,l), PythonGeneratorField("avtMeshType",n,l), avtMeshTypeField(n,l)
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
        : Field("avtExtentType",n,l), PythonGeneratorField("avtExentType",n,l), avtExtentTypeField(n,l)
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
        : Field("avtMeshCoordType",n,l), PythonGeneratorField("avtMeshCoordType",n,l), avtMeshCoordTypeField(n,l)
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
        : Field("LoadBalanceScheme",n,l), PythonGeneratorField("LoadBalanceScheme",n,l), LoadBalanceSchemeField(n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// --------------------------------- ScaleMode --------------------------------
//
class AttsGeneratorScaleMode : public virtual PythonGeneratorField, public virtual ScaleMode 
{
  public:
    AttsGeneratorScaleMode(const QString &n, const QString &l)
        : Field("scalemode",n,l), PythonGeneratorField("scalemode",n,l), ScaleMode(n,l) { }
    virtual void WriteSetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    int ival;" << Endl;
        c << "    if(!PyArg_ParseTuple(args, \"i\", &ival))" << Endl;
        c << "        return NULL;" << Endl;
        c << Endl;
        c << "    // Set the " << name << " in the object." << Endl;
        c << "    if(ival >= 0 && ival <= 1)" << Endl;
        c << "        obj->data->" << MethodNameSet() << "(ival);" << Endl;
        c << "    else" << Endl;
        c << "    {" << Endl;
        c << "        fprintf(stderr, \"An invalid  value was given. \"" << Endl;
        c << "                        \"Valid values are in the range of [0,1]. \"" << Endl;
        c << "                        \"You can also use the following names: \"" << Endl;
        c << "                        \"\\\"LINEAR\\\", \\\"LOG\\\"\\n\");" << Endl;
        c << "        return NULL;" << Endl;
        c << "    }" << Endl;
    }

    virtual void WriteGetMethodBody(QTextStream &c, const QString &className)
    {
        c << "    PyObject *retval = PyInt_FromLong(long(obj->data->"<<MethodNameGet()<<"()));" << Endl;
    }

    virtual void StringRepresentation(QTextStream &c, const QString &classname)
    {
        c << "    const char *" << name << "_values[] = {\"LINEAR\", \"LOG\"};" << Endl;
        c << "    SNPRINTF(tmpStr, 1000, \"%s" << name << " = %s  # LINEAR, LOG\\n\", prefix, " << name << "_values[atts->" << MethodNameGet() << "()]);" << Endl;
        c << "    str += tmpStr;" << Endl;
    }

    virtual void WriteGetAttr(QTextStream &c, const QString &classname)
    {
        if (internal)
            return;

        c << "    if(strcmp(name, \"" << name << "\") == 0)" << Endl;
        c << "        return " << classname << "_" << MethodNameGet() << "(self, NULL);" << Endl;
        c << "    if(strcmp(name, \"LINEAR\") == 0)" << Endl;
        c << "        return PyInt_FromLong(long(0));" << Endl;
        c << "    else if(strcmp(name, \"LOG\") == 0)" << Endl;
        c << "        return PyInt_FromLong(long(1));" << Endl;
        c << Endl;
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
        if      (type.isNull())          throw QString("Field %1 was specified with no type.").arg(name);
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
            throw QString("PythonFieldFactory: unknown type for field %1: %2").arg(name).arg(type);

        return f;
    }
};

// ----------------------------------------------------------------------------
//  Modifications:
//    Brad Whitlock, Thu Feb 28 16:29:20 PST 2008
//    Made it use a base class.
//
//    Tom Fogal, Fri Aug  8 10:22:02 EDT 2008
//    Add const in the doc string conditionally, based on python version.
//
//    Mark C. Miller, Mon Aug 17 17:02:51 PDT 2009
//    Omitted creation date from output .C file
//
//    Mark C. Miller, Wed Aug 26 10:55:31 PDT 2009
//    Added support for a custom base class to support derived state objects.
//    Added logic to count methods in method table without writing them.
// ----------------------------------------------------------------------------
#include <GeneratorBase.h>

class PythonGeneratorAttribute : public GeneratorBase
{
  public:
    vector<PythonGeneratorField*> fields;
    bool visitpy_api;
  public:
    PythonGeneratorAttribute(const QString &n, const QString &p, const QString &f,
                           const QString &e, const QString &ei, const QString &bc)
        : GeneratorBase(n,p,f,e,ei, GENERATOR_NAME, bc), fields()
    {
        visitpy_api = true;
    }

    virtual ~PythonGeneratorAttribute()
    {
        for (size_t i = 0; i < fields.size(); ++i)
            delete fields[i];
        fields.clear();
    }

    void DisableVISITPY() { visitpy_api = false; }

    void PrintFunction(QTextStream &out, const QString &f)
    {
        for (size_t i=0; i<functions.size(); i++)
            if (functions[i]->name == f)
                out << functions[i]->def;
    }

    void Print(QTextStream &out)
    {
        out << "    Attribute: " << name << " (" << purpose << ")" << Endl;
        size_t i;
        for (i=0; i<fields.size(); i++)
            fields[i]->Print(out);
        for (i=0; i<functions.size(); i++)
            functions[i]->Print(out, generatorName);
    }

    void WriteHeader(QTextStream &h)
    {
        h << copyright_str << Endl;
        h << "#ifndef PY_" << name.toUpper() << "_H" << Endl;
        h << "#define PY_" << name.toUpper() << "_H" << Endl;
        h << "#include <Python.h>" << Endl;
        h << "#include <"<<name<<".h>" << Endl;
        if (custombase)
            h << "#include <Py"<<baseClass<<".h>" << Endl;
        QString api(""); 
        if(visitpy_api)
        {
             h << "#include <visitpy_exports.h>" << Endl;
             api = "VISITPY_API ";
        }
        h << Endl;
        h << "//" << Endl;
        h << "// Functions exposed to the VisIt module." << Endl;
        h << "//" << Endl;
        QTextStream dummy;
        if (custombase)
            h << "#define " << name.toUpper()<<"_NMETH ("<<baseClass.toUpper()<<"_NMETH+"<<WritePyObjectMethodTable(dummy, true)<<")"<< Endl;
        else
            h << "#define " << name.toUpper()<<"_NMETH " << WritePyObjectMethodTable(dummy, true) << Endl;
        h << "void "<<api<<"          Py"<<name<<"_StartUp("<<name<<" *subj, void *data);" << Endl;
        h << "void "<<api<<"          Py"<<name<<"_CloseDown();" << Endl;
        h << api << "PyMethodDef * "<<" Py"<<name<<"_GetMethodTable(int *nMethods);" << Endl;
        h << "bool "<<api<<"          Py"<<name<<"_Check(PyObject *obj);" << Endl;
        h << api << name << " * "<<" Py"<<name<<"_FromPyObject(PyObject *obj);" << Endl;
        h << api << "PyObject * "<<"    Py"<<name<<"_New();" << Endl;
        h << api << "PyObject * "<<"    Py"<<name<<"_Wrap(const " << name << " *attr);" << Endl;
        h << "void "<<api<<"          Py"<<name<<"_SetParent(PyObject *obj, PyObject *parent);" << Endl;
        h << "void "<<api<<"          Py"<<name<<"_SetDefaults(const "<<name<<" *atts);" << Endl;
        h << "std::string "<<api<<"   Py"<<name<<"_GetLogString();" << Endl;
        h << "std::string "<<api<<"   Py"<<name<<"_ToString(const " << name << " *, const char *);" << Endl;
        h << api << "PyObject * "<<"    Py"<<name<<"_getattr(PyObject *self, char *name);" << Endl;
        h << "int "<<api<<"           Py"<<name<<"_setattr(PyObject *self, char *name, PyObject *args);" << Endl;
        h << api << "extern PyMethodDef Py"<<name<<"_methods["<<name.toUpper()<<"_NMETH];" << Endl;

        h << Endl;
        h << "#endif" << Endl;
        h << Endl;
    }

    void WriteIncludedHeaders(QTextStream &c)
    {
        c << "#include <snprintf.h>" << Endl;

        // Write the headers that are needed.
        for(size_t i = 0; i < fields.size(); ++i)
            fields[i]->WriteIncludedHeaders(c);
    }

    void WriteHeaderComment(QTextStream &c)
    {
        c << "// ****************************************************************************" << Endl;
        c << "// Module: Py" << name << Endl;
        c << "//" << Endl;
        c << "// Purpose: " << Endl;
        c << "//   " << purpose << Endl;
        c << "//" << Endl;
        c << "// Note:       Autogenerated by xml2python. Do not modify by hand!" << Endl;
        c << "//" << Endl;
        c << "// Programmer: xml2python" << Endl;
        c << "// Creation:   omitted" << Endl;       
        c << "//" << Endl;
        c << "// ****************************************************************************" << Endl;
        c << Endl;
    }

    void WritePyObjectStruct(QTextStream &c)
    {
        c << "//" << Endl;
        c << "// This struct contains the Python type information and a "<<name<<"." << Endl;
        c << "//" << Endl;
        c << "struct "<<name<<"Object" << Endl;
        c << "{" << Endl;
        c << "    PyObject_HEAD" << Endl;
        c << "    "<<name<<" *data;" << Endl;
        c << "    bool        owns;" << Endl;
        c << "    PyObject   *parent;" << Endl;
        c << "};" << Endl;
        c << Endl;
    }

    void WriteInternalPrototypes(QTextStream &c)
    {
        c << "//" << Endl;
        c << "// Internal prototypes" << Endl;
        c << "//" << Endl;
        c << "static PyObject *New"<<name<<"(int);" << Endl;
        c << Endl;
    }

    void WritePyObjectMethods(QTextStream &c)
    {
        // Write the Notify method.
        c << "static PyObject *" << Endl;
        c << name << "_Notify(PyObject *self, PyObject *args)" << Endl;
        c << "{" << Endl;
        c << "    " << name << "Object *obj = ("<<name<<"Object *)self;" << Endl;
        c << "    obj->data->Notify();" << Endl;
        c << "    Py_INCREF(Py_None);" << Endl;
        c << "    return Py_None;" << Endl;
        c << "}" << Endl;
        c << Endl;

        // Write the rest of the methods.
        for(size_t i = 0; i < fields.size(); ++i)
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
                    c << Endl;
                }
                else
                    fields[i]->WriteSetMethod(c, name);
            }

            QString gName(name + "_" + fields[i]->MethodNameGet());
            if(HasFunction(gName))
            {
                PrintFunction(c, gName);
                c << Endl;
            }
            else
                fields[i]->WriteGetMethod(c, name);

            fields[i]->WriteAdditionalMethods(c, name);
        }
        c << Endl;
    }

    void WriteUserDefinedFunctions(QTextStream &c)
    { 
        for(size_t i = 0; i < functions.size(); ++i)
        {
            if(functions[i]->target == generatorName &&
               functions[i]->user)
                c << functions[i]->def << Endl;
        }
    }

    int WritePyObjectMethodTable(QTextStream &c, bool countOnly = false)
    {
        int methCnt = 0;
        if (!countOnly)
        {
            c << Endl;
            c << "PyMethodDef Py"<<name<<"_methods["<<name.toUpper()<<"_NMETH] = {" << Endl;
            c << "    {\"Notify\", " << name << "_Notify, METH_VARARGS}," << Endl;
        }
        methCnt++;
        for(size_t i = 0; i < fields.size(); ++i)
        {
            methCnt += fields[i]->WritePyObjectMethodTable(c, name, countOnly);
        }
        for(size_t i = 0; i < functions.size(); ++i)
        {
            if(functions[i]->target == generatorName &&
               functions[i]->user)
            {
                QString pyMethName = functions[i]->decl;
                if (functions[i]->decl.startsWith(name + "_"))
                    pyMethName = functions[i]->decl.right(functions[i]->decl.length()-name.length()-1); 
                if (!countOnly)
                    c << "    {\"" << pyMethName << "\", " << functions[i]->decl << ", METH_VARARGS}," << Endl;
                methCnt++;
            }
        }
        if (!countOnly)
        {
            c << "    {NULL, NULL}" << Endl;
            c << "};" << Endl;
            c << Endl;
        }
        methCnt++;

        if (custombase && !countOnly)
        {
            c << "static void Py"<<name<<"_ExtendSetGetMethodTable()"<<Endl;
            c << "{" << Endl;
            c << "    static bool extended = false;" << Endl;
            c << "    if (extended) return;" << Endl;
            c << "    extended = true;" << Endl;
            c << Endl;
            c << "    int i = 0;" << Endl;
            c << "    while (Py"<<name<<"_methods[i].ml_name)" << Endl;
            c << "        i++;" << Endl;
            c << "    int n = i;" << Endl;
            c << "    while (Py"<<baseClass<<"_methods[i-n+1].ml_name)" << Endl;
            c << "    {" << Endl;
            c << "        Py"<<name<<"_methods[i] = Py"<<baseClass<<"_methods[i-n+1];" << Endl;
            c << "        i++;" << Endl;
            c << "    }" << Endl;
            c << Endl;
            c << "    PyMethodDef nullMethod = {NULL, NULL};" << Endl;
            c << "    Py"<<name<<"_methods[i] = nullMethod;" << Endl;
            c << "}" << Endl;
            c << Endl;
        }

        return methCnt;
    }

    void WriteGetAttrFunction(QTextStream &c)
    {
        QString mName("Py" + name + "_getattr");
        if(HasFunction(mName))
        {
            PrintFunction(c, mName);
            c << Endl;
            return;
        }

        c << "PyObject *" << Endl;
        c << mName << "(PyObject *self, char *name)" << Endl;
        c << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(c, mName, 0);
        for(size_t i = 0; i < fields.size(); ++i)
            fields[i]->WriteGetAttr(c, name);
        c << Endl;
        if(HasCode(mName, 1))
            PrintCode(c, mName, 1);
        if (custombase)
        {
            c << "    if(strcmp(name, \"__methods__\") != 0)" << Endl;
            c << "    {" << Endl;
            c << "        PyObject *retval = Py"<<baseClass<<"_getattr(self, name);" << Endl;
            c << "        if (retval) return retval;" << Endl;
            c << "    }" << Endl;
            c << Endl;
            c << "    Py"<<name<<"_ExtendSetGetMethodTable();" << Endl;
            c << Endl;
        }
        c << "    return Py_FindMethod(Py"<<name<<"_methods, self, name);" << Endl;
        c << "}" << Endl;
        c << Endl;
    }

    void WriteSetAttrFunction(QTextStream &c)
    {
        QString mName("Py" + name + "_setattr");
        if(HasFunction(mName))
        {
            PrintFunction(c, mName);
            c << Endl;
            return;
        }

        c << "int" << Endl;
        c << mName << "(PyObject *self, char *name, PyObject *args)" << Endl;
        c << "{" << Endl;
        if (custombase)
        {
            c << "    if (Py"<<baseClass<<"_setattr(self, name, args) != -1)" << Endl;
            c << "        return 0;" << Endl;
            c << Endl;
        }
        if(HasCode(mName, 0))
            PrintCode(c, mName, 0);
        c << "    // Create a tuple to contain the arguments since all of the Set" << Endl;
        c << "    // functions expect a tuple." << Endl;
        c << "    PyObject *tuple = PyTuple_New(1);" << Endl;
        c << "    PyTuple_SET_ITEM(tuple, 0, args);" << Endl;
        c << "    Py_INCREF(args);" << Endl;
        c << "    PyObject *obj = NULL;" << Endl;
        c << Endl;

        // Figure out the first field that can write a _setattr method.
        size_t i, index = 0;
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
        c << Endl;

        if(HasCode(mName, 1))
            PrintCode(c, mName, 1);

        c << "    if(obj != NULL)" << Endl;
        c << "        Py_DECREF(obj);" << Endl;
        c << Endl;
        c << "    Py_DECREF(tuple);" << Endl;
        c << "    return (obj != NULL) ? 0 : -1;" << Endl;
        c << "}" << Endl;
        c << Endl;
    }

    void WritePrintFunction(QTextStream &c)
    {
        QString mName(name + "_print");
        if(HasFunction(mName))
        {
            PrintFunction(c, mName);
            c << Endl;
            return;
        }

        c << "static int" << Endl;
        c << mName << "(PyObject *v, FILE *fp, int flags)" << Endl;
        c << "{" << Endl;
        c << "    "<<name<<"Object *obj = ("<<name<<"Object *)v;" << Endl;
        if(HasCode(mName, 0))
            PrintCode(c, mName, 0);
        c << "    fprintf(fp, \"%s\", Py" << name << "_ToString(obj->data, \"\").c_str());" << Endl;
        if(HasCode(mName, 1))
            PrintCode(c, mName, 1);
        c << "    return 0;" << Endl;
        c << "}" << Endl;
        c << Endl;
    }

    void WriteToStringFunction(QTextStream &c)
    {
        QString mName(QString("Py") + name + "_ToString");
        if(HasFunction(mName))
        {
            PrintFunction(c, mName);
            c << Endl;
            return;
        }

        c << "std::string" << Endl;
        c << mName << "(const "<<name<<" *atts, const char *prefix)" << Endl;
        c << "{" << Endl;
        c << "    std::string str; " << Endl;
        c << "    char tmpStr[1000]; " << Endl;
        c << Endl;
        if (custombase)
        {
            c << "    str = Py"<<baseClass<<"_ToString(atts, prefix);" << Endl;
            c << Endl;
        }
        if(HasCode(mName, 0))
            PrintCode(c, mName, 0);
        for(size_t i = 0; i < fields.size(); ++i)
        {
            if(!fields[i]->internal)
                fields[i]->StringRepresentation(c, name);
        }
        if(HasCode(mName, 1))
            PrintCode(c, mName, 1);
        c << "    return str;" << Endl;
        c << "}" << endl << Endl;
    }

    void WriteStringRepresentationFunction(QTextStream &c)
    {
        QString mName(name + "_str");
        if(HasFunction(mName))
        {
            PrintFunction(c, mName);
            c << Endl;
            return;
        }

        c << "PyObject *" << Endl;
        c << mName << "(PyObject *v)" << Endl;
        c << "{" << Endl;
        c << "    "<<name<<"Object *obj = ("<<name<<"Object *)v;" << Endl;
        c << "    return PyString_FromString(Py" << name << "_ToString(obj->data,\"\").c_str());" << Endl;
        c << "}" << endl << Endl;
    }


    void WriteTypeFunctions(QTextStream &c)
    {
        c << "//" << Endl;
        c << "// Type functions" << Endl;
        c << "//" << Endl;
        c << Endl;

        c << "static void" << Endl;
        c << name << "_dealloc(PyObject *v)" << Endl;
        c << "{" << Endl;
        c << "   " << name << "Object *obj = (" << name << "Object *)v;" << Endl;
        c << "   if(obj->parent != 0)" << Endl;
        c << "       Py_DECREF(obj->parent);" << Endl;
        c << "   if(obj->owns)" << Endl;
        c << "       delete obj->data;" << Endl;
        c << "}" << Endl;
        c << Endl;

        c << "static int" << Endl;
        c << name << "_compare(PyObject *v, PyObject *w)" << Endl;
        c << "{" << Endl;
        c << "    "<<name<<" *a = (("<<name<<"Object *)v)->data;" << Endl;
        c << "    "<<name<<" *b = (("<<name<<"Object *)w)->data;" << Endl;
        c << "    return (*a == *b) ? 0 : -1;" << Endl;
        c << "}" << Endl;
        c << Endl;

        // Write the getattr function
        WriteGetAttrFunction(c);

        // Write the setattr function
        WriteSetAttrFunction(c);

        // Write the print function
        WritePrintFunction(c);

        // Write the str function.
        WriteStringRepresentationFunction(c);

        c << "//" << Endl;
        c << "// The doc string for the class." << Endl;
        c << "//" << Endl;
        c << "#if PY_MAJOR_VERSION > 2 || "
          << "(PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 5)" << Endl;
        c << "static const char *" << name << "_Purpose = \"" << purpose << "\";" << Endl;
        c << "#else" << Endl;
        c << "static char *" << name << "_Purpose = \"" << purpose << "\";" << Endl;
        c << "#endif" << Endl;
        c << Endl;

        c << "//" << Endl;
        c << "// The type description structure" << Endl;
        c << "//" << Endl;
        c << "static PyTypeObject "<<name<<"Type =" << Endl;
        c << "{" << Endl;
        c << "    //" << Endl;
        c << "    // Type header" << Endl;
        c << "    //" << Endl;
        c << "    PyObject_HEAD_INIT(&PyType_Type)" << Endl;
        c << "    0,                                   // ob_size" << Endl;
        c << "    \""<<name<<"\",                    // tp_name" << Endl;
        c << "    sizeof("<<name<<"Object),        // tp_basicsize" << Endl;
        c << "    0,                                   // tp_itemsize" << Endl;
        c << "    //" << Endl;
        c << "    // Standard methods" << Endl;
        c << "    //" << Endl;
        c << "    (destructor)"<<name<<"_dealloc,  // tp_dealloc" << Endl;
        c << "    (printfunc)"<<name<<"_print,     // tp_print" << Endl;
        c << "    (getattrfunc)Py"<<name<<"_getattr, // tp_getattr" << Endl;
        c << "    (setattrfunc)Py"<<name<<"_setattr, // tp_setattr" << Endl;
        c << "    (cmpfunc)"<<name<<"_compare,     // tp_compare" << Endl;
        c << "    (reprfunc)0,                         // tp_repr" << Endl;
        c << "    //" << Endl;
        c << "    // Type categories" << Endl;
        c << "    //" << Endl;
        c << "    0,                                   // tp_as_number" << Endl;
        c << "    0,                                   // tp_as_sequence" << Endl;
        c << "    0,                                   // tp_as_mapping" << Endl;
        c << "    //" << Endl;
        c << "    // More methods" << Endl;
        c << "    //" << Endl;
        c << "    0,                                   // tp_hash" << Endl;
        c << "    0,                                   // tp_call" << Endl;
        c << "    (reprfunc)" << name << "_str,        // tp_str" << Endl;
        c << "    0,                                   // tp_getattro" << Endl;
        c << "    0,                                   // tp_setattro" << Endl;
        c << "    0,                                   // tp_as_buffer" << Endl;
        c << "    Py_TPFLAGS_CHECKTYPES,               // tp_flags" << Endl;
        c << "    "<<name<<"_Purpose,              // tp_doc" << Endl;
        c << "    0,                                   // tp_traverse" << Endl;
        c << "    0,                                   // tp_clear" << Endl;
        c << "    0,                                   // tp_richcompare" << Endl;
        c << "    0                                    // tp_weaklistoffset" << Endl;
        c << "};" << Endl;
        c << Endl;
        c << "//" << Endl;
        c << "// Helper functions for object allocation." << Endl;
        c << "//" << Endl;
        c << Endl;
        c << "static "<<name<<" *defaultAtts = 0;" << Endl;
        c << "static "<<name<<" *currentAtts = 0;" << Endl;
        c << Endl;
        c << "static PyObject *" << Endl;
        c << "New"<<name<<"(int useCurrent)" << Endl;
        c << "{" << Endl;
        c << "    "<<name<<"Object *newObject;" << Endl;
        c << "    newObject = PyObject_NEW("<<name<<"Object, &"<<name<<"Type);" << Endl;
        c << "    if(newObject == NULL)" << Endl;
        c << "        return NULL;" << Endl;
        c << "    if(useCurrent && currentAtts != 0)" << Endl;
        c << "        newObject->data = new "<<name<<"(*currentAtts);" << Endl;
        c << "    else if(defaultAtts != 0)" << Endl;
        c << "        newObject->data = new "<<name<<"(*defaultAtts);" << Endl;
        c << "    else" << Endl;
        c << "        newObject->data = new "<<name<<";" << Endl;
        c << "    newObject->owns = true;" << Endl;
        c << "    newObject->parent = 0;" << Endl;
        c << "    return (PyObject *)newObject;" << Endl;
        c << "}" << Endl;
        c << Endl;
        c << "static PyObject *" << Endl;
        c << "Wrap"<<name<<"(const " << name << " *attr)" << Endl;
        c << "{" << Endl;
        c << "    "<<name<<"Object *newObject;" << Endl;
        c << "    newObject = PyObject_NEW("<<name<<"Object, &"<<name<<"Type);" << Endl;
        c << "    if(newObject == NULL)" << Endl;
        c << "        return NULL;" << Endl;
        c << "    newObject->data = ("<<name<< " *)attr;" << Endl;
        c << "    newObject->owns = false;" << Endl;
        c << "    newObject->parent = 0;" << Endl;
        c << "    return (PyObject *)newObject;" << Endl;
        c << "}" << Endl;
        c << Endl;
    }

    void WriteCallLogRoutineMethod(QTextStream &c)
    {
        QString shortName(name);
        shortName.replace(QRegExp("Attributes"), "Atts");

        QString GetLogString(QString("Py") + name + "_GetLogString");
        if(HasFunction(GetLogString))
            PrintFunction(c, GetLogString);
        else
        { 
            c << "std::string" << Endl;
            c << "Py" << name << "_GetLogString()" << Endl;
            c << "{" << Endl;
            c << "    std::string s(\"" << shortName << " = " << name << "()\\n\");" << Endl;
            c << "    if(currentAtts != 0)" << Endl;
            c << "        s += Py" << name << "_ToString(currentAtts, \"" << shortName << ".\");" << Endl;
            c << "    return s;" << Endl;
            c << "}" << Endl;
        }
        c << Endl;

        QString CallLogRoutine(QString("Py") + name + "_CallLogRoutine");
        if(HasFunction(CallLogRoutine))
            PrintFunction(c, CallLogRoutine);
        else
        { 
            c << "static void" << Endl;
            c << CallLogRoutine << "(Subject *subj, void *data)" << Endl;
            c << "{" << Endl;
            c << "    "<<name<<" *atts = ("<<name<<" *)subj;" << Endl;
            if(HasCode(CallLogRoutine, 0))
                PrintCode(c, CallLogRoutine, 0);
            c << "    typedef void (*logCallback)(const std::string &);" << Endl;
            c << "    logCallback cb = (logCallback)data;" << Endl;
            c << Endl;
            c << "    if(cb != 0)" << Endl;
            c << "    {" << Endl;
            c << "        std::string s(\"" << shortName << " = " << name << "()\\n\");" << Endl;
            c << "        s += Py" << name << "_ToString(currentAtts, \"" << shortName << ".\");" << Endl;
            c << "        cb(s);" << Endl;
            c << "    }" << Endl;
            if(HasCode(CallLogRoutine, 1))
                PrintCode(c, CallLogRoutine, 1);
            c << "}" << Endl;
        }
    }

    void WriteExposedInterface(QTextStream &c)
    {
        c << "///////////////////////////////////////////////////////////////////////////////" << Endl;
        c << "//" << Endl;
        c << "// Interface that is exposed to the VisIt module." << Endl;
        c << "//" << Endl;
        c << "///////////////////////////////////////////////////////////////////////////////" << Endl;
        c << Endl;

        QString Py_new(name + "_new");
        if(HasFunction(Py_new))
            PrintFunction(c, Py_new);
        else
        { 
            c << "PyObject *" << Endl;
            c << Py_new << "(PyObject *self, PyObject *args)" << Endl;
            c << "{" << Endl;
            if(HasCode(Py_new, 0))
                PrintCode(c, Py_new, 0);
            c << "    int useCurrent = 0;" << Endl;
            c << "    if (!PyArg_ParseTuple(args, \"i\", &useCurrent))" << Endl;
            c << "    {" << Endl;
            c << "        if (!PyArg_ParseTuple(args, \"\"))" << Endl;
            c << "            return NULL;" << Endl;
            c << "        else" << Endl;
            c << "            PyErr_Clear();" << Endl;
            c << "    }" << Endl;
            c << Endl;
            if(HasCode(Py_new, 1))
                PrintCode(c, Py_new, 1);
            c << "    return (PyObject *)New"<<name<<"(useCurrent);" << Endl;
            c << "}" << Endl;
        }
        c << Endl;

        c << "//" << Endl;
        c << "// Plugin method table. These methods are added to the visitmodule's methods." << Endl;
        c << "//" << Endl;
        c << "static PyMethodDef "<<name<<"Methods[] = {" << Endl;
        c << "    {\""<<name<<"\", "<<name<<"_new, METH_VARARGS}," << Endl;
        c << "    {NULL,      NULL}        /* Sentinel */" << Endl;
        c << "};" << Endl;
        c << Endl;
        c << "static Observer *"<<name<<"Observer = 0;" << Endl;
        c << Endl;
        WriteCallLogRoutineMethod(c);
        c << Endl;

        QString StartUp(QString("Py") + name + "_StartUp");
        if(HasFunction(StartUp))
            PrintFunction(c, StartUp);
        else
        { 
            c << "void" << Endl;
            c << StartUp << "("<<name<<" *subj, void *data)" << Endl;
            c << "{" << Endl;
            c << "    if(subj == 0)" << Endl;
            c << "        return;" << Endl;
            c << Endl;
            if(HasCode(StartUp, 0))
                PrintCode(c, StartUp, 0);
            c << "    currentAtts = subj;" << Endl;
            c << "    Py" << name << "_SetDefaults(subj);" << Endl;
            c << Endl;
            c << "    //" << Endl;
            c << "    // Create the observer that will be notified when the attributes change." << Endl;
            c << "    //" << Endl;
            c << "    if("<<name<<"Observer == 0)" << Endl;
            c << "    {" << Endl;
            c << "        "<<name<<"Observer = new ObserverToCallback(subj," << Endl;
            c << "            Py"<<name<<"_CallLogRoutine, (void *)data);" << Endl;
            c << "    }" << Endl;
            c << Endl;
            if(HasCode(StartUp, 1))
                PrintCode(c, StartUp, 1);
            c << "}" << Endl;
        }
        c << Endl;

        QString CloseDown(QString("Py") + name + "_CloseDown");
        if(HasFunction(CloseDown))
            PrintFunction(c, CloseDown);
        else
        { 
            c << "void" << Endl;
            c << CloseDown << "()" << Endl;
            c << "{" << Endl;
            if(HasCode(CloseDown, 0))
                PrintCode(c, CloseDown, 0);
            c << "    delete defaultAtts;" << Endl;
            c << "    defaultAtts = 0;" << Endl;
            c << "    delete "<<name<<"Observer;" << Endl;
            c << "    "<<name<<"Observer = 0;" << Endl;
            if(HasCode(CloseDown, 1))
                PrintCode(c, CloseDown, 1);
            c << "}" << Endl;
        }
        c << Endl;

        c << "PyMethodDef *" << Endl;
        c << "Py"<<name<<"_GetMethodTable(int *nMethods)" << Endl;
        c << "{" << Endl;
        c << "    *nMethods = 1;" << Endl;
        c << "    return "<<name<<"Methods;" << Endl;
        c << "}" << Endl;
        c << Endl;
        c << "bool" << Endl;
        c << "Py"<<name<<"_Check(PyObject *obj)" << Endl;
        c << "{" << Endl;
        c << "    return (obj->ob_type == &"<<name<<"Type);" << Endl;
        c << "}" << Endl;
        c << Endl;

        QString FromPyObject(QString("Py") + name + "_FromPyObject");
        if(HasFunction(FromPyObject))
            PrintFunction(c, FromPyObject);
        else
        { 
            c << name << " *" << Endl;
            c << FromPyObject << "(PyObject *obj)" << Endl;
            c << "{" << Endl;
            c << "    "<<name<<"Object *obj2 = ("<<name<<"Object *)obj;" << Endl;
            c << "    return obj2->data;" << Endl;
            c << "}" << Endl;
        }
        c << Endl;

        QString PyNew(QString("Py") + name + "_New");
        if(HasFunction(PyNew))
            PrintFunction(c, PyNew);
        else
        { 
            c << "PyObject *" << Endl;
            c << PyNew << "()" << Endl;
            c << "{" << Endl;
            c << "    return New"<<name<<"(0);" << Endl;
            c << "}" << Endl;
        }
        c << Endl;

        QString Wrap(QString("Py") + name + "_Wrap");
        if(HasFunction(Wrap))
            PrintFunction(c, Wrap);
        else
        { 
            c << "PyObject *" << Endl;
            c << Wrap << "(const " << name << " *attr)" << Endl;
            c << "{" << Endl;
            c << "    return Wrap"<<name<<"(attr);" << Endl;
            c << "}" << Endl;
        }
        c << Endl;

        QString SetParent(QString("Py") + name + "_SetParent");
        if(HasFunction(SetParent))
            PrintFunction(c, SetParent);
        else
        { 
            c << "void" << Endl;
            c << SetParent << "(PyObject *obj, PyObject *parent)" << Endl;
            c << "{" << Endl;
            c << "    "<<name<<"Object *obj2 = ("<<name<<"Object *)obj;" << Endl;
            if(HasCode(SetParent, 0))
                PrintCode(c, SetParent, 0);
            c << "    obj2->parent = parent;" << Endl;
            if(HasCode(SetParent, 1))
                PrintCode(c, SetParent, 1);
            c << "}" << Endl;
        }
        c << Endl;
        
        QString SetDefaults(QString("Py") + name + "_SetDefaults");
        if(HasFunction(SetDefaults))
            PrintFunction(c, SetDefaults);
        else
        {    
            c << "void" << Endl;
            c << SetDefaults << "(const " << name << " *atts)" << Endl;
            c << "{" << Endl;
            if(HasCode(SetDefaults, 0))
                PrintCode(c, SetDefaults, 0);
            c << "    if(defaultAtts)" << Endl;
            c << "        delete defaultAtts;" << Endl;
            c << Endl;
            c << "    defaultAtts = new " << name << "(*atts);" << Endl;
            if(HasCode(SetDefaults, 1))
                PrintCode(c, SetDefaults, 1);
            c << "}" << Endl;
        }
        c << Endl;
    }

    void WriteSource(QTextStream &c)
    {
        c << copyright_str << Endl;
        c << "#include <Py" << name << ".h>" << Endl;
        c << "#include <ObserverToCallback.h>" << Endl;
        c << "#include <stdio.h>" << Endl;
        WriteIncludedHeaders(c);
        c << Endl;
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

    void Print(QTextStream &out)
    {
        out << "Plugin: "<<name<<" (\""<<label<<"\", type="<<type<<") -- version "<<version<< Endl;
        if (atts)
        {
            atts->DisableVISITPY();
            atts->Print(out);
        }
    }

    void WriteHeader(QTextStream &h)
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
