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
        c << "    "<<name<<"Label = new QLabel(tr(\""<<label<<"\"), central);" << endl;
        c << "    mainLayout->addWidget("<<name<<"Label,"<<index<<",0);" << endl;
    }
    // virtual functions
    virtual void               writeHeaderCallback(QTextStream &h)
    {
        h << "    //writeHeaderCallback unknown for " << type << " (variable " << name << ")" << endl;
    }
    virtual void               writeHeaderLabelData(QTextStream &h)
    {
        h << "    QLabel *"<<name<<"Label;" << endl;
    }
    virtual void               writeHeaderData(QTextStream &h)
    {
        h << "    //writeHeaderData unknown for " << type << " (variable " << name << ")" << endl;
    }
    virtual void               writeSourceCreate(QTextStream &c)
    {
        c << "    //writeSourceCreate unknown for " << type << " (variable " << name << ")" << endl;
    }
    virtual bool               providesSourceGetCurrent() const { return false; }
    virtual void               writeSourceGetCurrent(QTextStream &c)
    {
        c << "        //writeSourceGetCurrent unknown for " << type << " (variable " << name << ")" << endl;
    }
    virtual void               writeSourceUpdateWindow(QTextStream &c)
    {
        c << "        //writeSourceUpdate unknown for " << type << " (variable " << name << ")" << endl;
    }
    virtual void               writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
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
        : Field("int",n,l), Int(n,l), WindowGeneratorField("int",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        if (rangeSet)
            h << "    void "<<name<<"Changed(int val);" << endl;
        else
            h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        if (rangeSet)
            h << "    QSpinBox *"<<name<<";" << endl;
        else
            h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        if (rangeSet)
        {
            c << "    "<<name<<" = new QSpinBox(central);" << endl;
            c << "    "<<name<<"->setMinimum("<<min<<");"<<endl;
            c << "    "<<name<<"->setMaximum("<<max<<");"<<endl;
            c << "    connect("<<name<<", SIGNAL(valueChanged(int)), " << endl
              << "            this, SLOT("<<name<<"Changed(int)));" << endl;
        }
        else
        {
            c << "    "<<name<<" = new QLineEdit(central);" << endl;
            c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
              << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        }
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual bool            providesSourceGetCurrent() const { return !rangeSet; }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        if (rangeSet)
        {
            c << "        // Nothing for " << name << endl;
        }
        else
        {
            c << "        int val;" << endl;
            c << "        if(LineEditGetInt("<<name<<", val))" << endl;
            c << "            atts->Set"<<Name<<"(val);" << endl;
            c << "        else" << endl;
            c << "        {" << endl;
            QString msgLabel = (label.length()>0) ? label : name;
            c << "            ResettingError(tr(\""<<msgLabel<<"\")," << endl;
            c << "                IntToQString(atts->Get"<<Name<<"()));" << endl;
            c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << endl;
            c << "        }" << endl;
        }
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        if (rangeSet)
        {
            c << "            "<<name<<"->blockSignals(true);" << endl;
            c << "            "<<name<<"->setValue(atts->Get"<<Name<<"());" << endl;
            c << "            "<<name<<"->blockSignals(false);" << endl;
        }
        else
        {
            c << "            "<<name<<"->setText(IntToQString(atts->Get"<<Name<<"()));" << endl;
        }
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        if (rangeSet)
        {
            c << "void" << endl;
            c << windowname<<"::"<<name<<"Changed(int val)" << endl;
            c << "{" << endl;
            c << "    atts->Set"<<Name<<"(val);" << endl;
            if(!isEnabler)
                c << "    SetUpdate(false);" << endl;
            c << "    Apply();" << endl;
            c << "}" << endl;
        }
        else
        {
            c << "void" << endl;
            c << windowname<<"::"<<name<<"ProcessText()" << endl;
            c << "{" << endl;
            c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << endl;
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
        : Field("intArray",n,l), IntArray(s,n,l), WindowGeneratorField("intArray",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;        
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual bool            providesSourceGetCurrent() const { return true; }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        int val[" << length << "];" << endl;
        c << "        if(LineEditGetInts("<<name<<", val, " << length << "))" << endl;
        c << "            atts->Set"<<Name<<"(val);" << endl;
        c << "        else" << endl;
        c << "        {" << endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << endl;
        c << "                IntsToQString(atts->Get"<<Name<<"(), "<<length<<"));" << endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(IntsToQString(atts->Get"<<Name<<"(),"<<length<<"));" << endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
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
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;        
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual bool            providesSourceGetCurrent() const { return true; }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        intVector val;" << endl;
        c << "        if(LineEditGetInts("<<name<<", val))" << endl;
        c << "            atts->Set"<<Name<<"(val);" << endl;
        c << "        else" << endl;
        c << "        {" << endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << endl;
        c << "                IntsToQString(atts->Get"<<Name<<"()));" << endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(IntsToQString(atts->Get"<<Name<<"()));" << endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
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
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"Changed(bool val);" << endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QCheckBox *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        c << "    "<<name<<" = new QCheckBox(tr(\""<<label<<"\"), central);" << endl;
        c << "    connect("<<name<<", SIGNAL(toggled(bool))," << endl
          << "            this, SLOT("<<name<<"Changed(bool)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",0);" << endl;
    }
    virtual void            writeHeaderLabelData(QTextStream &h)
    {
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->blockSignals(true);" << endl;
        c << "            "<<name<<"->setChecked(atts->Get"<<Name<<"());" << endl;
        c << "            "<<name<<"->blockSignals(false);" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(bool val)" << endl;
        c << "{" << endl;
        c << "    atts->Set"<<Name<<"(val);" << endl;
        if(!isEnabler)
            c << "    SetUpdate(false);" << endl;
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
        : Field("float",n,l), Float(n,l), WindowGeneratorField("float",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual bool            providesSourceGetCurrent() const { return true; }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        float val;" << endl;
        c << "        if(LineEditGetFloat("<<name<<", val))" <<endl;
        c << "            atts->Set"<<Name<<"(val);" << endl;
        c << "        else" << endl;
        c << "        {" << endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << endl;
        c << "                FloatToQString(atts->Get"<<Name<<"()));" << endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(FloatToQString(atts->Get"<<Name<<"()));" << endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << endl;
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
        : Field("floatArray",n,l), FloatArray(s,n,l), WindowGeneratorField("floatArray",n,l) { }
    virtual void               writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void               writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void               writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual bool               providesSourceGetCurrent() const { return true; }
    virtual void               writeSourceGetCurrent(QTextStream &c)
    {
        c << "        float val["<<length<<"];"<<endl;
        c << "        if(LineEditGetFloats("<<name<<", val, " <<length<<"))" <<endl;
        c << "            atts->Set"<<Name<<"(val);" << endl;
        c << "        else" << endl;
        c << "        {" << endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << endl;
        c << "                FloatsToQString(atts->Get"<<Name<<"()," << length <<"));" << endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void               writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(FloatsToQString(atts->Get"<<Name<<"(), "<<length<<");" << endl;
    }
    virtual void               writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << endl;
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
        : Field("double",n,l), Double(n,l), WindowGeneratorField("double",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual bool            providesSourceGetCurrent() const { return true; }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        double val;" << endl;
        c << "        if(LineEditGetDouble("<<name<<", val))" <<endl;
        c << "            atts->Set"<<Name<<"(val);" << endl;
        c << "        else" << endl;
        c << "        {" << endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << endl;
        c << "                DoubleToQString(atts->Get"<<Name<<"()));" << endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(DoubleToQString(atts->Get"<<Name<<"()));" << endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << endl;
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
        : Field("doubleArray",n,l), DoubleArray(s,n,l), WindowGeneratorField("doubleArray",n,l) { }
    virtual void               writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void               writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void               writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual bool               providesSourceGetCurrent() const { return true; }
    virtual void               writeSourceGetCurrent(QTextStream &c)
    {
        c << "        double val["<<length<<"];"<<endl;
        c << "        if(LineEditGetDoubles("<<name<<", val, " <<length<<"))" <<endl;
        c << "            atts->Set"<<Name<<"(val);" << endl;
        c << "        else" << endl;
        c << "        {" << endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << endl;
        c << "                DoublesToQString(atts->Get"<<Name<<"()," << length <<"));" << endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void               writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(DoublesToQString(atts->Get"<<Name<<"(), "<<length<<"));" << endl;
    }
    virtual void               writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << endl;
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
        : Field("doubleVector",n,l), DoubleVector(n,l), WindowGeneratorField("doubleVector",n,l) { }
    virtual void               writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void               writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void               writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual bool               providesSourceGetCurrent() const { return true; }
    virtual void               writeSourceGetCurrent(QTextStream &c)
    {
        c << "        doubleVector val;"<<endl;
        c << "        if(LineEditGetDoubles("<<name<<", val))" <<endl;
        c << "            atts->Set"<<Name<<"(val);" << endl;
        c << "        else" << endl;
        c << "        {" << endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << endl;
        c << "                DoublesToQString(atts->Get"<<Name<<"()));" << endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void               writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(DoublesToQString(atts->Get"<<Name<<"()));" << endl;
    }
    virtual void               writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
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
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QLineEdit(central);" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual bool            providesSourceGetCurrent() const { return true; }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        QString temp = "<<name<<"->displayText();" << endl;
        c << "        if(!temp.isEmpty())" << endl;
        c << "            atts->Set"<<Name<<"(temp.toStdString());" << endl;
        c << "        else" << endl;
        c << "        {" << endl;
        QString msgLabel = (label.length()>0) ? label : name;
        c << "            ResettingError(tr(\""<<msgLabel<<"\")," << endl;
        c << "                QString(atts->Get"<<Name<<"().c_str()));" << endl;
        c << "            atts->Set"<<Name<<"(atts->Get"<<Name<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->setText(QString(atts->Get"<<Name<<"().c_str()));" << endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<classname << "::ID_" << name<<");" << endl;
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
        : Field("stringVector",n,l), StringVector(n,l), WindowGeneratorField("stringVector",n,l) { }
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
        h << "    void "<<name<<"Changed(bool useDefault, const QString &ctName);" << endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QvisColorTableButton *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QvisColorTableButton(central);" << endl;
        c << "    connect("<<name<<", SIGNAL(selectedColorTable(bool, const QString&))," << endl
          << "            this, SLOT("<<name<<"Changed(bool, const QString&)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->blockSignals(true);" << endl;
        c << "            "<<name<<"->setColorTable(QString(atts->Get"<<Name<<"().c_str()));" << endl;
        c << "            "<<name<<"->blockSignals(false);" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(bool useDefault, const QString &ctName)" << endl;
        c << "{" << endl;
        c << "    atts->Set"<<Name<<"(ctName.toStdString());" << endl;
        if(!isEnabler)
            c << "    SetUpdate(false);" << endl;
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
        : Field("color",n,l), Color(n,l), WindowGeneratorField("color",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"Changed(const QColor &color);" << endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QvisColorButton *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QvisColorButton(central);" << endl;
        c << "    connect("<<name<<", SIGNAL(selectedColor(const QColor&))," << endl
          << "            this, SLOT("<<name<<"Changed(const QColor&)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            { // new scope" << endl;
        c << "                tempcolor = QColor(atts->Get"<<Name<<"().Red()," << endl;
        c << "                                   atts->Get"<<Name<<"().Green()," << endl;
        c << "                                   atts->Get"<<Name<<"().Blue());" << endl;
        c << "                "<<name<<"->blockSignals(true);" << endl;
        c << "                "<<name<<"->setButtonColor(tempcolor);" << endl;
        c << "                "<<name<<"->blockSignals(false);" << endl;
        c << "            }" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(const QColor &color)" << endl;
        c << "{" << endl;
        c << "    ColorAttribute temp(color.red(), color.green(), color.blue());" << endl;
        c << "    atts->Set"<<Name<<"(temp);" << endl;
        if(!isEnabler)
            c << "    SetUpdate(false);" << endl;
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
        : Field("opacity",n,l), Opacity(n,l), WindowGeneratorField("opacity",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"Changed(int opacity, const void*);" << endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QvisOpacitySlider *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QvisOpacitySlider(0,255,25,255, central, NULL);" << endl;
        c << "    "<<name<<"->setTickInterval(64);" << endl;
        c << "    "<<name<<"->setGradientColor(QColor(0, 0, 0));" << endl;
        c << "    connect("<<name<<", SIGNAL(valueChanged(int, const void*))," << endl
          << "            this, SLOT("<<name<<"Changed(int, const void*)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->blockSignals(true);" << endl;
        c << "            "<<name<<"->setValue(int(atts->Get"<<Name<<"()*255.));" << endl;
        c << "            "<<name<<"->blockSignals(false);" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(int opacity, const void*)" << endl;
        c << "{" << endl;
        c << "    atts->Set"<<Name<<"((float)opacity/255.);" << endl;
        if(!isEnabler)
            c << "    SetUpdate(false);" << endl;
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
        : Field("linestyle",n,l), LineStyle(n,l), WindowGeneratorField("linestyle",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"Changed(int style);" << endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QvisLineStyleWidget *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QvisLineStyleWidget(0, central);" << endl;
        c << "    connect("<<name<<", SIGNAL(lineStyleChanged(int))," << endl
          << "            this, SLOT("<<name<<"Changed(int)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->blockSignals(true);" << endl;
        c << "            "<<name<<"->SetLineStyle(atts->Get"<<Name<<"());" << endl;
        c << "            "<<name<<"->blockSignals(false);" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(int style)" << endl;
        c << "{" << endl;
        c << "    atts->Set"<<Name<<"(style);" << endl;
        if(!isEnabler)
            c << "    SetUpdate(false);" << endl;
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
        : Field("linewidth",n,l), LineWidth(n,l), WindowGeneratorField("linewidth",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"Changed(int style);" << endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QvisLineWidthWidget *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QvisLineWidthWidget(0, central);" << endl;
        c << "    connect("<<name<<", SIGNAL(lineWidthChanged(int))," << endl
          << "            this, SLOT("<<name<<"Changed(int)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->blockSignals(true);" << endl;
        c << "            "<<name<<"->SetLineWidth(atts->Get"<<Name<<"());" << endl;
        c << "            "<<name<<"->blockSignals(false);" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(int style)" << endl;
        c << "{" << endl;
        c << "    atts->Set"<<Name<<"(style);" << endl;
        if(!isEnabler)
            c << "    SetUpdate(false);" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
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

class WindowGeneratorVariableName : public virtual VariableName , public virtual WindowGeneratorField
{
  public:
    WindowGeneratorVariableName(const QString &n, const QString &l)
        : Field("variablename",n,l), VariableName(n,l), WindowGeneratorField("variablename",n,l) { }
    virtual void            writeHeaderCallback(QTextStream &h)
    {
        h << "    void "<<name<<"Changed(const QString &varName);" << endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QvisVariableButton *"<<name<<";" << endl;
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
            "QvisVariableButton::SymmetricTensors"
            "QvisVariableButton::Labels",
            "QvisVariableButton::Arrays",
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
        c << ";" << endl;
        c << "    "<<name<<" = new QvisVariableButton(true, true, true, "<<name<<"Mask, central);" << endl;
        if(valueSet)
            c << "    " << name << "->setDefaultVariable(\"" << val << "\");" << endl;
        c << "    connect("<<name<<", SIGNAL(activated(const QString&))," << endl
          << "            this, SLOT("<<name<<"Changed(const QString&)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"->blockSignals(true);" << endl;
        c << "            "<<name<<"->setText(QString(atts->Get"<<Name<<"().c_str()));" << endl;
        c << "            "<<name<<"->blockSignals(false);" << endl;
    }
    virtual void            writeSourceCallback(QString &, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(const QString &varName)" << endl;
        c << "{" << endl;
        c << "    atts->Set"<<Name<<"(varName.toStdString());" << endl;
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
        h << "    void "<<name<<"Changed(int val);" << endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QWidget      *"<<name<<";" << endl;
        h << "    QButtonGroup *"<<name<<"ButtonGroup;" << endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    "<<name<<" = new QWidget(central);" << endl;
        c << "    "<<name<<"ButtonGroup= new QButtonGroup("<<name<<");" << endl;
        c << "    QHBoxLayout *"<<name<<"Layout = new QHBoxLayout("<<name<<");" << endl;
        c << "    "<<name<<"Layout->setMargin(0);" << endl;
        c << "    "<<name<<"Layout->setSpacing(10);" << endl;

        for (size_t i=0; i<enumType->values.size(); i++)
        {
            c << "    QRadioButton *"<<name<<enumType->type<<enumType->values[i]
              << " = new QRadioButton(tr(\""<<enumType->values[i]<<"\"), "<<name<<");" << endl;
            c << "    "<<name<<"ButtonGroup->addButton("<<name<<enumType->type<<enumType->values[i]<<","<<i<<");"<<endl;
            c << "    "<<name<<"Layout->addWidget("<<name<<enumType->type<<enumType->values[i]<<");" << endl;
        }

        c << "    connect("<<name<<"ButtonGroup, SIGNAL(buttonClicked(int))," << endl
          << "            this, SLOT("<<name<<"Changed(int)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"ButtonGroup->blockSignals(true);" << endl;
        c << "            if("<<name<<"ButtonGroup->button((int)atts->Get"<<Name<<"()) != 0)" << endl;
        c << "                "<<name<<"ButtonGroup->button((int)atts->Get"<<Name<<"())->setChecked(true);" << endl;
        c << "            "<<name<<"ButtonGroup->blockSignals(false);" << endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c, bool isEnabler)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(int val)" << endl;
        c << "{" << endl;
        c << "    if(val != atts->Get"<<Name<<"())" << endl;
        c << "    {" << endl;
        c << "        atts->Set"<<Name<<"("<<GetCPPName(true,classname)<<"(val));" << endl;
        if(!isEnabler)
            c << "        SetUpdate(false);" << endl;
        c << "        Apply();" << endl;
        c << "    }" << endl;
        c << "}" << endl;
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
        h << "    void "<<name<<"Changed(int val);" << endl;
    }
    virtual void            writeHeaderData(QTextStream &h)
    {
        h << "    QWidget      *"<<name<<";" << endl;
        h << "    QButtonGroup *"<<name<<"ButtonGroup;" << endl;
    }
    virtual void            writeSourceCreate(QTextStream &c)
    {
        writeSourceCreateLabel(c);
        c << "    QWidget *"<<name<<" = new QWidget(central);"<<endl;
        c << "    "<<name<<"ButtonGroup = new QButtonGroup("<<name<<");" << endl;
        c << "    QHBoxLayout *"<<name<<"Layout = new QHBoxLayout("<<name<<");" << endl;
        c << "    "<<name<<"Layout->setMargin(0);" << endl;
        c << "    "<<name<<"Layout->setSpacing(10);" << endl;

        c << "    QRadioButton *"<<name<<"ScaleModeLinear"
          << " = new QRadioButton(tr(\""<<"Linear"<<"\"), "<<name<<");" << endl;
        c << "    "<<name<<"Layout->addWidget("<<name<<"ScaleModeLinear"<<");" << endl;
        c << "    "<<name<<"ButtonGroup->addButton("<<name<<"ScaleModeLinear,0);"<<endl;
        c << "    QRadioButton *"<<name<<"ScaleModeLog"
          << " = new QRadioButton(tr(\""<<"Log"<<"\"), "<<name<<");" << endl;
        c << "    "<<name<<"ButtonGroup->addButton("<<name<<"ScaleModeLog,1);"<<endl;
        c << "    "<<name<<"Layout->addWidget("<<name<<"ScaleModeLog"<<");" << endl;

        c << "    connect("<<name<<"ButtonGroup, SIGNAL(buttonClicked(int))," << endl
          << "            this, SLOT("<<name<<"Changed(int)));" << endl;
        c << "    mainLayout->addWidget("<<name<<"Widget, "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(QTextStream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdateWindow(QTextStream &c)
    {
        c << "            "<<name<<"ButtonGroup->blockSignals(true);" << endl;
        c << "            if("<<name<<"ButtonGroup->button(atts->Get"<<Name<<"()) != 0)"<<endl;
        c << "                "<<name<<"ButtonGroup->button(atts->Get"<<Name<<"())->setChecked(true);" << endl;
        c << "            "<<name<<"ButtonGroup->blockSignals(false);" << endl;
    }
    virtual void            writeSourceCallback(QString &classname, QString &windowname, QTextStream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(int val)" << endl;
        c << "{" << endl;
        c << "    if(val != atts->Get"<<Name<<"())" << endl;
        c << "    {" << endl;
        c << "        atts->Set"<<Name<<"("<<GetCPPName(true,classname)<<"(val));" << endl;
        c << "        SetUpdate(false);" << endl;
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
//    Kathleen Bonnell, Thu Mar 22 16:58:23 PDT 2007 
//    Added scalemode.
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
// ----------------------------------------------------------------------------
#include "GeneratorBase.h"

class WindowGeneratorAttribute : public GeneratorBase
{
  public:
    vector<WindowGeneratorField*> fields;
    QString windowname;
    QString plugintype;
  public:
    WindowGeneratorAttribute(const QString &n, const QString &p, const QString &f,
                             const QString &e, const QString &ei)
        : GeneratorBase(n,p,f,e,ei, GENERATOR_NAME), fields(), 
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

    void Print(QTextStream &out)
    {
        out << "    Attribute: " << name << " (" << purpose << ")" << endl;
        size_t i;
        for (i=0; i<fields.size(); i++)
            fields[i]->Print(out);
        for (i=0; i<functions.size(); i++)
            functions[i]->Print(out);
    }

    bool RequiresGetCurrentValues() const
    {
        int n = 0;
        for (int i=0; i<fields.size(); i++)
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
    // ************************************************************************

    void WriteHeader(QTextStream &h)
    {
        h << copyright_str << endl;
        h << "#ifndef " << windowname.toUpper() << "_H" << endl;
        h << "#define " << windowname.toUpper() << "_H" << endl;
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
        
        WriteClassComment(h, windowname, QString("Defines ") + windowname + QString(" class."));

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
          << "                         const QString &caption = QString::null," << endl
          << "                         const QString &shortName = QString::null," << endl
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
        size_t i;
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
    //   Hank Childs, Fri Jun  9 09:54:46 PDT 2006
    //   Added copyright string.
    //
    //   Brad Whitlock, Wed Apr  9 12:49:53 PDT 2008
    //   Use QString for caption, shortName.
    //
    // ************************************************************************

    void WriteSource(QTextStream &c)
    {
        c << copyright_str << endl;
        c << "#include \""<<windowname<<".h\"" << endl;
        c << endl;
        c << "#include <"<<name<<".h>" << endl;
        c << "#include <ViewerProxy.h>" << endl;
        c << endl;
        c << "#include <QCheckBox>" << endl;
        c << "#include <QLabel>" << endl;
        c << "#include <QLayout>" << endl;
        c << "#include <QLineEdit>" << endl;
        c << "#include <QSpinBox>" << endl;
        c << "#include <QButtonGroup>" << endl;
        c << "#include <QRadioButton>" << endl;
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
        WriteMethodComment(c, windowname, windowname, "Constructor");
        c << windowname<<"::"<<windowname<<"("
          << (plugintype=="" ? "" : "const int type,") << endl
          << "                         "<<name<<" *subj," << endl
          << "                         const QString &caption," << endl
          << "                         const QString &shortName," << endl
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
        WriteMethodComment(c, windowname, QString("~") + windowname, "Destructor");
        c << windowname<<"::~"<<windowname<<"()" << endl;
        c << "{" << endl;
        c << "}" << endl;
        c << endl;
        c << endl;

        // CreateWindowContents
        WriteMethodComment(c, windowname, "CreateWindowContents", "Creates the widgets for the window.");
        c << "void" << endl;
        c << windowname<<"::CreateWindowContents()" << endl;
        c << "{" << endl;
        c << "    QGridLayout *mainLayout = new QGridLayout(0);" << endl;
        c << "    topLayout->addLayout(mainLayout);" << endl;
        c << endl;
        size_t i;
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
        WriteMethodComment(c, windowname, "UpdateWindow",
                           "Updates the widgets in the window when the subject changes.");
        c << "void" << endl;
        c << windowname << "::UpdateWindow(bool doAll)" << endl;
        c << "{" << endl;
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
        c << "        switch(i)" << endl;
        c << "        {" << endl;
        for (i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal) continue;

            WindowGeneratorField *field   = fields[i];
            c << "          case "<<name<<"::ID_"<<field->name << ":" << endl;

            vector<Field*> enablees;
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

                c << ")" << endl;
                c << "            {\n";
                c << "                "<<enablees[j]->name<<"->setEnabled(true);" << endl;
                c << "                if("<<enablees[j]->name<<"Label)" << endl;
                c << "                    "<<enablees[j]->name<<"Label->setEnabled(true);" << endl;
                c << "            }\n";
                c << "            else" << endl;
                c << "            {\n";
                c << "                "<<enablees[j]->name<<"->setEnabled(false);" << endl;
                c << "                if("<<enablees[j]->name<<"Label)" << endl;
                c << "                    "<<enablees[j]->name<<"Label->setEnabled(false);" << endl;
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
        WriteMethodComment(c, windowname, "GetCurrentValues",
                           "Gets values from certain widgets and stores them in the subject.");
        c << "void" << endl;
        c << windowname << "::GetCurrentValues(int which_widget)" << endl;
        c << "{" << endl;
        if(RequiresGetCurrentValues())
        {
            c << "    bool doAll = (which_widget == -1);" << endl;
            c << endl;
        }
        for (i=0; i<fields.size(); i++)
        {
            if (fields[i]->internal || !fields[i]->providesSourceGetCurrent())
                continue;
            c << "    // Do " << fields[i]->name << endl;
            c << "    if(which_widget == "<<name << "::ID_" << fields[i]->name<<" || doAll)" << endl;
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
            WriteMethodComment(c, windowname, "Apply", "Called to apply changes in the subject.");
            c << "void" << endl;
            c << windowname<<"::Apply(bool ignore)" << endl;
            c << "{" << endl;
            c << "    if(AutoUpdate() || ignore)" << endl;
            c << "    {" << endl;
            c << "        GetCurrentValues(-1);" << endl;
            c << "        atts->Notify();" << endl;
            c << "" << endl;
            if (plugintype == "plot")
                c << "        GetViewerMethods()->SetPlotOptions(plotType);" << endl;
            else if(plugintype != "operator")
                c << "        GetViewerMethods()->Set"<<name<<"();" << endl;
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
            WriteMethodComment(c, windowname, "apply", "Qt slot function called when apply button is clicked.");
            c << "void" << endl;
            c << windowname << "::apply()" << endl;
            c << "{" << endl;
            c << "    Apply(true);" << endl;
            c << "}" << endl;
            c << endl;
            c << endl;

            // makeDefault
            WriteMethodComment(c, windowname, "makeDefault", "Qt slot function called when \"Make default\" button is clicked.");
            c << "void" << endl;
            c << windowname<<"::makeDefault()" << endl;
            c << "{" << endl;
            c << "    GetCurrentValues(-1);" << endl;
            c << "    atts->Notify();" << endl;
            if (plugintype == "plot")
                c << "    GetViewerMethods()->SetDefaultPlotOptions(plotType);" << endl;
            else if(plugintype != "operator")
                c << "    GetViewerMethods()->SetDefault"<<name<<"();" << endl;
            c << "}" << endl;
            c << endl;
            c << endl;

            // reset
            WriteMethodComment(c, windowname, "reset", "Qt slot function called when reset button is clicked.");
            c << "void" << endl;
            c << windowname << "::reset()" << endl;
            c << "{" << endl;
            if (plugintype == "plot")
                c << "    GetViewerMethods()->ResetPlotOptions(plotType);" << endl;
            else if(plugintype != "operator")
                c << "    GetViewerMethods()->Reset"<<name<<"();" << endl;
            c << "}" << endl;
            c << endl;
            c << endl;
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
            c << endl;
            c << endl;
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
// ----------------------------------------------------------------------------

class WindowGeneratorPlugin : public PluginBase
{
  public:
    WindowGeneratorAttribute *atts;
    QString windowname;
  public:
    WindowGeneratorPlugin(const QString &n,const QString &l,const QString &t,
          const QString &vt,const QString &dt,const QString &v,const QString &ifile,
          bool hw,bool ho,bool onlyengine,bool noengine)
        : PluginBase(n,l,t,vt,dt,v,ifile,hw,ho,onlyengine,noengine), atts(NULL), windowname()
    {
        if (type == "plot")
            windowname = QString("Qvis")+name+QString("PlotWindow");
        else if (type == "operator")
            windowname = QString("Qvis")+name+QString("Window");
    }
    void Print(QTextStream &out)
    {
        out << "Plugin: "<<name<<" (\""<<label<<"\", type="<<type<<") -- version "<<version<< endl;
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
