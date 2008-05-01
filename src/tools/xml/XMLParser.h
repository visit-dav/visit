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

#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <qxml.h>
#include <visitstream.h>
#include "Field.h"
#include "XMLParserUtil.h"
#include "Attribute.h"
#include "Plugin.h"
#include "Function.h"
#include "Constant.h"
#include "CodeFile.h"
#include "Include.h"

#include <vector>
using std::vector;

enum ComponentTypes
{
    COMP_NONE            = 0x00,
    COMP_GUI             = 0x01,
    COMP_SCRIPTING       = 0x02,
    COMP_VIEWER          = 0x04,
    COMP_MDSERVER        = 0x08,
    COMP_ENGINE          = 0x10,
    COMP_WIDGETS         = 0x20,
    COMP_VIEWER_WIDGETS  = 0x40
};

inline vector<QString>
ParseCharacters(const QString &buff)
{
    vector<QString> output;

    // split one string into a list of strings when delimited by whitespace
    // or quotation marks, e.g.   <string1  "string two"  ""  string4>
    buff.stripWhiteSpace();
    bool quote=false;
    QString tmp="";
    for (size_t i=0; i<buff.length(); i++)
    {
        if (!quote && buff[i].isSpace())
        {
            if (!tmp.isEmpty())
                output.push_back(tmp);
            tmp="";
        }
        else if (!quote && buff[i] == '"')
        {
            quote = true;
        }
        else if (quote && buff[i] == '"')
        {
            quote = false;
            output.push_back(tmp);
            tmp="";
        }
        else
        {
            tmp += buff[i];
        }
    }
    if (!tmp.isEmpty())
        output.push_back(tmp);

    return output;
}

// ****************************************************************************
//  Class:  XMLParser
//
//  Purpose:
//    Parse a plugin/attributesubject.
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 28 14:15:14 PDT 2001
//    Added vartype to a plugin.  This is so plot plugins can specify
//    whether they handle meshes, scalars, vectors, etc.
//
//    Jeremy Meredith, Fri Apr 19 15:34:19 PDT 2002
//    Major enhancements.  Added more attributes to Field, added constants,
//    includes.  Renamed and added some more stuff for the code file.
//
//    Jeremy Meredith, Wed May  8 15:29:14 PDT 2002
//    Added "persistent" tag to Attributes.
//
//    Brad Whitlock, Tue May 14 10:23:01 PDT 2002
//    Added support for exports.
//
//    Jeremy Meredith, Mon Jul 15 01:22:09 PDT 2002
//    Big enhancements so all our plugin makefile can be build automatically.
//
//    Jeremy Meredith, Fri Jul 19 17:34:21 PDT 2002
//    Added default value for tag Function attribute member (true).
//
//    Jeremy Meredith, Mon Aug 26 17:33:24 PDT 2002
//    Added database plugin support.
//
//    Jeremy Meredith, Thu Oct 17 15:57:13 PDT 2002
//    Added filepath.  Added some improvements for the XML editor.
//
//    Brad Whitlock, Mon Nov 18 14:06:14 PST 2002
//    I renamed the class to avoid naming conflicts on Windows.
//
//    Brad Whitlock, Thu Mar 13 12:23:35 PDT 2003
//    I added support for icon files.
//
//    Jeremy Meredith, Thu Mar 27 12:51:05 PST 2003
//    Stopped adding the class scope to the enum enabler values.
//    It is now added only before use in GenerateWindow.
//
//    Hank Childs, Tue Sep  9 10:04:41 PDT 2003
//    Added support for file writers.
//
//    Jeremy Meredith, Tue Sep 23 17:00:16 PDT 2003
//    Made haswriter be a true bool.
//
//    Jeremy Meredith, Wed Nov  5 13:28:03 PST 2003
//    Added ability to disable plugins by default.
//
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Jeremy Meredith, Wed Aug 25 11:50:14 PDT 2004
//    Added the concept of an engine-only or everything-but-the-engine plugin.
//
//    Hank Childs, Tue May 24 09:41:53 PDT 2005
//    Added hasoptions.
//
//    Hank Childs, Tue Sep 12 09:25:05 PDT 2006
//    Fix problem where "NULL" QStrings were getting used elsewhere in the
//    code where empty strings should be used.
//
//    Brad Whitlock, Fri Feb 23 17:09:30 PST 2007
//    Added support for viewer widgets.
//
//    Cyrus Harrison, Wed Mar  7 09:35:48 PST 2007
//    Allow for engine-specific code in a plugin's source files.
//
//    Brad Whitlock, Wed Feb 28 18:34:33 PST 2007
//    Added support for fields that should be public. Added keyframe flag.
//
//    Hank Childs, Thu Jan 10 14:51:51 PST 2008
//    Added support for explicit filenames.
//
//    Brad Whitlock, Fri Apr 25 11:23:20 PDT 2008
//    Added support for setting access type on functions.
//
// ****************************************************************************

class XMLParser : public QXmlDefaultHandler
{
  public:
    QString    docType;
    Plugin    *plugin;
    Attribute *attribute;
    QString    filepath;
  public:
    XMLParser(FieldFactory *fieldFactory_, QString filename) 
    {
        filepath = FilePath(filename);
        currentPlugin = NULL;
        plugin    = NULL;
        attribute = NULL;
        currentConstants = NULL;
        currentFunctions = NULL;
        fieldFactory = fieldFactory_;
    }
    bool startDocument()
    {
        currentTag = "";
        tagStack.push_back("xml file");
        return true;
    }
    bool characters(const QString& buff)
    {
        vector<QString> strings = ParseCharacters(buff);

        for(size_t i=0; i<strings.size(); i++)
        {
            if (currentTag == "Enum")
                currentEnum->AddValue(strings[i]);
            else if (currentTag == "Field")
                currentField->SetValue(strings[i], i);
            else if (currentTag == "Include")
                currentInclude->SetInclude(strings[i]);
            else if (currentTag == "Files")
            {
                if (currentFileComponents & COMP_GUI)
                    currentPlugin->gfiles.push_back(strings[i]);
                if (currentFileComponents & COMP_SCRIPTING)
                    currentPlugin->sfiles.push_back(strings[i]);
                if (currentFileComponents & COMP_VIEWER)
                    currentPlugin->vfiles.push_back(strings[i]);
                if (currentFileComponents & COMP_MDSERVER)
                    currentPlugin->mfiles.push_back(strings[i]);
                if (currentFileComponents & COMP_ENGINE)
                    currentPlugin->efiles.push_back(strings[i]);
                if (currentFileComponents & COMP_WIDGETS)
                    currentPlugin->wfiles.push_back(strings[i]);
                if (currentFileComponents & COMP_VIEWER_WIDGETS)
                    currentPlugin->vwfiles.push_back(strings[i]);
            }
            else if (currentTag == "CXXFLAGS")
            {
                currentPlugin->cxxflags.push_back(strings[i]);
            }
            else if (currentTag == "LDFLAGS")
            {
                currentPlugin->ldflags.push_back(strings[i]);
            }
            else if (currentTag == "LIBS")
            {
                currentPlugin->libs.push_back(strings[i]);
            }
            else if (currentTag == "Extensions")
            {
                currentPlugin->extensions.push_back(strings[i]);
            }
            else if (currentTag == "Filenames")
            {
                currentPlugin->filenames.push_back(strings[i]);
            }
        }
        return true;
    }
    bool startElement(const QString&,
                      const QString&,
                      const QString& tag,
                      const QXmlAttributes& atts)
    {
        if (currentTag == "")
            docType = tag;

        QString name    = atts.value("name");
        if (tag == "Plugin")
        {
            QString label     = atts.value("label");
            QString type      = atts.value("type");
            QString vartype   = atts.value("vartype");
            QString dbtype    = atts.value("dbtype");
            QString haswriter = atts.value("haswriter");
            QString hasoptions= atts.value("hasoptions");
            QString specifiedFilename = atts.value("specifiedFilename");
            QString version   = atts.value("version");
            QString iconFile  = atts.value("iconFile");
            QString enabled   = atts.value("enabled");
            QString mdspecific= atts.value("mdspecificcode");
            QString engspecific= atts.value("engspecificcode");
            QString onlyengine= atts.value("onlyengine");
            QString noengine  = atts.value("noengine");
            currentPlugin = new Plugin(name, label, type, vartype,
                                       dbtype, version, iconFile, 
                                       haswriter.isNull() ? false : Text2Bool(haswriter),
                                       hasoptions.isNull() ? false : Text2Bool(hasoptions),
                                       onlyengine.isNull() ? false : Text2Bool(onlyengine),
                                       noengine.isNull() ? false : Text2Bool(noengine));
            if (!enabled.isNull())
            {
                currentPlugin->enabledByDefault = Text2Bool(enabled);
            }
            if (!mdspecific.isNull())
            {
                currentPlugin->has_MDS_specific_code = Text2Bool(mdspecific);
            }
            if (!engspecific.isNull())
            {
                currentPlugin->hasEngineSpecificCode = Text2Bool(engspecific);
            }
            if (!specifiedFilename.isNull())
            {
                currentPlugin->specifiedFilenames = Text2Bool(specifiedFilename);
            }
        }
        else if (tag == "Attribute")
        {
            QString purpose       = atts.value("purpose");
            QString codefile      = atts.value("codefile");
            QString persistent    = atts.value("persistent");
            QString keyframe      = atts.value("keyframe");
            QString exportAPI     = atts.value("exportAPI");
            if (exportAPI.isNull())
                exportAPI = "";
            QString exportInclude = atts.value("exportInclude");
            if (exportInclude.isNull())
                exportInclude = "";
            if (!filepath.isNull() && !codefile.isNull())
                codefile = filepath + codefile;
            currentAttribute = new Attribute(name, purpose, codefile,
                                             exportAPI, exportInclude);
            if (persistent.isNull())
                currentAttribute->persistent = true;
            else
                currentAttribute->persistent = Text2Bool(persistent);

            if (keyframe.isNull())
                currentAttribute->keyframe = true;
            else
                currentAttribute->keyframe = Text2Bool(keyframe);

            if (currentAttribute->codeFile)
            {
                QStringList targets, names, first, second;
                currentAttribute->codeFile->GetAllCodes(targets, names, first, second);
                for(size_t i = 0; i < targets.size(); ++i)
                {
                    currentAttribute->codes.push_back(new Code(names[i],
                                                               first[i],
                                                               second[i],
                                                               targets[i]));
                }
            }
        }
        else if (tag == "Enum")
        {
            currentEnum = new EnumType(name);
        }
        else if (tag == "Include")
        {
            QString file   = atts.value("file");
            QString quoted = atts.value("quoted");
            QString target = atts.value("target");
            bool    quote = false;
            if (!quoted.isNull()) 
                quote = Text2Bool(quoted);
            if (target.isNull()) 
                target = "xml2atts";

            currentInclude = new Include(file, quote, target);
        }
        else if (tag == "Constant")
        {
            QString member = atts.value("member");

            if (!currentAttribute)
                throw QString().sprintf("No current attribute when specifying constant %s",name.latin1());
            CodeFile *codeFile = currentAttribute->codeFile;
            if (!codeFile)
                throw QString().sprintf("No codefile found for constant %s.",name.latin1());
            if (!codeFile->HasConstant(name))
                throw QString().sprintf("no constant %s in codefile.", name.latin1());

            // Get the constant definitions
            QStringList targets, first, second;
            codeFile->GetConstant(name, targets, first, second);
            currentConstants = new Constant *[targets.size() + 1];
            for(size_t i = 0; i < targets.size(); ++i)
            {
                currentConstants[i] = new Constant(name,
                                                   first[i],
                                                   second[i],
                                                   Text2Bool(member),
                                                   targets[i]);
            }
            currentConstants[targets.size()] = NULL;
        }
        else if (tag == "Function")
        {
            QString user   = atts.value("user");
            QString member = atts.value("member");
            if (member.isNull())
                member = "true";
            QString access = atts.value("access");
            Function::AccessType a;
            if(access == "protected")
                a = Function::AccessProtected;
            else if(access == "private")
                a = Function::AccessPrivate;
            else
                a = Function::AccessPublic;

            if (!currentAttribute)
                throw QString().sprintf("No current attribute when specifying function %s",name.latin1());
            CodeFile *codeFile = currentAttribute->codeFile;
            if (!codeFile)
                throw QString().sprintf("No codefile found for function %s.",name.latin1());
            if (!codeFile->HasFunction(name))
                throw QString().sprintf("no function %s in codefile.",name.latin1());

            // Get the function definitions
            QStringList targets, first, second;
            codeFile->GetFunction(name, targets, first, second);
            currentFunctions = new Function*[targets.size()+1];
            for(size_t i = 0; i < targets.size(); ++i)
            {
                currentFunctions[i] = new Function(name,
                                                   first[i],
                                                   second[i],
                                                   Text2Bool(user), Text2Bool(member),
                                                   targets[i]);
                currentFunctions[i]->accessType = a;
            }
            currentFunctions[targets.size()] = NULL;
        }
        else if (tag == "CXXFLAGS")
        {
        }
        else if (tag == "LDFLAGS")
        {
        }
        else if (tag == "LIBS")
        {
        }
        else if (tag == "Extensions")
        {
        }
        else if (tag == "Filenames")
        {
        }
        else if (tag == "Files")
        {
            QString         comps1 = atts.value("components");
            vector<QString> comps2 = SplitValues(comps1);
            int             comps3 = COMP_NONE;

            for (size_t i=0; i<comps2.size(); i++)
            {
                if (comps2[i] == "G")
                {
                    currentPlugin->gfiles.clear();
                    comps3 |= COMP_GUI;
                    currentPlugin->customgfiles = true;
                }
                else if (comps2[i] == "S")
                {
                    currentPlugin->sfiles.clear();
                    comps3 |= COMP_SCRIPTING;
                    currentPlugin->customsfiles = true;
                }
                else if (comps2[i] == "V")
                {
                    currentPlugin->vfiles.clear();
                    comps3 |= COMP_VIEWER;
                    currentPlugin->customvfiles = true;
                }
                else if (comps2[i] == "M")
                {
                    currentPlugin->mfiles.clear();
                    comps3 |= COMP_MDSERVER;
                    currentPlugin->custommfiles = true;
                }
                else if (comps2[i] == "E")
                {
                    currentPlugin->efiles.clear();
                    comps3 |= COMP_ENGINE;
                    currentPlugin->customefiles = true;
                }
                else if (comps2[i] == "W")
                {
                    currentPlugin->wfiles.clear();
                    comps3 |= COMP_WIDGETS;
                    currentPlugin->customwfiles = true;
                }
                else if (comps2[i] == "VW")
                {
                    currentPlugin->vwfiles.clear();
                    comps3 |= COMP_VIEWER_WIDGETS;
                    currentPlugin->customvwfiles = true;
                }
                else
                    throw QString().sprintf("invalid file '%s' for components attribute of Files tag", comps2[i].latin1());
            }
            currentFileComponents = comps3;
        }
        else if (tag == "Field")
        {
            if (!currentAttribute)
                throw QString().sprintf("No current attribute when specifying field %s",name.latin1());

            QString type      = atts.value("type");
            QString subtype   = atts.value("subtype");
            QString length    = atts.value("length");
            QString label     = atts.value("label");

            currentField = fieldFactory->createField(name,type,subtype,length,label);
            currentField->codeFile = currentAttribute->codeFile;

            QString enabler   = atts.value("enabler");
            QString internal  = atts.value("internal");

            QString ignoreeq  = atts.value("ignoreeq");
            if (!ignoreeq.isNull())
            {
                currentField->ignoreEquality = Text2Bool(ignoreeq);
            }

            QString access    = atts.value("access");
            if (!access.isNull())
            {
                if(access == "public")
                    currentField->SetPublicAccess();
                else if(access == "protected")
                    currentField->SetProtectedAccess();
                else
                    currentField->SetPrivateAccess();
            }
            else
                currentField->SetPrivateAccess();

            QString init      = atts.value("init");
            if (!init.isNull())
            {
                if (!currentAttribute->codeFile)
                    throw QString().sprintf("No codefile found for initializer for %s.",name.latin1());
                if (!currentAttribute->codeFile->HasInit(name))
                    throw QString().sprintf("no initializer for %s in codefile.",name.latin1());

                QStringList targets, defs;
                currentAttribute->codeFile->GetInit(name, targets, defs);
                for(size_t i = 0; i < targets.size(); ++i)
                    currentField->SetInitCode(targets[i], defs[i]);
            }

            if (!enabler.isNull())
            {
                vector<QString> vals = SplitValues(enabler);
                if (vals.size() < 2)
                    throw QString().sprintf("enabler for field %s requires a value",name.latin1());

                Field *enablerField = NULL;
                for (size_t i=0; i<currentAttribute->fields.size(); i++)
                {
                    if (currentAttribute->fields[i]->name == vals[0])
                        enablerField = currentAttribute->fields[i];
                }

                if (!enablerField)
                    throw QString().sprintf("enabler field %s was not defined before current field %s",vals[0].latin1(),name.latin1());

                currentField->SetEnabler(enablerField);
                for (size_t i=1; i<vals.size(); i++)
                {
                    currentField->SetEnablerValue(vals[i]);
                }
            }

            if (!internal.isNull())
                currentField->SetInternal(internal);

            for (int j=0; j<atts.length(); j++)
            {
                if (atts.qName(j) != "name"    &&
                    atts.qName(j) != "type"    &&
                    atts.qName(j) != "subtype" &&
                    atts.qName(j) != "label"   &&
                    atts.qName(j) != "length"  &&
                    atts.qName(j) != "enabler" &&
                    atts.qName(j) != "internal"&&
                    atts.qName(j) != "ignoreeq"&&
                    atts.qName(j) != "access"  &&
                    atts.qName(j) != "init"    )
                {
                    currentField->SetAttrib(atts.qName(j),atts.value(j));
                }
            }
        }
        else
        {
            throw QString().sprintf("unknown tag for startElement: %s",tag.latin1());
        }
        currentTag = tag;
        tagStack.push_back(currentTag);

        return true;
    }
    bool endElement( const QString&, const QString&, const QString &tag)
    {
        if (tagStack.back() != tag)
            throw QString().sprintf("ending tag (%s) does not match latest tag started (%s)", tagStack.back().latin1(), tag.latin1());
        tagStack.pop_back();

        if (tag == "Attribute")
        {
            attribute = currentAttribute;
            if (currentPlugin)
            {
                currentPlugin->atts = currentAttribute;
                currentAttribute = NULL;
            }
        }
        else if (tag == "Field")
        {
            currentField->index = currentAttribute->fields.size();
            currentAttribute->fields.push_back(currentField);
            currentField = NULL;
        }
        else if (tag == "Enum")
        {
            EnumType::enums.push_back(currentEnum);
            currentEnum = NULL;
        }
        else if (tag == "Include")
        {
            currentAttribute->includes.push_back(currentInclude);
            currentInclude = NULL;
        }
        else if (tag == "Plugin")
        {
            plugin = currentPlugin;
        }
        else if (tag == "Function")
        {
            for(Function **f = currentFunctions; *f != NULL; ++f)
                currentAttribute->functions.push_back(*f);
            delete [] currentFunctions;
            currentFunctions = NULL;
        }
        else if (tag == "CXXFLAGS")
        {
        }
        else if (tag == "LDFLAGS")
        {
        }
        else if (tag == "LIBS")
        {
        }
        else if (tag == "Extensions")
        {
        }
        else if (tag == "Filenames")
        {
        }
        else if (tag == "Files")
        {
            currentFileComponents = COMP_NONE;
        }
        else if (tag == "Constant")
        {
            for(Constant **c = currentConstants; *c != NULL; ++c)
                currentAttribute->constants.push_back(*c);
            delete [] currentConstants;
            currentConstants = NULL;
        }
        else
        {
            throw QString().sprintf("unknown tag for endElement: %s",tag.latin1());
        }

        currentTag = tagStack.back();
        return true;
    }
  private:
    int             currentFileComponents;
    Include        *currentInclude;
    Constant      **currentConstants;
    Function      **currentFunctions;
    Field          *currentField;
    EnumType       *currentEnum;
    Attribute      *currentAttribute;
    Plugin         *currentPlugin;
    QString         currentTag;
    vector<QString> tagStack;
    FieldFactory   *fieldFactory;
};

#endif
