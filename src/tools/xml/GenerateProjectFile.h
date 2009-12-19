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

#ifndef GENERATE_PROJECTFILE_H
#define GENERATE_PROJECTFILE_H
#include <QTextStream>

#include "Field.h"
#include <visit-config.h> // for the plugin extension.

#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <iostream>
//using std::cerr;
//using std::endl;
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
//    Kathleen Bonnell, Mon Jun 2 18:12:57 PDT 2008 
//    Reworked due to windows-build source tree change to use same tree as 
//    stored in SVN, and the necessity of specifying individual include 
//    directories.  Added ability to set third party information from
//    xml file. 
//
//    Kathleen Bonnell, Fri Jun 20 08:18:07 PDT 2008 
//    Allow creation of projects for MSVC8.
//
//    Kathleen Bonnell, Tue Aug 19 10:28:23 PDT 2008 
//    Modified how moc files are handled.
//
//    Brad Whitlock, Tue Nov 18 16:16:37 PST 2008
//    Qt 4.
//
//    Kathleen Bonnell, Wed Dec 17 15:23:05 MST 2008
//    Change qt includes/libs to match QT 4.
//
//    Kathleen Bonnell, Wed May 27 9:05:15 MST 2009
//    Added support for custom windows files for mdserver and engine.
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

    void Print(QTextStream &out)
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
    QString visitIncludes;
    QString qtBase; 
    QString tpIncludes;
    QString tpLibs;
    QString tpLibDir;
    bool withinDevDir;
    bool publicVisIt;

    QString CreateKey() const
    {
        const char *digits = "0123456789ABCDEF";
        QString s("------------------------------------");
        for(int i = 0; i < (int)s.length(); ++i)
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
        regkey.sprintf("VISIT%s", VISIT_VERSION);
        if (*keyval == NULL)
            *keyval = new unsigned char[500];
        if(RegOpenKeyEx(which_root, regkey.toStdString().c_str(), 0, 
                        KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
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
    void ThirdParty(QString &tplibs, QString &tpincs, QString &tppreprocs,
                    bool version7)
    {
        tplibs = "";
        tpincs = "";
        tppreprocs = "";
        QString baseInclude = ";..\\..\\include\\";
        if (libs.size() > 0)
        {
            for (size_t i = 0; i < libs.size(); ++i)
            {
                if (libs[i].indexOf("BOXLIB2D", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " boxlib2D.lib";
                    tppreprocs += ";BL_SPACEDIM=2;BL_FORT_USE_UPPERCASE";
                }
                else if (libs[i].indexOf("BOXLIB3D", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " boxlib3D.lib";
                    tppreprocs += ";BL_SPACEDIM=3;BL_FORT_USE_UPPERCASE";
                }
                else if (libs[i].indexOf("CCMIO", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " ccmio.lib";
                }
                else if (libs[i].indexOf("CFITSIO", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " cfitsio.lib";
                    tppreprocs += ";_HDF5USEDLL_";
                }
                else if (libs[i].indexOf("CGNS", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " cgnslib.lib";
                }
                else if (libs[i].indexOf("EXODUS", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " exodusII.lib";
                }
                else if (libs[i].indexOf("GDAL", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " gdal_i.lib";
                }
                else if (libs[i].indexOf("HDF4", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " hd421m.lib hm421m.lib";
                    tppreprocs += ";_MFHDFLIB_;_HDFLIB_";
                }
                else if (libs[i].indexOf("HDF5", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " hdf5dll.lib";
                    tppreprocs += ";_HDF5USEDLL_";
                }
                else if (libs[i].indexOf("H5PART", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " h5part13.lib";
                }
                else if (libs[i].indexOf("NETCDF_CXX", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " netcdf_c++.lib";
                }
                else if (libs[i].indexOf("NETCDF", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " netcdf.lib";
                }
                else if (libs[i].indexOf("PDB", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " silohdf5.lib";
                }
                else if (libs[i].indexOf("SILO", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " silohdf5.lib";
                }
                else if (libs[i].indexOf("ZLIB", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " zlib1.lib";
                }
                else if (libs[i].indexOf("GLEW", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " glew32.lib";
                }
                else if (libs[i].indexOf("GL_LIBS", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " opengl32.lib MesaGL32.lib";
                    tpincs = tpincs + baseInclude + "Mesa";
                }
            }
        }
        if (ldflags.size() > 0)
        {
            for (size_t i = 0; i < ldflags.size(); ++i)
            {
                if (ldflags[i].indexOf("lz", 0, Qt::CaseInsensitive) > 0)
                {
                    tplibs += " vtkzlib.lib";
                }
            }
        }
        if (cxxflags.size() > 0)
        {
            for (size_t i = 0; i < cxxflags.size(); ++i)
            {
                if (cxxflags[i].indexOf("BOXLIB2D", 0, Qt::CaseInsensitive) > 0)
                {
                    tpincs = tpincs + baseInclude + "boxlib";
                }
                else if (cxxflags[i].indexOf("BOXLIB3D", 0, Qt::CaseInsensitive) > 0)
                {
                    tpincs = tpincs + baseInclude + "boxlib";
                }
                else if (cxxflags[i].indexOf("CCMIO", 0, Qt::CaseInsensitive) > 0)
                {
                    tpincs = tpincs + baseInclude; 
                    tpincs = tpincs + baseInclude + "libccmio";
                }
                else if (cxxflags[i].indexOf("CFITSIO", 0, Qt::CaseInsensitive) > 0)
                {
                    tpincs = tpincs + baseInclude + "cfitsio";
                }
                else if (cxxflags[i].indexOf("CGNS", 0, Qt::CaseInsensitive) > 0)
                {
                    tpincs = tpincs + baseInclude + "cgns";
                }
                else if (cxxflags[i].indexOf("EXODUS", 0, Qt::CaseInsensitive) > 0)
                {
                    tpincs = tpincs + baseInclude + "exodusII";
                    tpincs = tpincs + baseInclude + "netcdf";
                }
                else if (cxxflags[i].indexOf("GDAL", 0, Qt::CaseInsensitive) > 0)
                {
                    tpincs = tpincs + baseInclude + "gdal";
                }
                else if (cxxflags[i].indexOf("HDF4", 0, Qt::CaseInsensitive) > 0)
                {
                    tpincs = tpincs + baseInclude + "hdf4";
                }
                else if (cxxflags[i].indexOf("HDF5", 0, Qt::CaseInsensitive) > 0)
                {
                    tpincs = tpincs + baseInclude + "hdf5";
                }
                else if (cxxflags[i].indexOf("H5PART", 0, Qt::CaseInsensitive) > 0)
                {
                    tpincs = tpincs + baseInclude + "h5part";
                }
                else if (cxxflags[i].indexOf("NETCDF", 0, Qt::CaseInsensitive) > 0)
                {
                    tpincs = tpincs + baseInclude + "netcdf";
                }
                else if (cxxflags[i].indexOf("PDB", 0, Qt::CaseInsensitive) > 0)
                {
                    tpincs = tpincs + baseInclude + "silo";
                }
                else if (cxxflags[i].indexOf("SILO", 0, Qt::CaseInsensitive) > 0)
                {
                    tpincs = tpincs + baseInclude + "silo";
                }
                else if (cxxflags[i].indexOf("GLEW", 0, Qt::CaseInsensitive) > 0)
                {
                    tpincs = tpincs + baseInclude + "glew";
                    tppreprocs += ";HAVE_LIBGLEW";
                }
            }
        }

    }

    void BaseDirs(const QString &projectDir, bool version7)
    {
#if defined(_WIN32)
        withinDevDir = false;
        if (!publicVisIt)
        {
            if (fullCurrentDir.contains(QString(projectDir), 
                                        Qt::CaseInsensitive) > 0)
            {
                withinDevDir = true;
            }
        } 
        if (withinDevDir)
        {
            includeBase = "..\\..\\include";
            pluginBase  = "..\\..\\..\\src";
            binBase     = "..\\..\\bin";
            libBase     = "..\\..\\lib";
            if (version7)
            {
                binBase += "\\MSVC7.Net";
                libBase += "\\MSVC7.Net";
            }
            else 
            {
                binBase += "\\MSVC8.Net";
                libBase += "\\MSVC8.Net";
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
                    libBase     += "\\MSVC7.Net";
                else 
                    libBase     += "\\MSVC8.Net";
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
        pluginBase  = "..\\..\\..\\src";
        binBase     = "..\\..\\bin";
        libBase     = "..\\..\\lib";
        
        withinDevDir = true;
        if (version7)
        {
            binBase += "\\MSVC7.Net";
            libBase += "\\MSVC7.Net";
        }
        else
        {
            binBase += "\\MSVC8.Net";
            libBase += "\\MSVC8.Net";
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
            if (fullVisItDir.contains(retval, Qt::CaseInsensitive) > 0)
                publicVisIt = false;
            else
                publicVisIt = true;
        } 
        else
        {
            publicVisIt = true;
        }
       
        // Make sure that it ends with a separator.
        if(retval.length() > 0 && retval.right(1) != VISIT_SLASH_STRING)
        {
            retval += VISIT_SLASH_STRING;
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
        for(size_t i = 0; i < addFrom.size(); ++i)
            addTo.push_back(addFrom[i]);
    }

#include <GenerateVC7.h>
#include <GenerateVC8.h>


    void WriteProject_TOP_LEVEL(QTextStream &out, bool version7)
    {
        if(version7)
            WriteProject_TOP_LEVEL_Version7(out);
        else
            WriteProject_TOP_LEVEL_Version8(out);
    }

    void WriteProjectSolution(QTextStream &out, const vector<QString> &projects, 
                               bool version7)
    {
        if(version7)
            WriteProjectSolution_Version7(out, projects);
        else
            WriteProjectSolution_Version8(out, projects);
    }
    void WriteProjectHelper(QTextStream &out, const QString &pluginType, 
             char pluginComponent, const QString &exports, const QString &libs,
             const vector<QString> &srcFiles, bool version7)
    {
        vector<QString> hdrFiles, mocFiles;
        WriteProjectHelper(out, pluginType, pluginComponent, exports, libs,
                           srcFiles, version7, hdrFiles, mocFiles);
    }

    void WriteProjectHelper(QTextStream &out, const QString &pluginType, 
             char pluginComponent, const QString &exports, const QString &libs,
             const vector<QString> &srcFiles, bool version7, const vector<QString> &hdrFiles, const vector<QString> &mocFiles)
    {
        if(version7)
            WriteProjectHelper_Version7(out, pluginType, pluginComponent, 
                                 exports, libs, srcFiles, hdrFiles, mocFiles);
        else
            WriteProjectHelper_Version8(out, pluginType, pluginComponent, 
                                 exports, libs, srcFiles, hdrFiles, mocFiles);
    }

    /***************************************************************************
     ************************ BEGIN PLOT PROJECT CODING ************************
     **************************************************************************/

    void WritePlotProject_I(QTextStream &out, bool version7)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");

        WriteProjectHelper(out, "plots", 'I', "",
            "plugin.lib", srcFiles, version7);
    }

    void WritePlotProject_E(QTextStream &out, bool version7)
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
        if(customwefiles)
            AddElements(srcFiles, wefiles);

        WriteProjectHelper(out, "plots", 'E', "ENGINE_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib plotter.lib pipeline_ser.lib "
            "avtfilters.lib avtexceptions.lib viewer.lib dbatts.lib"
            " visit_vtk.lib vtkCommon.lib vtkGraphics.lib vtkFiltering.lib",
            srcFiles, version7);
    }

    void WritePlotProject_G(QTextStream &out, bool version7)
    {
        vector<QString> srcFiles, hdrFiles, mocFiles;
        srcFiles.push_back(name + "PluginInfo.C");
        srcFiles.push_back(name + "CommonPluginInfo.C");
        srcFiles.push_back(name + "GUIPluginInfo.C");
        srcFiles.push_back(atts->name + ".C");
        if(customwfiles)
        {
            for (size_t i=0; i<wfiles.size(); i++)
            {
                if(wfiles[i].right(2) == ".h")
                {
                    hdrFiles.push_back(wfiles[i]);
                    mocFiles.push_back(wfiles[i].left(
                        wfiles[i].length() - 2) + "_moc.C");
                    srcFiles.push_back(wfiles[i].left(
                        wfiles[i].length() - 2) + ".C");
                }
            }
        }
        for (size_t i=0; i<defaultwfiles.size(); i++)
        {
            if(defaultwfiles[i].right(2) == ".h")
            {
                hdrFiles.push_back(defaultwfiles[i]);
                mocFiles.push_back(defaultwfiles[i].left(
                    defaultwfiles[i].length() - 2) + "_moc.C");
                srcFiles.push_back(defaultwfiles[i].left(
                    defaultwfiles[i].length() - 2) + ".C");
            }
        }

        if (customgfiles)
            AddElements(srcFiles, gfiles);
        else
            AddElements(srcFiles, defaultgfiles);

        WriteProjectHelper(out, "plots", 'G', "GUI_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib gui.lib viewerproxy.lib "
            "viewerrpc.lib winutil.lib", srcFiles, version7, hdrFiles, mocFiles);
    }

    void WritePlotProject_S(QTextStream &out, const QString &addLibs, bool version7)
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
            addLibs, srcFiles, version7);
    }

    void WritePlotProject_V(QTextStream &out, bool version7)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");
        srcFiles.push_back(name + "CommonPluginInfo.C");
        srcFiles.push_back(name + "ViewerPluginInfo.C");
        srcFiles.push_back("avt"+name + "Plot.C");
        srcFiles.push_back(atts->name + ".C");

        if(customvwfiles)
        {
            for (size_t i=0; i<vwfiles.size(); i++)
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

    void WritePlotProjects(QFile * (*openCB)(const QString &), 
                           bool version7)
    {
        QString projectExtension(".vcproj");
        QString workSpaceExtension(".sln");
        QString projectDir(ProjectDir(version7));
        QString baseLibs("");

#if defined(_WIN32)
        if (withinDevDir)
        {
            if(version7)
                projectDir += QString("windowsbuild\\projects-MSVC7.Net\\plots\\");
            else
                projectDir += QString("windowsbuild\\projects-MSVC8.Net\\plots\\");
        }
#endif
        QString IProject(projectDir + (name + "I") + projectExtension);
        QString EProject(projectDir + (name + "E") + projectExtension);
        QString GProject(projectDir + (name + "G") + projectExtension);
        QString SProject(projectDir + (name + "S") + projectExtension);
        QString VProject(projectDir + (name + "V") + projectExtension);
        QString pluginTopProject(projectDir + name + projectExtension);
        QString workspace(projectDir + name + workSpaceExtension);

        cOut << "Plot projects saved to " << projectDir << Endl;

        // Write the I project
        QFile *f = 0;
        if((f = openCB(IProject)) != 0)
        {
            QTextStream out(f);
            visitIncludes = pluginBase + "\\common\\misc;" +
                            pluginBase + "\\common\\plugin;"  +
                            pluginBase + "\\common\\state;"  +
                            pluginBase + "\\common\\utility";
            tpIncludes = "";
            tpLibs = "";
            tpLibDir = "";
            WritePlotProject_I(out, version7);
            f->close();
        }

        // Write the E project
        if((f = openCB(EProject)) != 0)
        {
            QTextStream out(f);
            visitIncludes = pluginBase + "\\avt\\Database\\Database;" +
                            pluginBase + "\\avt\\DBAtts\\MetaData;" +
                            pluginBase + "\\avt\\DBAtts\\SIL;" +
                            pluginBase + "\\avt\\Filters;" +
                            pluginBase + "\\avt\\Math;" +
                            pluginBase + "\\avt\\Pipeline\\AbstractFilters;" +
                            pluginBase + "\\avt\\Pipeline\\Data;" +
                            pluginBase + "\\avt\\Pipeline\\Pipeline;" +
                            pluginBase + "\\avt\\Pipeline\\Sinks;" +
                            pluginBase + "\\avt\\Pipeline\\Sources;" +
                            pluginBase + "\\avt\\Plotter;" +
                            pluginBase + "\\avt\\View;" +
                            pluginBase + "\\avt\\VisWindow\\VisWindow;" +
                            pluginBase + "\\common\\comm;" +
                            pluginBase + "\\common\\Exceptions\\Database;" +
                            pluginBase + "\\common\\Exceptions\\Pipeline;" +
                            pluginBase + "\\common\\Exceptions\\Plotter;" +
                            pluginBase + "\\common\\expr;" +
                            pluginBase + "\\common\\misc;" +
                            pluginBase + "\\common\\parser;" +
                            pluginBase + "\\common\\plugin;" +
                            pluginBase + "\\common\\state;" +
                            pluginBase + "\\common\\utility;" + 
                            pluginBase + "\\visit_vtk\\full;" + 
                            pluginBase + "\\visit_vtk\\lightweight"; 
            QString tpPreProc;
            ThirdParty(tpLibs, tpIncludes, tpPreProc, version7);
            tpLibDir = "";
            WritePlotProject_E(out, version7);
            f->close();
        }

        // Write the G project
        if((f = openCB(GProject)) != 0)
        {
            QTextStream out(f);
            visitIncludes = pluginBase + "\\avt\\DBAtts\\MetaData;" +
                            pluginBase + "\\avt\\DBAtts\\SIL;" +
                            pluginBase + "\\common\\Exceptions\\Pipeline;"  +
                            pluginBase + "\\common\\misc;" +
                            pluginBase + "\\common\\plugin;" +
                            pluginBase + "\\common\\state;" +
                            pluginBase + "\\common\\utility;" +
                            pluginBase + "\\gui;" +
                            pluginBase + "\\viewer\\proxy;" +
                            pluginBase + "\\viewer\\rpc;" +
                            pluginBase + "\\winutil"; 
            if (version7)
            {
                qtBase = "..\\..\\include\\qt";
                tpIncludes = qtBase + ";" +
                             qtBase + "\\QtCore;" + 
                             qtBase + "\\QtGui;" ;
            }
            else
            {
                tpIncludes = "";
            }
            tpLibs = " QtCore4.lib QtGui4.lib";
            WritePlotProject_G(out, version7);
            f->close();
        }

        // Write the S project
        if((f = openCB(SProject)) != 0)
        {
            QTextStream out(f);
            visitIncludes = pluginBase + "\\common\\misc;" +
                            pluginBase + "\\common\\plugin;" +
                            pluginBase + "\\common\\state;" +
                            pluginBase + "\\common\\utility;" +
                            pluginBase + "\\visitpy\\visitpy" ;
            tpIncludes = includeBase + "\\Python-2.5";
            tpLibs = "";
            tpLibDir = "";
            QString addLibs = "state.lib misc.lib plugin.lib";
            WritePlotProject_S(out, addLibs, version7);
            f->close();
        }

        // Write the V project
        if((f = openCB(VProject)) != 0)
        {
            QTextStream out(f);
            visitIncludes = pluginBase + "\\avt\\Database\\Database;" +
                            pluginBase + "\\avt\\DBAtts\\MetaData;" +
                            pluginBase + "\\avt\\DBAtts\\SIL;" +
                            pluginBase + "\\avt\\Filters;" +
                            pluginBase + "\\avt\\Math;" +
                            pluginBase + "\\avt\\Pipeline\\AbstractFilters;" +
                            pluginBase + "\\avt\\Pipeline\\Data;" +
                            pluginBase + "\\avt\\Pipeline\\Pipeline;" +
                            pluginBase + "\\avt\\Pipeline\\Sinks;" +
                            pluginBase + "\\avt\\Pipeline\\Sources;" +
                            pluginBase + "\\avt\\Plotter;" +
                            pluginBase + "\\avt\\View;" +
                            pluginBase + "\\avt\\Viswindow\\VisWindow;" +
                            pluginBase + "\\common\\comm;" +
                            pluginBase + "\\common\\Exceptions\\Database;" +
                            pluginBase + "\\common\\Exceptions\\Pipeline;" +
                            pluginBase + "\\common\\Exceptions\\Plotter;" +
                            pluginBase + "\\common\\expr;" +
                            pluginBase + "\\common\\misc;" +
                            pluginBase + "\\common\\parser;" +
                            pluginBase + "\\common\\plugin;" +
                            pluginBase + "\\common\\state;"  +
                            pluginBase + "\\common\\utility;"  +
                            pluginBase + "\\viewer\\main;"  +
                            pluginBase + "\\viewer\\rpc;"  +
                            pluginBase + "\\visit_vtk\\full;"  +
                            pluginBase + "\\visit_vtk\\lightweight;" + 
                            pluginBase + "\\winutil";
            QString tpPreProc;
            tpLibs = "";
            tpIncludes = "";
            ThirdParty(tpLibs, tpIncludes, tpPreProc, version7);
            if (tpIncludes.length() > 0)
                tpIncludes += ";";
            if (version7)
            {
                qtBase = "..\\..\\include\\qt";
                tpIncludes += (qtBase + ";" +
                             qtBase + "\\QtCore;" );
            }
            tpLibs += " QtCore4.lib";
            WritePlotProject_V(out, version7);
            f->close();
        }

        // Write the top level project
        if((f = openCB(pluginTopProject)) != 0)
        {
            QTextStream out(f);
            WriteProject_TOP_LEVEL(out, version7);
            f->close();
        }

        // Write the plugin solution.
        if (!withinDevDir)
        {
            if((f = openCB(workspace)) != 0)
            {
                QTextStream out(f);
                vector<QString> projects;
                projects.push_back(name);
                projects.push_back(name + "I");
                projects.push_back(name + "E");
                projects.push_back(name + "G");
                projects.push_back(name + "S");
                projects.push_back(name + "V");
                WriteProjectSolution(out, projects, version7);
            }
        }
    }

    /***************************************************************************
     ************************* END PLOT PROJECT CODING *************************
     **************************************************************************/

    /***************************************************************************
     ********************** BEGIN OPERATOR PROJECT CODING **********************
     **************************************************************************/

    void WriteOperatorProject_I(QTextStream &out, bool version7)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");

        WriteProjectHelper(out, "operators", 'I', "",
            "plugin.lib", srcFiles, version7);
    }

    void WriteOperatorProject_E(QTextStream &out, bool version7)
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

        if(customwefiles)
            AddElements(srcFiles, wefiles);

        WriteProjectHelper(out, "operators", 'E', "ENGINE_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib plotter.lib avtfilters.lib "
            "pipeline_ser.lib avtexceptions.lib viewer.lib dbatts.lib "
            "visit_vtk.lib vtkCommon.lib vtkGraphics.lib vtkFiltering.lib",
            srcFiles, version7);
    }

    void WriteOperatorProject_G(QTextStream &out, bool version7)
    {
        vector<QString> srcFiles, hdrFiles, mocFiles;
        srcFiles.push_back(name + "PluginInfo.C");
        srcFiles.push_back(name + "CommonPluginInfo.C");
        srcFiles.push_back(name + "GUIPluginInfo.C");
        srcFiles.push_back(atts->name + ".C");
        srcFiles.push_back("Qvis" + name + "Window.C");
        hdrFiles.push_back("Qvis" + name + "Window.h");
        mocFiles.push_back("Qvis" + name + "Window_moc.C");

        if(customwfiles)
        {
            for (size_t i=0; i<wfiles.size(); i++)
            {
                if(wfiles[i].right(2) == ".h")
                {
                    hdrFiles.push_back(wfiles[i]);
                    mocFiles.push_back(wfiles[i].left(
                        wfiles[i].length() - 2) + "_moc.C");
                    srcFiles.push_back(wfiles[i].left(
                        wfiles[i].length() - 2) + ".C");
                }
            }
        }
        else
        {
            for (size_t i=0; i<defaultwfiles.size(); i++)
            {
                if(defaultwfiles[i].right(2) == ".h")
                {
                    hdrFiles.push_back(defaultwfiles[i]);
                    mocFiles.push_back(defaultwfiles[i].left(
                        defaultwfiles[i].length() - 2) + "_moc.C");
                    srcFiles.push_back(defaultwfiles[i].left(
                        defaultwfiles[i].length() - 2) + ".C");
                }
            }
        }

        if (customgfiles)
            AddElements(srcFiles, gfiles);
        else
            AddElements(srcFiles, defaultgfiles);

        WriteProjectHelper(out, "operators", 'G', "GUI_PLUGIN_EXPORTS",
            "state.lib misc.lib plugin.lib gui.lib viewerproxy.lib "
            "viewerrpc.lib", srcFiles, version7, hdrFiles, mocFiles);
    }

    void WriteOperatorProject_S(QTextStream &out, bool version7)
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

    void WriteOperatorProject_V(QTextStream &out, bool version7)
    {
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");
        srcFiles.push_back(name + "CommonPluginInfo.C");
        srcFiles.push_back(name + "ViewerPluginInfo.C");
        srcFiles.push_back(atts->name + ".C");

        if(customvwfiles)
        {
            for (size_t i=0; i<vwfiles.size(); i++)
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

    void WriteOperatorProjects(QFile *(*openCB)(const QString &), 
                               bool version7)
    {
        QString projectExtension(".vcproj");
        QString workSpaceExtension(".sln");
        QString projectDir(ProjectDir(version7));

#if defined(_WIN32)
        if (withinDevDir)
        {
            if(version7)
                projectDir += "windowsbuild\\projects-MSVC7.Net\\operators\\";
            else 
                projectDir += "windowsbuild\\projects-MSVC8.Net\\operators\\";
        }
#endif
        QString IProject(projectDir + (name + "I") + projectExtension);
        QString EProject(projectDir + (name + "E") + projectExtension);
        QString GProject(projectDir + (name + "G") + projectExtension);
        QString SProject(projectDir + (name + "S") + projectExtension);
        QString VProject(projectDir + (name + "V") + projectExtension);
        QString pluginTopProject(projectDir + name + projectExtension);
        QString workspace(projectDir + name + workSpaceExtension);

        cOut << "Operator projects saved to " << projectDir << Endl;

        // Write the I project
        QFile *f = 0;
        if((f = openCB(IProject)) != 0)
        {
            QTextStream out(f);
            visitIncludes = pluginBase + "\\avt\\Math;" +
                            pluginBase + "\\common\\misc;" +
                            pluginBase + "\\common\\plugin;" +
                            pluginBase + "\\common\\state;" +
                            pluginBase + "\\common\\utility" ;
            tpIncludes ="";
            tpLibs ="";
            tpLibDir ="";
            WriteOperatorProject_I(out, version7);
            f->close();
        }

        // Write the E project
        if((f = openCB(EProject)) != 0)
        {
            QTextStream out(f);
            visitIncludes = pluginBase + "\\avt\\Database\\Database;" +
                            pluginBase + "\\avt\\DBAtts\\MetaData;" +
                            pluginBase + "\\avt\\DBAtts\\SIL;" +
                            pluginBase + "\\avt\\Math;" +
                            pluginBase + "\\avt\\Expressions\\Abstract;" +
                            pluginBase + "\\avt\\Expressions\\Management;" +
                            pluginBase + "\\avt\\Expressions\\MeshQuality;" +
                            pluginBase + "\\avt\\Filters;" +
                            pluginBase + "\\avt\\Pipeline\\AbstractFilters;" +
                            pluginBase + "\\avt\\Pipeline\\Data;" +
                            pluginBase + "\\avt\\Pipeline\\Pipeline;" +
                            pluginBase + "\\avt\\Pipeline\\Sinks;" +
                            pluginBase + "\\avt\\Pipeline\\Sources;" +
                            pluginBase + "\\avt\\View;" +
                            pluginBase + "\\avt\\VisWindow\\VisWindow;" +
                            pluginBase + "\\common\\misc;" +
                            pluginBase + "\\common\\Exceptions\\Database;" +
                            pluginBase + "\\common\\Exceptions\\Pipeline;" +
                            pluginBase + "\\common\\expr;" +
                            pluginBase + "\\common\\parser;" +
                            pluginBase + "\\common\\plugin;" +
                            pluginBase + "\\common\\state;" +
                            pluginBase + "\\common\\utility;" +
                            pluginBase + "\\visit_vtk\\full;" +
                            pluginBase + "\\visit_vtk\\lightweight" ;
            QString tpPreProc;
            tpLibs = "";
            tpIncludes = "";
            ThirdParty(tpLibs, tpIncludes, tpPreProc, version7);
            tpLibDir ="";
            WriteOperatorProject_E(out, version7);
            f->close();
        }

        // Write the G project
        if((f = openCB(GProject)) != 0)
        {
            QTextStream out(f);
            visitIncludes = pluginBase + "\\avt\\DBAtts\\MetaData;" +
                            pluginBase + "\\avt\\DBAtts\\SIL;" +
                            pluginBase + "\\avt\\Math;" +
                            pluginBase + "\\gui;" +
                            pluginBase + "\\common\\comm;" +
                            pluginBase + "\\common\\Exceptions\\Pipeline;" +
                            pluginBase + "\\common\\misc;" +
                            pluginBase + "\\common\\plugin;" +
                            pluginBase + "\\common\\proxybase;" +
                            pluginBase + "\\common\\state;" +
                            pluginBase + "\\common\\utility;" + 
                            pluginBase + "\\mdserver\\proxy;" + 
                            pluginBase + "\\mdserver\\rpc;" + 
                            pluginBase + "\\viewer\\proxy;" +
                            pluginBase + "\\viewer\\rpc;" +
                            pluginBase + "\\winutil" ;
            if (version7)
            {
                qtBase = "..\\..\\include\\qt";
                tpIncludes = qtBase + ";" +
                             qtBase + "\\QtCore;" + 
                             qtBase + "\\QtGui;" ;
            }
            tpLibs = " QtCore4.lib QtGui4.lib";
            WriteOperatorProject_G(out, version7);
            f->close();
        }

        // Write the S project
        if((f = openCB(SProject)) != 0)
        {
            QTextStream out(f);
            visitIncludes = pluginBase + "\\avt\\Math;" +
                            pluginBase + "\\common\\misc;" +
                            pluginBase + "\\common\\plugin;" +
                            pluginBase + "\\common\\state;" +
                            pluginBase + "\\common\\utility" ;
            tpIncludes = includeBase + "\\Python-2.5";
            tpLibs ="";
            tpLibDir ="";
            WriteOperatorProject_S(out, version7);
            f->close();
        }

        // Write the V project
        if((f = openCB(VProject)) != 0)
        {
            QTextStream out(f);
            visitIncludes = pluginBase + "\\avt\\Database\\Database;" +
                            pluginBase + "\\avt\\DBAtts\\MetaData;" + 
                            pluginBase + "\\avt\\DBAtts\\SIL;" + 
                            pluginBase + "\\avt\\Expressions\\Abstract;" + 
                            pluginBase + "\\avt\\Expressions\\Management;" + 
                            pluginBase + "\\avt\\Expressions\\MeshQuality;" + 
                            pluginBase + "\\avt\\Filters;" + 
                            pluginBase + "\\avt\\Math;" + 
                            pluginBase + "\\avt\\Pipeline\\AbstractFilters;" + 
                            pluginBase + "\\avt\\Pipeline\\Data;" + 
                            pluginBase + "\\avt\\Pipeline\\Pipeline;" + 
                            pluginBase + "\\avt\\Pipeline\\Sinks;" + 
                            pluginBase + "\\avt\\Pipeline\\Sources;" + 
                            pluginBase + "\\avt\\Plotter;" + 
                            pluginBase + "\\avt\\View;" + 
                            pluginBase + "\\avt\\VisWindow\\VisWindow;" + 
                            pluginBase + "\\common\\comm;" + 
                            pluginBase + "\\common\\Exceptions\\Database;" + 
                            pluginBase + "\\common\\Exceptions\\Pipeline;" + 
                            pluginBase + "\\common\\expr;" + 
                            pluginBase + "\\common\\misc;" + 
                            pluginBase + "\\common\\parser;" + 
                            pluginBase + "\\common\\plugin;" +
                            pluginBase + "\\common\\state;" + 
                            pluginBase + "\\common\\utility;" +
                            pluginBase + "\\viewer\\main;" +
                            pluginBase + "\\visit_vtk\\full;" +
                            pluginBase + "\\visit_vtk\\lightweight" ;
            QString tpPreProc;
            tpLibs = "";
            tpIncludes = "";
            ThirdParty(tpLibs, tpIncludes, tpPreProc, version7);
            if (tpIncludes.length() > 0)
                tpIncludes += ";";
            if (version7)
            {
                qtBase = "..\\..\\include\\qt";
                tpIncludes += (qtBase + ";" +
                             qtBase + "\\QtCore;") ;
            }
            tpLibs += " QtCore4.lib QtGui4.lib";
            WriteOperatorProject_V(out, version7);
            f->close();
        }

        // Write the top level project
        if((f = openCB(pluginTopProject)) != 0)
        {
            QTextStream out(f);
            WriteProject_TOP_LEVEL(out, version7);
            f->close();
        }

        // Write the plugin solution.
        if (!withinDevDir)
        {
            if((f = openCB(workspace)) != 0)
            {
                QTextStream out(f);
                vector<QString> projects;
                projects.push_back(name);
                projects.push_back(name + "I");
                projects.push_back(name + "E");
                projects.push_back(name + "G");
                projects.push_back(name + "S");
                projects.push_back(name + "V");
                WriteProjectSolution(out, projects, version7);
                f->close();
            }
        }
    }

    /***************************************************************************
     *********************** END OPERATOR PROJECT CODING ***********************
     **************************************************************************/

    /***************************************************************************
     ********************** BEGIN DATABASE PROJECT CODING **********************
     **************************************************************************/

    void WriteDatabaseProject(QTextStream &out, char pluginComponent, 
                              bool version7, const QString &libs)
    {
        vector<QString> srcFiles;
        QString tplibs, tpincs, tppreproc;

        // Create the list of source files to build into the plugin.
        srcFiles.push_back(name + QString("PluginInfo.C"));
        if (pluginComponent == 'E')
        {
            srcFiles.push_back(name + QString("CommonPluginInfo.C"));
            srcFiles.push_back(name + QString("EnginePluginInfo.C"));
        }
        else if (pluginComponent == 'M')
        {
            srcFiles.push_back(name + QString("CommonPluginInfo.C"));
            srcFiles.push_back(name + QString("MDServerPluginInfo.C"));
        }
        if (pluginComponent != 'I')
        {
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
                for (size_t i=0; i<mfiles.size(); i++)
                    srcFiles.push_back(mfiles[i]);
            }
            else
            {
                for (size_t i=0; i<defaultmfiles.size(); i++)
                    srcFiles.push_back(defaultmfiles[i]);
            }
            if (customwmfiles)
            {
                for (size_t i=0; i<wmfiles.size(); i++)
                    srcFiles.push_back(wmfiles[i]);
            }
            ThirdParty(tplibs, tpincs, tppreproc, version7);
        }
        if (pluginComponent == 'I')
        {
            visitIncludes = pluginBase + "\\common\\plugin" ;
        }
        else
        {
            visitIncludes = pluginBase + "\\avt\\Database\\Database;" +
                            pluginBase + "\\avt\\Database\\Formats;" +
                            pluginBase + "\\avt\\Database\\Ghost;" +
                            pluginBase + "\\avt\\DBAtts\\MetaData;" +
                            pluginBase + "\\avt\\DBAtts\\SIL;" +
                            pluginBase + "\\avt\\Math;" +
                            pluginBase + "\\avt\\MIR\\Base;" +
                            pluginBase + "\\avt\\Pipeline\\Data;" +
                            pluginBase + "\\avt\\Pipeline\\Pipeline;" +
                            pluginBase + "\\avt\\Pipeline\\Sinks;" +
                            pluginBase + "\\avt\\VisWindow\\VisWindow;" +
                            pluginBase + "\\common\\Exceptions\\Database;" +
                            pluginBase + "\\common\\Exceptions\\Pipeline;" +
                            pluginBase + "\\common\\misc;" +
                            pluginBase + "\\common\\plugin;" +
                            pluginBase + "\\common\\state;" +
                            pluginBase + "\\common\\utility";
        }

        // Write the project file
        if(version7)
        {
            WriteDatabaseProject_Version7(out, pluginComponent, srcFiles, libs,
                    tplibs, tpincs, tppreproc);
        }
        else
            WriteDatabaseProject_Version8(out, pluginComponent, srcFiles,
                    libs, tplibs, tpincs, tppreproc);
    }

    void WriteDatabaseProjects(QFile *(*openCB)(const QString &), 
                               bool version7)
    {
        QString projectExtension(".vcproj");
        QString workSpaceExtension(".sln");
        QString projectDir(ProjectDir(version7));
        QString baseLibs("state.lib misc.lib plugin.lib pipeline_ser.lib "
                         "dbatts.lib database_ser.lib avtexceptions.lib");

#if defined(_WIN32)
        if (withinDevDir)
        {
            if(version7)
                projectDir += "windowsbuild\\projects-MSVC7.Net\\databases\\";
            else 
                projectDir += "windowsbuild\\projects-MSVC8.Net\\databases\\";
        }
#endif
        QString IProject(projectDir + (name + "I") + projectExtension);
        QString EProject(projectDir + (name + "E") + projectExtension);
        QString MProject(projectDir + (name + "M") + projectExtension);
        QString pluginTopProject(projectDir + name + projectExtension);
        QString workspace(projectDir + name + workSpaceExtension);

        cOut << "Database projects saved to " << projectDir << Endl;

        // Write the I project
        QFile *f = 0;
        if((f = openCB(IProject)) != 0)
        {
            QTextStream out(f);
            WriteDatabaseProject(out, 'I', version7, baseLibs);
            f->close();
        }

        baseLibs += " utility.lib";
        // Write the E project
        if((f = openCB(EProject)) != 0)
        {
            QTextStream out(f);
            WriteDatabaseProject(out, 'E', version7, baseLibs);
            f->close();
        }

        // Write the M project
        if((f = openCB(MProject)) != 0)
        {
            QTextStream out(f);
            WriteDatabaseProject(out, 'M', version7, baseLibs);
            f->close();
        }

        // Write the top level project
        if((f = openCB(pluginTopProject)) != 0)
        {
            QTextStream out(f);
            WriteProject_TOP_LEVEL(out, version7);
            f->close();
        }

        if (publicVisIt)
        {
            // Write the plugin solution.
            if((f = openCB(workspace)) != 0)
            {
                QTextStream out(f);
                vector<QString> projects;
                projects.push_back(name);
                projects.push_back(name + "I");
                projects.push_back(name + "E");
                projects.push_back(name + "M");
                WriteProjectSolution(out, projects, version7);
            }
        }
    }

    /***************************************************************************
     *********************** END DATABASE PROJECT CODING ***********************
     **************************************************************************/
public:

    void WriteProjectFiles(QFile* (*openCB)(const QString &), 
                           bool version7)
    {
        preproc = "WIN32;_WINDOWS;_USRDLL;_USE_MATH_DEFINES";
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
