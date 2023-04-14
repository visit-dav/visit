// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef GENERATE_WINDOW_H
#define GENERATE_WINDOW_H
#include <set>
#include <QTextStream>
#include "Field.h"
#include "PluginBase.h"

#define GENERATOR_NAME "xml2window"

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
//    Hank Childs, Tue May 24 09:54:36 PDT 2005
//    Added bool to constructor for hasoptions.
//
//    Hank Childs, Fri Jun  9 09:53:32 PDT 2006
//    Added copyright string.
//
//    Brad Whitlock, Tue Feb 13 13:34:11 PST 2007
//    Made it use GetViewerMethods() and made it more aggressive about
//    blocking signals in the Update method.
//
//    Cyrus Harrison, Wed Mar  7 09:54:36 PST 2007
//    Allow for engine-specific code in a plugin's source files.
//
//    Cyrus Harrison, Thu Mar  8 16:20:50 PST 2007
//    Fixed problem with uninitialized labels for bool types.
//
//    Brad Whitlock, Mon Dec 17 11:21:58 PST 2007
//    Made it use new Attribute ids instead of ints.
//
//    Brad Whitlock, Wed Mar 5 11:42:27 PDT 2008
//    Added generatorName.
//
//    Brad Whitlock, Wed Apr  9 12:29:52 PDT 2008
//    Made it generate code that is easier to internationalize.
//
//    Brad Whitlock, Wed Apr 23 13:34:15 PDT 2008
//    Made it use QString::arg for internationalization.
//
//    Jeremy Meredith, Thu Aug  7 14:34:01 EDT 2008
//    Reorder constructor initializers to be the correct order.
//
//    Cyrus Harrison, Tue Aug 26 15:40:55 PDT 2008
//    Fix enable case for Enums with QButtonGroup, ensure QButtonGroup has
//    valid parent.
//
//    Dave Pugmire, Tue Feb 24 15:26:46 EST 2009
//    tempcolor wasn't declared as a QColor before it was used.
//
//    Dave Pugmire, Wed Sep  5 11:01:33 EDT 2012
//    Enablers produced bad code for checkboxes. There is no label. Added
//    a virtual method, HasLabel() to determine if code should be generated.
//
//    Kathleen Biagas, Thu Jun  9 16:22:37 PDT 2016
//    For QSpinBox, turn off keyboardTracking.
//
//    Kathleen Biagas, Thu Nov 17, 2022
//    Added boolArray and boolVector.
//
// ****************************************************************************

class WindowGeneratorField : public virtual Field
{
  protected:
    QString generatorName;
  public:
    WindowGeneratorField(const QString &t, const QString &n, const QString &l)
        : Field(t,n,l), generatorName(GENERATOR_NAME)
    {
    }
    // helper functions
    void writeSourceCreateLabel(QTextStream &c)
    {
        c << "    "<<name<<"Label = new QLabel(tr(\""<<label<<"\"), central);" << Endl;
        c << "    mainLayout->addWidget("<<name<<"Label,"<<index<<",0);" << Endl;
    }
    // virtual functions
    virtual void               writeHeaderCallback(QTextStream &h)
    {
        h << "    //writeHeaderCallback unknown for " << type << " (variable " << name << ")" << Endl;
    }
    virtual void               writeHeaderLabelData(QTextStream &h)
    {
        h << "    QLabel *"<<name<<"Label;" << Endl;
    }
    virtual void               writeHeaderData(QTextStream &h)
    {
        h << "    //writeHeaderData unknown for " << type << " (variable " << name << ")" << Endl;
    }
    virtual void               writeSourceCreate(QTextStream &c)
    {
        c << "    //writeSourceCreate unknown for " << type << " (variable " << name << ")" << Endl;
    }
    virtual bool               providesSourceGetCurrent() const { return false; }
    virtual void               writeSourceGetCurrent(QTextStream &c)
    {
        c << "        //writeSourceGetCurrent unknown for " << type << " (variable " << name << ")" << Endl;
    }
    virtual void               writeSourceUpdateWindow(QTextStream &c)
    {
        c << "        //writeSourceUpdate unknown for " << type << " (variable " << name << ")" << Endl;
    }
    virtual void               writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "//writeSourceCallback unknown for " << type << " (variable " << name << ")" << Endl;
    }
};

//
// ------------------------------------ Int -----------------------------------
//
class WindowGeneratorInt : public virtual Int , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorInt(const QString &n, const QString &l)
        : Field("int",n,l), Int(n,l), WindowGeneratorField("int",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        if (rangeSet)
            h << "    void "<<name<<"Changed(int val);" << Endl;
        else
            h << "    void "<<name<<"ProcessText();" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        if (rangeSet)
            h << "    QSpinBox *"<<name<<";" << Endl;
        else
            h << "    QLineEdit *"<<name<<";" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        if (rangeSet)
        {
            c << "    "<<name<<" = new QSpinBox(central);" << Endl;
            c << "    "<<name<<"->setKeyboardTracking(false);"<<Endl;
            c << "    "<<name<<"->setMinimum("<<min<<");"<<Endl;
            c << "    "<<name<<"->setMaximum("<<max<<");"<<Endl;
            c << "    connect("<<name<<", SIGNAL(valueChanged(int)), " << Endl
              << "            this, SLOT("<<name<<"Changed(int)));" << Endl;
        }
        else
        {
            c << "    "<<name<<" = new QLineEdit(central);" << Endl;
            c << "    connect("<<name<<", SIGNAL(returnPressed())," << Endl
              << "            this, SLOT("<<name<<"ProcessText()));" << Endl;
        }
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual bool            providesSourceGetCurrent() const { return true; }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        if (rangeSet)
        {
            c << "        if ("<<name<<"->value() != atts->Get"<<Name<<"())" << Endl;
            c << "            atts->Set"<<Name<<"("<<name<<"->value())" << Endl;
        }
        else
        {
            c << "        int val;" << Endl;
            c << "        if(LineEditGetInt("<<name<<", val))" << Endl;
            c << "            atts->Set"<<Name<<"(val);" << Endl;
            c << "        else" << Endl;
            c << "        {" << Endl;
            QString msgLabel = (label.length()>0) ? label : name;
            c << "            ResettingError(tr(\""<<msgLabel<<"\")," << Endl;
            c << "                IntToQString(atts->Get"<<Name<<"()));" << Endl;
            c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << Endl;
            c << "        }" << Endl;
        }
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        if (rangeSet)
        {
            c << "            "<<name<<"->blockSignals(true);" << Endl;
            c << "            "<<name<<"->setValue(atts->Get"<<Name<<"());" << Endl;
            c << "            "<<name<<"->blockSignals(false);" << Endl;
        }
        else
        {
            c << "            "<<name<<"->setText(IntToQString(atts->Get"<<Name<<"()));" << Endl;
        }
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        if (rangeSet)
        {
            c << "void" << Endl;
            c << windowname<<"::"<<name<<"Changed(int val)" << Endl;
            c << "{" << Endl;
            c << "    atts->Set"<<Name<<"(val);" << Endl;
            if(!isEnabler)
                c << "    SetUpdate(false);" << Endl;
            c << "    Apply();" << Endl;
            c << "}" << Endl;
        }
        else
        {
            c << "void" << Endl;
            c << windowname<<"::"<<name<<"ProcessText()" << Endl;
            c << "{" << Endl;
            c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << Endl;
            c << "    Apply();" << Endl;
            c << "}" << Endl;
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
        : Field("intArray",n,l), IntArray(s,n,l), WindowGeneratorField("intArray",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << Endl
          << "            this, SLOT("<<name<<"ProcessText()));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual bool            providesSourceGetCurrent() const { return true; }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        int val[" << length << "];" << Endl;
        c << "        if(LineEditGetInts("<<name<<", val, " << length << "))" << Endl;
        c << "            atts->Set"<<Name<<"(val);" << Endl;
        c << "        else" << Endl;
        c << "        {" << Endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << Endl;
        c << "                IntsToQString(atts->Get"<<Name<<"(), "<<length<<"));" << Endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << Endl;
        c << "        }" << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(IntsToQString(atts->Get"<<Name<<"(),"<<length<<"));" << Endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"ProcessText()" << Endl;
        c << "{" << Endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};


//
// --------------------------------- IntVector --------------------------------
//
class WindowGeneratorIntVector : public virtual IntVector , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorIntVector(const QString &n, const QString &l)
        : Field("intVector",n,l), IntVector(n,l), WindowGeneratorField("intVector",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << Endl
          << "            this, SLOT("<<name<<"ProcessText()));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual bool            providesSourceGetCurrent() const { return true; }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        intVector val;" << Endl;
        c << "        if(LineEditGetInts("<<name<<", val))" << Endl;
        c << "            atts->Set"<<Name<<"(val);" << Endl;
        c << "        else" << Endl;
        c << "        {" << Endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << Endl;
        c << "                IntsToQString(atts->Get"<<Name<<"()));" << Endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << Endl;
        c << "        }" << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(IntsToQString(atts->Get"<<Name<<"()));" << Endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"ProcessText()" << Endl;
        c << "{" << Endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};


//
// ----------------------------------- Bool -----------------------------------
//
class WindowGeneratorBool : public virtual Bool , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorBool(const QString &n, const QString &l)
        : Field("bool",n,l), Bool(n,l), WindowGeneratorField("bool",n,l) { }
    virtual bool            HasLabel() {return false;}
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"Changed(bool val);" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QCheckBox *"<<name<<";" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        c << "    "<<name<<" = new QCheckBox(tr(\""<<label<<"\"), central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(toggled(bool))," << Endl
          << "            this, SLOT("<<name<<"Changed(bool)));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",0);" << Endl;
    }
    virtual void            writeHeaderLabelData(QTextStream &h)
    {
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->blockSignals(true);" << Endl;
        c << "            "<<name<<"->setChecked(atts->Get"<<Name<<"());" << Endl;
        c << "            "<<name<<"->blockSignals(false);" << Endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"Changed(bool val)" << Endl;
        c << "{" << Endl;
        c << "    atts->Set"<<Name<<"(val);" << Endl;
        if(!isEnabler)
            c << "    SetUpdate(false);" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};

//
// ---------------------------------- BoolArray --------------------------------
//
class WindowGeneratorBoolArray : public virtual BoolArray , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorBoolArray(const QString &s, const QString &n, const QString &l)
        : Field("boolArray",n,l), BoolArray(s,n,l), WindowGeneratorField("boolArray",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << Endl
          << "            this, SLOT("<<name<<"ProcessText()));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual bool            providesSourceGetCurrent() const { return true; }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        bool val[" << length << "];" << Endl;
        c << "        if(LineEditGetBools("<<name<<", val, " << length << "))" << Endl;
        c << "            atts->Set"<<Name<<"(val);" << Endl;
        c << "        else" << Endl;
        c << "        {" << Endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << Endl;
        c << "                BoolsToQString(atts->Get"<<Name<<"(), "<<length<<"));" << Endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << Endl;
        c << "        }" << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(BoolsToQString(atts->Get"<<Name<<"(),"<<length<<"));" << Endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"ProcessText()" << Endl;
        c << "{" << Endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};


//
// --------------------------------- BoolVector --------------------------------
//
class WindowGeneratorBoolVector : public virtual BoolVector , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorBoolVector(const QString &n, const QString &l)
        : Field("boolVector",n,l), BoolVector(n,l), WindowGeneratorField("boolVector",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << Endl
          << "            this, SLOT("<<name<<"ProcessText()));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual bool            providesSourceGetCurrent() const { return true; }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        boolVector val;" << Endl;
        c << "        if(LineEditGetBools("<<name<<", val))" << Endl;
        c << "            atts->Set"<<Name<<"(val);" << Endl;
        c << "        else" << Endl;
        c << "        {" << Endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << Endl;
        c << "                BoolsToQString(atts->Get"<<Name<<"()));" << Endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << Endl;
        c << "        }" << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(BoolsToQString(atts->Get"<<Name<<"()));" << Endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"ProcessText()" << Endl;
        c << "{" << Endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};


//
// ----------------------------------- Float ----------------------------------
//
class WindowGeneratorFloat : public virtual Float , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorFloat(const QString &n, const QString &l)
        : Field("float",n,l), Float(n,l), WindowGeneratorField("float",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << Endl
          << "            this, SLOT("<<name<<"ProcessText()));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual bool            providesSourceGetCurrent() const { return true; }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        float val;" << Endl;
        c << "        if(LineEditGetFloat("<<name<<", val))" <<Endl;
        c << "            atts->Set"<<Name<<"(val);" << Endl;
        c << "        else" << Endl;
        c << "        {" << Endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << Endl;
        c << "                FloatToQString(atts->Get"<<Name<<"()));" << Endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << Endl;
        c << "        }" << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(FloatToQString(atts->Get"<<Name<<"()));" << Endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"ProcessText()" << Endl;
        c << "{" << Endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};


//
// -------------------------------- FloatArray -------------------------------
//
class WindowGeneratorFloatArray : public virtual FloatArray , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorFloatArray(const QString &s, const QString &n, const QString &l)
        : Field("floatArray",n,l), FloatArray(s,n,l), WindowGeneratorField("floatArray",n,l) { }
    virtual void               writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << Endl;
    }
    virtual void               writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << Endl;
    }
    virtual void               writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << Endl
          << "            this, SLOT("<<name<<"ProcessText()));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual bool               providesSourceGetCurrent() const { return true; }
    virtual void               writeSourceGetCurrent(QTextStream &c)
    {
        c << "        float val["<<length<<"];"<<Endl;
        c << "        if(LineEditGetFloats("<<name<<", val, " <<length<<"))" <<Endl;
        c << "            atts->Set"<<Name<<"(val);" << Endl;
        c << "        else" << Endl;
        c << "        {" << Endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << Endl;
        c << "                FloatsToQString(atts->Get"<<Name<<"()," << length <<"));" << Endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << Endl;
        c << "        }" << Endl;
    }
    virtual void               writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(FloatsToQString(atts->Get"<<Name<<"(), "<<length<<");" << Endl;
    }
    virtual void               writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"ProcessText()" << Endl;
        c << "{" << Endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};


//
// -------------------------------- FloatVector ------------------------------
//
class WindowGeneratorFloatVector : public virtual FloatVector , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorFloatVector(const QString &n, const QString &l)
        : Field("floatVector",n,l), FloatVector(n,l), WindowGeneratorField("floatVector",n,l) { }
    virtual void               writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << Endl;
    }
    virtual void               writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << Endl;
    }
    virtual void               writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << Endl
          << "            this, SLOT("<<name<<"ProcessText()));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual bool               providesSourceGetCurrent() const { return true; }
    virtual void               writeSourceGetCurrent(QTextStream &c)
    {
        c << "        floatVector val;"<<Endl;
        c << "        if(LineEditGetFloats("<<name<<", val))" <<Endl;
        c << "            atts->Set"<<Name<<"(val);" << Endl;
        c << "        else" << Endl;
        c << "        {" << Endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << Endl;
        c << "                FloatsToQString(atts->Get"<<Name<<"()));" << Endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << Endl;
        c << "        }" << Endl;
    }
    virtual void               writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(FloatsToQString(atts->Get"<<Name<<"()));" << Endl;
    }
    virtual void               writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"ProcessText()" << Endl;
        c << "{" << Endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};

//
// ---------------------------------- Double ----------------------------------
//
class WindowGeneratorDouble : public virtual Double , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorDouble(const QString &n, const QString &l)
        : Field("double",n,l), Double(n,l), WindowGeneratorField("double",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << Endl
          << "            this, SLOT("<<name<<"ProcessText()));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual bool            providesSourceGetCurrent() const { return true; }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        double val;" << Endl;
        c << "        if(LineEditGetDouble("<<name<<", val))" <<Endl;
        c << "            atts->Set"<<Name<<"(val);" << Endl;
        c << "        else" << Endl;
        c << "        {" << Endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << Endl;
        c << "                DoubleToQString(atts->Get"<<Name<<"()));" << Endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << Endl;
        c << "        }" << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(DoubleToQString(atts->Get"<<Name<<"()));" << Endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"ProcessText()" << Endl;
        c << "{" << Endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};


//
// -------------------------------- DoubleArray -------------------------------
//
class WindowGeneratorDoubleArray : public virtual DoubleArray , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorDoubleArray(const QString &s, const QString &n, const QString &l)
        : Field("doubleArray",n,l), DoubleArray(s,n,l), WindowGeneratorField("doubleArray",n,l) { }
    virtual void               writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << Endl;
    }
    virtual void               writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << Endl;
    }
    virtual void               writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << Endl
          << "            this, SLOT("<<name<<"ProcessText()));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual bool               providesSourceGetCurrent() const { return true; }
    virtual void               writeSourceGetCurrent(QTextStream &c)
    {
        c << "        double val["<<length<<"];"<<Endl;
        c << "        if(LineEditGetDoubles("<<name<<", val, " <<length<<"))" <<Endl;
        c << "            atts->Set"<<Name<<"(val);" << Endl;
        c << "        else" << Endl;
        c << "        {" << Endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << Endl;
        c << "                DoublesToQString(atts->Get"<<Name<<"()," << length <<"));" << Endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << Endl;
        c << "        }" << Endl;
    }
    virtual void               writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(DoublesToQString(atts->Get"<<Name<<"(), "<<length<<"));" << Endl;
    }
    virtual void               writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"ProcessText()" << Endl;
        c << "{" << Endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};


//
// -------------------------------- DoubleVector ------------------------------
//
class WindowGeneratorDoubleVector : public virtual DoubleVector , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorDoubleVector(const QString &n, const QString &l)
        : Field("doubleVector",n,l), DoubleVector(n,l), WindowGeneratorField("doubleVector",n,l) { }
    virtual void               writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << Endl;
    }
    virtual void               writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << Endl;
    }
    virtual void               writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << Endl
          << "            this, SLOT("<<name<<"ProcessText()));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual bool               providesSourceGetCurrent() const { return true; }
    virtual void               writeSourceGetCurrent(QTextStream &c)
    {
        c << "        doubleVector val;"<<Endl;
        c << "        if(LineEditGetDoubles("<<name<<", val))" <<Endl;
        c << "            atts->Set"<<Name<<"(val);" << Endl;
        c << "        else" << Endl;
        c << "        {" << Endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << Endl;
        c << "                DoublesToQString(atts->Get"<<Name<<"()));" << Endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << Endl;
        c << "        }" << Endl;
    }
    virtual void               writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(DoublesToQString(atts->Get"<<Name<<"()));" << Endl;
    }
    virtual void               writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"ProcessText()" << Endl;
        c << "{" << Endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};


//
// ----------------------------------- UChar ----------------------------------
//
class WindowGeneratorUChar : public virtual UChar , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorUChar(const QString &n, const QString &l)
        : Field("uchar",n,l), UChar(n,l), WindowGeneratorField("uchar",n,l) { }
};


//
// -------------------------------- UCharArray --------------------------------
//
class WindowGeneratorUCharArray : public virtual UCharArray , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorUCharArray(const QString &s, const QString &n, const QString &l)
        : Field("ucharArray",n,l), UCharArray(s,n,l), WindowGeneratorField("ucharArray",n,l) { }
};

//
// -------------------------------- UCharVector --------------------------------
//
class WindowGeneratorUCharVector : public virtual UCharVector , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorUCharVector(const QString &n, const QString &l)
        : Field("ucharVector",n,l), UCharVector(n,l), WindowGeneratorField("ucharVector",n,l) { }
};


//
// ---------------------------------- String ----------------------------------
//
class WindowGeneratorString : public virtual String , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorString(const QString &n, const QString &l)
        : Field("string",n,l), String(n,l), WindowGeneratorField("string",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << Endl
          << "            this, SLOT("<<name<<"ProcessText()));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual bool            providesSourceGetCurrent() const { return true; }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        QString temp = "<<name<<"->displayText();" << Endl;
        c << "        if(!temp.isEmpty())" << Endl;
        c << "            atts->Set"<<Name<<"(temp.toStdString());" << Endl;
        c << "        else" << Endl;
        c << "        {" << Endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << Endl;
        c << "                QString(atts->Get"<<Name<<"().c_str()));" << Endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << Endl;
        c << "        }" << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(QString(atts->Get"<<Name<<"().c_str()));" << Endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"ProcessText()" << Endl;
        c << "{" << Endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};


//
// ------------------------------- StringVector -------------------------------
//
class WindowGeneratorStringVector : public virtual StringVector , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorStringVector(const QString &n, const QString &l)
        : Field("stringVector",n,l), StringVector(n,l), WindowGeneratorField("stringVector",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << Endl
          << "            this, SLOT("<<name<<"ProcessText()));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual bool            providesSourceGetCurrent() const { return true; }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        QString temp = "<<name<<"->displayText();" << Endl;
        c << "        if(!temp.isEmpty())" << Endl;
        c << "        {" << Endl;
        c << "            QStringList s = temp.split(\" \", QString::SkipEmptyParts);" << Endl;
        c << "            stringVector sv;" << Endl;
        c << "            for(int i = 0; i < s.size(); ++i)" << Endl;
        c << "                sv.push_back(s[i].toStdString());" << Endl;
        c << "            atts->Set"<<Name<<"(sv);" << Endl;
        c << "        }" << Endl;
        c << "        else" << Endl;
        c << "        {" << Endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << Endl;
        c << "        }" << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            {" << Endl;
        c << "            QString stmp;" << Endl;
        c << "            for(size_t idx = 0; idx < atts->Get"<<Name<<"().size(); ++idx)" << Endl;
        c << "            {" << Endl;
        c << "                stmp.append(QString(atts->Get"<<Name<<"()[idx].c_str()));" << Endl;
        c << "                if(idx < atts->Get"<<Name<<"().size()-1)" << Endl;
        c << "                    stmp.append(\" \");" << Endl;
        c << "            }" << Endl;
        c << "            "<<name<<"->setText(stmp);" << Endl;
        c << "            }" << Endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"ProcessText()" << Endl;
        c << "{" << Endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};


//
// -------------------------------- ColorTable --------------------------------
//
class WindowGeneratorColorTable : public virtual ColorTable , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorColorTable(const QString &n, const QString &l)
        : Field("colortable",n,l), ColorTable(n,l), WindowGeneratorField("colortable",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"Changed(bool useDefault, const QString &ctName);" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QvisColorTableButton *"<<name<<";" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QvisColorTableButton(central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(selectedColorTable(bool, const QString&))," << Endl
          << "            this, SLOT("<<name<<"Changed(bool, const QString&)));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->blockSignals(true);" << Endl;
        c << "            "<<name<<"->setColorTable(QString(atts->Get"<<Name<<"().c_str()));" << Endl;
        c << "            "<<name<<"->blockSignals(false);" << Endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"Changed(bool useDefault, const QString &ctName)" << Endl;
        c << "{" << Endl;
        c << "    atts->Set"<<Name<<"(ctName.toStdString());" << Endl;
        if(!isEnabler)
            c << "    SetUpdate(false);" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};


//
// ----------------------------------- Color ----------------------------------
//
class WindowGeneratorColor : public virtual Color , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorColor(const QString &n, const QString &l)
        : Field("color",n,l), Color(n,l), WindowGeneratorField("color",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"Changed(const QColor &color);" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QvisColorButton *"<<name<<";" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QvisColorButton(central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(selectedColor(const QColor&))," << Endl
          << "            this, SLOT("<<name<<"Changed(const QColor&)));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            { // new scope" << Endl;
        c << "                QColor tempcolor = QColor(atts->Get"<<Name<<"().Red()," << Endl;
        c << "                                   atts->Get"<<Name<<"().Green()," << Endl;
        c << "                                   atts->Get"<<Name<<"().Blue());" << Endl;
        c << "                "<<name<<"->blockSignals(true);" << Endl;
        c << "                "<<name<<"->setButtonColor(tempcolor);" << Endl;
        c << "                "<<name<<"->blockSignals(false);" << Endl;
        c << "            }" << Endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"Changed(const QColor &color)" << Endl;
        c << "{" << Endl;
        c << "    ColorAttribute temp(color.red(), color.green(), color.blue());" << Endl;
        c << "    atts->Set"<<Name<<"(temp);" << Endl;
        if(!isEnabler)
            c << "    SetUpdate(false);" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};


//
// --------------------------------- Opacity ----------------------------------
//
class WindowGeneratorOpacity : public virtual Opacity , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorOpacity(const QString &n, const QString &l)
        : Field("opacity",n,l), Opacity(n,l), WindowGeneratorField("opacity",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"Changed(int opacity, const void*);" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QvisOpacitySlider *"<<name<<";" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QvisOpacitySlider(0,255,25,255, central, NULL);" << Endl;
        c << "    "<<name<<"->setTickInterval(64);" << Endl;
        c << "    "<<name<<"->setGradientColor(QColor(0, 0, 0));" << Endl;
        c << "    connect("<<name<<", SIGNAL(valueChanged(int, const void*))," << Endl
          << "            this, SLOT("<<name<<"Changed(int, const void*)));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->blockSignals(true);" << Endl;
        c << "            "<<name<<"->setValue(int(atts->Get"<<Name<<"()*255.));" << Endl;
        c << "            "<<name<<"->blockSignals(false);" << Endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"Changed(int opacity, const void*)" << Endl;
        c << "{" << Endl;
        c << "    atts->Set"<<Name<<"((float)opacity/255.);" << Endl;
        if(!isEnabler)
            c << "    SetUpdate(false);" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};



//
// -------------------------------- LineWidth --------------------------------
//
class WindowGeneratorLineWidth : public virtual LineWidth , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorLineWidth(const QString &n, const QString &l)
        : Field("linewidth",n,l), LineWidth(n,l), WindowGeneratorField("linewidth",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"Changed(int style);" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QvisLineWidthWidget *"<<name<<";" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QvisLineWidthWidget(0, central);" << Endl;
        c << "    connect("<<name<<", SIGNAL(lineWidthChanged(int))," << Endl
          << "            this, SLOT("<<name<<"Changed(int)));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->blockSignals(true);" << Endl;
        c << "            "<<name<<"->SetLineWidth(atts->Get"<<Name<<"());" << Endl;
        c << "            "<<name<<"->blockSignals(false);" << Endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"Changed(int style)" << Endl;
        c << "{" << Endl;
        c << "    atts->Set"<<Name<<"(style);" << Endl;
        if(!isEnabler)
            c << "    SetUpdate(false);" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};


//
// -------------------------------- VariableName --------------------------------
//
//  Modifications:
//
//    Hank Childs, Tue Jul 19 14:00:57 PDT 2005
//    Added support for labels.
//
//    Jeremy Meredith, Thu Mar 19 12:11:54 EDT 2009
//    Added missing comma, which led to a poorly formed array.

class WindowGeneratorVariableName : public virtual VariableName , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorVariableName(const QString &n, const QString &l)
        : Field("variablename",n,l), VariableName(n,l), WindowGeneratorField("variablename",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"Changed(const QString &varName);" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QvisVariableButton *"<<name<<";" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
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
            "QvisVariableButton::SymmetricTensors",
            "QvisVariableButton::Labels",
            "QvisVariableButton::Arrays"
        };

        // Write the mask.
        c << "    int " << name <<"Mask = ";
        int m = 1, count = 0;
        for(int i = 0; i < 11; ++i)
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
        c << ";" << Endl;
        c << "    "<<name<<" = new QvisVariableButton(true, true, true, "<<name<<"Mask, central);" << Endl;
        if(valueSet)
            c << "    " << name << "->setDefaultVariable(\"" << val << "\");" << Endl;
        c << "    connect("<<name<<", SIGNAL(activated(const QString&))," << Endl
          << "            this, SLOT("<<name<<"Changed(const QString&)));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->blockSignals(true);" << Endl;
        c << "            "<<name<<"->setText(QString(atts->Get"<<Name<<"().c_str()));" << Endl;
        c << "            "<<name<<"->blockSignals(false);" << Endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"Changed(const QString &varName)" << Endl;
        c << "{" << Endl;
        c << "    atts->Set"<<Name<<"(varName.toStdString());" << Endl;
        c << "    SetUpdate(false);" << Endl;
        c << "    Apply();" << Endl;
        c << "}" << Endl;
    }
};


//
// ------------------------------------ Att -----------------------------------
//
class WindowGeneratorAtt : public virtual Att , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorAtt(const QString &t, const QString &n, const QString &l)
        : Field("att",n,l), Att(t,n,l), WindowGeneratorField("att",n,l) { }
};


//
// --------------------------------- AttVector --------------------------------
//
class WindowGeneratorAttVector : public virtual AttVector , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorAttVector(const QString &t, const QString &n, const QString &l)
        : Field("attVector",n,l), AttVector(t,n,l), WindowGeneratorField("attVector",n,l) { }
};


//
// ----------------------------------- Enum -----------------------------------
//
class WindowGeneratorEnum : public virtual Enum , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorEnum(const QString &t, const QString &n, const QString &l)
        : Field("enum",n,l), Enum(t,n,l), WindowGeneratorField("enum",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"Changed(int val);" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QWidget      *"<<name<<";" << Endl;
        h << "    QButtonGroup *"<<name<<"ButtonGroup;" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QWidget(central);" << Endl;
        c << "    "<<name<<"ButtonGroup= new QButtonGroup("<<name<<");" << Endl;
        c << "    QHBoxLayout *"<<name<<"Layout = new QHBoxLayout("<<name<<");" << Endl;
        c << "    "<<name<<"Layout->setContentsMargins(0,0,0,0);" << Endl;
        c << "    "<<name<<"Layout->setSpacing(10);" << Endl;

        for (size_t i=0; i<enumType->values.size(); i++)
        {
            c << "    QRadioButton *"<<name<<enumType->type<<enumType->values[i]
              << " = new QRadioButton(tr(\""<<enumType->values[i]<<"\"), "<<name<<");" << Endl;
            c << "    "<<name<<"ButtonGroup->addButton("<<name<<enumType->type<<enumType->values[i]<<","<<i<<");"<<Endl;
            c << "    "<<name<<"Layout->addWidget("<<name<<enumType->type<<enumType->values[i]<<");" << Endl;
        }

        c << "    connect("<<name<<"ButtonGroup, SIGNAL(buttonClicked(int))," << Endl
          << "            this, SLOT("<<name<<"Changed(int)));" << Endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << Endl;
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"ButtonGroup->blockSignals(true);" << Endl;
        c << "            if("<<name<<"ButtonGroup->button((int)atts->Get"<<Name<<"()) != 0)" << Endl;
        c << "                "<<name<<"ButtonGroup->button((int)atts->Get"<<Name<<"())->setChecked(true);" << Endl;
        c << "            "<<name<<"ButtonGroup->blockSignals(false);" << Endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"Changed(int val)" << Endl;
        c << "{" << Endl;
        c << "    if(val != atts->Get"<<Name<<"())" << Endl;
        c << "    {" << Endl;
        c << "        atts->Set"<<Name<<"("<<GetCPPName(true,classname)<<"(val));" << Endl;
        if(!isEnabler)
            c << "        SetUpdate(false);" << Endl;
        c << "        Apply();" << Endl;
        c << "    }" << Endl;
        c << "}" << Endl;
    }
};


//
// -------------------------------- ScaleMode ---------------------------------
//
class WindowGeneratorScaleMode : public virtual ScaleMode , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorScaleMode(const QString &n, const QString &l)
        : Field("scalemode",n,l), ScaleMode(n,l), WindowGeneratorField("scalemode",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"Changed(int val);" << Endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QWidget      *"<<name<<";" << Endl;
        h << "    QButtonGroup *"<<name<<"ButtonGroup;" << Endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    QWidget *"<<name<<" = new QWidget(central);"<<Endl;
        c << "    "<<name<<"ButtonGroup = new QButtonGroup("<<name<<");" << Endl;
        c << "    QHBoxLayout *"<<name<<"Layout = new QHBoxLayout("<<name<<");" << Endl;
        c << "    "<<name<<"Layout->setContentsMargins(0,0,0,0);" << Endl;
        c << "    "<<name<<"Layout->setSpacing(10);" << Endl;

        c << "    QRadioButton *"<<name<<"ScaleModeLinear"
          << " = new QRadioButton(tr(\""<<"Linear"<<"\"), "<<name<<");" << Endl;
        c << "    "<<name<<"Layout->addWidget("<<name<<"ScaleModeLinear"<<");" << Endl;
        c << "    "<<name<<"ButtonGroup->addButton("<<name<<"ScaleModeLinear,0);"<<Endl;
        c << "    QRadioButton *"<<name<<"ScaleModeLog"
          << " = new QRadioButton(tr(\""<<"Log"<<"\"), "<<name<<");" << Endl;
        c << "    "<<name<<"ButtonGroup->addButton("<<name<<"ScaleModeLog,1);"<<Endl;
        c << "    "<<name<<"Layout->addWidget("<<name<<"ScaleModeLog"<<");" << Endl;

        c << "    connect("<<name<<"ButtonGroup, SIGNAL(buttonClicked(int))," << Endl
          << "            this, SLOT("<<name<<"Changed(int)));" << Endl;
        c << "    mainLayout->addWidget("<<name<<"Widget, "<<index<<",1);" << Endl;
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << Endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"ButtonGroup->blockSignals(true);" << Endl;
        c << "            if("<<name<<"ButtonGroup->button(atts->Get"<<Name<<"()) != 0)"<<Endl;
        c << "                "<<name<<"ButtonGroup->button(atts->Get"<<Name<<"())->setChecked(true);" << Endl;
        c << "            "<<name<<"ButtonGroup->blockSignals(false);" << Endl;
    }
    ///TODO: check fix for overloaded virtual function writeSourceCallback (4 vs 3) params
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool /*isEnabler*/)
    {
        c << "void" << Endl;
        c << windowname<<"::"<<name<<"Changed(int val)" << Endl;
        c << "{" << Endl;
        c << "    if(val != atts->Get"<<Name<<"())" << Endl;
        c << "    {" << Endl;
        c << "        atts->Set"<<Name<<"("<<GetCPPName(true,classname)<<"(val));" << Endl;
        c << "        SetUpdate(false);" << Endl;
        c << "        Apply();" << Endl;
        c << "    }" << Endl;
        c << "}" << Endl;
    }
};


// ----------------------------------------------------------------------------
// Modifications:
//    Brad Whitlock, Wed Dec 8 16:12:01 PST 2004
//    Added support for variable names.
//
//    Kathleen Bonnell, Thu Mar 22 16:58:23 PDT 2007
//    Added scalemode.
//
//    Kathleen Biagas, Tue Nov 15 12:39:09 PST 2022
//    Added boolArray and boolVector.
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
        if      (type.isNull())          throw QString("Field %1 was specified with no type.").arg(name);
        else if (type == "int")          f = new WindowGeneratorInt(name,label);
        else if (type == "intArray")     f = new WindowGeneratorIntArray(length,name,label);
        else if (type == "intVector")    f = new WindowGeneratorIntVector(name,label);
        else if (type == "bool")         f = new WindowGeneratorBool(name,label);
        else if (type == "boolArray")    f = new WindowGeneratorBoolArray(length,name,label);
        else if (type == "boolVector")   f = new WindowGeneratorBoolVector(name,label);
        else if (type == "float")        f = new WindowGeneratorFloat(name,label);
        else if (type == "floatArray")   f = new WindowGeneratorFloatArray(length,name,label);
        else if (type == "floatVector")  f = new WindowGeneratorFloatVector(name,label);
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
        else if (type == "linewidth")    f = new WindowGeneratorLineWidth(name,label);
        else if (type == "variablename") f = new WindowGeneratorVariableName(name,label);
        else if (type == "att")          f = new WindowGeneratorAtt(subtype,name,label);
        else if (type == "attVector")    f = new WindowGeneratorAttVector(subtype,name,label);
        else if (type == "enum")         f = new WindowGeneratorEnum(subtype, name, label);
        else if (type == "scalemode")    f = new WindowGeneratorScaleMode(name, label);

        // Special built-in AVT enums -- but they don't really need to be treated like enums for this program.
        else if (type == "avtCentering")      f = new WindowGeneratorInt(name, label);
        else if (type == "avtVarType")        f = new WindowGeneratorInt(name, label);
        else if (type == "avtSubsetType")     f = new WindowGeneratorInt(name, label);
        else if (type == "avtExtentType")     f = new WindowGeneratorInt(name, label);
        else if (type == "avtMeshType")       f = new WindowGeneratorInt(name, label);
        else if (type == "avtGhostType")      f = new WindowGeneratorInt(name, label);
        else if (type == "avtMeshCoordType")  f = new WindowGeneratorInt(name, label);
        else if (type == "LoadBalanceScheme") f = new WindowGeneratorInt(name, label);

        if (!f)
            throw QString("WindowFieldFactory: unknown type for field %1: %2").arg(name).arg(type);

        return f;
    }
};

// ----------------------------------------------------------------------------
// Modifications:
//   Brad Whitlock, Thu Feb 28 12:03:30 PDT 2008
//   Made it use a base class so it can inherit some methods.
//
//    Mark C. Miller, Wed Aug 26 10:59:13 PDT 2009
//    Added custom base class for derived state objects.
// ----------------------------------------------------------------------------
#include "GeneratorBase.h"

class WindowGeneratorAttribute : public GeneratorBase
{
  public:
    std::vector<WindowGeneratorField*> fields;
    QString windowname;
    QString plugintype;
  public:
    WindowGeneratorAttribute(const QString &n, const QString &p, const QString &f,
                             const QString &e, const QString &ei, const QString &bc)
        : GeneratorBase(n,p,f,e,ei, GENERATOR_NAME, bc), fields(),
          windowname(), plugintype()
    {
        plugintype = "";
        if (name.right(9) == "Attribute")
            windowname = QString("Qvis") + name.left(name.length() - 9) + "Window";
        else if (name.right(10) == "Attributes")
            windowname = QString("Qvis") + name.left(name.length() - 10) + "Window";
        else
            windowname = QString("Qvis") + name + QString("Window");
    }
    virtual ~WindowGeneratorAttribute()
    {
        for (size_t i = 0; i < fields.size(); ++i)
            delete fields[i];
        fields.clear();
    }

    void Print(QTextStream &out) const
    {
        out << "    Attribute: " << name << " (" << purpose << ")" << Endl;
        size_t i;
        for (i=0; i<fields.size(); i++)
            fields[i]->Print(out);
        for (i=0; i<functions.size(); i++)
            functions[i]->Print(out);
    }

    bool RequiresGetCurrentValues() const
    {
        int n = 0;
        for (size_t i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal || !fields[i]->providesSourceGetCurrent())
                continue;
            ++n;
        }
        return n > 0;
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
    //   Hank Childs, Fri Jun  9 09:54:46 PDT 2006
    //   Added copyright string.
    //
    //   Brad Whitlock, Wed Apr  9 12:49:00 PDT 2008
    //   Use QString for the caption, shortName.
    //
    //   Kathleen Biagas, Wed Jun  8 16:57:39 PDT 2016
    //   Only forward declare color/oapcity/line widgets for plots.
    //
    // ************************************************************************

    void WriteHeader(QTextStream &h)
    {
        h << copyright_str << Endl;
        h << "#ifndef " << windowname.toUpper() << "_H" << Endl;
        h << "#define " << windowname.toUpper() << "_H" << Endl;
        h << Endl;
        if(plugintype == "operator")
            h << "#include <QvisOperatorWindow.h>" << Endl;
        else if(plugintype == "plot")
            h << "#include <QvisPostableWindowObserver.h>" << Endl;
        h << "#include <AttributeSubject.h>" << Endl;
        h << Endl;
        h << "class " << name << ";" << Endl;
        h << "class QLabel;" << Endl;
        h << "class QCheckBox;" << Endl;
        h << "class QLineEdit;" << Endl;
        h << "class QSpinBox;" << Endl;
        h << "class QButtonGroup;" << Endl;
        if(plugintype == "plot")
        {
            h << "class QvisColorTableButton;" << Endl;
            h << "class QvisOpacitySlider;" << Endl;
            h << "class QvisColorButton;" << Endl;
            h << "class QvisLineWidthWidget;" << Endl;
        }
        h << "class QvisVariableButton;" << Endl;
        h << Endl;

        WriteClassComment(h, windowname, QString("Defines ") + windowname + QString(" class."));

        if(plugintype == "operator")
            h << "class " << windowname << " : public QvisOperatorWindow" << Endl;
        else if(plugintype == "plot")
            h << "class " << windowname << " : public QvisPostableWindowObserver" << Endl;
        else
            h << "class " << windowname << " : public QvisPostableWindowObserver" << Endl;
        h << "{" << Endl;
        h << "    Q_OBJECT" << Endl;
        h << "  public:" << Endl;
        h << "    " << windowname << "("
          << (plugintype=="" ? "" : "const int type,") << Endl
          << "                         "<<name<<" *subj," << Endl
          << "                         const QString &caption = QString()," << Endl
          << "                         const QString &shortName = QString()," << Endl
          << "                         QvisNotepadArea *notepad = 0);" << Endl;
        h << "    virtual ~"<<windowname<<"();" << Endl;
        h << "    virtual void CreateWindowContents();" << Endl;
        if(plugintype != "operator")
        {
            h << "  public slots:" << Endl;
            h << "    virtual void apply();" << Endl;
            h << "    virtual void makeDefault();" << Endl;
            h << "    virtual void reset();" << Endl;
        }
        h << "  protected:" << Endl;
        h << "    void UpdateWindow(bool doAll);" << Endl;
        if(plugintype == "operator")
            h << "    virtual void GetCurrentValues(int which_widget);" << Endl;
        else if(plugintype == "plot")
        {
            h << "    void GetCurrentValues(int which_widget);" << Endl;
            h << "    void Apply(bool ignore = false);" << Endl;
        }
        else
        {
            h << "    void GetCurrentValues(int which_widget);" << Endl;
            h << "    void Apply(bool ignore = false);" << Endl;
        }
        h << "  private slots:" << Endl;
        size_t i;
        for (i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal) continue;
            fields[i]->writeHeaderCallback(h);
        }
        h << "  private:" << Endl;
        if (plugintype == "plot")
            h << "    int plotType;" << Endl;
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
        h << Endl;
        h << "    "<<name<<" *atts;" << Endl;

        h << "};" << Endl;
        h << Endl;
        h << Endl;
        h << Endl;

        h << "#endif" << Endl;
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
    //   Hank Childs, Fri Jun  9 09:54:46 PDT 2006
    //   Added copyright string.
    //
    //   Brad Whitlock, Wed Apr  9 12:49:53 PDT 2008
    //   Use QString for caption, shortName.
    //
    //   Dave Pugmire, Wed Sep  5 11:01:33 EDT 2012
    //   Enablers produced bad code for checkboxes. There is no label. Added
    //   a virtual method, HasLabel() to determine if code should be generated.
    //
    //   Kathleen Biagas, Wed Jun  8 16:57:39 PDT 2016
    //   Only include ViewerProxy and color/oapcity/line widgets for plots.
    //
    // ************************************************************************

    void WriteSource(QTextStream &c)
    {
        c << copyright_str << Endl;
        c << "#include \""<<windowname<<".h\"" << Endl;
        c << Endl;
        c << "#include <"<<name<<".h>" << Endl;
        if(plugintype == "plot")
            c << "#include <ViewerProxy.h>" << Endl;
        c << Endl;
        c << "#include <QCheckBox>" << Endl;
        c << "#include <QLabel>" << Endl;
        c << "#include <QLayout>" << Endl;
        c << "#include <QLineEdit>" << Endl;
        c << "#include <QSpinBox>" << Endl;
        c << "#include <QButtonGroup>" << Endl;
        c << "#include <QRadioButton>" << Endl;
        if(plugintype == "plot")
        {
            c << "#include <QvisColorTableButton.h>" << Endl;
            c << "#include <QvisOpacitySlider.h>" << Endl;
            c << "#include <QvisColorButton.h>" << Endl;
            c << "#include <QvisLineWidthWidget.h>" << Endl;
        }
        c << "#include <QvisVariableButton.h>" << Endl;
        c << Endl;
        c << Endl;

        // constructor
        WriteMethodComment(c, windowname, windowname, "Constructor");
        c << windowname<<"::"<<windowname<<"("
          << (plugintype=="" ? "" : "const int type,") << Endl
          << "                         "<<name<<" *subj," << Endl
          << "                         const QString &caption," << Endl
          << "                         const QString &shortName," << Endl
          << "                         QvisNotepadArea *notepad)" << Endl;
        if(plugintype == "operator")
        {
            c << "    : QvisOperatorWindow(type,subj, caption, shortName, notepad)" << Endl;
            c << "{" << Endl;
            c << "    atts = subj;" << Endl;
            c << "}" << Endl;
        }
        else if(plugintype == "plot")
        {
            c << "    : QvisPostableWindowObserver(subj, caption, shortName, notepad)" << Endl;
            c << "{" << Endl;
            c << "    plotType = type;" << Endl;
            c << "    atts = subj;" << Endl;
            c << "}" << Endl;
        }
        else
        {
            c << "    : QvisPostableWindowObserver(subj, caption, shortName, notepad)" << Endl;
            c << "{" << Endl;
            c << "    atts = subj;" << Endl;
            c << "}" << Endl;
        }
        c << Endl;
        c << Endl;

        // destructor
        WriteMethodComment(c, windowname, QString("~") + windowname, "Destructor");
        c << windowname<<"::~"<<windowname<<"()" << Endl;
        c << "{" << Endl;
        c << "}" << Endl;
        c << Endl;
        c << Endl;

        // CreateWindowContents
        WriteMethodComment(c, windowname, "CreateWindowContents", "Creates the widgets for the window.");
        c << "void" << Endl;
        c << windowname<<"::CreateWindowContents()" << Endl;
        c << "{" << Endl;
        c << "    QGridLayout *mainLayout = new QGridLayout(0);" << Endl;
        c << "    topLayout->addLayout(mainLayout);" << Endl;
        c << Endl;
        size_t i;
        for (i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal) continue;
            fields[i]->writeSourceCreate(c);
            c << Endl;
        }
        c << "}" << Endl;
        c << Endl;
        c << Endl;

        // updatewindow
        WriteMethodComment(c, windowname, "UpdateWindow",
                           "Updates the widgets in the window when the subject changes.");
        c << "void" << Endl;
        c << windowname << "::UpdateWindow(bool doAll)" << Endl;
        c << "{" << Endl;
        c << "" << Endl;
        c << "    for(int i = 0; i < atts->NumAttributes(); ++i)" << Endl;
        c << "    {" << Endl;
        c << "        if(!doAll)" << Endl;
        c << "        {" << Endl;
        c << "            if(!atts->IsSelected(i))" << Endl;
        c << "            {" << Endl;
        c << "                continue;" << Endl;
        c << "            }" << Endl;
        c << "        }" << Endl;
        c << "" << Endl;
        c << "        switch(i)" << Endl;
        c << "        {" << Endl;
        for (i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal) continue;

            WindowGeneratorField *field   = fields[i];
            c << "          case "<<name<<"::ID_"<<field->name << ":" << Endl;

            std::vector<Field*> enablees;
            size_t j;
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

                for (size_t k=1; k<enablees[j]->enableval.size(); k++)
                {
                    c << " || atts->Get"<<field->Name<<"() == ";
                    if (fields[i]->type=="enum")
                        c << name<<"::"<<enablees[j]->enableval[k];
                    else
                        c << enablees[j]->enableval[k];
                }

                c << ")" << Endl;
                c << "            {\n";
                c << "                "<<enablees[j]->name<<"->setEnabled(true);" << Endl;
                if (enablees[j]->HasLabel())
                {
                    c << "                if("<<enablees[j]->name<<"Label)" << Endl;
                    c << "                    "<<enablees[j]->name<<"Label->setEnabled(true);" << Endl;
                }
                c << "            }\n";
                c << "            else" << Endl;
                c << "            {\n";
                c << "                "<<enablees[j]->name<<"->setEnabled(false);" << Endl;
                if (enablees[j]->HasLabel())
                {
                    c << "                if("<<enablees[j]->name<<"Label)" << Endl;
                    c << "                    "<<enablees[j]->name<<"Label->setEnabled(false);" << Endl;
                }
                c << "            }\n";
            }
            fields[i]->writeSourceUpdateWindow(c);
            c << "            break;" << Endl;
        }
        c << "        }" << Endl;
        c << "    }" << Endl;
        c << "}" << Endl;
        c << Endl;
        c << Endl;

        // getcurrent
        WriteMethodComment(c, windowname, "GetCurrentValues",
                           "Gets values from certain widgets and stores them in the subject.");
        c << "void" << Endl;
        c << windowname << "::GetCurrentValues(int which_widget)" << Endl;
        c << "{" << Endl;
        if(RequiresGetCurrentValues())
        {
            c << "    bool doAll = (which_widget == -1);" << Endl;
            c << Endl;
        }
        for (i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal || !fields[i]->providesSourceGetCurrent())
                continue;
            c << "    // Do " << fields[i]->name << Endl;
            c << "    if(which_widget == "<<name << "::ID_" << fields[i]->name<<" || doAll)" << Endl;
            c << "    {" << Endl;
            fields[i]->writeSourceGetCurrent(c);
            c << "    }" << Endl;
            c << Endl;
        }
        c << "}" << Endl;
        c << Endl;
        c << Endl;

        if(plugintype != "operator")
        {
            // Apply
            WriteMethodComment(c, windowname, "Apply", "Called to apply changes in the subject.");
            c << "void" << Endl;
            c << windowname<<"::Apply(bool ignore)" << Endl;
            c << "{" << Endl;
            c << "    if(AutoUpdate() || ignore)" << Endl;
            c << "    {" << Endl;
            c << "        GetCurrentValues(-1);" << Endl;
            c << "        atts->Notify();" << Endl;
            c << "" << Endl;
            if (plugintype == "plot")
                c << "        GetViewerMethods()->SetPlotOptions(plotType);" << Endl;
            else if(plugintype != "operator")
                c << "        GetViewerMethods()->Set"<<name<<"();" << Endl;
            c << "    }" << Endl;
            c << "    else" << Endl;
            c << "        atts->Notify();" << Endl;
            c << "}" << Endl;
            c << Endl;
            c << Endl;
        }

        c << "//" << Endl;
        c << "// Qt Slot functions" << Endl;
        c << "//" << Endl;
        c << Endl;
        c << Endl;

        // Only plot plugins need to override these at present. Operators
        // now have a base class that does it.
        if(plugintype != "operator")
        {
            // apply
            WriteMethodComment(c, windowname, "apply", "Qt slot function called when apply button is clicked.");
            c << "void" << Endl;
            c << windowname << "::apply()" << Endl;
            c << "{" << Endl;
            c << "    Apply(true);" << Endl;
            c << "}" << Endl;
            c << Endl;
            c << Endl;

            // makeDefault
            WriteMethodComment(c, windowname, "makeDefault", "Qt slot function called when \"Make default\" button is clicked.");
            c << "void" << Endl;
            c << windowname<<"::makeDefault()" << Endl;
            c << "{" << Endl;
            c << "    GetCurrentValues(-1);" << Endl;
            c << "    atts->Notify();" << Endl;
            if (plugintype == "plot")
                c << "    GetViewerMethods()->SetDefaultPlotOptions(plotType);" << Endl;
            else if(plugintype != "operator")
                c << "    GetViewerMethods()->SetDefault"<<name<<"();" << Endl;
            c << "}" << Endl;
            c << Endl;
            c << Endl;

            // reset
            WriteMethodComment(c, windowname, "reset", "Qt slot function called when reset button is clicked.");
            c << "void" << Endl;
            c << windowname << "::reset()" << Endl;
            c << "{" << Endl;
            if (plugintype == "plot")
                c << "    GetViewerMethods()->ResetPlotOptions(plotType);" << Endl;
            else if(plugintype != "operator")
                c << "    GetViewerMethods()->Reset"<<name<<"();" << Endl;
            c << "}" << Endl;
            c << Endl;
            c << Endl;
        }

        // Create a set of the fields that enable other fields so we can
        // influence how the source callback methods get "SetUpdate(false)".
        std::set<Field*> enablers;
        for (i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal) continue;
            for (size_t j=0; j<fields.size(); j++)
                if (fields[j]->enabler == fields[i])
                    enablers.insert(fields[i]);
        }

        // callbacks
        for (i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal) continue;
            bool isEnabler = enablers.find(fields[i]) != enablers.end();
            fields[i]->writeSourceCallback(name, windowname, c, isEnabler);
            c << Endl;
            c << Endl;
        }
    }
};

// ----------------------------------------------------------------------------
//  Modifications:
//
//   Hank Childs, Thu Jan 10 14:33:30 PST 2008
//   Added filenames, specifiedFilenames.
//
//   Brad Whitlock, Wed Mar 5 12:00:40 PDT 2008
//   Made it use a base class.
//
//   Kathleen Biagas, Thu Jan  2 09:18:18 PST 2020
//   Added hl arg, for haslicense.
//
// ----------------------------------------------------------------------------

class WindowGeneratorPlugin : public PluginBase
{
  public:
    WindowGeneratorAttribute *atts;
    QString windowname;
  public:
    WindowGeneratorPlugin(const QString &n,const QString &l,const QString &t,
          const QString &vt,const QString &dt,const QString &v,const QString &ifile,
          bool hw,bool ho,bool hl,bool onlyengine,bool noengine)
        : PluginBase(n,l,t,vt,dt,v,ifile,hw,ho,hl,onlyengine,noengine), atts(NULL), windowname()
    {
        if (type == "plot")
            windowname = QString("Qvis")+name+QString("PlotWindow");
        else if (type == "operator")
            windowname = QString("Qvis")+name+QString("Window");
    }
    void Print(QTextStream &out)
    {
        out << "Plugin: "<<name<<" (\""<<label<<"\", type="<<type<<") -- version "<<version<< Endl;
        if (atts)
            atts->Print(out);
    }
    void WriteHeader(QTextStream &h)
    {
        atts->WriteHeader(h);
    }
    void WriteSource(QTextStream &c)
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
