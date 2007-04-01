#ifndef GENERATE_WINDOW_H
#define GENERATE_WINDOW_H
#if !defined(_WIN32)
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <visit-config.h>
#ifdef TIME_WITH_SYS_TIME
#include <time.h>
#endif

#include "Field.h"

// ****************************************************************************
//  File:  GenerateWindow
//
//  Purpose:
//    Contains a set of classes which override the default implementation
//    to create a window.
//
//  Note: This file overrides --
//    FieldFactory
//    Field
//    Attribute
//    Enum
//    Plugin
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Mon Sep 24 17:36:08 PDT 2001
//    Added FloatArray.
//
//    Jeremy Meredith, Sat Jan  5 17:33:03 PST 2002
//    Added #include <string> and using std::string to the source file.
//
//    Jeremy Meredith, Fri Apr 19 15:25:46 PDT 2002
//    Added constants, includes, and a codefile.  Made the HasFunction,
//    PrintFunction, DeleteFunction all check to make sure we are trying
//    to override a builtin function.  Removed "enabler".
//
//    Jeremy Meredith, Fri May 31 11:09:58 PDT 2002
//    Renamed CreateWindow to CreateWindowContents.
//    Added classname to the writeSourceCallback and made Enums do the
//    appropriate cast from an int.
//
//    Jeremy Meredith, Fri Jul 12 16:28:28 PDT 2002
//    Some portability improvements.
//
//    Jeremy Meredith, Tue Aug 27 14:32:50 PDT 2002
//    Added mfiles, dbtype, and libs.  Allowed NULL atts.
//
//    Jeremy Meredith, Thu Oct 17 15:58:29 PDT 2002
//    Added some enhancements for the XML editor.
//
//    Jeremy Meredith, Tue Sep 23 16:59:03 PDT 2003
//    Added ability for enablers to dis/enable a widget's associated label.
//    Made haswriter be a bool.
//
//    Jeremy Meredith, Wed Nov  5 13:28:03 PST 2003
//    Added ability to disable plugins by default.
//
//    Jeremy Meredith, Tue Jun 15 10:39:32 PDT 2004
//    Added missing blockSignals wrapper in opacity's UpdateWindow.
//
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Kathleen Bonnell, Thu Nov 11 16:56:21 PST 2004 
//    Added support for ucharVector.
//
//    Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//    Replaced simple QString::sprintf's with a setNum because there seems
//    to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//    Brad Whitlock, Wed Dec 8 15:56:14 PST 2004
//    Added support for variable names.
//
// ****************************************************************************

class WindowGeneratorField : public virtual Field
{
  public:
    WindowGeneratorField(const QString &t, const QString &n, const QString &l)
        : Field(t,n,l)
    {
    }
    // helper functions
    void writeSourceCreateLabel(ostream &c)
    {
        c << "    "<<name<<"Label = new QLabel(\""<<label<<"\", central, \""<<name<<"Label\");" << endl;
        c << "    mainLayout->addWidget("<<name<<"Label,"<<index<<",0);" << endl;
    }
    // virtual functions
    virtual void               writeHeaderCallback(ostream &h)
    {
        h << "    //writeHeaderCallback unknown for " << type << " (variable " << name << ")" << endl;
    }
    virtual void               writeHeaderLabelData(ostream &h)
    {
        h << "    QLabel *"<<name<<"Label;" << endl;
    }
    virtual void               writeHeaderData(ostream &h)
    {
        h << "    //writeHeaderData unknown for " << type << " (variable " << name << ")" << endl;
    }
    virtual void               writeSourceCreate(ostream &c)
    {
        c << "    //writeSourceCreate unknown for " << type << " (variable " << name << ")" << endl;
    }
    virtual void               writeSourceGetCurrent(ostream &c)
    {
        c << "        //writeSourceGetCurrent unknown for " << type << " (variable " << name << ")" << endl;
    }
    virtual void               writeSourceUpdateWindow(ostream &c)
    {
        c << "        //writeSourceUpdate unknown for " << type << " (variable " << name << ")" << endl;
    }
    virtual void               writeSourceCallback(QString &classname, QString &windowname, ostream &c)
    {
        c << "//writeSourceCallback unknown for " << type << " (variable " << name << ")" << endl;
    }
};

//
// ------------------------------------ Int -----------------------------------
//
class WindowGeneratorInt : public virtual Int , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorInt(const QString &n, const QString &l)
        : Int(n,l), WindowGeneratorField("int",n,l), Field("int",n,l) { }
    virtual void            writeHeaderCallback(ostream &h)
    {
        if (rangeSet)
            h << "    void "<<name<<"Changed(int val);" << endl;
        else
            h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(ostream &h)
    {
        if (rangeSet)
            h << "    QSpinBox *"<<name<<";" << endl;
        else
            h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ostream &c)
    {
        writeSourceCreateLabel(c);
        if (rangeSet)
        {
            c << "    "<<name<<" = new QSpinBox("<<min<<", "<<max<<", 1, central, \""<<name<<"\");" << endl;
            c << "    connect("<<name<<", SIGNAL(valueChanged(int)), " << endl
              << "            this, SLOT("<<name<<"Changed(int)));" << endl;
        }
        else
        {
            c << "    "<<name<<" = new QLineEdit(central, \""<<name<<"\");" << endl;
            c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
              << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        }
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ostream &c)
    {
        if (rangeSet)
        {
            c << "        // Nothing for " << name << endl;
        }
        else
        {
            c << "        temp = "<<name<<"->displayText().simplifyWhiteSpace();" << endl;
            c << "        okay = !temp.isEmpty();" << endl;
            c << "        if(okay)" << endl;
            c << "        {" << endl;
            c << "            int val = temp.toInt(&okay);" << endl;
            c << "            atts->Set"<<Name<<"(val);" << endl;
            c << "        }" << endl;
            c << endl;
            c << "        if(!okay)" << endl;
            c << "        {" << endl;
            c << "            msg.sprintf(\"The value of "<<name<<" was invalid. \"" << endl;
            c << "                \"Resetting to the last good value of %d.\"," << endl;
            c << "                atts->Get"<<Name<<"());" << endl;
            c << "            Message(msg);" << endl;
            c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << endl;
            c << "        }" << endl;
        }
    }
    virtual void            writeSourceUpdateWindow(ostream &c)
    {
        if (rangeSet)
        {
            c << "            "<<name<<"->setValue(atts->Get"<<Name<<"());" << endl;
        }
        else
        {
            c << "            temp.sprintf(\"%d\", atts->Get"<<Name<<"());" << endl;
            c << "            "<<name<<"->setText(temp);" << endl;
        }
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, ostream &c)
    {
        if (rangeSet)
        {
            c << "void" << endl;
            c << windowname<<"::"<<name<<"Changed(int val)" << endl;
            c << "{" << endl;
            c << "    atts->Set"<<Name<<"(val);" << endl;
            c << "    Apply();" << endl;
            c << "}" << endl;
        }
        else
        {
            c << "void" << endl;
            c << windowname<<"::"<<name<<"ProcessText()" << endl;
            c << "{" << endl;
            c << "    GetCurrentValues("<<index<<");" << endl;
            c << "    Apply();" << endl;
            c << "}" << endl;
        }
    }
};


//
// ---------------------------------- IntArray --------------------------------
//
class WindowGeneratorIntArray : public virtual IntArray , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorIntArray(const QString &s, const QString &n, const QString &l)
        : IntArray(s,n,l), WindowGeneratorField("intArray",n,l), Field("intArray",n,l) { }
};


//
// --------------------------------- IntVector --------------------------------
//
class WindowGeneratorIntVector : public virtual IntVector , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorIntVector(const QString &n, const QString &l)
        : IntVector(n,l), WindowGeneratorField("intVector",n,l), Field("intVector",n,l) { }
};


// 
// ----------------------------------- Bool -----------------------------------
//
class WindowGeneratorBool : public virtual Bool , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorBool(const QString &n, const QString &l)
        : Bool(n,l), WindowGeneratorField("bool",n,l), Field("bool",n,l) { }
    virtual void            writeHeaderCallback(ostream &h)
    {
        h << "    void "<<name<<"Changed(bool val);" << endl;
    }
    virtual void            writeHeaderData(ostream &h)
    {
        h << "    QCheckBox *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ostream &c)
    {
        c << "    "<<name<<" = new QCheckBox(\""<<label<<"\", central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(toggled(bool))," << endl
          << "            this, SLOT("<<name<<"Changed(bool)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",0);" << endl;
    }
    virtual void            writeSourceGetCurrent(ostream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(ostream &c)
    {
        c << "            "<<name<<"->setChecked(atts->Get"<<Name<<"());" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, ostream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(bool val)" << endl;
        c << "{" << endl;
        c << "    atts->Set"<<Name<<"(val);" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


//
// ----------------------------------- Float ----------------------------------
//
class WindowGeneratorFloat : public virtual Float , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorFloat(const QString &n, const QString &l)
        : Float(n,l), WindowGeneratorField("float",n,l), Field("float",n,l) { }
    virtual void            writeHeaderCallback(ostream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(ostream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ostream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ostream &c)
    {
        c << "        temp = "<<name<<"->displayText().simplifyWhiteSpace();" << endl;
        c << "        okay = !temp.isEmpty();" << endl;
        c << "        if(okay)" << endl;
        c << "        {" << endl;
        c << "            float val = temp.toFloat(&okay);" << endl;
        c << "            atts->Set"<<Name<<"(val);" << endl;
        c << "        }" << endl;
        c << endl;
        c << "        if(!okay)" << endl;
        c << "        {" << endl;
        c << "            msg.sprintf(\"The value of "<<name<<" was invalid. \"" << endl;
        c << "                \"Resetting to the last good value of %g.\"," << endl;
        c << "                atts->Get"<<Name<<"());" << endl;
        c << "            Message(msg);" << endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdateWindow(ostream &c)
    {
        c << "            temp.setNum(atts->Get"<<Name<<"());" << endl;
        c << "            "<<name<<"->setText(temp);" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, ostream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<index<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


//
// -------------------------------- FloatArray -------------------------------
//
class WindowGeneratorFloatArray : public virtual FloatArray , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorFloatArray(const QString &s, const QString &n, const QString &l)
        : FloatArray(s,n,l), WindowGeneratorField("floatArray",n,l), Field("floatArray",n,l) { }
    virtual void               writeHeaderCallback(ostream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void               writeHeaderData(ostream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void               writeSourceCreate(ostream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void               writeSourceGetCurrent(ostream &c)
    {
        c << "        temp = "<<name<<"->displayText().simplifyWhiteSpace();" << endl;
        c << "        okay = !temp.isEmpty();" << endl;
        c << "        if(okay)" << endl;
        c << "        {" << endl;
        c << "            float val["<<length<<"];" << endl;
        c << "            sscanf(temp.latin1(), \"";
        int i;
        for (i=0; i<length; i++)
        {
            c << "%g";
            if (i < length-1) c << " ";
        }
        c << "\", ";
        for (i=0; i<length; i++)
        {
            c << "&val["<<i<<"]";
            if (i < length-1) c << ", ";
        }
        c << ");" << endl;
        c << "            atts->Set"<<Name<<"(val);" << endl;
        c << "        }" << endl;
        c << endl;
        c << "        if(!okay)" << endl;
        c << "        {" << endl;
        c << "            const float *val = atts->Get"<<Name<<"();" << endl;
        c << "            msg.sprintf(\"The value of "<<name<<" was invalid. \"" << endl;
        c << "                \"Resetting to the last good value of <";
        for (i=0; i<length; i++)
        {
            c << "%g";
            if (i < length-1) c << " ";
        }
        c << ">\", " << endl;
        c << "                ";
        for (i=0; i<length; i++)
        {
            c << "val["<<i<<"]";
            if (i < length-1) c << ", ";
        }
        c << ");" << endl;
        c << "            Message(msg);" << endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void               writeSourceUpdateWindow(ostream &c)
    {
        c << "            fptr = atts->Get"<<Name<<"();" << endl;
        c << "            temp.sprintf(\"";
        int i;
        for (i=0; i<length; i++)
        {
            c << "%g";
            if (i<length-1) c << " ";
        }
        c << "\", ";
        for (i=0; i<length; i++)
        {
            c << "fptr["<<i<<"]";
            if (i<length-1) c << ", ";
        }
        c << ");" << endl;
        c << "            "<<name<<"->setText(temp);" << endl;
    }
    virtual void               writeSourceCallback(QString &, QString &windowname, ostream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<index<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


//
// ---------------------------------- Double ----------------------------------
//
class WindowGeneratorDouble : public virtual Double , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorDouble(const QString &n, const QString &l)
        : Double(n,l), WindowGeneratorField("double",n,l), Field("double",n,l) { }
    virtual void            writeHeaderCallback(ostream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(ostream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ostream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ostream &c)
    {
        c << "        temp = "<<name<<"->displayText().simplifyWhiteSpace();" << endl;
        c << "        okay = !temp.isEmpty();" << endl;
        c << "        if(okay)" << endl;
        c << "        {" << endl;
        c << "            double val = temp.toDouble(&okay);" << endl;
        c << "            atts->Set"<<Name<<"(val);" << endl;
        c << "        }" << endl;
        c << endl;
        c << "        if(!okay)" << endl;
        c << "        {" << endl;
        c << "            msg.sprintf(\"The value of "<<name<<" was invalid. \"" << endl;
        c << "                \"Resetting to the last good value of %g.\"," << endl;
        c << "                atts->Get"<<Name<<"());" << endl;
        c << "            Message(msg);" << endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdateWindow(ostream &c)
    {
        c << "            temp.setNum(atts->Get"<<Name<<"());" << endl;
        c << "            "<<name<<"->setText(temp);" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, ostream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<index<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


//
// -------------------------------- DoubleArray -------------------------------
//
class WindowGeneratorDoubleArray : public virtual DoubleArray , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorDoubleArray(const QString &s, const QString &n, const QString &l)
        : DoubleArray(s,n,l), WindowGeneratorField("doubleArray",n,l), Field("doubleArray",n,l) { }
    virtual void               writeHeaderCallback(ostream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void               writeHeaderData(ostream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void               writeSourceCreate(ostream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void               writeSourceGetCurrent(ostream &c)
    {
        c << "        temp = "<<name<<"->displayText().simplifyWhiteSpace();" << endl;
        c << "        okay = !temp.isEmpty();" << endl;
        c << "        if(okay)" << endl;
        c << "        {" << endl;
        c << "            double val["<<length<<"];" << endl;
        c << "            sscanf(temp.latin1(), \"";
        int i;
        for (i=0; i<length; i++)
        {
            c << "%lg";
            if (i < length-1) c << " ";
        }
        c << "\", ";
        for (i=0; i<length; i++)
        {
            c << "&val["<<i<<"]";
            if (i < length-1) c << ", ";
        }
        c << ");" << endl;
        c << "            atts->Set"<<Name<<"(val);" << endl;
        c << "        }" << endl;
        c << endl;
        c << "        if(!okay)" << endl;
        c << "        {" << endl;
        c << "            const double *val = atts->Get"<<Name<<"();" << endl;
        c << "            msg.sprintf(\"The value of "<<name<<" was invalid. \"" << endl;
        c << "                \"Resetting to the last good value of <";
        for (i=0; i<length; i++)
        {
            c << "%g";
            if (i < length-1) c << " ";
        }
        c << ">\", " << endl;
        c << "                ";
        for (i=0; i<length; i++)
        {
            c << "val["<<i<<"]";
            if (i < length-1) c << ", ";
        }
        c << ");" << endl;
        c << "            Message(msg);" << endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void               writeSourceUpdateWindow(ostream &c)
    {
        c << "            dptr = atts->Get"<<Name<<"();" << endl;
        c << "            temp.sprintf(\"";
        int i;
        for (i=0; i<length; i++)
        {
            c << "%g";
            if (i<length-1) c << " ";
        }
        c << "\", ";
        for (i=0; i<length; i++)
        {
            c << "dptr["<<i<<"]";
            if (i<length-1) c << ", ";
        }
        c << ");" << endl;
        c << "            "<<name<<"->setText(temp);" << endl;
    }
    virtual void               writeSourceCallback(QString &, QString &windowname, ostream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<index<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


//
// -------------------------------- DoubleVector ------------------------------
//
class WindowGeneratorDoubleVector : public virtual DoubleVector , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorDoubleVector(const QString &n, const QString &l)
        : DoubleVector(n,l), WindowGeneratorField("doubleVector",n,l), Field("doubleVector",n,l) { }
};


//
// ----------------------------------- UChar ----------------------------------
//
class WindowGeneratorUChar : public virtual UChar , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorUChar(const QString &n, const QString &l)
        : UChar(n,l), WindowGeneratorField("uchar",n,l), Field("uchar",n,l) { }
};


//
// -------------------------------- UCharArray --------------------------------
//
class WindowGeneratorUCharArray : public virtual UCharArray , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorUCharArray(const QString &s, const QString &n, const QString &l)
        : UCharArray(s,n,l), WindowGeneratorField("ucharArray",n,l), Field("ucharArray",n,l) { }
};

//
// -------------------------------- UCharVector --------------------------------
//
class WindowGeneratorUCharVector : public virtual UCharVector , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorUCharVector(const QString &n, const QString &l)
        : UCharVector(n,l), WindowGeneratorField("ucharVector",n,l), Field("ucharVector",n,l) { }
};


//
// ---------------------------------- String ----------------------------------
//
class WindowGeneratorString : public virtual String , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorString(const QString &n, const QString &l)
        : String(n,l), WindowGeneratorField("string",n,l), Field("string",n,l) { }
    virtual void            writeHeaderCallback(ostream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(ostream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ostream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ostream &c)
    {
        c << "        temp = "<<name<<"->displayText();" << endl;
        c << "        okay = !temp.isEmpty();" << endl;
        c << "        if(okay)" << endl;
        c << "        {" << endl;
        c << "            atts->Set"<<Name<<"(temp.latin1());" << endl;
        c << "        }" << endl;
        c << endl;
        c << "        if(!okay)" << endl;
        c << "        {" << endl;
        c << "            msg.sprintf(\"The value of "<<name<<" was invalid. \"" << endl;
        c << "                \"Resetting to the last good value of %s.\"," << endl;
        c << "                atts->Get"<<Name<<"().c_str());" << endl;
        c << "            Message(msg);" << endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdateWindow(ostream &c)
    {
        c << "            temp = atts->Get"<<Name<<"().c_str();" << endl;
        c << "            "<<name<<"->setText(temp);" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, ostream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<index<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


//
// ------------------------------- StringVector -------------------------------
//
class WindowGeneratorStringVector : public virtual StringVector , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorStringVector(const QString &n, const QString &l)
        : StringVector(n,l), WindowGeneratorField("stringVector",n,l), Field("stringVector",n,l) { }
};


//
// -------------------------------- ColorTable --------------------------------
//
class WindowGeneratorColorTable : public virtual ColorTable , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorColorTable(const QString &n, const QString &l)
        : ColorTable(n,l), WindowGeneratorField("colortable",n,l), Field("colortable",n,l) { }
    virtual void            writeHeaderCallback(ostream &h)
    {
        h << "    void "<<name<<"Changed(bool useDefault, const QString &ctName);" << endl;
    }
    virtual void            writeHeaderData(ostream &h)
    {
        h << "    QvisColorTableButton *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ostream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QvisColorTableButton(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(selectedColorTable(bool, const QString&))," << endl
          << "            this, SLOT("<<name<<"Changed(bool, const QString&)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ostream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(ostream &c)
    {
        c << "            "<<name<<"->setColorTable(atts->Get"<<Name<<"().c_str());" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, ostream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(bool useDefault, const QString &ctName)" << endl;
        c << "{" << endl;
        c << "    atts->Set"<<Name<<"(ctName.latin1());" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


//
// ----------------------------------- Color ----------------------------------
//
class WindowGeneratorColor : public virtual Color , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorColor(const QString &n, const QString &l)
        : Color(n,l), WindowGeneratorField("color",n,l), Field("color",n,l) { }
    virtual void            writeHeaderCallback(ostream &h)
    {
        h << "    void "<<name<<"Changed(const QColor &color);" << endl;
    }
    virtual void            writeHeaderData(ostream &h)
    {
        h << "    QvisColorButton *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ostream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QvisColorButton(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(selectedColor(const QColor&))," << endl
          << "            this, SLOT("<<name<<"Changed(const QColor&)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ostream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(ostream &c)
    {
        c << "            tempcolor = QColor(atts->Get"<<Name<<"().Red()," << endl;
        c << "                               atts->Get"<<Name<<"().Green()," << endl;
        c << "                               atts->Get"<<Name<<"().Blue());" << endl;
        //c << "            "<<name<<"->blockSignals(true);" << endl;
        c << "            "<<name<<"->setButtonColor(tempcolor);" << endl;
        //c << "            "<<name<<"->blockSignals(false);" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, ostream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(const QColor &color)" << endl;
        c << "{" << endl;
        c << "    ColorAttribute temp(color.red(), color.green(), color.blue());" << endl;
        c << "    atts->Set"<<Name<<"(temp);" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


//
// --------------------------------- Opacity ----------------------------------
//
class WindowGeneratorOpacity : public virtual Opacity , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorOpacity(const QString &n, const QString &l)
        : Opacity(n,l), WindowGeneratorField("opacity",n,l), Field("opacity",n,l) { }
    virtual void            writeHeaderCallback(ostream &h)
    {
        h << "    void "<<name<<"Changed(int opacity, const void*);" << endl;
    }
    virtual void            writeHeaderData(ostream &h)
    {
        h << "    QvisOpacitySlider *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ostream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QvisOpacitySlider(0,255,25,255, central, \""<<name<<"\", NULL);" << endl;
        c << "    "<<name<<"->setTickInterval(64);" << endl;
        c << "    "<<name<<"->setGradientColor(QColor(0, 0, 0));" << endl;
        c << "    connect("<<name<<", SIGNAL(valueChanged(int, const void*))," << endl
          << "            this, SLOT("<<name<<"Changed(int, const void*)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ostream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(ostream &c)
    {
        c << "            "<<name<<"->blockSignals(true);" << endl;
        c << "            "<<name<<"->setValue(int(atts->Get"<<Name<<"()*255.));" << endl;
        c << "            "<<name<<"->blockSignals(false);" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, ostream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(int opacity, const void*)" << endl;
        c << "{" << endl;
        c << "    atts->Set"<<Name<<"((float)opacity/255.);" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


//
// -------------------------------- LineStyle --------------------------------
//
class WindowGeneratorLineStyle : public virtual LineStyle , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorLineStyle(const QString &n, const QString &l)
        : LineStyle(n,l), WindowGeneratorField("linestyle",n,l), Field("linestyle",n,l) { }
    virtual void            writeHeaderCallback(ostream &h)
    {
        h << "    void "<<name<<"Changed(int style);" << endl;
    }
    virtual void            writeHeaderData(ostream &h)
    {
        h << "    QvisLineStyleWidget *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ostream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QvisLineStyleWidget(0, central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(lineStyleChanged(int))," << endl
          << "            this, SLOT("<<name<<"Changed(int)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ostream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(ostream &c)
    {
        c << "            "<<name<<"->blockSignals(true);" << endl;
        c << "            "<<name<<"->SetLineStyle(atts->Get"<<Name<<"());" << endl;
        c << "            "<<name<<"->blockSignals(false);" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, ostream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(int style)" << endl;
        c << "{" << endl;
        c << "    atts->Set"<<Name<<"(style);" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


//
// -------------------------------- LineWidth --------------------------------
//
class WindowGeneratorLineWidth : public virtual LineWidth , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorLineWidth(const QString &n, const QString &l)
        : LineWidth(n,l), WindowGeneratorField("linewidth",n,l), Field("linewidth",n,l) { }
    virtual void            writeHeaderCallback(ostream &h)
    {
        h << "    void "<<name<<"Changed(int style);" << endl;
    }
    virtual void            writeHeaderData(ostream &h)
    {
        h << "    QvisLineWidthWidget *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ostream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QvisLineWidthWidget(0, central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(lineWidthChanged(int))," << endl
          << "            this, SLOT("<<name<<"Changed(int)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ostream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(ostream &c)
    {
        c << "            "<<name<<"->blockSignals(true);" << endl;
        c << "            "<<name<<"->SetLineWidth(atts->Get"<<Name<<"());" << endl;
        c << "            "<<name<<"->blockSignals(false);" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, ostream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(int style)" << endl;
        c << "{" << endl;
        c << "    atts->Set"<<Name<<"(style);" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


//
// -------------------------------- VariableName --------------------------------
//
class WindowGeneratorVariableName : public virtual VariableName , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorVariableName(const QString &n, const QString &l)
        : VariableName(n,l), WindowGeneratorField("variablename",n,l), Field("variablename",n,l) { }
    virtual void            writeHeaderCallback(ostream &h)
    {
        h << "    void "<<name<<"Changed(const QString &varName);" << endl;
    }
    virtual void            writeHeaderData(ostream &h)
    {
        h << "    QvisVariableButton *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ostream &c)
    {
        writeSourceCreateLabel(c);

        static const char *categoryNames[] = {
            "QvisVariableButton::Meshes",
            "QvisVariableButton::Scalars",
            "QvisVariableButton::Materials",
            "QvisVariableButton::Vectors",
            "QvisVariableButton::Subsets",
            "QvisVariableButton::Species",
            "QvisVariableButton::Curves",
            "QvisVariableButton::Tensors",
            "QvisVariableButton::SymmetricTensors"
            "QvisVariableButton::Labels"
        };

        // Write the mask.
        c << "    int " << name <<"Mask = ";
        int m = 1, count = 0;
        for(int i = 0; i < 10; ++i)
        {
            if(varTypes & m)
            {
                if(count > 0)
                    c << " | ";
                c << categoryNames[i];
                ++count;
            }
            m = m << 1;
        }
        c << ";" << endl;
        c << "    "<<name<<" = new QvisVariableButton(true, true, true, "<<name<<"Mask, central, \""<<name<<"\");" << endl;
        if(valueSet)
            c << "    " << name << "->setDefaultVariable(\"" << val << "\");" << endl;
        c << "    connect("<<name<<", SIGNAL(activated(const QString&))," << endl
          << "            this, SLOT("<<name<<"Changed(const QString&)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ostream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(ostream &c)
    {
        c << "            "<<name<<"->setText(atts->Get"<<Name<<"().c_str());" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, ostream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(const QString &varName)" << endl;
        c << "{" << endl;
        c << "    atts->Set"<<Name<<"(varName.latin1());" << endl;
        c << "    SetUpdate(false);" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


//
// ------------------------------------ Att -----------------------------------
//
class WindowGeneratorAtt : public virtual Att , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorAtt(const QString &t, const QString &n, const QString &l)
        : Att(t,n,l), WindowGeneratorField("att",n,l), Field("att",n,l) { }
};


//
// --------------------------------- AttVector --------------------------------
//
class WindowGeneratorAttVector : public virtual AttVector , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorAttVector(const QString &t, const QString &n, const QString &l)
        : AttVector(t,n,l), WindowGeneratorField("attVector",n,l), Field("attVector",n,l) { }
};


//
// ----------------------------------- Enum -----------------------------------
//
class WindowGeneratorEnum : public virtual Enum , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorEnum(const QString &t, const QString &n, const QString &l)
        : Enum(t,n,l), WindowGeneratorField("enum",n,l), Field("enum",n,l) { }
    virtual void            writeHeaderCallback(ostream &h)
    {
        h << "    void "<<name<<"Changed(int val);" << endl;
    }
    virtual void            writeHeaderData(ostream &h)
    {
        h << "    QButtonGroup *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ostream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QButtonGroup(central, \""<<name<<"\");" << endl;
        c << "    "<<name<<"->setFrameStyle(QFrame::NoFrame);" << endl;
        c << "    QHBoxLayout *"<<name<<"Layout = new QHBoxLayout("<<name<<");" << endl;
        c << "    "<<name<<"Layout->setSpacing(10);" << endl;

        for (int i=0; i<enumType->values.size(); i++)
        {
            c << "    QRadioButton *"<<name<<enumType->type<<enumType->values[i]
              << " = new QRadioButton(\""<<enumType->values[i]<<"\", "<<name<<");" << endl;
            c << "    "<<name<<"Layout->addWidget("<<name<<enumType->type<<enumType->values[i]<<");" << endl;
        }

        c << "    connect("<<name<<", SIGNAL(clicked(int))," << endl
          << "            this, SLOT("<<name<<"Changed(int)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ostream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(ostream &c)
    {
        c << "            "<<name<<"->setButton(atts->Get"<<Name<<"());" << endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, ostream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(int val)" << endl;
        c << "{" << endl;
        c << "    if(val != atts->Get"<<Name<<"())" << endl;
        c << "    {" << endl;
        c << "        atts->Set"<<Name<<"("<<GetCPPName(true,classname)<<"(val));" << endl;
        c << "        Apply();" << endl;
        c << "    }" << endl;
        c << "}" << endl;
    }
};


// ----------------------------------------------------------------------------
// Modifications:
//    Brad Whitlock, Wed Dec 8 16:12:01 PST 2004
//    Added support for variable names.
//
// ----------------------------------------------------------------------------
class WindowFieldFactory
{
  public:
    static WindowGeneratorField *createField(const QString &name,
                                           const QString &type,
                                           const QString &subtype,
                                           const QString &length,
                                           const QString &label)
    {
        WindowGeneratorField *f = NULL;
        if      (type.isNull())          throw QString().sprintf("Field %s was specified with no type.",name.latin1());
        else if (type == "int")          f = new WindowGeneratorInt(name,label);
        else if (type == "intArray")     f = new WindowGeneratorIntArray(length,name,label);
        else if (type == "intVector")    f = new WindowGeneratorIntVector(name,label);
        else if (type == "bool")         f = new WindowGeneratorBool(name,label);
        else if (type == "float")        f = new WindowGeneratorFloat(name,label);
        else if (type == "floatArray")   f = new WindowGeneratorFloatArray(length,name,label);
        else if (type == "double")       f = new WindowGeneratorDouble(name,label);
        else if (type == "doubleArray")  f = new WindowGeneratorDoubleArray(length,name,label);
        else if (type == "doubleVector") f = new WindowGeneratorDoubleVector(name,label);
        else if (type == "uchar")        f = new WindowGeneratorUChar(name,label);
        else if (type == "ucharArray")   f = new WindowGeneratorUCharArray(length,name,label);
        else if (type == "ucharVector")   f = new WindowGeneratorUCharVector(name,label);
        else if (type == "string")       f = new WindowGeneratorString(name,label);
        else if (type == "stringVector") f = new WindowGeneratorStringVector(name,label);
        else if (type == "colortable")   f = new WindowGeneratorColorTable(name,label);
        else if (type == "color")        f = new WindowGeneratorColor(name,label);
        else if (type == "opacity")      f = new WindowGeneratorOpacity(name,label);
        else if (type == "linestyle")    f = new WindowGeneratorLineStyle(name,label);
        else if (type == "linewidth")    f = new WindowGeneratorLineWidth(name,label);
        else if (type == "variablename") f = new WindowGeneratorVariableName(name,label);
        else if (type == "att")          f = new WindowGeneratorAtt(subtype,name,label);
        else if (type == "attVector")    f = new WindowGeneratorAttVector(subtype,name,label);
        else if (type == "enum")         f = new WindowGeneratorEnum(subtype, name, label);

        if (!f)
            throw QString().sprintf("WindowFieldFactory: unknown type for field %s: %s",name.latin1(),type.latin1());

        return f;
    }
};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
class WindowGeneratorAttribute
{
  public:
    QString name;
    QString purpose;
    bool    persistent;
    QString exportAPI;
    QString exportInclude;
    QString windowname;
    QString plugintype;
    vector<WindowGeneratorField*> fields;
    vector<Function*> functions;
    vector<Constant*> constants;
    vector<Include*>  includes;
    vector<Code*>     codes;
    CodeFile *codeFile;
  public:
    WindowGeneratorAttribute(const QString &n, const QString &p, const QString &f,
                             const QString &e, const QString &ei)
        : name(n), purpose(p), exportAPI(e), exportInclude(ei)
    {
        if (f.isNull())
            codeFile = NULL;
        else
            codeFile = new CodeFile(f);
        if (codeFile)
            codeFile->Parse();

        plugintype = "";
        if (name.right(9) == "Attribute")
            windowname = QString("Qvis") + name.left(name.length() - 9) + "Window";
        else if (name.right(10) == "Attributes")
            windowname = QString("Qvis") + name.left(name.length() - 10) + "Window";
        else
            windowname = QString("Qvis") + name + QString("Window");
    }
    bool HasFunction(const QString &f)
    {
        for (int i=0; i<functions.size(); i++)
            if (functions[i]->name == f && functions[i]->user == false)
                return true;
        return false;
    }
    void PrintFunction(ostream &out, const QString &f)
    {
        for (int i=0; i<functions.size(); i++)
            if (functions[i]->name == f && functions[i]->user == false)
                out << functions[i]->def;
    }
    void DeleteFunction(ostream &out, const QString &f)
    {
        for (int i=0; i<functions.size(); i++)
            if (functions[i]->name == f && functions[i]->user == false)
            {
                for (int j=i+1; j<functions.size(); j++)
                    functions[j-1] = functions[j];
                return;
            }
    }
    void Print(ostream &out)
    {
        out << "    Attribute: " << name << " (" << purpose << ")" << endl;
        int i;
        for (i=0; i<fields.size(); i++)
            fields[i]->Print(out);
        for (i=0; i<functions.size(); i++)
            functions[i]->Print(out);
    }

    QString CurrentTime()
    {
        char *tstr[] = {"PDT", "PST"};
        char s1[10], s2[10], s3[10];
        time_t t;
        char *c = NULL;
        int h,m,s,y;
        t = time(NULL);
        c = asctime(localtime(&t));
        // Read the hour.
        sscanf(c, "%s %s %s %d:%d:%d %d", s1, s2, s3, &h, &m, &s, &y);
        // Reformat the string a little.
        QString retval;
        retval.sprintf("%s %s %s %02d:%02d:%02d %s %d",
                       s1, s2, s3, h, m, s, tstr[h > 12], y);

        return retval;
    }

    void WriteClassComment(ostream &c, const QString &purpose)
    {
        c << "// ****************************************************************************" << endl;
        c << "// Class: " << windowname << endl;
        c << "//" << endl;
        c << "// Purpose: " << endl;
        c << "//   " << purpose.latin1() << endl;
        c << "//" << endl;
        c << "// Notes:      This class was automatically generated!" << endl;
        c << endl;
        c << "// Programmer: xml2window" << endl;
        c << "// Creation:   " << CurrentTime() << endl;
        c << "//" << endl;
        c << "// Modifications:" << endl;
        c << "//   " << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
    }

    void WriteMethodComment(ostream &c, const QString &methodName,
                           const QString &purpose)
    {
        c << "// ****************************************************************************" << endl;
        c << "// Method: " << windowname << "::" << methodName.latin1() << endl;
        c << "//" << endl;
        c << "// Purpose: " << endl;
        c << "//   " << purpose.latin1() << endl;
        c << "//" << endl;
        c << "// Programmer: xml2window" << endl;
        c << "// Creation:   " << CurrentTime() << endl;
        c << "//" << endl;
        c << "// Modifications:" << endl;
        c << "//   " << endl;
        c << "// ****************************************************************************" << endl;
        c << endl;
    }

    // ************************************************************************
    // Modifications:
    //   Brad Whitlock, Fri Apr 12 13:22:04 PST 2002
    //   Made it generate operator windows that inherit from QvisOperatorWindow.
    //
    //   Jeremy Meredith, Thu Oct 24 10:04:58 PDT 2002
    //   Updated to work better with standalone atts.
    //
    //   Brad Whitlock, Wed Dec 8 16:12:32 PST 2004
    //   Added support for variable buttons.
    //
    // ************************************************************************

    void WriteHeader(ostream &h)
    {
        h << "#ifndef " << windowname.upper() << "_H" << endl;
        h << "#define " << windowname.upper() << "_H" << endl;
        h << endl;
        if(plugintype == "operator")
            h << "#include <QvisOperatorWindow.h>" << endl;
        else if(plugintype == "plot")
            h << "#include <QvisPostableWindowObserver.h>" << endl;
        h << "#include <AttributeSubject.h>" << endl;
        h << endl;
        h << "class " << name << ";" << endl;
        h << "class QLabel;" << endl;
        h << "class QCheckBox;" << endl;
        h << "class QLineEdit;" << endl;
        h << "class QSpinBox;" << endl;
        h << "class QVBox;" << endl;
        h << "class QButtonGroup;" << endl;
        h << "class QvisColorTableButton;" << endl;
        h << "class QvisOpacitySlider;" << endl;
        h << "class QvisColorButton;" << endl;
        h << "class QvisLineStyleWidget;" << endl;
        h << "class QvisLineWidthWidget;" << endl;
        h << "class QvisVariableButton;" << endl;
        h << endl;
        
        WriteClassComment(h, QString("Defines ") + windowname + QString(" class."));

        if(plugintype == "operator")
            h << "class " << windowname << " : public QvisOperatorWindow" << endl;
        else if(plugintype == "plot")
            h << "class " << windowname << " : public QvisPostableWindowObserver" << endl;
        else
            h << "class " << windowname << " : public QvisPostableWindowObserver" << endl;
        h << "{" << endl;
        h << "    Q_OBJECT" << endl;
        h << "  public:" << endl;
        h << "    " << windowname << "("
          << (plugintype=="" ? "" : "const int type,") << endl
          << "                         "<<name<<" *subj," << endl
          << "                         const char *caption = 0," << endl
          << "                         const char *shortName = 0," << endl
          << "                         QvisNotepadArea *notepad = 0);" << endl;
        h << "    virtual ~"<<windowname<<"();" << endl;
        h << "    virtual void CreateWindowContents();" << endl;
        if(plugintype != "operator")
        {
            h << "  public slots:" << endl;
            h << "    virtual void apply();" << endl;
            h << "    virtual void makeDefault();" << endl;
            h << "    virtual void reset();" << endl;
        }
        h << "  protected:" << endl;
        h << "    void UpdateWindow(bool doAll);" << endl;
        if(plugintype == "operator")
            h << "    virtual void GetCurrentValues(int which_widget);" << endl;
        else if(plugintype == "plot")
        {
            h << "    void GetCurrentValues(int which_widget);" << endl;
            h << "    void Apply(bool ignore = false);" << endl;
        }
        else
        {
            h << "    void GetCurrentValues(int which_widget);" << endl;
            h << "    void Apply(bool ignore = false);" << endl;
        }
        h << "  private slots:" << endl;
        int i;
        for (i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal) continue;
            fields[i]->writeHeaderCallback(h);
        }
        h << "  private:" << endl;
        if (plugintype == "plot")
            h << "    int plotType;" << endl;
        for (i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal) continue;
            fields[i]->writeHeaderData(h);
        }
        for (i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal) continue;
            fields[i]->writeHeaderLabelData(h);
        }
        h << endl;
        h << "    "<<name<<" *atts;" << endl;

        h << "};" << endl;
        h << endl;
        h << endl;
        h << endl;

        h << "#endif" << endl;
    }

    // ************************************************************************
    // Modifications:
    //   Brad Whitlock, Fri Apr 12 13:22:04 PST 2002
    //   Made it generate operator windows that inherit from QvisOperatorWindow.
    //
    //   Jeremy Meredith, Thu Oct 17 15:59:09 PDT 2002
    //   Made it create "enablees" on the fly.  This makes maintenance easier.
    //
    //   Jeremy Meredith, Thu Oct 24 10:04:42 PDT 2002
    //   Updated to work better with standalone atts.
    //
    //   Jeremy Meredith, Thu Mar 27 12:53:56 PST 2003
    //   No longer assume the class scope is before the enum enabler values.
    //
    //   Brad Whitlock, Wed Dec 8 16:13:25 PST 2004
    //   Added support for variable names.
    //
    // ************************************************************************

    void WriteSource(ostream &c)
    {
        c << "#include \""<<windowname<<".h\"" << endl;
        c << endl;
        c << "#include <"<<name<<".h>" << endl;
        c << "#include <ViewerProxy.h>" << endl;
        c << endl;
        c << "#include <qcheckbox.h>" << endl;
        c << "#include <qlabel.h>" << endl;
        c << "#include <qlayout.h>" << endl;
        c << "#include <qlineedit.h>" << endl;
        c << "#include <qspinbox.h>" << endl;
        c << "#include <qvbox.h>" << endl;
        c << "#include <qbuttongroup.h>" << endl;
        c << "#include <qradiobutton.h>" << endl;
        c << "#include <QvisColorTableButton.h>" << endl;
        c << "#include <QvisOpacitySlider.h>" << endl;
        c << "#include <QvisColorButton.h>" << endl;
        c << "#include <QvisLineStyleWidget.h>" << endl;
        c << "#include <QvisLineWidthWidget.h>" << endl;
        c << "#include <QvisVariableButton.h>" << endl;
        c << endl;
        c << "#include <stdio.h>" << endl;
        c << "#include <string>" << endl;
        c << endl;
        c << "using std::string;" << endl;
        c << endl;

        // constructor
        WriteMethodComment(c, windowname, "Constructor");
        c << windowname<<"::"<<windowname<<"("
          << (plugintype=="" ? "" : "const int type,") << endl
          << "                         "<<name<<" *subj," << endl
          << "                         const char *caption," << endl
          << "                         const char *shortName," << endl
          << "                         QvisNotepadArea *notepad)" << endl;
        if(plugintype == "operator")
        {
            c << "    : QvisOperatorWindow(type,subj, caption, shortName, notepad)" << endl;
            c << "{" << endl;
            c << "    atts = subj;" << endl;
            c << "}" << endl;
        }
        else if(plugintype == "plot")
        {
            c << "    : QvisPostableWindowObserver(subj, caption, shortName, notepad)" << endl;
            c << "{" << endl;
            c << "    plotType = type;" << endl;
            c << "    atts = subj;" << endl;
            c << "}" << endl;
        }
        else
        {
            c << "    : QvisPostableWindowObserver(subj, caption, shortName, notepad)" << endl;
            c << "{" << endl;
            c << "    atts = subj;" << endl;
            c << "}" << endl;
        }
        c << endl;
        c << endl;

        // destructor
        WriteMethodComment(c, QString("~") + windowname, "Destructor");
        c << windowname<<"::~"<<windowname<<"()" << endl;
        c << "{" << endl;
        c << "}" << endl;
        c << endl;
        c << endl;

        // CreateWindowContents
        WriteMethodComment(c, "CreateWindowContents", "Creates the widgets for the window.");
        c << "void" << endl;
        c << windowname<<"::CreateWindowContents()" << endl;
        c << "{" << endl;
        c << "    QGridLayout *mainLayout = new QGridLayout("
          << "topLayout, " << fields.size() << ",2,  10, \"mainLayout\");" << endl;
        c << endl;
        c << endl;
        int i;
        for (i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal) continue;
            fields[i]->writeSourceCreate(c);
            c << endl;
        }
        c << "}" << endl;
        c << endl;
        c << endl;

        // updatewindow
        WriteMethodComment(c, "UpdateWindow",
                           "Updates the widgets in the window when the subject changes.");
        c << "void" << endl;
        c << windowname << "::UpdateWindow(bool doAll)" << endl;
        c << "{" << endl;
        c << "    QString temp;" << endl;
        c << "    double r;" << endl;
        c << "" << endl;
        c << "    for(int i = 0; i < atts->NumAttributes(); ++i)" << endl;
        c << "    {" << endl;
        c << "        if(!doAll)" << endl;
        c << "        {" << endl;
        c << "            if(!atts->IsSelected(i))" << endl;
        c << "            {" << endl;
        c << "                continue;" << endl;
        c << "            }" << endl;
        c << "        }" << endl;
        c << "" << endl;
        c << "        const double         *dptr;" << endl;
        c << "        const float          *fptr;" << endl;
        c << "        const int            *iptr;" << endl;
        c << "        const char           *cptr;" << endl;
        c << "        const unsigned char  *uptr;" << endl;
        c << "        const string         *sptr;" << endl;
        c << "        QColor                tempcolor;" << endl;
        c << "        switch(i)" << endl;
        c << "        {" << endl;
        for (i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal) continue;

            WindowGeneratorField *field   = fields[i];
            c << "          case "<<i<<": //"<<field->name << endl;

            vector<Field*> enablees;
            int j;
            for (j=0; j<fields.size(); j++)
            {
                if (fields[j]->enabler == field)
                {
                    enablees.push_back(fields[j]);
                }
            }

            for (j=0; j<enablees.size(); j++)
            {
                c << "            if (atts->Get"<<field->Name<<"() == ";
                if (fields[i]->type=="enum")
                    c << name<<"::"<<enablees[j]->enableval[0];
                else
                    c << enablees[j]->enableval[0];

                for (int k=1; k<enablees[j]->enableval.size(); k++)
                {
                    c << " || atts->Get"<<field->Name<<"() == ";
                    if (fields[i]->type=="enum")
                        c << name<<"::"<<enablees[j]->enableval[k];
                    else
                        c << enablees[j]->enableval[k];
                }

                c << ")" << endl;
                c << "            {\n";
                c << "                "<<enablees[j]->name<<"->setEnabled(true);" << endl;
                c << "                "<<enablees[j]->name<<"Label->setEnabled(true);" << endl;
                c << "            }\n";
                c << "            else" << endl;
                c << "            {\n";
                c << "                "<<enablees[j]->name<<"->setEnabled(false);" << endl;
                c << "                "<<enablees[j]->name<<"Label->setEnabled(false);" << endl;
                c << "            }\n";
            }
            fields[i]->writeSourceUpdateWindow(c);
            c << "            break;" << endl;
        }
        c << "        }" << endl;
        c << "    }" << endl;
        c << "}" << endl;
        c << endl;
        c << endl;

        // getcurrent
        WriteMethodComment(c, "GetCurrentValues",
                           "Gets values from certain widgets and stores them in the subject.");
        c << "void" << endl;
        c << windowname << "::GetCurrentValues(int which_widget)" << endl;
        c << "{" << endl;
        c << "    bool okay, doAll = (which_widget == -1);" << endl;
        c << "    QString msg, temp;" << endl;
        c << endl;
        for (i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal) continue;
            c << "    // Do " << fields[i]->name << endl;
            c << "    if(which_widget == "<<i<<" || doAll)" << endl;
            c << "    {" << endl;
            fields[i]->writeSourceGetCurrent(c);
            c << "    }" << endl;
            c << endl;
        }
        c << "}" << endl;
        c << endl;
        c << endl;

        if(plugintype != "operator")
        {
            // Apply
            WriteMethodComment(c, "Apply", "Called to apply changes in the subject.");
            c << "void" << endl;
            c << windowname<<"::Apply(bool ignore)" << endl;
            c << "{" << endl;
            c << "    if(AutoUpdate() || ignore)" << endl;
            c << "    {" << endl;
            c << "        GetCurrentValues(-1);" << endl;
            c << "        atts->Notify();" << endl;
            c << "" << endl;
            if (plugintype == "plot")
                c << "        viewer->SetPlotOptions(plotType);" << endl;
            else if(plugintype != "operator")
                c << "        viewer->Set"<<name<<"();" << endl;
            c << "    }" << endl;
            c << "    else" << endl;
            c << "        atts->Notify();" << endl;
            c << "}" << endl;
            c << endl;
            c << endl;
        }

        c << "//" << endl;
        c << "// Qt Slot functions" << endl;
        c << "//" << endl;
        c << endl;
        c << endl;

        // Only plot plugins need to override these at present. Operators
        // now have a base class that does it.
        if(plugintype != "operator")
        {
            // apply
            WriteMethodComment(c, "apply", "Qt slot function called when apply button is clicked.");
            c << "void" << endl;
            c << windowname << "::apply()" << endl;
            c << "{" << endl;
            c << "    Apply(true);" << endl;
            c << "}" << endl;
            c << endl;
            c << endl;

            // makeDefault
            WriteMethodComment(c, "makeDefault", "Qt slot function called when \"Make default\" button is clicked.");
            c << "void" << endl;
            c << windowname<<"::makeDefault()" << endl;
            c << "{" << endl;
            c << "    GetCurrentValues(-1);" << endl;
            c << "    atts->Notify();" << endl;
            if (plugintype == "plot")
                c << "    viewer->SetDefaultPlotOptions(plotType);" << endl;
            else if(plugintype != "operator")
                c << "    viewer->SetDefault"<<name<<"();" << endl;
            c << "}" << endl;
            c << endl;
            c << endl;

            // reset
            WriteMethodComment(c, "reset", "Qt slot function called when reset button is clicked.");
            c << "void" << endl;
            c << windowname << "::reset()" << endl;
            c << "{" << endl;
            if (plugintype == "plot")
                c << "    viewer->ResetPlotOptions(plotType);" << endl;
            else if(plugintype != "operator")
                c << "    viewer->Reset"<<name<<"();" << endl;
            c << "}" << endl;
            c << endl;
            c << endl;
        }

        // callbacks
        for (i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal) continue;
            fields[i]->writeSourceCallback(name, windowname, c);
            c << endl;
            c << endl;
        }
    }
};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
class WindowGeneratorPlugin
{
  public:
    QString name;
    QString type;
    QString label;
    QString version;
    QString vartype;
    QString dbtype;
    QString windowname;
    bool    enabledByDefault;
    bool    has_MDS_specific_code;

    vector<QString> cxxflags;
    vector<QString> ldflags;
    vector<QString> libs;
    vector<QString> extensions; // for DB plugins
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

    WindowGeneratorAttribute *atts;
  public:
    WindowGeneratorPlugin(const QString &n,const QString &l,const QString &t,const QString &vt,const QString &dt,const QString &v, const QString &, bool,bool,bool)
        : name(n), type(t), label(l), version(v), vartype(vt), dbtype(dt), atts(NULL)
    {
        enabledByDefault = true;
        has_MDS_specific_code = false;
        if (type == "plot")
            windowname = QString("Qvis")+name+QString("PlotWindow");
        else if (type == "operator")
            windowname = QString("Qvis")+name+QString("Window");
    }
    void Print(ostream &out)
    {
        out << "Plugin: "<<name<<" (\""<<label<<"\", type="<<type<<") -- version "<<version<< endl;
        if (atts)
            atts->Print(cout);
    }
    void WriteHeader(ostream &h)
    {
        atts->WriteHeader(h);
    }
    void WriteSource(ostream &c)
    {
        atts->WriteSource(c);
    }
};


// ----------------------------------------------------------------------------
//                           Override default types
// ----------------------------------------------------------------------------
#define FieldFactory WindowFieldFactory
#define Field        WindowGeneratorField
#define Attribute    WindowGeneratorAttribute
#define Enum         WindowGeneratorEnum
#define Plugin       WindowGeneratorPlugin

#endif
