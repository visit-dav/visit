// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef GENERATE_CMAKE_H
#define GENERATE_CMAKE_H

#include <QTextStream>
#include "Field.h"
#include <visit-config.h> // for the plugin extension.
#include "Plugin.h"

// ****************************************************************************
//  File:  GenerateCMake
//
//  Purpose:
//    Contains a set of classes which override the default implementation
//    to create cmake input for the plugin.
//
//  Note: This file overrides --
//    Plugin
//
//  Programmer:  Brad Whitlock,
//  Creation:    Thu Jan 29 13:44:46 PST 2009
//
//  Modifications:
//    Brad Whitlock, Fri Nov  6 11:15:11 PST 2009
//    Handle serial and parallel engine libs.
//
//    Brad Whitlock, Mon Nov 23 15:19:10 PST 2009
//    I added server components and engine only builds.
//
//    David Camp, Thu Jan 14 17:56:29 PST 2010
//    Added the ADD_TARGET_DEFINITIONS function to define ENGINE for plots.
//
//    Kathleen Bonnell, Tue Jan 26 20:32:55 MST 2010
//    Remove setting of LIBRARY_OUTPUT_PATH, (set by parent instead). Add
//    call to VISIT_PLUGIN_TARGET_PREFIX macro.
//
//    Brad Whitlock, Wed Feb 10 16:36:00 PST 2010
//    I made all of the database plugins use the ADD_TARGET_DEFINITIONS
//    function.
//
//    Eric Brugger, Wed Feb 24 13:00:54 PST 2010
//    I modified the database plugins to list the include paths specified
//    in the xml file before the VTK include paths.
//
//    Eric Brugger, Fri Feb 26 09:47:00 PST 2010
//    I modified the database plugins to list the include paths specified
//    in the xml file before any of the VisIt include paths.  I also modified
//    the database plugins to also treat all flags in CXXFLAGS that start
//    with "-I" as include paths.
//
//    Kathleen Bonnell, Fri May 21 14:15:23 MST 2010
//    Add DLL_NETCDF, _CGNSDLL EXODUSII_BUILD_SHARED_LIBS defines for
//    windows projects linking with NETCDF, CGNS or EXODUSII.
//
//    Kathleen Bonnell, Thu May 27 14:59:13 MST 2010
//    Add some more defines for HDF4, discovered as necessary when compiling
//    with Visual Studio 9.
//
//    Kathleen Bonnell, Fri Sep 24 11:25:32 MST 2010
//    Add ENGINE target definition for operators if they contain
//    engine-specific code.
//
//    Kathleen Bonnell, Tue Nov 16 16:26:47 PST 2010
//    Remove logic for mesa.  Add newline after each extraInclude for
//    legibility in the CMakeLists.txt files.
//
//    David Camp, Wed Nov 17 14:54:02 PST 2010
//    Added the LIBS libraries to the Plot and Operators, did the samething
//    the database code was doing. Also added the link dirs from the ldflags.
//
//    Kathleen Bonnell, Fri Sep 24 11:25:32 MST 2010
//    Fix windows issues with viewer and gui libs building against an
//    installed version of VisIt.  Convert Windows paths to CMake paths
//    since we are creating a CMake file.
//
//    Kathleen Bonnell, Tue Jan  4 08:38:03 PST 2011
//    Fix CGNS dll define, due to update of cgns library.
//    Add call to VISIT_PLUGIN_TARGET_FOLDER for project grouping in VS.
//
//    Eric Brugger, Fri Jan  7 13:38:59 PST 2011
//    I replaced the BOXLIB2D and BOXLIB3D variables with just BOXLIB.
//
//    Kathleen Bonnell, Tue Jan 11 17:06:21 MST 2011
//    Removed setting EXODUSII_BUILD_SHARED_LIBS definition.
//
//    Kathleen Bonnell, Thu Jan 13 17:54:38 MST 2011
//    Only use VISIT_PLUGIN_TARGET_FOLDER if building from dev.
//
//    Brad Whitlock, Wed Feb 23 15:24:48 PST 2011
//    Enable Fortran language compilation if the user added Fortran code to the
//    list of files.
//
//    Kathleen Biagas, Fri Nov 18 10:09:26 MST 2011
//    Add plugin name to VISIT_PLUGIN_TARGET_FOLDER args. Eases building/
//    debugging individual plugins with Visual Studio when grouped by name.
//
//    Kathleen Biagas, Tue Nov 22 14:39:51 PST 2011
//    Remove VISIT_PLUGIN_TARGET_PREFIX in favor of VISIT_PLUGIN_TARGET_RUNTIME.
//
//    Kathleen Biagas, Mon Jun 18 10:49:07 MST 2012
//    Set VISIT_ARCHIVE_DIR on windows to be /lib. Change minimum CMake
//    version to 2.8.8.
//
//    Kathleen Biagas, Mon Jul 30 15:40:10 MST 2012
//    No longer add definition _HDF5USEDLL_ for hdf5 based plugins, as this
//    is now predefined in an hdf5 header.
//
//    Kathleen Biagas, Wed Oct  9 10:01:15 PDT 2013
//    Added handling of 'Code' and 'Condition' keywords in codefile.
//    'Condition' allows for conditional includes, definitions and links.
//
//    Kathleen Biagas, Tue Oct 29 16:04:19 MST 2013
//    For extraIncludes specified in CXXFLAGS, check for use of
//    ${VISIT_INCLUDE_DIR} and correct it if building against public VisIt.
//
//    Eric Brugger, Wed May 21 14:48:11 PDT 2014
//    I added support for EAVL.
//
//    Kathleen Biagas, Thu Oct 30 16:37:37 MST 2014
//    Added status message for plugins-against-a-public install on Windows,
//    to inform user the location of the plugin once complete.
//
//    Kathleen Biagas, Thu Nov  6 11:21:13 PST 2014
//    Added support for DEFINES tag.
//
//    Eric Brugger, Thu Dec 10 11:07:56 PST 2015
//    I added support for VTKm.
//
//    Kathleen Biagas, Fri Feb 19 10:29:33 PST 2016
//    Convert -I flags to proper IncludeDir when writing plot/operator
//    instead of stuffing them in an add_defintions call.  Removed commented-
//    out logic, clean up writing of whitespace.
//
//    Kathleen Biagas, Thu Feb  8 08:42:37 PST 2018
//    Remove Qt, Qwt, VTK from link_directories, their libraries contain
//    full paths.
//
//    Eric Brugger, Mon Sep 17 10:14:49 PDT 2018
//    I replaced support for vtkm with vtkh.
//
//    Kathleen Biagas, Thu Sep 27 11:36:45 PDT 2018
//    For non-dev builds: Filter VTK libs (to include version number), add
//    VISIT_ARCHIVE_DIR as linkDir.  Add QtWidgets include dir.
//
//    Kathleen Biagas, Wed Jan 30 10:44:21 PST 2019
//    Removed support for EAVL.
//
//    Kathleen Biagas, Thu Jan  2 09:18:18 PST 2020
//    Added hl arg, for haslicense.
//
//    Kathleen Biagas, Tue Jan 21 10:53:18 PST 2020
//    Removed VTKh and VTKm includes. Will be added via CXXFLAGS in .xml files
//    by plugins requiring the includes. (See Slice operator for example).
//
//    Kathleen Biagas, Fri July 16, 2021
//    Removed hard-coded database preprocessor defines. Now specified by
//    WIN32DEFINES in .xml file and parsed like other DEFINES.
//
//    Kathleen Biagas, Tue April 27, 2022
//    Add SKIP_INFO to ADD_DATABASE_CODE_GEN_TARGETS when skipInfoGen set in
//    the plugin's .xml file.  Allows plugins with custom info code to not have
//    their targets added to code gen targets.
//
//    Kathleen Biagas, Tue May 3, 2022
//    Consolidate Plot and Operator code into one method.
//    Consolidate engine target creation into one method.
//    Add support for component-specific CXXFLAGS, LDFLAGS and DEFINES.
//
//    Kathleen Biagas, Wed Aug 3, 2022
//    Modify FilterVTKLibs to add support for VTK9's new library naming
//    convention. Filter now creates separate vtk8 and vtk9 versions of the
//    libs for each component.  Both are written to the CMakeLists.txt file
//    with a VTK_VERSION check that specifies which should be used.
//    Filter happens all the time now, not just when not using dev.
//
//    Kathleen Biagas, Tue Nov 29, 2022
//    Remove inclusion of PluginMacros.cmake, now included in each 
//    plugin category (plot/operators/databases) root CMakeLists.txt.
//
//    Kathleen Biagas, Thu Mar 30, 2023
//    Use AUTOMOC target property instead of QT_WRAP_CPP macro.
//
//    Kathleen Biagas, Thu Sep 28 13:33:32 PDT 2023
//    Add AUTOMOC_EXECUTABLE target property when building against an install.
//
//    Kathleen Biagas, Wed Nov  8 10:16:09 PST 2023
//    VTK9 targets need to be handled differently for non-dev.
//
//    Kathleen Biagas, Thu May 2, 2024
//    Move CXX_STANDARD settings to apply only to GUI.
//    Add -ZC:__cplusplus for MSVC.
//    Add 'VISIT_PLUGIN_TARGET_OUTPUT_DIR' only for Dev builds.
//
//    Kathleen Biagas, Wed Sep 18, 2024
//    Add 'FilterConditionalLibs' so that VTKM version can be appended to
//    vtkm_ libraries when run outside dev environment (eg pluginVsInstall).
//
//    Kathleen Biagas, Mon May 5, 2025
//    The plugin code has been modified to utilize new cmake functions:
//    visit_add_plot_plugin, visit_add_operator_plugin,
//    visit_add_database_plugin.  Thus most of the CMake logic resides
//    there instead of here, where it can be hard to parse to make changes.
//
//    Removed no-longer used code.
//
// ****************************************************************************

class CMakeGeneratorPlugin : public Plugin
{
  public:
    CMakeGeneratorPlugin(const QString &n,const QString &l,const QString &t,
        const QString &vt,const QString &dt, const QString &v, const QString &ifile,
        bool hw, bool ho, bool hl, bool onlyengine, bool noengine) :
        Plugin(n,l,t,vt,dt,v,ifile,hw,ho,hl,onlyengine,noengine)
    {
    }

    virtual ~CMakeGeneratorPlugin()
    {
    }

    QString
    ConvertDollarParenthesis(const QString &s) const
    {
        QString retval(s);
        retval = retval.replace("$(", "${");
        retval = retval.replace(")", "}");
        return retval;
    }

    QString
    ToString(const std::vector<QString> &vec, bool withNewline=false) const
    {
        QString s;
        if (withNewline)
        {
            for(size_t i = 0; i < vec.size(); ++i)
                s += (ConvertDollarParenthesis(vec[i]) + "\n");
        }
        else
        {
            for(size_t i = 0; i < vec.size(); ++i)
                s += (ConvertDollarParenthesis(vec[i]) + " ");
        }
        return s;
    }

    void
    FilterVTKLibs(std::vector<QString> &libs,
                  std::vector<QString> &libs9)
    {
        QString vtkversion = QString("-%1.%2").arg(VTK_MAJ).arg(VTK_MIN);
        std::vector<QString> libs_sans_vtk;
        for(size_t i = 0; i < libs.size(); ++i)
        {
            if(libs[i].startsWith("vtk"))
            {
                QString tmp(libs[i]);

                // convert to VTK:: form for VTK-9
                QString tmp2(libs[i]);
                if (tmp2 == "vtksys")
                    tmp2.prepend("VTK::");
                else
                    tmp2.replace(0,3,"VTK::");
                libs[i] = tmp2;
            }
        }
    }

    QString
    VisItIncludeDir() const
    {
        return "${VISIT_INCLUDE_DIR}";
    }

    QString
    ConvertToProperVisItIncludeDir(const QString &s) const
    {
        QString VID = VisItIncludeDir();
        QString retval(s);
        if (!s.startsWith(VID))
        {
            if (!using_dev && s.startsWith("${VISIT_INCLUDE_DIR}"))
                retval = VID + s.right(s.length()-20);
        }
        return retval;
    }

#ifdef _WIN32
    QString
    ToCMakePath(const QString &s) const
    {
        char exppath[MAX_PATH];
        ExpandEnvironmentStrings(s.toStdString().c_str(), exppath, MAX_PATH);
        QString retval(exppath);
        retval = retval.replace("\\", "/");
        return retval;
    }
#endif

    QString
    IncludesToString(const std::vector<QString> &vec, bool withNewline=false, bool atBeg=false) const
    {
        QString s;
        QString sep;
        if(withNewline)
            sep="\n";
        else
            sep=" ";

        for(size_t i = 0; i < vec.size(); ++i)
        {
            if(atBeg)
                s += sep;
            if(vec[i].startsWith("${"))
                s += (ConvertToProperVisItIncludeDir(vec[i]));
            else if(vec[i].startsWith("$("))
                s += (ConvertToProperVisItIncludeDir(ConvertDollarParenthesis(vec[i])));
            else if(vec[i].startsWith("-I"))
                s += (ConvertToProperVisItIncludeDir(vec[i].right(vec[i].size()-2)));
            else
                s += (ConvertToProperVisItIncludeDir(vec[i]));
            if(!atBeg)
                s += sep;
        }
        return s;
    }

    void
    CMakeWrite_TargetIncludes(QTextStream &out,
                              const char *indent,
                              const char *comp,
                              const char *suffix,
                              const std::vector<QString> &inc)
    {
        QString ptype = type;
        ptype[0] = type[0].toUpper();
        out << indent << "TARGET_INCLUDE_DIRECTORIES(" << comp << name;
        out << ptype << suffix << " PRIVATE";
        out << IncludesToString(inc, false, true);
        out << ")" << Endl;
    }

    void
    CMakeWrite_TargetLinkDirs(QTextStream &out,
                              const char *indent,
                              const char *comp,
                              const char *suffix,
                              const std::vector<QString> &ld)
    {
        QString ptype = type;
        ptype[0] = type[0].toUpper();
        out << indent << "TARGET_LINK_DIRECTORIES(" << comp << name;
        out << ptype << suffix << " PRIVATE " << ToString(ld)<< ")" << Endl;
    }

    void
    CMakeWrite_TargetDefines(QTextStream &out,
                              const char *indent,
                              const char *comp,
                              const char *suffix,
                              const std::vector<QString> &def)
    {
        QString ptype = type;
        ptype[0] = type[0].toUpper();
        out << indent << "TARGET_COMPILE_DEFINITIONS(" << comp << name;
        out << ptype << suffix << " PRIVATE " << ToString(def)<< ")" << Endl;
    }

    bool
    GetCondition(const QString &c, QStringList &cond, QStringList &val) const
    {
        bool retval = false;
        if (atts != NULL && atts->codeFile != NULL)
        {
            retval = atts->codeFile->GetCondition("xml2cmake", c, cond, val);
        }
        return retval;
    }

    void WriteCMake_ConditionalIncludes(QTextStream &out)
    {
        QStringList conditions, incs;
        if(GetCondition("Includes:", conditions, incs))
        {
           for (int i = 0; i < conditions.size(); ++i)
           {
                out << "if(" << conditions[i] << ")" << Endl;
                out << "    include_directories(";
                out << incs[i];
                out << ")" << Endl;
                out << "endif()" << Endl;
                out << Endl;
            }
        }
    }

    void WriteCMake_ConditionalDefinitions(QTextStream &out)
    {
        QStringList conditions, defs;
        if(GetCondition("Definitions:", conditions, defs))
        {
            for (int i = 0; i < conditions.size(); ++i)
            {
                out << "if(" << conditions[i] << ")" << Endl;
                out << "    add_definitions(";
                out << defs[i];
                out << ")" << Endl;
                out << "endif()" << Endl;
                out << Endl;
            }
        }
    }

    void
    WriteCMake_AdditionalCode(QTextStream &out, bool prefix)
    {
        if (atts != NULL && atts->codeFile != NULL)
        {
            QStringList targets, names, first, second;
            atts->codeFile->GetAllCodes(targets, names, first, second);
            for (int i = 0; i < targets.size(); ++i)
            {
                if (targets[i] == "xml2cmake")
                {
                    if (prefix && !first[i].isEmpty())
                    {
                        out << first[i] << Endl;
                    }
                    else if (!prefix && !second[i].isEmpty())
                    {
                        out << second[i] << Endl;
                    }
                }
            }
        }
    }

    void WriteCMake_PluginSources(QTextStream &out,
        const QString component,
        const std::vector<QString> files)
    {
        out << Endl;
        out << "    " << component << "SRC       " << files[0];
        for(size_t i = 1; i < files.size(); ++i)
            out << Endl << "               " << files[i];
    }

    void WriteCMake_PluginLibs(QTextStream &out,
        const QString component,
        const std::vector<QString> clibs,
        const bool hasConditionalLibs)
    {
        if(clibs.empty() && !hasConditionalLibs)
            return;

        out << Endl;
        QString IOne("   ");
        QChar c(component[0]);
        if (c == 'V' || c =='G'|| c == 'M')
            IOne += QString("   ");
        out << "    " << component << "LIBS" << IOne;
        if(!clibs.empty())
        {
            out << clibs[0];
            for(size_t i = 1; i < clibs.size(); ++i)
                out << Endl << "               " << clibs[i];
        }
        if(hasConditionalLibs)
        {
            if(!clibs.empty())
                out << Endl << "               ";
            out << "${" << name << "_" << c << "Libs}";
        }
    }

    bool WriteCMake_PluginConditionalLibs(QTextStream &out, const char *libType)
    {
        QString c(libType);
        c += "LinkLibraries:";
        QStringList conditions, links;
        if (GetCondition(c, conditions, links))
        {
            out << Endl;
            for (int i = 0; i < conditions.size(); ++i)
            {
                out << "if(" << conditions[i] << ")" << Endl;
                out << "    set(" << name << "_" << libType << "Libs " << links[i] << ")" << Endl;
                out << "endif()" << Endl;
            }
            return true;
        }
        return false;
    }

    bool WriteCMake_PluginConditionalIncludes(QTextStream &out)
    {
        QStringList conditions, incs;
        bool hasIncludes = GetCondition("Includes:", conditions, incs);

        // check for Includes from xml file
        std::vector<QString> xmlIncludes;
        for (size_t i=0; i<cxxflags.size(); i++)
        {
            if(cxxflags[i].startsWith("${"))
                 xmlIncludes.push_back(ConvertToProperVisItIncludeDir(cxxflags[i]));
            else if(cxxflags[i].startsWith("$("))
                 xmlIncludes.push_back(ConvertToProperVisItIncludeDir(ConvertDollarParenthesis(cxxflags[i])));
            else if(cxxflags[i].startsWith("-I"))
                 xmlIncludes.push_back(ConvertToProperVisItIncludeDir(cxxflags[i].right(cxxflags[i].size()-2)));
        }
        hasIncludes |= !xmlIncludes.empty();

        if(hasIncludes)
        {
            out << Endl;
            out << "set(" << name << "_INCLUDES";
            for(int i = 0; i < xmlIncludes.size(); ++i)
                out << "\n    " << xmlIncludes[i];
            out << ")" << Endl;

            for (int i = 0; i < conditions.size(); ++i)
            {
                out << "if(" << conditions[i] << ")" << Endl;
                out << "    list(APPEND " << name << "_INCLUDES " << incs[i] << ")" << Endl;
                out << "endif()" << Endl;
            }
        }
        return hasIncludes;
    }

    bool WriteCMake_PluginConditionalDefinitions(QTextStream &out)
    {
        QStringList conditions, cdefs;
        bool hasDefines = !defs.empty();
        if(!defs.empty())
        {
            out << Endl;
            out << "set(" << name << "_DEFINES " << ToString(defs) << ")" << Endl;
        }
        if(GetCondition("Definitions:", conditions, cdefs))
        {
            if(defs.empty())
            {
                out << Endl;
                out << "set(" << name << "_DEFINES)" << Endl;
            }
            for (int i = 0; i < conditions.size(); ++i)
            {
                out << "if(" << conditions[i] << ")" << Endl;
                out << "    list(APPEND " << name << "_DEFINES " << cdefs[i];
                out << ")" << Endl;
                out << "endif()" << Endl;
            }
            hasDefines = true;
        }
        return hasDefines;
    }

    void WriteCMake_PlotOperator(QTextStream &out,
                         const QString &guilibname,
                         const QString &viewerlibname)
    {
        WriteCMake_AdditionalCode(out, true);
        bool hasDefines = WriteCMake_PluginConditionalDefinitions(out);
        bool hasIncludes = WriteCMake_PluginConditionalIncludes(out);
        bool hasGLibs  = WriteCMake_PluginConditionalLibs(out, "G");
        bool hasVLibs  = WriteCMake_PluginConditionalLibs(out, "V");
        bool hasELibs  = WriteCMake_PluginConditionalLibs(out, "E");
        out << Endl;
        if (type == "plot")
        {
            out << "visit_add_plot_plugin(" << Endl;
            out << "    PNAME      " << name;
        }
        else 
        {
            out << "visit_add_operator_plugin(" << Endl;
            out << "    ONAME      " << name;
        }
        if (customgfiles)
        {
            WriteCMake_PluginSources(out, "G", gfiles);
        }
        if (customvfiles)
        {
            WriteCMake_PluginSources(out, "V", vfiles);
        }
        if (customefiles)
        {
            WriteCMake_PluginSources(out, "E", efiles);
        }

        if(hasDefines)
        {
            out << "\n    DEFINES    ${" << name << "_DEFINES}";
        }
        if(hasIncludes)
        {
            out << "\n    INCLUDES   ${" << name << "_INCLUDES}";
        }

        // gui libs
        WriteCMake_PluginLibs(out, "G", glibs, hasGLibs);

        // viewer libs
        WriteCMake_PluginLibs(out, "V", vlibs, hasVLibs);

        // engine libs
        WriteCMake_PluginLibs(out, "ESER", elibsSer, hasELibs);
        WriteCMake_PluginLibs(out, "EPAR", elibsPar, hasELibs);
        if(!using_dev)
        {
            out << "\n    PUBLIC_BUILD";
        }
        out << ")" << Endl;

        WriteCMake_AdditionalCode(out, false);
    }

    void WriteCMake_DatabasePlugin(QTextStream &out)
    {
        bool useFortran = false;

        WriteCMake_AdditionalCode(out, true);
        bool hasDefines = WriteCMake_PluginConditionalDefinitions(out);
        bool hasIncludes = WriteCMake_PluginConditionalIncludes(out);
        bool hasMLibs  = WriteCMake_PluginConditionalLibs(out, "M");
        bool hasELibs  = WriteCMake_PluginConditionalLibs(out, "E");
        out << Endl;

        out << "visit_add_database_plugin(" << Endl;
        out << "    DNAME      " << name;

        if (custommfiles)
        {
            WriteCMake_PluginSources(out, "M", mfiles);
        }
        if (customefiles)
        {
            WriteCMake_PluginSources(out, "E", efiles);
        }

        if(hasDefines)
        {
            out << "\n    DEFINES    ${" << name << "_DEFINES}";
        }
        if(hasIncludes)
        {
            out << "\n    INCLUDES   ${" << name << "_INCLUDES}";
        }
        if(!libs.empty())
        {
            out << "\n    LIBS       " << libs[0] ;
            for(size_t i = 1; i < libs.size(); ++i)
                out << Endl << "               " << libs[i];
        }

        // mdserver libs
        WriteCMake_PluginLibs(out, "M", mlibs, hasMLibs);

        // engine libs
        WriteCMake_PluginLibs(out, "ESER", elibsSer, hasELibs);
        WriteCMake_PluginLibs(out, "EPAR", elibsPar, hasELibs);

        out << ")" << Endl;

        WriteCMake_AdditionalCode(out, false);
        out << Endl;
    }

    void WriteCMake(QTextStream &out)
    {
        const char *visithome = getenv("VISITARCHHOME");
        if (!visithome && !using_dev)
            throw QString().asprintf("Please set the VISITARCHHOME "
                                    "environment variable.\n"
                                    "You may have it set automatically "
                                    "using 'visit -xml2cmake'.");

        const char *visitplugdirpub = getenv("VISITPLUGININSTPUB");
        if (!visitplugdirpub && installpublic)
            throw QString().asprintf("Please set the VISITPLUGININSTPUB "
                                    "environment variable.\n"
                                    "You may have it set automatically "
                                    "using 'visit -xml2cmake'.");

        const char *visitplugdirpri = getenv("VISITPLUGININSTPRI");
        if (!visitplugdirpri)
        {
           if ((using_dev && installprivate) || !using_dev)
            throw QString().asprintf("Please set the VISITPLUGININSTPRI "
                                    "environment variable.\n"
                                    "You may have it set automatically "
                                    "using 'visit -xml2cmake'.");
        }

        out << "# DO NOT EDIT THIS FILE! THIS FILE IS AUTOMATICALLY GENERATED "
            << "BY xml2cmake" << Endl;

        QString qvisithome(visithome);
        QString qvisitplugdirpub(visitplugdirpub);
        QString qvisitplugdirpri(visitplugdirpri);
#ifdef _WIN32
        qvisithome       = ToCMakePath(qvisithome);
        qvisitplugdirpub = ToCMakePath(qvisitplugdirpub);
        qvisitplugdirpri = ToCMakePath(qvisitplugdirpri);
#endif
        FilterVTKLibs(libs,     vtk9_libs);
        FilterVTKLibs(mlibs,    vtk9_mlibs);
        FilterVTKLibs(glibs,    vtk9_glibs);
        FilterVTKLibs(vlibs,    vtk9_vlibs);
        FilterVTKLibs(elibsSer, vtk9_elibsSer);
        FilterVTKLibs(elibsPar, vtk9_elibsPar);

        // If we're not using a development version then we need to always
        // include something in the generated output.
        if(!using_dev)
        {
            out << "cmake_minimum_required(VERSION 3.24 FATAL_ERROR)\n" << Endl;
            out << "project(" << name << "_" << type << "_plugin)\n" << Endl;
            if(installpublic)
            {
                out << "set(VISIT_PLUGIN_DIR \"" << qvisitplugdirpub
                    << "\")" << Endl;
            }
            else // installprivate or default
            {
                out << "set(VISIT_PLUGIN_DIR \"" << qvisitplugdirpri
                    << "\")" << Endl;
            }

            out << "include(\"" << qvisithome
                << "/cmake/visitConfig.cmake\")" << Endl;
            out << Endl;
        }
        else
        {
            // We're using a development version but we're installing public
            // or private.
            if(installpublic)
            {
               out << "set(VISIT_PLUGIN_DIR " << qvisitplugdirpub << ")" << Endl;
            }

            if(installprivate)
            {
               out << "set(VISIT_PLUGIN_DIR " << qvisitplugdirpri << ")" << Endl;
            }
        }

        QString guilibname("gui");
        QString viewerlibname("viewer");
#ifdef _WIN32
        if (! using_dev)
        {
            // when calling from an installed version, cmake doesn't know that
            // the gui and viewer lib targets have been renamed to guilib and
            // viewer lib (to prevent conflicts with the exe targets), so they
            // must be explictily listed by the name of the actual lib created.
            guilibname    = "guilib";
            viewerlibname = "viewerlib";
        }
#endif
        if(type == "database")
            WriteCMake_DatabasePlugin(out);
        else
            WriteCMake_PlotOperator(out, guilibname, viewerlibname);
    }

private:
    // couldn't think of a way to support both VTK8 and VTK9 via codegen
    // without also requiring re-generation when switching between 8 and 9.
    // so creating extra storage for them here.
    std::vector<QString> vtk9_libs;
    std::vector<QString> vtk9_glibs;
    std::vector<QString> vtk9_mlibs;
    std::vector<QString> vtk9_vlibs;
    std::vector<QString> vtk9_elibsSer;
    std::vector<QString> vtk9_elibsPar;
};


// ----------------------------------------------------------------------------
//                           Override default types
// ----------------------------------------------------------------------------
#define Plugin       CMakeGeneratorPlugin

#endif
