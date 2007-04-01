#ifndef FIELD_H
#define FIELD_H

#include <qstring.h>
#include <visitstream.h>
#include <stdio.h>
#include "Enum.h"
#include "XMLParserUtil.h"
#include "CodeFile.h"

#if defined(_WIN32) && defined(USING_MSVC6)
// Turn off warnings about inheritance via dominance.
#pragma warning(disable:4250)
#endif

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

    bool              isArray;
    bool              isVector;

    bool              valueSet;

    bool              ignoreEquality;

    Field            *enabler;
    vector<QString>   enableval;

    QString           initcode;

    int               index;
    CodeFile         *codeFile;
  public:
    Field(const QString &t, const QString &n, const QString &l) 
        : type(t), name(n), label(l) 
    {
        codeFile = NULL;
        internal = false;
        valueSet = false;
        enabler = NULL;
        if (label.isNull())
            label = name;
        Name = name;
        Name[0] = Name[0].upper();
        length = 1;
        isArray  = (type.right(5) == "Array");
        isVector = (type.right(6) == "Vector");
        ignoreEquality = false;
    }
    virtual ~Field() { }
    virtual void ClearValues() { }
    void CopyValues(Field *f)
    {
        internal = f->internal;
        enabler = f->enabler;
        enableval = f->enableval;
        index = f->index;
        codeFile = f->codeFile;
    }
    void SetInitCode(const QString &ic)
    {
        initcode = ic;
    }
    virtual QString GetSubtype() { return ""; };
    virtual void SetSubtype(const QString &) {  };
    virtual void SetValue(const QString &s, int ix=0) = 0;
    virtual QString GetCPPName(bool subtypename=false, const QString &classname="") = 0;
    virtual void SetAttrib(const QString &a, const QString &v)
    {
        cout << "unknown attribute " << a << " for type " << type << endl;
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
    virtual void Print(ostream &out)
    {
        char s[1000];
        char s1[1000];
        sprintf(s1,"[%s]", type.latin1());
        sprintf(s, "        Field %-15s: %-15s  (\"%s\")", s1, name.latin1(), label.latin1());
        out << s << endl;
        if (internal)
            cout << "            (INTERNAL)" << endl;
        if (enabler)
        {
            cout << "            enabled when " << enabler->name << " equals ";
            cout << enableval[0];
            for (int i=1; i<enableval.size(); i++)
            {
                cout << " or " << enableval[i];
            }
            cout << endl;
        }
    }
    virtual vector<QString> GetValueAsText() = 0;
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
            vector<QString> minmax = SplitValues(v);
            min = minmax[0].toInt();
            max = minmax[1].toInt();
            rangeSet = true;
        }
        else
            Field::SetAttrib(a,v);
    }
    virtual void Print(ostream &out)
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
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            retval.push_back(QString().sprintf("%d", val));
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
    virtual QString GetCPPName(bool, const QString &) 
    {
        return "int";
    }
    virtual void SetValue(const QString &s, int ix = 0)
    {
        val[ix] = s.toInt();
        valueSet = true;
    }
    virtual void Print(ostream &out)
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
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            for (int i=0; i<length; i++)
                retval.push_back(QString().sprintf("%d", val[i]));
        return retval;
    }
};


//
// --------------------------------- IntVector --------------------------------
//
class IntVector : public virtual Field
{
  public:
    vector<int> val;
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
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: ";
            for (int i=0; i<val.size(); i++)
                out << val[i] << "  ";
            out << endl;
        }
    }
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            for (int i=0; i<val.size(); i++)
                retval.push_back(QString().sprintf("%d", val[i]));
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
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << (val ? "true" : "false") << endl;
        }
    }
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
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
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
    }
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            retval.push_back(QString().sprintf("%f", val));
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
    virtual QString GetCPPName(bool, const QString &) 
    {
        return "float";
    }
    virtual void SetValue(const QString &s, int ix = 0)
    {
        val[ix] = s.toFloat();
        valueSet = true;
    }
    virtual void Print(ostream &out)
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
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            for (int i=0; i<length; i++)
                retval.push_back(QString().sprintf("%f", val[i]));
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
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
    }
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            retval.push_back(QString().sprintf("%f", val));
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
    virtual QString GetCPPName(bool, const QString &) 
    {
        return "double";
    }
    virtual void SetValue(const QString &s, int ix = 0)
    {
        val[ix] = s.toDouble();
        valueSet = true;
    }
    virtual void Print(ostream &out)
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
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            for (int i=0; i<length; i++)
                retval.push_back(QString().sprintf("%f", val[i]));
        return retval;
    }
};


//
// ------------------------------- DoubleVector -------------------------------
//
class DoubleVector : public virtual Field
{
  public:
    vector<double> val;
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
        val.push_back(s.toDouble());
        valueSet = true;
    }
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: ";
            for (int i=0; i<val.size(); i++)
                out << val[i] << "  ";
            out << endl;
        }
    }
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            for (int i=0; i<val.size(); i++)
                retval.push_back(QString().sprintf("%f", val[i]));
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
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << ios::hex << int(val) << ios::dec << endl;
        }
    }
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            retval.push_back(QString().sprintf("%d", val));
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
    virtual QString GetCPPName(bool, const QString &) 
    {
        return "unsigned char";
    }
    virtual void SetValue(const QString &s, int ix = 0)
    {
        val[ix] = s.toInt();
        valueSet = true;
    }
    virtual void Print(ostream &out)
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
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            for (int i=0; i<length; i++)
                retval.push_back(QString().sprintf("%d", val[i]));
        return retval;
    }
};


//
// ------------------------------- UCharVector -------------------------------
//
class UCharVector : public virtual Field
{
  public:
    vector<unsigned char> val;
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
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: ";
            for (int i=0; i<val.size(); i++)
                out << int(val[i]) << "  ";
            out << endl;
        }
    }
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            for (int i=0; i<val.size(); i++)
                retval.push_back(QString().sprintf("%d", int(val[i])));
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
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
    }
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
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
    vector<QString> val;
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
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: ";
            for (int i=0; i<val.size(); i++)
                out << "\"" << val[i] << "\"  ";
            out << endl;
        }
    }
    virtual vector<QString> GetValueAsText()
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
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
    }
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
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
    Color(const QString &n, const QString &l) : Field("color",n,l) { };
    virtual QString GetCPPName(bool, const QString &) 
    {
        return "ColorAttribute";
    }
    virtual void SetValue(const QString &s, int ix = 0)
    {
        val[ix] = s.toInt();
        valueSet = true;
    }
    virtual void Print(ostream &out)
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
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            for (int i=0; i<4; i++)
                retval.push_back(QString().sprintf("%d", val[i]));
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
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
    }
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            retval.push_back(QString().sprintf("%f", val));
        return retval;
    }
};


//
// --------------------------------- LineStyle --------------------------------
//
class LineStyle : public virtual Field
{
  public:
    int  val;
  public:
    LineStyle(const QString &n, const QString &l) : Field("linestyle",n,l) { }
    virtual QString GetCPPName(bool, const QString &) 
    {
        return "int";
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        val = s.toInt();
        valueSet = true;
    }
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
    }
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            retval.push_back(QString().sprintf("%d", val));
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
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        if (valueSet)
        {
            out << "            value: " << val << endl;
        }
    }
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            retval.push_back(QString().sprintf("%d", val));
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
        AttType[0] = AttType[0].upper();
    };
    virtual QString GetSubtype()
    {
        return attType;
    }
    virtual void SetSubtype(const QString &t)
    {
        attType = t;
        AttType = t;
        AttType[0] = AttType[0].upper();
    }
    virtual QString GetCPPName(bool, const QString &) 
    {
        return attType;
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        throw QString().sprintf("Cannot set value %s for att type", s.latin1());
    }
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        out << "            subtype: " << attType << endl;
    }
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
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
        AttType[0] = AttType[0].upper();
    };
    virtual QString GetSubtype()
    {
        return attType;
    }
    virtual void SetSubtype(const QString &t)
    {
        attType = t;
        AttType = t;
        AttType[0] = AttType[0].upper();
    }
    virtual QString GetCPPName(bool, const QString &) 
    {
        return QString("AttributeGroupVector");
    }
    virtual void SetValue(const QString &s, int = 0)
    {
        throw QString().sprintf("Cannot set value %s for attVector type", s.latin1());
    }
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        out << "            subtype: " << attType << endl;
    }
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
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
        for (int i=0; i < enumType->values.size(); i++)
        {
            if (enumType->values[i] == s)
                val = i;
        }
        if (val == -1)
            throw QString("")+"Unknown value for field '"+name+"' of type enum "+enumType->type+": "+s;
        valueSet = true;
    }
    virtual void Print(ostream &out)
    {
        Field::Print(out);
        out << "            subtype: " << enumType->type << endl;
        if (valueSet)
        {
            out << "            value: " << val << " (" << enumType->values[val] << ")" << endl;
        }
    }
    virtual vector<QString> GetValueAsText()
    {
        vector<QString> retval;
        if (valueSet)
            retval.push_back(enumType->values[val]);
        return retval;
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
        if      (type.isNull())          throw QString().sprintf("Field %s was specified with no type.",name.latin1());
        else if (type == "int")          f = new Int(name,label);
        else if (type == "intArray")     f = new IntArray(length,name,label);
        else if (type == "intVector")    f = new IntVector(name,label);
        else if (type == "bool")         f = new Bool(name,label);
        else if (type == "float")        f = new Float(name,label);
        else if (type == "floatArray")   f = new FloatArray(length,name,label);
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
        else if (type == "linestyle")    f = new LineStyle(name,label);
        else if (type == "linewidth")    f = new LineWidth(name,label);
        else if (type == "att")          f = new Att(subtype,name,label);
        else if (type == "attVector")    f = new AttVector(subtype,name,label);
        else if (type == "enum")         f = new Enum(subtype, name, label);

        if (!f)
            throw QString().sprintf("FieldFactory: unknown type for field %s: %s",name.latin1(),type.latin1());

        return f;
    }
};

#endif
