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

// ----------------------------------------------------------------------------
//  Program:    windowmaker
//
//
//  Programmer: Jeremy Meredith
//  Date:       August 17, 2001
//
//  Purpose:
//    Takes as input a state.def and outputs a Qt window capable of setting
//    all the attributes.
//
//  Notes:
//    It currently supports the following:
//           bool
//           int
//           intArray
//           uchar
//           ucharArray
//           float
//           floatArray
//           double
//           doubleArray
//           string
//        It does not yet support vectors, or other more advanced
//        types such as colors or spectrums.
// ----------------------------------------------------------------------------

#include "util.h"
#include <stdio.h>
#include <visitstream.h>
#include <string>
#include <vector>
using std::string;
using std::vector;


class DataType
{
  public:
    DataType()
    {
        name   = "uninitialized";
        number = -1;
        index  = lastindex++;
        attName= name; 
        attName[0] = toUpper(attName[0]);
    }
    DataType(string s, int i)
    {
        name   = s;
        number = i;
        index  = lastindex++;
        attName= name; 
        attName[0] = toUpper(attName[0]);
    }
    virtual string             getName()
    {
        return name;
    }
    virtual int                getNumber()
    {
        return number;
    }
    virtual string             getType()
    {
        switch (getNumber())
        {
          case -1: return "ERROR -- unitialized data type";               break;
          case 0:  return string("vector<") + getRawType() + string(">"); break;
          case 1:  return getRawType();                                   break;
          default: return getRawType() + "[" + i2s(getNumber()) + "]";    break;
        }
    }
    virtual string             getRawType()                     = 0;
    virtual void               writeHeaderCallback(ofstream &h)
    {
        h << "    //writeHeaderCallback unknown for " << getType() << " (variable " << getName() << ")" << endl;
    }
    virtual void               writeHeaderData(ofstream &h)
    {
        h << "    //writeHeaderData unknown for " << getType() << " (variable " << getName() << ")" << endl;
    }
    virtual void               writeSourceCreate(ofstream &c)
    {
        // add a label so we know which ones we are missing
        c << "    mainLayout->addWidget(new QLabel(\""<<name<<"\", central, \""<<name<<"Label\"),"<<index<<",0);" << endl;
    }
    virtual void               writeSourceGetCurrent(ofstream &c)
    {
        c << "    //writeSourceGetCurrent unknown for " << getType() << " (variable " << getName() << ")" << endl;
    }
    virtual void               writeSourceUpdate(ofstream &c)
    {
        c << "    //writeSourceUpdate unknown for " << getType() << " (variable " << getName() << ")" << endl;
    }
    virtual void               writeSourceCallback(string windowname, ofstream &c)
    {
        c << "    //writeSourceCallback unknown for " << getType() << " (variable " << getName() << ")" << endl;
    }
  protected:
    string name;
    int    number;
    int    index;
    string attName;
    static int lastindex;
};
int DataType::lastindex = 0;

// ----------------------------------------------------------------------------
//                                   Data Types
// ----------------------------------------------------------------------------

class BoolBase : public DataType
{
  public:
                            BoolBase(string s,int n) : DataType(s,n)       { };
    virtual string          getRawType()           { return "bool";          };
};
class Bool : public BoolBase
{
  public:
                            Bool(string s) : BoolBase(s, 1) {};
    virtual void            writeHeaderCallback(ofstream &h)
    {
        h << "    void "<<name<<"Changed(bool val);" << endl;
    }
    virtual void            writeHeaderData(ofstream &h)
    {
        h << "    QCheckBox *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ofstream &c)
    {
        c << "    "<<name<<" = new QCheckBox(\""<<name<<"\", central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(toggled(bool))," << endl
          << "            this, SLOT("<<name<<"Changed(bool)));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",0);" << endl;
    }
    virtual void            writeSourceGetCurrent(ofstream &c)
    {
        c << "        // Nothing for " << name << endl;
    }
    virtual void            writeSourceUpdate(ofstream &c)
    {
        c << "            "<<name<<"->setChecked(atts->Get"<<attName<<"());" << endl;
    }
    virtual void            writeSourceCallback(string windowname, ofstream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"Changed(bool val)" << endl;
        c << "{" << endl;
        c << "    atts->Set"<<attName<<"(val);" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};
class BoolVector : public BoolBase
{
  public:
                            BoolVector(string s) : BoolBase(s, 0) {};
};
class BoolArray : public BoolBase
{
  public:
                            BoolArray(string s, int n) : BoolBase(s, n) {};
};


class IntBase : public DataType
{
  public:
                            IntBase(string s,int n) : DataType(s,n)        { };
    virtual string          getRawType()           { return "int";           };
};
class Int : public IntBase
{
  public:
                            Int(string s) : IntBase(s, 1) {};
    virtual void            writeHeaderCallback(ofstream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(ofstream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ofstream &c)
    {
        DataType::writeSourceCreate(c);
        c << "    "<<name<<" = new QLineEdit(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ofstream &c)
    {
        c << "        temp = "<<name<<"->displayText().simplifyWhiteSpace();" << endl;
        c << "        okay = !temp.isEmpty();" << endl;
        c << "        if(okay)" << endl;
        c << "        {" << endl;
        c << "            int val = temp.toInt(&okay);" << endl;
        c << "            atts->Set"<<attName<<"(val);" << endl;
        c << "        }" << endl;
        c << endl;
        c << "        if(!okay)" << endl;
        c << "        {" << endl;
        c << "            msg.sprintf(\"The value of "<<name<<" was invalid. \"" << endl;
        c << "                \"Resetting to the last good value of %d.\"," << endl;
        c << "                atts->Get"<<attName<<"());" << endl;
        c << "            Message(msg);" << endl;
        c << "            atts->Set"<<attName<<"(atts->Get"<<attName<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdate(ofstream &c)
    {
        c << "            temp.sprintf(\"%d\", atts->Get"<<attName<<"());" << endl;
        c << "            "<<name<<"->setText(temp);" << endl;
    }
    virtual void            writeSourceCallback(string windowname, ofstream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<index<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};
class IntVector : public IntBase
{
  public:
                            IntVector(string s) : IntBase(s, 0) {};
};
class IntArray : public IntBase
{
  public:
                            IntArray(string s, int n) : IntBase(s, n) {};
    virtual void            writeHeaderCallback(ofstream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(ofstream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ofstream &c)
    {
        DataType::writeSourceCreate(c);
        c << "    "<<name<<" = new QLineEdit(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ofstream &c)
    {
        c << "        temp = "<<name<<"->displayText().simplifyWhiteSpace();" << endl;
        c << "        okay = !temp.isEmpty();" << endl;
        c << "        if(okay)" << endl;
        c << "        {" << endl;
        c << "            int val["<<number<<"];" << endl;
        c << "            sscanf(temp.latin1(), \"";
        for (int i=0; i<number; i++)
        {
            c << "%d";
            if (i < number-1) c << " ";
        }
        c << "\", ";
        for (int i=0; i<number; i++)
        {
            c << "&val["<<i<<"]";
            if (i < number-1) c << ", ";
        }
        c << ");" << endl;
        c << "            atts->Set"<<attName<<"(val);" << endl;
        c << "        }" << endl;
        c << endl;
        c << "        if(!okay)" << endl;
        c << "        {" << endl;
        c << "            const int *val = atts->Get"<<attName<<"();" << endl;
        c << "            msg.sprintf(\"The value of "<<name<<" was invalid. \"" << endl;
        c << "                \"Resetting to the last good value of <";
        for (int i=0; i<number; i++)
        {
            c << "%d";
            if (i < number-1) c << " ";
        }
        c << ">\", " << endl;
        c << "                ";
        for (int i=0; i<number; i++)
        {
            c << "&val["<<i<<"]";
            if (i < number-1) c << ", ";
        }
        c << ");" << endl;
        c << "            Message(msg);" << endl;
        c << "            atts->Set"<<attName<<"(atts->Get"<<attName<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdate(ofstream &c)
    {
        c << "            iptr = atts->Get"<<attName<<"();" << endl;
        c << "            temp.sprintf(\"";
        for (int i=0; i<number; i++)
        {
            c << "%d";
            if (i<number-1) c << " ";
        }
        c << "\", ";
        for (int i=0; i<number; i++)
        {
            c << "iptr["<<i<<"]";
            if (i<number-1) c << ", ";
        }
        c << ");" << endl;
        c << "            "<<name<<"->setText(temp);" << endl;
    }
    virtual void            writeSourceCallback(string windowname, ofstream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<index<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


class CharBase : public DataType
{
  public:
                            CharBase(string s,int n) : DataType(s,n)       { };
    virtual string          getRawType()           { return "char";          };
};
class Char : public CharBase
{
  public:
                            Char(string s) : CharBase(s, 1) {};
};
class CharVector : public CharBase
{
  public:
                            CharVector(string s) : CharBase(s, 0) {};
};
class CharArray : public CharBase
{
  public:
                            CharArray(string s, int n) : CharBase(s, n) {};
};


class UCharBase : public DataType
{
  public:
                            UCharBase(string s,int n) : DataType(s,n)      { };
    virtual string          getRawType()           { return "unsigned char"; };
};
class UChar : public UCharBase
{
  public:
                            UChar(string s) : UCharBase(s, 1) {};
    virtual void            writeHeaderCallback(ofstream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(ofstream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ofstream &c)
    {
        DataType::writeSourceCreate(c);
        c << "    "<<name<<" = new QLineEdit(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ofstream &c)
    {
        c << "        temp = "<<name<<"->displayText().simplifyWhiteSpace();" << endl;
        c << "        okay = !temp.isEmpty();" << endl;
        c << "        if(okay)" << endl;
        c << "        {" << endl;
        c << "            unsigned char val = temp.toInt(&okay);" << endl;
        c << "            atts->Set"<<attName<<"(val);" << endl;
        c << "        }" << endl;
        c << endl;
        c << "        if(!okay)" << endl;
        c << "        {" << endl;
        c << "            msg.sprintf(\"The value of "<<name<<" was invalid. \"" << endl;
        c << "                \"Resetting to the last good value of %d.\"," << endl;
        c << "                atts->Get"<<attName<<"());" << endl;
        c << "            Message(msg);" << endl;
        c << "            atts->Set"<<attName<<"(atts->Get"<<attName<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdate(ofstream &c)
    {
        c << "            temp.sprintf(\"%d\", atts->Get"<<attName<<"());" << endl;
        c << "            "<<name<<"->setText(temp);" << endl;
    }
    virtual void            writeSourceCallback(string windowname, ofstream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<index<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};
class UCharVector : public UCharBase
{
  public:
                            UCharVector(string s) : UCharBase(s, 0) {};
};
class UCharArray : public UCharBase
{
  public:
                            UCharArray(string s, int n) : UCharBase(s, n) {};
    virtual void            writeHeaderCallback(ofstream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(ofstream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ofstream &c)
    {
        DataType::writeSourceCreate(c);
        c << "    "<<name<<" = new QLineEdit(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ofstream &c)
    {
        c << "        temp = "<<name<<"->displayText().simplifyWhiteSpace();" << endl;
        c << "        okay = !temp.isEmpty();" << endl;
        c << "        if(okay)" << endl;
        c << "        {" << endl;
        c << "            int ival["<<number<<"];" << endl;
        c << "            unsigned char val["<<number<<"];" << endl;
        c << "            sscanf(temp.latin1(), \"";
        for (int i=0; i<number; i++)
        {
            c << "%d";
            if (i < number-1) c << " ";
        }
        c << "\", ";
        for (int i=0; i<number; i++)
        {
            c << "&ival["<<i<<"]";
            if (i < number-1) c << ", ";
        }
        c << ");" << endl;
        c << "            for (int i=0; i<"<<number<<"; i++)" << endl;
        c << "                val[i] = ival[i];" << endl;
        c << "            atts->Set"<<attName<<"(val);" << endl;
        c << "        }" << endl;
        c << endl;
        c << "        if(!okay)" << endl;
        c << "        {" << endl;
        c << "            const unsigned char *val = atts->Get"<<attName<<"();" << endl;
        c << "            msg.sprintf(\"The value of "<<name<<" was invalid. \"" << endl;
        c << "                \"Resetting to the last good value of <";
        for (int i=0; i<number; i++)
        {
            c << "%d";
            if (i < number-1) c << " ";
        }
        c << ">\", " << endl;
        c << "                ";
        for (int i=0; i<number; i++)
        {
            c << "&val["<<i<<"]";
            if (i < number-1) c << ", ";
        }
        c << ");" << endl;
        c << "            Message(msg);" << endl;
        c << "            atts->Set"<<attName<<"(atts->Get"<<attName<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdate(ofstream &c)
    {
        c << "            uptr = atts->Get"<<attName<<"();" << endl;
        c << "            temp.sprintf(\"";
        for (int i=0; i<number; i++)
        {
            c << "%d";
            if (i<number-1) c << " ";
        }
        c << "\", ";
        for (int i=0; i<number; i++)
        {
            c << "uptr["<<i<<"]";
            if (i<number-1) c << ", ";
        }
        c << ");" << endl;
        c << "            "<<name<<"->setText(temp);" << endl;
    }
    virtual void            writeSourceCallback(string windowname, ofstream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<index<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


class FloatBase : public DataType
{
  public:
                            FloatBase(string s,int n) : DataType(s,n)      { };
    virtual string          getRawType()           { return "float";         };
};
class Float : public FloatBase
{
  public:
                            Float(string s) : FloatBase(s, 1) {};
    virtual void            writeHeaderCallback(ofstream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(ofstream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ofstream &c)
    {
        DataType::writeSourceCreate(c);
        c << "    "<<name<<" = new QLineEdit(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ofstream &c)
    {
        c << "        temp = "<<name<<"->displayText().simplifyWhiteSpace();" << endl;
        c << "        okay = !temp.isEmpty();" << endl;
        c << "        if(okay)" << endl;
        c << "        {" << endl;
        c << "            float val = temp.toFloat(&okay);" << endl;
        c << "            atts->Set"<<attName<<"(val);" << endl;
        c << "        }" << endl;
        c << endl;
        c << "        if(!okay)" << endl;
        c << "        {" << endl;
        c << "            msg.sprintf(\"The value of "<<name<<" was invalid. \"" << endl;
        c << "                \"Resetting to the last good value of %g.\"," << endl;
        c << "                atts->Get"<<attName<<"());" << endl;
        c << "            Message(msg);" << endl;
        c << "            atts->Set"<<attName<<"(atts->Get"<<attName<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdate(ofstream &c)
    {
        c << "            temp.sprintf(\"%g\", atts->Get"<<attName<<"());" << endl;
        c << "            "<<name<<"->setText(temp);" << endl;
    }
    virtual void            writeSourceCallback(string windowname, ofstream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<index<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};
class FloatVector : public FloatBase
{
  public:
                            FloatVector(string s) : FloatBase(s, 0) {};
};
class FloatArray : public FloatBase
{
  public:
                            FloatArray(string s, int n) : FloatBase(s, n) {};
    virtual void            writeHeaderCallback(ofstream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(ofstream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ofstream &c)
    {
        DataType::writeSourceCreate(c);
        c << "    "<<name<<" = new QLineEdit(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ofstream &c)
    {
        c << "        temp = "<<name<<"->displayText().simplifyWhiteSpace();" << endl;
        c << "        okay = !temp.isEmpty();" << endl;
        c << "        if(okay)" << endl;
        c << "        {" << endl;
        c << "            float val["<<number<<"];" << endl;
        c << "            sscanf(temp.latin1(), \"";
        for (int i=0; i<number; i++)
        {
            c << "%g";
            if (i < number-1) c << " ";
        }
        c << "\", ";
        for (int i=0; i<number; i++)
        {
            c << "&val["<<i<<"]";
            if (i < number-1) c << ", ";
        }
        c << ");" << endl;
        c << "            atts->Set"<<attName<<"(val);" << endl;
        c << "        }" << endl;
        c << endl;
        c << "        if(!okay)" << endl;
        c << "        {" << endl;
        c << "            const float *val = atts->Get"<<attName<<"();" << endl;
        c << "            msg.sprintf(\"The value of "<<name<<" was invalid. \"" << endl;
        c << "                \"Resetting to the last good value of <";
        for (int i=0; i<number; i++)
        {
            c << "%g";
            if (i < number-1) c << " ";
        }
        c << ">\", " << endl;
        c << "                ";
        for (int i=0; i<number; i++)
        {
            c << "&val["<<i<<"]";
            if (i < number-1) c << ", ";
        }
        c << ");" << endl;
        c << "            Message(msg);" << endl;
        c << "            atts->Set"<<attName<<"(atts->Get"<<attName<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdate(ofstream &c)
    {
        c << "            fptr = atts->Get"<<attName<<"();" << endl;
        c << "            temp.sprintf(\"";
        for (int i=0; i<number; i++)
        {
            c << "%g";
            if (i<number-1) c << " ";
        }
        c << "\", ";
        for (int i=0; i<number; i++)
        {
            c << "fptr["<<i<<"]";
            if (i<number-1) c << ", ";
        }
        c << ");" << endl;
        c << "            "<<name<<"->setText(temp);" << endl;
    }
    virtual void            writeSourceCallback(string windowname, ofstream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<index<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


class DoubleBase : public DataType
{
  public:
                            DoubleBase(string s,int n) : DataType(s,n)     { };
    virtual string          getRawType()           { return "double";        };
};
class Double : public DoubleBase
{
  public:
                            Double(string s) : DoubleBase(s, 1) {};
    virtual void            writeHeaderCallback(ofstream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(ofstream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ofstream &c)
    {
        DataType::writeSourceCreate(c);
        c << "    "<<name<<" = new QLineEdit(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ofstream &c)
    {
        c << "        temp = "<<name<<"->displayText().simplifyWhiteSpace();" << endl;
        c << "        okay = !temp.isEmpty();" << endl;
        c << "        if(okay)" << endl;
        c << "        {" << endl;
        c << "            double val = temp.toDouble(&okay);" << endl;
        c << "            atts->Set"<<attName<<"(val);" << endl;
        c << "        }" << endl;
        c << endl;
        c << "        if(!okay)" << endl;
        c << "        {" << endl;
        c << "            msg.sprintf(\"The value of "<<name<<" was invalid. \"" << endl;
        c << "                \"Resetting to the last good value of %lg.\"," << endl;
        c << "                atts->Get"<<attName<<"());" << endl;
        c << "            Message(msg);" << endl;
        c << "            atts->Set"<<attName<<"(atts->Get"<<attName<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdate(ofstream &c)
    {
        c << "            temp.sprintf(\"%lg\", atts->Get"<<attName<<"());" << endl;
        c << "            "<<name<<"->setText(temp);" << endl;
    }
    virtual void            writeSourceCallback(string windowname, ofstream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<index<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};
class DoubleVector : public DoubleBase
{
  public:
                            DoubleVector(string s) : DoubleBase(s, 0) {};
};
class DoubleArray : public DoubleBase
{
  public:
                            DoubleArray(string s, int n) : DoubleBase(s, n) {};
    virtual void            writeHeaderCallback(ofstream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(ofstream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ofstream &c)
    {
        DataType::writeSourceCreate(c);
        c << "    "<<name<<" = new QLineEdit(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ofstream &c)
    {
        c << "        temp = "<<name<<"->displayText().simplifyWhiteSpace();" << endl;
        c << "        okay = !temp.isEmpty();" << endl;
        c << "        if(okay)" << endl;
        c << "        {" << endl;
        c << "            double val["<<number<<"];" << endl;
        c << "            sscanf(temp.latin1(), \"";
        for (int i=0; i<number; i++)
        {
            c << "%lg";
            if (i < number-1) c << " ";
        }
        c << "\", ";
        for (int i=0; i<number; i++)
        {
            c << "&val["<<i<<"]";
            if (i < number-1) c << ", ";
        }
        c << ");" << endl;
        c << "            atts->Set"<<attName<<"(val);" << endl;
        c << "        }" << endl;
        c << endl;
        c << "        if(!okay)" << endl;
        c << "        {" << endl;
        c << "            const double *val = atts->Get"<<attName<<"();" << endl;
        c << "            msg.sprintf(\"The value of "<<name<<" was invalid. \"" << endl;
        c << "                \"Resetting to the last good value of <";
        for (int i=0; i<number; i++)
        {
            c << "%lg";
            if (i < number-1) c << " ";
        }
        c << ">\", " << endl;
        c << "                ";
        for (int i=0; i<number; i++)
        {
            c << "&val["<<i<<"]";
            if (i < number-1) c << ", ";
        }
        c << ");" << endl;
        c << "            Message(msg);" << endl;
        c << "            atts->Set"<<attName<<"(atts->Get"<<attName<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdate(ofstream &c)
    {
        c << "            dptr = atts->Get"<<attName<<"();" << endl;
        c << "            temp.sprintf(\"";
        for (int i=0; i<number; i++)
        {
            c << "%lg";
            if (i<number-1) c << " ";
        }
        c << "\", ";
        for (int i=0; i<number; i++)
        {
            c << "dptr["<<i<<"]";
            if (i<number-1) c << ", ";
        }
        c << ");" << endl;
        c << "            "<<name<<"->setText(temp);" << endl;
    }
    virtual void            writeSourceCallback(string windowname, ofstream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<index<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};


class StringBase : public DataType
{
  public:
                            StringBase(string s,int n) : DataType(s,n)     { };
    virtual string          getRawType()           { return "string";        };
};
class String : public StringBase
{
  public:
                            String(string s) : StringBase(s, 1) {};
    virtual void            writeHeaderCallback(ofstream &h)
    {
        h << "    void "<<name<<"ProcessText();" << endl;
    }
    virtual void            writeHeaderData(ofstream &h)
    {
        h << "    QLineEdit *"<<name<<";" << endl;
    }
    virtual void            writeSourceCreate(ofstream &c)
    {
        DataType::writeSourceCreate(c);
        c << "    "<<name<<" = new QLineEdit(central, \""<<name<<"\");" << endl;
        c << "    connect("<<name<<", SIGNAL(returnPressed())," << endl
          << "            this, SLOT("<<name<<"ProcessText()));" << endl;
        c << "    mainLayout->addWidget("<<name<<", "<<index<<",1);" << endl;
    }
    virtual void            writeSourceGetCurrent(ofstream &c)
    {
        c << "        temp = "<<name<<"->displayText();" << endl;
        c << "        okay = !temp.isEmpty();" << endl;
        c << "        if(okay)" << endl;
        c << "        {" << endl;
        c << "            atts->Set"<<attName<<"(temp.latin1());" << endl;
        c << "        }" << endl;
        c << endl;
        c << "        if(!okay)" << endl;
        c << "        {" << endl;
        c << "            msg.sprintf(\"The value of "<<name<<" was invalid. \"" << endl;
        c << "                \"Resetting to the last good value of %d.\"," << endl;
        c << "                atts->Get"<<attName<<"().c_str());" << endl;
        c << "            Message(msg);" << endl;
        c << "            atts->Set"<<attName<<"(atts->Get"<<attName<<"());" << endl;
        c << "        }" << endl;
    }
    virtual void            writeSourceUpdate(ofstream &c)
    {
        c << "            temp = atts->Get"<<attName<<"().c_str();" << endl;
        c << "            "<<name<<"->setText(temp);" << endl;
    }
    virtual void            writeSourceCallback(string windowname, ofstream &c)
    {
        c << "void" << endl;
        c << windowname<<"::"<<name<<"ProcessText()" << endl;
        c << "{" << endl;
        c << "    GetCurrentValues("<<index<<");" << endl;
        c << "    Apply();" << endl;
        c << "}" << endl;
    }
};
class StringVector : public StringBase
{
  public:
                            StringVector(string s) : StringBase(s, 0) {};
};
class StringArray : public StringBase
{
  public:
                            StringArray(string s, int n) : StringBase(s, n) {};
};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class Parser
{
  public:
    void Parse(string file, string classname);
  public:
    vector<DataType*>  members;
};


void
Parser::Parse(string filename, string classname)
{
    ifstream in(filename.c_str(), ios::in);
    char cbuff[4096];
    string buff;

    in.getline(cbuff, 4096);
    buff=cbuff;

    bool found = false;
    while (in)
    {
        if (stripWord(buff)  == "class:" &&
            stripWord(buff)  == classname)
        {
            found = true;
            break;
        }
        in.getline(cbuff, 4096);
        buff=cbuff;
    }

    if (!found)
    {
        cerr << "Couldn't find class '"<<classname<<"' in "<<filename<<endl;
        exit(-1);
    }

    in.getline(cbuff, 4096);
    buff=cbuff;
    stripWord(buff);
    cerr << "Class " << classname << ": " << buff << endl;
    cerr << "----------------------------------------------------------------------" << endl;

    in.getline(cbuff, 4096);
    buff=cbuff;

    while (in && findWord(buff)!="")
    {
        string membername = stripWord(buff);
        string datachar   = stripWord(buff);
        int    number     = -1;
        bool found = false;
        DataType *t = NULL;
        if      (datachar == "b" )  t = new Bool(membername);
        else if (datachar == "b*")  t = new BoolVector(membername);
        else if (datachar == "B" )  t = new BoolArray(membername, atoi(stripWord(buff).c_str()));
        else if (datachar == "i" )  t = new Int(membername);
        else if (datachar == "i*")  t = new IntVector(membername);
        else if (datachar == "I" )  t = new IntArray(membername, atoi(stripWord(buff).c_str()));
        else if (datachar == "c" )  t = new Char(membername);
        else if (datachar == "c*")  t = new CharVector(membername);
        else if (datachar == "C" )  t = new CharArray(membername, atoi(stripWord(buff).c_str()));
        else if (datachar == "u" )  t = new UChar(membername);
        else if (datachar == "u*")  t = new UCharVector(membername);
        else if (datachar == "U" )  t = new UCharArray(membername, atoi(stripWord(buff).c_str()));
        else if (datachar == "f" )  t = new Float(membername);
        else if (datachar == "f*")  t = new FloatVector(membername);
        else if (datachar == "F" )  t = new FloatArray(membername, atoi(stripWord(buff).c_str()));
        else if (datachar == "d" )  t = new Double(membername);
        else if (datachar == "d*")  t = new DoubleVector(membername);
        else if (datachar == "D" )  t = new DoubleArray(membername, atoi(stripWord(buff).c_str()));
        else if (datachar == "s" )  t = new String(membername);
        else if (datachar == "s*")  t = new StringVector(membername);
        else if (datachar == "S" )  t = new StringArray(membername, atoi(stripWord(buff).c_str()));
        else
            cerr << "unknown type: '" << datachar << "'" << endl;

        if (t)
            members.push_back(t);

        in.getline(cbuff, 4096);
        buff=cbuff;
    }
    

    in.close();
};

void
PrintOutput(Parser &parser, string classname, string windowname)
{
    // ------------------------------------------------------------------------
    // Header
    // ------------------------------------------------------------------------
    string headerfilename(windowname + ".h");
    ofstream h(headerfilename.c_str(), ios::out);

    h << "#ifndef " << toUpper(windowname) << "_H" << endl;
    h << "#define " << toUpper(windowname) << "_H" << endl;
    h << endl;
    h << "#include <QvisPostableWindowObserver.h>" << endl;
    h << "#include <AttributeSubject.h>" << endl;
    h << endl;
    h << "class " << classname << ";" << endl;
    h << "class QLabel;" << endl;
    h << "class QCheckBox;" << endl;
    h << "class QLineEdit;" << endl;
    h << "class QSpinBox;" << endl;
    h << "class QVBox;" << endl;
    h << endl;
    h << "class " << windowname << " : public QvisPostableWindowObserver" << endl;
    h << "{" << endl;
    h << "    Q_OBJECT" << endl;
    h << "  public:" << endl;
    h << "    " << windowname << "(const int type," << endl
      << "                         "<<classname<<" *subj," << endl
      << "                         const char *caption = 0," << endl
      << "                         const char *shortName = 0," << endl
      << "                         QvisNotepadArea *notepad = 0);" << endl;
    h << "    virtual ~"<<windowname<<"();" << endl;
    h << "    virtual void CreateWindow();" << endl;
    h << "  public slots:" << endl;
    h << "    virtual void apply();" << endl;
    h << "    virtual void makeDefault();" << endl;
    h << "    virtual void reset();" << endl;
    h << "  protected:" << endl;
    h << "    void UpdateWindow(bool doAll);" << endl;
    h << "    void GetCurrentValues(int which_widget);" << endl;
    h << "    void Apply(bool ignore = false);" << endl;
    h << "  private slots:" << endl;
    for (int i=0; i<parser.members.size(); i++)
    {
        parser.members[i]->writeHeaderCallback(h);
    }
    h << "  private:" << endl;
#ifdef OPERATOR
    h << "    int operatorType;" << endl;
#endif
#ifdef PLOT
    h << "    int plotType;" << endl;
#endif
    for (int i=0; i<parser.members.size(); i++)
    {
        parser.members[i]->writeHeaderData(h);
    }
    h << endl;
    h << "    "<<classname<<" *atts;" << endl;

    h << "};" << endl;
    h << endl;
    h << endl;
    h << endl;

    h << "#endif" << endl;

    h.close();


    // ------------------------------------------------------------------------
    // Source
    // ------------------------------------------------------------------------
    string sourcefilename(windowname + ".C");
    ofstream c(sourcefilename.c_str(), ios::out);

    c << "#include \""<<windowname<<".h\"" << endl;
    c << endl;
    c << "#include <"<<classname<<".h>" << endl;
    c << "#include <ViewerProxy.h>" << endl;
    c << endl;
    c << "#include <qcheckbox.h>" << endl;
    c << "#include <qlabel.h>" << endl;
    c << "#include <qlayout.h>" << endl;
    c << "#include <qlineedit.h>" << endl;
    c << "#include <qspinbox.h>" << endl;
    c << "#include <qvbox.h>" << endl;
    c << "#include <stdio.h>" << endl;
    c << "#include <string>" << endl;
    c << "using std::string;" << endl;

    // constructor
    c << windowname<<"::"<<windowname<<"(const int type," << endl
      << "                         "<<classname<<" *subj," << endl
      << "                         const char *caption," << endl
      << "                         const char *shortName," << endl
      << "                         QvisNotepadArea *notepad)" << endl;
    c << "    : QvisPostableWindowObserver(subj, caption, shortName, notepad)" << endl;
    c << "{" << endl;
#ifdef OPERATOR
    c << "    operatorType = type;" << endl;
#endif
#ifdef PLOT
    c << "    plotType = type;" << endl;
#endif
    c << "    atts = subj;" << endl;
    c << "}" << endl;
    c << endl;

    // destructor
    c << windowname<<"::~"<<windowname<<"()" << endl;
    c << "{" << endl;
    c << "}" << endl;
    c << endl;

    // CreateWindow
    c << "void" << endl;
    c << windowname<<"::CreateWindow()" << endl;
    c << "{" << endl;
    c << "    QGridLayout *mainLayout = new QGridLayout("
      << "topLayout, " << parser.members.size() << ",2,  10, \"mainLayout\");" << endl;
    c << endl;
    c << endl;
    for (int i=0; i<parser.members.size(); i++)
    {
        parser.members[i]->writeSourceCreate(c);
        c << endl;
    }
    c << "}" << endl;
    c << endl;
    c << endl;

    // reset
    c << "void" << endl;
    c << windowname << "::reset()" << endl;
    c << "{" << endl;
#ifdef OPERATOR
    c << "    viewer->ResetOperatorOptions(operatorType);" << endl;
#endif
#ifdef PLOT
    c << "    viewer->ResetPlotOptions(plotType);" << endl;
#endif
    c << "}" << endl;
    c << endl;
    c << endl;

    // apply
    c << "void" << endl;
    c << windowname << "::apply()" << endl;
    c << "{" << endl;
    c << "    Apply(true);" << endl;
    c << "}" << endl;
    c << endl;
    c << endl;

    // updatewindow
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
    c << "        switch(i)" << endl;
    c << "        {" << endl;
    for (int i=0; i<parser.members.size(); i++)
    {
        c << "          case "<<i<<": //"<<parser.members[i]->getName() << endl;
        parser.members[i]->writeSourceUpdate(c);
        c << "            break;" << endl;
    }
    c << "        }" << endl;
    c << "    }" << endl;
    c << "}" << endl;
    c << endl;
    c << endl;

    // callbacks
    for (int i=0; i<parser.members.size(); i++)
    {
        parser.members[i]->writeSourceCallback(windowname, c);
        c << endl;
        c << endl;
    }

    // getcurrent
    c << "void" << endl;
    c << windowname << "::GetCurrentValues(int which_widget)" << endl;
    c << "{" << endl;
    c << "    bool okay, doAll = (which_widget == -1);" << endl;
    c << "    QString msg, temp;" << endl;
    c << endl;
    for (int i=0; i<parser.members.size(); i++)
    {
        c << "    // Do " << parser.members[i]->getName() << endl;
        c << "    if(which_widget == "<<i<<" || doAll)" << endl;
        c << "    {" << endl;
        parser.members[i]->writeSourceGetCurrent(c);
        c << "    }" << endl;
        c << endl;
    }
    c << "}" << endl;
    c << endl;
    c << endl;

    // Apply
    c << "void" << endl;
    c << windowname<<"::Apply(bool ignore)" << endl;
    c << "{" << endl;
    c << "    if(AutoUpdate() || ignore)" << endl;
    c << "    {" << endl;
    c << "        GetCurrentValues(-1);" << endl;
    c << "        atts->Notify();" << endl;
    c << "" << endl;
#ifdef OPERATOR
    c << "        viewer->SetOperatorOptions(operatorType);" << endl;
#endif
#ifdef PLOT
    c << "        viewer->SetPlotOptions(plotType);" << endl;
#endif
    c << "    }" << endl;
    c << "    else" << endl;
    c << "        atts->Notify();" << endl;
    c << "}" << endl;

    // makeDefault
    c << "void" << endl;
    c << windowname<<"::makeDefault()" << endl;
    c << "{" << endl;
    c << "    GetCurrentValues(-1);" << endl;
    c << "    atts->Notify();" << endl;
#ifdef OPERATOR
    c << "    viewer->SetDefaultOperatorOptions(operatorType);" << endl;
#endif
#ifdef PLOT
    c << "    viewer->SetDefaultPlotOptions(plotType);" << endl;
#endif
    c << "}" << endl;


#if 0
    c << "void" << endl;
    c << windowname<<"::" << endl;
    c << endl;
    c << "void" << endl;
    c << windowname<<"::" << endl;
    c << endl;
    c << "void" << endl;
    c << windowname<<"::" << endl;
    c << endl;
    c << "void" << endl;
    c << windowname<<"::" << endl;
    c << endl;
    c << endl;
    c << endl;
    c << endl;
    c << endl;
    c << endl;
    c << endl;
#endif

    c.close();
}

int
main(int argc, char *argv[])
{
    cerr << endl;
    if (argc < 4)
    {
        cerr << "Usage:  " << argv[0] << " <deffile> <classname> <windowname>" << endl;
        cerr << endl;
        cerr << "(e.g. "<<argv[0]<<" Aslice.def AsliceAttributes QvisAsliceWindow)" << endl;
        cerr << endl;
        exit(-1);
    }

    string statefile(argv[1]);
    string classname(argv[2]);
    string windowname(argv[3]);

    Parser parser;
    parser.Parse(statefile.c_str(), classname);

    for (int i=0; i<parser.members.size(); i++)
    {
        cerr << "\t" << parser.members[i]->getType()
             << "\t" << parser.members[i]->getName() << endl;
    }

    cerr << "--------------------" << endl;
    cerr << "Writing to output files." << endl;
    PrintOutput(parser, classname, windowname);
}
