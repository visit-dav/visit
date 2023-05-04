// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Kathleen Biagas, Tue Dec 20 16:04:19 PST 2016
//    Added GlyphType.
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
    QString bGetCPPName() { return GetCPPName(); }

    virtual QString CArgument() const { return type; }
    virtual QString CArgumentSet(const QString &var) const { return !var.isEmpty() ? (CArgument()+" "+var) : CArgument(); }
    virtual QString CArgumentGet(const QString &var) const { return CArgument()+"*"+var; }

    virtual void WriteVisItFunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "int VisIt_" << classname << "_set" << Name << "(visit_handle h, "
          << CArgumentSet("") << ");" << Endl;
        h << "int VisIt_" << classname << "_get" << Name << "(visit_handle h, "
          << CArgumentGet("") << ");" << Endl;
    }

    virtual void WriteVisItFunction(QTextStream &h, const QString &classname)
    {
        h << "int\nVisIt_" << classname << "_set" << Name
          << "(visit_handle h, " << CArgumentSet("val") << ")" << Endl;
        h << "{" << Endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_set" << Name << "," << Endl;
        h << "        int, (visit_handle, " << CArgumentSet("") << ")," << Endl;
        h << "        (h, val));" << Endl;
        h << "}" << Endl;
        h << Endl;

        h << "int\nVisIt_" << classname << "_get" << Name
          << "(visit_handle h, " << CArgumentGet("val") + ")" << Endl;
        h << "{" << Endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_get" << Name << "," << Endl;
        h << "        int, (visit_handle, " << CArgumentGet("") << ")," << Endl;
        h << "        (h, val));" << Endl;
        h << "}" << Endl;
        h << Endl;
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
          << mName.toLower() << ", " << mName << ")" << Endl;

        mName = QString(FortranName(classname, "GET"));
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_,"
          << mName.toLower() << ", " << mName << ")" << Endl;
    }

    virtual void WriteVisItFortranFunction(QTextStream &h, const QString &classname, const QString &fclass)
    {
        QString mName(FortranName(fclass, "SET"));

        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, " << CArgument() << " *val)" << Endl;
        h << "{" << Endl;
        h << "    return VisIt_" << classname << "_set" << Name << "(*h, *val);" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = QString(FortranName(fclass, "GET"));
        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, " << CArgument() << " *val)" << Endl;
        h << "{" << Endl;
        h << "    return VisIt_" << classname << "_get" << Name << "(*h, val);" << Endl;
        h << "}" << Endl;
        h << Endl;
    }

    virtual void WriteSimV2FunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "SIMV2_API int simv2_" << classname << "_set" << Name
          << "(visit_handle h, " << CArgumentSet("") << ");" << Endl;
        h << "SIMV2_API int simv2_" << classname << "_get" << Name
          << "(visit_handle h, " << CArgumentGet("") << ");" << Endl;
    }

    virtual void WriteSimV2ClassField(QTextStream &h, int maxLen)
    {
        h << "    " << pad(GetCPPName(), maxLen) << " " << name << ";" << Endl;
    }

    virtual void WriteSimV2Ctor(QTextStream &h)
    {
    }

    virtual void WriteSimV2Function_set_helper(QTextStream &h)
    {
        h << "        obj->" << name << " = val;" << Endl;
    }

    virtual void WriteSimV2Function_get_helper(QTextStream &h)
    {
        h << "        *val = obj->" << name << ";" << Endl;
    }

    virtual void WriteSimV2Function(QTextStream &h, const QString &classname)
    {
        QString mName(QString("simv2_") + classname + "_set" + Name);

        h << "int" << Endl;
        h << mName
          << "(visit_handle h, " << CArgumentSet("val") << ")" << Endl;
        h << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << Endl;
        h << "    if(obj != NULL)" << Endl;
        h << "    {" << Endl;
        WriteSimV2Function_set_helper(h);
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = QString("simv2_") + classname + "_get" + Name;
        h << "int" << Endl;
        h << mName
          << "(visit_handle h, " << CArgumentGet(" val") << ")" << Endl;
        h << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    if(val == NULL)" << Endl;
        h << "    {" << Endl;
        h << "        VisItError(\"" << mName << ": Invalid address.\");" << Endl;
        h << "        return VISIT_ERROR;" << Endl;
        h << "    }" << Endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << Endl;
        h << "    if(obj != NULL)" << Endl;
        h << "    {" << Endl;
        WriteSimV2Function_get_helper(h);
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        h << "    else" << Endl;
        h << "        *val = 0;" << Endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;
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
            h << "    " << name << " = " << val << ";" << Endl;
        else
            h << "    " << name << " = 0;" << Endl;
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
    virtual void WriteSimV2ClassField(QTextStream &h, int maxLen)
    {
        h << "    " << pad(bGetCPPName(), maxLen) << " " << name << "[" << length << "];" << Endl;
    }

    virtual void WriteSimV2Function_set_helper(QTextStream &h)
    {
        for(int i = 0; i < length; ++i)
             h << "        obj->" << name << "[" << i << "] = val[" << i << "];" << Endl;
    }

    virtual void WriteSimV2Function_get_helper(QTextStream &h)
    {
        for(int i = 0; i < length; ++i)
             h << "        val[" << i << "] = obj->" << name << "[" << i << "];" << Endl;
    }
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
          << "(visit_handle h, int);" << Endl;
        h << "int VisIt_" << classname << "_getNum" << sing()
          << "(visit_handle h, int *);" << Endl;
        h << "int VisIt_" << classname << "_get" << sing()
          << "(visit_handle h, int, int *);" << Endl;
    }

    virtual void WriteVisItFunction(QTextStream &h, const QString &classname)
    {
        h << "int\nVisIt_" << classname << "_add" << sing()
          << "(visit_handle h, int val)" << Endl;
        h << "{" << Endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_add" << sing() << "," << Endl;
        h << "        int, (visit_handle, int)," << Endl;
        h << "        (h, val));" << Endl;
        h << "}" << Endl;
        h << Endl;
        h << "int\nVisIt_" << classname << "_getNum" << sing()
          << "(visit_handle h, int *n)" << Endl;
        h << "{" << Endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_getNum" << sing() << "," << Endl;
        h << "        int, (visit_handle, int*)," << Endl;
        h << "        (h, n));" << Endl;
        h << "}" << Endl;
        h << Endl;
        h << "int\nVisIt_" << classname << "_get" << sing()
          << "(visit_handle h, int i, int *val)" << Endl;
        h << "{" << Endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_get" << sing() << "," << Endl;
        h << "        int, (visit_handle, int, int*)," << Endl;
        h << "        (h, i, val));" << Endl;
        h << "}" << Endl;
        h << Endl;
    }

    virtual QString FortranName(const QString &classname, const QString &action) const
    {
        QString mName(QString("VISIT") + classname + action + FortranFieldName(Name));
        return mName;
    }

    virtual void WriteVisItFortranMacro(QTextStream &h, const QString &classname)
    {
        QString mName(FortranName(classname, "ADD"));
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_,"
          << mName.toLower() << ", " << mName << ")" << Endl;

        mName = FortranName(classname, "GETNUM");
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_,"
          << mName.toLower() << ", " << mName << ")" << Endl;

        mName = FortranName(classname, "GET");
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_,"
          << mName.toLower() << ", " << mName << ")" << Endl;
    }

    virtual void WriteVisItFortranFunction(QTextStream &h, const QString &classname, const QString &fclass)
    {
        QString mName(FortranName(fclass, "ADD"));
        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, int *val)" << Endl;
        h << "{" << Endl;
        h << "    return VisIt_" << classname << "_add" << sing() << "(*h, *val);" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = FortranName(fclass, "GETNUM");
        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, int *n)" << Endl;
        h << "{" << Endl;
        h << "    return VisIt_" << classname << "_getNum" << sing() << "(*h, n);" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = FortranName(fclass, "GET");
        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, int *i, int *val)" << Endl;
        h << "{" << Endl;
        h << "    return VisIt_" << classname << "_get" << sing() << "(*h, *i, val);" << Endl;
        h << "}" << Endl;
        h << Endl;

    }

    virtual void WriteSimV2FunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "SIMV2_API int simv2_" << classname << "_add" << sing()
          << "(visit_handle h, int);" << Endl;
        h << "SIMV2_API int simv2_" << classname << "_getNum" << sing()
          << "(visit_handle h, int *);" << Endl;
        h << "SIMV2_API int simv2_" << classname << "_get" << sing()
          << "(visit_handle h, int, int *);" << Endl;
    }

    virtual void WriteSimV2Function(QTextStream &h, const QString &classname)
    {
        QString mName(QString("simv2_") + classname + "_add" + sing());

        h << "int" << Endl;
        h << mName << "(visit_handle h, int val)" << Endl;
        h << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << Endl;
        h << "    if(obj != NULL)" << Endl;
        h << "    {" << Endl;
        h << "        obj->" << name << ".push_back(val);" << Endl;
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = (QString("simv2_") + classname + "_getNum" + sing());
        h << "int" << Endl;
        h << mName << "(visit_handle h, int *n)" << Endl;
        h << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << Endl;
        h << "    if(obj != NULL && n != NULL)" << Endl;
        h << "    {" << Endl;
        h << "        *n = obj->" << name << ".size();" << Endl;
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        h << "    else" << Endl;
        h << "        *n = 0;" << Endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = (QString("simv2_") + classname + "_get" + sing());
        h << "int" << Endl;
        h << mName << "(visit_handle h, int i, int *val)" << Endl;
        h << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << Endl;
        h << "    if(obj != NULL && i >= 0 && i < obj->" << name << ".size())" << Endl;
        h << "    {" << Endl;
        h << "        *val = obj->" << name << "[i];" << Endl;
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        h << "    else" << Endl;
        h << "        *val = 0;" << Endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;
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
            h << "    " << name << " = " << (val ? "true":"false") << ";" << Endl;
        else
            h << "    " << name << " = false;" << Endl;
    }

    virtual void WriteSimV2Function_set_helper(QTextStream &h)
    {
        h << "        obj->" << name << " = (val > 0);" << Endl;
    }

    virtual void WriteSimV2Function_get_helper(QTextStream &h)
    {
        h << "        *val = obj->" << name << " ? 1 : 0;" << Endl;
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
            h << "    " << name << " = " << val << ";" << Endl;
        else
            h << "    " << name << " = 0.f;" << Endl;
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
    virtual QString CArgument() const { return QString("float[%1]").arg(length); }
    virtual QString CArgumentSet(const QString &var) const { return QString("float %1[%2]").arg(var).arg(length); }
    virtual QString CArgumentGet(const QString &var) const { return QString("float %1[%2]").arg(var).arg(length); }
    virtual void WriteSimV2ClassField(QTextStream &h, int maxLen)
    {
        h << "    " << pad(bGetCPPName(), maxLen) << " " << name << "[" << length << "];" << Endl;
    }
    virtual void WriteSimV2Function_set_helper(QTextStream &h)
    {
        for(int i = 0; i < length; ++i)
             h << "        obj->" << name << "[" << i << "] = val[" << i << "];" << Endl;
    }

    virtual void WriteSimV2Function_get_helper(QTextStream &h)
    {
        for(int i = 0; i < length; ++i)
             h << "        val[" << i << "] = obj->" << name << "[" << i << "];" << Endl;
    }

    virtual void WriteVisItFortranFunction(QTextStream &h, const QString &classname, const QString &fclass)
    {
        QString mName(FortranName(fclass, "SET"));

        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, " << CArgumentSet("val") << ")" << Endl;
        h << "{" << Endl;
        h << "    return VisIt_" << classname << "_set" << Name << "(*h, val);" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = QString(FortranName(fclass, "GET"));
        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, " << CArgumentGet("val") << ")" << Endl;
        h << "{" << Endl;
        h << "    return VisIt_" << classname << "_get" << Name << "(*h, val);" << Endl;
        h << "}" << Endl;
        h << Endl;
    }

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
            h << "    " << name << " = " << val << ";" << Endl;
        else
            h << "    " << name << " = 0.;" << Endl;
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
    virtual QString CArgumentSet(const QString &var) const { return QString("double %1[%2]").arg(var).arg(length); }
    virtual QString CArgumentGet(const QString &var) const { return QString("double %1[%2]").arg(var).arg(length); }
    virtual void WriteSimV2ClassField(QTextStream &h, int maxLen)
    {
        h << "    " << pad(bGetCPPName(), maxLen) << " " << name << "[" << length << "];" << Endl;
    }
    virtual void WriteSimV2Function_set_helper(QTextStream &h)
    {
        for(int i = 0; i < length; ++i)
             h << "        obj->" << name << "[" << i << "] = val[" << i << "];" << Endl;
    }

    virtual void WriteSimV2Function_get_helper(QTextStream &h)
    {
        for(int i = 0; i < length; ++i)
             h << "        val[" << i << "] = obj->" << name << "[" << i << "];" << Endl;
    }

    virtual void WriteVisItFortranFunction(QTextStream &h, const QString &classname, const QString &fclass)
    {
        QString mName(FortranName(fclass, "SET"));

        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, " << CArgumentSet("val") << ")" << Endl;
        h << "{" << Endl;
        h << "    return VisIt_" << classname << "_set" << Name << "(*h, val);" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = QString(FortranName(fclass, "GET"));
        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, " << CArgumentGet("val") << ")" << Endl;
        h << "{" << Endl;
        h << "    return VisIt_" << classname << "_get" << Name << "(*h, val);" << Endl;
        h << "}" << Endl;
        h << Endl;
    }

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
        h << "int VisIt_" << classname << "_set" << Name << "(visit_handle h, const char *);" << Endl;
        h << "int VisIt_" << classname << "_get" << Name << "(visit_handle h, char **);" << Endl;
    }

    virtual void WriteVisItFunction(QTextStream &h, const QString &classname)
    {
        h << "int\nVisIt_" << classname << "_set" << Name
          << "(visit_handle h, const char *val)" << Endl;
        h << "{" << Endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_set" << Name << "," << Endl;
        h << "        int, (visit_handle, const char *)," << Endl;
        h << "        (h, val));" << Endl;
        h << "}" << Endl;
        h << Endl;

        h << "int\nVisIt_" << classname << "_get" << Name
          << "(visit_handle h, char **val)" << Endl;
        h << "{" << Endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_get" << Name << "," << Endl;
        h << "        int, (visit_handle, char **)," << Endl;
        h << "        (h, val));" << Endl;
        h << "}" << Endl;
        h << Endl;
    }

    virtual void WriteVisItFortranFunction(QTextStream &h, const QString &classname, const QString &fclass)
    {
        QString mName(FortranName(fclass, "SET"));

        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, const char *val, int *lval)" << Endl;
        h << "{" << Endl;
        h << "    char *f_val = NULL;" << Endl;
        h << "    int retval;" << Endl;
        h << "    COPY_FORTRAN_STRING(f_val, val, lval);" << Endl;
        h << "    retval = VisIt_" << classname << "_set" << Name << "(*h, f_val);" << Endl;
        h << "    FREE(f_val);" << Endl;
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = FortranName(fclass, "GET");
        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, char *val, int *lval)" << Endl;
        h << "{" << Endl;
        h << "    char *s = NULL;" << Endl;
        h << "    int retval = VisIt_" << classname << "_get" << Name << "(*h, &s);" << Endl;
        h << "    if(s != NULL)" << Endl;
        h << "    {" << Endl;
        h << "        visit_cstring_to_fstring(s, val, *lval);" << Endl;
        h << "        free(s);" << Endl;
        h << "    }" << Endl;
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;
    }

    virtual void WriteSimV2FunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "SIMV2_API int simv2_" << classname << "_set" << Name << "(visit_handle h, const char *);" << Endl;
        h << "SIMV2_API int simv2_" << classname << "_get" << Name << "(visit_handle h, char **);" << Endl;
    }

    virtual void WriteSimV2Ctor(QTextStream &h)
    {
        if(valueSet)
            h << "    " << name << " = \"" << val << "\";" << Endl;
        else
            h << "    " << name << " = \"\";" << Endl;
    }

    virtual void WriteSimV2Function(QTextStream &h, const QString &classname)
    {
        QString mName(QString("simv2_") + classname + "_set" + Name);

        h << "int" << Endl;
        h << mName << "(visit_handle h, const char *val)" << Endl;
        h << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == NULL)" << Endl;
        h << "    {" << Endl;
        h << "        VisItError(\"An invalid string was provided for " << name << "\");" << Endl;
        h << "        return VISIT_ERROR;" << Endl;
        h << "    }" << Endl;
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << Endl;
        h << "    if(obj != NULL)" << Endl;
        h << "    {" << Endl;
        h << "        obj->" << name << " = val;" << Endl;
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = (QString("simv2_") + classname + "_get" + Name);
        h << "int" << Endl;
        h << mName << "(visit_handle h, char **val)" << Endl;
        h << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == NULL)" << Endl;
        h << "    {" << Endl;
        h << "        VisItError(\"" << mName << ": Invalid address.\");" << Endl;
        h << "        return VISIT_ERROR;" << Endl;
        h << "    }" << Endl;
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << Endl;
        h << "    if(obj != NULL)" << Endl;
        h << "    {" << Endl;
        h << "        *val = (char*)malloc(obj->" << name << ".size() + 1);" << Endl;
        h << "        strcpy(*val, obj->" << name << ".c_str());" << Endl;
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        h << "    else" << Endl;
        h << "        *val = NULL;" << Endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;
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
        h << "int VisIt_" << classname << "_add" << sing() << "(visit_handle h, const char *);" << Endl;
        h << "int VisIt_" << classname << "_getNum" << sing() << "(visit_handle h, int *);" << Endl;
        h << "int VisIt_" << classname << "_get" << sing() << "(visit_handle h, int, char **);" << Endl;
    }

    virtual void WriteVisItFunction(QTextStream &h, const QString &classname)
    {
        h << "int\nVisIt_" << classname << "_add" << sing()
          << "(visit_handle h, const char *val)" << Endl;
        h << "{" << Endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_add" << sing() << "," << Endl;
        h << "        int, (visit_handle, const char *)," << Endl;
        h << "        (h, val));" << Endl;
        h << "}" << Endl;
        h << Endl;

        h << "int\nVisIt_" << classname << "_getNum" << sing()
          << "(visit_handle h, int *val)" << Endl;
        h << "{" << Endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_getNum" << sing() << "," << Endl;
        h << "        int, (visit_handle, int *)," << Endl;
        h << "        (h, val));" << Endl;
        h << "}" << Endl;
        h << Endl;

        h << "int\nVisIt_" << classname << "_get" << sing()
          << "(visit_handle h, int i, char **val)" << Endl;
        h << "{" << Endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_get" << sing() << "," << Endl;
        h << "        int, (visit_handle, int, char **)," << Endl;
        h << "        (h, i, val));" << Endl;
        h << "}" << Endl;
        h << Endl;
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
          << mName.toLower() << ", " << mName << ")" << Endl;

        mName = QString(FortranName(classname, "GETNUM"));
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_,"
          << mName.toLower() << ", " << mName << ")" << Endl;

        mName = QString(FortranName(classname, "GET"));
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_,"
          << mName.toLower() << ", " << mName << ")" << Endl;
    }

    virtual void WriteVisItFortranFunction(QTextStream &h, const QString &classname, const QString &fclass)
    {
        QString mName(FortranName(fclass, "ADD"));

        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, const char *val, int *lval)" << Endl;
        h << "{" << Endl;
        h << "    char *f_val = NULL;" << Endl;
        h << "    int retval;" << Endl;
        h << "    COPY_FORTRAN_STRING(f_val, val, lval);" << Endl;
        h << "    retval = VisIt_" << classname << "_add" << sing() << "(*h, f_val);" << Endl;
        h << "    FREE(f_val);" << Endl;
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = FortranName(fclass, "GETNUM");
        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, int *val)" << Endl;
        h << "{" << Endl;
        h << "    return VisIt_" << classname << "_getNum" << sing() << "(*h, val);" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = FortranName(fclass, "GET");
        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, int *i, char *val, int *lval)" << Endl;
        h << "{" << Endl;
        h << "    char *s = NULL;" << Endl;
        h << "    int retval;" << Endl;
        h << "    retval = VisIt_" << classname << "_get" << sing() << "(*h, *i, &s);" << Endl;
        h << "    if(s != NULL)" << Endl;
        h << "    {" << Endl;
        h << "        visit_cstring_to_fstring(s, val, *lval);" << Endl;
        h << "        free(s);" << Endl;
        h << "    }" << Endl;
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;
    }

    virtual void WriteSimV2FunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "SIMV2_API int simv2_" << classname << "_add" << sing()
          << "(visit_handle h, const char *);" << Endl;

        h << "SIMV2_API int simv2_" << classname << "_getNum" << sing()
          << "(visit_handle h, int *);" << Endl;

        h << "SIMV2_API int simv2_" << classname << "_get" << sing()
          << "(visit_handle h, int, char **);" << Endl;
    }

    virtual void WriteSimV2Function(QTextStream &h, const QString &classname)
    {
        QString mName(QString("simv2_") + classname + "_add" + sing());

        h << "int" << Endl;
        h << mName << "(visit_handle h, const char *val)" << Endl;
        h << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == NULL)" << Endl;
        h << "    {" << Endl;
        h << "        VisItError(\"An invalid string was provided for " << name << "\");" << Endl;
        h << "        return VISIT_ERROR;" << Endl;
        h << "    }" << Endl;
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << Endl;
        h << "    if(obj != NULL)" << Endl;
        h << "    {" << Endl;
        h << "        obj->" << name << ".push_back(val);" << Endl;
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = (QString("simv2_") + classname + "_getNum" + sing());
        h << "int" << Endl;
        h << mName << "(visit_handle h, int *val)" << Endl;
        h << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == NULL)" << Endl;
        h << "    {" << Endl;
        h << "        VisItError(\"" << mName << ": Invalid address\");" << Endl;
        h << "        return VISIT_ERROR;" << Endl;
        h << "    }" << Endl;
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << Endl;
        h << "    if(obj != NULL)" << Endl;
        h << "    {" << Endl;
        h << "        *val = obj->" << name << ".size();" << Endl;
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        h << "    else" << Endl;
        h << "        *val = 0;" << Endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = (QString("simv2_") + classname + "_get" + sing());
        h << "int" << Endl;
        h << mName << "(visit_handle h, int i, char **val)" << Endl;
        h << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == NULL)" << Endl;
        h << "    {" << Endl;
        h << "        VisItError(\"" << mName << ": Invalid address\");" << Endl;
        h << "        return VISIT_ERROR;" << Endl;
        h << "    }" << Endl;
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << Endl;
        h << "    if(obj != NULL && i >= 0 && i < obj->" << name << ".size())" << Endl;
        h << "    {" << Endl;
        h << "        *val = (char *)malloc(obj->" << name << "[i].size() + 1);" << Endl;
        h << "        strcpy(*val, obj->" << name << "[i].c_str());" << Endl;
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        h << "    else" << Endl;
        h << "        *val = NULL;" << Endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;
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
        h << "int VisIt_" << classname << "_add" << sing() << "(visit_handle h, visit_handle);" << Endl;
        h << "int VisIt_" << classname << "_getNum" << sing() << "(visit_handle h, visit_handle *);" << Endl;
        h << "int VisIt_" << classname << "_get" << sing() << "(visit_handle h, int, visit_handle*);" << Endl;
    }

    virtual void WriteVisItFunction(QTextStream &h, const QString &classname)
    {
        h << "int\nVisIt_" << classname << "_add" << sing()
          << "(visit_handle h, visit_handle val)" << Endl;
        h << "{" << Endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_add" << sing() << "," << Endl;
        h << "        int, (visit_handle, visit_handle)," << Endl;
        h << "        (h, val));" << Endl;
        h << "}" << Endl;
        h << Endl;

        h << "int\nVisIt_" << classname << "_getNum" << sing()
          << "(visit_handle h, int *val)" << Endl;
        h << "{" << Endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_getNum" << sing() << "," << Endl;
        h << "        int, (visit_handle, int *)," << Endl;
        h << "        (h, val));" << Endl;
        h << "}" << Endl;
        h << Endl;

        h << "int\nVisIt_" << classname << "_get" << sing()
          << "(visit_handle h, int i, visit_handle *val)" << Endl;
        h << "{" << Endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << classname << "_get" << sing() << "," << Endl;
        h << "        int, (visit_handle, int, visit_handle *)," << Endl;
        h << "        (h, i, val));" << Endl;
        h << "}" << Endl;
        h << Endl;
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
          << mName.toLower() << ", " << mName << ")" << Endl;

        mName = QString(FortranName(classname, "GETNUM"));
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_,"
          << mName.toLower() << ", " << mName << ")" << Endl;

        mName = QString(FortranName(classname, "GET"));
        h << "#define F_" << pad(mName,30) << "  F77_ID(" << mName.toLower() << "_,"
          << mName.toLower() << ", " << mName << ")" << Endl;
    }

    virtual void WriteVisItFortranFunction(QTextStream &h, const QString &classname, const QString &fclass)
    {
        QString mName(FortranName(fclass, "ADD"));

        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, visit_handle *val)" << Endl;
        h << "{" << Endl;
        h << "    return VisIt_" << classname << "_add" << sing() << "(*h, *val);" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = FortranName(fclass, "GETNUM");
        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, int *val)" << Endl;
        h << "{" << Endl;
        h << "    return VisIt_" << classname << "_getNum" << sing() << "(*h, val);" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = FortranName(fclass, "GET");
        h << "int" << Endl;
        h << "F_" << mName << "(visit_handle *h, int *i, visit_handle *val)" << Endl;
        h << "{" << Endl;
        h << "    return VisIt_" << classname << "_get" << sing() << "(*h, *i, val);" << Endl;
        h << "}" << Endl;
        h << Endl;
    }

    virtual void WriteSimV2FunctionPrototype(QTextStream &h, const QString &classname)
    {
        h << "SIMV2_API int simv2_" << classname << "_add" << sing()
          << "(visit_handle h, visit_handle);" << Endl;

        h << "SIMV2_API int simv2_" << classname << "_getNum" << sing()
          << "(visit_handle h, int *);" << Endl;

        h << "SIMV2_API int simv2_" << classname << "_get" << sing()
          << "(visit_handle h, int, visit_handle *);" << Endl;
    }

    virtual void WriteSimV2ClassField(QTextStream &h, int maxLen)
    {
        h << "    " << pad("std::vector<visit_handle>", maxLen) << " " << name << ";" << Endl;
    }

    virtual void WriteSimV2Function(QTextStream &h, const QString &classname)
    {
        QString mName(QString("simv2_") + classname + "_add" + sing());

        h << "int" << Endl;
        h << mName << "(visit_handle h, visit_handle val)" << Endl;
        h << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == VISIT_INVALID_HANDLE)" << Endl;
        h << "    {" << Endl;
        h << "        VisItError(\"An invalid handle was provided for " << name << "\");" << Endl;
        h << "        return VISIT_ERROR;" << Endl;
        h << "    }" << Endl;
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << Endl;
        h << "    if(obj != NULL)" << Endl;
        h << "    {" << Endl;
        h << "        obj->" << name << ".push_back(val);" << Endl;
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = (QString("simv2_") + classname + "_getNum" + sing());
        h << "int" << Endl;
        h << mName << "(visit_handle h, int *val)" << Endl;
        h << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == NULL)" << Endl;
        h << "    {" << Endl;
        h << "        VisItError(\"" << mName << ": Invalid address\");" << Endl;
        h << "        return VISIT_ERROR;" << Endl;
        h << "    }" << Endl;
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << Endl;
        h << "    if(obj != NULL)" << Endl;
        h << "    {" << Endl;
        h << "        *val = obj->" << name << ".size();" << Endl;
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        h << "    else" << Endl;
        h << "        *val = 0;" << Endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = (QString("simv2_") + classname + "_get" + sing());
        h << "int" << Endl;
        h << mName << "(visit_handle h, int i, visit_handle *val)" << Endl;
        h << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(val == NULL)" << Endl;
        h << "    {" << Endl;
        h << "        VisItError(\"" << mName << ": Invalid address\");" << Endl;
        h << "        return VISIT_ERROR;" << Endl;
        h << "    }" << Endl;
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << Endl;
        h << "    if(obj != NULL && i >= 0 && i < obj->" << name << ".size())" << Endl;
        h << "    {" << Endl;
        h << "        *val = obj->" << name << "[i];" << Endl;
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        h << "    else" << Endl;
        h << "        *val = NULL;" << Endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;
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
             h << "    " << name << " = " <<enumType->values[val] << ";" << Endl;
        else
             h << "    " << name << " = " <<enumType->values[0] << ";" << Endl;
    }
    virtual void WriteSimV2Function(QTextStream &h, const QString &classname)
    {
        QString mName(QString("simv2_") + classname + "_set" + Name);

        h << "int" << Endl;
        h << mName << "(visit_handle h, " << CArgument() << " val)" << Endl;
        h << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    if(";
        for(size_t i = 0; i < enumType->values.size(); ++i)
        {
            if(i > 0)
                h << "       ";
            h << "val != " << enumType->values[i];
            if(i < enumType->values.size()-1)
                h << " &&" << Endl;
        }
        h << ")" << Endl;
        h << "    {" << Endl;
        h << "        VisItError(\"The value for " << name << " must be one of: ";
        for(size_t i = 0; i < enumType->values.size(); ++i)
        {
            h << enumType->values[i];
            if(i < enumType->values.size()-1)
                h << ", ";
        }
        h << "\");" << Endl;
        h << "        return VISIT_ERROR;" << Endl;
        h << "    }" << Endl;
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << Endl;
        h << "    if(obj != NULL)" << Endl;
        h << "    {" << Endl;
        h << "        obj->" << name << " = val;" << Endl;
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;

        mName = QString("simv2_") + classname + "_get" + Name;
        h << "int" << Endl;
        h << mName
          << "(visit_handle h, " << CArgument() << " *val)" << Endl;
        h << "{" << Endl;
        if(HasCode(mName, 0))
            PrintCode(h, mName, 0);
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    if(val == NULL)" << Endl;
        h << "    {" << Endl;
        h << "        VisItError(\"" << mName << ": Invalid address.\");" << Endl;
        h << "        return VISIT_ERROR;" << Endl;
        h << "    }" << Endl;
        h << "    VisIt_" << classname << " *obj = GetObject(h, \"" << mName << "\");" << Endl;
        h << "    if(obj != NULL)" << Endl;
        h << "    {" << Endl;
        WriteSimV2Function_get_helper(h);
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        h << "    else" << Endl;
        h << "        *val = 0;" << Endl;
        if(HasCode(mName, 1))
            PrintCode(h, mName, 1);
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;
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


//
// --------------------------------- GlyphType --------------------------------
//
class AttsGeneratorGlyphType : public virtual GlyphType , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorGlyphType(const QString &n, const QString &l)
        : Field("glyphtype",n,l), GlyphType(n,l), AttsGeneratorField("glyphtype",n,l) { }
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
        else if (type == "glyphtype")         f = new AttsGeneratorGlyphType(name,label);

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

    virtual void Print(QTextStream &out) const
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
        h << copyright_str << Endl;

        h << "#ifndef VISIT_" << name.toUpper() << "_H" << Endl;
        h << "#define VISIT_" << name.toUpper() << "_H" << Endl;
        h << Endl;
        h << "#ifdef __cplusplus" << Endl;
        h << "extern \"C\" {" << Endl;
        h << "#endif" << Endl;
        h << Endl;
        h << "int VisIt_" << name << "_alloc(visit_handle *obj);" << Endl;
        h << "int VisIt_" << name << "_free(visit_handle obj);" << Endl;
        for (size_t i=0; i<fields.size(); i++)
            fields[i]->WriteVisItFunctionPrototype(h, name);
        h << Endl;
        h << "#ifdef __cplusplus" << Endl;
        h << "}" << Endl;
        h << "#endif" << Endl;
        h << Endl;
        h << "#endif" << Endl;
    }

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    void WriteVisItSource(QTextStream &h)
    {
        h << copyright_str << Endl;

        h << "#include <VisItDataInterface_V2.h>" << Endl;
        h << "#include \"VisItDynamic.h\"" << Endl;
        h << "#include \"VisItFortran.h\"" << Endl;
        h << Endl;
        h << "int" << Endl;
        h << "VisIt_" << name << "_alloc(visit_handle *obj)" << Endl;
        h << "{" << Endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << name << "_alloc," << Endl;
        h << "                    int, (visit_handle*)," << Endl;
        h << "                    (obj))" << Endl;
        h << "}" << Endl;
        h << Endl;
        h << "int" << Endl;
        h << "VisIt_" << name << "_free(visit_handle obj)" << Endl;
        h << "{" << Endl;
        h << "    VISIT_DYNAMIC_EXECUTE(" << name << "_free," << Endl;
        h << "                    int, (visit_handle), " << Endl;
        h << "                    (obj));" << Endl;
        h << "}" << Endl;
        h << Endl;

        for (size_t i=0; i<fields.size(); i++)
            fields[i]->WriteVisItFunction(h, name);
        h << Endl;

        h << "/************************** Fortran callable routines *************************/" << Endl;
        h << "/* maxlen 012345678901234567890123456789                                      */" << Endl;
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

        h << "#define F_" << pad(QString("VISIT") + n + "ALLOC", 30) << "  F77_ID(visit"<<n.toLower()<<"alloc_,visit"<<n.toLower()<<"alloc,VISIT"<<n<<"ALLOC)" << Endl;
        h << "#define F_" << pad(QString("VISIT") + n + "FREE", 30) << "  F77_ID(visit"<<n.toLower()<<"free_,visit"<<n.toLower()<<"free,VISIT"<<n<<"FREE)" << Endl;
        for (size_t i=0; i<fields.size(); i++)
            fields[i]->WriteVisItFortranMacro(h, n);
        h << Endl;

        h << "int" << Endl;
        h << "F_VISIT"<<n<<"ALLOC(visit_handle *h)" << Endl;
        h << "{" << Endl;
        h << "    return VisIt_" << name << "_alloc(h);" << Endl;
        h << "}" << Endl;
        h << Endl;

        h << "int" << Endl;
        h << "F_VISIT"<<n<<"FREE(visit_handle *h)" << Endl;
        h << "{" << Endl;
        h << "    return VisIt_"<<name<<"_free(*h);" << Endl;
        h << "}" << Endl;
        h << Endl;

        for (size_t i=0; i<fields.size(); i++)
            fields[i]->WriteVisItFortranFunction(h, name, n);
    }

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    void WriteSimV2Header(QTextStream &h)
    {
        h << copyright_str << Endl;

        h << "#ifndef SIMV2_" << name.toUpper() << "_H" << Endl;
        h << "#define SIMV2_" << name.toUpper() << "_H" << Endl;
        h << "#include <VisItSimV2Exports.h>" << Endl;
        h << "#include <VisItInterfaceTypes_V2.h>" << Endl;
        h << Endl;
        h << "// C-callable implementation of front end functions" << Endl;
        h << "#ifdef __cplusplus" << Endl;
        h << "extern \"C\" {" << Endl;
        h << "#endif" << Endl;
        h << Endl;
        h << "SIMV2_API int simv2_" << name << "_alloc(visit_handle *obj);" << Endl;
        h << "SIMV2_API int simv2_" << name << "_free(visit_handle obj);" << Endl;
        for (size_t i=0; i<fields.size(); i++)
            fields[i]->WriteSimV2FunctionPrototype(h, name);
        h << Endl;
        h << "#ifdef __cplusplus" << Endl;
        h << "};" << Endl;
        h << "#endif" << Endl;
        h << Endl;
        h << "// Callable from within the runtime and SimV2" << Endl;
        // Write user-defined methods
        for (size_t i=0; i<functions.size(); i++)
        {
            if (functions[i]->user &&
                functions[i]->target == generatorName)
            {
                h << functions[i]->decl << Endl;
            }
        }
        h << Endl;
        h << "#endif" << Endl;
    }

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    void WriteSimV2Source(QTextStream &h)
    {
        h << copyright_str << Endl;

        h << "#include <cstring>" << Endl;
        h << "#include <vectortypes.h>" << Endl;
        h << "#include \"VisItDataInterfaceRuntime.h\"" << Endl;
        h << "#include \"VisItDataInterfaceRuntimeP.h\"" << Endl;
        h << "" << Endl;
        h << "#include \"simv2_" << name << ".h\"" << Endl;
        h << "" << Endl;
        h << "struct VisIt_" << name << " : public VisIt_ObjectBase" << Endl;
        h << "{" << Endl;
        h << "    VisIt_"<<name<<"();" << Endl;
        h << "    virtual ~VisIt_"<<name<<"();" << Endl;
        h << Endl;
        int maxLen = 3;
        for(size_t i = 0; i < fields.size(); ++i)
        {
            int len = fields[i]->GetCPPName().length();
            if(len > maxLen)
                maxLen = len;
        }
        for(size_t i = 0; i < fields.size(); ++i)
            fields[i]->WriteSimV2ClassField(h, maxLen);
        h << "};" << Endl;
        h << Endl;

        // ctor
        h << "VisIt_" << name << "::VisIt_" << name << "() : VisIt_ObjectBase(VISIT_" << name.toUpper() << ")" << Endl;
        h << "{" << Endl;
        for(size_t i = 0; i < fields.size(); ++i)
            fields[i]->WriteSimV2Ctor(h);
        h << "}" << Endl;
        h <<Endl;

        // dtor
        h << "VisIt_" << name << "::~VisIt_" << name << "()" << Endl;
        h << "{" << Endl;
        h << "}" << Endl;
        h <<Endl;

        // GetObject
        h << "static VisIt_"<<name<<" *" << Endl;
        h << "GetObject(visit_handle h, const char *fname)" << Endl;
        h << "{" << Endl;
        h << "    char tmp[150];" << Endl;
        h << "    VisIt_"<<name<<" *obj = (VisIt_"<<name<<" *)VisItGetPointer(h);" << Endl;
        h << "    if(obj != NULL)" << Endl;
        h << "    {" << Endl;
        h << "        if(obj->objectType() != VISIT_"<<name.toUpper()<<")" << Endl;
        h << "        {" << Endl;
        h << "            snprintf(tmp, 150, \"%s: The provided handle does not point to \"" << Endl;
        h << "                \"a "<<name<<" object. (type=%d)\", fname, obj->objectType());" << Endl;
        h << "            VisItError(tmp);" << Endl;
        h << "            obj = NULL;" << Endl;
        h << "        }" << Endl;
        h << "    }" << Endl;
        h << "    else" << Endl;
        h << "    {" << Endl;
        h << "        snprintf(tmp, 150, \"%s: An invalid handle was provided.\", fname);" << Endl;
        h << "        VisItError(tmp);" << Endl;
        h << "    }" << Endl;
        h << Endl;
        h << "    return obj;" << Endl;
        h << "}" << Endl;
        h << Endl;

        h << "/*******************************************************************************" << Endl;
        h << " * Public functions, available to C " << Endl;
        h << " ******************************************************************************/" << Endl;
        h << "" << Endl;
        h << "int" << Endl;
        h << "simv2_"<<name<<"_alloc(visit_handle *h)" << Endl;
        h << "{" << Endl;
        h << "    *h = VisItStorePointer(new VisIt_"<<name<<");" << Endl;
        h << "    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;" << Endl;
        h << "}" << Endl;
        h << "" << Endl;
        h << "int" << Endl;
        h << "simv2_"<<name<<"_free(visit_handle h)" << Endl;
        h << "{" << Endl;
        h << "    int retval = VISIT_ERROR;" << Endl;
        h << "    VisIt_"<<name<<" *obj = GetObject(h, \"simv2_"<<name<<"_free\");" << Endl;
        h << "    if(obj != NULL)" << Endl;
        h << "    {" << Endl;
        h << "        delete obj;" << Endl;
        h << "        VisItFreePointer(h);" << Endl;
        h << "        retval = VISIT_OKAY;" << Endl;
        h << "    }" << Endl;
        h << "    return retval;" << Endl;
        h << "}" << Endl;
        h << Endl;

        for(size_t i = 0; i < fields.size(); ++i)
            fields[i]->WriteSimV2Function(h, name);

        // Write user-defined methods
        for (size_t i=0; i<functions.size(); i++)
        {
            if (functions[i]->user &&
                functions[i]->target == generatorName)
            {
                h << functions[i]->def << Endl;
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
//   Kathleen Biagas, Thu Jan  2 09:18:18 PST 2020
//   Added hl arg, for haslicense.
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
        bool hw, bool ho, bool hl, bool onlyengine, bool noengine) :
        PluginBase(n,l,t,vt,dt,v,ifile,hw,ho,hl,onlyengine,noengine), atts(NULL)
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
