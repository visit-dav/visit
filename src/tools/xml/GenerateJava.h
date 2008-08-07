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

#ifndef GENERATE_JAVA_H
#define GENERATE_JAVA_H

#include <vector>
#include <map>
#include <algorithm>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include "Field.h"

using std::vector;

#define GENERATOR_NAME "xml2java"

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
//  Creation:    Thu Aug 8 17:55:57 PST 2002
//
//  Modifications:
//    Jeremy Meredith, Tue Aug 27 14:32:50 PDT 2002
//    Added mfiles, dbtype, and libs.  Allowed NULL atts.
//
//    Jeremy Meredith, Thu Oct 17 15:58:29 PDT 2002
//    Added some enhancements for the XML editor.
//
//    Jeremy Meredith, Tue Sep 23 17:08:53 PDT 2003
//    Made haswriter be a bool.
//
//    Jeremy Meredith, Wed Nov  5 13:28:03 PST 2003
//    Added ability to disable plugins by default.
//
//    Brad Whitlock, Tue Jun 29 12:01:39 PDT 2004
//    Improved how constants are written out.
//
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Jeremy Meredith, Mon Sep  6 16:29:41 PDT 2004
//    Check if a value's initializer is set before trying to
//    write the initialization code.
//
//    Brad Whitlock, Wed Dec 8 15:52:36 PST 2004
//    Added support for variable names.
//
//    Hank Childs, Tue May 24 10:19:40 PDT 2005
//    Add hasoptions.
//
//    Hank Childs, Fri Jun  9 09:46:13 PDT 2006
//    Added copyright string.
//
//    Cyrus Harrison, Wed Mar  7 09:49:20 PST 2007
//    Allow for engine-specific code in a plugin's source files.
//
//    Brad Whitlock, Wed Mar 14 17:50:07 PST 2007
//    Change the names of the attVector access functions.
//
//    Brad Whitlock, Mon Feb 25 14:04:48 PST 2008
//    Added methods to create toString methods in Java.
//
//    Brad Whitlock, Thu Feb 28 16:06:19 PST 2008
//    Made use of base classes for easier maintenance.
//
//    Jeremy Meredith, Thu Aug  7 14:34:01 EDT 2008
//    Reorder constructor initializers to be the correct order.
//
// ****************************************************************************

class JavaGeneratorField : public virtual Field
{
  protected:
    QString generatorName;
  public:
    bool generatePlugin;

    JavaGeneratorField(const QString &t, const QString &n, const QString &l)
        : Field(t,n,l), generatorName(GENERATOR_NAME)
    {
        generatePlugin = false;
    }

    QString GetCPPNameW(int w, bool subtypename=false, const QString &classname="")
    {
        QString s = GetCPPName(subtypename,classname);
        for (size_t i = w - s.length(); i > 0; --i)
            s += " ";
        return s;
    }

    virtual void WriteSourceWriteAtts(ostream &h, const QString &indent) = 0;
    virtual bool WriteSourceReadAtts(ostream &h, const QString &indent) = 0;
    virtual void WriteSourceSetDefault(ostream &c) = 0;

    virtual void AddImports(UniqueStringList &sl) { }

    virtual void WriteSourceAttribute(ostream &h, int w)
    {
        h << "    private " << GetCPPNameW(w) << " " << name << ";" << endl;
    }

    // ------------------------------------------------------------------------

    virtual void WriteAuxiliarySetFunction(ostream &c)
    {
    }

    virtual void WriteSourceSetFunction(ostream &c, const QString &classname)
    {
        // Write prototype.
        c << "    public void Set" << Name << "(";
        c << GetCPPName(true,classname) << " ";
        c << name << "_)" << endl;
        c << "    {" << endl;
        if (!isArray)
        {
            c << "        " << name << " = " << name << "_;" << endl;
            c << "        Select(" << index << ");" << endl;
        }
        else
        {
            if (length < 5)
            {
                for(int i = 0; i < length; ++i)
                    c << "        " << name << "[" << i << "] = " << name << "_[" << i << "];"<< endl;
            }
            else
            {
                c << "        for(int i = 0; i < " << length << "; ++i)" << endl;
                c << "             " << name << "[i] = " << name << "_[i];"<< endl;
            }
            c << "        Select(" << index << ");" << endl;
        }
        c << "    }" << endl;
        c << endl;

        // If it is an array with a size less than 6, write another set method.
        WriteAuxiliarySetFunction(c);
    }

    virtual void WriteSourceGetFunction(ostream &c, int w)
    {
        c << "    public " << GetCPPNameW(w) << " Get" << Name << "() { return "
          << name << "; }" << endl;
    }

    virtual void WriteSourceAGVectorFunctions(ostream &c)
    {
    }


    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "        " << name << " = obj." << name << ";" << endl;
    }

    virtual void WriteSourceComparisonPrecalc(ostream &c)
    {
        if (isArray)
        {
            c << "        // Compare the " << name << " arrays." << endl;
            c << "        boolean " << name << "_equal = true;" << endl; 
            c << "        for(i = 0; i < " << length << " && "
              << name << "_equal; ++i)" << endl;
            c << "            " << name << "_equal = ("
              << name << "[i] == obj." << name
              << "[i]);" << endl << endl;
        }
        else if (isVector)
        {
            c << "        // Compare the elements in the " << name << " vector." << endl;
            c << "        boolean " << name << "_equal = (obj." << name << ".size() == " << name << ".size());" << endl; 
            c << "        for(i = 0; (i < " << name << ".size()) && "
              << name << "_equal; ++i)" << endl;
            c << "        {" << endl;
            c << "            // Make references to " << GetVectorStorageName() << " from Object." << endl;
            c << "            " << GetVectorStorageName() << " " << name << "1 = (" << GetVectorStorageName() << ")" << name << ".elementAt(i);" << endl;
            c << "            " << GetVectorStorageName() << " " << name << "2 = (" << GetVectorStorageName() << ")obj." << name << ".elementAt(i);" << endl;
            c << "            " << name << "_equal = " << name << "1.equals(" << name << "2);" << endl;
            c << "        }" << endl;
        }
    }

    virtual QString GetVectorStorageName() const
    {
        return "";
    }

    virtual void WriteSourceComparison(ostream &c)
    {
        if (isArray || isVector)
            c << name << "_equal";
        else
        {
            c << "(" << name << " == obj." << name << ")";
        }
    }

    virtual void WriteToString(ostream &c, const QString &indent)
    {
    }
};

//
// ------------------------------------ Int -----------------------------------
//
class JavaGeneratorInt : public virtual Int , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorInt(const QString &n, const QString &l)
        : Field("int",n,l), Int(n,l), JavaGeneratorField("int",n,l) { }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        if(valueSet)
            c << "    " << name << " = " << val << ";" << endl;
        else
            c << "    " << name << " = 0;" << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteInt(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadInt());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + intToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
};


//
// -------------------------------- IntArray --------------------------------
//
class JavaGeneratorIntArray : public virtual IntArray , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorIntArray(const QString &s, const QString &n, const QString &l)
        : Field("intArray",n,l), IntArray(s,n,l), JavaGeneratorField("intArray",n,l) { }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "int[]";
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = new int[" << length << "];" << endl;
        if(valueSet)
        {
            for (int i = 0; i < length; ++i)
                c << "        " << name << "["<<i<<"] = " << val[i] << ";" << endl;
        }
        else
        {
            c << "    " << "for (int i = 0; i < " << length << "; ++i)" << endl;
            c << "        " << name << "[i] = 0;" << endl;
        }
    }

    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "        " << name << " = new int[" << length << "];" << endl;

        if (length < 4)
        { 
           for(int i = 0; i < length; ++i)
           {
               c << "        " << name << "[" << i << "] = "
                 << "obj." << name << "[" << i << "];" << endl;
           }
        }
        else
        {
            c << "        for(i = 0; i < obj." << name << ".length; ++i)" << endl;
            c << "            " << name << "[i] = obj." << name << "[i];" << endl;
        }
        c << endl;
    }

    virtual void WriteAuxiliarySetFunction(ostream &c)
    {
        if(length < 6)
        {
            c << "    public void Set" << Name << "(";
            for(int i = 0; i < length; ++i)
            {
                c << "int e" << i;
                if(i < length-1)
                    c << ", ";
            }
            c << ")" << endl;
            c << "    {" << endl;
            for(int j = 0; j < length; ++j)
                c << "        " << name << "[" << j << "] = e" << j << ";"<< endl;
            c << "        Select(" << index << ");" << endl;
            c << "    }" << endl;
            c << endl;
        }
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteIntArray(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadIntArray());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + intArrayToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
};


//
// -------------------------------- IntVector --------------------------------
//
class JavaGeneratorIntVector : public virtual IntVector , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorIntVector(const QString &n, const QString &l)
        : Field("intVector",n,l), IntVector(n,l), JavaGeneratorField("intVector",n,l) { }

    virtual void AddImports(UniqueStringList &sl) 
    { 
        sl.AddString("import java.lang.Integer;\n");
        sl.AddString("import java.util.Vector;\n");
    }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "Vector";
    }

    virtual void WriteSourceAttribute(ostream &h, int w)
    {
        h << "    private " << GetCPPNameW(w) << " " << name << "; // vector of Integer objects" << endl;
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = new Vector();" << endl;
        if(valueSet)
        {
            for (size_t i = 0; i < val.size(); ++i)
                c << "        " << name << ".addElement(new Integer(" << val[i] << "));" << endl;
        }
    }

    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "        " << name << " = new Vector();" << endl;
        c << "        for(i = 0; i < obj." << name << ".size(); ++i)" << endl;
        c << "        {" << endl;
        c << "            Integer iv = (Integer)obj." << name << ".elementAt(i);" << endl;
        c << "            " << name << ".addElement(new Integer(iv.intValue()));" << endl;
        c << "        }" << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteIntVector(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadIntVector());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + intVectorToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
    virtual QString GetVectorStorageName() const
    {
        return "Integer";
    }
};


// 
// ----------------------------------- Bool -----------------------------------
//
class JavaGeneratorBool : public virtual Bool , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorBool(const QString &n, const QString &l)
        : Field("boolean",n,l), Bool(n,l), JavaGeneratorField("boolean",n,l) { }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "boolean";
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << (val ? "true" : "false") << ";" << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteBool(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadBool());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + boolToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
};


//
// ----------------------------------- Float ----------------------------------
//
class JavaGeneratorFloat : public virtual Float , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorFloat(const QString &n, const QString &l)
        : Field("float",n,l), Float(n,l), JavaGeneratorField("float",n,l) { }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << val << "f;" << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteFloat(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadFloat());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + floatToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
};


//
// -------------------------------- FloatArray -------------------------------
//
class JavaGeneratorFloatArray : public virtual FloatArray , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorFloatArray(const QString &s, const QString &n, const QString &l)
        : Field("floatArray",n,l), FloatArray(s,n,l), JavaGeneratorField("floatArray",n,l) { }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "float[]";
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = new float[" << length << "];" << endl;
        if(valueSet)
        {
            for (int i = 0; i < length; ++i)
                c << "        " << name << "["<<i<<"] = " << val[i] << "f;" << endl;
        }
        else
        {
            c << "        for (int i = 0; i < "<<name<<".length; ++i)" << endl;
            c << "            " << name << "[i] = 0f;" << endl;
        }
    }

    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "        " << name << " = new float[" << length << "];" << endl;

        if (length < 4)
        { 
           for(int i = 0; i < length; ++i)
           {
               c << "        " << name << "[" << i << "] = "
                 << "obj." << name << "[" << i << "];" << endl;
           }
        }
        else
        {
            c << "        for(i = 0; i < obj." << name << ".length; ++i)" << endl;
            c << "            " << name << "[i] = obj." << name << "[i];" << endl;
        }
        c << endl;
    }

    virtual void WriteAuxiliarySetFunction(ostream &c)
    {
        if(length < 6)
        {
            c << "    public void Set" << Name << "(";
            for(int i = 0; i < length; ++i)
            {
                c << "float e" << i;
                if(i < length-1)
                    c << ", ";
            }
            c << ")" << endl;
            c << "    {" << endl;
            for(int j = 0; j < length; ++j)
                c << "        " << name << "[" << j << "] = e" << j << ";"<< endl;
            c << "        Select(" << index << ");" << endl;
            c << "    }" << endl;
            c << endl;
        }
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteFloatArray(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadFloatArray());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + floatArrayToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
};


//
// ---------------------------------- Double ----------------------------------
//
class JavaGeneratorDouble : public virtual Double , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorDouble(const QString &n, const QString &l)
        : Field("double",n,l), Double(n,l), JavaGeneratorField("double",n,l) { }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << val << ";" << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteDouble(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadDouble());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + doubleToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
};


//
// -------------------------------- DoubleArray -------------------------------
//
class JavaGeneratorDoubleArray : public virtual DoubleArray , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorDoubleArray(const QString &s, const QString &n, const QString &l)
        : Field("doubleArray",n,l), DoubleArray(s,n,l), JavaGeneratorField("doubleArray",n,l) { }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "double[]";
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = new double[" << length << "];" << endl;
        if(valueSet)
        {
            for (int i = 0; i < length; ++i)
                c << "        " << name << "["<<i<<"] = " << val[i] << ";" << endl;
        }
        else
        {
            c << "        for (int i = 0; i < "<<name<<".length; ++i)" << endl;
            c << "            " << name << "[i] = 0.;" << endl;
        }

    }

    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "        " << name << " = new double[" << length << "];" << endl;

        if (length < 4)
        { 
           for(int i = 0; i < length; ++i)
           {
               c << "        " << name << "[" << i << "] = "
                 << "obj." << name << "[" << i << "];" << endl;
           }
        }
        else
        {
            c << "        for(i = 0; i < obj." << name << ".length; ++i)" << endl;
            c << "            " << name << "[i] = obj." << name << "[i];" << endl;
        }
        c << endl;
    }

    virtual void WriteAuxiliarySetFunction(ostream &c)
    {
        if(length < 6)
        {
            c << "    public void Set" << Name << "(";
            for(int i = 0; i < length; ++i)
            {
                c << "double e" << i;
                if(i < length-1)
                    c << ", ";
            }
            c << ")" << endl;
            c << "    {" << endl;
            for(int j = 0; j < length; ++j)
                c << "        " << name << "[" << j << "] = e" << j << ";"<< endl;
            c << "        Select(" << index << ");" << endl;
            c << "    }" << endl;
            c << endl;
        }
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteDoubleArray(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadDoubleArray());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + doubleArrayToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
};

//
// ------------------------------- DoubleVector -------------------------------
//
class JavaGeneratorDoubleVector : public virtual DoubleVector , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorDoubleVector(const QString &n, const QString &l)
        : Field("doubleVector",n,l), DoubleVector(n,l), JavaGeneratorField("doubleVector",n,l) { }

    virtual void AddImports(UniqueStringList &sl) 
    { 
        sl.AddString("import java.lang.Double;\n");
        sl.AddString("import java.util.Vector;\n");
    }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "Vector";
    }

    virtual void WriteSourceAttribute(ostream &h, int w)
    {
        h << "    private " << GetCPPNameW(w) << " " << name << "; // vector of Double objects" << endl;
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = new Vector();" << endl;
        for (size_t i = 0; i < val.size(); ++i)
            c << "        " << name << ".addElement(new Double(" << val[i] << "));" << endl;
    }

    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "        " << name << " = new Vector(obj." << name << ".size());" << endl;
        c << "        for(i = 0; i < obj." << name << ".size(); ++i)" << endl;
        c << "        {" << endl;
        c << "            Double dv = (Double)obj." << name << ".elementAt(i);" << endl;
        c << "            " << name << ".addElement(new Double(dv.doubleValue()));" << endl;
        c << "        }" << endl;
        c << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteDoubleVector(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadDoubleVector());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + doubleVectorToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
    virtual QString GetVectorStorageName() const
    {
        return "Double";
    }
};


//
// ----------------------------------- UChar ----------------------------------
//
class JavaGeneratorUChar : public virtual UChar , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorUChar(const QString &n, const QString &l)
        : Field("uchar",n,l), UChar(n,l), JavaGeneratorField("uchar",n,l) { }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "byte";
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << int(val) << ";" << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteByte(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadByte());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + ucharToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
};


//
// -------------------------------- UCharArray --------------------------------
//
class JavaGeneratorUCharArray : public virtual UCharArray , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorUCharArray(const QString &s, const QString &n, const QString &l)
        : Field("ucharArray",n,l), UCharArray(s,n,l), JavaGeneratorField("byte",n,l) { }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "byte[]";
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = new byte[" << length << "];" << endl;
        if(valueSet)
        {
            for (int i = 0; i < length; ++i)
                c << "        " << name << "["<<i<<"] = (byte)" << int(val[i]) << ";" << endl;
        }
        else
        {
            c << "        for (int i = 0; i < "<<name<<".length; ++i)" << endl;
            c << "            " << name << "[i] = 0;" << endl;
        }
    }

    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "        " << name << " = new byte[" << length << "];" << endl;

        if (length < 4)
        { 
           for(int i = 0; i < length; ++i)
           {
               c << "        " << name << "[" << i << "] = "
                 << "obj." << name << "[" << i << "];" << endl;
           }
        }
        else
        {
            c << "        for(i = 0; i < obj." << name << ".length; ++i)" << endl;
            c << "            " << name << "[i] = obj." << name << "[i];" << endl;
        }
        c << endl;
    }

    virtual void WriteAuxiliarySetFunction(ostream &c)
    {
        if(length < 6)
        {
            c << "    public void Set" << Name << "(";
            for(int i = 0; i < length; ++i)
            {
                c << "byte e" << i;
                if(i < length-1)
                    c << ", ";
            }
            c << ")" << endl;
            c << "    {" << endl;
            for(int j = 0; j < length; ++j)
                c << "        " << name << "[" << j << "] = e" << j << ";"<< endl;
            c << "        Select(" << index << ");" << endl;
            c << "    }" << endl;
            c << endl;
        }
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteByteArray(" << name << ", true);" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadByteArray());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + ucharArrayToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
};


//
// ------------------------------- UCharVector -------------------------------
//
class JavaGeneratorUCharVector : public virtual UCharVector , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorUCharVector(const QString &n, const QString &l)
        : Field("ucharVector",n,l), UCharVector(n,l), JavaGeneratorField("ucharVector",n,l) { }

    virtual void AddImports(UniqueStringList &sl) 
    { 
        sl.AddString("import java.lang.Byte;\n");
        sl.AddString("import java.util.Vector;\n");
    }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "Vector";
    }

    virtual void WriteSourceAttribute(ostream &h, int w)
    {
        h << "    private " << GetCPPNameW(w) << " " << name << "; // vector of Byte objects" << endl;
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = new Vector();" << endl;
        for (size_t i = 0; i < val.size(); ++i)
            c << "        " << name << ".addElement(new Byte(" << val[i] << "));" << endl;
    }

    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "        " << name << " = new Vector(obj." << name << ".size());" << endl;
        c << "        for(i = 0; i < obj." << name << ".size(); ++i)" << endl;
        c << "        {" << endl;
        c << "            Byte bv = (Byte)obj." << name << ".elementAt(i);" << endl;
        c << "            " << name << ".addElement(new Byte(bv.byteValue()));" << endl;
        c << "        }" << endl;
        c << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteByteVector(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadByteVector());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + ucharVectorToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
    virtual QString GetVectorStorageName() const
    {
        return "Byte";
    }
};


//
// ---------------------------------- String ----------------------------------
//
class JavaGeneratorString : public virtual String , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorString(const QString &n, const QString &l)
        : Field("string",n,l), String(n,l), JavaGeneratorField("string",n,l) { }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "String";
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        if (valueSet)
            c << "    " << name << " = new String(\"" << val << "\");" << endl;
        else
            c << "    " << name << " = new String(\"\");" << endl;
    }

    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "        " << name << " = new String(obj." << name << ");" << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteString(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadString());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + stringToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
    virtual void WriteSourceComparison(ostream &c)
    {
        c << "(" << name << ".equals(obj." << name << "))";
    }
};


//
// ------------------------------- StringVector -------------------------------
//
class JavaGeneratorStringVector : public virtual StringVector , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorStringVector(const QString &n, const QString &l)
        : Field("stringVector",n,l), StringVector(n,l), JavaGeneratorField("stringVector",n,l) { }

    virtual void AddImports(UniqueStringList &sl) 
    { 
        sl.AddString("import java.util.Vector;\n");
    }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "Vector";
    }

    virtual void WriteSourceAttribute(ostream &h, int w)
    {
        h << "    private " << GetCPPNameW(w) << " " << name << "; // vector of String objects" << endl;
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = new Vector();" << endl;
        for (size_t i = 0; i < val.size(); ++i)
        c << "        " << name << ".addElement(new String(" << "\"" << val[i].latin1() << "\"));" << endl;
    }

    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "        " << name << " = new Vector(obj." << name << ".size());" << endl;
        c << "        for(i = 0; i < obj." << name << ".size(); ++i)" << endl;
        c << "            " << name << ".addElement(new String((String)obj." << name << ".elementAt(i)));" << endl;
        c << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteStringVector(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadStringVector());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + stringVectorToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
    virtual QString GetVectorStorageName() const
    {
        return "String";
    }
};


//
// -------------------------------- ColorTable --------------------------------
//
class JavaGeneratorColorTable : public virtual ColorTable , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorColorTable(const QString &n, const QString &l)
        : Field("colortable",n,l), ColorTable(n,l), JavaGeneratorField("colortable",n,l) { }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "String";
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = new String(\"" << val << "\");" << endl;
    }

    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "        " << name << " = new String(obj." << name << ");" << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteString(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadString());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + stringToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
    virtual void WriteSourceComparison(ostream &c)
    {
        c << "(" << name << ".equals(obj." << name << "))";
    }
};


//
// ----------------------------------- Color ----------------------------------
//
class JavaGeneratorColor : public virtual Color , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorColor(const QString &n, const QString &l)
        : Field("color",n,l), Color(n,l), JavaGeneratorField("color",n,l) { }

    virtual void AddImports(UniqueStringList &sl) 
    { 
        if(generatePlugin)
            sl.AddString("import llnl.visit.ColorAttribute;\n");
    }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "ColorAttribute";
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = new ColorAttribute(";
        if(valueSet)
        {
            if(val[3] == 255)
                c << int(val[0]) << ", " << int(val[1]) << ", "
                  << int(val[2]) << ");" << endl;
            else
                c << int(val[0]) << ", " << int(val[1]) << ", "
                  << int(val[2]) << ", " << int(val[3]) << ");" << endl;
        }
        else
            c << ");" << endl;
    }

    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "        " << name << " = new ColorAttribute(obj." << name << ");" << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    " << name << ".Write(buf);" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << name << ".Read(buf);" << endl;
        return false;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + indent + \"" << name << " = {\" + ";
        c << name << ".Red() + \", \" + ";
        c << name << ".Green() + \", \" + ";
        c << name << ".Blue() + \", \" + ";
        c << name << ".Alpha() + \"}\\n\";" << endl;
    }
};


//
// --------------------------------- LineStyle --------------------------------
//
class JavaGeneratorLineStyle : public virtual LineStyle , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorLineStyle(const QString &n, const QString &l)
        : Field("linestyle",n,l), LineStyle(n,l), JavaGeneratorField("linestyle",n,l) { }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << val << ";" << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteInt(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadInt());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + intToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
};


//
// --------------------------------- LineWidth --------------------------------
//
class JavaGeneratorLineWidth : public virtual LineWidth , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorLineWidth(const QString &n, const QString &l)
        : Field("linewidth",n,l), LineWidth(n,l), JavaGeneratorField("linewidth",n,l) { }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << val << ";" << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteInt(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadInt());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + intToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
};


//
// --------------------------------- Opacity ----------------------------------
//
class JavaGeneratorOpacity : public virtual Opacity , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorOpacity(const QString &n, const QString &l)
        : Field("opacity",n,l), Opacity(n,l), JavaGeneratorField("opacity",n,l) { }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << val << ";" << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteDouble(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadDouble());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + doubleToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
};


//
// -------------------------------- VariableName --------------------------------
//
class JavaGeneratorVariableName : public virtual VariableName,
    public virtual JavaGeneratorField
{
  public:
    JavaGeneratorVariableName(const QString &n, const QString &l)
        : Field("variablename",n,l), VariableName(n,l),
          JavaGeneratorField("variablename",n,l)
           { }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "String";
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = new String(\"";
        if(!val.isEmpty())
           c << val;
        c << "\");" << endl;
    }

    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "        " << name << " = new String(obj." << name << ");" << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteString(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadString());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + stringToString(\"" << name << "\", " << name << ", indent) + \"\\n\";" << endl;       
    }
    virtual void WriteSourceComparison(ostream &c)
    {
        c << "(" << name << ".equals(obj." << name << "))";
    }
};


//
// ------------------------------------ Att -----------------------------------
//
class JavaGeneratorAtt : public virtual Att , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorAtt(const QString &t, const QString &n, const QString &l)
        : Field("att",n,l), Att(t,n,l), JavaGeneratorField("att",n,l) { }

    virtual void AddImports(UniqueStringList &sl) 
    { 
        if(generatePlugin)
        {
            QString import;
            import.sprintf("import llnl.visit.%s;\n", attType.latin1());
            sl.AddString(import);
        }
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = new " << attType << "();" << endl;
    }

    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "        " << name << " = new " << attType << "(obj." << name << ");" << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    " << name << ".Write(buf);" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << name << ".Read(buf);" << endl;
        return false;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + indent + \"" << name << " = {\\n\" + " << name << ".toString(indent + \"    \") + indent + \"}\\n\";" << endl;
    }
    virtual void WriteSourceComparison(ostream &c)
    {
        c << "(" << name << ".equals(obj." << name << "))";
    }
};


//
// --------------------------------- AttVector --------------------------------
//
class JavaGeneratorAttVector : public virtual AttVector , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorAttVector(const QString &t, const QString &n, const QString &l)
        : Field("attVector",n,l), AttVector(t,n,l), JavaGeneratorField("attVector",n,l) { }

    virtual void AddImports(UniqueStringList &sl) 
    { 
        sl.AddString("import java.util.Vector;\n");
        if(generatePlugin)
        {
            QString import;
            import.sprintf("import llnl.visit.%s;\n", attType.latin1());
            sl.AddString(import);
        }
    }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "Vector";
    }

    virtual void WriteSourceAttribute(ostream &h, int w)
    {
        h << "    private " << GetCPPNameW(w) << " " << name << "; // vector of "<<attType<<" objects" << endl;
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = new Vector();" << endl;
    }

    virtual void WriteSourceSetFunction(ostream &c, const QString &classname)
    {
    }

    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "        // *** Copy the " << name << " field ***" << endl;
        c << "        " << name << " = new Vector(obj." << name << ".size());" << endl;
        c << "        for(i = 0; i < obj." << name << ".size(); ++i)" << endl;
        c << "        {" << endl;
        c << "            " << attType << " newObj" << " = (" << attType << ")" << name << ".elementAt(i);" << endl;
        c << "            " << name << ".addElement(new " << attType << "(newObj));" << endl;
        c << "        }" << endl << endl;
    }

    virtual void WriteSourceAGVectorFunctions(ostream &c)
    {
        QString s = attType;
        QString plural("");
        if(Name.right(1) != "s")
            plural = "s";

        // Write the Add method.
        c << "    public void Add" << Name << "(" << s << " obj)" << endl;
        c << "    {" << endl;
        c << "        " << name << ".addElement(new " << s << "(obj));" << endl;
        c << "        Select(" << index << ");" << endl;
        c << "    }" << endl << endl;

        // Write the Clear method
        c << "    public void Clear" << Name << plural << "()" << endl;
        c << "    {" << endl;
        c << "        " << name << ".clear();" << endl;
        c << "        Select(" << index << ");" << endl;
        c << "    }" << endl << endl;

        // Write the Remove method
        c << "    public void Remove" << Name << "(int index)" << endl;
        c << "    {" << endl;
        c << "        if(index >= 0 && index < " << name << ".size())" << endl;
        c << "        {" << endl;
        c << "            " << name << ".remove(index);" << endl;
        c << "            Select(" << index << ");" << endl;
        c << "        }" << endl;
        c << "    }" << endl << endl;

        // Write the GetNum method
        c << "    public int GetNum" << Name << plural << "()" << endl;
        c << "    {" << endl;
        c << "        return " << name << ".size();" << endl;
        c << "    }" << endl << endl;

        // Write the Get method
        c << "    public " << s << " Get" << Name << "(int i)" << endl;
        c << "    {" << endl;
        c << "        " << s << " tmp = (" << s << ")" << name << ".elementAt(i);" << endl;
        c << "        return tmp;" << endl;
        c << "    }" << endl << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "{" << endl;
        c << indent << "    buf.WriteInt(" << name << ".size());" << endl;
        c << indent << "    for(int i = 0; i < " << name << ".size(); ++i)" << endl;
        c << indent << "    {" << endl;
        c << indent << "        " << attType << " tmp = (" << attType << ")" << name << ".elementAt(i);" << endl;
        c << indent << "        tmp.Write(buf);" << endl;
        c << indent << "    }" << endl;
        c << indent << "}" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "{" << endl;
        c << indent << "    int len = buf.ReadInt();" << endl;
        c << indent << "    " << name << ".clear();" << endl;
        c << indent << "    for(int j = 0; j < len; ++j)" << endl;
        c << indent << "    {" << endl;
        c << indent << "        " << attType << " tmp = new " << attType << "();" << endl;
        c << indent << "        tmp.Read(buf);" << endl;
        c << indent << "        " << name << ".addElement(tmp);" << endl;
        c << indent << "    }" << endl;
        c << indent << "}" << endl;
        return false;
    }

    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + indent + \""<<name<<" = {\\n\";" << endl;
        c << indent << "for(int i = 0; i < "<<name<<".size(); ++i)" << endl;
        c << indent << "{" << endl;
        c << indent << "    AttributeSubject s = (AttributeSubject)"<<name<<".elementAt(i);" << endl;
        c << indent << "    str = str + s.toString(indent + \"    \");" << endl;
        c << indent << "    if(i < "<<name<<".size()-1)" << endl;
        c << indent << "        str = str + \", \";" << endl;
        c << indent << "    str = str + \"\\n\";" << endl;
        c << indent << "}" << endl;
        c << indent << "str = str + \"}\\n\";" << endl;
    }

    virtual QString GetVectorStorageName() const
    {
        return attType;
    }
};


//
// ----------------------------------- Enum -----------------------------------
//
class JavaGeneratorEnum : public virtual Enum , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorEnum(const QString &t, const QString &n, const QString &l)
        : Field("enum",n,l), Enum(t,n,l), JavaGeneratorField("enum",n,l) { }

    virtual QString GetCPPName(bool, const QString &) 
    {
        return "int";
    }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        if (valueSet)
        {
            QString constName(enumType->type + QString("_") + enumType->values[val]);
            c << "    " << name << " = " << constName.upper() << ";" << endl;
        }
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteInt(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadInt());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + indent + \"" << name <<" = \";" << endl;
        for(size_t i = 0; i < enumType->values.size(); ++i)
        {
            QString constName(enumType->type + QString("_") + enumType->values[i]);
            c << indent << "if(" << name << " == " << constName.upper() << ")" << endl;
            c << indent << "    str = str + \"" << constName.upper() << "\";" << endl;
        }
        c << indent << "str = str + \"\\n\";" << endl;
    }
};


//
// --------------------------------- ScaleMode --------------------------------
//
class JavaGeneratorScaleMode : public virtual ScaleMode , public virtual JavaGeneratorField
{
  public:
    JavaGeneratorScaleMode(const QString &n, const QString &l)
        : Field("scalemode",n,l), ScaleMode(n,l), JavaGeneratorField("scalemode",n,l) { }

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << val << ";" << endl;
    }

    virtual void WriteSourceWriteAtts(ostream &c, const QString &indent)
    {
        c << indent << "    buf.WriteInt(" << name << ");" << endl;
    }

    virtual bool WriteSourceReadAtts(ostream &c, const QString &indent)
    {
        c << indent << "Set" << Name << "(buf.ReadInt());" << endl;
        return true;
    }
    virtual void WriteToString(ostream &c, const QString &indent)
    {
        c << indent << "str = str + intToString(\"" << name << "\", " << name << ", indent);" << endl;       
    }
};


// ----------------------------------------------------------------------------
// Modifications:
//   Brad Whitlock, Wed Dec 8 15:52:11 PST 2004
//   Added support for variable names.
//
//   Kathleen Bonnell, Thu Mar 22 16:58:23 PDT 2007 
//   Added scalemode.
//
// ----------------------------------------------------------------------------

class JavaFieldFactory
{
  public:
    static JavaGeneratorField *createField(const QString &name,
                                           const QString &type,
                                           const QString &subtype,
                                           const QString &length,
                                           const QString &label)
    {
        JavaGeneratorField *f = NULL;
        if      (type.isNull())          throw QString().sprintf("Field %s was specified with no type.",name.latin1());
        else if (type == "int")          f = new JavaGeneratorInt(name,label);
        else if (type == "intArray")     f = new JavaGeneratorIntArray(length,name,label);
        else if (type == "intVector")    f = new JavaGeneratorIntVector(name,label);
        else if (type == "bool")         f = new JavaGeneratorBool(name,label);
        else if (type == "float")        f = new JavaGeneratorFloat(name,label);
        else if (type == "floatArray")   f = new JavaGeneratorFloatArray(length,name,label);
        else if (type == "double")       f = new JavaGeneratorDouble(name,label);
        else if (type == "doubleArray")  f = new JavaGeneratorDoubleArray(length,name,label);
        else if (type == "doubleVector") f = new JavaGeneratorDoubleVector(name,label);
        else if (type == "uchar")        f = new JavaGeneratorUChar(name,label);
        else if (type == "ucharArray")   f = new JavaGeneratorUCharArray(length,name,label);
        else if (type == "ucharVector")  f = new JavaGeneratorUCharVector(name,label);
        else if (type == "string")       f = new JavaGeneratorString(name,label);
        else if (type == "stringVector") f = new JavaGeneratorStringVector(name,label);
        else if (type == "colortable")   f = new JavaGeneratorColorTable(name,label);
        else if (type == "color")        f = new JavaGeneratorColor(name,label);
        else if (type == "opacity")      f = new JavaGeneratorOpacity(name,label);
        else if (type == "linestyle")    f = new JavaGeneratorLineStyle(name,label);
        else if (type == "linewidth")    f = new JavaGeneratorLineWidth(name,label);
        else if (type == "variablename") f = new JavaGeneratorVariableName(name,label);
        else if (type == "att")          f = new JavaGeneratorAtt(subtype,name,label);
        else if (type == "attVector")    f = new JavaGeneratorAttVector(subtype,name,label);
        else if (type == "enum")         f = new JavaGeneratorEnum(subtype, name, label);
        else if (type == "scalemode")    f = new JavaGeneratorScaleMode(name,label);

        // Special built-in AVT enums -- but they don't really need to be treated like enums for this program.
        else if (type == "avtCentering")      f = new JavaGeneratorInt(name, label);
        else if (type == "avtVarType")        f = new JavaGeneratorInt(name, label);
        else if (type == "avtSubsetType")     f = new JavaGeneratorInt(name, label);
        else if (type == "avtExtentType")     f = new JavaGeneratorInt(name, label);
        else if (type == "avtMeshType")       f = new JavaGeneratorInt(name, label);
        else if (type == "avtGhostType")      f = new JavaGeneratorInt(name, label);
        else if (type == "avtMeshCoordType")  f = new JavaGeneratorInt(name, label);
        else if (type == "LoadBalanceScheme") f = new JavaGeneratorInt(name, label);

        if (!f)
            throw QString().sprintf("JavaFieldFactory: unknown type for field %s: %s",name.latin1(),type.latin1());

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

class JavaGeneratorAttribute : public GeneratorBase
{
  public:
    vector<JavaGeneratorField*> fields;
    QString pluginVersion;
    QString pluginName;
    QString pluginType;
  public:
    JavaGeneratorAttribute(const QString &n, const QString &p, const QString &f,
                           const QString &e, const QString &ei)
        : GeneratorBase(n,p,f,e,ei, GENERATOR_NAME), fields(), pluginVersion("1.0"),
          pluginName(), pluginType()
    {
    }

    virtual ~JavaGeneratorAttribute()
    {
        for (size_t i = 0; i < fields.size(); ++i)
            delete fields[i];
        fields.clear();
    }

    void Print(ostream &out)
    {
        out << "    Attribute: " << name << " (" << purpose << ")" << endl;
        out << "        exportAPI=" << exportAPI << endl;
        out << "        exportInclude=" << exportInclude << endl;
        size_t i;
        for (i = 0; i < fields.size(); ++i)
            fields[i]->Print(out);
        for (i=0; i<includes.size(); i++)
            includes[i]->Print(out, generatorName);
        for (i=0; i<functions.size(); i++)
            functions[i]->Print(out, generatorName);
        for (i=0; i<constants.size(); i++)
            constants[i]->Print(out, generatorName);
    }

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    void WriteSource(ostream &h)
    {
        bool generatePlugin = (pluginType == "plot" || pluginType == "operator");
        h << java_copyright_str.c_str() << endl;

        if(pluginType == "plot")
            h << "package llnl.visit.plots;" << endl;
        else if(pluginType == "operator")
            h << "package llnl.visit.operators;" << endl;
        else
            h << "package llnl.visit;" << endl;
        h << endl;

        // Give a little information to the fields.
        size_t i;
        for (i = 0; i < fields.size(); ++i)
            fields[i]->generatePlugin = generatePlugin;

        //
        // Write the list of imported classes
        //
        WriteSourceImports(h);

        //
        // Write the class header comment
        //
        WriteClassComment(h, purpose);

        if(generatePlugin)
            h << "public class " << name << " extends AttributeSubject implements Plugin" << endl;
        else
            h << "public class " << name << " extends AttributeSubject" << endl;
        h << "{" << endl;

        //
        // Write the source code representation of any enums that have been specified.
        //
        WriteSourceEnumsAndConstants(h);

        //
        // Write the no argument constructor.
        //
        WriteSourceConstructor(h);

        //
        // Write the copy constructor.
        //
        WriteSourceCopyConstructor(h);

        //
        // Write the comparison method.
        //
        WriteSourceComparison(h);

        //
        // Write Plugin interface methods.
        //
        if(generatePlugin)
        {
            h << "    public String GetName() { return \"" << pluginName << "\"; }" << endl;
            h << "    public String GetVersion() { return \"" << pluginVersion << "\"; }" << endl;
            h << endl;
        }

        //
        // Write out all the set prototypes
        //
        h << "    // Property setting methods" << endl;
        for (i = 0; i < fields.size(); ++i)
            fields[i]->WriteSourceSetFunction(h, name);

        //
        // Write out all the get prototypes
        //
        int totalWidth = CalculateTotalWidth();
        h << "    // Property getting methods" << endl;
        for (i = 0; i < fields.size(); ++i)
        {
            fields[i]->WriteSourceGetFunction(h, totalWidth);
        }
        h << endl;

        //
        // Write the WriteAtts method.
        //
        WriteSourceWriteAtts(h);

        //
        // Write the ReadAtts method
        //
        WriteSourceReadAtts(h);

        //
        // Write the toString method
        //
        WriteToString(h);

        //
        // Write out AttributeGroupVector convenience methods.
        //
        WriteSourceAGVectorFunctions(h);

        //
        // Write user-defined functions 
        //
        WriteUserDefinedFunctions(h);

        //
        // Write out all the private attributes
        //
        h << "    // Attributes" << endl;
        for (i = 0; i < fields.size(); ++i)
        {
            fields[i]->WriteSourceAttribute(h, totalWidth);
        }

        h << "}" << endl;
        h << endl;
    }

private:
    void WriteSourceImports(ostream &h)
    {
        UniqueStringList sysincludes;

        if(pluginType == "plot" || pluginType == "operator")
        {
            sysincludes.AddString("import llnl.visit.AttributeSubject;\n");
            sysincludes.AddString("import llnl.visit.CommunicationBuffer;\n");
            sysincludes.AddString("import llnl.visit.Plugin;\n");
        }

        for (size_t i = 0; i < fields.size(); ++i)
            fields[i]->AddImports(sysincludes);

        // Add some includes based on the includes from the XML file
        for(size_t i = 0; i < includes.size(); ++i)
        {
            if(includes[i]->target == generatorName)
            {
                QString importLine;
                importLine.sprintf("import %s;\n", includes[i]->include.latin1());
                sysincludes.AddString(importLine);
            }
        }

        sysincludes.Write(h);
        h << endl;
    }

    int CalculateTotalWidth()
    {
        int retval = 0;

        // Iterate through the list of attibutes and find the one with
        // the longest name.
        for (size_t i = 0; i < fields.size(); ++i)
        {
            int len = fields[i]->GetCPPName().length();

            if (len > retval)
                retval = len;
        }
 
        return retval;
    }

    bool HaveAGVectors()
    {
        for (size_t i = 0; i < fields.size(); ++i)
            if (fields[i]->type=="attVector")
                return true;
        return false;
    }

    bool HaveArrays()
    {
        for (size_t i = 0; i < fields.size(); ++i)
            if (fields[i]->isArray)
                return true;
        return false;
    }

    bool HaveVectors()
    {
        for (size_t i = 0; i < fields.size(); ++i)
            if (fields[i]->isVector)
                return true;
        return false;
    }

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------

    void WriteSourceConstructor(ostream &c)
    {
        c << "    public " << name << "()" << endl;
        c << "    {" << endl;
        c << "        super(" << fields.size() << ");" << endl;
        c << endl;

        if(HasCode(name, 0))
            PrintCode(c, name, 0);

        size_t i;
        for (i = 0; i < fields.size(); ++i)
        {
            c << "    ";
            if(!fields[i]->PrintInit(c, generatorName))
                fields[i]->WriteSourceSetDefault(c);
        }

        if(HasCode(name, 1))
            PrintCode(c, name, 1);

        c << "    }" << endl << endl;
    }

    void WriteSourceCopyConstructor(ostream &c)
    {
        c << "    public " << name << "(" << name << " obj)" << endl;
        c << "    {" << endl;
        c << "        super(" << fields.size() << ");" << endl;
        c << endl;

        bool skipLine = false;
        if (HaveArrays() || HaveVectors())
        {
            c << "        int i;" << endl;
            skipLine = true;
        }
        if(skipLine)
            c << endl;
        for (size_t i = 0; i < fields.size(); ++i)
        {
            fields[i]->WriteSourceCopyCode(c);
        }
        c << endl << "        SelectAll();" << endl;
        c << "    }" << endl << endl;
    }

    void WriteSourceComparison(ostream &c)
    {
        if(HasFunction("equals"))
        {
            PrintFunction(c, "equals");
            c << endl;
            return;
        }
        c << "    public boolean equals(" << name << " obj)" << endl;
        c << "    {" << endl;

        if(HaveArrays() || HaveVectors())
        {
            c << "        int i;" << endl << endl;
        }

        // Create bool values to evaluate the arrays.
        QString prevValue("true");
        for (size_t i = 0; i < fields.size(); ++i)
        {
            if (!fields[i]->ignoreEquality)
                fields[i]->WriteSourceComparisonPrecalc(c);
        }

        c << "        // Create the return value" << endl;
        c << "        return (";

        // Create a big boolean return statement.
        if (fields.size() == 0)
        {
            c << "true";
        }
        else
        {
            for (size_t i = 0; i < fields.size(); ++i)
            {
                if (i > 0)
                    c << "                ";

                if (!fields[i]->ignoreEquality)
                    fields[i]->WriteSourceComparison(c);
                else
                    c << "true /* can ignore " << fields[i]->name << " */";

                if (i < fields.size() - 1)
                    c << " &&" << endl;
            }
        }
        c << ");" << endl;
        c << "    }" << endl << endl;
    }

    void WriteSourceEnumsAndConstants(ostream &h)
    {
        // Write the enums out as groups of static int constants.
        if(EnumType::enums.size() > 0)
            h << "    // Enum values" << endl;
        size_t i;
        for (i = 0; i < EnumType::enums.size(); ++i)
        {
            for (size_t j = 0; j < EnumType::enums[i]->values.size(); ++j)
            {
                QString constName(EnumType::enums[i]->type + QString("_") + EnumType::enums[i]->values[j]);
                h << "    public final static int " << constName.upper() << " = " << j << ";" << endl;
            }
            h << endl;
        }

        //
        // Write any constants that have been specified.
        //
        bool haveConstants = false;
        for (i = 0; i < constants.size(); ++i)
        {
            if(constants[i]->target == generatorName)
                haveConstants = true;
        }
        if(haveConstants)
            h << "    // Constants" << endl;
        for (i = 0; i < constants.size(); ++i)
        {
            if(constants[i]->target != generatorName)
                continue;

            QString def(constants[i]->def);
            if (def.simplifyWhiteSpace().isEmpty())
                continue;

            h << def << endl;
        }

        if (EnumType::enums.size() || constants.size())
            h << endl;
    }

    void WriteSourceWriteAtts(ostream &h)
    {
        h << "    // Write and read methods." << endl;
        if(HasFunction("WriteAtts"))
        {
            PrintFunction(h, "WriteAtts");
            h << endl;
            return;
        }
        h << "    public void WriteAtts(CommunicationBuffer buf)" << endl;
        h << "    {" << endl;
        for (size_t i = 0; i < fields.size(); ++i)
        {
            h << "        if(WriteSelect(" << i << ", buf))" << endl;
            fields[i]->WriteSourceWriteAtts(h, "        ");
        }
        h << "    }" << endl;
        h << endl;
    }

    void WriteSourceReadAtts(ostream &h)
    {
        if(HasFunction("ReadAtts"))
        {
            PrintFunction(h, "ReadAtts");
            h << endl;
            return;
        }
        h << "    public void ReadAtts(int n, CommunicationBuffer buf)" << endl;
        h << "    {" << endl;
        if(fields.size() > 1)
        {
            h << "        for(int i = 0; i < n; ++i)" << endl;
            h << "        {" << endl;
            h << "            int index = (int)buf.ReadByte();" << endl;
            h << "            switch(index)" << endl;
            h << "            {" << endl;
            for (size_t i = 0; i < fields.size(); ++i)
            {
                h << "            case " << i << ":" << endl;
                if(!fields[i]->WriteSourceReadAtts(h, "                "))
                    h << "                Select(" << i << ");" << endl;
                h << "                break;" << endl;
            }
            h << "            }" << endl;
            h << "        }" << endl;
        }
        else if(fields.size() == 1)
        {
            h << "        buf.ReadByte();" << endl;
            if(!fields[0]->WriteSourceReadAtts(h, "        "))
                 h << "        Select(0);" << endl;
        }
        h << "    }" << endl;
        h << endl;
    }

    void WriteSourceAGVectorFunctions(ostream &h)
    {
        if (HaveAGVectors())
        {
            h << "    // Attributegroup convenience methods" << endl;
            for (size_t i = 0; i < fields.size(); ++i)
                fields[i]->WriteSourceAGVectorFunctions(h);
        }
        h << endl;
    }

    void WriteToString(ostream &h)
    {
        h << "    public String toString(String indent)" << endl;
        h << "    {" << endl;
        if(HasCode("toString", 0))
            PrintCode(h, "toString", 0);
        h << "        String str = new String();" << endl;
        for (size_t i = 0; i < fields.size(); ++i)
        {
            fields[i]->WriteToString(h, "        ");
        }
        if(HasCode("toString", 1))
            PrintCode(h, "toString", 1);
        h << "        return str;" << endl;
        h << "    }" << endl;
        h << endl;
//        h << "    public String toString()" << endl;
//        h << "    {" << endl;
//        h << "        return toString(new String());" << endl;
//        h << "    }" << endl;
//        h << endl;
    }

    void WriteUserDefinedFunctions(ostream &h)
    {
        for (size_t i=0; i<functions.size(); i++)
            if (functions[i]->target == generatorName)
            {
                h << functions[i]->def << endl;
            }
    }
};

// ----------------------------------------------------------------------------
//  Modifications:
//
//   Hank Childs, Thu Jan 10 14:33:30 PST 2008
//   Added filenames, specifiedFilenames.
//
//   Brad Whitlock, Thu Feb 28 16:26:46 PST 2008
//   Made it use a base class.
//
// ----------------------------------------------------------------------------
#include <PluginBase.h>

class JavaGeneratorPlugin : public PluginBase
{
  public:
    JavaGeneratorAttribute *atts;
  public:
    JavaGeneratorPlugin(const QString &n,const QString &l,const QString &t,
        const QString &vt,const QString &dt, const QString &v, const QString &ifile,
        bool hw, bool ho, bool onlyengine, bool noengine) : 
        PluginBase(n,l,t,vt,dt,v,ifile,hw,ho,onlyengine,noengine), atts(NULL)
    {
    }

    void Print(ostream &out)
    {
        out << "Plugin: "<<name<<" (\""<<label<<"\", type="<<type<<") -- version "<<version<< endl;
        if (atts)
            atts->Print(out);
    }
};


// ----------------------------------------------------------------------------
//                           Override default types
// ----------------------------------------------------------------------------
#define FieldFactory JavaFieldFactory
#define Field        JavaGeneratorField
#define Attribute    JavaGeneratorAttribute
#define Enum         JavaGeneratorEnum
#define Plugin       JavaGeneratorPlugin

#endif
