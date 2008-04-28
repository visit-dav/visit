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
#include <shlobj.h>
#include <shlwapi.h>
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
//    Hank Childs, Thu Jan 10 14:33:30 PST 2008
//    Added filenames, specifiedFilenames.
//
//    Brad Whitlock, Thu Mar 6 11:50:13 PDT 2008
//    Made it use the Plugin base class.
//
//    Kathleen Bonnell, Thu Apr 17 09:55:07 PDT 2008 
//    Moved comipler-version specific code to its own .h file.  Removed
//    references to Version6 in favor of Version8 (to be added soon.) 
//    Added BaseDirs method to setup certain directory paths correctly
//    depending on whether or not this is being run on windows, and whether
//    or not this is being run from a development version.  Refactored
//    so that project files could be generated for use with a public
//    (binary) version of VisIt on windows.  
//    
//
// ****************************************************************************

class ProjectFileGeneratorPlugin : public Plugin
{
  public:
    ProjectFileGeneratorPlugin(const QString &n,const QString &l,
             const QString &t,const QString &vt,const QString &dt,
             const QString &v, const QString&ifile, bool hw, bool ho, 
             bool onlyengine, bool noengine)
        :  Plugin(n,l,t,vt,dt,v,ifile,hw,ho,onlyengine,noengine)
    {
    }

    void Print(ostream &out)
    {
        out << "Plugin: " << name << " (\"" << label << "\", type=" << type
            << ") -- version " << version << endl;
        if (atts)
            atts->Print(out);
    }

protected:

    QString preproc;
    QString includeBase;
    QString pluginBase;
    QString binBase;
    QString libBase;
    bool withinDevDir;
    bool publicVisIt;

#if defined(_WIN32)
    bool ReadKey(const char *key, unsigned char **keyval) const
    {
        bool retval = false;
        *keyval = 0;

        if((retval = ReadKeyFromRoot(HKEY_CLASSES_ROOT, key, keyval)) == false)
            retval = ReadKeyFromRoot(HKEY_CURRENT_USER, key, keyval);

        return retval;
    }

    bool ReadKeyFromRoot(HKEY which_root, const char *key, 
                         unsigned char **keyval) const
    {
        bool  readSuccess = false;
        QString regkey;
        HKEY hkey;

        /* Try and read the key from the system registry. */
        regkey.sprintf("VISIT%s", VERSION);
        *keyval = new unsigned char[500];
        if(RegOpenKeyEx(which_root, regkey.latin1(), 0, KEY_QUERY_VALUE, &hkey)
               == ERROR_SUCCESS)
        {
            DWORD keyType, strSize = 500;
            if(RegQueryValueEx(hkey, key, NULL, &keyType, *keyval, &strSize) 
                   == ERROR_SUCCESS)
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

    void BaseDirs(const QString &projectDir, bool version7)
    {
#if defined(_WIN32)
        withinDevDir = false;
        if (!publicVisIt)
        {
            if (fullCurrentDir.contains(projectDir, false) > 0)
            {
                withinDevDir = true;
            }
        } 

        if (withinDevDir)
        {
            includeBase = "..\\..\\include";
            pluginBase  = "..\\..\\visit";
            binBase     = "..\\..\\bin";
            libBase     = "..\\..\\lib";
            if (version7)
            {
                binBase += "\\MSVC7.Net\\$(ConfigurationName)";
                libBase += "\\MSVC7.Net\\$(ConfigurationName)";
            }
        }
        else
        {
            // find location for private plugin storage
            char *tmp = new char[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL,
                              SHGFP_TYPE_CURRENT, tmp)))
            {
                PathAppend(tmp, "LLNL");
                PathAppend(tmp, "VisIt");
                privatePluginDir = tmp;
            }
            else
            {
                cerr << "GenerateProjectfile, get Appdata path failed " 
                       << endl;
                privatePluginDir = "";
            }
            delete [] tmp;

            if (!publicVisIt)
            {
                includeBase = projectDir + "\\include";
                libBase     = projectDir + "\\lib";
                if (version7)
                    libBase     += "\\MSVC7.Net\\$(ConfigurationName)";
            }
            else
            {
                includeBase = fullVisItDir + "\\include";
                libBase     = fullVisItDir + "\\lib";
            }
            pluginBase  = ".\\";
            binBase     = privatePluginDir;
        }
#else
        includeBase = "..\\..\\include";
        pluginBase  = "..\\..\\visit";
        binBase     = "..\\..\\bin";
        libBase     = "..\\..\\lib";
        
        withinDevDir = true;
        if (version7)
        {
            binBase += "\\MSVC7.Net\\$(ConfigurationName)";
            libBase += "\\MSVC7.Net\\$(ConfigurationName)";
        }
#endif
    }

    QString ProjectDir(bool version7) 
    {
#if defined(_WIN32)
        unsigned char *VISITDEVDIR = 0;
        withinDevDir = false;
        QString retval;

        if(ReadKey("VISITDEVDIR", &VISITDEVDIR))
        {
            retval = QString((char *)VISITDEVDIR);
            delete [] VISITDEVDIR;
        }
        else
        {
            const char *visitDevDirEnv = getenv("VISITDEVDIR");
            if (visitDevDirEnv)
            {
                retval = visitDevDirEnv;
            }
        }
         
        if (retval.length() > 0)
        {
            if (fullVisItDir.contains(retval, false) > 0)
                publicVisIt = false;
            else
                publicVisIt = true;
        } 
        else
        {
            publicVisIt = true;
        }
       
        // Make sure that it ends with a separator.
        if(retval.length() > 0 && retval.right(1) != SLASH_STRING)
        {
            retval += SLASH_STRING;
        }
       
        if (retval.length() == 0)
        {
            retval = "";
        } 

        BaseDirs(retval, version7);
        if (!withinDevDir)
            retval = "";

        return retval;
#else
        BaseDirs(QString(""), version7);
        return QString("");
#endif
    }

    void AddElements(vector<QString> &addTo, const vector<QString> &addFrom)
    {
        for(int i = 0; i < addFrom.size(); ++i)
            addTo.push_back(addFrom[i]);
    }

#include <GenerateVC7.h>
#if 0
#include <GenerateVC8.h>
#endif


    void WriteProject_TOP_LEVEL(ostream &out, bool version7)
    {
        if(version7)
            WriteProject_TOP_LEVEL_Version7(out);
#if 0
        else
            WriteProject_TOP_LEVEL_Version8(out);
#endif
    }

    void WriteProjectWorkspace(ostream &out, const vector<QString> &projects, 
                               bool version7)
    {
        if(version7)
            WriteProjectWorkspace_Version7(out, projects);
#if 0
        else
            WriteProjectWorkspace_Version8(out, projects);
#endif
    }

    void WriteProjectHelper(ostream &out, const QString &pluginType, 
             char pluginComponent, const QString &exports, const QString &libs,
             const vector<QString> &srcFiles, bool version7)
    {
        if(version7)
            WriteProjectHelper_Version7(out, pluginType, pluginComponent, 
                                        exports, libs, srcFiles);
#if 0
        else
            WriteProjectHelper_Version8(out, pluginType, pluginComponent, 
                                        exports, libs, srcFiles);
#endif
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
            "state.lib misc.lib plugin.lib plotter.lib pipeline_ser.lib "
            "avtfilters.lib avtexceptions.lib viewer.lib dbatts.lib"
            " visit_vtk.lib vtkCommon.lib vtkGraphics.lib vtkFiltering.lib",
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
                    srcFiles.push_back(wfiles[i].left(
                        wfiles[i].length() - 2) + ".C");
                    srcFiles.push_back(wfiles[i].left(
                        wfiles[i].length() - 2) + "_moc.C");
                }
            }
        }
        else
        {
            for (int i=0; i<defaultwfiles.size(); i++)
            {
                if(defaultwfiles[i].right(2) == ".h")
                {
                    srcFiles.push_back(defaultwfiles[i].left(
                        defaultwfiles[i].length() - 2) + ".C");
                    srcFiles.push_back(defaultwfiles[i].left(
                        defaultwfiles[i].length() - 2) + "_moc.C");
                }
            }
        }

        if (customgfiles)
            AddElements(srcFiles, gfiles);
        else
            AddElements(srcFiles, defaultgfiles);

        WriteProjectHelper(out, "plots", 'G', "GUI_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib gui.lib viewerproxy.lib "
            "viewerrpc.lib $(QTLIB)", srcFiles, version7);
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
                    srcFiles.push_back(vwfiles[i].left(
                        vwfiles[i].length() - 2) + ".C");
                    srcFiles.push_back(vwfiles[i].left(
                        vwfiles[i].length() - 2) + "_moc.C");
                }
            }
        }

        if (customvfiles)
            AddElements(srcFiles, vfiles);
        else
            AddElements(srcFiles, defaultvfiles);

        WriteProjectHelper(out, "plots", 'V', "VIEWER_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib plotter.lib pipeline_ser.lib "
            "avtfilters.lib avtexceptions.lib viewer.lib dbatts.lib "
            "visit_vtk.lib vtkCommon.lib vtkGraphics.lib vtkFiltering.lib",
            srcFiles, version7);
    }

    void WritePlotProjects(bool (*openCB)(ofstream &, const QString &), 
                           bool version7)
    {
        QString projectExtension(".dsp");
        QString workSpaceExtension(".dsw");
        QString projectDir(ProjectDir(version7));

        if(version7)
        {
            projectExtension = ".vcproj";
            workSpaceExtension = ".sln";
        }
#if defined(_WIN32)
        if(version7)
        {
            if (withinDevDir)
                projectDir += QString("projects-MSVC7.Net\\plots\\");
        }
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
            "state.lib misc.lib plugin.lib plotter.lib avtfilters.lib "
            "pipeline_ser.lib avtexceptions.lib viewer.lib dbatts.lib "
            "visit_vtk.lib vtkCommon.lib vtkGraphics.lib vtkFiltering.lib",
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
                    srcFiles.push_back(wfiles[i].left(
                        wfiles[i].length() - 2) + ".C");
                    srcFiles.push_back(wfiles[i].left(
                        wfiles[i].length() - 2) + "_moc.C");
                }
            }
        }
        else
        {
            for (int i=0; i<defaultwfiles.size(); i++)
            {
                if(defaultwfiles[i].right(2) == ".h")
                {
                    srcFiles.push_back(defaultwfiles[i].left(
                        defaultwfiles[i].length() - 2) + ".C");
                    srcFiles.push_back(defaultwfiles[i].left(
                        defaultwfiles[i].length() - 2) + "_moc.C");
                }
            }
        }

        if (customgfiles)
            AddElements(srcFiles, gfiles);
        else
            AddElements(srcFiles, defaultgfiles);

        WriteProjectHelper(out, "operators", 'G', "GUI_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib gui.lib viewerproxy.lib "
            "viewerrpc.lib $(QTLIB)", srcFiles, version7);
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
                    srcFiles.push_back(vwfiles[i].left(
                        vwfiles[i].length() - 2) + ".C");
                    srcFiles.push_back(vwfiles[i].left(
                        vwfiles[i].length() - 2) + "_moc.C");
                }
            }
        }

        if (customvfiles)
            AddElements(srcFiles, vfiles);
        else
            AddElements(srcFiles, defaultvfiles);

        WriteProjectHelper(out, "operators", 'V', "VIEWER_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib plotter.lib avtfilters.lib "
            "pipeline_ser.lib avtexceptions.lib viewer.lib dbatts.lib "
            "visit_vtk.lib vtkCommon.lib vtkGraphics.lib vtkFiltering.lib",
            srcFiles, version7);
    }

    void WriteOperatorProjects(bool (*openCB)(ofstream &, const QString &), 
                               bool version7)
    {
        QString projectExtension(".dsp");
        QString workSpaceExtension(".dsw");
        QString projectDir(ProjectDir(version7));
        if(version7)
        {
            projectExtension = ".vcproj";
            workSpaceExtension = ".sln";
        }
#if defined(_WIN32)
        if(version7)
        {
            if (withinDevDir)
                projectDir += QString("projects-MSVC7.Net\\operators\\");
        }
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

    void WriteDatabaseProject_E(ostream &out, bool version7, 
                                const QString &libs)
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
            WriteDatabaseProject_EM_Helper_Version7(out, 'E', srcFiles, libs);
#if 0
        else
            WriteDatabaseProject_EM_Helper_Version8(out, 'E', srcFiles);
#endif
    }

    void WriteDatabaseProject_M(ostream &out, bool version7, 
                                const QString &libs)
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
            WriteDatabaseProject_EM_Helper_Version7(out, 'M', srcFiles, libs);
#if 0
        else
            WriteDatabaseProject_EM_Helper_Version8(out, 'M', srcFiles);
#endif
    }

    void WriteDatabaseProjects(bool (*openCB)(ofstream &, const QString &), 
                               bool version7)
    {
        QString projectExtension(".dsp");
        QString workSpaceExtension(".dsw");
        QString projectDir(ProjectDir(version7));
        QString baseLibs("state.lib misc.lib plugin.lib pipeline_ser.lib "
                         "dbatts.lib database_ser.lib avtexceptions.lib");
        if(version7)
        {
            projectExtension = ".vcproj";
            workSpaceExtension = ".sln";
        }

#if defined(_WIN32)
        if(version7)
        {
            if (withinDevDir)
                projectDir += QString("projects-MSVC7.Net\\databases\\");
        }
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
                WriteDatabaseProject_I_Version7(out, baseLibs);
#if 0
            else
                WriteDatabaseProject_I_Version8(out);
#endif
            out.close();
        }

        // Write the E project
        if(openCB(out, EProject))
        {
            WriteDatabaseProject_E(out, version7, baseLibs);
            out.close();
        }

        // Write the M project
        if(openCB(out, MProject))
        {
            WriteDatabaseProject_M(out, version7, baseLibs);
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

    void WriteProjectFiles(bool (*openCB)(ofstream &, const QString &), 
                           bool version7)
    {
        preproc = "WIN32;_WINDOWS;_USRDLL;";
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
