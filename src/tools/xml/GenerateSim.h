/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#ifndef GENERATE_SIM_H
#define GENERATE_SIM_H
#include <QTextStream>

#include <vector>
#include <map>
#include <algorithm>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include "Field.h"

#define GENERATOR_NAME "xml2sim"

QString
pad(const QString &s, int len) 
{
    QString p(s);
    while(p.length() < len)
        p += " ";
    return p;
}

// ****************************************************************************
//  File:  GenerateSim
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
//  Creation:    Thu Mar  4 14:18:29 PST 2010
//
//  Modifications:
//    Brad Whitlock, Tue Oct 25 09:36:22 PDT 2011
//    I changed the format of the VISIT_DYNAMIC_EXECUTE macro and fixed up
//    some function mismatches.
//
// ****************************************************************************

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class AttsGeneratorField : public virtual Field
{
  protected:
    QString generatorName;
  public:
    AttsGeneratorField(const QString &t, const QString &n, const QString &l)
        : Field(t,n,l), generatorName(GENERATOR_NAME)
    {
    }
    QString GetCPPNameW(int w, bool subtypename=false, const QString &classname="")
    {
        QString s = GetCPPName(subtypename,classname);
        for (int i=w - s.length(); i>0; i--)
            s += " ";
        return s;
    }

    virtual QString CArgument() const { return type; }

    virtual void WriteVisItFunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "int VisIt_" << classname << "_set" << Name << "(visit_handle h, "
          << CArgument() << ");" << endl;
        h << "int VisIt_" << classname << "_get" << Name << "(visit_handle h, "
          << CArgument() << "*);" << endl;
    }

    virtual void WriteVisItFunction(QTextStream &h, const QString &classname)
    {
        h << "int\nVisIt_" << classname << "_set" << Name
          << "(visit_handle h, " << CArgument() << " val)" << endl;
        h << "{" << endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_set" << Name << "," << endl;
        h << "        int, (visit_handle, " << CArgument() << ")," << endl;
        h << "        (h, val));" << endl;
        h << "}" << endl;
        h << endl;

        h << "int\nVisIt_" << classname << "_get" << Name
          << "(visit_handle h, " << CArgument() << " *val)" << endl;
        h << "{" << endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_get" << Name << "," << endl;
        h << "        int, (visit_handle, " << CArgument() << "*)," << endl;
        h << "        (h, val));" << endl;
        h << "}" << endl;
        h << endl;
    }

    // Some replacements to make field names shorter for Fortran
    virtual QString FortranFieldName(const QString &fname) const
    {
        QString n(fname.toUpper());
        if(n.endsWith("DIMENSION"))
            n.replace("DIMENSION", "DIM");
        return n;
    }

    virtual QString FortranName(const QString &classname, const QString &action) const
    {
        QString mName(QString("VISIT") + classname + action + FortranFieldName(Name));
        return mName;
    }

    virtual void WriteVisItFortranMacro(QTextStream &h, const QString &classname)
    {
        QString mName(FortranName(classname, "SET"));
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_," 
          << mName.toLower() << ", " << mName << ")" << endl;

        mName = QString(FortranName(classname, "GET"));
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_," 
          << mName.toLower() << ", " << mName << ")" << endl;
    }

    virtual void WriteVisItFortranFunction(QTextStream &h, const QString &classname, const QString &fclass)
    {
        QString mName(FortranName(fclass, "SET"));

        h << "int" << endl;
        h << "F_" << mName << "(visit_handle *h, " << CArgument() << " *val)" << endl;
        h << "{" << endl;
        h << "    return VisIt_" << classname << "_set" << Name << "(*h, *val);" << endl;
        h << "}" << endl;
        h << endl;

        mName = QString(FortranName(fclass, "GET"));
        h << "int" << endl;
        h << "F_" << mName << "(visit_handle *h, " << CArgument() << " *val)" << endl;
        h << "{" << endl;
        h << "    return VisIt_" << classname << "_get" << Name << "(*h, val);" << endl;
        h << "}" << endl;
        h << endl;
    }

    virtual void WriteSimV2FunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "SIMV2_API int simv2_" << classname << "_set" << Name
          << "(visit_handle h, " << CArgument() << ");" << endl;
        h << "SIMV2_API int simv2_" << classname << "_get" << Name
          << "(visit_handle h, " << CArgument() << "*);" << endl;
    }

    virtual void WriteSimV2ClassField(QTextStream &h, int maxLen)
    {
        h << "    " << pad(GetCPPName(), maxLen) << " " << name << ";" << endl;
    }
  
    virtual void WriteSimV2Ctor(QTextStream &h)
    {
    }

    virtual void WriteSimV2Function_set_helper(QTextStream &h)
    {
        h << "        obj->" << name << " = val;" << endl;
    }

    virtual void WriteSimV2Function_get_helper(QTextStream &h)
    {
        h << "        *val = obj->" << name << ";" << endl;
    }

    virtual void WriteSimV2Function(QTextStream &h, const QString &classname)
    {
        QString mName(QString("simv2_") + classname + "_set" + Name);

        h << "int" << endl;
        h << mName
          << "(visit_handle h, " << CArgument() << " val)" << endl;
        h << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << endl;
        h << "    if(obj != NULL)" << endl;
        h << "    {" << endl;
        WriteSimV2Function_set_helper(h);
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;

        mName = QString("simv2_") + classname + "_get" + Name;
        h << "int" << endl;
        h << mName
          << "(visit_handle h, " << CArgument() << " *val)" << endl;
        h << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    if(val == NULL)" << endl;
        h << "    {" << endl;
        h << "        VisItError(\"" << mName << ": Invalid address.\");" << endl;
        h << "        return VISIT_ERROR;" << endl;
        h << "    }" << endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << endl;
        h << "    if(obj != NULL)" << endl;
        h << "    {" << endl;
        WriteSimV2Function_get_helper(h);
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        h << "    else" << endl;
        h << "        *val = 0;" << endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;
    }

protected:

    QString sing() const
    {
        QString n(Name);
        if(n.right(1) == "s")
            n = n.left(n.size()-1);
        return n;
    }
};

//
// ------------------------------------ Int -----------------------------------
//
class AttsGeneratorInt : public virtual Int , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorInt(const QString &n, const QString &l)
        : Field("int",n,l), Int(n,l), AttsGeneratorField("int",n,l) { }

    virtual void WriteSimV2Ctor(QTextStream &h)
    {
        if(valueSet)
            h << "    " << name << " = " << val << ";" << endl;
        else
            h << "    " << name << " = 0;" << endl;
    }
};

//
// -------------------------------- IntArray --------------------------------
//
class AttsGeneratorIntArray : public virtual IntArray , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorIntArray(const QString &s, const QString &n, const QString &l)
        : Field("intArray",n,l), IntArray(s,n,l), AttsGeneratorField("intArray",n,l) { }
};


//
// -------------------------------- IntVector --------------------------------
//
class AttsGeneratorIntVector : public virtual IntVector , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorIntVector(const QString &n, const QString &l)
        : Field("intVector",n,l), IntVector(n,l), AttsGeneratorField("intVector",n,l) { }

    virtual void WriteVisItFunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "int VisIt_" << classname << "_add" << sing()
          << "(visit_handle h, int);" << endl;
        h << "int VisIt_" << classname << "_getNum" << sing()
          << "(visit_handle h, int *);" << endl;
        h << "int VisIt_" << classname << "_get" << sing()
          << "(visit_handle h, int, int *);" << endl;
    }

    virtual void WriteVisItFunction(QTextStream &h, const QString &classname)
    {
        h << "int\nVisIt_" << classname << "_add" << sing()
          << "(visit_handle h, int val)" << endl;
        h << "{" << endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_add" << sing() << "," << endl;
        h << "        int, (visit_handle, int)," << endl;
        h << "        (h, val));" << endl;
        h << "}" << endl;
        h << endl;
        h << "int\nVisIt_" << classname << "_getNum" << sing()
          << "(visit_handle h, int *n)" << endl;
        h << "{" << endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_getNum" << sing() << "," << endl;
        h << "        int, (visit_handle, int*)," << endl;
        h << "        (h, n));" << endl;
        h << "}" << endl;
        h << endl;
        h << "int\nVisIt_" << classname << "_get" << sing()
          << "(visit_handle h, int i, int *val)" << endl;
        h << "{" << endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_get" << sing() << "," << endl;
        h << "        int, (visit_handle, int, int*)," << endl;
        h << "        (h, i, val));" << endl;
        h << "}" << endl;
        h << endl;
    }

    virtual QString FortranName(const QString &classname, const QString &action) const
    {
        QString mName(QString("VISIT") + classname + action + FortranFieldName(Name));
        return mName;
    }

    virtual void WriteVisItFortranFunction(QTextStream &h, const QString &classname, const QString &fclass)
    {
        QString mName(FortranName(fclass, "SET"));
        h << "int" << endl;
        h << "F_" << mName << "(visit_handle *h, int *val)" << endl;
        h << "{" << endl;
        h << "    return VisIt_" << classname << "_add" << sing() << "(*h, *val);" << endl;
        h << "}" << endl;
        h << endl;

        mName = FortranName(fclass, "GETNUM");
        h << "int" << endl;
        h << "F_" << mName << "(visit_handle *h, int *n)" << endl;
        h << "{" << endl;
        h << "    return VisIt_" << classname << "_getNum" << sing() << "(*h, n);" << endl;
        h << "}" << endl;
        h << endl;

        mName = FortranName(fclass, "GET");
        h << "int" << endl;
        h << "F_" << mName << "(visit_handle *h, int *i, int *val)" << endl;
        h << "{" << endl;
        h << "    return VisIt_" << classname << "_get" << sing() << "(*h, *i, val);" << endl;
        h << "}" << endl;
        h << endl;

    }

    virtual void WriteSimV2FunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "SIMV2_API int simv2_" << classname << "_add" << sing()
          << "(visit_handle h, int);" << endl;
        h << "SIMV2_API int simv2_" << classname << "_getNum" << sing()
          << "(visit_handle h, int *);" << endl;
        h << "SIMV2_API int simv2_" << classname << "_get" << sing()
          << "(visit_handle h, int, int *);" << endl;
    }

    virtual void WriteSimV2Function(QTextStream &h, const QString &classname)
    {
        QString mName(QString("simv2_") + classname + "_add" + sing());

        h << "int" << endl;
        h << mName << "(visit_handle h, int val)" << endl;
        h << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << endl;
        h << "    if(obj != NULL)" << endl;
        h << "    {" << endl;
        h << "        obj->" << name << ".push_back(val);" << endl;
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;

        mName = (QString("simv2_") + classname + "_getNum" + sing());
        h << "int" << endl;
        h << mName << "(visit_handle h, int *n)" << endl;
        h << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << endl;
        h << "    if(obj != NULL && n != NULL)" << endl;
        h << "    {" << endl;
        h << "        *n = obj->" << name << ".size();" << endl;
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        h << "    else" << endl;
        h << "        *n = 0;" << endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;

        mName = (QString("simv2_") + classname + "_get" + sing());
        h << "int" << endl;
        h << mName << "(visit_handle h, int i, int *val)" << endl;
        h << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << endl;
        h << "    if(obj != NULL && i >= 0 && i < obj->" << name << ".size())" << endl;
        h << "    {" << endl;
        h << "        *val = obj->" << name << "[i];" << endl;
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        h << "    else" << endl;
        h << "        *val = 0;" << endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;
    }
};


// 
// ----------------------------------- Bool -----------------------------------
//
class AttsGeneratorBool : public virtual Bool , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorBool(const QString &n, const QString &l)
        : Field("bool",n,l), Bool(n,l), AttsGeneratorField("bool",n,l) { }
    virtual QString CArgument() const { return "int"; }

    virtual void WriteSimV2Ctor(QTextStream &h)
    {
        if(valueSet)
            h << "    " << name << " = " << (val ? "true":"false") << ";" << endl;
        else
            h << "    " << name << " = false;" << endl;
    }

    virtual void WriteSimV2Function_set_helper(QTextStream &h)
    {
        h << "        obj->" << name << " = (val > 0);" << endl;
    }

    virtual void WriteSimV2Function_get_helper(QTextStream &h)
    {
        h << "        *val = obj->" << name << " ? 1 : 0;" << endl;
    }
};


//
// ----------------------------------- Float ----------------------------------
//
class AttsGeneratorFloat : public virtual Float , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorFloat(const QString &n, const QString &l)
        : Field("float",n,l), Float(n,l), AttsGeneratorField("float",n,l) { }
    virtual void WriteSimV2Ctor(QTextStream &h)
    {
        if(valueSet)
            h << "    " << name << " = " << val << ";" << endl;
        else
            h << "    " << name << " = 0.f;" << endl;
    }
};


//
// -------------------------------- FloatArray -------------------------------
//
class AttsGeneratorFloatArray : public virtual FloatArray , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorFloatArray(const QString &s, const QString &n, const QString &l)
        : Field("floatArray",n,l), FloatArray(s,n,l), AttsGeneratorField("floatArray",n,l) { }
};


//
// ---------------------------------- Double ----------------------------------
//
class AttsGeneratorDouble : public virtual Double , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorDouble(const QString &n, const QString &l)
        : Field("double",n,l), Double(n,l), AttsGeneratorField("double",n,l) { }
    virtual void WriteSimV2Ctor(QTextStream &h)
    {
        if(valueSet)
            h << "    " << name << " = " << val << ";" << endl;
        else
            h << "    " << name << " = 0.;" << endl;
    }
};


//
// -------------------------------- DoubleArray -------------------------------
//
class AttsGeneratorDoubleArray : public virtual DoubleArray , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorDoubleArray(const QString &s, const QString &n, const QString &l)
        : Field("doubleArray",n,l), DoubleArray(s,n,l), AttsGeneratorField("doubleArray",n,l) { }
    virtual QString CArgument() const { return QString("double[%1]").arg(length); }
};


//
// ------------------------------- DoubleVector -------------------------------
//
class AttsGeneratorDoubleVector : public virtual DoubleVector , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorDoubleVector(const QString &n, const QString &l)
        : Field("doubleVector",n,l), DoubleVector(n,l), AttsGeneratorField("doubleVector",n,l) { }
};


//
// ----------------------------------- UChar ----------------------------------
//
class AttsGeneratorUChar : public virtual UChar , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorUChar(const QString &n, const QString &l)
        : Field("uchar",n,l), UChar(n,l), AttsGeneratorField("uchar",n,l) { }
};


//
// -------------------------------- UCharArray --------------------------------
//
class AttsGeneratorUCharArray : public virtual UCharArray , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorUCharArray(const QString &s, const QString &n, const QString &l)
        : Field("ucharArray",n,l), UCharArray(s,n,l), AttsGeneratorField("ucharArray",n,l) { }
};


//
// ------------------------------- unsignedCharVector -------------------------------
//
class AttsGeneratorUCharVector : public virtual UCharVector , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorUCharVector(const QString &n, const QString &l)
        : Field("ucharVector",n,l), UCharVector(n,l), AttsGeneratorField("ucharVector",n,l) { }
};


//
// ---------------------------------- String ----------------------------------
//
class AttsGeneratorString : public virtual String , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorString(const QString &n, const QString &l)
        : Field("string",n,l), String(n,l), AttsGeneratorField("string",n,l) { }
    virtual QString CArgument() const { return "const char *"; }

    virtual void WriteVisItFunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "int VisIt_" << classname << "_set" << Name << "(visit_handle h, const char *);" << endl;
        h << "int VisIt_" << classname << "_get" << Name << "(visit_handle h, char **);" << endl;
    }

    virtual void WriteVisItFunction(QTextStream &h, const QString &classname)
    {
        h << "int\nVisIt_" << classname << "_set" << Name
          << "(visit_handle h, const char *val)" << endl;
        h << "{" << endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_set" << Name << "," << endl;
        h << "        int, (visit_handle, const char *)," << endl;
        h << "        (h, val));" << endl;
        h << "}" << endl;
        h << endl;

        h << "int\nVisIt_" << classname << "_get" << Name
          << "(visit_handle h, char **val)" << endl;
        h << "{" << endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_get" << Name << "," << endl;
        h << "        int, (visit_handle, char **)," << endl;
        h << "        (h, val));" << endl;
        h << "}" << endl;
        h << endl;
    }

    virtual void WriteVisItFortranFunction(QTextStream &h, const QString &classname, const QString &fclass)
    {
        QString mName(FortranName(fclass, "SET"));

        h << "int" << endl;
        h << "F_" << mName << "(visit_handle *h, const char *val, int *lval)" << endl;
        h << "{" << endl;
        h << "    char *f_val = NULL;" << endl;
        h << "    int retval;" << endl;
        h << "    COPY_FORTRAN_STRING(f_val, val, lval);" << endl;
        h << "    retval = VisIt_" << classname << "_set" << Name << "(*h, f_val);" << endl;
        h << "    FREE(f_val);" << endl;
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;

        mName = FortranName(fclass, "GET");
        h << "int" << endl;
        h << "F_" << mName << "(visit_handle *h, char *val, int *lval)" << endl;
        h << "{" << endl;
        h << "    char *s = NULL;" << endl;
        h << "    int retval = VisIt_" << classname << "_get" << Name << "(*h, &s);" << endl;
        h << "    if(s != NULL)" << endl;
        h << "    {" << endl;
        h << "        visit_cstring_to_fstring(s, val, *lval);" << endl;
        h << "        free(s);" << endl;
        h << "    }" << endl;
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;
    }

    virtual void WriteSimV2FunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "SIMV2_API int simv2_" << classname << "_set" << Name << "(visit_handle h, const char *);" << endl;
        h << "SIMV2_API int simv2_" << classname << "_get" << Name << "(visit_handle h, char **);" << endl;
    }

    virtual void WriteSimV2Ctor(QTextStream &h)
    {
        if(valueSet)
            h << "    " << name << " = \"" << val << "\";" << endl;
        else
            h << "    " << name << " = \"\";" << endl;
    }

    virtual void WriteSimV2Function(QTextStream &h, const QString &classname)
    {
        QString mName(QString("simv2_") + classname + "_set" + Name);

        h << "int" << endl;
        h << mName << "(visit_handle h, const char *val)" << endl;
        h << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == NULL)" << endl;
        h << "    {" << endl;
        h << "        VisItError(\"An invalid string was provided for " << name << "\");" << endl;
        h << "        return VISIT_ERROR;" << endl;
        h << "    }" << endl;
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << endl;
        h << "    if(obj != NULL)" << endl;
        h << "    {" << endl;
        h << "        obj->" << name << " = val;" << endl;
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;

        mName = (QString("simv2_") + classname + "_get" + Name);
        h << "int" << endl;
        h << mName << "(visit_handle h, char **val)" << endl;
        h << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == NULL)" << endl;
        h << "    {" << endl;
        h << "        VisItError(\"" << mName << ": Invalid address.\");" << endl;
        h << "        return VISIT_ERROR;" << endl;
        h << "    }" << endl;
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << endl;
        h << "    if(obj != NULL)" << endl;
        h << "    {" << endl;
        h << "        *val = (char*)malloc(obj->" << name << ".size() + 1);" << endl;
        h << "        strcpy(*val, obj->" << name << ".c_str());" << endl;
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        h << "    else" << endl;
        h << "        *val = NULL;" << endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;
    }
};


//
// ------------------------------- StringVector -------------------------------
//
class AttsGeneratorStringVector : public virtual StringVector , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorStringVector(const QString &n, const QString &l)
        : Field("stringVector",n,l), StringVector(n,l), AttsGeneratorField("stringVector",n,l) { }

    virtual void WriteVisItFunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "int VisIt_" << classname << "_add" << sing() << "(visit_handle h, const char *);" << endl;
        h << "int VisIt_" << classname << "_getNum" << sing() << "(visit_handle h, int *);" << endl;
        h << "int VisIt_" << classname << "_get" << sing() << "(visit_handle h, int, char **);" << endl;
    }

    virtual void WriteVisItFunction(QTextStream &h, const QString &classname)
    {
        h << "int\nVisIt_" << classname << "_add" << sing()
          << "(visit_handle h, const char *val)" << endl;
        h << "{" << endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_add" << sing() << "," << endl;
        h << "        int, (visit_handle, const char *)," << endl;
        h << "        (h, val));" << endl;
        h << "}" << endl;
        h << endl;

        h << "int\nVisIt_" << classname << "_getNum" << sing()
          << "(visit_handle h, int *val)" << endl;
        h << "{" << endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_getNum" << sing() << "," << endl;
        h << "        int, (visit_handle, int *)," << endl;
        h << "        (h, val));" << endl;
        h << "}" << endl;
        h << endl;

        h << "int\nVisIt_" << classname << "_get" << sing()
          << "(visit_handle h, int i, char **val)" << endl;
        h << "{" << endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_get" << sing() << "," << endl;
        h << "        int, (visit_handle, int, char **)," << endl;
        h << "        (h, i, val));" << endl;
        h << "}" << endl;
        h << endl;
    }

    virtual QString FortranName(const QString &classname, const QString &action) const
    {
        QString mName(QString("VISIT") + classname + action + FortranFieldName(sing()));
        return mName;
    }

    virtual void WriteVisItFortranMacro(QTextStream &h, const QString &classname)
    {
        QString mName(FortranName(classname, "ADD"));
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_," 
          << mName.toLower() << ", " << mName << ")" << endl;

        mName = QString(FortranName(classname, "GETNUM"));
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_," 
          << mName.toLower() << ", " << mName << ")" << endl;

        mName = QString(FortranName(classname, "GET"));
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_," 
          << mName.toLower() << ", " << mName << ")" << endl;
    }

    virtual void WriteVisItFortranFunction(QTextStream &h, const QString &classname, const QString &fclass)
    {
        QString mName(FortranName(fclass, "ADD"));

        h << "int" << endl;
        h << "F_" << mName << "(visit_handle *h, const char *val, int *lval)" << endl;
        h << "{" << endl;
        h << "    char *f_val = NULL;" << endl;
        h << "    int retval;" << endl;
        h << "    COPY_FORTRAN_STRING(f_val, val, lval);" << endl;
        h << "    retval = VisIt_" << classname << "_add" << sing() << "(*h, f_val);" << endl;
        h << "    FREE(f_val);" << endl;
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;

        mName = FortranName(fclass, "GETNUM");
        h << "int" << endl;
        h << "F_" << mName << "(visit_handle *h, int *val)" << endl;
        h << "{" << endl;
        h << "    return VisIt_" << classname << "_getNum" << sing() << "(*h, val);" << endl;
        h << "}" << endl;
        h << endl;

        mName = FortranName(fclass, "GET");
        h << "int" << endl;
        h << "F_" << mName << "(visit_handle *h, int *i, char *val, int *lval)" << endl;
        h << "{" << endl;
        h << "    char *s = NULL;" << endl;
        h << "    int retval;" << endl;
        h << "    retval = VisIt_" << classname << "_get" << sing() << "(*h, *i, &s);" << endl;
        h << "    if(s != NULL)" << endl;
        h << "    {" << endl;
        h << "        visit_cstring_to_fstring(s, val, *lval);" << endl;
        h << "        free(s);" << endl;
        h << "    }" << endl;
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;
    }

    virtual void WriteSimV2FunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "SIMV2_API int simv2_" << classname << "_add" << sing()
          << "(visit_handle h, const char *);" << endl;

        h << "SIMV2_API int simv2_" << classname << "_getNum" << sing()
          << "(visit_handle h, int *);" << endl;

        h << "SIMV2_API int simv2_" << classname << "_get" << sing()
          << "(visit_handle h, int, char **);" << endl;
    }

    virtual void WriteSimV2Function(QTextStream &h, const QString &classname)
    {
        QString mName(QString("simv2_") + classname + "_add" + sing());

        h << "int" << endl;
        h << mName << "(visit_handle h, const char *val)" << endl;
        h << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == NULL)" << endl;
        h << "    {" << endl;
        h << "        VisItError(\"An invalid string was provided for " << name << "\");" << endl;
        h << "        return VISIT_ERROR;" << endl;
        h << "    }" << endl;
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << endl;
        h << "    if(obj != NULL)" << endl;
        h << "    {" << endl;
        h << "        obj->" << name << ".push_back(val);" << endl;
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;

        mName = (QString("simv2_") + classname + "_getNum" + sing());
        h << "int" << endl;
        h << mName << "(visit_handle h, int *val)" << endl;
        h << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == NULL)" << endl;
        h << "    {" << endl;
        h << "        VisItError(\"" << mName << ": Invalid address\");" << endl;
        h << "        return VISIT_ERROR;" << endl;
        h << "    }" << endl;
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << endl;
        h << "    if(obj != NULL)" << endl;
        h << "    {" << endl;
        h << "        *val = obj->" << name << ".size();" << endl;
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        h << "    else" << endl;
        h << "        *val = 0;" << endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;

        mName = (QString("simv2_") + classname + "_get" + sing());
        h << "int" << endl;
        h << mName << "(visit_handle h, int i, char **val)" << endl;
        h << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == NULL)" << endl;
        h << "    {" << endl;
        h << "        VisItError(\"" << mName << ": Invalid address\");" << endl;
        h << "        return VISIT_ERROR;" << endl;
        h << "    }" << endl;
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << endl;
        h << "    if(obj != NULL && i >= 0 && i < obj->" << name << ".size())" << endl;
        h << "    {" << endl;
        h << "        *val = (char *)malloc(obj->" << name << "[i].size() + 1);" << endl;
        h << "        strcpy(*val, obj->" << name << "[i].c_str());" << endl;
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        h << "    else" << endl;
        h << "        *val = NULL;" << endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;
    }

};


//
// -------------------------------- ColorTable --------------------------------
//
class AttsGeneratorColorTable : public virtual ColorTable , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorColorTable(const QString &n, const QString &l)
        : Field("colortable",n,l), ColorTable(n,l), AttsGeneratorField("colortable",n,l) { }
};


//
// ----------------------------------- Color ----------------------------------
//
class AttsGeneratorColor : public virtual Color , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorColor(const QString &n, const QString &l)
        : Field("color",n,l), Color(n,l), AttsGeneratorField("color",n,l) { }
};


//
// --------------------------------- LineStyle --------------------------------
//
class AttsGeneratorLineStyle : public virtual LineStyle , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorLineStyle(const QString &n, const QString &l)
        : Field("linestyle",n,l), LineStyle(n,l), AttsGeneratorField("linestyle",n,l) { }
};


//
// --------------------------------- LineWidth --------------------------------
//
class AttsGeneratorLineWidth : public virtual LineWidth , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorLineWidth(const QString &n, const QString &l)
        : Field("linewidth",n,l), LineWidth(n,l), AttsGeneratorField("linewidth",n,l) { }
};


//
// --------------------------------- Opacity ----------------------------------
//
class AttsGeneratorOpacity : public virtual Opacity , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorOpacity(const QString &n, const QString &l)
        : Field("opacity",n,l), Opacity(n,l), AttsGeneratorField("opacity",n,l) { }
};


//
// -------------------------------- VariableName --------------------------------
//
class AttsGeneratorVariableName : public virtual VariableName , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorVariableName(const QString &n, const QString &l)
        : Field("variablename",n,l), VariableName(n,l),
          AttsGeneratorField("variablename",n,l)
           { }
};


//
// ------------------------------------ Att -----------------------------------
//
class AttsGeneratorAtt : public virtual Att , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorAtt(const QString &t, const QString &n, const QString &l)
        : Field("att",n,l), Att(t,n,l), AttsGeneratorField("att",n,l) { }
};


//
// --------------------------------- AttVector --------------------------------
//
class AttsGeneratorAttVector : public virtual AttVector , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorAttVector(const QString &t, const QString &n, const QString &l)
        : Field("attVector",n,l), AttVector(t,n,l), AttsGeneratorField("attVector",n,l) { }
    virtual QString CArgument() const { return "visit_handle"; }
    virtual void WriteVisItFunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "int VisIt_" << classname << "_add" << sing() << "(visit_handle h, visit_handle);" << endl;
        h << "int VisIt_" << classname << "_getNum" << sing() << "(visit_handle h, visit_handle *);" << endl;
        h << "int VisIt_" << classname << "_get" << sing() << "(visit_handle h, int, visit_handle*);" << endl;
    }

    virtual void WriteVisItFunction(QTextStream &h, const QString &classname)
    {
        h << "int\nVisIt_" << classname << "_add" << sing()
          << "(visit_handle h, visit_handle val)" << endl;
        h << "{" << endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_add" << sing() << "," << endl;
        h << "        int, (visit_handle, visit_handle)," << endl;
        h << "        (h, val));" << endl;
        h << "}" << endl;
        h << endl;

        h << "int\nVisIt_" << classname << "_getNum" << sing()
          << "(visit_handle h, int *val)" << endl;
        h << "{" << endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_getNum" << sing() << "," << endl;
        h << "        int, (visit_handle, int *)," << endl;
        h << "        (h, val));" << endl;
        h << "}" << endl;
        h << endl;

        h << "int\nVisIt_" << classname << "_get" << sing()
          << "(visit_handle h, int i, visit_handle *val)" << endl;
        h << "{" << endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_get" << sing() << "," << endl;
        h << "        int, (visit_handle, int, visit_handle *)," << endl;
        h << "        (h, i, val));" << endl;
        h << "}" << endl;
        h << endl;
    }

    virtual QString FortranName(const QString &classname, const QString &action) const
    {
        QString mName(QString("VISIT") + classname + action + FortranFieldName(sing()));
        return mName;
    }

    virtual void WriteVisItFortranMacro(QTextStream &h, const QString &classname)
    {
        QString mName(FortranName(classname, "ADD"));
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_," 
          << mName.toLower() << ", " << mName << ")" << endl;

        mName = QString(FortranName(classname, "GETNUM"));
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_," 
          << mName.toLower() << ", " << mName << ")" << endl;

        mName = QString(FortranName(classname, "GET"));
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_," 
          << mName.toLower() << ", " << mName << ")" << endl;
    }

    virtual void WriteVisItFortranFunction(QTextStream &h, const QString &classname, const QString &fclass)
    {
        QString mName(FortranName(fclass, "ADD"));

        h << "int" << endl;
        h << "F_" << mName << "(visit_handle *h, visit_handle *val)" << endl;
        h << "{" << endl;
        h << "    return VisIt_" << classname << "_add" << sing() << "(*h, *val);" << endl;
        h << "}" << endl;
        h << endl;

        mName = FortranName(fclass, "GETNUM");
        h << "int" << endl;
        h << "F_" << mName << "(visit_handle *h, int *val)" << endl;
        h << "{" << endl;
        h << "    return VisIt_" << classname << "_getNum" << sing() << "(*h, val);" << endl;
        h << "}" << endl;
        h << endl;

        mName = FortranName(fclass, "GET");
        h << "int" << endl;
        h << "F_" << mName << "(visit_handle *h, int *i, visit_handle *val)" << endl;
        h << "{" << endl;
        h << "    return VisIt_" << classname << "_get" << sing() << "(*h, *i, val);" << endl;
        h << "}" << endl;
        h << endl;
    }

    virtual void WriteSimV2FunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "SIMV2_API int simv2_" << classname << "_add" << sing()
          << "(visit_handle h, visit_handle);" << endl;

        h << "SIMV2_API int simv2_" << classname << "_getNum" << sing()
          << "(visit_handle h, int *);" << endl;

        h << "SIMV2_API int simv2_" << classname << "_get" << sing()
          << "(visit_handle h, int, visit_handle *);" << endl;
    }

    virtual void WriteSimV2ClassField(QTextStream &h, int maxLen)
    {
        h << "    " << pad("std::vector<visit_handle>", maxLen) << " " << name << ";" << endl;
    }

    virtual void WriteSimV2Function(QTextStream &h, const QString &classname)
    {
        QString mName(QString("simv2_") + classname + "_add" + sing());

        h << "int" << endl;
        h << mName << "(visit_handle h, visit_handle val)" << endl;
        h << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == VISIT_INVALID_HANDLE)" << endl;
        h << "    {" << endl;
        h << "        VisItError(\"An invalid handle was provided for " << name << "\");" << endl;
        h << "        return VISIT_ERROR;" << endl;
        h << "    }" << endl;
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << endl;
        h << "    if(obj != NULL)" << endl;
        h << "    {" << endl;
        h << "        obj->" << name << ".push_back(val);" << endl;
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;

        mName = (QString("simv2_") + classname + "_getNum" + sing());
        h << "int" << endl;
        h << mName << "(visit_handle h, int *val)" << endl;
        h << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == NULL)" << endl;
        h << "    {" << endl;
        h << "        VisItError(\"" << mName << ": Invalid address\");" << endl;
        h << "        return VISIT_ERROR;" << endl;
        h << "    }" << endl;
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << endl;
        h << "    if(obj != NULL)" << endl;
        h << "    {" << endl;
        h << "        *val = obj->" << name << ".size();" << endl;
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        h << "    else" << endl;
        h << "        *val = 0;" << endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;

        mName = (QString("simv2_") + classname + "_get" + sing());
        h << "int" << endl;
        h << mName << "(visit_handle h, int i, visit_handle *val)" << endl;
        h << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == NULL)" << endl;
        h << "    {" << endl;
        h << "        VisItError(\"" << mName << ": Invalid address\");" << endl;
        h << "        return VISIT_ERROR;" << endl;
        h << "    }" << endl;
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << endl;
        h << "    if(obj != NULL && i >= 0 && i < obj->" << name << ".size())" << endl;
        h << "    {" << endl;
        h << "        *val = obj->" << name << "[i];" << endl;
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        h << "    else" << endl;
        h << "        *val = NULL;" << endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;
    }
};


//
// ----------------------------------- Enum -----------------------------------
//
class AttsGeneratorEnum : public virtual Enum , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorEnum(const QString &t, const QString &n, const QString &l)
        : Field("enum",n,l), Enum(t,n,l), AttsGeneratorField("enum",n,l) { }
    virtual QString CArgument() const { return "int"; }

    virtual void WriteSimV2Ctor(QTextStream &h)
    {
        if(valueSet)
             h << "    " << name << " = " <<enumType->values[val] << ";" << endl;
        else
             h << "    " << name << " = " <<enumType->values[0] << ";" << endl;
    }
    virtual void WriteSimV2Function(QTextStream &h, const QString &classname)
    {
        QString mName(QString("simv2_") + classname + "_set" + Name);

        h << "int" << endl;
        h << mName << "(visit_handle h, " << CArgument() << " val)" << endl;
        h << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(";
        for(size_t i = 0; i < enumType->values.size(); ++i)
        {
            if(i > 0)
                h << "       ";
            h << "val != " << enumType->values[i];
            if(i < enumType->values.size()-1)
                h << " &&" << endl;
        }
        h << ")" << endl;
        h << "    {" << endl;
        h << "        VisItError(\"The value for " << name << " must be one of: ";
        for(size_t i = 0; i < enumType->values.size(); ++i)
        {
            h << enumType->values[i];
            if(i < enumType->values.size()-1)
                h << ", ";
        }
        h << "\");" << endl;
        h << "        return VISIT_ERROR;" << endl;
        h << "    }" << endl;
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << endl;
        h << "    if(obj != NULL)" << endl;
        h << "    {" << endl;
        h << "        obj->" << name << " = val;" << endl;
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;

        mName = QString("simv2_") + classname + "_get" + Name;
        h << "int" << endl;
        h << mName
          << "(visit_handle h, " << CArgument() << " *val)" << endl;
        h << "{" << endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    if(val == NULL)" << endl;
        h << "    {" << endl;
        h << "        VisItError(\"" << mName << ": Invalid address.\");" << endl;
        h << "        return VISIT_ERROR;" << endl;
        h << "    }" << endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << endl;
        h << "    if(obj != NULL)" << endl;
        h << "    {" << endl;
        WriteSimV2Function_get_helper(h);
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        h << "    else" << endl;
        h << "        *val = 0;" << endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;
    }

};


//
// --------------------------------- ScaleMode --------------------------------
//
class AttsGeneratorScaleMode : public virtual ScaleMode , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorScaleMode(const QString &n, const QString &l)
        : Field("scalemode",n,l), ScaleMode(n,l), AttsGeneratorField("scalemode",n,l) { }
    virtual QString GetAttributeGroupID()
    {
        return "i";
    }
    virtual QString DataNodeConversion()
    {
        return "AsInt";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        c << "    " << name << " = " << val << ";" << Endl;
    }
};


#define AVT_GENERATOR_METHODS \
    virtual QString CArgument() const { return "int"; }

//
// ----------------------------------- avtCentering -----------------------------------
//
class AttsGeneratoravtCentering : public virtual avtCenteringField, public virtual AttsGeneratorField
{
  public:
    AttsGeneratoravtCentering(const QString &n, const QString &l)
        : Field("avtCentering",n,l), avtCenteringField(n,l), AttsGeneratorField("avtCentering",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtGhostType -----------------------------------
//
class AttsGeneratoravtGhostType : public virtual avtGhostTypeField, public virtual AttsGeneratorField
{
  public:
    AttsGeneratoravtGhostType(const QString &n, const QString &l)
        : Field("avtGhostType",n,l), avtGhostTypeField(n,l), AttsGeneratorField("avtGhostType",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtSubsetType -----------------------------------
//
class AttsGeneratoravtSubsetType : public virtual avtSubsetTypeField, public virtual AttsGeneratorField
{
  public:
    AttsGeneratoravtSubsetType(const QString &n, const QString &l)
        : Field("int",n,l), avtSubsetTypeField(n,l), AttsGeneratorField("int",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtVarType -----------------------------------
//
class AttsGeneratoravtVarType : public virtual avtVarTypeField, public virtual AttsGeneratorField
{
  public:
    AttsGeneratoravtVarType(const QString &n, const QString &l)
        : Field("avtVarTypeField",n,l), avtVarTypeField(n,l), AttsGeneratorField("avtVarType",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtMeshType -----------------------------------
//
class AttsGeneratoravtMeshType : public virtual avtMeshTypeField, public virtual AttsGeneratorField
{
  public:
    AttsGeneratoravtMeshType(const QString &n, const QString &l)
        : Field("avtMeshTypeField",n,l), avtMeshTypeField(n,l), AttsGeneratorField("avtMeshType",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtExtentType -----------------------------------
//
class AttsGeneratoravtExtentType : public virtual avtExtentTypeField, public virtual AttsGeneratorField
{
  public:
    AttsGeneratoravtExtentType(const QString &n, const QString &l)
        : Field("avtExtentType",n,l), avtExtentTypeField(n,l), AttsGeneratorField("avtExentType",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtMeshCoordType -----------------------------------
//
class AttsGeneratoravtMeshCoordType : public virtual avtMeshCoordTypeField, public virtual AttsGeneratorField
{
  public:
    AttsGeneratoravtMeshCoordType(const QString &n, const QString &l)
        : Field("avtMeshCoordType",n,l), avtMeshCoordTypeField(n,l), AttsGeneratorField("avtMeshCoordType",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- LoadBalanceScheme -----------------------------------
//
class AttsGeneratorLoadBalanceScheme : public virtual LoadBalanceSchemeField, public virtual AttsGeneratorField
{
  public:
    AttsGeneratorLoadBalanceScheme(const QString &n, const QString &l)
        : Field("LoadBalanceScheme",n,l), LoadBalanceSchemeField(n,l), AttsGeneratorField("LoadBalanceScheme",n,l)
    { }
    AVT_GENERATOR_METHODS
};

// ----------------------------------------------------------------------------
// Modifications:
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
        else if (type == "enum")         f = new AttsGeneratorEnum(subtype, name, label);
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
            throw QString("AttsFieldFactory: unknown type for field %1: %2").arg(name).arg(type);

        return f;
    }
};

// ----------------------------------------------------------------------------
// Modifications:
//
// ----------------------------------------------------------------------------
#include <GeneratorBase.h>

class AttsGeneratorAttribute : public GeneratorBase
{
  public:
    std::vector<AttsGeneratorField*> fields;
  public:
    AttsGeneratorAttribute(const QString &n, const QString &p, const QString &f,
                           const QString &e, const QString &ei, const QString &bc)
        : GeneratorBase(n,p,f,e,ei, GENERATOR_NAME, bc), fields()
    {
    }

    virtual ~AttsGeneratorAttribute()
    {
        for (size_t i = 0; i < fields.size(); ++i)
            delete fields[i];
        fields.clear();
    }

    virtual void Print(QTextStream &out)
    {
        out << "    Attribute: " << name << " (" << purpose << ")" << Endl;
        out << "        exportAPI=" << exportAPI << Endl;
        out << "        exportInclude=" << exportInclude << Endl;
        for (size_t i=0; i<fields.size(); i++)
            fields[i]->Print(out);
        for (size_t i=0; i<includes.size(); i++)
            includes[i]->Print(out, generatorName);
        for (size_t i=0; i<functions.size(); i++)
            functions[i]->Print(out, generatorName);
        for (size_t i=0; i<constants.size(); i++)
            constants[i]->Print(out, generatorName);
    }

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    void WriteVisItHeader(QTextStream &h)
    {
        h << copyright_str << endl;

        h << "#ifndef VISIT_" << name.toUpper() << "_H" << endl;
        h << "#define VISIT_" << name.toUpper() << "_H" << endl;
        h << endl;
        h << "#ifdef __cplusplus" << endl;
        h << "extern \"C\" {" << endl;
        h << "#endif" << endl;
        h << endl;
        h << "int VisIt_" << name << "_alloc(visit_handle *obj);" << endl;
        h << "int VisIt_" << name << "_free(visit_handle obj);" << endl;
        for (size_t i=0; i<fields.size(); i++)
            fields[i]->WriteVisItFunctionPrototype(h, name);
        h << endl;
        h << "#ifdef __cplusplus" << endl;
        h << "}" << endl;
        h << "#endif" << endl;
        h << endl;
        h << "#endif" << endl;
    }

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    void WriteVisItSource(QTextStream &h)
    {
        h << copyright_str << endl;

        h << "#include <VisItDataInterface_V2.h>" << endl;
        h << "#include \"VisItDynamic.h\"" << endl;
        h << "#include \"VisItFortran.h\"" << endl;
        h << endl;
        h << "int" << endl;
        h << "VisIt_" << name << "_alloc(visit_handle *obj)" << endl;
        h << "{" << endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << name << "_alloc," << endl;
        h << "                    int, (visit_handle*)," << endl;
        h << "                    (obj))" << endl;
        h << "}" << endl;
        h << endl;
        h << "int" << endl;
        h << "VisIt_" << name << "_free(visit_handle obj)" << endl;
        h << "{" << endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << name << "_free," << endl;
        h << "                    int, (visit_handle), " << endl;
        h << "                    (obj));" << endl;
        h << "}" << endl;
        h << endl;

        for (size_t i=0; i<fields.size(); i++)
            fields[i]->WriteVisItFunction(h, name);
        h << endl;

        h << "/************************** Fortran callable routines *************************/" << endl;
        h << "/* maxlen 012345678901234567890123456789                                      */" << endl;
        // HACK: shrink the classname for Fortran to make function names shorter.
        QString n(name.toUpper());
        if(name == "MeshMetaData")
            n = "MDMESH";
        else if(name == "VariableMetaData")
            n = "MDVAR";
        else if(name == "MaterialMetaData")
            n = "MDMAT";
        else if(name == "CurveMetaData")
            n = "MDCURVE";
        else if(name == "ExpressionMetaData")
            n = "MDEXPR";
        else if(name == "SpeciesMetaData")
            n = "MDSPECIES";
        else if(name == "CommandMetaData")
            n = "MDCMD";

        h << "#define F_" << pad(QString("VISIT") + n + "ALLOC", 30) << "  F77_ID(visit"<<n.toLower()<<"alloc_,visit"<<n.toLower()<<"alloc,VISIT"<<n<<"ALLOC)" << endl;
        h << "#define F_" << pad(QString("VISIT") + n + "FREE", 30) << "  F77_ID(visit"<<n.toLower()<<"free_,visit"<<n.toLower()<<"free,VISIT"<<n<<"FREE)" << endl;
        for (size_t i=0; i<fields.size(); i++)
            fields[i]->WriteVisItFortranMacro(h, n);
        h << endl;

        h << "int" << endl;
        h << "F_VISIT"<<n<<"ALLOC(visit_handle *h)" << endl;
        h << "{" << endl;
        h << "    return VisIt_" << name << "_alloc(h);" << endl;
        h << "}" << endl;
        h << endl;

        h << "int" << endl;
        h << "F_VISIT"<<n<<"FREE(visit_handle *h)" << endl;
        h << "{" << endl;
        h << "    return VisIt_"<<name<<"_free(*h);" << endl;
        h << "}" << endl;
        h << endl;

        for (size_t i=0; i<fields.size(); i++)
            fields[i]->WriteVisItFortranFunction(h, name, n);
    }

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    void WriteSimV2Header(QTextStream &h)
    {
        h << copyright_str << endl;

        h << "#ifndef SIMV2_" << name.toUpper() << "_H" << endl;
        h << "#define SIMV2_" << name.toUpper() << "_H" << endl;
        h << "#include <VisItSimV2Exports.h>" << endl;
        h << "#include <VisItInterfaceTypes_V2.h>" << endl;
        h << endl;
        h << "// C-callable implementation of front end functions" << endl;
        h << "#ifdef __cplusplus" << endl;
        h << "extern \"C\" {" << endl;
        h << "#endif" << endl;
        h << endl;
        h << "SIMV2_API int simv2_" << name << "_alloc(visit_handle *obj);" << endl;
        h << "SIMV2_API int simv2_" << name << "_free(visit_handle obj);" << endl;
        for (size_t i=0; i<fields.size(); i++)
            fields[i]->WriteSimV2FunctionPrototype(h, name);
        h << endl;
        h << "#ifdef __cplusplus" << endl;
        h << "};" << endl;
        h << "#endif" << endl;
        h << endl;
        h << "// Callable from within the runtime and SimV2" << endl;
        // Write user-defined methods
        for (size_t i=0; i<functions.size(); i++)
        {
            if (functions[i]->user && 
                functions[i]->target == generatorName)
            {
                h << functions[i]->decl << endl;
            }
        }
        h << endl;
        h << "#endif" << endl;
    }

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    void WriteSimV2Source(QTextStream &h)
    {
        h << copyright_str << endl;

        h << "#include <cstring>" << endl;
        h << "#include <snprintf.h>" << endl;
        h << "#include <vectortypes.h>" << endl;
        h << "#include \"VisItDataInterfaceRuntime.h\"" << endl;
        h << "#include \"VisItDataInterfaceRuntimeP.h\"" << endl;
        h << "" << endl;
        h << "#include \"simv2_" << name << ".h\"" << endl;
        h << "" << endl;
        h << "struct VisIt_" << name << " : public VisIt_ObjectBase" << endl;
        h << "{" << endl;
        h << "    VisIt_"<<name<<"();" << endl;
        h << "    virtual ~VisIt_"<<name<<"();" << endl;
        h << endl;
        int maxLen = 3;
        for(size_t i = 0; i < fields.size(); ++i)
        {
            int len = fields[i]->GetCPPName().length();
            if(len > maxLen)
                maxLen = len;
        }
        for(size_t i = 0; i < fields.size(); ++i)
            fields[i]->WriteSimV2ClassField(h, maxLen);
        h << "};" << endl;
        h << endl;

        // ctor
        h << "VisIt_" << name << "::VisIt_" << name << "() : VisIt_ObjectBase(VISIT_" << name.toUpper() << ")" << endl;
        h << "{" << endl;
        for(size_t i = 0; i < fields.size(); ++i)
            fields[i]->WriteSimV2Ctor(h);
        h << "}" << endl;
        h <<endl;

        // dtor
        h << "VisIt_" << name << "::~VisIt_" << name << "()" << endl;
        h << "{" << endl;
        h << "}" << endl;
        h <<endl;

        // GetObject
        h << "static VisIt_"<<name<<" *" << endl;
        h << "GetObject(visit_handle h, const char *fname)" << endl;
        h << "{" << endl;
        h << "    char tmp[150];" << endl;
        h << "    VisIt_"<<name<<" *obj = (VisIt_"<<name<<" *)VisItGetPointer(h);" << endl;
        h << "    if(obj != NULL)" << endl;
        h << "    {" << endl;
        h << "        if(obj->objectType() != VISIT_"<<name.toUpper()<<")" << endl;
        h << "        {" << endl;
        h << "            SNPRINTF(tmp, 150, \"%s: The provided handle does not point to \"" << endl;
        h << "                \"a "<<name<<" object. (type=%d)\", fname, obj->type);" << endl;
        h << "            VisItError(tmp);" << endl;
        h << "            obj = NULL;" << endl;
        h << "        }" << endl;
        h << "    }" << endl;
        h << "    else" << endl;
        h << "    {" << endl;
        h << "        SNPRINTF(tmp, 150, \"%s: An invalid handle was provided.\", fname);" << endl;
        h << "        VisItError(tmp);" << endl;
        h << "    }" << endl;
        h << endl;
        h << "    return obj;" << endl;
        h << "}" << endl;
        h << endl;

        h << "/*******************************************************************************" << endl;
        h << " * Public functions, available to C " << endl;
        h << " ******************************************************************************/" << endl;
        h << "" << endl;
        h << "int" << endl;
        h << "simv2_"<<name<<"_alloc(visit_handle *h)" << endl;
        h << "{" << endl;
        h << "    *h = VisItStorePointer(new VisIt_"<<name<<");" << endl;
        h << "    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;" << endl;
        h << "}" << endl;
        h << "" << endl;
        h << "int" << endl;
        h << "simv2_"<<name<<"_free(visit_handle h)" << endl;
        h << "{" << endl;
        h << "    int retval = VISIT_ERROR;" << endl;
        h << "    VisIt_"<<name<<" *obj = GetObject(h, \"simv2_"<<name<<"_free\");" << endl;
        h << "    if(obj != NULL)" << endl;
        h << "    {" << endl;
        h << "        delete obj;" << endl;
        h << "        VisItFreePointer(h);" << endl;
        h << "        retval = VISIT_OKAY;" << endl;
        h << "    }" << endl;
        h << "    return retval;" << endl;
        h << "}" << endl;
        h << endl;

        for(size_t i = 0; i < fields.size(); ++i)
            fields[i]->WriteSimV2Function(h, name);

        // Write user-defined methods
        for (size_t i=0; i<functions.size(); i++)
        {
            if (functions[i]->user && 
                functions[i]->target == generatorName)
            {
                h << functions[i]->def << endl;
            }
        }
    }

private:
    QString PadStringWithSpaces(const QString &s, int len) const
    {
        QString ret(s);
        while((int)(ret.length()) < len)
            ret += QString(" ");
        return ret;
    }
};

// ----------------------------------------------------------------------------
// Modifications:
//
// ----------------------------------------------------------------------------
#include <PluginBase.h>

class AttsGeneratorPlugin : public PluginBase
{
  public:
    AttsGeneratorAttribute *atts;
  public:
    AttsGeneratorPlugin(const QString &n,const QString &l,const QString &t,
        const QString &vt,const QString &dt, const QString &v, const QString &ifile,
        bool hw, bool ho, bool onlyengine, bool noengine) : 
        PluginBase(n,l,t,vt,dt,v,ifile,hw,ho,onlyengine,noengine), atts(NULL)
    {
    }

    void Print(QTextStream &out)
    {
        out << "Plugin: "<<name<<" (\""<<label<<"\", type="<<type<<") -- version "<<version<< Endl;
        if (atts)
            atts->Print(out);
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
