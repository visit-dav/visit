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
// ****************************************************************************

// ----------------------------------------------------------------------------
//                             Utility Functions
// ----------------------------------------------------------------------------

QString
CurrentTime()
{
    char *tstr[] = {"PDT", "PST"};
    char s1[10], s2[10], s3[10], tmpbuf[200];
    time_t t;
    char *c = NULL;
    int h,m,s,y;
    t = time(NULL);
    c = asctime(localtime(&t));
    // Read the hour.
    sscanf(c, "%s %s %s %d:%d:%d %d", s1, s2, s3, &h, &m, &s, &y);
    // Reformat the string a little.
    sprintf(tmpbuf, "%s %s %s %02d:%02d:%02d %s %d",
            s1, s2, s3, h, m, s, tstr[h > 12], y);

    return QString(tmpbuf);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class AttsGeneratorField : public virtual Field
{
  public:
    bool generatePlugin;

    AttsGeneratorField(const QString &t, const QString &n, const QString &l)
        : Field(t,n,l)
    {
        generatePlugin = false;
    }

    QString GetCPPNameW(int w, bool subtypename=false, const QString &classname="")
    {
        QString s = GetCPPName(subtypename,classname);
        for (int i = w - s.length(); i > 0; --i)
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
    }

    virtual void WriteSourceComparison(ostream &c)
    {
        if (isArray)
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
class AttsGeneratorInt : public virtual Int , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorInt(const QString &n, const QString &l)
        : Int(n,l), AttsGeneratorField("int",n,l), Field("int",n,l) { }

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
class AttsGeneratorIntArray : public virtual IntArray , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorIntArray(const QString &s, const QString &n, const QString &l)
        : IntArray(s,n,l), AttsGeneratorField("intArray",n,l), Field("intArray",n,l) { }

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
class AttsGeneratorIntVector : public virtual IntVector , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorIntVector(const QString &n, const QString &l)
        : IntVector(n,l), AttsGeneratorField("intVector",n,l), Field("intVector",n,l) { }

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
            for (int i = 0; i < val.size(); ++i)
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
};


// 
// ----------------------------------- Bool -----------------------------------
//
class AttsGeneratorBool : public virtual Bool , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorBool(const QString &n, const QString &l)
        : Bool(n,l), AttsGeneratorField("boolean",n,l), Field("boolean",n,l) { }

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
class AttsGeneratorFloat : public virtual Float , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorFloat(const QString &n, const QString &l)
        : Float(n,l), AttsGeneratorField("float",n,l), Field("float",n,l) { }

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
class AttsGeneratorFloatArray : public virtual FloatArray , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorFloatArray(const QString &s, const QString &n, const QString &l)
        : FloatArray(s,n,l), AttsGeneratorField("floatArray",n,l), Field("floatArray",n,l) { }

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
class AttsGeneratorDouble : public virtual Double , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorDouble(const QString &n, const QString &l)
        : Double(n,l), AttsGeneratorField("double",n,l), Field("double",n,l) { }

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
class AttsGeneratorDoubleArray : public virtual DoubleArray , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorDoubleArray(const QString &s, const QString &n, const QString &l)
        : DoubleArray(s,n,l), AttsGeneratorField("doubleArray",n,l), Field("doubleArray",n,l) { }

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
class AttsGeneratorDoubleVector : public virtual DoubleVector , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorDoubleVector(const QString &n, const QString &l)
        : DoubleVector(n,l), AttsGeneratorField("doubleVector",n,l), Field("doubleVector",n,l) { }

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
        for (int i = 0; i < val.size(); ++i)
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
};


//
// ----------------------------------- UChar ----------------------------------
//
class AttsGeneratorUChar : public virtual UChar , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorUChar(const QString &n, const QString &l)
        : UChar(n,l), AttsGeneratorField("uchar",n,l), Field("uchar",n,l) { }

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
class AttsGeneratorUCharArray : public virtual UCharArray , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorUCharArray(const QString &s, const QString &n, const QString &l)
        : UCharArray(s,n,l), AttsGeneratorField("byte",n,l), Field("ucharArray",n,l) { }

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
                c << "        " << name << "["<<i<<"] = " << int(val[i]) << ";" << endl;
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
class AttsGeneratorUCharVector : public virtual UCharVector , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorUCharVector(const QString &n, const QString &l)
        : UCharVector(n,l), AttsGeneratorField("ucharVector",n,l), Field("ucharVector",n,l) { }

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
        for (int i = 0; i < val.size(); ++i)
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
};


//
// ---------------------------------- String ----------------------------------
//
class AttsGeneratorString : public virtual String , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorString(const QString &n, const QString &l)
        : String(n,l), AttsGeneratorField("string",n,l), Field("string",n,l) { }

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
};


//
// ------------------------------- StringVector -------------------------------
//
class AttsGeneratorStringVector : public virtual StringVector , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorStringVector(const QString &n, const QString &l)
        : StringVector(n,l), AttsGeneratorField("stringVector",n,l), Field("stringVector",n,l) { }

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
        for (int i = 0; i < val.size(); ++i)
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
};


//
// -------------------------------- ColorTable --------------------------------
//
class AttsGeneratorColorTable : public virtual ColorTable , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorColorTable(const QString &n, const QString &l)
        : ColorTable(n,l), AttsGeneratorField("colortable",n,l), Field("colortable",n,l) { }

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
};


//
// ----------------------------------- Color ----------------------------------
//
class AttsGeneratorColor : public virtual Color , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorColor(const QString &n, const QString &l)
        : Color(n,l), AttsGeneratorField("color",n,l), Field("color",n,l) { }

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
class AttsGeneratorLineStyle : public virtual LineStyle , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorLineStyle(const QString &n, const QString &l)
        : LineStyle(n,l), AttsGeneratorField("linestyle",n,l), Field("linestyle",n,l) { }

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
class AttsGeneratorLineWidth : public virtual LineWidth , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorLineWidth(const QString &n, const QString &l)
        : LineWidth(n,l), AttsGeneratorField("linewidth",n,l), Field("linewidth",n,l) { }

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
class AttsGeneratorOpacity : public virtual Opacity , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorOpacity(const QString &n, const QString &l)
        : Opacity(n,l), AttsGeneratorField("opacity",n,l), Field("opacity",n,l) { }

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
class AttsGeneratorVariableName : public virtual VariableName,
    public virtual AttsGeneratorField
{
  public:
    AttsGeneratorVariableName(const QString &n, const QString &l)
        : VariableName(n,l), AttsGeneratorField("variablename",n,l),
          Field("variablename",n,l) { }

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
};


//
// ------------------------------------ Att -----------------------------------
//
class AttsGeneratorAtt : public virtual Att , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorAtt(const QString &t, const QString &n, const QString &l)
        : Att(t,n,l), AttsGeneratorField("att",n,l), Field("att",n,l) { }

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
};


//
// --------------------------------- AttVector --------------------------------
//
class AttsGeneratorAttVector : public virtual AttVector , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorAttVector(const QString &t, const QString &n, const QString &l)
        : AttVector(t,n,l), AttsGeneratorField("attVector",n,l), Field("attVector",n,l) { }

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

    virtual void WriteSourceComparisonPrecalc(ostream &c)
    {
        QString s = attType;
        c << "        boolean " << name << "_equal = (obj." << name 
          << ".size() == " << name << ".size());" << endl;
        c << "        for(i = 0; (i < " << name << ".size()) && " << name
          << "_equal; ++i)" << endl;
        c << "        {" << endl;
        c << "            // Make references to " << s << " from Object." << endl;
        c << "            " << s << " " << name << "1 = (" << s << ")"
          << name << ".elementAt(i);" << endl;
        c << "            " << s << " " << name << "2 = (" << s << ")obj."
          << name << ".elementAt(i);" << endl;
        c << "            " << name << "_equal = "
          << name << "1.equals(" << name << "2);" << endl;
        c << "        }" << endl << endl;
    }

    virtual void WriteSourceComparison(ostream &c)
    {
        c << name << "_equal";
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
};


//
// ----------------------------------- Enum -----------------------------------
//
class AttsGeneratorEnum : public virtual Enum , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorEnum(const QString &t, const QString &n, const QString &l)
        : Enum(t,n,l), AttsGeneratorField("enum",n,l), Field("enum",n,l) { }

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
        for(int i = 0; i < enumType->values.size(); ++i)
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
class AttsGeneratorScaleMode : public virtual ScaleMode , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorScaleMode(const QString &n, const QString &l)
        : ScaleMode(n,l), AttsGeneratorField("scalemode",n,l), Field("scalemode",n,l) { }

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

class AttsFieldFactory
{
  public:
    static AttsGeneratorField *createField(const QString &name,
                                           const QString &type,
                                           const QString &subtype,
                                           const QString &length,
                                           const QString &label)
    {
        AttsGeneratorField *f = NULL;
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
        else if (type == "enum")         f = new AttsGeneratorEnum(subtype, name, label);
        else if (type == "scalemode")    f = new AttsGeneratorScaleMode(name,label);

        // Special built-in AVT enums -- but they don't really need to be treated like enums for this program.
        else if (type == "avtCentering")      f = new AttsGeneratorInt(name, label);
        else if (type == "avtVarType")        f = new AttsGeneratorInt(name, label);
        else if (type == "avtSubsetType")     f = new AttsGeneratorInt(name, label);
        else if (type == "avtExtentType")     f = new AttsGeneratorInt(name, label);
        else if (type == "avtMeshType")       f = new AttsGeneratorInt(name, label);
        else if (type == "avtGhostType")      f = new AttsGeneratorInt(name, label);
        else if (type == "avtMeshCoordType")  f = new AttsGeneratorInt(name, label);
        else if (type == "LoadBalanceScheme") f = new AttsGeneratorInt(name, label);

        if (!f)
            throw QString().sprintf("AttsFieldFactory: unknown type for field %s: %s",name.latin1(),type.latin1());

        return f;
    }
};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class AttsGeneratorAttribute
{
  public:
    QString name;
    QString purpose;
    bool    persistent, keyframe;
    QString exportAPI;
    QString exportInclude;
    QString pluginType;
    QString pluginName;
    QString pluginVersion;

    vector<AttsGeneratorField*> fields;
    vector<Function*> functions;
    vector<Constant*> constants;
    vector<Include*>  includes;
    vector<Code*>     codes;
    CodeFile *codeFile;
  public:
    AttsGeneratorAttribute(const QString &n, const QString &p, const QString &f,
                           const QString &e, const QString &ei)
        : name(n), purpose(p), exportAPI(e), exportInclude(ei), pluginType(""),
          pluginName(""), pluginVersion("1.0")
    {
        if (f.isNull())
            codeFile = NULL;
        else
            codeFile = new CodeFile(f);
        if (codeFile)
            codeFile->Parse();
        persistent = false;
    }

    virtual ~AttsGeneratorAttribute()
    {
        int i;
        for (i = 0; i < fields.size(); ++i)
            delete fields[i];
        fields.clear();
        for (i = 0; i < functions.size(); ++i)
            delete functions[i];
        functions.clear();
        for (i = 0; i < constants.size(); ++i)
            delete constants[i];
        constants.clear();
        for (i = 0; i < includes.size(); ++i)
            delete includes[i];
        includes.clear();

        delete codeFile;
    }

    void Print(ostream &out)
    {
        out << "    Attribute: " << name << " (" << purpose << ")" << endl;
        out << "        exportAPI=" << exportAPI << endl;
        out << "        exportInclude=" << exportInclude << endl;
        int i;
        for (i = 0; i < fields.size(); ++i)
            fields[i]->Print(out);
        for (i = 0; i<includes.size(); ++i)
            includes[i]->Print(out);
        for (i = 0; i<functions.size(); ++i)
            functions[i]->Print(out);
        for (i = 0; i<constants.size(); ++i)
            constants[i]->Print(out);
    }

    void WriteSourceClassComment(ostream &h)
    {
        h << "// ****************************************************************************" << endl;
        h << "// Class: " << name << endl;
        h << "//" << endl;
        h << "// Purpose:" << endl;
        h << "//    " << purpose << endl;
        h << "//" << endl;
        h << "// Notes:      Autogenerated by xml2java." << endl;
        h << "//" << endl;
        h << "// Programmer: xml2java" << endl;
        h << "// Creation:   " << CurrentTime() << endl;
        h << "//" << endl; 
        h << "// Modifications:" << endl;
        h << "//   " << endl;
        h << "// ****************************************************************************" << endl;
        h << endl;
    }

    void WriteSourceImports(ostream &h)
    {
        UniqueStringList sysincludes;

        if(pluginType == "plot" || pluginType == "operator")
        {
            sysincludes.AddString("import llnl.visit.AttributeSubject;\n");
            sysincludes.AddString("import llnl.visit.CommunicationBuffer;\n");
            sysincludes.AddString("import llnl.visit.Plugin;\n");
        }

        for (int i = 0; i < fields.size(); ++i)
            fields[i]->AddImports(sysincludes);
        sysincludes.Write(h);
        h << endl;
    }

    int CalculateTotalWidth()
    {
        int retval = 0;

        // Iterate through the list of attibutes and find the one with
        // the longest name.
        for (int i = 0; i < fields.size(); ++i)
        {
            int len = fields[i]->GetCPPName().length();

            if (len > retval)
                retval = len;
        }
 
        return retval;
    }

    bool HaveAGVectors()
    {
        for (int i = 0; i < fields.size(); ++i)
            if (fields[i]->type=="attVector")
                return true;
        return false;
    }

    bool HaveArrays()
    {
        for (int i = 0; i < fields.size(); ++i)
            if (fields[i]->isArray)
                return true;
        return false;
    }

    bool HaveVectors()
    {
        for (int i = 0; i < fields.size(); ++i)
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

        for (int i = 0; i < fields.size(); ++i)
        {
            c << "    ";
            fields[i]->WriteSourceSetDefault(c);
        }
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
        for (int i = 0; i < fields.size(); ++i)
        {
            fields[i]->WriteSourceCopyCode(c);
        }
        c << endl << "        SelectAll();" << endl;
        c << "    }" << endl << endl;
    }

    void WriteSourceComparison(ostream &c)
    {
        c << "    public boolean equals(" << name << " obj)" << endl;
        c << "    {" << endl;

        if(HaveArrays() || HaveVectors())
        {
            c << "        int i;" << endl << endl;
        }

        // Create bool values to evaluate the arrays.
        QString prevValue("true");
        for (int i = 0; i < fields.size(); ++i)
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
            for (int i = 0; i < fields.size(); ++i)
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
        int i;
        for (i = 0; i < EnumType::enums.size(); ++i)
        {
            for (int j = 0; j < EnumType::enums[i]->values.size(); ++j)
            {
                QString constName(EnumType::enums[i]->type + QString("_") + EnumType::enums[i]->values[j]);
                h << "    public final static int " << constName.upper() << " = " << j << ";" << endl;
            }
            h << endl;
        }

        //
        // Write any constants that have been specified.
        //
        if(constants.size() > 0)
            h << "    // Constants" << endl;
        for (i = 0; i < constants.size(); ++i)
        {
            QString def(constants[i]->def);
            if (def.simplifyWhiteSpace().isEmpty())
                continue;

            // Remove const
            int index = def.find("const");
            if(index >= 0)
            {
                def.remove(index, 6);
            }

            // Remove the class name
            index = def.find("::");
            if(index >= 0)
            {
                int index2 = def.findRev(" ", index);
                if(index2 >= 0)
                    def.remove(index2+1, index-index2+1);
            }

            // Find an equals sign and then look to see if the constant
            // value matches any of the enums' values. If so, alter the
            // constant definition so it prints the right enum value.
            bool noEnumMatches = true;
            index = def.find("=");
            if(index >= 0)
            {
                QString value(def.mid(index + 2, def.length() - index - 4));
                for (int j = 0; j < EnumType::enums.size() && noEnumMatches; ++j)
                {
                    for (int k = 0; k < EnumType::enums[j]->values.size() && noEnumMatches; ++k)
                    {
                        if(EnumType::enums[j]->values[k] == value)
                        {
                            noEnumMatches = false;
                            QString constName(EnumType::enums[j]->type + QString("_") + EnumType::enums[j]->values[k]);
                            h << "    public final static " << def.left(index + 2)
                              << constName.upper() << ";\n" << endl;
                        }
                    }
                }
            }

            if(noEnumMatches)
                h << "    public final static " << def << endl;
        }

        if (EnumType::enums.size() || constants.size())
            h << endl;
    }

    void WriteSourceWriteAtts(ostream &h)
    {
        h << "    // Write and read methods." << endl;
        h << "    public void WriteAtts(CommunicationBuffer buf)" << endl;
        h << "    {" << endl;
        for (int i = 0; i < fields.size(); ++i)
        {
            h << "        if(WriteSelect(" << i << ", buf))" << endl;
            fields[i]->WriteSourceWriteAtts(h, "        ");
        }
        h << "    }" << endl;
        h << endl;
    }

    void WriteSourceReadAtts(ostream &h)
    {
        h << "    public void ReadAtts(int n, CommunicationBuffer buf)" << endl;
        h << "    {" << endl;
        if(fields.size() > 1)
        {
            h << "        for(int i = 0; i < n; ++i)" << endl;
            h << "        {" << endl;
            h << "            int index = (int)buf.ReadByte();" << endl;
            h << "            switch(index)" << endl;
            h << "            {" << endl;
            for (int i = 0; i < fields.size(); ++i)
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
            for (int i = 0; i < fields.size(); ++i)
                fields[i]->WriteSourceAGVectorFunctions(h);
        }
        h << endl;
    }

    void WriteToString(ostream &h)
    {
        h << "    public String toString(String indent)" << endl;
        h << "    {" << endl;
        h << "        String str = new String();" << endl;
        for (int i = 0; i < fields.size(); ++i)
        {
            fields[i]->WriteToString(h, "        ");
        }
        h << "        return str;" << endl;
        h << "    }" << endl;
        h << endl;
//        h << "    public String toString()" << endl;
//        h << "    {" << endl;
//        h << "        return toString(new String());" << endl;
//        h << "    }" << endl;
//        h << endl;
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
        int i;
        for (i = 0; i < fields.size(); ++i)
            fields[i]->generatePlugin = generatePlugin;

        //
        // Write the list of imported classes
        //
        WriteSourceImports(h);

        //
        // Write the class header comment
        //
        WriteSourceClassComment(h);

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
};

// ----------------------------------------------------------------------------
//  Modifications:
//
//   Hank Childs, Thu Jan 10 14:33:30 PST 2008
//   Added filenames, specifiedFilenames.
//
// ----------------------------------------------------------------------------
class AttsGeneratorPlugin
{
  public:
    QString name;
    QString type;
    QString label;
    QString version;
    QString vartype;
    QString dbtype;
    bool    enabledByDefault;
    bool    has_MDS_specific_code;
    bool    hasEngineSpecificCode;
    bool    specifiedFilenames;  // for DB plugins

    vector<QString> cxxflags;
    vector<QString> ldflags;
    vector<QString> libs;
    vector<QString> extensions; // for DB plugins
    vector<QString> filenames;  // for DB plugins
    bool customgfiles;
    vector<QString> gfiles;     // gui
    bool customsfiles;
    vector<QString> sfiles;     // scripting
    bool customvfiles;
    vector<QString> vfiles;     // viewer
    bool custommfiles;
    vector<QString> mfiles;     // mdserver
    bool customefiles;
    vector<QString> efiles;     // engine
    bool customwfiles;
    vector<QString> wfiles;     // widgets
    bool customvwfiles;
    vector<QString> vwfiles;    // viewer widgets

    AttsGeneratorAttribute *atts;
  public:
    AttsGeneratorPlugin(const QString &n,const QString &l,const QString &t,
                        const QString &vt,const QString &dt,const QString &v,
                        const QString &, bool,bool,bool,bool)
        : name(n), type(t), label(l), version(v), vartype(vt), dbtype(dt), 
          atts(NULL)
    {
        enabledByDefault = true;
        has_MDS_specific_code = false;
        hasEngineSpecificCode = false;
    }
    void Print(ostream &out)
    {
        out << "Plugin: "<<name<<" (\""<<label<<"\", type="<<type<<") -- version "<<version<< endl;
        if (atts)
            atts->Print(cout);
    }
};


// ----------------------------------------------------------------------------
//                           Override default types
// ----------------------------------------------------------------------------
#define FieldFactory AttsFieldFactory
#define Field        AttsGeneratorField
#define Attribute    AttsGeneratorAttribute
#define Enum         AttsGeneratorEnum
#define Plugin       AttsGeneratorPlugin

#endif
