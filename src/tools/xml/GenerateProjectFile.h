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
    bool    enabledByDefault;

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
    vector<QString> defaultgfiles;
    vector<QString> defaultsfiles;
    vector<QString> defaultvfiles;
    vector<QString> defaultmfiles;
    vector<QString> defaultefiles;
    vector<QString> defaultwfiles;

    Attribute *atts;
  public:
    ProjectFileGeneratorPlugin(const QString &n,const QString &l,const QString &t,const QString &vt,const QString &dt,const QString &v, const QString&w, bool hw)
        : name(n), type(t), label(l), version(v), vartype(vt), dbtype(dt), haswriter(hw), atts(NULL)
    {
        enabledByDefault = true;
        customgfiles = false;
        customsfiles = false;
        customvfiles = false;
        custommfiles = false;
        customefiles = false;
        customwfiles = false;
        gfiles.clear();
        sfiles.clear();
        vfiles.clear();
        mfiles.clear();
        efiles.clear();
        wfiles.clear();
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
        bool  readSuccess = false;
        QString regkey;
        HKEY hkey;

        /* Try and read the key from the system registry. */
        regkey.sprintf("VISIT%s", VERSION);
        *keyval = new unsigned char[500];
        if(RegOpenKeyEx(HKEY_CLASSES_ROOT, regkey.latin1(), 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
        {
            DWORD keyType, strSize = 500;
            if(RegQueryValueEx(hkey, key, NULL, &keyType, *keyval, &strSize) == ERROR_SUCCESS)
            {
                readSuccess = true;
            }

            RegCloseKey(hkey);
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
            retval = QString((char *)VISITDEVDIR) + QString("\\projects\\databases\\");
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

    void WriteProject_TOP_LEVEL(ostream &out)
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

    void WriteProjectWorkspace(ostream &out, const vector<QString> &projects)
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
        for(i = 0; i < projects.size(); ++i)
        {
            out << "    Begin Project Dependency" << endl;
            out << "    Project_Dep_Name " << projects[i] << endl;
            out << "    End Project Dependency" << endl;
        }

        out << "}}}" << endl;
        out << "" << endl;

        for(i = 0; i < projects.size(); ++i)
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

    void WriteProjectHelper(ostream &out, const QString &pluginType, char pluginComponent,
        const QString &exports, const QString &libs, const vector<QString> &srcFiles)
    {
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
        out << "# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I \"..\\..\\visit\\" << pluginType << "\\" << name << "\" /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"USING_MSVC6\" ";
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
        out << "# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib " << libs << " /nologo /dll /machine:I386 /out:\"Release/lib" << pluginComponent << name << ".dll\"" << endl;
        out << "# Begin Special Build Tool" << endl;
        out << "SOURCE=\"$(InputPath)\"" << endl;
        out << "PostBuild_Cmds=copy Release\\lib" << pluginComponent << name << ".dll ..\\..\\bin\\Release\\" << pluginType << "" << endl;
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
        out << "# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /YX /FD /GZ /c" << endl;
        out << "# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I \"..\\..\\visit\\" << pluginType << "\\" << name << "\" /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"USING_MSVC6\" ";
        if(exports != "")
            out << "/D \"" << exports << "\" " << endl;
        out << "/D \"GENERAL_PLUGIN_EXPORTS\" /YX /FD /GZ /TP /c" << endl;
        out << "# ADD BASE MTL /nologo /D \"_DEBUG\" /mktyplib203 /win32" << endl;
        out << "# ADD MTL /nologo /D \"_DEBUG\" /mktyplib203 /win32" << endl;
        out << "# ADD BASE RSC /l 0x409 /d \"_DEBUG\"" << endl;
        out << "# ADD RSC /l 0x409 /d \"_DEBUG\"" << endl;
        out << "BSC32=bscmake.exe" << endl;
        out << "# ADD BASE BSC32 /nologo" << endl;
        out << "# ADD BSC32 /nologo" << endl;
        out << "LINK32=link.exe" << endl;
        out << "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept" << endl;
        out << "# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib " << libs << " /nologo /dll /debug /machine:I386 /out:\"Debug/lib" << pluginComponent << name << ".dll\" /pdbtype:sept" << endl;
        out << "# Begin Special Build Tool" << endl;
        out << "SOURCE=\"$(InputPath)\"" << endl;
        out << "PostBuild_Cmds=copy Debug\\lib" << pluginComponent << name << ".dll ..\\..\\bin\\Debug\\" << pluginType << endl;
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

    /***************************************************************************
     ************************ BEGIN PLOT PROJECT CODING ************************
     **************************************************************************/

    void WritePlotProject_I(ostream &out)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");

        WriteProjectHelper(out, "plots", 'I', "",
            "plugin.lib", srcFiles);
    }

    void WritePlotProject_E(ostream &out)
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
            "state.lib misc.lib plugin.lib plotter.lib pipeline_ser.lib avtexceptions.lib viewer.lib dbatts.lib visit_vtk.lib vtkCommon.lib vtkGraphics.lib vtkFiltering.lib", srcFiles);
    }

    void WritePlotProject_G(ostream &out)
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
            "state.lib misc.lib plugin.lib gui.lib viewerproxy.lib viewerrpc.lib qt-mt302.lib", srcFiles);
    }

    void WritePlotProject_S(ostream &out)
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
            "state.lib misc.lib plugin.lib", srcFiles);
    }

    void WritePlotProject_V(ostream &out)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");
        srcFiles.push_back(name + "CommonPluginInfo.C");
        srcFiles.push_back(name + "ViewerPluginInfo.C");
        srcFiles.push_back("avt"+name + "Plot.C");
        srcFiles.push_back(atts->name + ".C");

        if (customvfiles)
            AddElements(srcFiles, vfiles);
        else
            AddElements(srcFiles, defaultvfiles);

        WriteProjectHelper(out, "plots", 'V', "VIEWER_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib plotter.lib pipeline_ser.lib avtexceptions.lib viewer.lib dbatts.lib visit_vtk.lib vtkCommon.lib vtkGraphics.lib vtkFiltering.lib", srcFiles);
    }

    void WritePlotProjects(bool (*openCB)(ofstream &, const QString &))
    {
        QString projectDir(ProjectDir());
        QString IProject(projectDir + name + "I.dsp");
        QString EProject(projectDir + name + "E.dsp");
        QString GProject(projectDir + name + "G.dsp");
        QString SProject(projectDir + name + "S.dsp");
        QString VProject(projectDir + name + "V.dsp");
        QString pluginTopProject(projectDir + name + ".dsp");
        QString workspace(projectDir + name + ".dsw");

        // Write the I project
        ofstream out;
        if(openCB(out, IProject))
        {
            WritePlotProject_I(out);
            out.close();
        }

        // Write the E project
        if(openCB(out, EProject))
        {
            WritePlotProject_E(out);
            out.close();
        }

        // Write the G project
        if(openCB(out, GProject))
        {
            WritePlotProject_G(out);
            out.close();
        }

        // Write the S project
        if(openCB(out, SProject))
        {
            WritePlotProject_S(out);
            out.close();
        }

        // Write the V project
        if(openCB(out, VProject))
        {
            WritePlotProject_V(out);
            out.close();
        }

        // Write the top level project
        if(openCB(out, pluginTopProject))
        {
            WriteProject_TOP_LEVEL(out);
            out.close();
        }

        // Write the plugin workspace.
        if(openCB(out, workspace))
        {
            vector<QString> projects;
            projects.push_back(name + "I");
            projects.push_back(name + "E");
            projects.push_back(name + "G");
            projects.push_back(name + "S");
            projects.push_back(name + "V");
            WriteProjectWorkspace(out, projects);
        }
    }

    /***************************************************************************
     ************************* END PLOT PROJECT CODING *************************
     **************************************************************************/

    /***************************************************************************
     ********************** BEGIN OPERATOR PROJECT CODING **********************
     **************************************************************************/

    void WriteOperatorProject_I(ostream &out)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");

        WriteProjectHelper(out, "operators", 'I', "",
            "plugin.lib", srcFiles);
    }

    void WriteOperatorProject_E(ostream &out)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");
        srcFiles.push_back(name + "CommonPluginInfo.C");
        srcFiles.push_back(atts->name + ".C");

        srcFiles.push_back(name + "EnginePluginInfo.C");
        srcFiles.push_back("avt" + name + "Operator.C");

        if(customefiles)
            AddElements(srcFiles, efiles);
        else
            AddElements(srcFiles, defaultefiles);

        WriteProjectHelper(out, "operators", 'E', "ENGINE_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib plotter.lib pipeline_ser.lib avtexceptions.lib viewer.lib dbatts.lib visit_vtk.lib vtkCommon.lib vtkGraphics.lib vtkFiltering.lib", srcFiles);
    }

    void WriteOperatorProject_G(ostream &out)
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
            "state.lib misc.lib plugin.lib gui.lib viewerproxy.lib viewerrpc.lib qt-mt302.lib", srcFiles);
    }

    void WriteOperatorProject_S(ostream &out)
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
            "state.lib misc.lib plugin.lib", srcFiles);
    }

    void WriteOperatorProject_V(ostream &out)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");
        srcFiles.push_back(name + "CommonPluginInfo.C");
        srcFiles.push_back(name + "ViewerPluginInfo.C");
        srcFiles.push_back("avt"+name + "Operator.C");
        srcFiles.push_back(atts->name + ".C");

        if (customvfiles)
            AddElements(srcFiles, vfiles);
        else
            AddElements(srcFiles, defaultvfiles);

        WriteProjectHelper(out, "operators", 'V', "VIEWER_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib Operatorter.lib pipeline_ser.lib avtexceptions.lib viewer.lib dbatts.lib visit_vtk.lib vtkCommon.lib vtkGraphics.lib vtkFiltering.lib", srcFiles);
    }

    void WriteOperatorProjects(bool (*openCB)(ofstream &, const QString &))
    {
        QString projectDir(ProjectDir());
        QString IProject(projectDir + name + "I.dsp");
        QString EProject(projectDir + name + "E.dsp");
        QString GProject(projectDir + name + "G.dsp");
        QString SProject(projectDir + name + "S.dsp");
        QString VProject(projectDir + name + "V.dsp");
        QString pluginTopProject(projectDir + name + ".dsp");
        QString workspace(projectDir + name + ".dsw");

        // Write the I project
        ofstream out;
        if(openCB(out, IProject))
        {
            WriteOperatorProject_I(out);
            out.close();
        }

        // Write the E project
        if(openCB(out, EProject))
        {
            WriteOperatorProject_E(out);
            out.close();
        }

        // Write the G project
        if(openCB(out, GProject))
        {
            WriteOperatorProject_G(out);
            out.close();
        }

        // Write the S project
        if(openCB(out, SProject))
        {
            WriteOperatorProject_S(out);
            out.close();
        }

        // Write the V project
        if(openCB(out, VProject))
        {
            WriteOperatorProject_V(out);
            out.close();
        }

        // Write the top level project
        if(openCB(out, pluginTopProject))
        {
            WriteProject_TOP_LEVEL(out);
            out.close();
        }

        // Write the plugin workspace.
        if(openCB(out, workspace))
        {
            vector<QString> projects;
            projects.push_back(name + "I");
            projects.push_back(name + "E");
            projects.push_back(name + "G");
            projects.push_back(name + "S");
            projects.push_back(name + "V");
            WriteProjectWorkspace(out, projects);
        }
    }

    /***************************************************************************
     *********************** END OPERATOR PROJECT CODING ***********************
     **************************************************************************/

    /***************************************************************************
     ********************** BEGIN DATABASE PROJECT CODING **********************
     **************************************************************************/

    void WriteDatabaseProject_EM_Helper(ostream &out, char pluginType, const vector<QString> &srcFiles)
    {
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
        out << "# PROP Intermediate_Dir \"Release\"\n";
        out << "# PROP Ignore_Export_Lib 0\n";
        out << "# PROP Target_Dir \"\"\n";
        out << "# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"" << name << pluginType << "_EXPORTS\" /YX /FD /c\n";
        out << "# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I \"..\\..\\visit\\databases\\" << name << "\" /D \"NDEBUG\" /D \"WIN32\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"GENERAL_PLUGIN_EXPORTS\" /D \"ENGINE_PLUGIN_EXPORTS\" /D \"USING_MSVC6\" /YX /FD /TP /c\n";
        out << "# ADD BASE MTL /nologo /D \"NDEBUG\" /mktyplib203 /win32\n";
        out << "# ADD MTL /nologo /D \"NDEBUG\" /mktyplib203 /win32\n";
        out << "# ADD BASE RSC /l 0x409 /d \"NDEBUG\"\n";
        out << "# ADD RSC /l 0x409 /d \"NDEBUG\"\n";
        out << "BSC32=bscmake.exe\n";
        out << "# ADD BASE BSC32 /nologo\n";
        out << "# ADD BSC32 /nologo\n";
        out << "LINK32=link.exe\n";
        out << "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386\n";
        out << "# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  state.lib misc.lib plugin.lib pipeline_ser.lib dbatts.lib database_ser.lib avtexceptions.lib vtkCommon.lib /nologo /dll /machine:I386 /out:\"Release/lib" << pluginType << name << "Database_ser.dll\"\n";
        out << "# Begin Special Build Tool\n";
        out << "SOURCE=\"$(InputPath)\"\n";
        out << "PostBuild_Cmds=copy Release\\lib" << pluginType << name << "Database_ser.dll ..\\..\\bin\\Release\\databases\n";
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
        out << "# PROP Intermediate_Dir \"Debug\"\n";
        out << "# PROP Ignore_Export_Lib 0\n";
        out << "# PROP Target_Dir \"\"\n";
        out << "# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"" << name << pluginType << "_EXPORTS\" /YX /FD /GZ /c\n";
        out << "# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I \"..\\..\\visit\\databases\\" << name << "\" /D \"_DEBUG\" /D \"WIN32\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"GENERAL_PLUGIN_EXPORTS\" /D \"ENGINE_PLUGIN_EXPORTS\" /D \"USING_MSVC6\" /YX /FD /GZ /TP /c\n";
        out << "# ADD BASE MTL /nologo /D \"_DEBUG\" /mktyplib203 /win32\n";
        out << "# ADD MTL /nologo /D \"_DEBUG\" /mktyplib203 /win32\n";
        out << "# ADD BASE RSC /l 0x409 /d \"_DEBUG\"\n";
        out << "# ADD RSC /l 0x409 /d \"_DEBUG\"\n";
        out << "BSC32=bscmake.exe\n";
        out << "# ADD BASE BSC32 /nologo\n";
        out << "# ADD BSC32 /nologo\n";
        out << "LINK32=link.exe\n";
        out << "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept\n";
        out << "# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib state.lib misc.lib plugin.lib pipeline_ser.lib dbatts.lib database_ser.lib avtexceptions.lib vtkCommon.lib /nologo /dll /debug /machine:I386 /out:\"Debug/lib" << pluginType << name << "Database_ser.dll\" /pdbtype:sept\n";
        out << "# Begin Special Build Tool\n";
        out << "SOURCE=\"$(InputPath)\"\n";
        out << "PostBuild_Cmds=copy Debug\\lib" << pluginType << name << "Database_ser.dll ..\\..\\bin\\Debug\\databases\n";
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

    void WriteDatabaseProject_E(ostream &out)
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
        WriteDatabaseProject_EM_Helper(out, 'E', srcFiles);
    }

    void WriteDatabaseProject_M(ostream &out)
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
        WriteDatabaseProject_EM_Helper(out, 'M', srcFiles);
    }

    void WriteDatabaseProject_I(ostream &out)
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
        out << "# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I \"..\\..\\visit\\databases\\" << name << "\" /D \"NDEBUG\" /D \"WIN32\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"GENERAL_PLUGIN_EXPORTS\" /D \"USING_MSVC6\" /YX /FD /TP /c\n";
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
        out << "# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I \"..\\..\\visit\\databases\\" << name << "\" /D \"_DEBUG\" /D \"WIN32\" /D \"_WINDOWS\" /D \"_MBCS\" /D \"_USRDLL\" /D \"GENERAL_PLUGIN_EXPORTS\" /D \"USING_MSVC6\" /YX /FD /GZ /TP /c\n";
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

    void WriteDatabaseProjects(bool (*openCB)(ofstream &, const QString &))
    {
        QString projectDir(ProjectDir());
        QString IProject(projectDir + name + "I.dsp");
        QString EProject(projectDir + name + "E.dsp");
        QString MProject(projectDir + name + "M.dsp");
        QString pluginTopProject(projectDir + name + ".dsp");
        QString workspace(projectDir + name + ".dsw");

        // Write the I project
        ofstream out;
        if(openCB(out, IProject))
        {
            WriteDatabaseProject_I(out);
            out.close();
        }

        // Write the E project
        if(openCB(out, EProject))
        {
            WriteDatabaseProject_E(out);
            out.close();
        }

        // Write the M project
        if(openCB(out, MProject))
        {
            WriteDatabaseProject_M(out);
            out.close();
        }

        // Write the top level project
        if(openCB(out, pluginTopProject))
        {
            WriteProject_TOP_LEVEL(out);
            out.close();
        }

        // Write the plugin workspace.
        if(openCB(out, workspace))
        {
            vector<QString> projects;
            projects.push_back(name + "I");
            projects.push_back(name + "E");
            projects.push_back(name + "M");
            WriteProjectWorkspace(out, projects);
        }
    }

    /***************************************************************************
     *********************** END DATABASE PROJECT CODING ***********************
     **************************************************************************/
public:

    void WriteProjectFiles(bool (*openCB)(ofstream &, const QString &))
    {
        if (type == "operator")
        {
            WriteOperatorProjects(openCB);
        }
        else if (type == "plot")
        {
            WritePlotProjects(openCB);
        }
        else if (type == "database")
        {
            WriteDatabaseProjects(openCB);
        }
    }
};


// ----------------------------------------------------------------------------
//                           Override default types
// ----------------------------------------------------------------------------
#define Plugin       ProjectFileGeneratorPlugin

#endif
