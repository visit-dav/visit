// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef FIELD_H
#define FIELD_H

#include <stdio.h>
#include <QTextStream>
#include "Enum.h"
#include "XMLParserUtil.h"
#include "CodeFile.h"

#if defined(_WIN32)
// Turn off warnings about inheritance via dominance.
#pragma warning(disable:4250)
#endif

#include <cstdlib>
#include <iostream>

// ****************************************************************************
//  Class:  Field
//
//  Purpose:
//    Abstraction for a single field in an AttributeSubject
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 28 14:13:39 PDT 2001
//    Added a virtual destructor.
//    Added a check to Enum::SetValue to make sure the given value
//    was valid.
//
//    Jeremy Meredith, Fri Apr 19 15:32:20 PDT 2002
//    Added ignoreEquality, initcode, codeFile.
//
//    Jeremy Meredith, Wed May  8 15:25:40 PDT 2002
//    Added ability for Enums to return their subtype as their c++ name
//    instead of just "int", when needed.
//
//    Jeremy Meredith, Fri Jul 12 16:28:03 PDT 2002
//    Some portability improvements.
//
//    Jeremy Meredith, Thu Oct 17 15:56:14 PDT 2002
//    Removed "enablees" for easier maintenance.  Added some helper functions.
//
//    Jeremy Meredith, Thu Mar 27 12:54:18 PST 2003
//    Removed some unused helper functions.
//
//    Brad Whitlock, Wed Dec 8 15:41:56 PST 2004
//    Added VariableName type.
//
//    Brad Whitlock, Wed Feb 28 18:35:43 PST 2007
//    Added public/protected/private access.
//
//    Jeremy Meredith, Tue Mar 13 15:16:08 EDT 2007
//    Qt's ToDouble doesn't correctly understant +/-1e37, so use atof instead.
//
//    Jeremy Meredith, Fri Mar 16 14:15:24 EDT 2007
//    Set Color's length to 4 so the XML editor will set its values properly.
//
//    Brad Whitlock, Thu Feb 28 09:47:50 PDT 2008
//    Changed init to a map so there can be inits for many targets.
//
//    Kathleen Bonnell, Thu Dec 2 15:55:06 MST 2010
//    Changed how vartypes is parsed to fix a variable scoping issue on
//    Windows.
//
//    Kathleen Biagas, Thu Aug 25 14:18:42 PST 2011
//    Added persistent flag.
//
//    Kathleen Biagas, Tue Mar  1 11:00:37 PST 2011
//    Added MapNode type.
//
//    Dave Pugmire, Wed Sep  5 11:01:33 EDT 2012
//    Enablers produced bad code for checkboxes. There is no label. Added
//    a virtual method, HasLabel() to determine if code should be generated.
//
//    Kathleen Biagas, Tue Dec 20 16:04:19 PST 2016
//    Added GlyphType.
//
// ****************************************************************************


// ----------------------------------------------------------------------------
//                                  Field
// ----------------------------------------------------------------------------
class Field
{
  public:
    QString           type;
    QString           name;
    QString           label;
    QString           Name;

    int               length;

    bool              internal;
    bool              persistent;

    bool              isArray;
    bool              isVector;

    bool              valueSet;

    bool              ignoreEquality;

    typedef enum {AccessPrivate, AccessProtected, AccessPublic} AccessType;

    AccessType        accessType;

    Field            *enabler;
    std::vector<QString>   enableval;

    std::map<QString,QString> initcode;
    int               varTypes;

    int               index;
    CodeFile         *codeFile;

  public:
    Field(const QString &t, const QString &n, const QString &l)
        : type(t), name(n), label(l)
    {
        codeFile = NULL;
        internal = false;
        persistent = true;
        valueSet = false;
        enabler = NULL;
        if (label.isNull())
            label = name;
        Name = name;
        Name[0] = Name[0].toUpper();
        length = 1;
        isArray  = (type.right(5) == "Array");
        isVector = (type.right(6) == "Vector");
        ignoreEquality = false;
        varTypes = 0;
        accessType = AccessPrivate;
    }
    virtual ~Field() { }
    virtual void ClearValues() { }
    void CopyValues(Field *f)
    {
        internal = f->internal;
        persistent = f->persistent;
        enabler = f->enabler;
        enableval = f->enableval;
        index = f->index;
        codeFile = f->codeFile;
        accessType = f->accessType;
    }
    void SetInitCode(const QString &target, const QString &ic)
    {
        initcode[target] = ic;
    }
    QString InitCodeAsString() const
    {
        QString s;
        QString currentTarget;
        bool multipleTargets = false;
        std::map<QString,QString>::const_iterator it;
        for(it = initcode.begin(); it != initcode.end(); ++it)
        {
            if(currentTarget.isEmpty())
                currentTarget = it->first;
            if(currentTarget != it->first)
            {
                multipleTargets = true;
                break;
            }
        }
        if(multipleTargets)
        {
            for(it = initcode.begin(); it != initcode.end(); ++it)
            {
                s = s + "Target: " + it->first + "\n";
                s = s + it->second;
            }
        }
        else
        {
            for(it = initcode.begin(); it != initcode.end(); ++it)
                s = s + it->second;
        }
        return s;
    }
    void SetInitCodeFromString(const QString &value)
    {
        initcode.clear();
        if (value.isEmpty())
            return;

        // s contains a string with various targets and init codes.
        QStringList lines(value.split("\n"));
        QString currentTarget("xml2atts");
        QString currentInit;
        for(int i = 0; i < lines.size(); ++i)
        {
            if(lines[i].left(7) == QString("Target:"))
            {
                QString target = lines[i].mid(7).trimmed();
                if(!currentInit.isEmpty())
                {
                    initcode[currentTarget] = currentInit;
                }
                currentTarget = target;
                currentInit = "";
            }
            else
                currentInit += (lines[i] + "\n");
        }
        if(!currentInit.isEmpty())
        {
            initcode[currentTarget] = currentInit;
        }
    }
    virtual bool HasLabel() {return true; };
    virtual QString GetSubtype() { return ""; };
    virtual void SetSubtype(const QString &) {  };
    virtual void SetValue(const QString &s, int ix=0) = 0;
    virtual QString GetCPPName(bool subtypename=false, const QString &classname="") = 0;
    virtual void SetAttrib(const QString &a, const QString &v)
    {
        cOut << "unknown attribute " << a << " for type " << type << Endl;
    }
    void SetEnabler(Field *f)
    {
        enabler = f;
    }
    void SetEnablerValue(const QString &v)
    {
        enableval.push_back(v);
    }
    void SetInternal(const QString &v)
    {
        internal = Text2Bool(v);
    }
    void SetPersistent(const QString &v)
    {
        persistent = Text2Bool(v);
    }
    void SetPublicAccess()
    {
        accessType = AccessPublic;
    }
    void SetProtectedAccess()
    {
        accessType = AccessProtected;
    }
    void SetPrivateAccess()
    {
        accessType = AccessPrivate;
    }
    virtual void Print(QTextStream &out)
    {
        char s[1000];
        char s1[1000];
        sprintf(s1,"[%s]", type.toStdString().c_str());
        sprintf(s, "        Field %-15s: %-15s  (\"%s\")", s1, name.toStdString().c_str(), label.toStdString().c_str());
        out << s << Endl;

        if (internal)
            cOut << "            (INTERNAL)" << Endl;
        if (!persistent)
            cOut << "            (NOT PERSISTENT)" << Endl;
        if (accessType == AccessPublic)
            cOut << "            (PUBLIC)" << Endl;
        else if (accessType == AccessProtected)
            cOut << "            (PROTECTED)" << Endl;
        if (enabler)
        {
            cOut << "            enabled when " << enabler->name << " equals ";
            cOut << enableval[0];
            for (size_t i=1; i<enableval.size(); i++)
            {
                cOut << " or " << enableval[i];
            }
            cOut << Endl;
        }
    }

    bool HasCode(const QString &cName, int part, const QString &generatorName = QString())
    {
        bool retval = false;
        QStringList targets, prefix, postfix;
        if(codeFile!=NULL && codeFile->GetCode(cName, targets, prefix, postfix))
        {
            for (int i=0; i<targets.size(); i++)
                if(generatorName.isEmpty() || generatorName == targets[i])
                {
                    if(part == 0)
                        retval = !prefix[i].isNull();
                    else
                        retval = !postfix[i].isNull();
                    break;
                }
        }
        return retval;
    }

    void PrintCode(QTextStream &out, const QString &cName, int part, const QString &generatorName = QString())
    {
        QStringList targets, prefix, postfix;
        if(codeFile!=NULL && codeFile->GetCode(cName, targets, prefix, postfix))
        {
            for (int i=0; i<targets.size(); i++)
                if(generatorName.isEmpty() || generatorName == targets[i])
                {
                    if(part == 0)
                        out << prefix[i];
                    else
                        out << postfix[i];
                    break;
                }
        }
    }

    bool PrintInit(QTextStream &out, const QString &generatorName = QString()) const
    {
        // Look through the map for the init code that is for the generatorName
        // that was passed.
        for(std::map<QString,QString>::const_iterator it = initcode.begin();
            it != initcode.end(); ++it)
        {
            if(generatorName.isEmpty() || generatorName == it->first)
            {
                out << it->second;
                return true;
            }
        }
        return false;
    }

    void
    WriteMethodComment(QTextStream &out, const QString &className,
        const QString &methodName, const QString &purposeString,
        const QString &generatorName) const
    {
        out << "// ****************************************************************************" << endl;
        out << "// Method: " << className << "::" << methodName << endl;
        out << "//" << endl;
        out << "// Purpose:" << endl;
        if(!purposeString.isEmpty())
            out << "//   " << purposeString << endl;
        out << "//" << endl;
        out << "// Note:       Autogenerated by xml2atts." << endl;
        out << "//" << endl;
        out << "// Programmer: " << generatorName << endl;
        out << "// Creation:   omitted" << endl;
        out << "//" << endl;
        out << "// Modifications:" << endl;
        out << "//" << endl;
        out << "// ****************************************************************************" << endl;
        out << endl;
    }

    virtual std::vector<QString> GetValueAsText() = 0;
};


//
// ------------------------------------ Int -----------------------------------
//
class Int : public virtual Field
{
  public:
    int  val;
    bool rangeSet;
    int  min;
    int  max;
  public:
    Int(const QString &n, const QString &l) : Field("int",n,l)
    {
        rangeSet = false;
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "int";
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val = s.toInt();
        valueSet = true;
    }
    virtual void SetAttrib(const QString &a, const QString &v)
    {
        if (a == "range")
        {
            std::vector<QString> minmax = SplitValues(v);
            min = minmax[0].toInt();
            max = minmax[1].toInt();
            rangeSet = true;
        }
        else
            Field::SetAttrib(a,v);
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
        if (rangeSet)
        {
            out << "            range: " << min << " - " << max << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            retval.push_back(QString().asprintf("%d", val));
        return retval;
    }
};


//
// --------------------------------- IntArray ---------------------------------
//
class IntArray : public virtual Field
{
  public:
    int *val;
  public:
    IntArray(const QString &s, const QString &n, const QString &l) : Field("intArray",n,l)
    {
        length = s.toInt();
        val    = new int[length];
    }
    virtual ~IntArray()
    {
        if( val ) delete [] val;
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "int";
    }
    virtual void SetValue(const QString &s, int ix = 0)
    {
        val[ix] = s.toInt();
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: ";
            for (int i=0; i<length; i++)
                out << val[i] << "  ";
            out << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            for (int i=0; i<length; i++)
                retval.push_back(QString().asprintf("%d", val[i]));
        return retval;
    }
};


//
// --------------------------------- IntVector --------------------------------
//
class IntVector : public virtual Field
{
  public:
    std::vector<int> val;
  public:
    IntVector(const QString &n, const QString &l) : Field("intVector",n,l) { }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "intVector";
    }
    virtual void ClearValues()
    {
        val.clear();
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val.push_back(s.toInt());
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: ";
            for (size_t i=0; i<val.size(); i++)
                out << val[i] << "  ";
            out << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            for (size_t i=0; i<val.size(); i++)
                retval.push_back(QString().asprintf("%d", val[i]));
        return retval;
    }
};


//
// ----------------------------------- Bool -----------------------------------
//
class Bool : public virtual Field
{
  public:
    bool val;
  public:
    Bool(const QString &n, const QString &l) : Field("bool",n,l) { };
    virtual QString GetCPPName(bool, const QString &)
    {
        return "bool";
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val = Text2Bool(s);
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << (val ? "true" : "false") << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            retval.push_back(Bool2Text(val));
        return retval;
    }
};


//
// ----------------------------------- Float ----------------------------------
//
class Float : public virtual Field
{
  public:
    float val;
  public:
    Float(const QString &n, const QString &l) : Field("float",n,l) { };
    virtual QString GetCPPName(bool, const QString &)
    {
        return "float";
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val = s.toFloat();
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            retval.push_back(QString().asprintf("%f", val));
        return retval;
    }
};


//
// -------------------------------- FloatArray -------------------------------
//
class FloatArray : public virtual Field
{
  public:
    float *val;
  public:
    FloatArray(const QString &s, const QString &n, const QString &l) : Field("floatArray",n,l)
    {
        length = s.toInt();
        val    = new float[length];
    }
    virtual ~FloatArray()
    {
        if( val ) delete [] val;
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "float";
    }
    virtual void SetValue(const QString &s, int ix = 0)
    {
        val[ix] = s.toFloat();
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: ";
            for (int i=0; i<length; i++)
                out << int(val[i]) << "  ";
            out << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            for (int i=0; i<length; i++)
                retval.push_back(QString().asprintf("%f", val[i]));
        return retval;
    }
};


//
// ------------------------------- FloatVector -------------------------------
//
class FloatVector : public virtual Field
{
  public:
    std::vector<float> val;
  public:
    FloatVector(const QString &n, const QString &l) : Field("floatVector",n,l) { }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "floatVector";
    }
    virtual void ClearValues()
    {
        val.clear();
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        // can't use toFloat -- some versions of Qt disallow exponents beyond
        // single precision range.
        //val.push_back(s.toFloat());
        val.push_back(atof(s.toStdString().c_str()));
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: ";
            for (size_t i=0; i<val.size(); i++)
                out << val[i] << "  ";
            out << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            for (size_t i=0; i<val.size(); i++)
                retval.push_back(QString().asprintf("%f", val[i]));
        return retval;
    }
};


//
// ---------------------------------- Double ----------------------------------
//
class Double : public virtual Field
{
  public:
    double val;
  public:
    Double(const QString &n, const QString &l) : Field("double",n,l) { };
    virtual QString GetCPPName(bool, const QString &)
    {
        return "double";
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val = s.toDouble();
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            retval.push_back(QString().asprintf("%f", val));
        return retval;
    }
};


//
// -------------------------------- DoubleArray -------------------------------
//
class DoubleArray : public virtual Field
{
  public:
    double *val;
  public:
    DoubleArray(const QString &s, const QString &n, const QString &l) : Field("doubleArray",n,l)
    {
        length = s.toInt();
        val    = new double[length];
    }
    virtual ~DoubleArray()
    {
        if( val ) delete [] val;
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "double";
    }
    virtual void SetValue(const QString &s, int ix = 0)
    {
        val[ix] = s.toDouble();
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: ";
            for (int i=0; i<length; i++)
                out << val[i] << "  ";
            out << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            for (int i=0; i<length; i++)
                retval.push_back(QString().asprintf("%f", val[i]));
        return retval;
    }
};


//
// ------------------------------- DoubleVector -------------------------------
//
class DoubleVector : public virtual Field
{
  public:
    std::vector<double> val;
  public:
    DoubleVector(const QString &n, const QString &l) : Field("doubleVector",n,l) { }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "doubleVector";
    }
    virtual void ClearValues()
    {
        val.clear();
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        // can't use toDouble -- some versions of Qt disallow exponents beyond
        // single precision range.
        //val.push_back(s.toDouble());
        val.push_back(atof(s.toStdString().c_str()));
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: ";
            for (size_t i=0; i<val.size(); i++)
                out << val[i] << "  ";
            out << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            for (size_t i=0; i<val.size(); i++)
                retval.push_back(QString().asprintf("%f", val[i]));
        return retval;
    }
};


//
// ----------------------------------- UChar ----------------------------------
//
class UChar : public virtual Field
{
  public:
    unsigned char val;
  public:
    UChar(const QString &n, const QString &l) : Field("uchar",n,l) { };
    virtual QString GetCPPName(bool, const QString &)
    {
        return "unsigned char";
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val = s.toInt();
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << std::ios::hex << int(val) << std::ios::dec << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            retval.push_back(QString().asprintf("%d", val));
        return retval;
    }
};


//
// -------------------------------- UCharArray --------------------------------
//
class UCharArray : public virtual Field
{
  public:
    unsigned char *val;
  public:
    UCharArray(const QString &s, const QString &n, const QString &l) : Field("ucharArray",n,l)
    {
        length = s.toInt();
        val    = new unsigned char[length];
    }
    virtual ~UCharArray()
    {
        if( val ) delete [] val;
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "unsigned char";
    }
    virtual void SetValue(const QString &s, int ix = 0)
    {
        val[ix] = s.toInt();
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: ";
            for (int i=0; i<length; i++)
                out << int(val[i]) << "  ";
            out << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            for (int i=0; i<length; i++)
                retval.push_back(QString().asprintf("%d", val[i]));
        return retval;
    }
};


//
// ------------------------------- UCharVector -------------------------------
//
class UCharVector : public virtual Field
{
  public:
    std::vector<unsigned char> val;
  public:
    UCharVector(const QString &n, const QString &l) : Field("ucharVector",n,l) { }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "unsignedCharVector";
    }
    virtual void ClearValues()
    {
        val.clear();
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        int i = s.toInt();
        if(i < 0) i = 0;
        if(i > 255) i = 255;
        val.push_back(i);
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: ";
            for (size_t i=0; i<val.size(); i++)
                out << int(val[i]) << "  ";
            out << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            for (size_t i=0; i<val.size(); i++)
                retval.push_back(QString().asprintf("%d", int(val[i])));
        return retval;
    }
};


//
// ---------------------------------- String ----------------------------------
//
class String : public virtual Field
{
  public:
    QString val;
  public:
    String(const QString &n, const QString &l) : Field("string",n,l) { };
    virtual QString GetCPPName(bool, const QString &)
    {
        return "std::string";
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val = s;
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            retval.push_back(val);
        return retval;
    }
};


//
// ------------------------------- StringVector -------------------------------
//
class StringVector : public virtual Field
{
  public:
    std::vector<QString> val;
  public:
    StringVector(const QString &n, const QString &l) : Field("stringVector",n,l) { };
    virtual QString GetCPPName(bool, const QString &)
    {
        return "stringVector";
    }
    virtual void ClearValues()
    {
        val.clear();
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val.push_back(s);
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: ";
            for (size_t i=0; i<val.size(); i++)
                out << "\"" << val[i] << "\"  ";
            out << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        return val;
    }
};


//
// ----------------------------------- ColorTable -----------------------------
//
class ColorTable : public virtual Field
{
  public:
    QString val;
  public:
    ColorTable(const QString &n, const QString &l) : Field("colortable",n,l) { };
    virtual QString GetCPPName(bool, const QString &)
    {
        return "std::string";
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val = s;
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            retval.push_back(val);
        return retval;
    }
};


//
// ----------------------------------- Color ----------------------------------
//
class Color : public virtual Field
{
  public:
    unsigned char val[4];
  public:
    Color(const QString &n, const QString &l) : Field("color",n,l) { length=4; };
    virtual QString GetCPPName(bool, const QString &)
    {
        return "ColorAttribute";
    }
    virtual void SetValue(const QString &s, int ix = 0)
    {
        val[ix] = s.toInt();
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value:"
                << "  R=" << int(val[0])
                << "  G=" << int(val[1])
                << "  B=" << int(val[2])
                << "  A=" << int(val[3])
                << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            for (int i=0; i<4; i++)
                retval.push_back(QString().asprintf("%d", val[i]));
        return retval;
    }
};


//
// --------------------------------- Opacity ----------------------------------
//
class Opacity : public virtual Field
{
  public:
    float val;
  public:
    Opacity(const QString &n, const QString &l) : Field("opacity",n,l) { };
    virtual QString GetCPPName(bool, const QString &)
    {
        return "double";
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val = s.toFloat();
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            retval.push_back(QString().asprintf("%f", val));
        return retval;
    }
};


//
// --------------------------------- LineWidth --------------------------------
//
class LineWidth : public virtual Field
{
  public:
    int  val;
  public:
    LineWidth(const QString &n, const QString &l) : Field("linewidth",n,l) { }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "int";
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val = s.toInt();
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            retval.push_back(QString().asprintf("%d", val));
        return retval;
    }
};


//
// ----------------------------------- VariableName ---------------------------
//
//  Modifications:
//
//    Hank Childs, Tue Jul 19 14:03:23 PDT 2005
//    Add support for arrays.
//
class VariableName : public virtual Field
{
  public:
    QString val;
  public:
    VariableName(const QString &n, const QString &l) : Field("variablename",n,l)
    {
        varTypes = 2; // VAR_CATEGORY_SCALAR
    };

    virtual QString GetCPPName(bool, const QString &)
    {
        return "std::string";
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val = s;
        valueSet = true;
    }

    virtual void SetAttrib(const QString &a, const QString &v)
    {
        if(a == "vartypes")
        {
            if(v.length() != 11)
            {
                cOut << "The vartypes attribute must be 11 characters long!" << Endl;
            }
            else
            {
                int m = 1;
                varTypes = 0;
                for(int i = 0; i < v.length(); ++i)
                {
                    if(v[i] == '1')
                        varTypes |= m;
                    else if(v[i] != '0')
                        cOut << "Bad character in vartypes attribute!" << Endl;
                    m = m << 1;
                }
            }
        }
        else
            cOut << "unknown attribute " << a << " for type " << type << Endl;
    }

    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            retval.push_back(val);
        return retval;
    }
};


//
// ------------------------------------ Att -----------------------------------
//
class Att : public virtual Field
{
  public:
    QString attType;
    QString AttType;
  public:
    Att(const QString &t, const QString &n, const QString &l) : Field("att",n,l)
    {
        attType = t;
        AttType = t;
        AttType[0] = AttType[0].toUpper();
    };
    virtual QString GetSubtype()
    {
        return attType;
    }
    virtual void SetSubtype(const QString &t)
    {
        attType = t;
        AttType = t;
        AttType[0] = AttType[0].toUpper();
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return attType;
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        throw QString("Cannot set value %1 for att type").arg(s);
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        out << "            subtype: " << attType << endl;
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        return retval;
    }
};


//
// --------------------------------- AttVector --------------------------------
//
class AttVector : public virtual Field
{
  public:
    QString attType;
    QString AttType;
  public:
    AttVector(const QString &t, const QString &n, const QString &l) : Field("attVector",n,l)
    {
        attType = t;
        AttType = t;
        AttType[0] = AttType[0].toUpper();
    };
    virtual QString GetSubtype()
    {
        return attType;
    }
    virtual void SetSubtype(const QString &t)
    {
        attType = t;
        AttType = t;
        AttType[0] = AttType[0].toUpper();
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return QString("AttributeGroupVector");
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        throw QString("Cannot set value %1 for attVector type").arg(s);
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        out << "            subtype: " << attType << endl;
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        return retval;
    }
};


//
// --------------------------------- MapNode --------------------------------
//
class MapNode : public virtual Field
{
  public:
    MapNode(const QString &n, const QString &l) : Field("MapNode",n,l)
    {
    };
    virtual QString GetCPPName(bool, const QString &)
    {
        return QString("MapNode");
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        throw QString("Cannot set value %1 for MapNode type").arg(s);
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        return retval;
    }
};


//
// ----------------------------------- Enum -----------------------------------
//
class Enum : public virtual Field
{
  public:
    EnumType *enumType;
    int val;
  public:
    Enum(const QString &t, const QString &n, const QString &l) : Field("enum",n,l)
    {
        enumType = EnumType::FindEnum(t);
    };
    virtual QString GetSubtype()
    {
        return enumType->type;
    }
    virtual QString GetCPPName(bool subtypename=false, const QString &classname="")
    {
        if (subtypename)
        {
            if (classname.isEmpty())
                return enumType->type;
            else
                return classname + "::" + enumType->type;
        }
        else
            return "int";
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val = -1;
        for (size_t i=0; i < enumType->values.size(); i++)
        {
            if (enumType->values[i] == s)
                val = (int)i;
        }
        if (val == -1)
            throw QString("")+"Unknown value for field '"+name+"' of type enum "+enumType->type+": "+s;
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        out << "            subtype: " << enumType->type << endl;
        if (valueSet)
        {
            out << "            value: " << val << " (" << enumType->values[val] << ")" << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            retval.push_back(enumType->values[val]);
        return retval;
    }
};


#define AVT_FIELD_METHODS \
    virtual void SetValue(const QString &s, int = 0)\
    {\
        val = -1;\
        int nsym = 0;\
        const char **sym = GetSymbols(nsym);\
        for(int i = 0; val == -1 && i < nsym; ++i)\
        {\
            if(s == sym[i])\
                val = i;\
        }\
        if(val == -1)\
            val = s.toInt();\
        valueSet = true;\
    }\
    virtual std::vector<QString> GetValueAsText()\
    {\
        std::vector<QString> retval;\
        if (valueSet)\
        {\
            int n = 0;\
            const char **sym = GetSymbols(n);\
            if(val >= 0 && val < n)\
                retval.push_back(sym[val]);\
            else\
                retval.push_back(QString().asprintf("%d", val));\
        }\
        return retval;\
    }\
    virtual void Print(QTextStream &out)\
    {\
        Field::Print(out);\
        if (valueSet)\
        {\
            int n = 0;\
            const char **sym = GetSymbols(n);\
            if(val >= 0 && val < n)\
                out << "            value: " << sym[val] << endl;\
            else\
                out << "            value: " << val << endl;\
        }\
    }


//
// ----------------------- avtCenteringField ----------------------------------
//
class avtCenteringField : public virtual Field
{
  public:
    int  val;
  public:
    avtCenteringField(const QString &n, const QString &l) : Field("avtCentering",n,l)
    {
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "avtCentering";
    }
    virtual const char **GetSymbols(int &n) const
    {
        static const char *symbols[] = {
            "AVT_NODECENT",
            "AVT_ZONECENT",
            "AVT_NO_VARIABLE",
            "AVT_UNKNOWN_CENT"
        };
        n = 4;
        return symbols;
    }
    AVT_FIELD_METHODS
};

//
// ---------------------- avtGhostTypeField -----------------------------------
//
class avtGhostTypeField : public virtual Field
{
  public:
    int  val;
  public:
    avtGhostTypeField(const QString &n, const QString &l) : Field("avtGhostType",n,l)
    {
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "avtGhostType";
    }
    virtual const char **GetSymbols(int &n) const
    {
        static const char *symbols[] = {
            "AVT_NO_GHOSTS",
            "AVT_HAS_GHOSTS",
            "AVT_CREATED_GHOSTS",
            "AVT_MAYBE_GHOSTS"
        };
        n = 4;
        return symbols;
    }
    AVT_FIELD_METHODS
};

//
// --------------------- avtSubsetTypeField -----------------------------------
//
class avtSubsetTypeField : public virtual Field
{
  public:
    int  val;
  public:
    avtSubsetTypeField(const QString &n, const QString &l) : Field("avtSubsetType",n,l)
    {
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "avtSubsetType";
    }
    virtual const char **GetSymbols(int &n) const
    {
        static const char *symbols[] = {
            "AVT_DOMAIN_SUBSET",
            "AVT_GROUP_SUBSET",
            "AVT_MATERIAL_SUBSET",
            "AVT_ENUMSCALAR_SUBSET",
            "AVT_UNKNOWN_SUBSET"
        };
        n = 5;
        return symbols;
    }
    AVT_FIELD_METHODS
};

//
// ------------------------------------ avtVarTypeField -----------------------------------
//
class avtVarTypeField : public virtual Field
{
  public:
    int  val;
  public:
    avtVarTypeField(const QString &n, const QString &l) : Field("avtVarType",n,l)
    {
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "avtVarType";
    }
    virtual const char **GetSymbols(int &n) const
    {
        static const char *symbols[] = {
            "AVT_MESH",
            "AVT_SCALAR_VAR",
            "AVT_VECTOR_VAR",
            "AVT_TENSOR_VAR",
            "AVT_SYMMETRIC_TENSOR_VAR",
            "AVT_ARRAY_VAR",
            "AVT_LABEL_VAR",
            "AVT_MATERIAL",
            "AVT_MATSPECIES",
            "AVT_CURVE",
            "AVT_UNKNOWN_TYPE"
        };
        n = 11;
        return symbols;
    }
    AVT_FIELD_METHODS
};

//
// ----------------------- avtMeshTypeField -----------------------------------
//
class avtMeshTypeField : public virtual Field
{
  public:
    int  val;
  public:
    avtMeshTypeField(const QString &n, const QString &l) : Field("avtMeshType",n,l)
    {
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "avtMeshType";
    }
    virtual const char **GetSymbols(int &n) const
    {
        static const char *symbols[] = {
            "AVT_RECTILINEAR_MESH",
            "AVT_CURVILINEAR_MESH",
            "AVT_UNSTRUCTURED_MESH",
            "AVT_POINT_MESH",
            "AVT_SURFACE_MESH",
            "AVT_CSG_MESH",
            "AVT_AMR_MESH",
            "AVT_UNKNOWN_MESH"
        };
        n = 8;
        return symbols;
    }
    AVT_FIELD_METHODS
};

//
// --------------------- avtExtentTypeField -----------------------------------
//
class avtExtentTypeField : public virtual Field
{
  public:
    int  val;
  public:
    avtExtentTypeField(const QString &n, const QString &l) : Field("avtExtentType",n,l)
    {
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "avtExtentType";
    }
    virtual const char **GetSymbols(int &n) const
    {
        static const char *symbols[] = {
            "AVT_ORIGINAL_EXTENTS",
            "AVT_ACTUAL_EXTENTS",
            "AVT_SPECIFIED_EXTENTS",
            "AVT_UNKNOWN_EXTENT_TYPE"
        };
        n = 4;
        return symbols;
    }
    AVT_FIELD_METHODS
};

//
// ------------------- avtMeshCoordTypeField -----------------------------------
//
class avtMeshCoordTypeField : public virtual Field
{
  public:
    int  val;
  public:
    avtMeshCoordTypeField(const QString &n, const QString &l) : Field("avtMeshCoordType",n,l)
    {
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "avtMeshCoordType";
    }
    virtual const char **GetSymbols(int &n) const
    {
        static const char *symbols[] = {
            "AVT_XY",
            "AVT_RZ",
            "AVT_ZR"
        };
        n = 3;
        return symbols;
    }
    AVT_FIELD_METHODS
};

//
// ------------------ LoadBalanceSchemeField -----------------------------------
//
class LoadBalanceSchemeField : public virtual Field
{
  public:
    int  val;
  public:
    LoadBalanceSchemeField(const QString &n, const QString &l) : Field("LoadBalanceScheme",n,l)
    {
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "LoadBalanceScheme";
    }
    virtual const char **GetSymbols(int &n) const
    {
        static const char *symbols[] = {
            "LOAD_BALANCE_UNKNOWN",
            "LOAD_BALANCE_CONTIGUOUS_BLOCKS_TOGETHER",
            "LOAD_BALANCE_STRIDE_ACROSS_BLOCKS",
            "LOAD_BALANCE_RANDOM_ASSIGNMENT",
            "LOAD_BALANCE_DBPLUGIN_DYNAMIC",
            "LOAD_BALANCE_RESTRICTED",
            "LOAD_BALANCE_ABSOLUTE"
        };
        n = 7;
        return symbols;
    }
    AVT_FIELD_METHODS
};

//
// --------------------------------- ScaleMode --------------------------------
//
class ScaleMode : public virtual Field
{
  public:
    int  val;
  public:
    ScaleMode(const QString &n, const QString &l) : Field("scalemode",n,l) { }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "int";
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val = s.toInt();
        valueSet = true;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
            retval.push_back(QString().asprintf("%d", val));
        return retval;
    }
};


//
// --------------------------------- GlyphType --------------------------------
//
class GlyphType : public virtual Field
{
  public:
    int  val;
    int  nvals;
  public:
    GlyphType(const QString &n, const QString &l) : Field("glyphtype",n,l)
    {
        nvals = 8;
    }
    virtual QString GetCPPName(bool, const QString &)
    {
        return "GlyphType";
    }
    virtual const char **GetSymbols(int &n) const
    {
        static const char *symbols[] = {
            "Box",
            "Axis",
            "Icosahedron",
            "Octahedron",
            "Tetrahedron",
            "SphereGeometry",
            "Point",
            "Sphere"
        };
        n = nvals;
        return symbols;
    }
    virtual int GetNValues() const
    {
        return nvals;
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val = -1;
        int nsym = 0;
        const char **sym = GetSymbols(nsym);
        for(int i = 0; val == -1 && i < nsym; ++i)
        {
            if(s == sym[i])
                val = i;
        }
        if(val == -1)
            val = s.toInt();
        valueSet = true;
    }
    virtual std::vector<QString> GetValueAsText()
    {
        std::vector<QString> retval;
        if (valueSet)
        {
            int n = 0;
            const char **sym = GetSymbols(n);
            if(val >= 0 && val < n)
                retval.push_back(sym[val]);
            else
                retval.push_back(QString().asprintf("%d", val));
        }
        return retval;
    }
    virtual void Print(QTextStream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            int n = 0;
            const char **sym = GetSymbols(n);
            if(val >= 0 && val < n)
                out << "            value: " << sym[val] << endl;
            else
                out << "            value: " << val << endl;
        }
    }

};

// ****************************************************************************
//  Class:  FieldFactory
//
//  Purpose:
//    Creates a field of the right type on demand.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Dec 9 11:18:31 PDT 2002
//    Added UCharVector.
//
//    Brad Whitlock, Wed Dec 8 15:41:27 PST 2004
//    Added VariableName.
//
//    Brad Whitlock, Fri Mar 2 14:31:41 PST 2007
//    Added some built-in AVT enums.
//
//    Kathleen Bonnell, Thu Mar 22 16:52:37 PDT 2007
//    Added scalemode.
//
// ****************************************************************************

class FieldFactory
{
  public:
    static Field *createField(const QString &name,
                              const QString &type,
                              const QString &subtype,
                              const QString &length,
                              const QString &label)
    {
        Field *f = NULL;
        if      (type.isNull())          throw QString("Field %1 was specified with no type.").arg(name);
        else if (type == "int")          f = new Int(name,label);
        else if (type == "intArray")     f = new IntArray(length,name,label);
        else if (type == "intVector")    f = new IntVector(name,label);
        else if (type == "bool")         f = new Bool(name,label);
        else if (type == "float")        f = new Float(name,label);
        else if (type == "floatArray")   f = new FloatArray(length,name,label);
        else if (type == "floatVector")  f = new FloatVector(name,label);
        else if (type == "double")       f = new Double(name,label);
        else if (type == "doubleArray")  f = new DoubleArray(length,name,label);
        else if (type == "doubleVector") f = new DoubleVector(name,label);
        else if (type == "uchar")        f = new UChar(name,label);
        else if (type == "ucharArray")   f = new UCharArray(length,name,label);
        else if (type == "ucharVector")  f = new UCharVector(name,label);
        else if (type == "string")       f = new String(name,label);
        else if (type == "stringVector") f = new StringVector(name,label);
        else if (type == "colortable")   f = new ColorTable(name,label);
        else if (type == "color")        f = new Color(name,label);
        else if (type == "opacity")      f = new Opacity(name,label);
        else if (type == "linewidth")    f = new LineWidth(name,label);
        else if (type == "variablename") f = new VariableName(name,label);
        else if (type == "att")          f = new Att(subtype,name,label);
        else if (type == "attVector")    f = new AttVector(subtype,name,label);
        else if (type == "enum")         f = new Enum(subtype, name, label);
        else if (type == "scalemode")    f = new ScaleMode(name,label);
        else if (type == "MapNode")      f = new MapNode(name,label);
        else if (type == "glyphtype")    f = new GlyphType(name,label);

        // Special built-in AVT enums
        else if (type == "avtCentering")      f = new avtCenteringField(name, label);
        else if (type == "avtVarType")        f = new avtVarTypeField(name, label);
        else if (type == "avtSubsetType")     f = new avtSubsetTypeField(name, label);
        else if (type == "avtExtentType")     f = new avtExtentTypeField(name, label);
        else if (type == "avtMeshType")       f = new avtMeshTypeField(name, label);
        else if (type == "avtGhostType")      f = new avtGhostTypeField(name, label);
        else if (type == "avtMeshCoordType")  f = new avtMeshCoordTypeField(name, label);
        else if (type == "LoadBalanceScheme") f = new LoadBalanceSchemeField(name, label);

        if (!f)
            throw QString("FieldFactory: unknown type for field %1: %2").arg(name).arg(type);

        return f;
    }
};

#endif
