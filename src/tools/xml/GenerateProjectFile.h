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

#ifndef GENERATE_PROJECTIFLE_H
#define GENERATE_PROJECTFILE_H

#include "Field.h"
#include <visit-config.h> // for the plugin extension.

#if defined(_WIN32)
#include <windows.h>
#endif

// ****************************************************************************
//  File:  GenerateProjectFile
//
//  Purpose:
//    Creates project files for plugin.
//
//  Note: This file overrides --
//    Plugin
//
//  Programmer:  Brad Whitlock
//  Creation:    Mon Dec 15 14:14:13 PST 2003
//
//  Modifications:
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Brad Whitlock, Wed Jul 14 10:54:35 PDT 2004
//    Changed the registry stuff so it gets the keys from the right place. I
//    also fixed a bunch of bugs with the project files generated for
//    mdserver and engine plugins.
//
//    Brad Whitlock, Thu Aug 12 17:43:51 PST 2004
//    I added code to generate MSVC7.Net project files in addition to the
//    regular version 6.0 project files.
//
//    Jeremy Meredith, Wed Aug 25 11:57:08 PDT 2004
//    Added the concept of an engine-only or everything-but-the-engine plugin.
//
//    Brad Whitlock, Wed Sep 22 16:11:54 PST 2004
//    I fixed a bug with how some database plugins were named. I also fixed
//    some minor errors that caused confusing build failures when building
//    in Debug mode with MSVC6.0.
//
//    Hank Childs, Tue May 24 09:41:53 PDT 2005
//    Added hasoptions.
//
//    Brad Whitlock, Thu Aug 18 11:17:29 PDT 2005
//    Prevented Windows directory names when not on Windows.
//
//    Brad Whitlock, Wed Jun 14 15:32:44 PST 2006
//    Added vtkFiltering.lib to the list of libs for database plugins.
//
//    Brad Whitlock, Fri Feb 23 17:18:42 PST 2007
//    Added viewer widgets.
//
//    Cyrus Harrison, Wed Mar  7 09:50:01 PST 2007
//    Allow for engine-specific code in a plugin's source files.
//
//    Kathleen Bonnell, Wed Oct 10 12:08:03 PDT 2007 
//    Change Python include to version 2.5.  Changed %QTDIR% to $(QTDIR),
//    and qt-mt302.lib to $(QTLIB).
//
// ****************************************************************************

class ProjectFileGeneratorPlugin
{
  public:
    QString name;
    QString type;
    QString label;
    QString version;
    QString vartype;
    QString dbtype;
    bool    haswriter;
    bool    hasoptions;
    bool    enabledByDefault;
    bool    has_MDS_specific_code;
    bool    hasEngineSpecificCode;
    bool    onlyEnginePlugin;
    bool    noEnginePlugin;

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
    bool customvwfiles;
    vector<QString> vwfiles;    // viewer widgets
    vector<QString> defaultgfiles;
    vector<QString> defaultsfiles;
    vector<QString> defaultvfiles;
    vector<QString> defaultmfiles;
    vector<QString> defaultefiles;
    vector<QString> defaultwfiles;

    Attribute *atts;
  public:
    ProjectFileGeneratorPlugin(const QString &n,const QString &l,
             const QString &t,const QString &vt,const QString &dt,
             const QString &v, const QString&w, bool hw, bool ho, 
             bool onlyengine, bool noengine)
        : name(n), type(t), label(l), version(v), vartype(vt), dbtype(dt),
          haswriter(hw), hasoptions(ho), onlyEnginePlugin(onlyengine), 
          noEnginePlugin(noengine), atts(NULL)
    {
        enabledByDefault = true;
        customgfiles = false;
        customsfiles = false;
        customvfiles = false;
        custommfiles = false;
        customefiles = false;
        customwfiles = false;
        customvwfiles = false;
        gfiles.clear();
        sfiles.clear();
        vfiles.clear();
        mfiles.clear();
        efiles.clear();
        wfiles.clear();
        vwfiles.clear();
        if (type == "database")
        {
            QString filter = QString("avt") + name + "FileFormat.C";
            defaultmfiles.push_back(filter);
            defaultefiles.push_back(filter);
        }
        else if (type == "plot")
        {
            QString filter = QString("avt") + name + "Filter.C";
            defaultvfiles.push_back(filter);
            defaultefiles.push_back(filter);
            QString widgets = QString("Qvis") + name + "PlotWindow.h";
            defaultwfiles.push_back(widgets);
        }
        else if (type == "operator")
        {
            QString filter = QString("avt") + name + "Filter.C";
            defaultvfiles.push_back(filter);
            defaultefiles.push_back(filter);
        }
    }

    void Print(ostream &out)
    {
        out << "Plugin: "<<name<<" (\""<<label<<"\", type="<<type<<") -- version "<<version<< endl;
        if (atts)
            atts->Print(cout);
    }

protected:

#if defined(_WIN32)
    bool ReadKey(const char *key, unsigned char **keyval) const
    {
        bool retval = false;
        *keyval = 0;

        if((retval = ReadKeyFromRoot(HKEY_CLASSES_ROOT, key, keyval)) == false)
            retval = ReadKeyFromRoot(HKEY_CURRENT_USER, key, keyval);

        return retval;
    }

    bool ReadKeyFromRoot(HKEY which_root, const char *key, unsigned char **keyval) const
    {
        bool  readSuccess = false;
        QString regkey;
        HKEY hkey;

        /* Try and read the key from the system registry. */
        regkey.sprintf("VISIT%s", VERSION);
        *keyval = new unsigned char[500];
        if(RegOpenKeyEx(which_root, regkey.latin1(), 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
        {
            DWORD keyType, strSize = 500;
            if(RegQueryValueEx(hkey, key, NULL, &keyType, *keyval, &strSize) == ERROR_SUCCESS)
            {
                readSuccess = true;
            }

            RegCloseKey(hkey);
        }

        if(!readSuccess)
        {
            delete [] *keyval;
            *keyval = 0;
        }

        return readSuccess;
    }
#endif

    QString ProjectDir() const
    {
#if defined(_WIN32)
        unsigned char *VISITDEVDIR = 0;
        QString retval;
        
        if(ReadKey("VISITDEVDIR", &VISITDEVDIR))
        {
            retval = QString((char *)VISITDEVDIR);
            delete [] VISITDEVDIR;
        }
        else
        {
            const char *visitDevDirEnv = getenv("VISITDEVDIR");
            if (!visitDevDirEnv)
            {
                throw QString().sprintf("Please set the VISITDEVDIR environment variable to "
                    "C:\\VisItDev%s or wherever you installed the VisIt source code.", VERSION);
            }
            else
                retval = visitDevDirEnv;
        }

        // Make sure that it ends with a separator.
        if(retval.length() > 0 && retval.right(1) != SLASH_STRING)
        {
            retval += SLASH_STRING;
        }

        return retval;
#else
        return QString("");
#endif
    }

    void AddElements(vector<QString> &addTo, const vector<QString> &addFrom)
    {
        for(int i = 0; i < addFrom.size(); ++i)
            addTo.push_back(addFrom[i]);
    }

    void WriteProject_TOP_LEVEL_Version6(ostream &out)
    {
        out << "# Microsoft Developer Studio Project File - Name=\"" << name << "\" - Package Owner=<4>" << endl;
        out << "# Microsoft Developer Studio Generated Build File, Format Version 6.00" << endl;
        out << "# ** DO NOT EDIT **" << endl;
        out << "" << endl;
        out << "# TARGTYPE \"Win32 (x86) Generic Project\" 0x010a" << endl;
        out << "" << endl;
        out << "CFG=" << name << " - Win32 Debug" << endl;
        out << "!MESSAGE This is not a valid makefile. To build this project using NMAKE," << endl;
        out << "!MESSAGE use the Export Makefile command and run" << endl;
        out << "!MESSAGE " << endl;
        out << "!MESSAGE NMAKE /f \"" << name << ".mak\"." << endl;
        out << "!MESSAGE " << endl;
        out << "!MESSAGE You can specify a configuration when running NMAKE" << endl;
        out << "!MESSAGE by defining the macro CFG on the command line. For example:" << endl;
        out << "!MESSAGE " << endl;
        out << "!MESSAGE NMAKE /f \"" << name << ".mak\" CFG=\"" << name << " - Win32 Debug\"" << endl;
        out << "!MESSAGE " << endl;
        out << "!MESSAGE Possible choices for configuration are:" << endl;
        out << "!MESSAGE " << endl;
        out << "!MESSAGE \"" << name << " - Win32 Release\" (based on \"Win32 (x86) Generic Project\")" << endl;
        out << "!MESSAGE \"" << name << " - Win32 Debug\" (based on \"Win32 (x86) Generic Project\")" << endl;
        out << "!MESSAGE " << endl;
        out << "" << endl;
        out << "# Begin Project" << endl;
        out << "# PROP AllowPerConfigDependencies 0" << endl;
        out << "# PROP Scc_ProjName \"\"" << endl;
        out << "# PROP Scc_LocalPath \"\"" << endl;
        out << "MTL=midl.exe" << endl;
        out << "" << endl;
        out << "!IF  \"$(CFG)\" == \"" << name << " - Win32 Release\"" << endl;
        out << "" << endl;
        out << "# PROP BASE Use_MFC 0" << endl;
        out << "# PROP BASE Use_Debug_Libraries 0" << endl;
        out << "# PROP BASE Output_Dir \"Release\"" << endl;
        out << "# PROP BASE Intermediate_Dir \"Release\"" << endl;
        out << "# PROP BASE Target_Dir \"\"" << endl;
        out << "# PROP Use_MFC 0" << endl;
        out << "# PROP Use_Debug_Libraries 0" << endl;
        out << "# PROP Output_Dir \"Release\"" << endl;
        out << "# PROP Intermediate_Dir \"Release\"" << endl;
        out << "# PROP Target_Dir \"\"" << endl;
        out << "" << endl;
        out << "!ELSEIF  \"$(CFG)\" == \"" << name << " - Win32 Debug\"" << endl;
        out << "" << endl;
        out << "# PROP BASE Use_MFC 0" << endl;
        out << "# PROP BASE Use_Debug_Libraries 1" << endl;
        out << "# PROP BASE Output_Dir \"Debug\"" << endl;
        out << "# PROP BASE Intermediate_Dir \"Debug\"" << endl;
        out << "# PROP BASE Target_Dir \"\"" << endl;
        out << "# PROP Use_MFC 0" << endl;
        out << "# PROP Use_Debug_Libraries 1" << endl;
        out << "# PROP Output_Dir \"Debug\"" << endl;
        out << "# PROP Intermediate_Dir \"Debug\"" << endl;
        out << "# PROP Target_Dir \"\"" << endl;
        out << "" << endl;
        out << "!ENDIF " << endl;
        out << "" << endl;
        out << "# Begin Target" << endl;
        out << "" << endl;
        out << "# Name \"" << name << " - Win32 Release\"" << endl;
        out << "# Name \"" << name << " - Win32 Debug\"" << endl;
        out << "# End Target" << endl;
        out << "# End Project" << endl;
    }

    void WriteProject_TOP_LEVEL_Version7(ostream &out)
    {
        out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << endl;
        out << "<VisualStudioProject" << endl;
        out << "\tProjectType=\"Visual C++\"" << endl;
        out << "\tVersion=\"7.10\"" << endl;
        out << "\tName=\"" << name << "\"" << endl;
        out << "\tSccProjectName=\"\"" << endl;
        out << "\tSccLocalPath=\"\"" << endl;
        out << "\tKeyword=\"MakeFileProj\">" << endl;
        out << "\t<Platforms>" << endl;
        out << "\t\t<Platform" << endl;
        out << "\t\t\tName=\"Win32\"/>" << endl;
        out << "\t</Platforms>" << endl;
        out << "\t<Configurations>" << endl;
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Debug|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Debug\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Debug\"" << endl;
        out << "\t\t\tConfigurationType=\"10\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Debug\\" << name << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        out << "\t\t</Configuration>" << endl;
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Release|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Release\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Release\"" << endl;
        out << "\t\t\tConfigurationType=\"10\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Release\\" << name << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        out << "\t\t</Configuration>" << endl;
        out << "\t</Configurations>" << endl;
        out << "\t<References>" << endl;
        out << "\t</References>" << endl;
        out << "\t<Files>" << endl;
        out << "\t</Files>" << endl;
        out << "\t<Globals>" << endl;
        out << "\t</Globals>" << endl;
        out << "</VisualStudioProject>" << endl;
    }

    void WriteProject_TOP_LEVEL(ostream &out, bool version7)
    {
        if(version7)
            WriteProject_TOP_LEVEL_Version7(out);
        else
            WriteProject_TOP_LEVEL_Version6(out);
    }

    void WriteProjectWorkspace_Version6(ostream &out, const vector<QString> &projects)
    {
        out << "Microsoft Developer Studio Workspace File, Format Version 6.00" << endl;
        out << "# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!" << endl;
        out << "" << endl;
        out << "###############################################################################" << endl;
        out << "" << endl;
        out << "Project: \"" << name << "\"=.\\"<<name<<".dsp - Package Owner=<4>" << endl;
        out << "" << endl;
        out << "Package=<5>" << endl;
        out << "{{{" << endl;
        out << "}}}" << endl;
        out << "" << endl;
        out << "Package=<4>" << endl;
        out << "{{{" << endl;

        int i;
        for(i = 1; i < projects.size(); ++i)
        {
            out << "    Begin Project Dependency" << endl;
            out << "    Project_Dep_Name " << projects[i] << endl;
            out << "    End Project Dependency" << endl;
        }

        out << "}}}" << endl;
        out << "" << endl;

        for(i = 1; i < projects.size(); ++i)
        {
            out << "###############################################################################" << endl;
            out << "" << endl;
            out << "Project: \"" << projects[i] << "\"=.\\"<<projects[i]<<".dsp - Package Owner=<4>" << endl;
            out << "" << endl;
            out << "Package=<5>" << endl;
            out << "{{{" << endl;
            out << "}}}" << endl;
            out << "" << endl;
            out << "Package=<4>" << endl;
            out << "{{{" << endl;
            out << "}}}" << endl;
            out << "" << endl;
        }

        out << "###############################################################################" << endl;
        out << "" << endl;
        out << "Global:" << endl;
        out << "" << endl;
        out << "Package=<5>" << endl;
        out << "{{{" << endl;
        out << "}}}" << endl;
        out << "" << endl;
        out << "Package=<3>" << endl;
        out << "{{{" << endl;
        out << "}}}" << endl;
        out << "" << endl;
        out << "###############################################################################" << endl;
        out << "" << endl;
    }

    QString CreateKey() const
    {
        const char *digits = "0123456789ABCDEF";
        QString s("------------------------------------");
        for(int i = 0; i < s.length(); ++i)
        {
            if(i == 8 || i == 13 || i == 18 || i == 23)
                continue;
            else
            {
                int v = rand() % 16;
                s[i] = digits[v];
            }
        }

        return QString("{") + s + QString("}");
    }

    void WriteProjectWorkspace_Version7(ostream &out, const vector<QString> &projects)
    {
        QString solutionKey("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}");
        vector<QString> keys;

        int i;
        for(i = 0; i < projects.size(); ++i)
            keys.push_back(CreateKey());

        out << "Microsoft Visual Studio Solution File, Format Version 8.00" << endl;
        out << "Project(\"" << solutionKey << "\") = \"" << projects[0] << "\", \"" << projects[0] << ".vcproj\", \"" << keys[0] << "\"" << endl;
        out << "\tProjectSection(ProjectDependencies) = postProject" << endl;
        for(i = 1; i < keys.size(); ++i)
            out << "\t\t" << keys[i] << " = " << keys[i] << endl;

        out << "\tEndProjectSection" << endl;
        out << "EndProject" << endl;

        for(i = 1; i < keys.size(); ++i)
        {
            out << "Project(\"" << solutionKey << "\") = \"" << projects[i] << "\", \"" << projects[i] << ".vcproj\", \"" << keys[i] << "\"" << endl;
            out << "\tProjectSection(ProjectDependencies) = postProject" << endl;
            out << "\tEndProjectSection" << endl;
            out << "EndProject" << endl;
        }

        out << "Global" << endl;
        out << "\tGlobalSection(SolutionConfiguration) = preSolution" << endl;
        out << "\t\tDebug = Debug" << endl;
        out << "\t\tRelease = Release" << endl;
        out << "\tEndGlobalSection" << endl;
        out << "\tGlobalSection(ProjectConfiguration) = postSolution" << endl;
    
        for(i = 0; i < keys.size(); ++i)
        {
            out << "\t\t" << keys[i] << ".Debug.ActiveCfg = Debug|Win32" << endl;
            out << "\t\t" << keys[i] << ".Debug.Build.0 = Debug|Win32" << endl;
            out << "\t\t" << keys[i] << ".Release.ActiveCfg = Release|Win32" << endl;
            out << "\t\t" << keys[i] << ".Release.Build.0 = Release|Win32" << endl;
        }

        out << "\tEndGlobalSection" << endl;
        out << "\tGlobalSection(ExtensibilityGlobals) = postSolution" << endl;
        out << "\tEndGlobalSection" << endl;
        out << "\tGlobalSection(ExtensibilityAddIns) = postSolution" << endl;
        out << "\tEndGlobalSection" << endl;
        out << "EndGlobal" << endl;
    }

    void WriteProjectWorkspace(ostream &out, const vector<QString> &projects, bool version7)
    {
        if(version7)
            WriteProjectWorkspace_Version7(out, projects);
        else
            WriteProjectWorkspace_Version6(out, projects);
    }

    void WriteProjectHelper_Version6(ostream &out, const QString &pluginType, char pluginComponent,
        const QString &exports, const QString &libs, const vector<QString> &srcFiles)
    {
        const char *suffix = (pluginComponent == 'E') ? "_ser" : "";

        out << "# Microsoft Developer Studio Project File - Name=\"" << name << pluginComponent << "\" - Package Owner=<4>" << endl;
        out << "# Microsoft Developer Studio Generated Build File, Format Version 6.00" << endl;
        out << "# ** DO NOT EDIT **" << endl;
        out << "" << endl;
        out << "# TARGTYPE \"Win32 (x86) Dynamic-Link Library\" 0x0102" << endl;
        out << "" << endl;
        out << "CFG=" << name << pluginComponent << " - Win32 Debug" << endl;
        out << "!MESSAGE This is not a valid makefile. To build this project using NMAKE," << endl;
        out << "!MESSAGE use the Export Makefile command and run" << endl;
        out << "!MESSAGE " << endl;
        out << "!MESSAGE NMAKE /f \"" << name << pluginComponent << ".mak\"." << endl;
        out << "!MESSAGE " << endl;
        out << "!MESSAGE You can specify a configuration when running NMAKE" << endl;
        out << "!MESSAGE by defining the macro CFG on the command line. For example:" << endl;
        out << "!MESSAGE " << endl;
        out << "!MESSAGE NMAKE /f \"" << name << pluginComponent << ".mak\" CFG=\"" << name << pluginComponent << " - Win32 Debug\"" << endl;
        out << "!MESSAGE " << endl;
        out << "!MESSAGE Possible choices for configuration are:" << endl;
        out << "!MESSAGE " << endl;
        out << "!MESSAGE \"" << name << pluginComponent << " - Win32 Release\" (based on \"Win32 (x86) Dynamic-Link Library\")" << endl;
        out << "!MESSAGE \"" << name << pluginComponent << " - Win32 Debug\" (based on \"Win32 (x86) Dynamic-Link Library\")" << endl;
        out << "!MESSAGE " << endl;
        out << "" << endl;
        out << "# Begin Project" << endl;
        out << "# PROP AllowPerConfigDependencies 0" << endl;
        out << "# PROP Scc_ProjName \"\"" << endl;
        out << "# PROP Scc_LocalPath \"\"" << endl;
        out << "CPP=cl.exe" << endl;
        out << "MTL=midl.exe" << endl;
        out << "RSC=rc.exe" << endl;
        out << "" << endl;
        out << "!IF  \"$(CFG)\" == \"" << name << pluginComponent << " - Win32 Release\"" << endl;
        out << "" << endl;
        out << "# PROP BASE Use_MFC 0" << endl;
        out << "# PROP BASE Use_Debug_Libraries 0" << endl;
        out << "# PROP BASE Output_Dir \"Release\"" << endl;
        out << "# PROP BASE Intermediate_Dir \"Release\"" << endl;
        out << "# PROP BASE Target_Dir \"\"" << endl;
        out << "# PROP Use_MFC 0" << endl;
        out << "# PROP Use_Debug_Libraries 0" << endl;
        out << "# PROP Output_Dir \"Release\"" << endl;
        out << "# PROP Intermediate_Dir \"Release\"" << endl;
        out << "# PROP Ignore_Export_Lib 0" << endl;
        out << "# PROP Target_Dir \"\"" << endl;
        out << "# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /YX /FD /c" << endl;
        out << "# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I \"..\\..\\visit\\" << pluginType << "\\" << name << "\" /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" ";
        if(exports != "")
            out << "/D \"" << exports << "\" ";
        out << "/D \"GENERAL_PLUGIN_EXPORTS\" /YX /FD /TP /c" << endl;
        out << "# ADD BASE MTL /nologo /D \"NDEBUG\" /mktyplib203 /win32" << endl;
        out << "# ADD MTL /nologo /D \"NDEBUG\" /mktyplib203 /win32" << endl;
        out << "# ADD BASE RSC /l 0x409 /d \"NDEBUG\"" << endl;
        out << "# ADD RSC /l 0x409 /d \"NDEBUG\"" << endl;
        out << "BSC32=bscmake.exe" << endl;
        out << "# ADD BASE BSC32 /nologo" << endl;
        out << "# ADD BSC32 /nologo" << endl;
        out << "LINK32=link.exe" << endl;
        out << "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386" << endl;
        out << "# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib " << libs << " /nologo /dll /machine:I386 /out:\"Release/lib" << pluginComponent << name << suffix <<".dll\"" << endl;
        out << "# Begin Special Build Tool" << endl;
        out << "SOURCE=\"$(InputPath)\"" << endl;
        out << "PostBuild_Cmds=copy Release\\lib" << pluginComponent << name << suffix <<".dll ..\\..\\bin\\Release\\" << pluginType << "" << endl;
        out << "# End Special Build Tool" << endl;
        out << "" << endl;
        out << "!ELSEIF  \"$(CFG)\" == \"" << name << pluginComponent << " - Win32 Debug\"" << endl;
        out << "" << endl;
        out << "# PROP BASE Use_MFC 0" << endl;
        out << "# PROP BASE Use_Debug_Libraries 1" << endl;
        out << "# PROP BASE Output_Dir \"Debug\"" << endl;
        out << "# PROP BASE Intermediate_Dir \"Debug\"" << endl;
        out << "# PROP BASE Target_Dir \"\"" << endl;
        out << "# PROP Use_MFC 0" << endl;
        out << "# PROP Use_Debug_Libraries 1" << endl;
        out << "# PROP Output_Dir \"Debug\"" << endl;
        out << "# PROP Intermediate_Dir \"Debug\"" << endl;
        out << "# PROP Ignore_Export_Lib 0" << endl;
        out << "# PROP Target_Dir \"\"" << endl;
        QString debugDef("/D \"_DEBUG\" ");
        if(pluginComponent == 'S')
            debugDef = "";
        out << "# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D \"WIN32\" " << debugDef<< "/D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /YX /FD /GZ /c" << endl;
        out << "# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I \"..\\..\\visit\\" << pluginType << "\\" << name << "\" /D \"WIN32\" " << debugDef << "/D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" ";
        if(exports != "")
            out << "/D \"" << exports << "\" ";
        out << "/D \"GENERAL_PLUGIN_EXPORTS\" ";
        out << "/YX /FD /GZ /TP /c" << endl;
        out << "# ADD BASE MTL /nologo /D \"_DEBUG\" /mktyplib203 /win32" << endl;
        out << "# ADD MTL /nologo /D \"_DEBUG\" /mktyplib203 /win32" << endl;
        out << "# ADD BASE RSC /l 0x409 /d \"_DEBUG\"" << endl;
        out << "# ADD RSC /l 0x409 /d \"_DEBUG\"" << endl;
        out << "BSC32=bscmake.exe" << endl;
        out << "# ADD BASE BSC32 /nologo" << endl;
        out << "# ADD BSC32 /nologo" << endl;
        out << "LINK32=link.exe" << endl;
        out << "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept" << endl;
        out << "# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib " << libs << " /nologo /dll /debug /machine:I386 /out:\"Debug/lib" << pluginComponent << name << suffix << ".dll\" /pdbtype:sept" << endl;
        out << "# Begin Special Build Tool" << endl;
        out << "SOURCE=\"$(InputPath)\"" << endl;
        out << "PostBuild_Cmds=copy Debug\\lib" << pluginComponent << name << suffix << ".dll ..\\..\\bin\\Debug\\" << pluginType << endl;
        out << "# End Special Build Tool" << endl;
        out << "" << endl;
        out << "!ENDIF " << endl;
        out << "" << endl;
        out << "# Begin Target" << endl;
        out << "" << endl;
        out << "# Name \"" << name << pluginComponent << " - Win32 Release\"" << endl;
        out << "# Name \"" << name << pluginComponent << " - Win32 Debug\"" << endl;
        for (int i = 0; i < srcFiles.size(); ++i)
        {
            out << "# Begin Source File" << endl;
            out << "" << endl;
            out << "SOURCE=..\\..\\visit\\" << pluginType << "\\" << name << "\\" << srcFiles[i] << endl;
            out << "# End Source File" << endl;
        }
        out << "# End Target" << endl;
        out << "# End Project" << endl;
    }

    void WriteProjectHelper_Version7(ostream &out, const QString &pluginType, char pluginComponent,
        const QString &exports, const QString &libs, const vector<QString> &srcFiles)
    {
        const char *suffix = (pluginComponent == 'E') ? "_ser" : "";

        QString pType("Operator");
        if(pluginType == "plots")
            pType = "Plot";

        out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << endl;
        out << "<VisualStudioProject" << endl;
        out << "\tProjectType=\"Visual C++\"" << endl;
        out << "\tVersion=\"7.10\"" << endl;
        out << "\tName=\"" << name << pluginComponent << "\"" << endl;
        out << "\tRootNamespace=\"" << name << pluginComponent << "\"" << endl;
        out << "\tSccProjectName=\"\"" << endl;
        out << "\tSccLocalPath=\"\">" << endl;
        out << "\t<Platforms>" << endl;
        out << "\t\t<Platform" << endl;
        out << "\t\t\tName=\"Win32\"/>" << endl;
        out << "\t</Platforms>" << endl;
        out << "\t<Configurations>" << endl;
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Release|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Release\\" << name << pluginComponent << "\\\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Release\\" << name << pluginComponent << "\\\"" << endl;
        out << "\t\t\tConfigurationType=\"2\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
        out << "\t\t\tCharacterSet=\"2\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
        out << "\t\t\t\tOptimization=\"2\"" << endl;
        out << "\t\t\t\tInlineFunctionExpansion=\"1\"" << endl;
        out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
        out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
        out << "\t\t\t\tAdditionalIncludeDirectories=\"$(QTDIR)\\include;..\\..\\visit\\" << pluginType << "\\" << name << ";..\\..\\include\\VisIt;..\\..\\include\\vtk;..\\..\\include\\vtk\\MSVC7.Net;..\\..\\include\\mesa;..\\..\\include\\Python-2.5;..\\..\\include\\zlib\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"WIN32;NDEBUG;_WINDOWS;_USRDLL;GENERAL_PLUGIN_EXPORTS";
        if (exports != "")
            out << ";" << exports;
        out << "\"" << endl;
        out << "\t\t\t\tStringPooling=\"TRUE\"" << endl;
        out << "\t\t\t\tRuntimeLibrary=\"2\"" << endl;
        out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
        out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
        out << "\t\t\t\tPrecompiledHeaderFile=\".\\Release\\" << name << pluginComponent << "\\" << name << pluginComponent << ".pch\"" << endl;
        out << "\t\t\t\tAssemblerListingLocation=\".\\Release\\" << name << pluginComponent << "\\\"" << endl;
        out << "\t\t\t\tObjectFile=\".\\Release\\" << name << pluginComponent << "\\\"" << endl;
        out << "\t\t\t\tProgramDataBaseFileName=\".\\Release\\" << name << pluginComponent << "\\\"" << endl;
        out << "\t\t\t\tWarningLevel=\"3\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
        out << "\t\t\t\tAdditionalDependencies=\"odbc32.lib odbccp32.lib " << libs << "\"" << endl;
        out << "\t\t\t\tOutputFile=\"..\\..\\bin\\MSVC7.Net\\Release\\" << pluginType << "\\lib" << pluginComponent << name << pType << suffix << ".dll\"" << endl;
        out << "\t\t\t\tLinkIncremental=\"1\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tProgramDatabaseFile=\".\\Release\\" << name << pluginComponent << "\\" << name << pluginComponent << ".pdb\"" << endl;
        out << "\t\t\t\tAdditionalLibraryDirectories=\"$(QTDIR)\\lib;..\\..\\lib\\MSVC7.Net\\Release\"" << endl;
        out << "\t\t\t\tImportLibrary=\".\\Release\\" << name << pluginComponent << "\\lib" << pluginComponent << name << pType << suffix << ".lib\"" << endl;
        out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"NDEBUG\"" << endl;
        out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Release\\" << name << pluginComponent << "\\lib" << pluginComponent << name << pType << suffix << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;

        if(pluginComponent == 'G')
        {
            QString winType("PlotWindow");
            if(pluginType == "operators")
                winType = "Window";
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreBuildEventTool\"" << endl;
            out << "\t\t\t\tCommandLine=\"$(QTDIR)\\bin\\moc.exe ..\\..\\visit\\" << pluginType << "\\" << name << "\\Qvis" << name << winType << ".h -o ..\\..\\visit\\" << pluginType << "\\" << name << "\\Qvis" << name << winType << "_moc.C\"/>" << endl;
        }
        else
        {
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        }

        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"NDEBUG\"" << endl;
        out << "\t\t\t\tCulture=\"1033\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCWebDeploymentTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCManagedWrapperGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>" << endl;
        out << "\t\t</Configuration>" << endl;
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Debug|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Debug\\" << name << pluginComponent << "\\\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Debug\\" << name << pluginComponent << "\\\"" << endl;
        out << "\t\t\tConfigurationType=\"2\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
        out << "\t\t\tCharacterSet=\"2\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
        out << "\t\t\t\tOptimization=\"0\"" << endl;
        out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
        out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
        out << "\t\t\t\tAdditionalIncludeDirectories=\"$(QTDIR)\\include;..\\..\\visit\\" << pluginType << "\\" << name << ";..\\..\\include\\VisIt;..\\..\\include\\vtk;..\\..\\include\\vtk\\MSVC7.Net;..\\..\\include\\mesa;..\\..\\include\\Python-2.5;..\\..\\include\\zlib\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"WIN32;_DEBUG;_WINDOWS;_USRDLL;GENERAL_PLUGIN_EXPORTS";
        if (exports != "")
            out << ";" << exports;
        out << "\"" << endl;
        out << "\t\t\t\tBasicRuntimeChecks=\"3\"" << endl;
        out << "\t\t\t\tRuntimeLibrary=\"3\"" << endl;
        out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
        out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
        out << "\t\t\t\tPrecompiledHeaderFile=\".\\Debug\\" << name << pluginComponent << "\\" << name << pluginComponent<< ".pch\"" << endl;
        out << "\t\t\t\tAssemblerListingLocation=\".\\Debug\\" << name << pluginComponent << "\\\"" << endl;
        out << "\t\t\t\tObjectFile=\".\\Debug\\" << name << pluginComponent << "\\\"" << endl;
        out << "\t\t\t\tProgramDataBaseFileName=\".\\Debug\\" << name << pluginComponent << "\\\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tDebugInformationFormat=\"4\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
        out << "\t\t\t\tAdditionalDependencies=\"odbc32.lib odbccp32.lib " << libs << "\"" << endl;
        out << "\t\t\t\tOutputFile=\"..\\..\\bin\\MSVC7.Net\\Debug\\" << pluginType << "\\lib" << pluginComponent << name << pType << suffix << ".dll\"" << endl;
        out << "\t\t\t\tLinkIncremental=\"1\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tGenerateDebugInformation=\"TRUE\"" << endl;
        out << "\t\t\t\tProgramDatabaseFile=\".\\Debug\\" << name << pluginComponent << "\\" << name << pluginComponent << ".pdb\"" << endl;
        out << "\t\t\t\tAdditionalLibraryDirectories=\"$(QTDIR)\\lib;..\\..\\lib\\MSVC7.Net\\Debug\"" << endl;
        out << "\t\t\t\tImportLibrary=\".\\Debug\\" << name << pluginComponent << "\\lib" << pluginComponent << name << pType << suffix << ".lib\"" << endl;
        out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
        out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Debug\\" << name << pluginComponent << "\\lib" << pluginComponent << name << pType << suffix << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;

        if(pluginComponent == 'G')
        {
            QString winType("PlotWindow");
            if(pluginType == "operators")
                winType = "Window";
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreBuildEventTool\"" << endl;
            out << "\t\t\t\tCommandLine=\"$(QTDIR)\\bin\\moc.exe ..\\..\\visit\\" << pluginType << "\\" << name << "\\Qvis" << name << winType << ".h -o ..\\..\\visit\\" << pluginType << "\\" << name << "\\Qvis" << name << winType << "_moc.C\"/>" << endl;
        }
        else
        {
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        }
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
        out << "\t\t\t\tCulture=\"1033\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCWebDeploymentTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCManagedWrapperGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>" << endl;
        out << "\t\t</Configuration>" << endl;
        out << "\t</Configurations>" << endl;
        out << "\t<References>" << endl;
        out << "\t</References>" << endl;
        out << "\t<Files>" << endl;
        out << "\t\t<Filter" << endl;
        out << "\t\t\tName=\"Source Files\"" << endl;
        out << "\t\t\tFilter=\"cpp;c;cxx;rc;def;r;odl;idl;hpj;bat\">" << endl;
    
        for(int i = 0; i < srcFiles.size(); ++i)
        {
            out << "\t\t\t<File" << endl;
            out << "\t\t\t\tRelativePath=\"..\\..\\visit\\" << pluginType << "\\" << name << "\\" << srcFiles[i] << "\">" << endl;
            out << "\t\t\t\t<FileConfiguration" << endl;
            out << "\t\t\t\t\tName=\"Debug|Win32\">" << endl;
            out << "\t\t\t\t\t<Tool" << endl;
            out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\t\t\tOptimization=\"0\"" << endl;
            out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" << endl;
            out << "\t\t\t\t\t\tBasicRuntimeChecks=\"3\"" << endl;
            out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
            out << "\t\t\t\t</FileConfiguration>" << endl;
            out << "\t\t\t\t<FileConfiguration" << endl;
            out << "\t\t\t\t\tName=\"Release|Win32\">" << endl;
            out << "\t\t\t\t\t<Tool" << endl;
            out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\t\t\tOptimization=\"2\"" << endl;
            out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" << endl;
            out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
            out << "\t\t\t\t</FileConfiguration>" << endl;
            out << "\t\t\t</File>" << endl;
        }
    
        out << "\t\t</Filter>" << endl;
        out << "\t</Files>" << endl;
        out << "\t<Globals>" << endl;
        out << "\t</Globals>" << endl;
        out << "</VisualStudioProject>" << endl;
    }

    void WriteProjectHelper(ostream &out, const QString &pluginType, char pluginComponent,
        const QString &exports, const QString &libs, const vector<QString> &srcFiles, bool version7)
    {
        if(version7)
            WriteProjectHelper_Version7(out, pluginType, pluginComponent, exports, libs, srcFiles);
        else
            WriteProjectHelper_Version6(out, pluginType, pluginComponent, exports, libs, srcFiles);
    }

    /***************************************************************************
     ************************ BEGIN PLOT PROJECT CODING ************************
     **************************************************************************/

    void WritePlotProject_I(ostream &out, bool version7)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");

        WriteProjectHelper(out, "plots", 'I', "",
            "plugin.lib", srcFiles, version7);
    }

    void WritePlotProject_E(ostream &out, bool version7)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");
        srcFiles.push_back(name + "CommonPluginInfo.C");
        srcFiles.push_back(atts->name + ".C");

        srcFiles.push_back(name + "EnginePluginInfo.C");
        srcFiles.push_back("avt" + name + "Plot.C");

        if(customefiles)
            AddElements(srcFiles, efiles);
        else
            AddElements(srcFiles, defaultefiles);

        WriteProjectHelper(out, "plots", 'E', "ENGINE_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib plotter.lib pipeline_ser.lib avtfilters.lib avtexceptions.lib viewer.lib dbatts.lib visit_vtk.lib vtkCommon.lib vtkGraphics.lib vtkFiltering.lib",
            srcFiles, version7);
    }

    void WritePlotProject_G(ostream &out, bool version7)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");
        srcFiles.push_back(name + "CommonPluginInfo.C");
        srcFiles.push_back(name + "GUIPluginInfo.C");
        srcFiles.push_back(atts->name + ".C");

        if(customwfiles)
        {
            for (int i=0; i<wfiles.size(); i++)
            {
                if(wfiles[i].right(2) == ".h")
                {
                    srcFiles.push_back(wfiles[i].left(wfiles[i].length() - 2) + ".C");
                    srcFiles.push_back(wfiles[i].left(wfiles[i].length() - 2) + "_moc.C");
                }
            }
        }
        else
        {
            for (int i=0; i<defaultwfiles.size(); i++)
            {
                if(defaultwfiles[i].right(2) == ".h")
                {
                    srcFiles.push_back(defaultwfiles[i].left(defaultwfiles[i].length() - 2) + ".C");
                    srcFiles.push_back(defaultwfiles[i].left(defaultwfiles[i].length() - 2) + "_moc.C");
                }
            }
        }

        if (customgfiles)
            AddElements(srcFiles, gfiles);
        else
            AddElements(srcFiles, defaultgfiles);

        WriteProjectHelper(out, "plots", 'G', "GUI_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib gui.lib viewerproxy.lib viewerrpc.lib $(QTLIB)",
            srcFiles, version7);
    }

    void WritePlotProject_S(ostream &out, bool version7)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");
        srcFiles.push_back(name + "CommonPluginInfo.C");
        srcFiles.push_back(name + "ScriptingPluginInfo.C");
        srcFiles.push_back("Py"+atts->name + ".C");
        srcFiles.push_back(atts->name + ".C");

        if (customsfiles)
            AddElements(srcFiles, sfiles);
        else
            AddElements(srcFiles, defaultsfiles);

        WriteProjectHelper(out, "plots", 'S', "SCRIPTING_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib", srcFiles, version7);
    }

    void WritePlotProject_V(ostream &out, bool version7)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");
        srcFiles.push_back(name + "CommonPluginInfo.C");
        srcFiles.push_back(name + "ViewerPluginInfo.C");
        srcFiles.push_back("avt"+name + "Plot.C");
        srcFiles.push_back(atts->name + ".C");

        if(customvwfiles)
        {
            for (int i=0; i<vwfiles.size(); i++)
            {
                if(vwfiles[i].right(2) == ".h")
                {
                    srcFiles.push_back(vwfiles[i].left(vwfiles[i].length() - 2) + ".C");
                    srcFiles.push_back(vwfiles[i].left(vwfiles[i].length() - 2) + "_moc.C");
                }
            }
        }

        if (customvfiles)
            AddElements(srcFiles, vfiles);
        else
            AddElements(srcFiles, defaultvfiles);

        WriteProjectHelper(out, "plots", 'V', "VIEWER_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib plotter.lib pipeline_ser.lib avtfilters.lib avtexceptions.lib viewer.lib dbatts.lib visit_vtk.lib vtkCommon.lib vtkGraphics.lib vtkFiltering.lib",
            srcFiles, version7);
    }

    void WritePlotProjects(bool (*openCB)(ofstream &, const QString &), bool version7)
    {
        QString projectExtension(".dsp");
        QString workSpaceExtension(".dsw");
        QString projectDir(ProjectDir());

        if(version7)
        {
            projectExtension = ".vcproj";
            workSpaceExtension = ".sln";
        }
#if defined(_WIN32)
        if(version7)
            projectDir += QString("projects-MSVC7.Net\\plots\\");
        else
            projectDir += QString("projects\\plots\\");
#endif
        QString IProject(projectDir + (name + "I") + projectExtension);
        QString EProject(projectDir + (name + "E") + projectExtension);
        QString GProject(projectDir + (name + "G") + projectExtension);
        QString SProject(projectDir + (name + "S") + projectExtension);
        QString VProject(projectDir + (name + "V") + projectExtension);
        QString pluginTopProject(projectDir + name + projectExtension);
        QString workspace(projectDir + name + workSpaceExtension);

        cout << "Plot projects saved to " << projectDir << endl;

        // Write the I project
        ofstream out;
        if(openCB(out, IProject))
        {
            WritePlotProject_I(out, version7);
            out.close();
        }

        // Write the E project
        if(openCB(out, EProject))
        {
            WritePlotProject_E(out, version7);
            out.close();
        }

        // Write the G project
        if(openCB(out, GProject))
        {
            WritePlotProject_G(out, version7);
            out.close();
        }

        // Write the S project
        if(openCB(out, SProject))
        {
            WritePlotProject_S(out, version7);
            out.close();
        }

        // Write the V project
        if(openCB(out, VProject))
        {
            WritePlotProject_V(out, version7);
            out.close();
        }

        // Write the top level project
        if(openCB(out, pluginTopProject))
        {
            WriteProject_TOP_LEVEL(out, version7);
            out.close();
        }

        // Write the plugin workspace.
        if(openCB(out, workspace))
        {
            vector<QString> projects;
            projects.push_back(name);
            projects.push_back(name + "I");
            projects.push_back(name + "E");
            projects.push_back(name + "G");
            projects.push_back(name + "S");
            projects.push_back(name + "V");
            WriteProjectWorkspace(out, projects, version7);
        }
    }

    /***************************************************************************
     ************************* END PLOT PROJECT CODING *************************
     **************************************************************************/

    /***************************************************************************
     ********************** BEGIN OPERATOR PROJECT CODING **********************
     **************************************************************************/

    void WriteOperatorProject_I(ostream &out, bool version7)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");

        WriteProjectHelper(out, "operators", 'I', "",
            "plugin.lib", srcFiles, version7);
    }

    void WriteOperatorProject_E(ostream &out, bool version7)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");
        srcFiles.push_back(name + "CommonPluginInfo.C");
        srcFiles.push_back(atts->name + ".C");

        srcFiles.push_back(name + "EnginePluginInfo.C");

        if(customefiles)
            AddElements(srcFiles, efiles);
        else
            AddElements(srcFiles, defaultefiles);

        WriteProjectHelper(out, "operators", 'E', "ENGINE_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib plotter.lib avtfilters.lib pipeline_ser.lib avtexceptions.lib viewer.lib dbatts.lib visit_vtk.lib vtkCommon.lib vtkGraphics.lib vtkFiltering.lib",
            srcFiles, version7);
    }

    void WriteOperatorProject_G(ostream &out, bool version7)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");
        srcFiles.push_back(name + "CommonPluginInfo.C");
        srcFiles.push_back(name + "GUIPluginInfo.C");
        srcFiles.push_back(atts->name + ".C");
        srcFiles.push_back("Qvis" + name + "Window.C");
        srcFiles.push_back("Qvis" + name + "Window_moc.C");

        if(customwfiles)
        {
            for (int i=0; i<wfiles.size(); i++)
            {
                if(wfiles[i].right(2) == ".h")
                {
                    srcFiles.push_back(wfiles[i].left(wfiles[i].length() - 2) + ".C");
                    srcFiles.push_back(wfiles[i].left(wfiles[i].length() - 2) + "_moc.C");
                }
            }
        }
        else
        {
            for (int i=0; i<defaultwfiles.size(); i++)
            {
                if(defaultwfiles[i].right(2) == ".h")
                {
                    srcFiles.push_back(defaultwfiles[i].left(defaultwfiles[i].length() - 2) + ".C");
                    srcFiles.push_back(defaultwfiles[i].left(defaultwfiles[i].length() - 2) + "_moc.C");
                }
            }
        }

        if (customgfiles)
            AddElements(srcFiles, gfiles);
        else
            AddElements(srcFiles, defaultgfiles);

        WriteProjectHelper(out, "operators", 'G', "GUI_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib gui.lib viewerproxy.lib viewerrpc.lib $(QTLIB)",
            srcFiles, version7);
    }

    void WriteOperatorProject_S(ostream &out, bool version7)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");
        srcFiles.push_back(name + "CommonPluginInfo.C");
        srcFiles.push_back(name + "ScriptingPluginInfo.C");
        srcFiles.push_back("Py"+atts->name + ".C");
        srcFiles.push_back(atts->name + ".C");

        if (customsfiles)
            AddElements(srcFiles, sfiles);
        else
            AddElements(srcFiles, defaultsfiles);

        WriteProjectHelper(out, "operators", 'S', "SCRIPTING_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib", srcFiles, version7);
    }

    void WriteOperatorProject_V(ostream &out, bool version7)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");
        srcFiles.push_back(name + "CommonPluginInfo.C");
        srcFiles.push_back(name + "ViewerPluginInfo.C");
        srcFiles.push_back(atts->name + ".C");

        if(customvwfiles)
        {
            for (int i=0; i<vwfiles.size(); i++)
            {
                if(vwfiles[i].right(2) == ".h")
                {
                    srcFiles.push_back(vwfiles[i].left(vwfiles[i].length() - 2) + ".C");
                    srcFiles.push_back(vwfiles[i].left(vwfiles[i].length() - 2) + "_moc.C");
                }
            }
        }

        if (customvfiles)
            AddElements(srcFiles, vfiles);
        else
            AddElements(srcFiles, defaultvfiles);

        WriteProjectHelper(out, "operators", 'V', "VIEWER_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib plotter.lib avtfilters.lib pipeline_ser.lib avtexceptions.lib viewer.lib dbatts.lib visit_vtk.lib vtkCommon.lib vtkGraphics.lib vtkFiltering.lib",
            srcFiles, version7);
    }

    void WriteOperatorProjects(bool (*openCB)(ofstream &, const QString &), bool version7)
    {
        QString projectExtension(".dsp");
        QString workSpaceExtension(".dsw");
        QString projectDir(ProjectDir());
        if(version7)
        {
            projectExtension = ".vcproj";
            workSpaceExtension = ".sln";
        }

#if defined(_WIN32)
        if(version7)
            projectDir += QString("projects-MSVC7.Net\\operators\\");
        else
            projectDir += QString("projects\\operators\\");
#endif
        QString IProject(projectDir + (name + "I") + projectExtension);
        QString EProject(projectDir + (name + "E") + projectExtension);
        QString GProject(projectDir + (name + "G") + projectExtension);
        QString SProject(projectDir + (name + "S") + projectExtension);
        QString VProject(projectDir + (name + "V") + projectExtension);
        QString pluginTopProject(projectDir + name + projectExtension);
        QString workspace(projectDir + name + workSpaceExtension);

        cout << "Operator projects saved to " << projectDir << endl;

        // Write the I project
        ofstream out;
        if(openCB(out, IProject))
        {
            WriteOperatorProject_I(out, version7);
            out.close();
        }

        // Write the E project
        if(openCB(out, EProject))
        {
            WriteOperatorProject_E(out, version7);
            out.close();
        }

        // Write the G project
        if(openCB(out, GProject))
        {
            WriteOperatorProject_G(out, version7);
            out.close();
        }

        // Write the S project
        if(openCB(out, SProject))
        {
            WriteOperatorProject_S(out, version7);
            out.close();
        }

        // Write the V project
        if(openCB(out, VProject))
        {
            WriteOperatorProject_V(out, version7);
            out.close();
        }

        // Write the top level project
        if(openCB(out, pluginTopProject))
        {
            WriteProject_TOP_LEVEL(out, version7);
            out.close();
        }

        // Write the plugin workspace.
        if(openCB(out, workspace))
        {
            vector<QString> projects;
            projects.push_back(name);
            projects.push_back(name + "I");
            projects.push_back(name + "E");
            projects.push_back(name + "G");
            projects.push_back(name + "S");
            projects.push_back(name + "V");
            WriteProjectWorkspace(out, projects, version7);
        }
    }

    /***************************************************************************
     *********************** END OPERATOR PROJECT CODING ***********************
     **************************************************************************/

    /***************************************************************************
     ********************** BEGIN DATABASE PROJECT CODING **********************
     **************************************************************************/

    void WriteDatabaseProject_EM_Helper_Version6(ostream &out, char pluginType, const vector<QString> &srcFiles)
    {
        char *pluginDefs = "/D \"MDSERVER_PLUGIN_EXPORTS\" /D \"MDSERVER\"";
        char *pluginSuffix = "";
        if(pluginType == 'E')
        {
            pluginDefs = "/D \"ENGINE_PLUGIN_EXPORTS\"";
            pluginSuffix = "_ser";
        }

        out << "# Microsoft Developer Studio Project File - Name=\"" << name << pluginType << "\" - Package Owner=<4>\n";
        out << "# Microsoft Developer Studio Generated Build File, Format Version 6.00\n";
        out << "# ** DO NOT EDIT **\n";
        out << "\n";
        out << "# TARGTYPE \"Win32 (x86) Dynamic-Link Library\" 0x0102\n";
        out << "\n";
        out << "CFG=" << name << pluginType << " - Win32 Debug\n";
        out << "!MESSAGE This is not a valid makefile. To build this project using NMAKE,\n";
        out << "!MESSAGE use the Export Makefile command and run\n";
        out << "!MESSAGE \n";
        out << "!MESSAGE NMAKE /f \"" << name << pluginType << ".mak\".\n";
        out << "!MESSAGE \n";
        out << "!MESSAGE You can specify a configuration when running NMAKE\n";
        out << "!MESSAGE by defining the macro CFG on the command line. For example:\n";
        out << "!MESSAGE \n";
        out << "!MESSAGE NMAKE /f \"" << name << pluginType << ".mak\" CFG=\"" << name << pluginType << " - Win32 Debug\"\n";
        out << "!MESSAGE \n";
        out << "!MESSAGE Possible choices for configuration are:\n";
        out << "!MESSAGE \n";
        out << "!MESSAGE \"" << name << pluginType << " - Win32 Release\" (based on \"Win32 (x86) Dynamic-Link Library\")\n";
        out << "!MESSAGE \"" << name << pluginType << " - Win32 Debug\" (based on \"Win32 (x86) Dynamic-Link Library\")\n";
        out << "!MESSAGE \n";
        out << "\n";
        out << "# Begin Project\n";
        out << "# PROP AllowPerConfigDependencies 0\n";
        out << "# PROP Scc_ProjName \"\"\n";
        out << "# PROP Scc_LocalPath \"\"\n";
        out << "CPP=cl.exe\n";
        out << "MTL=midl.exe\n";
        out << "RSC=rc.exe\n";
        out << "\n";
        out << "!IF  \"$(CFG)\" == \"" << name << pluginType << " - Win32 Release\"\n";
        out << "\n";
        out << "# PROP BASE Use_MFC 0\n";
        out << "# PROP BASE Use_Debug_Libraries 0\n";
        out << "# PROP BASE Output_Dir \"Release\"\n";
        out << "# PROP BASE Intermediate_Dir \"Release\"\n";
        out << "# PROP BASE Target_Dir \"\"\n";
        out << "# PROP Use_MFC 0\n";
        out << "# PROP Use_Debug_Libraries 0\n";
        out << "# PROP Output_Dir \"Release\"\n";
        out << "# PROP Intermediate_Dir \"Release\\" << name << pluginType << "\"\n";
        out << "# PROP Ignore_Export_Lib 0\n";
        out << "# PROP Target_Dir \"\"\n";
        out << "# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"" << name << pluginType << "_EXPORTS\" /YX /FD /c\n";
        out << "# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I \"..\\..\\visit\\databases\\" << name << "\" /D \"NDEBUG\" /D \"WIN32\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"GENERAL_PLUGIN_EXPORTS\" " << pluginDefs << " /YX /FD /TP /c\n";
        out << "# ADD BASE MTL /nologo /D \"NDEBUG\" /mktyplib203 /win32\n";
        out << "# ADD MTL /nologo /D \"NDEBUG\" /mktyplib203 /win32\n";
        out << "# ADD BASE RSC /l 0x409 /d \"NDEBUG\"\n";
        out << "# ADD RSC /l 0x409 /d \"NDEBUG\"\n";
        out << "BSC32=bscmake.exe\n";
        out << "# ADD BASE BSC32 /nologo\n";
        out << "# ADD BSC32 /nologo\n";
        out << "LINK32=link.exe\n";
        out << "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386\n";
        out << "# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  state.lib misc.lib plugin.lib pipeline_ser.lib dbatts.lib database_ser.lib avtexceptions.lib vtkCommon.lib vtkFiltering.lib /nologo /dll /machine:I386 /out:\"Release/lib" << pluginType << name << "Database"<<pluginSuffix<<".dll\"\n";
        out << "# Begin Special Build Tool\n";
        out << "SOURCE=\"$(InputPath)\"\n";
        out << "PostBuild_Cmds=copy Release\\lib" << pluginType << name << "Database" << pluginSuffix << ".dll ..\\..\\bin\\Release\\databases\n";
        out << "# End Special Build Tool\n";
        out << "\n";
        out << "!ELSEIF  \"$(CFG)\" == \"" << name << pluginType << " - Win32 Debug\"\n";
        out << "\n";
        out << "# PROP BASE Use_MFC 0\n";
        out << "# PROP BASE Use_Debug_Libraries 1\n";
        out << "# PROP BASE Output_Dir \"Debug\"\n";
        out << "# PROP BASE Intermediate_Dir \"Debug\"\n";
        out << "# PROP BASE Target_Dir \"\"\n";
        out << "# PROP Use_MFC 0\n";
        out << "# PROP Use_Debug_Libraries 1\n";
        out << "# PROP Output_Dir \"Debug\"\n";
        out << "# PROP Intermediate_Dir \"Debug\\" << name << pluginType << "\"\n";
        out << "# PROP Ignore_Export_Lib 0\n";
        out << "# PROP Target_Dir \"\"\n";
        out << "# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"" << name << pluginType << "_EXPORTS\" /YX /FD /GZ /c\n";
        out << "# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I \"..\\..\\visit\\databases\\" << name << "\" /D \"_DEBUG\" /D \"WIN32\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"GENERAL_PLUGIN_EXPORTS\" "<< pluginDefs << " /YX /FD /GZ /TP /c\n";
        out << "# ADD BASE MTL /nologo /D \"_DEBUG\" /mktyplib203 /win32\n";
        out << "# ADD MTL /nologo /D \"_DEBUG\" /mktyplib203 /win32\n";
        out << "# ADD BASE RSC /l 0x409 /d \"_DEBUG\"\n";
        out << "# ADD RSC /l 0x409 /d \"_DEBUG\"\n";
        out << "BSC32=bscmake.exe\n";
        out << "# ADD BASE BSC32 /nologo\n";
        out << "# ADD BSC32 /nologo\n";
        out << "LINK32=link.exe\n";
        out << "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept\n";
        out << "# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib state.lib misc.lib plugin.lib pipeline_ser.lib dbatts.lib database_ser.lib avtexceptions.lib vtkCommon.lib vtkFiltering.lib /nologo /dll /debug /machine:I386 /out:\"Debug/lib" << pluginType << name << "Database"<<pluginSuffix<<".dll\" /pdbtype:sept\n";
        out << "# Begin Special Build Tool\n";
        out << "SOURCE=\"$(InputPath)\"\n";
        out << "PostBuild_Cmds=copy Debug\\lib" << pluginType << name << "Database" << pluginSuffix << ".dll ..\\..\\bin\\Debug\\databases\n";
        out << "# End Special Build Tool\n";
        out << "\n";
        out << "!ENDIF \n";
        out << "\n";
        out << "# Begin Target\n";
        out << "\n";
        out << "# Name \"" << name << pluginType << " - Win32 Release\"\n";
        out << "# Name \"" << name << pluginType << " - Win32 Debug\"\n";

        for(int i = 0; i < srcFiles.size(); ++i)
        {
            out << "# Begin Source File\n";
            out << "\n";
            out << "SOURCE=..\\..\\visit\\databases\\" << name << "\\" << srcFiles[i] << endl;
            out << "# End Source File\n";
        }
        out << "# End Target\n";
        out << "# End Project\n";
    }

    void WriteDatabaseProject_EM_Helper_Version7(ostream &out, char pluginType, const vector<QString> &srcFiles)
    {
        char *pluginDefs = "";
        char *pluginSuffix = "Database";

        pluginDefs = "GENERAL_PLUGIN_EXPORTS;MDSERVER_PLUGIN_EXPORTS;MDSERVER";
        if(pluginType == 'E')
        {
            pluginDefs = "GENERAL_PLUGIN_EXPORTS;ENGINE_PLUGIN_EXPORTS";
            pluginSuffix = "Database_ser";
        }

        out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << endl;
        out << "<VisualStudioProject" << endl;
        out << "\tProjectType=\"Visual C++\"" << endl;
        out << "\tVersion=\"7.10\"" << endl;
        out << "\tName=\"" << name << pluginType << "\"" << endl;
        out << "\tRootNamespace=\"" << name << pluginType << "\"" << endl;
        out << "\tSccProjectName=\"\"" << endl;
        out << "\tSccLocalPath=\"\">" << endl;
        out << "\t<Platforms>" << endl;
        out << "\t\t<Platform" << endl;
        out << "\t\t\tName=\"Win32\"/>" << endl;
        out << "\t</Platforms>" << endl;
        out << "\t<Configurations>" << endl;
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Release|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Release\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Release\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\tConfigurationType=\"2\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
        out << "\t\t\tCharacterSet=\"2\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
        out << "\t\t\t\tOptimization=\"2\"" << endl;
        out << "\t\t\t\tInlineFunctionExpansion=\"1\"" << endl;
        out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
        out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
        out << "\t\t\t\tAdditionalIncludeDirectories=\"..\\..\\visit\\databases\\" << name << ";..\\..\\include\\VisIt;..\\..\\include\\vtk;..\\..\\include\\vtk\\MSVC7.Net;..\\..\\include\\silo;..\\..\\include\\zlib\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"WIN32;NDEBUG;_WINDOWS;_USRDLL" << pluginDefs << "\"" << endl;
        out << "\t\t\t\tStringPooling=\"TRUE\"" << endl;
        out << "\t\t\t\tRuntimeLibrary=\"2\"" << endl;
        out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
        out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
        out << "\t\t\t\tPrecompiledHeaderFile=\".\\Release\\" << name << pluginType << "\\" << name << pluginType << ".pch\"" << endl;
        out << "\t\t\t\tAssemblerListingLocation=\".\\Release\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\t\tObjectFile=\".\\Release\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\t\tProgramDataBaseFileName=\".\\Release\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\t\tWarningLevel=\"3\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
        out << "\t\t\t\tAdditionalDependencies=\"odbc32.lib odbccp32.lib state.lib misc.lib plugin.lib utility.lib pipeline_ser.lib dbatts.lib database_ser.lib avtexceptions.lib vtkCommon.lib vtkFiltering.lib\"" << endl;
        out << "\t\t\t\tOutputFile=\"..\\..\\bin\\MSVC7.Net\\Release\\databases\\lib" << pluginType << name << pluginSuffix << ".dll\"" << endl;
        out << "\t\t\t\tLinkIncremental=\"1\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tProgramDatabaseFile=\".\\Release\\" << name << pluginType << "\\" << name << pluginType << ".pdb\"" << endl;
        out << "\t\t\t\tAdditionalLibraryDirectories=\"..\\..\\lib\\MSVC7.Net\\Release\"" << endl;
        out << "\t\t\t\tImportLibrary=\".\\Release\\" << name << pluginType << "\\lib" << pluginType << name << pluginSuffix << ".lib\"" << endl;
        out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"NDEBUG\"" << endl;
        out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Release\\" << name << pluginType << "\\lib" << pluginType << name << pluginSuffix << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"NDEBUG\"" << endl;
        out << "\t\t\t\tCulture=\"1033\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCWebDeploymentTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCManagedWrapperGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>" << endl;
        out << "\t\t</Configuration>" << endl;
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Debug|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Debug\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Debug\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\tConfigurationType=\"2\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
        out << "\t\t\tCharacterSet=\"2\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
        out << "\t\t\t\tOptimization=\"0\"" << endl;
        out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
        out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
        out << "\t\t\t\tAdditionalIncludeDirectories=\"..\\..\\visit\\databases\\" << name << ";..\\..\\include\\VisIt;..\\..\\include\\vtk;..\\..\\include\\vtk\\MSVC7.Net;..\\..\\include\\silo;..\\..\\include\\zlib\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"WIN32;_DEBUG;_WINDOWS;_USRDLL" << pluginDefs << "\"" << endl;;
        out << "\t\t\t\tBasicRuntimeChecks=\"3\"" << endl;
        out << "\t\t\t\tRuntimeLibrary=\"3\"" << endl;
        out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
        out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
        out << "\t\t\t\tPrecompiledHeaderFile=\".\\Debug\\" << name << pluginType << "\\" << name << pluginType<< ".pch\"" << endl;
        out << "\t\t\t\tAssemblerListingLocation=\".\\Debug\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\t\tObjectFile=\".\\Debug\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\t\tProgramDataBaseFileName=\".\\Debug\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tDebugInformationFormat=\"4\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
        out << "\t\t\t\tAdditionalDependencies=\"odbc32.lib odbccp32.lib state.lib misc.lib plugin.lib utility.lib pipeline_ser.lib dbatts.lib database_ser.lib avtexceptions.lib vtkCommon.lib vtkFiltering.lib\"" << endl;
        out << "\t\t\t\tOutputFile=\"..\\..\\bin\\MSVC7.Net\\Debug\\databases\\lib" << pluginType << name << pluginSuffix << ".dll\"" << endl;
        out << "\t\t\t\tLinkIncremental=\"1\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tGenerateDebugInformation=\"TRUE\"" << endl;
        out << "\t\t\t\tProgramDatabaseFile=\".\\Debug\\" << name << pluginType << "\\" << name << pluginType << ".pdb\"" << endl;
        out << "\t\t\t\tAdditionalLibraryDirectories=\"..\\..\\lib\\MSVC7.Net\\Debug\"" << endl;
        out << "\t\t\t\tImportLibrary=\".\\Debug\\" << name << pluginType << "\\lib" << pluginType << name << pluginSuffix << ".lib\"" << endl;
        out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
        out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Debug\\" << name << pluginType << "\\lib" << pluginType << name << pluginSuffix << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
        out << "\t\t\t\tCulture=\"1033\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCWebDeploymentTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCManagedWrapperGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>" << endl;
        out << "\t\t</Configuration>" << endl;
        out << "\t</Configurations>" << endl;
        out << "\t<References>" << endl;
        out << "\t</References>" << endl;
        out << "\t<Files>" << endl;
        out << "\t\t<Filter" << endl;
        out << "\t\t\tName=\"Source Files\"" << endl;
        out << "\t\t\tFilter=\"cpp;c;cxx;rc;def;r;odl;idl;hpj;bat\">" << endl;
    
        for(int i = 0; i < srcFiles.size(); ++i)
        {
            out << "\t\t\t<File" << endl;
            out << "\t\t\t\tRelativePath=\"..\\..\\visit\\databases\\" << name << "\\" << srcFiles[i] << "\">" << endl;
            out << "\t\t\t\t<FileConfiguration" << endl;
            out << "\t\t\t\t\tName=\"Debug|Win32\">" << endl;
            out << "\t\t\t\t\t<Tool" << endl;
            out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\t\t\tOptimization=\"0\"" << endl;
            out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" << endl;
            out << "\t\t\t\t\t\tBasicRuntimeChecks=\"3\"" << endl;
            out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
            out << "\t\t\t\t</FileConfiguration>" << endl;
            out << "\t\t\t\t<FileConfiguration" << endl;
            out << "\t\t\t\t\tName=\"Release|Win32\">" << endl;
            out << "\t\t\t\t\t<Tool" << endl;
            out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\t\t\tOptimization=\"2\"" << endl;
            out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" << endl;
            out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
            out << "\t\t\t\t</FileConfiguration>" << endl;
            out << "\t\t\t</File>" << endl;
        }
    
        out << "\t\t</Filter>" << endl;
        out << "\t</Files>" << endl;
        out << "\t<Globals>" << endl;
        out << "\t</Globals>" << endl;
        out << "</VisualStudioProject>" << endl;
    }

    void WriteDatabaseProject_E(ostream &out, bool version7)
    {
        vector<QString> srcFiles;

        // Create the list of source files to build into the plugin.
        srcFiles.push_back(name + QString("PluginInfo.C"));
        srcFiles.push_back(name + QString("CommonPluginInfo.C"));
        srcFiles.push_back(name + QString("EnginePluginInfo.C"));
        if (haswriter)
        {
            srcFiles.push_back(QString("avt") + name + QString("Writer.C"));
        }
        if (hasoptions)
        {
            srcFiles.push_back(QString("avt") + name + QString("Options.C"));
        }
        if (custommfiles)
        {
            for (int i=0; i<mfiles.size(); i++)
                srcFiles.push_back(mfiles[i]);
        }
        else
        {
            for (int i=0; i<defaultmfiles.size(); i++)
                srcFiles.push_back(defaultmfiles[i]);
        }

        // Write the project file
        if(version7)
            WriteDatabaseProject_EM_Helper_Version7(out, 'E', srcFiles);
        else
            WriteDatabaseProject_EM_Helper_Version6(out, 'E', srcFiles);
    }

    void WriteDatabaseProject_M(ostream &out, bool version7)
    {
        vector<QString> srcFiles;

        // Create the list of source files to build into the plugin.
        srcFiles.push_back(name + QString("PluginInfo.C"));
        srcFiles.push_back(name + QString("CommonPluginInfo.C"));
        srcFiles.push_back(name + QString("MDServerPluginInfo.C"));
        if (haswriter)
        {
            srcFiles.push_back(QString("avt") + name + QString("Writer.C"));
        }
        if (hasoptions)
        {
            srcFiles.push_back(QString("avt") + name + QString("Options.C"));
        }
        if (custommfiles)
        {
            for (int i=0; i<efiles.size(); i++)
                srcFiles.push_back(efiles[i]);
        }
        else
        {
            for (int i=0; i<defaultefiles.size(); i++)
                srcFiles.push_back(defaultefiles[i]);
        }

        // Write the project file
        if(version7)
            WriteDatabaseProject_EM_Helper_Version7(out, 'M', srcFiles);
        else
            WriteDatabaseProject_EM_Helper_Version6(out, 'M', srcFiles);
    }

    void WriteDatabaseProject_I_Version6(ostream &out)
    {
        out << "# Microsoft Developer Studio Project File - Name=\"" << name << "I\" - Package Owner=<4>\n";
        out << "# Microsoft Developer Studio Generated Build File, Format Version 6.00\n";
        out << "# ** DO NOT EDIT **\n";
        out << "\n";
        out << "# TARGTYPE \"Win32 (x86) Dynamic-Link Library\" 0x0102\n";
        out << "\n";
        out << "CFG=" << name << "I - Win32 Debug\n";
        out << "!MESSAGE This is not a valid makefile. To build this project using NMAKE,\n";
        out << "!MESSAGE use the Export Makefile command and run\n";
        out << "!MESSAGE \n";
        out << "!MESSAGE NMAKE /f \"" << name << "I.mak\".\n";
        out << "!MESSAGE \n";
        out << "!MESSAGE You can specify a configuration when running NMAKE\n";
        out << "!MESSAGE by defining the macro CFG on the command line. For example:\n";
        out << "!MESSAGE \n";
        out << "!MESSAGE NMAKE /f \"" << name << "I.mak\" CFG=\"" << name << "I - Win32 Debug\"\n";
        out << "!MESSAGE \n";
        out << "!MESSAGE Possible choices for configuration are:\n";
        out << "!MESSAGE \n";
        out << "!MESSAGE \"" << name << "I - Win32 Release\" (based on \"Win32 (x86) Dynamic-Link Library\")\n";
        out << "!MESSAGE \"" << name << "I - Win32 Debug\" (based on \"Win32 (x86) Dynamic-Link Library\")\n";
        out << "!MESSAGE \n";
        out << "\n";
        out << "# Begin Project\n";
        out << "# PROP AllowPerConfigDependencies 0\n";
        out << "# PROP Scc_ProjName \"\"\n";
        out << "# PROP Scc_LocalPath \"\"\n";
        out << "CPP=cl.exe\n";
        out << "MTL=midl.exe\n";
        out << "RSC=rc.exe\n";
        out << "\n";
        out << "!IF  \"$(CFG)\" == \"" << name << "I - Win32 Release\"\n";
        out << "\n";
        out << "# PROP BASE Use_MFC 0\n";
        out << "# PROP BASE Use_Debug_Libraries 0\n";
        out << "# PROP BASE Output_Dir \"Release\"\n";
        out << "# PROP BASE Intermediate_Dir \"Release\"\n";
        out << "# PROP BASE Target_Dir \"\"\n";
        out << "# PROP Use_MFC 0\n";
        out << "# PROP Use_Debug_Libraries 0\n";
        out << "# PROP Output_Dir \"Release\"\n";
        out << "# PROP Intermediate_Dir \"Release\"\n";
        out << "# PROP Ignore_Export_Lib 0\n";
        out << "# PROP Target_Dir \"\"\n";
        out << "# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"" << name << "I_EXPORTS\" /YX /FD /c\n";
        out << "# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I \"..\\..\\visit\\databases\\" << name << "\" /D \"NDEBUG\" /D \"WIN32\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"GENERAL_PLUGIN_EXPORTS\" /YX /FD /TP /c\n";
        out << "# ADD BASE MTL /nologo /D \"NDEBUG\" /mktyplib203 /win32\n";
        out << "# ADD MTL /nologo /D \"NDEBUG\" /mktyplib203 /win32\n";
        out << "# ADD BASE RSC /l 0x409 /d \"NDEBUG\"\n";
        out << "# ADD RSC /l 0x409 /d \"NDEBUG\"\n";
        out << "BSC32=bscmake.exe\n";
        out << "# ADD BASE BSC32 /nologo\n";
        out << "# ADD BSC32 /nologo\n";
        out << "LINK32=link.exe\n";
        out << "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386\n";
        out << "# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  plugin.lib /nologo /dll /machine:I386 /out:\"Release/libI" << name << "Database.dll\"\n";
        out << "# Begin Special Build Tool\n";
        out << "SOURCE=\"$(InputPath)\"\n";
        out << "PostBuild_Cmds=copy Release\\libI" << name << "Database.dll ..\\..\\bin\\Release\\databases\n";
        out << "# End Special Build Tool\n";
        out << "\n";
        out << "!ELSEIF  \"$(CFG)\" == \"" << name << "I - Win32 Debug\"\n";
        out << "\n";
        out << "# PROP BASE Use_MFC 0\n";
        out << "# PROP BASE Use_Debug_Libraries 1\n";
        out << "# PROP BASE Output_Dir \"Debug\"\n";
        out << "# PROP BASE Intermediate_Dir \"Debug\"\n";
        out << "# PROP BASE Target_Dir \"\"\n";
        out << "# PROP Use_MFC 0\n";
        out << "# PROP Use_Debug_Libraries 1\n";
        out << "# PROP Output_Dir \"Debug\"\n";
        out << "# PROP Intermediate_Dir \"Debug\"\n";
        out << "# PROP Ignore_Export_Lib 0\n";
        out << "# PROP Target_Dir \"\"\n";
        out << "# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"" << name << "I_EXPORTS\" /YX /FD /GZ /c\n";
        out << "# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I \"..\\..\\visit\\databases\\" << name << "\" /D \"_DEBUG\" /D \"WIN32\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"GENERAL_PLUGIN_EXPORTS\" /YX /FD /GZ /TP /c\n";
        out << "# ADD BASE MTL /nologo /D \"_DEBUG\" /mktyplib203 /win32\n";
        out << "# ADD MTL /nologo /D \"_DEBUG\" /mktyplib203 /win32\n";
        out << "# ADD BASE RSC /l 0x409 /d \"_DEBUG\"\n";
        out << "# ADD RSC /l 0x409 /d \"_DEBUG\"\n";
        out << "BSC32=bscmake.exe\n";
        out << "# ADD BASE BSC32 /nologo\n";
        out << "# ADD BSC32 /nologo\n";
        out << "LINK32=link.exe\n";
        out << "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept\n";
        out << "# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib plugin.lib /nologo /dll /debug /machine:I386 /out:\"Debug/libI" << name << "Database.dll\" /pdbtype:sept\n";
        out << "# Begin Special Build Tool\n";
        out << "SOURCE=\"$(InputPath)\"\n";
        out << "PostBuild_Cmds=copy Debug\\libI" << name << "Database.dll ..\\..\\bin\\Debug\\databases\n";
        out << "# End Special Build Tool\n";
        out << "\n";
        out << "!ENDIF \n";
        out << "\n";
        out << "# Begin Target\n";
        out << "\n";
        out << "# Name \"" << name << "I - Win32 Release\"\n";
        out << "# Name \"" << name << "I - Win32 Debug\"\n";
        out << "# Begin Source File\n";
        out << "\n";
        out << "SOURCE=..\\..\\visit\\databases\\" << name << "\\" << name << "PluginInfo.C\n";
        out << "# End Source File\n";
        out << "# End Target\n";
        out << "# End Project\n";
    }

    void WriteDatabaseProject_I_Version7(ostream &out)
    {
        const char pluginType = 'I';
        out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << endl;
        out << "<VisualStudioProject" << endl;
        out << "\tProjectType=\"Visual C++\"" << endl;
        out << "\tVersion=\"7.10\"" << endl;
        out << "\tName=\"" << name << pluginType << "\"" << endl;
        out << "\tRootNamespace=\"" << name << pluginType << "\"" << endl;
        out << "\tSccProjectName=\"\"" << endl;
        out << "\tSccLocalPath=\"\">" << endl;
        out << "\t<Platforms>" << endl;
        out << "\t\t<Platform" << endl;
        out << "\t\t\tName=\"Win32\"/>" << endl;
        out << "\t</Platforms>" << endl;
        out << "\t<Configurations>" << endl;
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Release|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Release\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Release\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\tConfigurationType=\"2\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
        out << "\t\t\tCharacterSet=\"2\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
        out << "\t\t\t\tOptimization=\"2\"" << endl;
        out << "\t\t\t\tInlineFunctionExpansion=\"1\"" << endl;
        out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
        out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
        out << "\t\t\t\tAdditionalIncludeDirectories=\"..\\..\\visit\\databases\\" << name << ";..\\..\\include\\VisIt;..\\..\\include\\vtk;..\\..\\include\\vtk\\MSVC7.Net;..\\..\\include\\silo;..\\..\\include\\zlib\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"WIN32;NDEBUG;_WINDOWS;_USRDLL;GENERAL_PLUGIN_EXPORTS\"" << endl;
        out << "\t\t\t\tStringPooling=\"TRUE\"" << endl;
        out << "\t\t\t\tRuntimeLibrary=\"2\"" << endl;
        out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
        out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
        out << "\t\t\t\tPrecompiledHeaderFile=\".\\Release\\" << name << pluginType << "\\" << name << pluginType << ".pch\"" << endl;
        out << "\t\t\t\tAssemblerListingLocation=\".\\Release\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\t\tObjectFile=\".\\Release\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\t\tProgramDataBaseFileName=\".\\Release\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\t\tWarningLevel=\"3\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
        out << "\t\t\t\tAdditionalDependencies=\"odbc32.lib odbccp32.lib state.lib misc.lib plugin.lib pipeline_ser.lib dbatts.lib database_ser.lib avtexceptions.lib vtkCommon.lib\"" << endl;
        out << "\t\t\t\tOutputFile=\"..\\..\\bin\\MSVC7.Net\\Release\\databases\\lib" << pluginType << name << "Database.dll\"" << endl;
        out << "\t\t\t\tLinkIncremental=\"1\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tProgramDatabaseFile=\".\\Release\\" << name << pluginType << "\\" << name << pluginType << ".pdb\"" << endl;
        out << "\t\t\t\tAdditionalLibraryDirectories=\"..\\..\\lib\\MSVC7.Net\\Release\"" << endl;
        out << "\t\t\t\tImportLibrary=\".\\Release\\" << name << pluginType << "\\" << name << pluginType << ".lib\"" << endl;
        out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"NDEBUG\"" << endl;
        out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Release\\" << name << pluginType << "\\" << name << pluginType << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"NDEBUG\"" << endl;
        out << "\t\t\t\tCulture=\"1033\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCWebDeploymentTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCManagedWrapperGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>" << endl;
        out << "\t\t</Configuration>" << endl;
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Debug|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Debug\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Debug\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\tConfigurationType=\"2\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
        out << "\t\t\tCharacterSet=\"2\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
        out << "\t\t\t\tOptimization=\"0\"" << endl;
        out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
        out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
        out << "\t\t\t\tAdditionalIncludeDirectories=\"..\\..\\visit\\databases\\" << name << ";..\\..\\include\\VisIt;..\\..\\include\\vtk;..\\..\\include\\vtk\\MSVC7.Net;..\\..\\include\\silo;..\\..\\include\\zlib\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"WIN32;_DEBUG;_WINDOWS;_USRDLL;GENERAL_PLUGIN_EXPORTS\"" << endl;;
        out << "\t\t\t\tBasicRuntimeChecks=\"3\"" << endl;
        out << "\t\t\t\tRuntimeLibrary=\"3\"" << endl;
        out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
        out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
        out << "\t\t\t\tPrecompiledHeaderFile=\".\\Debug\\" << name << pluginType << "\\" << name << pluginType<< ".pch\"" << endl;
        out << "\t\t\t\tAssemblerListingLocation=\".\\Debug\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\t\tObjectFile=\".\\Debug\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\t\tProgramDataBaseFileName=\".\\Debug\\" << name << pluginType << "\\\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tDebugInformationFormat=\"4\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
        out << "\t\t\t\tAdditionalDependencies=\"odbc32.lib odbccp32.lib state.lib misc.lib plugin.lib pipeline_ser.lib dbatts.lib database_ser.lib avtexceptions.lib vtkCommon.lib\"" << endl;
        out << "\t\t\t\tOutputFile=\"..\\..\\bin\\MSVC7.Net\\Debug\\databases\\lib" << pluginType << name << "Database.dll\"" << endl;
        out << "\t\t\t\tLinkIncremental=\"1\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tGenerateDebugInformation=\"TRUE\"" << endl;
        out << "\t\t\t\tProgramDatabaseFile=\".\\Debug\\" << name << pluginType << "\\" << name << pluginType << ".pdb\"" << endl;
        out << "\t\t\t\tAdditionalLibraryDirectories=\"..\\..\\lib\\MSVC7.Net\\Debug\"" << endl;
        out << "\t\t\t\tImportLibrary=\".\\Debug\\" << name << pluginType << "\\" << name << pluginType << ".lib\"" << endl;
        out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
        out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Debug\\" << name << pluginType << "\\" << name << pluginType << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
        out << "\t\t\t\tCulture=\"1033\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCWebDeploymentTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCManagedWrapperGeneratorTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>" << endl;
        out << "\t\t</Configuration>" << endl;
        out << "\t</Configurations>" << endl;
        out << "\t<References>" << endl;
        out << "\t</References>" << endl;
        out << "\t<Files>" << endl;
        out << "\t\t<Filter" << endl;
        out << "\t\t\tName=\"Source Files\"" << endl;
        out << "\t\t\tFilter=\"cpp;c;cxx;rc;def;r;odl;idl;hpj;bat\">" << endl;
    
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");

        for(int i = 0; i < srcFiles.size(); ++i)
        {
            out << "\t\t\t<File" << endl;
            out << "\t\t\t\tRelativePath=\"..\\..\\visit\\databases\\" << name << "\\" << srcFiles[i] << "\">" << endl;
            out << "\t\t\t\t<FileConfiguration" << endl;
            out << "\t\t\t\t\tName=\"Debug|Win32\">" << endl;
            out << "\t\t\t\t\t<Tool" << endl;
            out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\t\t\tOptimization=\"0\"" << endl;
            out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" << endl;
            out << "\t\t\t\t\t\tBasicRuntimeChecks=\"3\"" << endl;
            out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
            out << "\t\t\t\t</FileConfiguration>" << endl;
            out << "\t\t\t\t<FileConfiguration" << endl;
            out << "\t\t\t\t\tName=\"Release|Win32\">" << endl;
            out << "\t\t\t\t\t<Tool" << endl;
            out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\t\t\tOptimization=\"2\"" << endl;
            out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" << endl;
            out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
            out << "\t\t\t\t</FileConfiguration>" << endl;
            out << "\t\t\t</File>" << endl;
        }
    
        out << "\t\t</Filter>" << endl;
        out << "\t</Files>" << endl;
        out << "\t<Globals>" << endl;
        out << "\t</Globals>" << endl;
        out << "</VisualStudioProject>" << endl;

    }

    void WriteDatabaseProjects(bool (*openCB)(ofstream &, const QString &), bool version7)
    {
        QString projectExtension(".dsp");
        QString workSpaceExtension(".dsw");
        QString projectDir(ProjectDir());
        if(version7)
        {
            projectExtension = ".vcproj";
            workSpaceExtension = ".sln";
        }

#if defined(_WIN32)
        if(version7)
            projectDir += QString("projects-MSVC7.Net\\databases\\");
        else
            projectDir += QString("projects\\databases\\");
#endif
        QString IProject(projectDir + (name + "I") + projectExtension);
        QString EProject(projectDir + (name + "E") + projectExtension);
        QString MProject(projectDir + (name + "M") + projectExtension);
        QString pluginTopProject(projectDir + name + projectExtension);
        QString workspace(projectDir + name + workSpaceExtension);

        cout << "Database projects saved to " << projectDir << endl;

        // Write the I project
        ofstream out;
        if(openCB(out, IProject))
        {
            if(version7)
                WriteDatabaseProject_I_Version7(out);
            else
                WriteDatabaseProject_I_Version6(out);
            out.close();
        }

        // Write the E project
        if(openCB(out, EProject))
        {
            WriteDatabaseProject_E(out, version7);
            out.close();
        }

        // Write the M project
        if(openCB(out, MProject))
        {
            WriteDatabaseProject_M(out, version7);
            out.close();
        }

        // Write the top level project
        if(openCB(out, pluginTopProject))
        {
            WriteProject_TOP_LEVEL(out, version7);
            out.close();
        }

        // Write the plugin workspace.
        if(openCB(out, workspace))
        {
            vector<QString> projects;
            projects.push_back(name);
            projects.push_back(name + "I");
            projects.push_back(name + "E");
            projects.push_back(name + "M");
            WriteProjectWorkspace(out, projects, version7);
        }
    }

    /***************************************************************************
     *********************** END DATABASE PROJECT CODING ***********************
     **************************************************************************/
public:

    void WriteProjectFiles(bool (*openCB)(ofstream &, const QString &), bool version7)
    {
        if (type == "operator")
        {
            WriteOperatorProjects(openCB, version7);
        }
        else if (type == "plot")
        {
            WritePlotProjects(openCB, version7);
        }
        else if (type == "database")
        {
            WriteDatabaseProjects(openCB, version7);
        }
    }
};


// ----------------------------------------------------------------------------
//                           Override default types
// ----------------------------------------------------------------------------
#define Plugin       ProjectFileGeneratorPlugin

#endif
