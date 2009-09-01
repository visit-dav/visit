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

#ifndef GENERATE_ATTS_H
#define GENERATE_ATTS_H
#include <QTextStream>

#include <vector>
#include <map>
#include <algorithm>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include "Field.h"

using std::vector;

#define GENERATOR_NAME "xml2atts"

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
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Wed Feb  6 16:51:14 PST 2002
//    Removed some of the source Initializer routines since the SetDefault
//    routines should work just fine when an initializer is needed for these.
//
//    Jeremy Meredith, Fri Apr 19 15:25:46 PDT 2002
//    Added constants, includes, and a codefile.  Made the HasFunction,
//    PrintFunction, DeleteFunction all check to make sure we are trying
//    to override a builtin function.  Made use of the prefix/postfix
//    additional code from the codefile.  Made use of overridden functions
//    in more places.
//
//    Jeremy Meredith, Wed May  8 15:26:37 PDT 2002
//    Added ability to have non-persistent objects.
//    Made enums work like enums, not just static ints.
//    Added some methods needed for keyframing.
//
//    Brad Whitlock, Tue May 14 10:23:57 PDT 2002
//    Added support for exporting symbols.
//
//    Kathleen Bonnell, Fri Jun 28 09:32:26 PDT 2002 
//    Implemented SetDefault for IntVector, DoubleVector, and StringVector.
//
//    Jeremy Meredith, Sat Aug  3 11:58:40 PDT 2002
//    Allowed attributes without fields.
//
//    Jeremy Meredith, Wed Aug 14 17:30:09 PDT 2002
//    Fixed a bug where an "int i;" was missing from some functions.
//
//    Jeremy Meredith, Tue Aug 27 14:32:50 PDT 2002
//    Added mfiles, dbtype, and libs.  Allowed NULL atts.
//
//    Jeremy Meredith, Thu Oct 17 15:58:29 PDT 2002
//    Added some enhancements for the XML editor.
//
//    Jeremy Meredith, Sun Nov 17 17:41:12 PST 2002
//    Fixed a compiler warning on the SGIs.
//
//    Sean Ahern, Mon Nov 18 14:43:14 PST 2002
//    Added widget files.
//
//    Brad Whitlock, Mon Nov 18 14:46:41 PST 2002
//    Ported to Windows.
//
//    Jeremy Meredith, Fri Jan 17 10:01:08 PST 2003
//    Made it handle AGs with multiple AG vectors.
//
//    Brad Whitlock, Mon Feb 10 13:19:36 PST 2003
//    I made it generate static methods to convert enums to and from strings.
//
//    Brad Whitlock, Thu Mar 13 12:36:58 PDT 2003
//    I added another argument to the Plugin constructor.
//
//    Brad Whitlock, Tue May 20 08:50:48 PDT 2003
//    I changed the code generation for the CreateNode method so it writes out
//    only the fields that differ from the default values for the class. I also
//    made the generated code have fewer unneeded variables.
//
//    Brad Whitlock, Tue Jul 15 13:22:36 PST 2003
//    I added range checking for enums so reading from config files is safer.
//
//    Brad Whitlock, Wed Jul 23 11:17:25 PDT 2003
//    I added code to generate a NewInstance method.
//
//    Jeremy Meredith, Tue Sep 23 16:17:41 PDT 2003
//    Changed haswriter to be a bool.
//    Made attributes with multiple vector fields use a single counter.
//
//    Brad Whitlock, Thu Oct 9 11:47:28 PDT 2003
//    I changed the code generation so that colors are always added to the
//    DataNode in CreateNode.
//
//    Jeremy Meredith, Wed Nov  5 13:28:03 PST 2003
//    Added ability to disable plugins by default.
//
//    Brad Whitlock, Wed Dec 17 11:55:11 PDT 2003
//    I changed the generated code so CreateNode has three args.
//
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Jeremy Meredith, Wed Sep  1 12:38:10 PDT 2004
//    Removed AttVector's header SetXXX method prototype.
//    Honor non-member user-defined functions.
//
//    Brad Whitlock, Wed Dec 8 15:48:10 PST 2004
//    Added support for variable names as a type.
//
//    Jeremy Meredith, Mon Apr  4 16:45:55 PDT 2005
//    Fixed capitalization for the att-vector helper methods.
//
//    Hank Childs, Tue May 24 09:54:36 PDT 2005
//    Added new argument to constructor for hasoptions.
//
//    Kathleen Bonnell, Mon Feb  6 16:23:30 PST 2006 
//    Allow assignment operator and 'SetProperty' methods from codefile to 
//    replace default generated methods. 
//
//    Hank Childs, Thu Jun  8 16:52:47 PDT 2006
//    Added copyright.
//
//    Cyrus Harrison, Wed Mar  7 09:47:13 PST 2007
//    Allow for engine-specific code in a plugin's source files
//
//    Brad Whitlock, Thu Mar 1 16:06:53 PST 2007
//    Added support for some AVT enums and for public/protected/private members.
//    I also changed the names of the generated access methods for attVectors.
//
//    Hank Childs, Tue Aug 28 14:46:34 PDT 2007
//    Print out a warning if someone declares a builtin function that is
//    being ignored.
//
//    Brad Whitlock, Fri Dec 14 14:13:02 PST 2007
//    Add case identifiers for fields.
//
//    Brad Whitlock, Fri Feb 15 15:01:39 PST 2008
//    Added some tests that should prevent source from being flagged as an
//    error in Klockwork.
//
//    Brad Whitlock, Thu Feb 28 14:10:04 PST 2008
//    Added generatorName and made it use HasCode,PrintCode instead of 
//    accessing the code file directly.
//
//    Jeremy Meredith, Thu Aug  7 14:34:01 EDT 2008
//    Reorder constructor initializers to be the correct order.
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
    virtual bool CanHaveConst() { return false; }
    virtual QString GetAttributeGroupID() = 0;
    virtual QString DataNodeConversion()
    {
        return "BadType";
    }
    virtual QString FieldID() const
    {
        return QString("ID_") + name;
    }
    virtual void AddAttributeIncludes(UniqueStringList &sl) const
    {
    }
    virtual void WriteHeaderForwardDeclarations(QTextStream &h) { }
    virtual void AddSystemIncludes(UniqueStringList &sl) { }
    virtual void WriteHeaderSelectFunction(QTextStream &h)
    {
        if (CanHaveConst())
        {
            h << "    void Select" << Name << "();" << Endl;
        }
    }
    virtual void WriteHeaderSetFunction(QTextStream &h)
    {
        h << "    void Set"<<Name<<"(";
        if (CanHaveConst())
            h << "const ";
        h << GetCPPName(true) << " ";
        if (isArray) 
            h << "*";
        else if (CanHaveConst())
            h << "&";
        h << name << "_);" << Endl;
    }
    virtual void WriteHeaderGetFunction(QTextStream &h, int w)
    {
        if (isArray)
        {
            h << "    const " << GetCPPNameW(w) << " *Get" << Name << "() const;" << Endl;
            h << "          " << GetCPPNameW(w) << " *Get" << Name << "();" << Endl;
        }
        else if (CanHaveConst())
        {
            h << "    const " << GetCPPNameW(w) << " &Get" << Name << "() const;" << Endl;
            h << "          " << GetCPPNameW(w) << " &Get" << Name << "();" << Endl;
        }
        else
            h << "    " << GetCPPNameW(w,true) << " Get" << Name << "() const;" << Endl;
    }
    virtual void WriteHeaderAGVectorProto(QTextStream &h) { }
    virtual void WriteHeaderSoloAGVectorProto(QTextStream &h) { }
    virtual void WriteHeaderAttribute(QTextStream &h, int w)
    {
        h << "    " << GetCPPNameW(w) << " " << name;
        if (isArray)
            h << "[" << length << "]";
        h << ";" << Endl;
    }
    // ------------------------------------------------------------------------
    virtual void WriteSourceIncludes(QTextStream &c)
    {
    }
    virtual void WriteSourceInitializer(QTextStream &c)
    {
    }
    virtual bool RequiresSourceInitializer() const
    {
        return false;
    }
    virtual void WriteSourceSetFunction(QTextStream &c, const QString &classname)
    {
        // Write prototype.
        c << "void" << Endl;
        c << classname << "::Set" << Name << "(";
        if (CanHaveConst())
            c << "const ";
        c << GetCPPName(true,classname) << " ";
        if (isArray)
            c << "*";
        else if (CanHaveConst())
            c << "&";
        c << name << "_)" << Endl;

        // Write function body
        c << "{" << Endl;

        QString codeName(QString("Set")+Name);
        if(HasCode(codeName, 0, generatorName))
        {
            PrintCode(c, codeName, 0, generatorName);
            c << Endl;
        }

        if (!isArray)
        {
            c << "    " << name << " = " << name << "_;" << Endl;
            c << "    Select(" << FieldID() << ", "
              << "(void *)&" << name << ");" << Endl;
        }
        else
        {
            if (length < 5)
            {
                for(int i = 0; i < length; ++i)
                    c << "    " << name << "[" << i << "] = " << name << "_[" << i << "];"<< Endl;
            }
            else
            {
                c << "    for(int i = 0; i < " << length << "; ++i)" << Endl;
                c << "        " << name << "[i] = " << name << "_[i];"<< Endl;
            }
            c << "    Select(" << FieldID() << ", (void *)" << name << ", " << length << ");" << Endl;
        }

        if(HasCode(codeName, 1, generatorName))
        {
            PrintCode(c, codeName, 1, generatorName);
            c << Endl;
        }

        c << "}" << Endl;
        c << Endl;
    }
    virtual void WriteSourceGetFunction(QTextStream &c, const QString &classname, bool doConst)
    {
        if (isArray)
        {
            if (doConst)
            {
                c << "const " << GetCPPName() << " *" << Endl << classname << "::Get"
                  << Name << "() const" << Endl;
            }
            else
            {
                c << GetCPPName() << " *" << Endl << classname << "::Get"
                  << Name << "()" << Endl;
            }
        }
        else if (CanHaveConst())
        {
            if (doConst)
            {
                c << "const " << GetCPPName() << " &" << Endl << classname
                  << "::Get" << Name << "() const" << Endl;
            }
            else
            {
                c << GetCPPName() << " &" << Endl << classname << "::Get"
                  << Name << "()" << Endl;
            }
        }
        else
        {
            c << GetCPPName(true,classname) << Endl << classname << "::Get"
              << Name << "() const" << Endl;
        }

        // Function body
        c << "{" << Endl;
        if (type == "attArray")
        {
            cErr << "EEEEEERRRRRRRRROOOOOOORRRRRRR" << Endl;
        }
        else
        {
            if (type == "enum")
                c << "    return " << GetCPPName(true) << "(" << name << ");" << Endl;
            else
                c << "    return " << name << ";" << Endl;
        }
        c << "}" << Endl;
        c << Endl;
    }
    virtual void WriteSourceSelectFunction(QTextStream &c, const QString &classname)
    {
        if (! CanHaveConst())
            return;

        c << "void" << Endl << classname << "::Select" << Name << "()" << Endl;
        c << "{" << Endl;
        if (isArray)
        {
            c << "    Select(" << FieldID() << ", (void *)"
              << name << ", " << length  << ");";
        }
        else
        {
            c << "    Select(" << FieldID() << ", (void *)&"
              << name << ");";
        }
        c << Endl << "}" << Endl << Endl;
    }
    virtual void WriteSourceAGVectorFunctions(QTextStream &c, const QString &classname, const QString &purpose)
    {
    }
    virtual void WriteSourceSoloAGVectorFunctions(QTextStream &c, const QString &classname, const QString &purpose)
    {
    }
    virtual void WriteSourceSetDefault(QTextStream &c) = 0;
    virtual void WriteSourceCopyCode(QTextStream &c)
    {
        if (isArray)
        {
            if (length < 4)
            {
                for(int i = 0; i < length; ++i)
                {
                    c << "    " << name << "[" << i << "] = "
                      << "obj." << name << "[" << i << "];" << Endl;
                }
            }
            else
            {
                c << "    for(int i = 0; i < " << length << "; ++i)" << Endl;
                c << "        " << name << "[i] = " 
                  <<     "obj." << name << "[i];" << Endl;
            }
            c << Endl;
        }
        else
        {
            c << "    " << name << " = obj." << name << ";" << Endl;
        }
    }
    virtual void WriteSourceDestructor(QTextStream &c)
    {
    }
    virtual void WriteSourceSetFromNode(QTextStream &c)
    {
        c << "    if((node = searchNode->GetNode(\"" << name
          << "\")) != 0)" << Endl;
        c << "        Set" << Name << "(";
        c << "node->" << DataNodeConversion() << "());" << Endl;
    }
    virtual void WriteSourceSubAttributeGroupSingle(QTextStream &c)
    {
    }
    virtual void WriteSourceSubAttributeGroup(QTextStream &c)
    {
    }
    virtual void WriteSourceComparisonPrecalc(QTextStream &c, const QString &indent)
    {
        if (isArray)
        {
            c << indent << "// Compare the " << name << " arrays." << Endl;
            c << indent << "bool " << name << "_equal = true;" << Endl; 
            c << indent << "for(int i = 0; i < " << length << " && "
              << name << "_equal; ++i)" << Endl;
            c << indent << "    " << name << "_equal = ("
              << name << "[i] == obj." << name
              << "[i]);" << Endl << Endl;
        }
    }
    virtual void WriteSourceComparison(QTextStream &c)
    {
        if (isArray)
            c << name << "_equal";
        else
        {
            c << "(" << name << " == obj." << name << ")";
        }
    }
    virtual QString GetFieldType() const { return type; }
};

//
// ------------------------------------ Int -----------------------------------
//
class AttsGeneratorInt : public virtual Int , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorInt(const QString &n, const QString &l)
        : Field("int",n,l), Int(n,l), AttsGeneratorField("int",n,l) { }
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

//
// -------------------------------- IntArray --------------------------------
//
class AttsGeneratorIntArray : public virtual IntArray , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorIntArray(const QString &s, const QString &n, const QString &l)
        : Field("intArray",n,l), IntArray(s,n,l), AttsGeneratorField("intArray",n,l) { }
    virtual bool CanHaveConst() { return true; }
    virtual QString GetAttributeGroupID()
    {
        return "I";
    }
    virtual QString DataNodeConversion()
    {
        return "AsIntArray";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        for (int i=0; i<length; i++)
            c << "    " << name << "["<<i<<"] = " << val[i] << ";" << Endl;
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
    virtual bool CanHaveConst() { return true; }
    virtual QString GetAttributeGroupID()
    {
        return "i*";
    }
    virtual QString DataNodeConversion()
    {
        return "AsIntVector";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        for (size_t i=0; i < val.size(); i++)
            c << "    " << name << ".push_back(" << val[i] << ");" << Endl;
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
    virtual QString GetAttributeGroupID()
    {
        return "b";
    }
    virtual QString DataNodeConversion()
    {
        return "AsBool";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        c << "    " << name << " = " << (val ? "true" : "false") << ";" << Endl;
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
    virtual QString GetAttributeGroupID()
    {
        return "f";
    }
    virtual QString DataNodeConversion()
    {
        return "AsFloat";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        c << "    " << name << " = " << val << ";" << Endl;
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
    virtual bool CanHaveConst() { return true; }
    virtual QString GetAttributeGroupID()
    {
        return "F";
    }
    virtual QString DataNodeConversion()
    {
        return "AsFloatArray";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        for (int i=0; i<length; i++)
            c << "    " << name << "["<<i<<"] = " << val[i] << ";" << Endl;
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
    virtual QString GetAttributeGroupID()
    {
        return "d";
    }
    virtual QString DataNodeConversion()
    {
        return "AsDouble";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        c << "    " << name << " = " << val << ";" << Endl;
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
    virtual bool CanHaveConst() { return true; }
    virtual QString GetAttributeGroupID()
    {
        return "D";
    }
    virtual QString DataNodeConversion()
    {
        return "AsDoubleArray";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        for (int i=0; i<length; i++)
            c << "    " << name << "["<<i<<"] = " << val[i] << ";" << Endl;
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
    virtual bool CanHaveConst() { return true; }
    virtual QString GetAttributeGroupID()
    {
        return "d*";
    }
    virtual QString DataNodeConversion()
    {
        return "AsDoubleVector";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        for (size_t i=0; i < val.size(); i++)
            c << "    " << name << ".push_back(" << val[i] << ");" << Endl;
    }
};


//
// ----------------------------------- UChar ----------------------------------
//
class AttsGeneratorUChar : public virtual UChar , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorUChar(const QString &n, const QString &l)
        : Field("uchar",n,l), UChar(n,l), AttsGeneratorField("uchar",n,l) { }
    virtual QString GetAttributeGroupID()
    {
        return "u";
    }
    virtual QString DataNodeConversion()
    {
        return "AsUnsignedChar";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        c << "    " << name << " = " << int(val) << ";" << Endl;
    }
};


//
// -------------------------------- UCharArray --------------------------------
//
class AttsGeneratorUCharArray : public virtual UCharArray , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorUCharArray(const QString &s, const QString &n, const QString &l)
        : Field("ucharArray",n,l), UCharArray(s,n,l), AttsGeneratorField("ucharArray",n,l) { }
    virtual bool CanHaveConst() { return true; }
    virtual QString GetAttributeGroupID()
    {
        return "U";
    }
    virtual QString DataNodeConversion()
    {
        return "AsUnsignedCharArray";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        for (int i=0; i<length; i++)
            c << "    " << name << "["<<i<<"] = " << int(val[i]) << ";" << Endl;
    }
};


//
// ------------------------------- unsignedCharVector -------------------------------
//
class AttsGeneratorUCharVector : public virtual UCharVector , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorUCharVector(const QString &n, const QString &l)
        : Field("ucharVector",n,l), UCharVector(n,l), AttsGeneratorField("ucharVector",n,l) { }
    virtual bool CanHaveConst() { return true; }
    virtual QString GetAttributeGroupID()
    {
        return "u*";
    }
    virtual QString DataNodeConversion()
    {
        return "AsUnsignedCharVector";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        for (size_t i=0; i < val.size(); i++)
            c << "    " << name << ".push_back(" << val[i] << ");" << Endl;
    }
};


//
// ---------------------------------- String ----------------------------------
//
class AttsGeneratorString : public virtual String , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorString(const QString &n, const QString &l)
        : Field("string",n,l), String(n,l), AttsGeneratorField("string",n,l) { }
    virtual bool CanHaveConst() { return true; }
    virtual void AddSystemIncludes(UniqueStringList &sl) 
    { 
        sl.AddString("#include <string>\n");
    }
    virtual QString GetAttributeGroupID()
    {
        return "s";
    }
    virtual QString DataNodeConversion()
    {
        return "AsString";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        c << "    " << name << " = \"" << val << "\";" << Endl;
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
    virtual bool CanHaveConst() { return true; }
    virtual void AddSystemIncludes(UniqueStringList &sl) 
    { 
        sl.AddString("#include <string>\n");
    }
    virtual QString GetAttributeGroupID()
    {
        return "s*";
    }
    virtual QString DataNodeConversion()
    {
        return "AsStringVector";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        for (size_t i=0; i < val.size(); i++)
        c << "    " << name << ".push_back(" << "\"" << val[i] << "\");" << Endl;
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
    virtual bool CanHaveConst() { return true; }
    virtual void AddSystemIncludes(UniqueStringList &sl) 
    { 
        sl.AddString("#include <string>\n");
    }
    virtual QString GetAttributeGroupID()
    {
        return "s";
    }
    virtual QString DataNodeConversion()
    {
        return "AsString";
    }
    virtual bool RequiresSourceInitializer() const
    {
        return true;
    }
    virtual void WriteSourceInitializer(QTextStream &c)
    {
        if(valueSet)
            c << name << "(\""<<val<<"\")";
        else
            c << name << "(\"hot\")";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
    }
};


//
// ----------------------------------- Color ----------------------------------
//
class AttsGeneratorColor : public virtual Color , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorColor(const QString &n, const QString &l)
        : Field("color",n,l), Color(n,l), AttsGeneratorField("color",n,l) { }
    virtual bool CanHaveConst() { return true; }
    virtual void AddAttributeIncludes(UniqueStringList &sl) const
    {
        sl.AddString("#include <ColorAttribute.h>\n");
    }
    virtual QString GetAttributeGroupID()
    {
        return "a";
    }
    virtual bool RequiresSourceInitializer() const
    {
        return true;
    }
    virtual void WriteSourceInitializer(QTextStream &c)
    {
        if(valueSet)
        {
            if(val[3] == 255)
                c << name << "(" << int(val[0]) << ", " << int(val[1]) << ", "
                                 << int(val[2]) << ")";
            else
                c << name << "(" << int(val[0]) << ", " << int(val[1]) << ", "
                                 << int(val[2]) << ", " << int(val[3]) << ")";
        }
        else
            c << name << "()";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
    }
    virtual void WriteSourceSetFromNode(QTextStream &c)
    {
        c << "    if((node = searchNode->GetNode(\"" << name
          << "\")) != 0)" << Endl;
        c << "        " << name << ".SetFromNode(node);" << Endl;
    }
};


//
// --------------------------------- LineStyle --------------------------------
//
class AttsGeneratorLineStyle : public virtual LineStyle , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorLineStyle(const QString &n, const QString &l)
        : Field("linestyle",n,l), LineStyle(n,l), AttsGeneratorField("linestyle",n,l) { }
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


//
// --------------------------------- LineWidth --------------------------------
//
class AttsGeneratorLineWidth : public virtual LineWidth , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorLineWidth(const QString &n, const QString &l)
        : Field("linewidth",n,l), LineWidth(n,l), AttsGeneratorField("linewidth",n,l) { }
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


//
// --------------------------------- Opacity ----------------------------------
//
class AttsGeneratorOpacity : public virtual Opacity , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorOpacity(const QString &n, const QString &l)
        : Field("opacity",n,l), Opacity(n,l), AttsGeneratorField("opacity",n,l) { }
    virtual QString GetAttributeGroupID()
    {
        return "d";
    }
    virtual QString DataNodeConversion()
    {
        return "AsDouble";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        c << "    " << name << " = " << val << ";" << Endl;
    }
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
    virtual bool CanHaveConst() { return true; }
    virtual void AddSystemIncludes(UniqueStringList &sl) 
    { 
        sl.AddString("#include <string>\n");
    }
    virtual QString GetAttributeGroupID()
    {
        return "s";
    }
    virtual QString DataNodeConversion()
    {
        return "AsString";
    }
    virtual bool RequiresSourceInitializer() const
    {
        return true;
    }
    virtual void WriteSourceInitializer(QTextStream &c)
    {
        if(valueSet)
            c << name << "(\""<<val<<"\")";
        else
            c << name << "(\"default\")";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
    }
};


//
// ------------------------------------ Att -----------------------------------
//
class AttsGeneratorAtt : public virtual Att , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorAtt(const QString &t, const QString &n, const QString &l)
        : Field("att",n,l), Att(t,n,l), AttsGeneratorField("att",n,l) { }
    virtual bool CanHaveConst() { return true; }
    virtual void AddAttributeIncludes(UniqueStringList &sl) const
    {
        QString tmp(QString("#include <%1.h>\n").arg(attType));
        sl.AddString(tmp);
    }
    virtual QString GetAttributeGroupID()
    {
        return "a";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
    }
    virtual void WriteSourceSetFromNode(QTextStream &c)
    {
        c << "    if((node = searchNode->GetNode(\"" << name
          << "\")) != 0)" << Endl;
        c << "        " << name << ".SetFromNode(node);" << Endl;
    }
};


//
// --------------------------------- AttVector --------------------------------
//
class AttsGeneratorAttVector : public virtual AttVector , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorAttVector(const QString &t, const QString &n, const QString &l)
        : Field("attVector",n,l), AttVector(t,n,l), AttsGeneratorField("attVector",n,l) { }
    virtual bool CanHaveConst() { return true; }
    virtual void WriteHeaderForwardDeclarations(QTextStream &h)
    {
        h << "class " << attType << ";" << Endl;
    }
    virtual void WriteHeaderAGVectorProto(QTextStream &h)
    {
        QString plural("");
        if(Name.right(1) != "s")
            plural = "s";

        h << "    void Add" << Name << "(const " << attType << " &);" << Endl;
        h << "    void Clear" << Name << plural << "();" << Endl;
        h << "    void Remove" << Name << "(int i);" << Endl;
        h << "    int  GetNum" << Name << plural << "() const;" << Endl;
        h << "    " << attType << " &Get" << Name << "(int i);" << Endl;
        h << "    const " << attType << " &Get" << Name << "(int i) const;" << Endl;
        h << Endl;
    }
    virtual void WriteHeaderSoloAGVectorProto(QTextStream &h)
    {
        h << "    " << attType << " &operator [] (int i);" << Endl;
        h << "    const " << attType << " &operator [] (int i) const;" << Endl;
        h << Endl;
    }
    virtual void WriteSourceIncludes(QTextStream &c)
    {
        c << "#include <"<<attType<<".h>" << Endl;
    }
    virtual QString GetAttributeGroupID()
    {
        return "a*";
    }
    virtual void WriteSourceSetDefault(QTextStream &c)
    {
    }
    virtual void WriteHeaderSetFunction(QTextStream &h)
    {
        // Disabled the header method because the .C implementation (below)
        // does not output anything.
    }
    virtual void WriteSourceSetFunction(QTextStream &c, const QString &classname)
    {
        /*
        if (codeFile && codeFile->code.count(QString("Set")+Name) && !codeFile->code[QString("Set")+Name].first.isNull())
            c << codeFile->code[QString("Set")+Name].first;

        if (codeFile && codeFile->code.count(QString("Set")+Name) && !codeFile->code[QString("Set")+Name].second.isNull())
            c << codeFile->code[QString("Set")+Name].second;
        */
    }
    virtual void WriteSourceCopyCode(QTextStream &c)
    {
        c << "    // *** Copy the " << name << " field ***" << Endl;
        c << "    // Delete the AttributeGroup objects and clear the vector." << Endl;
        c << "    for(pos = " << name << ".begin(); pos != "
          << name << ".end(); ++pos)" << Endl;
        c << "        delete *pos;" << Endl;
        c << "    " << name << ".clear();" << Endl;
        c << "    if(obj." << name << ".size() > 0)" << Endl;
        c << "        " << name << ".reserve(obj." << name << ".size());" << Endl;
        c << "    // Duplicate the " << name << " from obj." << Endl;
        c << "    for(pos = obj." << name << ".begin(); pos != obj." << name << ".end(); ++pos)" << Endl;
        c << "    {" << Endl;
        c << "        " << attType << " *old" << attType << " = (" << attType << " *)(*pos);" << Endl;
        c << "        " << attType << " *new" << attType << " = new " << attType << "(*old" << attType << ");" << Endl;
        c << "        " << name << ".push_back(new" << attType << ");" << Endl;
        c << "    }" << Endl << Endl;
    }
    virtual void WriteSourceDestructor(QTextStream &c)
    {
        c << "    // Destroy the " << name << " field." << Endl;
        c << "    for(pos = " << name << ".begin();"
          << " pos != " << name << ".end(); ++pos)" << Endl;
        c << "        delete *pos;" << Endl;
    }
    virtual void WriteSourceSetFromNode(QTextStream &c)
    {
        QString plural("");
        if(Name.right(1) != "s")
            plural = "s";

        c << "    // Clear all the " << attType << "s." << Endl;
        c << "    Clear" << Name << plural << "();" << Endl;
        c << Endl;
        c << "    // Go through all of the children and construct a new" << Endl;
        c << "    // " << attType << " for each one of them." << Endl;
                  
        c << "    children = searchNode->GetChildren();" << Endl;
        c << "    if(children != 0)" << Endl;
        c << "    {" << Endl;
        c << "        for(int i = 0; i < searchNode->GetNumChildren(); ++i)" << Endl;
        c << "        {" << Endl;
        c << "            if(children[i]->GetKey() == std::string(\"" << attType << "\"))" << Endl;
        c << "            {" << Endl;
        c << "                " << attType << " temp;" << Endl;
        c << "                temp.SetFromNode(children[i]);" << Endl;
        c << "                Add" << Name << "(temp);" << Endl;
        c << "            }" << Endl;
        c << "        }" << Endl;
        c << "    }" << Endl << Endl;
    }
    virtual void WriteSourceAGVectorFunctions(QTextStream &c, const QString &classname, const QString &purpose)
    {
        QString methodName;
        QString s = attType;
        QString plural("");
        if(Name.right(1) != "s")
            plural = "s";

        // Write the Add method.
        methodName = "Add";
        methodName += Name;
        WriteMethodComment(c, classname, methodName, purpose, generatorName);
        c << "void" << Endl;
        c << classname << "::" << methodName << "(const " << s << " &obj)" << Endl;
        c << "{" << Endl;
        c << "    " << s << " *new" << s << " = new " << s << "(obj);" << Endl;
        c << "    " << name << ".push_back(new" << s << ");" << Endl;
        c << Endl;
        c << "    // Indicate that things have changed by selecting it." << Endl;
        c << "    Select(" << FieldID() << ", (void *)&" << name << ");" << Endl;
        c << "}" << Endl << Endl;

        // Write the Clear method
        methodName = "Clear";
        methodName += Name + plural;
        WriteMethodComment(c, classname, methodName, purpose, generatorName);
        c << "void" << Endl;
        c << classname << "::" << methodName << "()" << Endl;
        c << "{" << Endl;
        c << "    AttributeGroupVector::iterator pos;" << Endl;
        c << Endl;
        c << "    for(pos = " << name << ".begin(); pos != " << name << ".end(); ++pos)" << Endl;
        c << "        delete *pos;" << Endl;
        c << "    " << name << ".clear();" << Endl;
        c << Endl;
        c << "    // Indicate that things have changed by selecting the list." << Endl;
        c << "    Select(" << FieldID() << ", (void *)&" << name << ");" << Endl;
        c << "}" << Endl << Endl;

        // Write the Remove method
        methodName = "Remove";
        methodName += Name;
        WriteMethodComment(c, classname, methodName, purpose, generatorName);
        c << "void" << Endl;
        c << classname << "::" << methodName << "(int index)" << Endl;
        c << "{" << Endl;
        c << "    AttributeGroupVector::iterator pos = " << name << ".begin();" << Endl;
        c << Endl;
        c << "    // Iterate through the vector \"index\" times. " << Endl;
        c << "    for(int i = 0; i < index; ++i)" << Endl;
        c << "        if(pos != " << name << ".end()) ++pos;" << Endl;
        c << Endl;
        c << "    // If pos is still a valid iterator, remove that element." << Endl;
        c << "    if(pos != " << name << ".end())" << Endl;
        c << "    {" << Endl;
        c << "        delete *pos;" << Endl;
        c << "        " << name << ".erase(pos);" << Endl;
        c << "    }" << Endl;
        c << Endl;
        c << "    // Indicate that things have changed by selecting the list." << Endl;
        c << "    Select(" << FieldID() << ", (void *)&" << name << ");" << Endl;
        c << "}" << Endl << Endl;

        // Write the GetNum method
        methodName = "GetNum";
        methodName += Name + plural;
        WriteMethodComment(c, classname, methodName, purpose, generatorName);
        c << "int" << Endl;
        c << classname << "::" << methodName << "() const" << Endl;
        c << "{" << Endl;
        c << "    return " << name << ".size();" << Endl;
        c << "}" << Endl << Endl;

        // Write the Get method
        methodName = "Get";
        methodName += Name;
        WriteMethodComment(c, classname, methodName, purpose, generatorName);
        c << s << " &" << Endl;
        c << classname << "::" << methodName << "(int i)" << Endl;
        c << "{" << Endl;
        c << "    return *((" << s << " *)" << name << "[i]);" << Endl;
        c << "}" << Endl << Endl;

        // Write the const Get method
        methodName = "Get";
        methodName += Name;
        WriteMethodComment(c, classname, methodName, purpose, generatorName);
        c << "const " << s << " &" << Endl;
        c << classname << "::" << methodName << "(int i) const" << Endl;
        c << "{" << Endl;
        c << "    return *((" << s << " *)" << name << "[i]);" << Endl;
        c << "}" << Endl << Endl;
    }
    virtual void WriteSourceSoloAGVectorFunctions(QTextStream &c, const QString &classname, const QString &purpose)
    {
        QString methodName;
        QString s = attType;

        // Write the non-const [] operator
        methodName = "operator []";
        WriteMethodComment(c, classname, methodName, purpose, generatorName);
        c << s << " &" << Endl;
        c << classname << "::" << methodName << " (int i)" << Endl;
        c << "{" << Endl;
        c << "    return *((" << s << " *)" << name << "[i]);" << Endl;
        c << "}" << Endl << Endl;

        // Write the const [] operator
        WriteMethodComment(c, classname, methodName, purpose, generatorName);
        c << "const " << s << " &" << Endl;
        c << classname << "::" << methodName << " (int i) const" << Endl;
        c << "{" << Endl;
        c << "    return *((" << s << " *)" << name << "[i]);" << Endl;
        c << "}" << Endl << Endl;
    }
    virtual void WriteSourceSubAttributeGroupSingle(QTextStream &c)
    {
        c << "    return new " << attType << ";" << Endl;
    }
    virtual void WriteSourceSubAttributeGroup(QTextStream &c)
    {
        c << "    case " << FieldID() << ":" << Endl;
        c << "        retval = new " << attType << ";" << Endl;
        c << "        break;" << Endl;
    }
    virtual void WriteSourceComparisonPrecalc(QTextStream &c, const QString &indent)
    {
        QString s = attType;
        c << indent << "bool " << name << "_equal = (obj." << name 
          << ".size() == " << name << ".size());" << Endl;
        c << indent << "for(size_t i = 0; (i < " << name
          << ".size()) && " << name << "_equal; ++i)" << Endl;
        c << indent << "{" << Endl;
        c << indent << "    // Make references to " << s
          << " from AttributeGroup *." << Endl;
        c << indent << "    const " << s << " &" << name
          << "1 = *((const " << s << " *)(" << name << "[i]));" << Endl;
        c << indent << "    const " << s << " &" << name
          << "2 = *((const " << s << " *)(obj." << name << "[i]));" << Endl;
        c << indent << "    " << name << "_equal = ("
          << name << "1 == " << name << "2);" << Endl;
        c << indent << "}" << Endl << Endl;
    }
    virtual void WriteSourceComparison(QTextStream &c)
    {
        c << name << "_equal";
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
    virtual QString GetAttributeGroupID()
    {
        return "i";
    }
    virtual QString DataNodeConversion()
    {
        return "AsInt";
    }

    virtual void AddSystemIncludes(UniqueStringList &sl) 
    { 
        sl.AddString("#include <string>\n");
    }

    virtual void WriteSourceSetDefault(QTextStream &c)
    {
        c << "    " << name << " = " <<enumType->values[val] << ";" << Endl;
    }
    virtual void WriteSourceSetFromNode(QTextStream &c)
    {
        c << "    if((node = searchNode->GetNode(\"" << name
          << "\")) != 0)" << Endl;
        c << "    {" << Endl;
        c << "        // Allow enums to be int or string in the config file" << Endl;
        c << "        if(node->GetNodeType() == INT_NODE)" << Endl;
        c << "        {" << Endl;
        c << "            int ival = node->AsInt();" << Endl;
        c << "            if(ival >= 0 && ival < " << enumType->values.size() << ")" << Endl;
        c << "                Set" << Name << "(" << GetCPPName(true) << "(ival));" << Endl;
        c << "        }" << Endl;
        c << "        else if(node->GetNodeType() == STRING_NODE)" << Endl;
        c << "        {" << Endl;
        c << "            " << GetCPPName(true) << " value;" << Endl;
        c << "            if(" << GetCPPName(true) << "_FromString(node->AsString(), value))" << Endl;
        c << "                Set" << Name << "(value);" << Endl;
        c << "        }" << Endl;
        c << "    }" << Endl;
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
    virtual void AddSystemIncludes(UniqueStringList &sl) \
    { \
        sl.AddString("#include <avtTypes.h>\n");\
    }\
    virtual QString GetAttributeGroupID()\
    {\
        return "i";\
    }\
    virtual QString DataNodeConversion()\
    {\
        return "AsInt";\
    }\
    virtual void WriteSourceSetDefault(QTextStream &c)\
    {\
        c << "    " << name << " = ";\
        int nsym = 0;\
        const char **sym = GetSymbols(nsym);\
        if(val >= 0 && val < nsym)\
            c << sym[val];\
        else\
            c << val;\
        c << ";" << Endl;\
    }\
    virtual QString GetFieldType() const { return "int"; }

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
//    Brad Whitlock, Wed Dec 8 15:47:03 PST 2004
//    Added support for variable names.
//
//    Brad Whitlock, Thu Mar 1 14:17:10 PST 2007
//    Added support for various avt enums.
//
//    Kathleen Bonnell, Thu Mar 22 16:58:23 PDT 2007 
//    Added scalemode.
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
//   Brad Whitlock, Tue May 14 10:30:51 PDT 2002
//   Added export support.
//
//   Mark C. Miller, Tue Oct 26 16:18:21 PDT 2004
//   Changed operator = to return & to class & check for assignment to self
//
//   Brad Whitlock, Wed Mar 7 15:57:54 PST 2007
//   Added ability to turn off keyframing methods.
//
//   Jeremy Meredith, Tue Mar 13 15:17:14 EDT 2007
//   Made non-member constants written before the class, and don't add
//   whitespace for empty constant defs (like #defines in the header file).
//
//   Hank Childs, Thu Jan 10 14:33:30 PST 2008
//   Added filenames, specifiedFilenames.
//
//   Brad Whitlock, Thu Feb 28 12:03:30 PDT 2008
//   Made it use a base class so it can inherit some methods.
//
//   Mark C. Miller, Tue Mar 18 20:56:47 PDT 2008
//   Made it write field id enum regardless of only public fields
//
//   Mark C. Miller, Wed Aug 26 11:01:01 PDT 2009
//   Added custom base class for derived state objects.
//
//   Mark C. Miller, Mon Aug 31 14:05:19 PDT 2009
//   Added logic to support header/source file prefix/postfix code blocks.
//   Added logic to support prefix/postfix code blocks in destructor.
//
//   Mark C. Miller, Tue Sep  1 09:19:06 PDT 2009
//   Fixed missing call to SelectAll in implementation of Init() func.
// ----------------------------------------------------------------------------
#include <GeneratorBase.h>

class AttsGeneratorAttribute : public GeneratorBase
{
  public:
    vector<AttsGeneratorField*> fields;
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
    void WriteHeader(QTextStream &h)
    {
        h << copyright_str << Endl;
        h << "#ifndef " << name.toUpper() << "_H" << Endl;
        h << "#define " << name.toUpper() << "_H" << Endl;
        if(!exportInclude.isEmpty())
            h << "#include <" << exportInclude << ">" << Endl;
        WriteHeaderSystemIncludes(h);
        h << "#include <" << baseClass << ".h>" << Endl;
        h << Endl;
        WriteHeaderForwardDeclarations(h);
        WriteHeaderAttributeIncludes(h);
        // write user header includes
        for (size_t i=0; i<includes.size(); i++)
        {
            if (includes[i]->destination=="header" &&
                includes[i]->target == generatorName)
            {
                if (includes[i]->quoted == true)
                    h << "#include \"" << includes[i]->include << "\"" << Endl;
                else
                    h << "#include <"  << includes[i]->include << ">"  << Endl;
            }
        }
        if(HasCode(name+".h", 0))
        {
            PrintCode(h, name+".h", 0);
            h << Endl;
        }

        h << Endl;

        // write non-member constants
        for (size_t i=0; i<constants.size(); i++)
        {
            if (! constants[i]->member && constants[i]->target == generatorName)
                h << constants[i]->decl << Endl;
        }

        WriteClassComment(h, purpose);
        if(exportAPI.isEmpty())
            h << "class " << name << " : public " << baseClass << Endl;
        else
            h << "class " << exportAPI << " " << name << " : public " << baseClass << Endl;
        h << "{" << Endl;
        h << "public:" << Endl;
        for (size_t i=0; i<EnumType::enums.size(); i++)
        {
            h << "    enum " << EnumType::enums[i]->type << Endl;
            h << "    {" << Endl;
            for (size_t j=0; j<EnumType::enums[i]->values.size(); j++)
            {
                h << "        " << EnumType::enums[i]->values[j];
                if (j < EnumType::enums[i]->values.size()-1)
                    h << ",";
                h << Endl;
            }
            h << "    };" << Endl;
        }
        // write member constants
        for (size_t i=0; i<constants.size(); i++)
        {
            if (constants[i]->member && constants[i]->target == generatorName)
                h << "    " << constants[i]->decl << Endl;
        }
        if (EnumType::enums.size() || constants.size())
            h << Endl;
        h << "    // These constructors are for objects of this class" << Endl;
        h << "    " << name << "();" << Endl;
        h << "    " << name << "(const " << name << " &obj);" << Endl;
        h << "protected:" << Endl;
        h << "    // These constructors are for objects derived from this class" << Endl;
        h << "    " << name << "(private_tmfs_t tmfs);" << Endl;
        h << "    " << name << "(const " << name << " &obj, private_tmfs_t tmfs);" << Endl;
        h << "public:" << Endl;
        h << "    virtual ~" << name << "();" << Endl;
        h << Endl;
        h << "    virtual " << name << "& operator = (const " << name << " &obj);" << Endl;
        h << "    virtual bool operator == (const " << name << " &obj) const;" << Endl;
        h << "    virtual bool operator != (const " << name << " &obj) const;" << Endl;
        h << "private:" << Endl;
        h << "    void Init();" << Endl;
        h << "    void Copy(const " << name << " &obj);" << Endl;
        h << "public:" << Endl;
        h << Endl;
        h << "    virtual const std::string TypeName() const;" << Endl;
        h << "    virtual bool CopyAttributes(const AttributeGroup *);" << Endl;
        h << "    virtual AttributeSubject *CreateCompatible(const std::string &) const;" << Endl;
        h << "    virtual AttributeSubject *NewInstance(bool) const;" << Endl;
        h << Endl;
        h << "    // Property selection methods" << Endl;
        h << "    virtual void SelectAll();" << Endl;
        WriteHeaderSelectFunctions(h);

        // Determine whether there are public and private fields.
        bool hasPrivateFields = false;
        bool hasProtectedFields = false;
        bool hasPublicFields = false;
        for (size_t i=0; i<fields.size(); i++)
        {
            if(fields[i]->accessType == Field::AccessPrivate)
            {
                hasPrivateFields = true;
            }
            if(fields[i]->accessType == Field::AccessProtected)
            {
                hasProtectedFields = true;
            }
            if(fields[i]->accessType == Field::AccessPublic)
                hasPublicFields = true;
        }

        if(hasPrivateFields)
        {
            h << Endl;
            h << "    // Property setting methods" << Endl;
        }
        // Write out all the set prototypes
        for (size_t i=0; i<fields.size(); i++)
        {
            if(fields[i]->accessType != Field::AccessPrivate)
                continue;
            fields[i]->WriteHeaderSetFunction(h);
        }

        int totalWidth = CalculateTotalWidth(true);
        if(hasPrivateFields)
        {
            h << Endl;
            h << "    // Property getting methods" << Endl;
        }
        // Write out all the get prototypes
        for (size_t i=0; i<fields.size(); i++)
        {
            if(fields[i]->accessType != Field::AccessPrivate)
                continue;
            int minus = 0;
            if (fields[i]->CanHaveConst())
                minus = 6;

            fields[i]->WriteHeaderGetFunction(h, totalWidth - minus);
        }

        if (persistent)
        {
            // Persistence methods
            h << Endl;
            h << "    // Persistence methods" << Endl;
            h << "    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);" << Endl;
            h << "    virtual void SetFromNode(DataNode *node);" << Endl;
        }
        h << Endl;

        // If there are AttributeGroupVectors in the list, write out
        // the convenience methods.
        if (HaveAGVectors())
        {
            h << Endl;
            h << "    // Attributegroup convenience methods" << Endl;
            for (size_t i=0; i<fields.size(); i++)
            {
                fields[i]->WriteHeaderAGVectorProto(h);
            }
        }
        if (HaveSoloAGVector())
        {
            for (size_t i=0; i<fields.size(); i++)
            {
                fields[i]->WriteHeaderSoloAGVectorProto(h);
            }
        }

        WriteHeaderEnumConversions(h);

        // Methods for keyframing
        if(keyframe)
        {
        h << Endl;
        h << "    // Keyframing methods" << Endl;
        h << "    virtual std::string               GetFieldName(int index) const;" << Endl;
        h << "    virtual AttributeGroup::FieldType GetFieldType(int index) const;" << Endl;
        h << "    virtual std::string               GetFieldTypeName(int index) const;" << Endl;
        h << "    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;" << Endl;
        h << Endl;
        }

        // Write user-defined methods
        bool wroteUserDefinedHeading = false;
        for (size_t i=0; i<functions.size(); i++)
        {
            if (functions[i]->user && 
                functions[i]->member &&
                functions[i]->target == generatorName)
            {
                if (! wroteUserDefinedHeading)
                {
                    h << "    // User-defined methods" << Endl;
                    wroteUserDefinedHeading = true;
                }
                h << "    " << functions[i]->decl << Endl;
            }
        }
        h << Endl;

        // Write field IDs
        WriteHeaderFieldIDs(h);

        // If there are any AttributeGroupVectors, we'll need this method.
        if (HaveAGVectors())
        {
            h << "protected:" << Endl;
            h << "    AttributeGroup *CreateSubAttributeGroup(int index);" << Endl;
        }

        totalWidth = CalculateTotalWidth(false);

        // Write out all the public attributes.
        if(hasPublicFields)
        {
            h << "public:" << Endl;
            for (size_t i=0; i<fields.size(); i++)
            {
                if(fields[i]->accessType != Field::AccessPublic)
                    continue;
                fields[i]->WriteHeaderAttribute(h, totalWidth);
            }
        }
        // Write out all the protected attributes
        if(hasProtectedFields)
        {
            h << "protected:" << Endl;
            for (size_t i=0; i<fields.size(); i++)
            {
                if(fields[i]->accessType != Field::AccessProtected)
                    continue;
                fields[i]->WriteHeaderAttribute(h, totalWidth);
            }
        }
        // Write out all the private attributes
        if(hasPrivateFields)
        {
            h << "private:" << Endl;
            for (size_t i=0; i<fields.size(); i++)
            {
                if(fields[i]->accessType != Field::AccessPrivate)
                    continue;
                fields[i]->WriteHeaderAttribute(h, totalWidth);
            }
        }
        h << Endl;
        if(!hasPrivateFields)
            h << "private:" << Endl;
        h << "    // Static class format string for type map." << Endl;
        h << "    static const char *TypeMapFormatString;" << Endl;
        h << "    static const private_tmfs_t TmfsStruct;" << Endl;
        h << "};" << Endl;
        QString formatString;
        for (size_t i=0; i<fields.size(); i++)
            formatString += fields[i]->GetAttributeGroupID();
        if (custombase)
            h << "#define " << name.toUpper() << "_TMFS (" << baseClass.toUpper()
              << "_TMFS \"" << formatString << "\")" << Endl;
        else
            h << "#define " << name.toUpper() << "_TMFS \""
              << formatString << "\"" << Endl;
        h << Endl;

        bool wroteUserDefinedFunctionsHeading = false;
        for (size_t i=0; i<functions.size(); i++)
        {
            if (functions[i]->user && 
                !functions[i]->member &&
                 functions[i]->target == generatorName)
            {
                if (! wroteUserDefinedFunctionsHeading)
                {
                    h << "// User-defined functions" << Endl;
                    wroteUserDefinedFunctionsHeading = true;
                }
                h << functions[i]->decl << Endl;
            }
        }
        if(HasCode(name+".h", 1))
        {
            PrintCode(h, name+".h", 1);
            h << Endl;
        }
        h << "#endif" << Endl;
    }

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    void WriteSource(QTextStream &c)
    {
        c << copyright_str << Endl;
        c << "#include <" << name << ".h>" << Endl;
        WriteSourceIncludes(c);

        if(HasCode(name+".C", 0))
        {
            PrintCode(c, name+".C", 0);
            c << Endl;
        }

        if (!constants.empty())
        {
            for (size_t i=0; i<constants.size(); i++)
            {
                if (constants[i]->target == generatorName &&
                    !constants[i]->def.simplified().isEmpty())
                {
                    c << constants[i]->def;
                    c << Endl;
                }
            }
        }

        // Write out enum conversions.
        WriteSourceEnumConversions(c);

        // Write the more complex methods.
        WriteSourceInitFunc(c);
        WriteSourceCopyFunc(c);
        WriteSourceConstructors(c);
        WriteSourceCopyConstructors(c);
        WriteSourceDestructor(c);
        WriteSourceAssignmentOperator(c);
        WriteSourceComparisonOperators(c);
        WriteSourceTypeName(c);
        WriteSourceCopyAttributes(c);
        WriteSourceCreateCompatible(c);
        WriteSourceNewInstance(c);
        WriteSourceSelectAll(c);
        WriteSourceSubAttributeGroup(c);

        if (persistent)
        {
            c << "///////////////////////////////////////////////////////////////////////////////" << Endl;
            c << "// Persistence methods" << Endl;
            c << "///////////////////////////////////////////////////////////////////////////////" << Endl;
            c << Endl;
            WriteSourceCreateNode(c);
            WriteSourceSetFromNode(c);
        }

        // Write out all the set methods
        c << "///////////////////////////////////////////////////////////////////////////////" << Endl;
        c << "// Set property methods" << Endl;
        c << "///////////////////////////////////////////////////////////////////////////////" << Endl << Endl;
        for (size_t i=0; i<fields.size(); i++)
        {
            if(fields[i]->accessType == Field::AccessPublic)
                continue;

            if (!HasFunction("Set"+fields[i]->Name))
            {
                fields[i]->WriteSourceSetFunction(c, name);
            }
            else
            {
                PrintFunction(c, "Set"+fields[i]->Name);
                c << Endl;
            }
        }

        // Write out all the get methods
        c << "///////////////////////////////////////////////////////////////////////////////" << Endl;
        c << "// Get property methods" << Endl;
        c << "///////////////////////////////////////////////////////////////////////////////" << Endl << Endl;
        for (size_t i=0; i<fields.size(); i++)
        {
            if(fields[i]->accessType == Field::AccessPublic)
                continue;
            fields[i]->WriteSourceGetFunction(c, name, true);
            if (fields[i]->CanHaveConst())
                fields[i]->WriteSourceGetFunction(c, name, false);
        }

        if (SelectFunctionsNeeded())
        {
            // Write out all the select methods
            c << "///////////////////////////////////////////////////////////////////////////////" << Endl;
            c << "// Select property methods" << Endl;
            c << "///////////////////////////////////////////////////////////////////////////////" << Endl << Endl;
            for (size_t i=0; i<fields.size(); i++)
            {
                if(fields[i]->accessType == Field::AccessPublic)
                    continue;
                fields[i]->WriteSourceSelectFunction(c, name);
            }
        }

        if (HaveAGVectors())
        {
            c << "///////////////////////////////////////////////////////////////////////////////" << Endl;
            c << "// AttributeGroupVector convenience methods." << Endl;
            c << "///////////////////////////////////////////////////////////////////////////////" << Endl << Endl;
            for (size_t i=0; i<fields.size(); i++)
            {
                if(fields[i]->accessType != Field::AccessPrivate)
                    continue;
                fields[i]->WriteSourceAGVectorFunctions(c, name, purpose);
            }
        }

        if (HaveSoloAGVector())
        {
            for (size_t i=0; i<fields.size(); i++)
                fields[i]->WriteSourceSoloAGVectorFunctions(c, name, purpose);
        }

        // Write out all the keyframe methods
        if(keyframe)
        {
            c << "///////////////////////////////////////////////////////////////////////////////" << Endl;
            c << "// Keyframing methods" << Endl;
            c << "///////////////////////////////////////////////////////////////////////////////" << Endl << Endl;
            WriteSourceKeyframeFunctions(c);
        }

        c << "///////////////////////////////////////////////////////////////////////////////" << Endl;
        c << "// User-defined methods." << Endl;
        c << "///////////////////////////////////////////////////////////////////////////////" << Endl << Endl;
        for (size_t i=0; i<functions.size(); i++)
        {
            if (functions[i]->user &&
                functions[i]->target == generatorName)
            {
                c << functions[i]->def;
                c << Endl;
            }
        }

        for (size_t i=0; i<functions.size(); i++) 
        {
            if (!functions[i]->user &&
                functions[i]->target == generatorName)
            {
                if (!functions[i]->usedThisFunction)
                {
                    cErr << "\n\n!!! WARNING !!!\n\n";
                    cErr << "You declared the function \"" << functions[i]->name
                         << "\" as replacing a builtin.  But the xml2atts\n"
                         << "program could not find a builtin to replace it "
                         << "with.  It is being ignored.\n\n"
                         << "You might want to declare it as a \"New Function"
                         << "\" instead.\n\n" << Endl;
                }
            }
        }

        if(HasCode(name+".C", 1))
        {
            PrintCode(c, name+".C", 1);
            c << Endl;
        }
    }
private:
    void WriteHeaderSystemIncludes(QTextStream &h)
    {
        UniqueStringList sysincludes;
        for (size_t i=0; i<fields.size(); i++)
            fields[i]->AddSystemIncludes(sysincludes);
        sysincludes.Write(h);
    }
    void WriteHeaderAttributeIncludes(QTextStream &h)
    {
        UniqueStringList attsincludes;
        for (size_t i=0; i<fields.size(); i++)
            fields[i]->AddAttributeIncludes(attsincludes);
        attsincludes.Write(h);
    }
    void WriteHeaderForwardDeclarations(QTextStream &h)
    {
        for (size_t i=0; i<fields.size(); i++)
            fields[i]->WriteHeaderForwardDeclarations(h);
    }
    void WriteHeaderSelectFunctions(QTextStream &h)
    {
        for (size_t i=0; i<fields.size(); i++)
        {
            if(fields[i]->accessType == Field::AccessPrivate)
            fields[i]->WriteHeaderSelectFunction(h);
        }
    }
    void WriteHeaderFieldIDs(QTextStream &h)
    {
        if(fields.size() > 0)
        {
            h << "    // IDs that can be used to identify fields in case statements" << Endl;
            h << "    enum {" << Endl;
        }
        for (size_t i=0; i<fields.size(); i++)
        {
            h << "        " << fields[i]->FieldID();
            if(i == 0)
            {
                if(custombase)
                    h << " = " << baseClass << "::ID__LAST";
                else
                    h << " = 0";
            }
            h << "," << Endl;
        }
        if(fields.size() > 0)
        {
            h << "        ID__LAST" << Endl;
            h << "    };" << Endl;
            h << Endl;
        }
    }
    void WriteHeaderEnumConversions(QTextStream &h)
    {
        // Write the enums functions
        if(EnumType::enums.size() > 0)
        {
            h << "    // Enum conversion functions" << Endl;
            for (size_t i = 0; i < EnumType::enums.size(); ++i)
            {
                h << "    static std::string "<<EnumType::enums[i]->type<<"_ToString("<<EnumType::enums[i]->type<<");" << Endl;
                h << "    static bool "<<EnumType::enums[i]->type<<"_FromString(const std::string &, " << EnumType::enums[i]->type<<" &);" << Endl;
                h << "protected:" << Endl;
                h << "    static std::string "<<EnumType::enums[i]->type<<"_ToString(int);" << Endl;
                h << "public:" << Endl;
            }
        }
    }

    int CalculateTotalWidth(bool considerConst)
    {
        int retval = 0;

        // Iterate through the list of attibutes and find the one with
        // the longest name.
        for (size_t i=0; i<fields.size(); i++)
        {
            if ( fields[i]->type=="attVector" && considerConst)
                continue;

            int len = fields[i]->GetCPPName().length();

            if (considerConst)
            {
                if (fields[i]->CanHaveConst())
                    len += 6; // the length of const and a space.
            }

            if (len > retval)
                retval = len;
        }
 
        return retval;
    }
    bool SelectFunctionsNeeded()
    {
        for (size_t i=0; i<fields.size(); i++)
        {
            if(fields[i]->accessType != Field::AccessPrivate)
                continue;
            if (fields[i]->CanHaveConst())
                return true;
        }
        return false;
    }
    bool HaveAGVectors()
    {
        for (size_t i=0; i<fields.size(); i++)
        {
            if (fields[i]->type=="attVector")
                return true;
        }
        return false;
    }
    bool HaveSoloAGVector()
    {
        int count = 0;
        for (size_t i=0; i<fields.size(); i++)
        {
            if (fields[i]->type=="attVector")
                count++;
        }
        return (count == 1);
    }
    bool HaveArrays()
    {
        for (size_t i=0; i<fields.size(); i++)
        {
            if (fields[i]->isArray)
                return true;
        }
        return false;
    }
    bool HaveArraysThatNeedIndexVar()
    {
        for (size_t i=0; i<fields.size(); i++)
        {
            if (fields[i]->isArray && fields[i]->length >= 4)
                return true;
        }
        return false;
    }
    bool HaveVectors()
    {
        for (size_t i=0; i<fields.size(); i++)
        {
            if (fields[i]->isVector)
                return true;
        }
        return false;
    }

    void WriteSourceIncludes(QTextStream &c)
    {
        c << "#include <DataNode.h>" << Endl;

        // write user source includes
        for (size_t i=0; i<includes.size(); i++)
        {
            if (includes[i]->destination=="source" &&
                includes[i]->target == generatorName)
            {
                if (includes[i]->quoted == true)
                    c << "#include \"" << includes[i]->include << "\"" << Endl;
                else
                    c << "#include <"  << includes[i]->include << ">"  << Endl;
            }
        }

        for (size_t i=0; i<fields.size(); i++)
        {
            fields[i]->WriteSourceIncludes(c);
        }

        c << Endl;
    }

    void WriteSourceInitFunc(QTextStream &c)
    {
        QString purposeString("Init utility for the ");
        purposeString += (name + " class.");
        WriteMethodComment(c, name, name, purposeString);

        c << "void " << name << "::Init()" << Endl;
        c << "{" << Endl;
        for (size_t i=0; i<fields.size(); i++)
        {
            if (!fields[i]->PrintInit(c, generatorName) &&
                fields[i]->valueSet)
            {
                fields[i]->WriteSourceSetDefault(c);
            }
        }
        c << Endl;
        c << "    " << name << "::SelectAll();" << Endl;
        c << "}" << Endl << Endl;
    }

    void WriteSourceInitializerList(QTextStream &c)
    {
        // Count the number of fields that require an initializer
        size_t nInitializers = 0;
        for (size_t i=0; i<fields.size(); ++i)
        {
            if(fields[i]->RequiresSourceInitializer())
                ++nInitializers;
        }
        // Write the initializers.
        if(nInitializers > 0)
        {
            bool Endline;
            c << "," << Endl << "    ";
            for(size_t i = 0, j = 0; i < fields.size(); ++i)
            {
                if(fields[i]->RequiresSourceInitializer())
                {
                    Endline = false;
                    fields[i]->WriteSourceInitializer(c);
                    if(j < nInitializers - 1)
                        c << ", ";
                    if(((j + 1) % 2) == 0)
                    {
                        c << Endl;
                        Endline = true;
                    }
                    if(Endline && (j < nInitializers - 1))
                        c << "    ";
                    ++j;
                }
            }
            if(!Endline)
                c << Endl;
        }
        else
            c << Endl;
    }

    void WriteSourceConstructors(QTextStream &c)
    {
        // Write the typemap format string.
        c << "// Type map format string" << Endl;
        c << "const char *" << name << "::TypeMapFormatString = "
          << name.toUpper() << "_TMFS;" << Endl;
        c << "const AttributeGroup::private_tmfs_t " << name << "::TmfsStruct = {"
          << name.toUpper() << "_TMFS};" << Endl;
        c << Endl << Endl;

        // Write the method comment.
        QString purposeString("Default constructor for the ");
        purposeString += (name + " class.");
        WriteMethodComment(c, name, name, purposeString);
        c << name << "::" << name << "() : \n    " << baseClass << "(" << name
          << (custombase?"::TmfsStruct)":"::TypeMapFormatString)");
        WriteSourceInitializerList(c);
        c << "{" << Endl;
        c << "    " << name << "::Init();" << Endl;
        c << "}" << Endl << Endl;

        // Write the method comment.
        purposeString = "Constructor for the derived classes of ";
        purposeString += (name + " class.");
        WriteMethodComment(c, name, name, purposeString);

        c << name << "::" << name << "(private_tmfs_t tmfs) : \n    "
          << baseClass << (custombase?"(tmfs)":"(tmfs.tmfs)");
        WriteSourceInitializerList(c);
        c << "{" << Endl;
        c << "    " << name << "::Init();" << Endl;
        c << "}" << Endl << Endl;
    }

    void WriteSourceCopyCode(QTextStream &c)
    {
        bool skipLine = false;
        if (HaveAGVectors())
        {
            c << "    AttributeGroupVector::const_iterator pos;" << Endl;
            skipLine = true;
        }
        if (HaveArraysThatNeedIndexVar())
        {
            skipLine = true;
        }
        if(skipLine)
            c << Endl;
        for (size_t i=0; i<fields.size(); i++)
        {
            fields[i]->WriteSourceCopyCode(c);
        }
        c << Endl << "    " << name << "::SelectAll();" << Endl;
    }

    void WriteSourceCopyFunc(QTextStream &c)
    {
        QString purposeString("Copy utility for the ");
        purposeString += (name + " class.");
        WriteMethodComment(c, name, name, purposeString);
        c << "void " << name << "::Copy(const " << name << " &obj)" << Endl;
        c << "{" << Endl;
        WriteSourceCopyCode(c);
        c << "}" << Endl << Endl;
    }

    void WriteSourceCopyConstructors(QTextStream &c)
    {
        QString purposeString("Copy constructor for the ");
        purposeString += (name + " class.");
        WriteMethodComment(c, name, name, purposeString);

        c << name << "::" << name << "(const "
          << name << " &obj) : \n    " << baseClass << (custombase?"(obj,":"(") << name
          << (custombase?"::TmfsStruct)":"::TypeMapFormatString)") << Endl;

        c << "{" << Endl;
        c << "    " << name << "::Copy(obj);" << Endl;
        c << "}" << Endl << Endl;

        purposeString = "Copy constructor for derived classes of the ";
        purposeString += (name + " class.");
        WriteMethodComment(c, name, name, purposeString);

        c << name << "::" << name << "(const "
          << name << " &obj, private_tmfs_t tmfs) : \n    "
          << baseClass << (custombase?"(obj,tmfs)":"(tmfs.tmfs)") << Endl;

        c << "{" << Endl;
        c << "    " << name << "::Copy(obj);" << Endl;
        c << "}" << Endl << Endl;

    }
    void WriteSourceDestructor(QTextStream &c)
    {
        // Write the method comment.
        QString purposeString("Destructor for the ");
        purposeString += (name + " class.");
        QString methodName("~");
        methodName += name;
        WriteMethodComment(c, name, methodName, purposeString);

        // See if there are any AG vectors.
        c << name << "::~" << name << "()" << Endl;
        c << "{" << Endl;
        if (/*!HaveAGArrays() && */!HaveAGVectors())
        {
            if(HasCode(methodName, 0))
            {
                PrintCode(c, methodName, 0);
                c << Endl;
            }
            else
            {
                c << "    // nothing here" << Endl;
            }
        }
        else 
        {
            if (HaveAGVectors())
                c << "    AttributeGroupVector::iterator pos;" << Endl;
            c << Endl;

            for (size_t i=0; i<fields.size(); i++)
                fields[i]->WriteSourceDestructor(c);
        }
        if(HasCode(methodName, 1))
        {
            PrintCode(c, methodName, 1);
            c << Endl;
        }
        c << "}" << Endl << Endl;
    }
    void WriteSourceAssignmentOperator(QTextStream &c)
    {
        if (HasFunction("operator ="))
        {
            PrintFunction(c, "operator =");
            c << Endl;
            return;
        }
        // Write the method comment.
        QString purposeString("Assignment operator for the ");
        purposeString += (name + " class.");
        QString methodName("operator = ");
        WriteMethodComment(c, name, methodName, purposeString);

        c << name << "& " << Endl;
        c << name << "::operator = (const "
          << name << " &obj)" << Endl;
        c << "{" << Endl;
        c << "    if (this == &obj) return *this;" << Endl;
        c << Endl;
        if (custombase)
        {
            c << "    // call the base class' assignment operator first" << Endl;
            c << "    " << baseClass << "::operator=(obj);" << Endl;
            c << Endl;
        }
        WriteSourceCopyCode(c);
        c << Endl;
        c << "    return *this;" << Endl;
        c << "}" << Endl << Endl;
    }
    void WriteSourceTypeName(QTextStream &c)
    {
        // Write the method comment.
        QString purposeString("Type name method for the ");
        purposeString += (name + " class.");
        QString methodName("TypeName");
        WriteMethodComment(c, name, methodName, purposeString);

        c << "const std::string" << Endl;
        c << name << "::TypeName() const" << Endl;
        c << "{" << Endl;
        c << "    return \"" << name << "\";" << Endl;
        c << "}" << Endl << Endl;
    }
    void WriteSourceCopyAttributes(QTextStream &c)
    {
        if (HasFunction("CopyAttributes"))
        {
            PrintFunction(c, "CopyAttributes");
            c << Endl;
            return;
        }

        // Write the method comment.
        QString purposeString("CopyAttributes method for the ");
        purposeString += (name + " class.");
        QString methodName("CopyAttributes");
        WriteMethodComment(c, name, methodName, purposeString);

        c << "bool" << Endl;
        c << name << "::CopyAttributes(const AttributeGroup *atts)" << Endl;
        c << "{" << Endl;
        c << "    if(TypeName() != atts->TypeName())" << Endl;
        c << "        return false;" << Endl;
        c << Endl;
        c << "    // Call assignment operator." << Endl;
        c << "    const " << name << " *tmp = (const " << name << " *)atts;" << Endl;
        c << "    *this = *tmp;" << Endl;
        c << Endl;
        c << "    return true;" << Endl;
        c << "}" << Endl << Endl;
    }
    void WriteSourceNewInstance(QTextStream &c)
    {
        if (HasFunction("NewInstance"))
        {
            PrintFunction(c, "NewInstance");
            c << Endl;
            return;
        }

        // Write the method comment.
        QString purposeString("NewInstance method for the ");
        purposeString += (name + " class.");
        QString methodName("NewInstance");
        WriteMethodComment(c, name, methodName, purposeString);

        c << "AttributeSubject *" << Endl;
        c << name << "::NewInstance(bool copy) const" << Endl;
        c << "{" << Endl;
        c << "    AttributeSubject *retval = 0;" << Endl;
        c << "    if(copy)" << Endl;
        c << "        retval = new " << name << "(*this);" << Endl;
        c << "    else" << Endl;
        c << "        retval = new " << name << ";" << Endl;
        c << Endl;
        c << "    return retval;" << Endl;
        c << "}" << Endl << Endl;
    }
    void WriteSourceCreateCompatible(QTextStream &c)
    {
        if (HasFunction("CreateCompatible"))
        {
            PrintFunction(c, "CreateCompatible");
            c << Endl;
            return;
        }

        // Write the method comment.
        QString purposeString("CreateCompatible method for the ");
        purposeString += (name + " class.");
        QString methodName("CreateCompatible");
        WriteMethodComment(c, name, methodName, purposeString);

        c << "AttributeSubject *" << Endl;
        c << name << "::CreateCompatible(const std::string &tname) const" << Endl;
        c << "{" << Endl;
        c << "    AttributeSubject *retval = 0;" << Endl;
        c << "    if(TypeName() == tname)" << Endl;
        c << "        retval = new " << name << "(*this);" << Endl;
        c << "    // Other cases could go here too. " << Endl;
        c << Endl;
        c << "    return retval;" << Endl;
        c << "}" << Endl << Endl;
    }
    void WriteSourceComparisonOperators(QTextStream &c)
    {
        QString purposeString("Comparison operator == for the ");
        purposeString += (name + " class.");
        QString methodName("operator == ");
        WriteMethodComment(c, name, methodName, purposeString);

        c << "bool" << Endl;
        c << name << "::operator == (const "
          << name << " &obj) const" << Endl;
        c << "{" << Endl;

        // Create bool values to evaluate the arrays.
        QString prevValue("true");
        for (size_t i=0; i<fields.size(); i++)
        {
            if (!fields[i]->ignoreEquality)
                fields[i]->WriteSourceComparisonPrecalc(c, "    ");
        }

        c << "    // Create the return value" << Endl;
        c << "    return (";

        // Create a big boolean return statement.
        if (fields.size() == 0)
        {
            c << "true";
        }
        else
        {
            for (size_t i=0; i<fields.size(); i++)
            {
                if (i > 0)
                    c << "            ";

                if (!fields[i]->ignoreEquality)
                    fields[i]->WriteSourceComparison(c);
                else
                    c << "true /* can ignore " << fields[i]->name << " */";

                if (i < fields.size() - 1)
                    c << " &&" << Endl;
            }
            if (custombase)
            {
                c << " &&" << Endl;
                c << "            ";
                c << baseClass << "::operator==(obj)";
            }
        }
        c << ");" << Endl;

        c << "}" << Endl << Endl;

        // Write the != operator
        purposeString = "Comparison operator != for the ";
        purposeString += (name + " class.");
        methodName = "operator != ";
        WriteMethodComment(c, name, methodName, purposeString);

        c << "bool" << Endl;
        c << name << "::operator != (const "
          << name << " &obj) const" << Endl;
        c << "{" << Endl;
        c << "    return !(this->operator == (obj));" << Endl;
        c << "}" << Endl << Endl;
    }
    int MaxFieldLength() const
    {
        int maxlen = 0;
        for (size_t i=0; i<fields.size(); i++)
        {
            int len = fields[i]->FieldID().length();
            maxlen = (len > maxlen) ? len : maxlen;
        }
        return maxlen;
    }
    QString PadStringWithSpaces(const QString &s, int len) const
    {
        QString ret(s);
        while((int)(ret.length()) < len)
            ret += QString(" ");
        return ret;
    }
    void WriteSourceSelectAll(QTextStream &c)
    {
        // Write the method comment.
        QString purposeString("Selects all attributes.");
        QString methodName("SelectAll");
        WriteMethodComment(c, name, methodName, purposeString);

        c << "void" << Endl;
        c << name << "::SelectAll()" << Endl;
        c << "{" << Endl;
        if (custombase)
        {
            c << "    // call the base class' SelectAll() first" << Endl;
            c << "    " << baseClass << "::SelectAll();" << Endl;
        }
        int maxlen = MaxFieldLength() + 2;
        for (size_t i=0; i<fields.size(); i++)
        {
            QString fieldID(PadStringWithSpaces(fields[i]->FieldID() + QString(", "), maxlen));
            if (fields[i]->isArray)
            {
                c << "    Select(" << fieldID << "(void *)"
                  << fields[i]->name << ", " << fields[i]->length  << ");";
            }
            else
            {
                c << "    Select(" << fieldID << "(void *)&"
                  << fields[i]->name << ");";
            }
            c << Endl;
        }
        c << "}" << Endl << Endl;
    }
    void WriteSourceSubAttributeGroup(QTextStream &c)
    {
        // See if there are any AG vectors.
        int AG_dynamic_count = 0;
        for (size_t i=0; i<fields.size(); i++)
        {
            if (fields[i]->type == "attVector"/* || fields[i]->type == "attArray"*/)
                ++AG_dynamic_count;
        }
        if(AG_dynamic_count < 1)
            return;

        WriteMethodComment(c, name, "CreateSubAttributeGroup", purpose);
        c << "AttributeGroup *" << Endl;
        c << name << "::CreateSubAttributeGroup(int";

        if(AG_dynamic_count == 1)
        {
            c << ")" << Endl;
            c << "{" << Endl;

            for (size_t i=0; i<fields.size(); i++)
            {
                fields[i]->WriteSourceSubAttributeGroupSingle(c);
            }
        }
        else
        {
            c << " attr_id)" << Endl;
            c << "{" << Endl;

            c << "    AttributeGroup *retval = 0;" << Endl;
            c << "    switch(attr_id)" << Endl;
            c << "    {" << Endl;
            for (size_t i=0; i<fields.size(); i++)
            {
                fields[i]->WriteSourceSubAttributeGroup(c);
            }
            c << "    }" << Endl;
            c << Endl << "    return retval;" << Endl;
        }
        c << "}" << Endl << Endl;
    }
    void WriteSourceCreateNode(QTextStream &c)
    {
        if (HasFunction("CreateNode"))
        {
            PrintFunction(c, "CreateNode");
            return;
        }
        WriteMethodComment(c, name, "CreateNode",
                           "This method creates a DataNode representation of the object so it can be saved to a config file.");

        c << "bool" << Endl;
        c << name << "::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)" << Endl;
        c << "{" << Endl;
        c << "    if(parentNode == 0)" << Endl;
        c << "        return false;" << Endl << Endl;
        c << "    " << name << " defaultObject;" << Endl;
        c << "    bool addToParent = false;" << Endl;
        c << "    // Create a node for " << name << "." << Endl;
        c << "    DataNode *node = new DataNode(\"" << name << "\");" << Endl << Endl;

        // Write out the DataNode creation for all attributes.
        for (size_t i=0; i<fields.size(); i++)
        {
            if(fields[i]->accessType == Field::AccessPublic)
            {
                c << "    // " << fields[i]->name << " is public and should not be saved." << Endl;
                continue;
            }

            QString forceAdd("false"); 
            if(fields[i]->type != "color")
            {
                c << "    if(completeSave || !FieldsEqual(" << fields[i]->FieldID() << ", &defaultObject))" << Endl;
                c << "    {" << Endl;
            }
            else
                forceAdd = "true";

            if (fields[i]->GetAttributeGroupID() == "a")
            {
                QString nodeName = fields[i]->name + "Node";
                c << "        DataNode *" << nodeName << " = new DataNode(\""
                  << fields[i]->name << "\");" << Endl;
                c << "        if(" << fields[i]->name << ".CreateNode("
                  << nodeName << ", completeSave, " << forceAdd << "))" << Endl;
                c << "        {" << Endl;
                c << "            addToParent = true;" << Endl;
                c << "            node->AddNode(" << nodeName << ");" << Endl;
                c << "        }" << Endl;
                c << "        else" << Endl;
                c << "            delete " << nodeName << ";" << Endl;
            }
            else if(fields[i]->type == "attArray")
            {
                c << "        addToParent = true;" << Endl;
                c << "        for(int i = 0; i < " << fields[i]->length << "; ++i)" << Endl;
                c << "            " << fields[i]->name << "[i]->CreateNode(node, completeSave. true);" << Endl;
            }
            else if(fields[i]->type == "attVector")
            {
                c << "        addToParent = true;" << Endl;
                c << "        for(size_t i = 0; i < " << fields[i]->name << ".size(); ++i)" << Endl;
                c << "            " << fields[i]->name << "[i]->CreateNode(node, completeSave, true);" << Endl;
            }
            else if (fields[i]->isArray)
            {
                c << "        addToParent = true;" << Endl;
                c << "        node->AddNode(new DataNode(\"" << fields[i]->name
                  << "\", " << fields[i]->name << ", " << fields[i]->length << "));" << Endl;
            }
            else if (fields[i]->type == "enum")
            {
                c << "        addToParent = true;" << Endl;
                c << "        node->AddNode(new DataNode(\"" << fields[i]->name
                  << "\", " << fields[i]->GetCPPName(true) << "_ToString("
                  << fields[i]->name << ")));" << Endl;
            }
            else
            {
                c << "        addToParent = true;" << Endl;
                c << "        node->AddNode(new DataNode(\"" << fields[i]->name
                  << "\", " << fields[i]->name << "));" << Endl;
            }

            if(fields[i]->type != "color")
                c << "    }" << Endl << Endl;
        }

        c << Endl;
        c << "    // Add the node to the parent node." << Endl;
        c << "    if(addToParent || forceAdd)" << Endl;
        c << "        parentNode->AddNode(node);" << Endl;
        c << "    else" << Endl;
        c << "        delete node;" << Endl << Endl;
        c << "    return (addToParent || forceAdd);" << Endl;
        c << "}" << Endl;
        c << Endl;
    }
    void WriteSourceSetFromNode(QTextStream &c)
    {
        QString mName("SetFromNode");

        if (HasFunction(mName))
        {
            PrintFunction(c, mName);
            return;
        }
        WriteMethodComment(c, name, mName,
            "This method sets attributes in this object from values in a DataNode representation of the object.");

        c << "void" << Endl;
        c << name << "::SetFromNode(DataNode *parentNode)" << Endl;
        c << "{" << Endl;
        c << "    if(parentNode == 0)" << Endl;
        c << "        return;" << Endl << Endl;
        c << "    DataNode *searchNode = parentNode->GetNode(\"" << name << "\");" << Endl;
        c << "    if(searchNode == 0)" << Endl;
        c << "        return;" << Endl;
        c << Endl;
        c << "    DataNode *node;" << Endl;
        if (HaveAGVectors())
            c << "    DataNode **children;" << Endl;
        
        if(HasCode(mName, 0))
        {
            PrintCode(c, mName, 0);
            c << Endl;
        }

        for (size_t i=0; i<fields.size(); i++)
        {
            if(fields[i]->accessType == Field::AccessPublic)
            {
                c << "    // " << fields[i]->name << " is public and was not saved." << Endl;
                continue;
            }

            fields[i]->WriteSourceSetFromNode(c);
        } // end for

        if(HasCode(mName, 1))
        {
            PrintCode(c, mName, 1);
            c << Endl;
        }

        c << "}" << Endl << Endl;
    }
    void WriteSourceKeyframeFunctions(QTextStream &c)
    {
        WriteMethodComment(c, name, "GetFieldName",
            "This method returns the name of a field given its index.");

        int maxlen = MaxFieldLength() + 1;

        c << "std::string" << Endl;
        c << name << "::GetFieldName(int index) const" << Endl;
        c << "{" << Endl;
        c << "    switch (index)" << Endl;
        c << "    {" << Endl;
        for (size_t i=0; i<fields.size(); i++)
        {
            QString fieldID(PadStringWithSpaces(fields[i]->FieldID() + QString(":"), maxlen));
            c << "    case "<<fieldID<<" return \""<<fields[i]->name<<"\";" << Endl;
        }
        c << "    default:  return \"invalid index\";" << Endl;
        c << "    }" << Endl;
        c << "}" << Endl;
        c << Endl;

        WriteMethodComment(c, name, "GetFieldType",
            "This method returns the type of a field given its index.");

        c << "AttributeGroup::FieldType" << Endl;
        c << name << "::GetFieldType(int index) const" << Endl;
        c << "{" << Endl;
        c << "    switch (index)" << Endl;
        c << "    {" << Endl;
        for (size_t i=0; i<fields.size(); i++)
        {
            QString fieldID(PadStringWithSpaces(fields[i]->FieldID() + QString(":"), maxlen));
            c << "    case "<<fieldID<<" return FieldType_"<<fields[i]->GetFieldType()<<";" << Endl;
        }
        c << "    default:  return FieldType_unknown;" << Endl;
        c << "    }" << Endl;
        c << "}" << Endl;
        c << Endl;

        WriteMethodComment(c, name, "GetFieldTypeName",
            "This method returns the name of a field type given its index.");

        c << "std::string" << Endl;
        c << name << "::GetFieldTypeName(int index) const" << Endl;
        c << "{" << Endl;
        c << "    switch (index)" << Endl;
        c << "    {" << Endl;
        for (size_t i=0; i<fields.size(); i++)
        {
            QString fieldID(PadStringWithSpaces(fields[i]->FieldID() + QString(":"), maxlen));
            c << "    case "<<fieldID<<" return \""<<fields[i]->type<<"\";" << Endl;
        }
        c << "    default:  return \"invalid index\";" << Endl;
        c << "    }" << Endl;
        c << "}" << Endl;
        c << Endl;

        WriteMethodComment(c, name, "FieldsEqual",
            "This method compares two fields and return true if they are equal.");

        c << "bool" << Endl;
        c << name << "::FieldsEqual(int index_, const AttributeGroup *rhs) const" << Endl;
        c << "{" << Endl;

        c << "    const "<<name<<" &obj = *((const "<<name<<"*)rhs);" << Endl;

        // Create bool values to evaluate the arrays.
        c << "    bool retval = false;" << Endl;
        c << "    switch (index_)" << Endl;
        c << "    {" << Endl;

        // Create a big boolean return statement.
        for (size_t i=0; i<fields.size(); i++)
        {
            c << "    case "<<fields[i]->FieldID()<<":" << Endl;
            c << "        {  // new scope" << Endl;
            fields[i]->WriteSourceComparisonPrecalc(c, "        ");
            c << "        retval = ";
            fields[i]->WriteSourceComparison(c);
            c << ";" << Endl << "        }" << Endl;
            c << "        break;" << Endl;
        }
        c << "    default: retval = false;" << Endl;
        c << "    }" << Endl << Endl;
        c << "    return retval;" << Endl;
        c << "}" << Endl << Endl;
    }

    void WriteSourceEnumConversions(QTextStream &c)
    {
        for(size_t i = 0; i < EnumType::enums.size(); ++i)
        {
            c << "//" << Endl;
            c << "// Enum conversion methods for "<<name << "::" << EnumType::enums[i]->type << Endl;
            c << "//" << Endl;
            c << Endl;
            c << "static const char *" << EnumType::enums[i]->type << "_strings[] = {" << Endl;
            for(size_t j = 0; j < EnumType::enums[i]->values.size(); ++j)
            {
                c << "\"" << EnumType::enums[i]->values[j] << "\"";
                if(j < EnumType::enums[i]->values.size() - 1)
                    c << ", ";
                if(((j+1) % 3) == 0)
                    c << Endl;
            }
            c << "};" << Endl << Endl;

            c << "std::string"<< Endl;
            c << name << "::" << EnumType::enums[i]->type<<"_ToString("<<name<<"::"<<EnumType::enums[i]->type<<" t)" << Endl;
            c << "{" << Endl;
            c << "    int index = int(t);" << Endl;
            c << "    if(index < 0 || index >= " << EnumType::enums[i]->values.size() << ") index = 0;" << Endl;
            c << "    return " << EnumType::enums[i]->type << "_strings[index];" << Endl;
            c << "}" << Endl << Endl;

            c << "std::string"<< Endl;
            c << name << "::" << EnumType::enums[i]->type<<"_ToString(int t)" << Endl;
            c << "{" << Endl;
            c << "    int index = (t < 0 || t >= " << EnumType::enums[i]->values.size() << ") ? 0 : t;" << Endl;
            c << "    return " << EnumType::enums[i]->type << "_strings[index];" << Endl;
            c << "}" << Endl << Endl;

            c << "bool" << Endl;
            c << name << "::"<< EnumType::enums[i]->type<<"_FromString(const std::string &s, "<<name<<"::"<<EnumType::enums[i]->type<<" &val)" << Endl;
            c << "{" << Endl;
            if(EnumType::enums[i]->values.size() > 0)
                c << "    val = " << name << "::" << EnumType::enums[i]->values[0] << ";" << Endl;
            c << "    for(int i = 0; i < "<<EnumType::enums[i]->values.size()<<"; ++i)" << Endl;
            c << "    {" << Endl;
            c << "        if(s == "<< EnumType::enums[i]->type<<"_strings[i])" << Endl;
            c << "        {" << Endl;
            c << "            val = ("<< EnumType::enums[i]->type<<")i;" << Endl;
            c << "            return true;" << Endl;
            c << "        }" << Endl;
            c << "    }" << Endl;
            c << "    return false;" << Endl;
            c << "}" << Endl << Endl;
        }
    }
};

// ----------------------------------------------------------------------------
// Modifications:
//   Brad Whitlock, Thu Feb 28 13:54:04 PST 2008
//   Made it use a base class for uniformity.
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
