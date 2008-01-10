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

#ifndef GENERATE_ATTS_H
#define GENERATE_ATTS_H

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

void
WriteMethodComment(ostream &out, const QString &className,
                   const QString &methodName, const QString &purposeString)
{
    out << "// ****************************************************************************" << endl;
    out << "// Method: " << className << "::" << methodName << endl;
    out << "//" << endl;
    out << "// Purpose: " << endl;
    out << "//   " << purposeString << endl;
    out << "//" << endl;
    out << "// Note:       Autogenerated by xml2atts." << endl;
    out << "//" << endl;
    out << "// Programmer: xml2atts" << endl;
    out << "// Creation:   ";

    // Create the generation time, date.
    out << CurrentTime() << endl;
       
    out << "//" << endl;
    out << "// Modifications:" << endl;
    out << "//   " << endl;
    out << "// ****************************************************************************" << endl;
    out << endl;
}



// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class AttsGeneratorField : public virtual Field
{
  public:
    AttsGeneratorField(const QString &t, const QString &n, const QString &l)
        : Field(t,n,l) { }
    QString      GetCPPNameW(int w, bool subtypename=false, const QString &classname="")
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
    virtual void WriteHeaderForwardDeclarations(ostream &h) { }
    virtual void AddSystemIncludes(UniqueStringList &sl) { }
    virtual void WriteHeaderSelectFunction(ostream &h)
    {
        if (CanHaveConst())
        {
            h << "    void Select" << Name << "();" << endl;
        }
    }
    virtual void WriteHeaderSetFunction(ostream &h)
    {
        h << "    void Set"<<Name<<"(";
        if (CanHaveConst())
            h << "const ";
        h << GetCPPName(true) << " ";
        if (isArray) 
            h << "*";
        else if (CanHaveConst())
            h << "&";
        h << name << "_);" << endl;
    }
    virtual void WriteHeaderGetFunction(ostream &h, int w)
    {
        if (isArray)
        {
            h << "    const " << GetCPPNameW(w) << " *Get" << Name << "() const;" << endl;
            h << "          " << GetCPPNameW(w) << " *Get" << Name << "();" << endl;
        }
        else if (CanHaveConst())
        {
            h << "    const " << GetCPPNameW(w) << " &Get" << Name << "() const;" << endl;
            h << "          " << GetCPPNameW(w) << " &Get" << Name << "();" << endl;
        }
        else
            h << "    " << GetCPPNameW(w,true) << " Get" << Name << "() const;" << endl;
    }
    virtual void WriteHeaderAGVectorProto(ostream &h) { }
    virtual void WriteHeaderSoloAGVectorProto(ostream &h) { }
    virtual void WriteHeaderAttribute(ostream &h, int w)
    {
        h << "    " << GetCPPNameW(w) << " " << name;
        if (isArray)
            h << "[" << length << "]";
        h << ";" << endl;
    }
    // ------------------------------------------------------------------------
    virtual void WriteSourceIncludes(ostream &c)
    {
    }
    virtual void WriteSourceInitializer(ostream &c)
    {
    }
    virtual bool RequiresSourceInitializer() const
    {
        return false;
    }
    virtual void WriteSourceSetFunction(ostream &c, const QString &classname)
    {
        // Write prototype.
        c << "void" << endl;
        c << classname << "::Set" << Name << "(";
        if (CanHaveConst())
            c << "const ";
        c << GetCPPName(true,classname) << " ";
        if (isArray)
            c << "*";
        else if (CanHaveConst())
            c << "&";
        c << name << "_)" << endl;

        // Write function body
        c << "{" << endl;
        
        if (codeFile && codeFile->code.count(QString("Set")+Name) && !codeFile->code[QString("Set")+Name].first.isNull())
            c << codeFile->code[QString("Set")+Name].first << endl;

        if (!isArray)
        {
            c << "    " << name << " = " << name << "_;" << endl;
            c << "    Select(" << FieldID() << ", "
              << "(void *)&" << name << ");" << endl;
        }
        else
        {
            if (length < 5)
            {
                for(int i = 0; i < length; ++i)
                    c << "    " << name << "[" << i << "] = " << name << "_[" << i << "];"<< endl;
            }
            else
            {
                c << "    for(int i = 0; i < " << length << "; ++i)" << endl;
                c << "        " << name << "[i] = " << name << "_[i];"<< endl;
            }
            c << "    Select(" << FieldID() << ", (void *)" << name << ", " << length << ");" << endl;
        }

        if (codeFile && codeFile->code.count(QString("Set")+Name) && !codeFile->code[QString("Set")+Name].second.isNull())
            c << endl << codeFile->code[QString("Set")+Name].second;

        c << "}" << endl;
        c << endl;
    }
    virtual void WriteSourceGetFunction(ostream &c, const QString &classname, bool doConst)
    {
        if (isArray)
        {
            if (doConst)
            {
                c << "const " << GetCPPName() << " *" << endl << classname << "::Get"
                  << Name << "() const" << endl;
            }
            else
            {
                c << GetCPPName() << " *" << endl << classname << "::Get"
                  << Name << "()" << endl;
            }
        }
        else if (CanHaveConst())
        {
            if (doConst)
            {
                c << "const " << GetCPPName() << " &" << endl << classname
                  << "::Get" << Name << "() const" << endl;
            }
            else
            {
                c << GetCPPName() << " &" << endl << classname << "::Get"
                  << Name << "()" << endl;
            }
        }
        else
        {
            c << GetCPPName(true,classname) << endl << classname << "::Get"
              << Name << "() const" << endl;
        }

        // Function body
        c << "{" << endl;
        if (type == "attArray")
        {
            cerr << "EEEEEERRRRRRRRROOOOOOORRRRRRR" << endl;
        }
        else
        {
            if (type == "enum")
                c << "    return " << GetCPPName(true) << "(" << name << ");" << endl;
            else
                c << "    return " << name << ";" << endl;
        }
        c << "}" << endl;
        c << endl;
    }
    virtual void WriteSourceSelectFunction(ostream &c, const QString &classname)
    {
        if (! CanHaveConst())
            return;

        c << "void" << endl << classname << "::Select" << Name << "()" << endl;
        c << "{" << endl;
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
        c << endl << "}" << endl << endl;
    }
    virtual void WriteSourceAGVectorFunctions(ostream &c, const QString &classname, const QString &purpose)
    {
    }
    virtual void WriteSourceSoloAGVectorFunctions(ostream &c, const QString &classname, const QString &purpose)
    {
    }
    virtual void WriteSourceSetDefault(ostream &c) = 0;
    virtual void WriteSourceCopyCode(ostream &c)
    {
        if (isArray)
        {
            if (length < 4)
            {
                for(int i = 0; i < length; ++i)
                {
                    c << "    " << name << "[" << i << "] = "
                      << "obj." << name << "[" << i << "];" << endl;
                }
            }
            else
            {
                c << "    for(i = 0; i < " << length << "; ++i)" << endl;
                c << "        " << name << "[i] = " 
                  <<     "obj." << name << "[i];" << endl;
            }
            c << endl;
        }
        else
        {
            c << "    " << name << " = obj." << name << ";" << endl;
        }
    }
    virtual void WriteSourceDestructor(ostream &c)
    {
    }
    virtual void WriteSourceSetFromNode(ostream &c)
    {
        c << "    if((node = searchNode->GetNode(\"" << name
          << "\")) != 0)" << endl;
        c << "        Set" << Name << "(";
        c << "node->" << DataNodeConversion() << "());" << endl;
    }
    virtual void WriteSourceSubAttributeGroupSingle(ostream &c)
    {
    }
    virtual void WriteSourceSubAttributeGroup(ostream &c)
    {
    }
    virtual void WriteSourceComparisonPrecalc(ostream &c, const QString &indent)
    {
        if (isArray)
        {
            c << indent << "// Compare the " << name << " arrays." << endl;
            c << indent << "bool " << name << "_equal = true;" << endl; 
            c << indent << "for(i = 0; i < " << length << " && "
              << name << "_equal; ++i)" << endl;
            c << indent << "    " << name << "_equal = ("
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
    virtual QString GetFieldType() const { return type; }
};

//
// ------------------------------------ Int -----------------------------------
//
class AttsGeneratorInt : public virtual Int , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorInt(const QString &n, const QString &l)
        : Int(n,l), AttsGeneratorField("int",n,l), Field("int",n,l) { }
    virtual QString GetAttributeGroupID()
    {
        return "i";
    }
    virtual QString DataNodeConversion()
    {
        return "AsInt";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << val << ";" << endl;
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
    virtual bool CanHaveConst() { return true; }
    virtual QString GetAttributeGroupID()
    {
        return "I";
    }
    virtual QString DataNodeConversion()
    {
        return "AsIntArray";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        for (int i=0; i<length; i++)
            c << "    " << name << "["<<i<<"] = " << val[i] << ";" << endl;
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
    virtual bool CanHaveConst() { return true; }
    virtual QString GetAttributeGroupID()
    {
        return "i*";
    }
    virtual QString DataNodeConversion()
    {
        return "AsIntVector";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        for (int i=0; i < val.size(); i++)
            c << "    " << name << ".push_back(" << val[i] << ");" << endl;
    }
};


// 
// ----------------------------------- Bool -----------------------------------
//
class AttsGeneratorBool : public virtual Bool , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorBool(const QString &n, const QString &l)
        : Bool(n,l), AttsGeneratorField("bool",n,l), Field("bool",n,l) { }
    virtual QString GetAttributeGroupID()
    {
        return "b";
    }
    virtual QString DataNodeConversion()
    {
        return "AsBool";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << (val ? "true" : "false") << ";" << endl;
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
    virtual QString GetAttributeGroupID()
    {
        return "f";
    }
    virtual QString DataNodeConversion()
    {
        return "AsFloat";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << val << ";" << endl;
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
    virtual bool CanHaveConst() { return true; }
    virtual QString GetAttributeGroupID()
    {
        return "F";
    }
    virtual QString DataNodeConversion()
    {
        return "AsFloatArray";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        for (int i=0; i<length; i++)
            c << "    " << name << "["<<i<<"] = " << val[i] << ";" << endl;
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
    virtual QString GetAttributeGroupID()
    {
        return "d";
    }
    virtual QString DataNodeConversion()
    {
        return "AsDouble";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << val << ";" << endl;
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
    virtual bool CanHaveConst() { return true; }
    virtual QString GetAttributeGroupID()
    {
        return "D";
    }
    virtual QString DataNodeConversion()
    {
        return "AsDoubleArray";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        for (int i=0; i<length; i++)
            c << "    " << name << "["<<i<<"] = " << val[i] << ";" << endl;
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
    virtual bool CanHaveConst() { return true; }
    virtual QString GetAttributeGroupID()
    {
        return "d*";
    }
    virtual QString DataNodeConversion()
    {
        return "AsDoubleVector";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        for (int i=0; i < val.size(); i++)
            c << "    " << name << ".push_back(" << val[i] << ");" << endl;
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
    virtual QString GetAttributeGroupID()
    {
        return "u";
    }
    virtual QString DataNodeConversion()
    {
        return "AsUnsignedChar";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << int(val) << ";" << endl;
    }
};


//
// -------------------------------- UCharArray --------------------------------
//
class AttsGeneratorUCharArray : public virtual UCharArray , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorUCharArray(const QString &s, const QString &n, const QString &l)
        : UCharArray(s,n,l), AttsGeneratorField("ucharArray",n,l), Field("ucharArray",n,l) { }
    virtual bool CanHaveConst() { return true; }
    virtual QString GetAttributeGroupID()
    {
        return "U";
    }
    virtual QString DataNodeConversion()
    {
        return "AsUnsignedCharArray";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        for (int i=0; i<length; i++)
            c << "    " << name << "["<<i<<"] = " << int(val[i]) << ";" << endl;
    }
};


//
// ------------------------------- unsignedCharVector -------------------------------
//
class AttsGeneratorUCharVector : public virtual UCharVector , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorUCharVector(const QString &n, const QString &l)
        : UCharVector(n,l), AttsGeneratorField("ucharVector",n,l), Field("ucharVector",n,l) { }
    virtual bool CanHaveConst() { return true; }
    virtual QString GetAttributeGroupID()
    {
        return "u*";
    }
    virtual QString DataNodeConversion()
    {
        return "AsUnsignedCharVector";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        for (int i=0; i < val.size(); i++)
            c << "    " << name << ".push_back(" << val[i] << ");" << endl;
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
    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = \"" << val << "\";" << endl;
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
    virtual void WriteSourceSetDefault(ostream &c)
    {
        for (int i=0; i < val.size(); i++)
        c << "    " << name << ".push_back(" << "\"" << val[i].latin1() << "\");" << endl;
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
    virtual void WriteSourceInitializer(ostream &c)
    {
        if(valueSet)
            c << name << "(\""<<val<<"\")";
        else
            c << name << "(\"hot\")";
    }
    virtual void WriteSourceSetDefault(ostream &c)
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
        : Color(n,l), AttsGeneratorField("color",n,l), Field("color",n,l) { }
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
    virtual void WriteSourceInitializer(ostream &c)
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
    virtual void WriteSourceSetDefault(ostream &c)
    {
    }
    virtual void WriteSourceSetFromNode(ostream &c)
    {
        c << "    if((node = searchNode->GetNode(\"" << name
          << "\")) != 0)" << endl;
        c << "        " << name << ".SetFromNode(node);" << endl;
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
    virtual QString GetAttributeGroupID()
    {
        return "i";
    }
    virtual QString DataNodeConversion()
    {
        return "AsInt";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << val << ";" << endl;
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
    virtual QString GetAttributeGroupID()
    {
        return "i";
    }
    virtual QString DataNodeConversion()
    {
        return "AsInt";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << val << ";" << endl;
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
    virtual QString GetAttributeGroupID()
    {
        return "d";
    }
    virtual QString DataNodeConversion()
    {
        return "AsDouble";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << val << ";" << endl;
    }
};


//
// -------------------------------- VariableName --------------------------------
//
class AttsGeneratorVariableName : public virtual VariableName , public virtual AttsGeneratorField
{
  public:
    AttsGeneratorVariableName(const QString &n, const QString &l)
        : VariableName(n,l), AttsGeneratorField("variablename",n,l),
          Field("variablename",n,l) { }
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
    virtual void WriteSourceInitializer(ostream &c)
    {
        if(valueSet)
            c << name << "(\""<<val<<"\")";
        else
            c << name << "(\"default\")";
    }
    virtual void WriteSourceSetDefault(ostream &c)
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
        : Att(t,n,l), AttsGeneratorField("att",n,l), Field("att",n,l) { }
    virtual bool CanHaveConst() { return true; }
    virtual void AddAttributeIncludes(UniqueStringList &sl) const
    {
        QString tmp;
        tmp.sprintf("#include <%s.h>\n", attType.latin1());
        sl.AddString(tmp.latin1());
    }
    virtual QString GetAttributeGroupID()
    {
        return "a";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
    }
    virtual void WriteSourceSetFromNode(ostream &c)
    {
        c << "    if((node = searchNode->GetNode(\"" << name
          << "\")) != 0)" << endl;
        c << "        " << name << ".SetFromNode(node);" << endl;
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
    virtual bool CanHaveConst() { return true; }
    virtual void WriteHeaderForwardDeclarations(ostream &h)
    {
        h << "class " << attType << ";" << endl;
    }
    virtual void WriteHeaderAGVectorProto(ostream &h)
    {
        QString plural("");
        if(Name.right(1) != "s")
            plural = "s";

        h << "    void Add" << Name << "(const " << attType << " &);" << endl;
        h << "    void Clear" << Name << plural << "();" << endl;
        h << "    void Remove" << Name << "(int i);" << endl;
        h << "    int  GetNum" << Name << plural << "() const;" << endl;
        h << "    " << attType << " &Get" << Name << "(int i);" << endl;
        h << "    const " << attType << " &Get" << Name << "(int i) const;" << endl;
        h << endl;
    }
    virtual void WriteHeaderSoloAGVectorProto(ostream &h)
    {
        h << "    " << attType << " &operator [] (int i);" << endl;
        h << "    const " << attType << " &operator [] (int i) const;" << endl;
        h << endl;
    }
    virtual void WriteSourceIncludes(ostream &c)
    {
        c << "#include <"<<attType<<".h>" << endl;
    }
    virtual QString GetAttributeGroupID()
    {
        return "a*";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
    }
    virtual void WriteHeaderSetFunction(ostream &h)
    {
        // Disabled the header method because the .C implementation (below)
        // does not output anything.
    }
    virtual void WriteSourceSetFunction(ostream &c, const QString &classname)
    {
        /*
        if (codeFile && codeFile->code.count(QString("Set")+Name) && !codeFile->code[QString("Set")+Name].first.isNull())
            c << codeFile->code[QString("Set")+Name].first;

        if (codeFile && codeFile->code.count(QString("Set")+Name) && !codeFile->code[QString("Set")+Name].second.isNull())
            c << codeFile->code[QString("Set")+Name].second;
        */
    }
    virtual void WriteSourceCopyCode(ostream &c)
    {
        c << "    // *** Copy the " << name << " field ***" << endl;
        c << "    // Delete the AttributeGroup objects and clear the vector." << endl;
        c << "    for(pos = " << name << ".begin(); pos != "
          << name << ".end(); ++pos)" << endl;
        c << "        delete *pos;" << endl;
        c << "    " << name << ".clear();" << endl;
        c << "    if(obj." << name << ".size() > 0)" << endl;
        c << "        " << name << ".reserve(obj." << name << ".size());" << endl;
        c << "    // Duplicate the " << name << " from obj." << endl;
        c << "    for(pos = obj." << name << ".begin(); pos != obj." << name << ".end(); ++pos)" << endl;
        c << "    {" << endl;
        c << "        " << attType << " *old" << attType << " = (" << attType << " *)(*pos);" << endl;
        c << "        " << attType << " *new" << attType << " = new " << attType << "(*old" << attType << ");" << endl;
        c << "        " << name << ".push_back(new" << attType << ");" << endl;
        c << "    }" << endl << endl;
    }
    virtual void WriteSourceDestructor(ostream &c)
    {
        c << "    // Destroy the " << name << " field." << endl;
        c << "    for(pos = " << name << ".begin();"
          << " pos != " << name << ".end(); ++pos)" << endl;
        c << "        delete *pos;" << endl;
    }
    virtual void WriteSourceSetFromNode(ostream &c)
    {
        QString plural("");
        if(Name.right(1) != "s")
            plural = "s";

        c << "    // Clear all the " << attType << "s." << endl;
        c << "    Clear" << Name << plural << "();" << endl;
        c << endl;
        c << "    // Go through all of the children and construct a new" << endl;
        c << "    // " << attType << " for each one of them." << endl;
                  
        c << "    children = searchNode->GetChildren();" << endl;
        c << "    for(i = 0; i < searchNode->GetNumChildren(); ++i)" << endl;
        c << "    {" << endl;
        c << "        if(children[i]->GetKey() == std::string(\"" << attType << "\"))" << endl;
        c << "        {" << endl;
        c << "            " << attType << " temp;" << endl;
        c << "            temp.SetFromNode(children[i]);" << endl;
        c << "            Add" << Name << "(temp);" << endl;
        c << "        }" << endl;
        c << "    }" << endl << endl;
    }
    virtual void WriteSourceAGVectorFunctions(ostream &c, const QString &classname, const QString &purpose)
    {
        QString methodName;
        QString s = attType;
        QString plural("");
        if(Name.right(1) != "s")
            plural = "s";

        // Write the Add method.
        methodName = "Add";
        methodName += Name;
        WriteMethodComment(c, classname, methodName, purpose);
        c << "void" << endl;
        c << classname << "::" << methodName << "(const " << s << " &obj)" << endl;
        c << "{" << endl;
        c << "    " << s << " *new" << s << " = new " << s << "(obj);" << endl;
        c << "    " << name << ".push_back(new" << s << ");" << endl;
        c << endl;
        c << "    // Indicate that things have changed by selecting it." << endl;
        c << "    Select(" << FieldID() << ", (void *)&" << name << ");" << endl;
        c << "}" << endl << endl;

        // Write the Clear method
        methodName = "Clear";
        methodName += Name + plural;
        WriteMethodComment(c, classname, methodName, purpose);
        c << "void" << endl;
        c << classname << "::" << methodName << "()" << endl;
        c << "{" << endl;
        c << "    AttributeGroupVector::iterator pos;" << endl;
        c << endl;
        c << "    for(pos = " << name << ".begin(); pos != " << name << ".end(); ++pos)" << endl;
        c << "        delete *pos;" << endl;
        c << "    " << name << ".clear();" << endl;
        c << endl;
        c << "    // Indicate that things have changed by selecting the list." << endl;
        c << "    Select(" << FieldID() << ", (void *)&" << name << ");" << endl;
        c << "}" << endl << endl;

        // Write the Remove method
        methodName = "Remove";
        methodName += Name;
        WriteMethodComment(c, classname, methodName, purpose);
        c << "void" << endl;
        c << classname << "::" << methodName << "(int index)" << endl;
        c << "{" << endl;
        c << "    AttributeGroupVector::iterator pos = " << name << ".begin();" << endl;
        c << endl;
        c << "    // Iterate through the vector \"index\" times. " << endl;
        c << "    for(int i = 0; i < index; ++i)" << endl;
        c << "        ++pos;" << endl;
        c << endl;
        c << "    // If pos is still a valid iterator, remove that element." << endl;
        c << "    if(pos != " << name << ".end())" << endl;
        c << "    {" << endl;
        c << "        delete *pos;" << endl;
        c << "        " << name << ".erase(pos);" << endl;
        c << "    }" << endl;
        c << endl;
        c << "    // Indicate that things have changed by selecting the list." << endl;
        c << "    Select(" << FieldID() << ", (void *)&" << name << ");" << endl;
        c << "}" << endl << endl;

        // Write the GetNum method
        methodName = "GetNum";
        methodName += Name + plural;
        WriteMethodComment(c, classname, methodName, purpose);
        c << "int" << endl;
        c << classname << "::" << methodName << "() const" << endl;
        c << "{" << endl;
        c << "    return " << name << ".size();" << endl;
        c << "}" << endl << endl;

        // Write the Get method
        methodName = "Get";
        methodName += Name;
        WriteMethodComment(c, classname, methodName, purpose);
        c << s << " &" << endl;
        c << classname << "::" << methodName << "(int i)" << endl;
        c << "{" << endl;
        c << "    return *((" << s << " *)" << name << "[i]);" << endl;
        c << "}" << endl << endl;

        // Write the const Get method
        methodName = "Get";
        methodName += Name;
        WriteMethodComment(c, classname, methodName, purpose);
        c << "const " << s << " &" << endl;
        c << classname << "::" << methodName << "(int i) const" << endl;
        c << "{" << endl;
        c << "    return *((" << s << " *)" << name << "[i]);" << endl;
        c << "}" << endl << endl;
    }
    virtual void WriteSourceSoloAGVectorFunctions(ostream &c, const QString &classname, const QString &purpose)
    {
        QString methodName;
        QString s = attType;

        // Write the non-const [] operator
        methodName = "operator []";
        WriteMethodComment(c, classname, methodName, purpose);
        c << s << " &" << endl;
        c << classname << "::" << methodName << " (int i)" << endl;
        c << "{" << endl;
        c << "    return *((" << s << " *)" << name << "[i]);" << endl;
        c << "}" << endl << endl;

        // Write the const [] operator
        WriteMethodComment(c, classname, methodName, purpose);
        c << "const " << s << " &" << endl;
        c << classname << "::" << methodName << " (int i) const" << endl;
        c << "{" << endl;
        c << "    return *((" << s << " *)" << name << "[i]);" << endl;
        c << "}" << endl << endl;
    }
    virtual void WriteSourceSubAttributeGroupSingle(ostream &c)
    {
        c << "    return new " << attType << ";" << endl;
    }
    virtual void WriteSourceSubAttributeGroup(ostream &c)
    {
        c << "    case " << FieldID() << ":" << endl;
        c << "        retval = new " << attType << ";" << endl;
        c << "        break;" << endl;
    }
    virtual void WriteSourceComparisonPrecalc(ostream &c, const QString &indent)
    {
        QString s = attType;
        c << indent << "bool " << name << "_equal = (obj." << name 
          << ".size() == " << name << ".size());" << endl;
        c << indent << "for(i = 0; (i < " << name
          << ".size()) && " << name << "_equal; ++i)" << endl;
        c << indent << "{" << endl;
        c << indent << "    // Make references to " << s
          << " from AttributeGroup *." << endl;
        c << indent << "    const " << s << " &" << name
          << "1 = *((const " << s << " *)(" << name << "[i]));" << endl;
        c << indent << "    const " << s << " &" << name
          << "2 = *((const " << s << " *)(obj." << name << "[i]));" << endl;
        c << indent << "    " << name << "_equal = ("
          << name << "1 == " << name << "2);" << endl;
        c << indent << "}" << endl << endl;
    }
    virtual void WriteSourceComparison(ostream &c)
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
        : Enum(t,n,l), AttsGeneratorField("enum",n,l), Field("enum",n,l) { }
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

    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " <<enumType->values[val] << ";" << endl;
    }
    virtual void WriteSourceSetFromNode(ostream &c)
    {
        c << "    if((node = searchNode->GetNode(\"" << name
          << "\")) != 0)" << endl;
        c << "    {" << endl;
        c << "        // Allow enums to be int or string in the config file" << endl;
        c << "        if(node->GetNodeType() == INT_NODE)" << endl;
        c << "        {" << endl;
        c << "            int ival = node->AsInt();" << endl;
        c << "            if(ival >= 0 && ival < " << enumType->values.size() << ")" << endl;
        c << "                Set" << Name << "(" << GetCPPName(true) << "(ival));" << endl;
        c << "        }" << endl;
        c << "        else if(node->GetNodeType() == STRING_NODE)" << endl;
        c << "        {" << endl;
        c << "            " << GetCPPName(true) << " value;" << endl;
        c << "            if(" << GetCPPName(true) << "_FromString(node->AsString(), value))" << endl;
        c << "                Set" << Name << "(value);" << endl;
        c << "        }" << endl;
        c << "    }" << endl;
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
    virtual QString GetAttributeGroupID()
    {
        return "i";
    }
    virtual QString DataNodeConversion()
    {
        return "AsInt";
    }
    virtual void WriteSourceSetDefault(ostream &c)
    {
        c << "    " << name << " = " << val << ";" << endl;
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
    virtual void WriteSourceSetDefault(ostream &c)\
    {\
        c << "    " << name << " = ";\
        int nsym = 0;\
        const char **sym = GetSymbols(nsym);\
        if(val >= 0 && val < nsym)\
            c << sym[val];\
        else\
            c << val;\
        c << ";" << endl;\
    }\
    virtual QString GetFieldType() const { return "int"; }

//
// ----------------------------------- avtCentering -----------------------------------
//
class AttsGeneratoravtCentering : public virtual AttsGeneratorField, public virtual avtCenteringField
{
  public:
    AttsGeneratoravtCentering(const QString &n, const QString &l)
        : avtCenteringField(n,l), AttsGeneratorField("avtCentering",n,l), Field("avtCentering",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtGhostType -----------------------------------
//
class AttsGeneratoravtGhostType : public virtual AttsGeneratorField, public virtual avtGhostTypeField
{
  public:
    AttsGeneratoravtGhostType(const QString &n, const QString &l)
        : avtGhostTypeField(n,l), AttsGeneratorField("avtGhostType",n,l), Field("avtGhostType",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtSubsetType -----------------------------------
//
class AttsGeneratoravtSubsetType : public virtual AttsGeneratorField, public virtual avtSubsetTypeField
{
  public:
    AttsGeneratoravtSubsetType(const QString &n, const QString &l)
        : avtSubsetTypeField(n,l), AttsGeneratorField("int",n,l), Field("int",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtVarType -----------------------------------
//
class AttsGeneratoravtVarType : public virtual AttsGeneratorField, public virtual avtVarTypeField
{
  public:
    AttsGeneratoravtVarType(const QString &n, const QString &l)
        : avtVarTypeField(n,l), AttsGeneratorField("avtVarType",n,l), Field("avtVarTypeField",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtMeshType -----------------------------------
//
class AttsGeneratoravtMeshType : public virtual AttsGeneratorField, public virtual avtMeshTypeField
{
  public:
    AttsGeneratoravtMeshType(const QString &n, const QString &l)
        : avtMeshTypeField(n,l), AttsGeneratorField("avtMeshType",n,l), Field("avtMeshTypeField",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtExtentType -----------------------------------
//
class AttsGeneratoravtExtentType : public virtual AttsGeneratorField, public virtual avtExtentTypeField
{
  public:
    AttsGeneratoravtExtentType(const QString &n, const QString &l)
        : avtExtentTypeField(n,l), AttsGeneratorField("avtExentType",n,l), Field("avtExtentType",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- avtMeshCoordType -----------------------------------
//
class AttsGeneratoravtMeshCoordType : public virtual AttsGeneratorField, public virtual avtMeshCoordTypeField
{
  public:
    AttsGeneratoravtMeshCoordType(const QString &n, const QString &l)
        : avtMeshCoordTypeField(n,l), AttsGeneratorField("avtMeshCoordType",n,l), Field("avtMeshCoordType",n,l)
    { }
    AVT_GENERATOR_METHODS
};

//
// ----------------------------------- LoadBalanceScheme -----------------------------------
//
class AttsGeneratorLoadBalanceScheme : public virtual AttsGeneratorField, public virtual LoadBalanceSchemeField
{
  public:
    AttsGeneratorLoadBalanceScheme(const QString &n, const QString &l)
        : LoadBalanceSchemeField(n,l), AttsGeneratorField("LoadBalanceScheme",n,l), Field("LoadBalanceScheme",n,l)
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
            throw QString().sprintf("AttsFieldFactory: unknown type for field %s: %s",name.latin1(),type.latin1());

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
// ----------------------------------------------------------------------------
class AttsGeneratorAttribute
{
  public:
    QString name;
    QString purpose;
    bool    persistent, keyframe;
    QString exportAPI;
    QString exportInclude;
    vector<AttsGeneratorField*> fields;
    vector<Function*> functions;
    vector<Constant*> constants;
    vector<Include*>  includes;
    vector<Code*>     codes;
    CodeFile *codeFile;
  public:
    AttsGeneratorAttribute(const QString &n, const QString &p, const QString &f,
                           const QString &e, const QString &ei)
        : name(n), purpose(p), exportAPI(e), exportInclude(ei)
    {
        if (f.isNull())
            codeFile = NULL;
        else
            codeFile = new CodeFile(f);
        if (codeFile)
            codeFile->Parse();
        persistent = false;
        keyframe = true;
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
            {
                out << functions[i]->def;
                functions[i]->usedThisFunction = true;
            }
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
        out << "        exportAPI=" << exportAPI << endl;
        out << "        exportInclude=" << exportInclude << endl;
        int i;
        for (i=0; i<fields.size(); i++)
            fields[i]->Print(out);
        for (i=0; i<includes.size(); i++)
            includes[i]->Print(out);
        for (i=0; i<functions.size(); i++)
            functions[i]->Print(out);
        for (i=0; i<constants.size(); i++)
            constants[i]->Print(out);
    }
    void WriteHeaderClassComment(ostream &h)
    {
        h << "// ****************************************************************************" << endl;
        h << "// Class: " << name << endl;
        h << "//" << endl;
        h << "// Purpose:" << endl;
        h << "//    " << purpose << endl;
        h << "//" << endl;
        h << "// Notes:      Autogenerated by xml2atts." << endl;
        h << "//" << endl;
        h << "// Programmer: xml2atts" << endl;
        h << "// Creation:   " << CurrentTime() << endl;
        h << "//" << endl; 
        h << "// Modifications:" << endl;
        h << "//   " << endl;
        h << "// ****************************************************************************" << endl;
        h << endl;
    }
    void WriteHeaderSystemIncludes(ostream &h)
    {
        UniqueStringList sysincludes;
        for (int i=0; i<fields.size(); i++)
            fields[i]->AddSystemIncludes(sysincludes);
        sysincludes.Write(h);
    }
    void WriteHeaderAttributeIncludes(ostream &h)
    {
        UniqueStringList attsincludes;
        for (int i=0; i<fields.size(); i++)
            fields[i]->AddAttributeIncludes(attsincludes);
        attsincludes.Write(h);
    }
    void WriteHeaderForwardDeclarations(ostream &h)
    {
        for (int i=0; i<fields.size(); i++)
            fields[i]->WriteHeaderForwardDeclarations(h);
    }
    void WriteHeaderSelectFunctions(ostream &h)
    {
        for (int i=0; i<fields.size(); i++)
        {
            if(fields[i]->accessType == Field::AccessPrivate)
            fields[i]->WriteHeaderSelectFunction(h);
        }
    }
    void WriteHeaderFieldIDs(ostream &h)
    {
        if(fields.size() > 0)
        {
            h << "    // IDs that can be used to identify fields in case statements" << endl;
            h << "    enum {" << endl;
        }
        for (int i=0; i<fields.size(); i++)
        {
            h << "        " << fields[i]->FieldID();
            if(i == 0)
                h << " = 0";
            if(i < fields.size()-1)
                h << ",";
            h << endl;
        }
        if(fields.size() > 0)
        {
            h << "    };" << endl;
            h << endl;
        }
    }
    void WriteHeaderEnumConversions(ostream &h)
    {
        // Write the enums functions
        if(EnumType::enums.size() > 0)
        {
            h << "    // Enum conversion functions" << endl;
            for (int i = 0; i < EnumType::enums.size(); ++i)
            {
                h << "    static std::string "<<EnumType::enums[i]->type<<"_ToString("<<EnumType::enums[i]->type<<");" << endl;
                h << "    static bool "<<EnumType::enums[i]->type<<"_FromString(const std::string &, " << EnumType::enums[i]->type<<" &);" << endl;
                h << "protected:" << endl;
                h << "    static std::string "<<EnumType::enums[i]->type<<"_ToString(int);" << endl;
                h << "public:" << endl;
            }
        }
    }

    int CalculateTotalWidth(bool considerConst)
    {
        int retval = 0;

        // Iterate through the list of attibutes and find the one with
        // the longest name.
        for (int i=0; i<fields.size(); i++)
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
        for (int i=0; i<fields.size(); i++)
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
        for (int i=0; i<fields.size(); i++)
        {
            if (fields[i]->type=="attVector")
                return true;
        }
        return false;
    }
    bool HaveSoloAGVector()
    {
        int count = 0;
        for (int i=0; i<fields.size(); i++)
        {
            if (fields[i]->type=="attVector")
                count++;
        }
        return (count == 1);
    }
    bool HaveArrays()
    {
        for (int i=0; i<fields.size(); i++)
        {
            if (fields[i]->isArray)
                return true;
        }
        return false;
    }
    bool HaveArraysThatNeedIndexVar()
    {
        for (int i=0; i<fields.size(); i++)
        {
            if (fields[i]->isArray && fields[i]->length >= 4)
                return true;
        }
        return false;
    }
    bool HaveVectors()
    {
        for (int i=0; i<fields.size(); i++)
        {
            if (fields[i]->isVector)
                return true;
        }
        return false;
    }

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    void WriteHeader(ostream &h)
    {
        h << copyright_str.c_str() << endl;
        h << "#ifndef " << name.upper() << "_H" << endl;
        h << "#define " << name.upper() << "_H" << endl;
        if(!exportInclude.isEmpty())
            h << "#include <" << exportInclude << ">" << endl;
        WriteHeaderSystemIncludes(h);
        h << "#include <AttributeSubject.h>" << endl;
        WriteHeaderForwardDeclarations(h);
        WriteHeaderAttributeIncludes(h);
        // write user header includes
        int i;
        for (i=0; i<includes.size(); i++)
        {
            if (includes[i]->target=="header")
            {
                if (includes[i]->quoted == true)
                    h << "#include \"" << includes[i]->include << "\"" << endl;
                else
                    h << "#include <"  << includes[i]->include << ">"  << endl;
            }
        }

        h << endl;

        // write non-member constants
        for (i=0; i<constants.size(); i++)
        {
            if (! constants[i]->member)
                h << constants[i]->decl << endl;
        }

        WriteHeaderClassComment(h);
        if(exportAPI.isEmpty())
            h << "class " << name << " : public AttributeSubject" << endl;
        else
            h << "class " << exportAPI << " " << name << " : public AttributeSubject" << endl;
        h << "{" << endl;
        h << "public:" << endl;
        for (i=0; i<EnumType::enums.size(); i++)
        {
            h << "    enum " << EnumType::enums[i]->type << endl;
            h << "    {" << endl;
            for (int j=0; j<EnumType::enums[i]->values.size(); j++)
            {
                h << "        " << EnumType::enums[i]->values[j];
                if (j < EnumType::enums[i]->values.size()-1)
                    h << ",";
                h << endl;
            }
            h << "    };" << endl;
        }
        // write member constants
        for (i=0; i<constants.size(); i++)
        {
            if (constants[i]->member)
            h << "    " << constants[i]->decl << endl;
        }
        if (EnumType::enums.size() || constants.size())
            h << endl;
        h << "    " << name << "();" << endl;
        h << "    " << name << "(const " << name << " &obj);" << endl;
        h << "    virtual ~" << name << "();" << endl;
        h << endl;
        h << "    virtual " << name << "& operator = (const " << name << " &obj);" << endl;
        h << "    virtual bool operator == (const " << name << " &obj) const;" << endl;
        h << "    virtual bool operator != (const " << name << " &obj) const;" << endl;
        h << endl;
        h << "    virtual const std::string TypeName() const;" << endl;
        h << "    virtual bool CopyAttributes(const AttributeGroup *);" << endl;
        h << "    virtual AttributeSubject *CreateCompatible(const std::string &) const;" << endl;
        h << "    virtual AttributeSubject *NewInstance(bool) const;" << endl;
        h << endl;
        h << "    // Property selection methods" << endl;
        h << "    virtual void SelectAll();" << endl;
        WriteHeaderSelectFunctions(h);

        // Determine whether there are public and private fields.
        bool hasPrivateFields = false;
        bool hasProtectedFields = false;
        bool hasPublicFields = false;
        bool onlyHasPublicFields = true;
        for (i=0; i<fields.size(); i++)
        {
            if(fields[i]->accessType == Field::AccessPrivate)
            {
                hasPrivateFields = true;
                onlyHasPublicFields = false;
            }
            if(fields[i]->accessType == Field::AccessProtected)
            {
                hasProtectedFields = true;
                onlyHasPublicFields = false;
            }
            if(fields[i]->accessType == Field::AccessPublic)
                hasPublicFields = true;
        }

        if(hasPrivateFields)
        {
            h << endl;
            h << "    // Property setting methods" << endl;
        }
        // Write out all the set prototypes
        for (i=0; i<fields.size(); i++)
        {
            if(fields[i]->accessType != Field::AccessPrivate)
                continue;
            fields[i]->WriteHeaderSetFunction(h);
        }

        int totalWidth = CalculateTotalWidth(true);
        if(hasPrivateFields)
        {
            h << endl;
            h << "    // Property getting methods" << endl;
        }
        // Write out all the get prototypes
        for (i=0; i<fields.size(); i++)
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
            h << endl;
            h << "    // Persistence methods" << endl;
            h << "    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);" << endl;
            h << "    virtual void SetFromNode(DataNode *node);" << endl;
        }
        h << endl;

        // If there are AttributeGroupVectors in the list, write out
        // the convenience methods.
        if (HaveAGVectors())
        {
            h << endl;
            h << "    // Attributegroup convenience methods" << endl;
            for (i=0; i<fields.size(); i++)
            {
                fields[i]->WriteHeaderAGVectorProto(h);
            }
        }
        if (HaveSoloAGVector())
        {
            for (i=0; i<fields.size(); i++)
            {
                fields[i]->WriteHeaderSoloAGVectorProto(h);
            }
        }

        WriteHeaderEnumConversions(h);

        // Methods for keyframing
        if(keyframe)
        {
        h << endl;
        h << "    // Keyframing methods" << endl;
        h << "    virtual std::string               GetFieldName(int index) const;" << endl;
        h << "    virtual AttributeGroup::FieldType GetFieldType(int index) const;" << endl;
        h << "    virtual std::string               GetFieldTypeName(int index) const;" << endl;
        h << "    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;" << endl;
        h << endl;
        }

        // Write user-defined methods
        bool wroteUserDefinedHeading = false;
        for (i=0; i<functions.size(); i++)
        {
            if (functions[i]->user && functions[i]->member)
            {
                if (! wroteUserDefinedHeading)
                {
                    h << "    // User-defined methods" << endl;
                    wroteUserDefinedHeading = true;
                }
                h << "    " << functions[i]->decl << endl;
            }
        }
        h << endl;

        // Write field IDs
        if(!onlyHasPublicFields)
            WriteHeaderFieldIDs(h);

        // If there are any AttributeGroupVectors, we'll need this method.
        if (HaveAGVectors())
        {
            h << "protected:" << endl;
            h << "    AttributeGroup *CreateSubAttributeGroup(int index);" << endl;
        }

        totalWidth = CalculateTotalWidth(false);

        // Write out all the public attributes.
        if(hasPublicFields)
        {
            h << "public:" << endl;
            for (i=0; i<fields.size(); i++)
            {
                if(fields[i]->accessType != Field::AccessPublic)
                    continue;
                fields[i]->WriteHeaderAttribute(h, totalWidth);
            }
        }
        // Write out all the protected attributes
        if(hasProtectedFields)
        {
            h << "protected:" << endl;
            for (i=0; i<fields.size(); i++)
            {
                if(fields[i]->accessType != Field::AccessProtected)
                    continue;
                fields[i]->WriteHeaderAttribute(h, totalWidth);
            }
        }
        // Write out all the private attributes
        if(hasPrivateFields)
        {
            h << "private:" << endl;
            for (i=0; i<fields.size(); i++)
            {
                if(fields[i]->accessType != Field::AccessPrivate)
                    continue;
                fields[i]->WriteHeaderAttribute(h, totalWidth);
            }
        }
        h << endl;
        if(!hasPrivateFields)
            h << "private:" << endl;
        h << "    // Static class format string for type map." << endl;
        h << "    static const char *TypeMapFormatString;" << endl;
        h << "};" << endl;
        h << endl;

        bool wroteUserDefinedFunctionsHeading = false;
        for (i=0; i<functions.size(); i++)
        {
            if (functions[i]->user && !functions[i]->member)
            {
                if (! wroteUserDefinedFunctionsHeading)
                {
                    h << "// User-defined functions" << endl;
                    wroteUserDefinedFunctionsHeading = true;
                }
                h << functions[i]->decl << endl;
            }
        }

        h << "#endif" << endl;
    }
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    void WriteSourceIncludes(ostream &c)
    {
        c << "#include <DataNode.h>" << endl;

        // write user source includes
        int i;
        for (i=0; i<includes.size(); i++)
        {
            if (includes[i]->target=="source")
            {
                if (includes[i]->quoted == true)
                    c << "#include \"" << includes[i]->include << "\"" << endl;
                else
                    c << "#include <"  << includes[i]->include << ">"  << endl;
            }
        }

        for (i=0; i<fields.size(); i++)
        {
            fields[i]->WriteSourceIncludes(c);
        }

        c << endl;
    }

    void WriteSourceConstructor(ostream &c)
    {
        // Write the typemap format string.
        QString formatString;
        int i;
        for (i=0; i<fields.size(); i++)
            formatString += fields[i]->GetAttributeGroupID();
        c << "// Type map format string" << endl;
        c << "const char *" << name << "::TypeMapFormatString = \"" << formatString << "\";";
        c << endl << endl;

        // Write the method comment.
        QString purposeString("Constructor for the ");
        purposeString += (name + " class.");
        WriteMethodComment(c, name, name, purposeString);

        c << name << "::" << name << "() : \n    AttributeSubject(" << name << "::TypeMapFormatString)";

        // Count the number of fields that require an initializer
        int nInitializers = 0;
        for (i=0; i<fields.size(); ++i)
        {
            if(fields[i]->RequiresSourceInitializer())
                ++nInitializers;
        }
        // Write the initializers.
        if(nInitializers > 0)
        {
            bool endLine;
            c << "," << endl << "    ";
            for(int i = 0, j = 0; i < fields.size(); ++i)
            {
                if(fields[i]->RequiresSourceInitializer())
                {
                    endLine = false;
                    fields[i]->WriteSourceInitializer(c);
                    if(j < nInitializers - 1)
                        c << ", ";
                    if(((j + 1) % 2) == 0)
                    {
                        c << endl;
                        endLine = true;
                    }
                    if(endLine && (j < nInitializers - 1))
                        c << "    ";
                    ++j;
                }
            }
            if(!endLine)
                c << endl;
        }
        else
            c << endl;

        c << "{" << endl;
        for (i=0; i<fields.size(); i++)
        {
            if (!fields[i]->initcode.isNull())
                c << fields[i]->initcode;
            else if (fields[i]->valueSet)
                fields[i]->WriteSourceSetDefault(c);
        }
        c << "}" << endl << endl;
    }

    void WriteSourceCopyCode(ostream &c)
    {
        bool skipLine = false;
        if (HaveAGVectors())
        {
            c << "    AttributeGroupVector::const_iterator pos;" << endl;
            skipLine = true;
        }
        if (HaveArraysThatNeedIndexVar())
        {
            c << "    int i;" << endl;
            skipLine = true;
        }
        if(skipLine)
            c << endl;
        for (int i=0; i<fields.size(); i++)
        {
            fields[i]->WriteSourceCopyCode(c);
        }
        c << endl << "    SelectAll();" << endl;
    }

    void WriteSourceCopyConstructor(ostream &c)
    {
        QString purposeString("Copy constructor for the ");
        purposeString += (name + " class.");
        WriteMethodComment(c, name, name, purposeString);

        c << name << "::" << name << "(const "
          << name << " &obj) : \n    AttributeSubject(" << name
          << "::TypeMapFormatString)" << endl;

        c << "{" << endl;
        WriteSourceCopyCode(c);
        c << "}" << endl << endl;
    }
    void WriteSourceDestructor(ostream &c)
    {
        // Write the method comment.
        QString purposeString("Destructor for the ");
        purposeString += (name + " class.");
        QString methodName("~");
        methodName += name;
        WriteMethodComment(c, name, methodName, purposeString);

        // See if there are any AG vectors.
        c << name << "::~" << name << "()" << endl;
        c << "{" << endl;
        if (/*!HaveAGArrays() && */!HaveAGVectors())
            c << "    // nothing here" << endl;
        else 
        {
            if (HaveAGVectors())
                c << "    AttributeGroupVector::iterator pos;" << endl;
            /*if (HaveAGArrays())
              c << "    int i;" << endl;*/
            c << endl;

            for (int i=0; i<fields.size(); i++)
                fields[i]->WriteSourceDestructor(c);
        }
        c << "}" << endl << endl;
    }
    void WriteSourceAssignmentOperator(ostream &c)
    {
        if (HasFunction("operator ="))
        {
            PrintFunction(c, "operator =");
            c << endl;
            return;
        }
        // Write the method comment.
        QString purposeString("Assignment operator for the ");
        purposeString += (name + " class.");
        QString methodName("operator = ");
        WriteMethodComment(c, name, methodName, purposeString);

        c << name << "& " << endl;
        c << name << "::operator = (const "
          << name << " &obj)" << endl;
        c << "{" << endl;
        c << "    if (this == &obj) return *this;" << endl;
        WriteSourceCopyCode(c);
        c << "    return *this;" << endl;
        c << "}" << endl << endl;
    }
    void WriteSourceTypeName(ostream &c)
    {
        // Write the method comment.
        QString purposeString("Type name method for the ");
        purposeString += (name + " class.");
        QString methodName("TypeName");
        WriteMethodComment(c, name, methodName, purposeString);

        c << "const std::string" << endl;
        c << name << "::TypeName() const" << endl;
        c << "{" << endl;
        c << "    return \"" << name << "\";" << endl;
        c << "}" << endl << endl;
    }
    void WriteSourceCopyAttributes(ostream &c)
    {
        if (HasFunction("CopyAttributes"))
        {
            PrintFunction(c, "CopyAttributes");
            c << endl;
            return;
        }

        // Write the method comment.
        QString purposeString("CopyAttributes method for the ");
        purposeString += (name + " class.");
        QString methodName("CopyAttributes");
        WriteMethodComment(c, name, methodName, purposeString);

        c << "bool" << endl;
        c << name << "::CopyAttributes(const AttributeGroup *atts)" << endl;
        c << "{" << endl;
        c << "    if(TypeName() != atts->TypeName())" << endl;
        c << "        return false;" << endl;
        c << endl;
        c << "    // Call assignment operator." << endl;
        c << "    const " << name << " *tmp = (const " << name << " *)atts;" << endl;
        c << "    *this = *tmp;" << endl;
        c << endl;
        c << "    return true;" << endl;
        c << "}" << endl << endl;
    }
    void WriteSourceNewInstance(ostream &c)
    {
        if (HasFunction("NewInstance"))
        {
            PrintFunction(c, "NewInstance");
            c << endl;
            return;
        }

        // Write the method comment.
        QString purposeString("NewInstance method for the ");
        purposeString += (name + " class.");
        QString methodName("NewInstance");
        WriteMethodComment(c, name, methodName, purposeString);

        c << "AttributeSubject *" << endl;
        c << name << "::NewInstance(bool copy) const" << endl;
        c << "{" << endl;
        c << "    AttributeSubject *retval = 0;" << endl;
        c << "    if(copy)" << endl;
        c << "        retval = new " << name << "(*this);" << endl;
        c << "    else" << endl;
        c << "        retval = new " << name << ";" << endl;
        c << endl;
        c << "    return retval;" << endl;
        c << "}" << endl << endl;
    }
    void WriteSourceCreateCompatible(ostream &c)
    {
        if (HasFunction("CreateCompatible"))
        {
            PrintFunction(c, "CreateCompatible");
            c << endl;
            return;
        }

        // Write the method comment.
        QString purposeString("CreateCompatible method for the ");
        purposeString += (name + " class.");
        QString methodName("CreateCompatible");
        WriteMethodComment(c, name, methodName, purposeString);

        c << "AttributeSubject *" << endl;
        c << name << "::CreateCompatible(const std::string &tname) const" << endl;
        c << "{" << endl;
        c << "    AttributeSubject *retval = 0;" << endl;
        c << "    if(TypeName() == tname)" << endl;
        c << "        retval = new " << name << "(*this);" << endl;
        c << "    // Other cases could go here too. " << endl;
        c << endl;
        c << "    return retval;" << endl;
        c << "}" << endl << endl;
    }
    void WriteSourceComparisonOperators(ostream &c)
    {
        QString purposeString("Comparison operator == for the ");
        purposeString += (name + " class.");
        QString methodName("operator == ");
        WriteMethodComment(c, name, methodName, purposeString);

        c << "bool" << endl;
        c << name << "::operator == (const "
          << name << " &obj) const" << endl;
        c << "{" << endl;

        if(HaveArrays() || HaveAGVectors())
        {
            c << "    int i;" << endl << endl;
        }

        // Create bool values to evaluate the arrays.
        QString prevValue("true");
        for (int i=0; i<fields.size(); i++)
        {
            if (!fields[i]->ignoreEquality)
                fields[i]->WriteSourceComparisonPrecalc(c, "    ");
        }

        c << "    // Create the return value" << endl;
        c << "    return (";

        // Create a big boolean return statement.
        if (fields.size() == 0)
        {
            c << "true";
        }
        else
        {
            for (int i=0; i<fields.size(); i++)
            {
                if (i > 0)
                    c << "            ";

                if (!fields[i]->ignoreEquality)
                    fields[i]->WriteSourceComparison(c);
                else
                    c << "true /* can ignore " << fields[i]->name << " */";

                if (i < fields.size() - 1)
                    c << " &&" << endl;
            }
        }
        c << ");" << endl;

        c << "}" << endl << endl;

        // Write the != operator
        purposeString = "Comparison operator != for the ";
        purposeString += (name + " class.");
        methodName = "operator != ";
        WriteMethodComment(c, name, methodName, purposeString);

        c << "bool" << endl;
        c << name << "::operator != (const "
          << name << " &obj) const" << endl;
        c << "{" << endl;
        c << "    return !(this->operator == (obj));" << endl;
        c << "}" << endl << endl;
    }
    int MaxFieldLength() const
    {
        int maxlen = 0;
        for (int i=0; i<fields.size(); i++)
        {
            int len = fields[i]->FieldID().length();
            maxlen = (len > maxlen) ? len : maxlen;
        }
        return maxlen;
    }
    QString PadStringWithSpaces(const QString &s, int len) const
    {
        QString ret(s);
        while(ret.length() < len)
            ret += QString(" ");
        return ret;
    }
    void WriteSourceSelectAll(ostream &c)
    {
        // Write the method comment.
        QString purposeString("Selects all attributes.");
        QString methodName("SelectAll");
        WriteMethodComment(c, name, methodName, purposeString);

        c << "void" << endl;
        c << name << "::SelectAll()" << endl;
        c << "{" << endl;
        int maxlen = MaxFieldLength() + 2;
        for (int i=0; i<fields.size(); i++)
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
            c << endl;
        }

        c << "}" << endl << endl;
    }
    void WriteSourceSubAttributeGroup(ostream &c)
    {
        // See if there are any AG vectors.
        int AG_dynamic_count = 0;
        for (int i=0; i<fields.size(); i++)
        {
            if (fields[i]->type == "attVector"/* || fields[i]->type == "attArray"*/)
                ++AG_dynamic_count;
        }
        if(AG_dynamic_count < 1)
            return;

        WriteMethodComment(c, name, "CreateSubAttributeGroup", purpose);
        c << "AttributeGroup *" << endl;
        c << name << "::CreateSubAttributeGroup(int";

        if(AG_dynamic_count == 1)
        {
            c << ")" << endl;
            c << "{" << endl;

            for (int i=0; i<fields.size(); i++)
            {
                fields[i]->WriteSourceSubAttributeGroupSingle(c);
            }
        }
        else
        {
            c << " attr_id)" << endl;
            c << "{" << endl;

            c << "    AttributeGroup *retval = 0;" << endl;
            c << "    switch(attr_id)" << endl;
            c << "    {" << endl;
            for (int i=0; i<fields.size(); i++)
            {
                fields[i]->WriteSourceSubAttributeGroup(c);
            }
            c << "    }" << endl;
            c << endl << "    return retval;" << endl;
        }
        c << "}" << endl << endl;
    }
    void WriteSourceCreateNode(ostream &c)
    {
        if (HasFunction("CreateNode"))
        {
            PrintFunction(c, "CreateNode");
            return;
        }
        WriteMethodComment(c, name, "CreateNode",
                           "This method creates a DataNode representation of the object so it can be saved to a config file.");

        c << "bool" << endl;
        c << name << "::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)" << endl;
        c << "{" << endl;
        c << "    if(parentNode == 0)" << endl;
        c << "        return false;" << endl << endl;
        c << "    " << name << " defaultObject;" << endl;
        c << "    bool addToParent = false;" << endl;
        c << "    // Create a node for " << name << "." << endl;
        c << "    DataNode *node = new DataNode(\"" << name << "\");" << endl << endl;

        // Write out the DataNode creation for all attributes.
        for (int i=0; i<fields.size(); i++)
        {
            QString forceAdd("false"); 
            if(fields[i]->type != "color")
            {
                c << "    if(completeSave || !FieldsEqual(" << fields[i]->FieldID() << ", &defaultObject))" << endl;
                c << "    {" << endl;
            }
            else
                forceAdd = "true";

            if (fields[i]->GetAttributeGroupID() == "a")
            {
                QString nodeName = fields[i]->name + "Node";
                c << "        DataNode *" << nodeName << " = new DataNode(\""
                  << fields[i]->name << "\");" << endl;
                c << "        if(" << fields[i]->name << ".CreateNode("
                  << nodeName << ", completeSave, " << forceAdd << "))" << endl;
                c << "        {" << endl;
                c << "            addToParent = true;" << endl;
                c << "            node->AddNode(" << nodeName << ");" << endl;
                c << "        }" << endl;
                c << "        else" << endl;
                c << "            delete " << nodeName << ";" << endl;
            }
            else if(fields[i]->type == "attArray")
            {
                c << "        addToParent = true;" << endl;
                c << "        for(int i = 0; i < " << fields[i]->length << "; ++i)" << endl;
                c << "            " << fields[i]->name << "[i]->CreateNode(node, completeSave. true);" << endl;
            }
            else if(fields[i]->type == "attVector")
            {
                c << "        addToParent = true;" << endl;
                c << "        for(int i = 0; i < " << fields[i]->name << ".size(); ++i)" << endl;
                c << "            " << fields[i]->name << "[i]->CreateNode(node, completeSave, true);" << endl;
            }
            else if (fields[i]->isArray)
            {
                c << "        addToParent = true;" << endl;
                c << "        node->AddNode(new DataNode(\"" << fields[i]->name
                  << "\", " << fields[i]->name << ", " << fields[i]->length << "));" << endl;
            }
            else if (fields[i]->type == "enum")
            {
                c << "        addToParent = true;" << endl;
                c << "        node->AddNode(new DataNode(\"" << fields[i]->name
                  << "\", " << fields[i]->GetCPPName(true) << "_ToString("
                  << fields[i]->name << ")));" << endl;
            }
            else
            {
                c << "        addToParent = true;" << endl;
                c << "        node->AddNode(new DataNode(\"" << fields[i]->name
                  << "\", " << fields[i]->name << "));" << endl;
            }

            if(fields[i]->type != "color")
                c << "    }" << endl << endl;
        }

        c << endl;
        c << "    // Add the node to the parent node." << endl;
        c << "    if(addToParent || forceAdd)" << endl;
        c << "        parentNode->AddNode(node);" << endl;
        c << "    else" << endl;
        c << "        delete node;" << endl << endl;
        c << "    return (addToParent || forceAdd);" << endl;
        c << "}" << endl;
        c << endl;
    }
    void WriteSourceSetFromNode(ostream &c)
    {
        if (HasFunction("SetFromNode"))
        {
            PrintFunction(c, "SetFromNode");
            return;
        }
        WriteMethodComment(c, name, "SetFromNode",
            "This method sets attributes in this object from values in a DataNode representation of the object.");

        c << "void" << endl;
        c << name << "::SetFromNode(DataNode *parentNode)" << endl;
        c << "{" << endl;
        c << "    int i;" << endl;
        c << "    if(parentNode == 0)" << endl;
        c << "        return;" << endl << endl;
        c << "    DataNode *searchNode = parentNode->GetNode(\"" << name << "\");" << endl;
        c << "    if(searchNode == 0)" << endl;
        c << "        return;" << endl;
        c << endl;
        c << "    DataNode *node;" << endl;
        if (HaveAGVectors())
            c << "    DataNode **children;" << endl;

        if (codeFile && codeFile->code.count("SetFromNode") && !codeFile->code["SetFromNode"].first.isNull())
            c << codeFile->code["SetFromNode"].first << endl;

        for (int i=0; i<fields.size(); i++)
        {
            fields[i]->WriteSourceSetFromNode(c);
        } // end for

        if (codeFile && codeFile->code.count("SetFromNode") && !codeFile->code["SetFromNode"].second.isNull())
            c << endl << codeFile->code["SetFromNode"].second;

        c << "}" << endl << endl;
    }
    void WriteSourceKeyframeFunctions(ostream &c)
    {
        WriteMethodComment(c, name, "GetFieldName",
            "This method returns the name of a field given its index.");

        int maxlen = MaxFieldLength() + 1;

        c << "std::string" << endl;
        c << name << "::GetFieldName(int index) const" << endl;
        c << "{" << endl;
        c << "    switch (index)" << endl;
        c << "    {" << endl;
        int i;
        for (i=0; i<fields.size(); i++)
        {
            QString fieldID(PadStringWithSpaces(fields[i]->FieldID() + QString(":"), maxlen));
            c << "    case "<<fieldID<<" return \""<<fields[i]->name<<"\";" << endl;
        }
        c << "    default:  return \"invalid index\";" << endl;
        c << "    }" << endl;
        c << "}" << endl;
        c << endl;

        WriteMethodComment(c, name, "GetFieldType",
            "This method returns the type of a field given its index.");

        c << "AttributeGroup::FieldType" << endl;
        c << name << "::GetFieldType(int index) const" << endl;
        c << "{" << endl;
        c << "    switch (index)" << endl;
        c << "    {" << endl;
        for (i=0; i<fields.size(); i++)
        {
            QString fieldID(PadStringWithSpaces(fields[i]->FieldID() + QString(":"), maxlen));
            c << "    case "<<fieldID<<" return FieldType_"<<fields[i]->GetFieldType()<<";" << endl;
        }
        c << "    default:  return FieldType_unknown;" << endl;
        c << "    }" << endl;
        c << "}" << endl;
        c << endl;

        WriteMethodComment(c, name, "GetFieldTypeName",
            "This method returns the name of a field type given its index.");

        c << "std::string" << endl;
        c << name << "::GetFieldTypeName(int index) const" << endl;
        c << "{" << endl;
        c << "    switch (index)" << endl;
        c << "    {" << endl;
        for (i=0; i<fields.size(); i++)
        {
            QString fieldID(PadStringWithSpaces(fields[i]->FieldID() + QString(":"), maxlen));
            c << "    case "<<fieldID<<" return \""<<fields[i]->type<<"\";" << endl;
        }
        c << "    default:  return \"invalid index\";" << endl;
        c << "    }" << endl;
        c << "}" << endl;
        c << endl;

        WriteMethodComment(c, name, "FieldsEqual",
            "This method compares two fields and return true if they are equal.");

        c << "bool" << endl;
        c << name << "::FieldsEqual(int index_, const AttributeGroup *rhs) const" << endl;
        c << "{" << endl;

        if(HaveArrays() || HaveAGVectors())
        {
            c << "    int i;" << endl << endl;
        }

        c << "    const "<<name<<" &obj = *((const "<<name<<"*)rhs);" << endl;

        // Create bool values to evaluate the arrays.
        c << "    bool retval = false;" << endl;
        c << "    switch (index_)" << endl;
        c << "    {" << endl;

        // Create a big boolean return statement.
        for (i=0; i<fields.size(); i++)
        {
            c << "    case "<<fields[i]->FieldID()<<":" << endl;
            c << "        {  // new scope" << endl;
            fields[i]->WriteSourceComparisonPrecalc(c, "        ");
            c << "        retval = ";
            fields[i]->WriteSourceComparison(c);
            c << ";" << endl << "        }" << endl;
            c << "        break;" << endl;
        }
        c << "    default: retval = false;" << endl;
        c << "    }" << endl << endl;
        c << "    return retval;" << endl;
        c << "}" << endl << endl;
    }

    void WriteSourceEnumConversions(ostream &c)
    {
        for(int i = 0; i < EnumType::enums.size(); ++i)
        {
            c << "//" << endl;
            c << "// Enum conversion methods for "<<name << "::" << EnumType::enums[i]->type << endl;
            c << "//" << endl;
            c << endl;
            c << "static const char *" << EnumType::enums[i]->type << "_strings[] = {" << endl;
            for(int j = 0; j < EnumType::enums[i]->values.size(); ++j)
            {
                c << "\"" << EnumType::enums[i]->values[j] << "\"";
                if(j < EnumType::enums[i]->values.size() - 1)
                    c << ", ";
                if(((j+1) % 3) == 0)
                    c << endl;
            }
            c << "};" << endl << endl;

            c << "std::string"<< endl;
            c << name << "::" << EnumType::enums[i]->type<<"_ToString("<<name<<"::"<<EnumType::enums[i]->type<<" t)" << endl;
            c << "{" << endl;
            c << "    int index = int(t);" << endl;
            c << "    if(index < 0 || index >= " << EnumType::enums[i]->values.size() << ") index = 0;" << endl;
            c << "    return " << EnumType::enums[i]->type << "_strings[index];" << endl;
            c << "}" << endl << endl;

            c << "std::string"<< endl;
            c << name << "::" << EnumType::enums[i]->type<<"_ToString(int t)" << endl;
            c << "{" << endl;
            c << "    int index = (t < 0 || t >= " << EnumType::enums[i]->values.size() << ") ? 0 : t;" << endl;
            c << "    return " << EnumType::enums[i]->type << "_strings[index];" << endl;
            c << "}" << endl << endl;

            c << "bool" << endl;
            c << name << "::"<< EnumType::enums[i]->type<<"_FromString(const std::string &s, "<<name<<"::"<<EnumType::enums[i]->type<<" &val)" << endl;
            c << "{" << endl;
            if(EnumType::enums[i]->values.size() > 0)
                c << "    val = " << name << "::" << EnumType::enums[i]->values[0] << ";" << endl;
            c << "    for(int i = 0; i < "<<EnumType::enums[i]->values.size()<<"; ++i)" << endl;
            c << "    {" << endl;
            c << "        if(s == "<< EnumType::enums[i]->type<<"_strings[i])" << endl;
            c << "        {" << endl;
            c << "            val = ("<< EnumType::enums[i]->type<<")i;" << endl;
            c << "            return true;" << endl;
            c << "        }" << endl;
            c << "    }" << endl;
            c << "    return false;" << endl;
            c << "}" << endl << endl;
        }
    }

    void WriteSource(ostream &c)
    {
        int i;
        c << copyright_str.c_str() << endl;
        c << "#include <" << name << ".h>" << endl;
        WriteSourceIncludes(c);

        if (!constants.empty())
        {
            for (i=0; i<constants.size(); i++)
            {
                if (!constants[i]->def.simplifyWhiteSpace().isEmpty())
                {
                    c << constants[i]->def;
                    c << endl;
                }
            }
        }

        // Write out enum conversions.
        WriteSourceEnumConversions(c);

        // Write the more complex methods.
        WriteSourceConstructor(c);
        WriteSourceCopyConstructor(c);
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
            c << "///////////////////////////////////////////////////////////////////////////////" << endl;
            c << "// Persistence methods" << endl;
            c << "///////////////////////////////////////////////////////////////////////////////" << endl;
            c << endl;
            WriteSourceCreateNode(c);
            WriteSourceSetFromNode(c);
        }

        // Write out all the set methods
        c << "///////////////////////////////////////////////////////////////////////////////" << endl;
        c << "// Set property methods" << endl;
        c << "///////////////////////////////////////////////////////////////////////////////" << endl << endl;
        for (i=0; i<fields.size(); i++)
        {
            if(fields[i]->accessType != Field::AccessPrivate)
                continue;

            if (!HasFunction("Set"+fields[i]->Name))
            {
                fields[i]->WriteSourceSetFunction(c, name);
            }
            else
            {
                PrintFunction(c, "Set"+fields[i]->Name);
                c << endl;
            }
        }

        // Write out all the get methods
        c << "///////////////////////////////////////////////////////////////////////////////" << endl;
        c << "// Get property methods" << endl;
        c << "///////////////////////////////////////////////////////////////////////////////" << endl << endl;
        for (i=0; i<fields.size(); i++)
        {
            if(fields[i]->accessType != Field::AccessPrivate)
                continue;
            fields[i]->WriteSourceGetFunction(c, name, true);
            if (fields[i]->CanHaveConst())
                fields[i]->WriteSourceGetFunction(c, name, false);
        }

        if (SelectFunctionsNeeded())
        {
            // Write out all the select methods
            c << "///////////////////////////////////////////////////////////////////////////////" << endl;
            c << "// Select property methods" << endl;
            c << "///////////////////////////////////////////////////////////////////////////////" << endl << endl;
            for (i=0; i<fields.size(); i++)
            {
                if(fields[i]->accessType != Field::AccessPrivate)
                    continue;
                fields[i]->WriteSourceSelectFunction(c, name);
            }
        }

        if (HaveAGVectors())
        {
            c << "///////////////////////////////////////////////////////////////////////////////" << endl;
            c << "// AttributeGroupVector convenience methods." << endl;
            c << "///////////////////////////////////////////////////////////////////////////////" << endl << endl;
            for (i=0; i<fields.size(); i++)
            {
                if(fields[i]->accessType != Field::AccessPrivate)
                    continue;
                fields[i]->WriteSourceAGVectorFunctions(c, name, purpose);
            }
        }

        if (HaveSoloAGVector())
        {
            for (i=0; i<fields.size(); i++)
                fields[i]->WriteSourceSoloAGVectorFunctions(c, name, purpose);
        }

        // Write out all the keyframe methods
        if(keyframe)
        {
            c << "///////////////////////////////////////////////////////////////////////////////" << endl;
            c << "// Keyframing methods" << endl;
            c << "///////////////////////////////////////////////////////////////////////////////" << endl << endl;
            WriteSourceKeyframeFunctions(c);
        }

        c << "///////////////////////////////////////////////////////////////////////////////" << endl;
        c << "// User-defined methods." << endl;
        c << "///////////////////////////////////////////////////////////////////////////////" << endl << endl;
        for (i=0; i<functions.size(); i++)
        {
            if (functions[i]->user)
            {
                c << functions[i]->def;
                c << endl;
            }
        }

        for (i=0; i<functions.size(); i++) 
        {
            if (!functions[i]->user)
            {
                if (!functions[i]->usedThisFunction)
                {
                    cerr << "\n\n!!! WARNING !!!\n\n";
                    cerr << "You declared the function \"" << functions[i]->name
                         << "\" as replacing a builtin.  But the xml2atts\n"
                         << "program could not find a builtin to replace it "
                         << "with.  It is being ignored.\n\n"
                         << "You might want to declare it as a \"New Function"
                         << "\" instead.\n\n" << endl;
                }
            }
        }
    }
};

// ----------------------------------------------------------------------------
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
    AttsGeneratorPlugin(const QString &n,const QString &l,const QString &t,const QString &vt,const QString &dt,const QString &v, const QString &, bool,bool,bool,bool)
        : name(n), type(t), label(l), version(v), vartype(vt), dbtype(dt), atts(NULL)
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
