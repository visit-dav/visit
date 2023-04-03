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
// ****************************************************************************

class CMakeGeneratorPlugin : public Plugin
{
  public:
    CMakeGeneratorPlugin(const QString &n,const QString &l,const QString &t,
        const QString &vt,const QString &dt, const QString &v, const QString &ifile,
        bool hw, bool ho, bool hl, bool onlyengine, bool noengine) :
        Plugin(n,l,t,vt,dt,v,ifile,hw,ho,hl,onlyengine,noengine)
    {
        defaultgfiles.clear();
        defaultsfiles.clear();
        defaultvfiles.clear();
        defaultmfiles.clear();
        defaultefiles.clear();
        defaultwfiles.clear();
        if (type == "database")
        {
            QString filter = QString("avt") + name + "FileFormat.C";
            defaultmfiles.push_back(filter);
            defaultefiles.push_back(filter);
            if (haswriter)
                defaultefiles.push_back(QString("avt") + name + "Writer.C");
            if (hasoptions)
            {
                QString options = QString("avt") + name + QString("Options.C");
                defaultmfiles.push_back(options);
                defaultefiles.push_back(options);
            }
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
            defaultefiles.push_back(filter);
        }
    }

    virtual ~CMakeGeneratorPlugin()
    {
    }

    void
    GetFilesWith(const QString &name, const std::vector<QString> &input,
                 std::set<QString> &output)
    {
         for(size_t i = 0; i < input.size(); ++i)
         {
             if(input[i].indexOf(name) != -1)
                 output.insert(input[i]);
         }
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
                  std::vector<QString> &libs8,
                  std::vector<QString> &libs9)
    {
        QString vtkversion = QString("-%1.%2").arg(VTK_MAJ).arg(VTK_MIN);
        std::vector<QString> libs_sans_vtk;
        for(size_t i = 0; i < libs.size(); ++i)
        {
            if(libs[i].startsWith("vtk"))
            {
                QString tmp(libs[i]);
                if (!using_dev)
                    // append the vtk version for VTK-8
                    tmp.append(vtkversion);
                libs8.push_back(tmp);

                // convert to VTK:: form for VTK-9
                QString tmp2(libs[i]);
                if (tmp2 == "vtksys")
                    tmp2.prepend("VTK::");
                else
                    tmp2.replace(0,3,"VTK::");
                libs9.push_back(tmp2);
            }
            else
            {
                libs_sans_vtk.push_back(libs[i]);
            }
        }
        libs = libs_sans_vtk;
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
        out << ")" << endl;
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
        out << ptype << suffix << " PRIVATE " << ToString(ld)<< ")" << endl;
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
        out << ptype << suffix << " PRIVATE " << ToString(def)<< ")" << endl;
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
                out << "if(" << conditions[i] << ")" << endl;
                out << "    include_directories(";
                out << incs[i];
                out << ")" << endl;
                out << "endif()" << endl;
                out << endl;
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
                out << "if(" << conditions[i] << ")" << endl;
                out << "    add_definitions(";
                out << defs[i];
                out << ")" << endl;
                out << "endif()" << endl;
                out << endl;
            }
        }
    }

    void WriteCMake_ConditionalTargetLinks(QTextStream &out, const QString &target, const char *libType, const QString &plugType, const char *indent)
    {
        QString c(libType);
        c += "LinkLibraries:";
        QStringList conditions, links;
        if (GetCondition(c, conditions, links))
        {
            for (int i = 0; i < conditions.size(); ++i)
            {
                out << indent << "if(" << conditions[i] << ")" << endl;
                out << indent << "    target_link_libraries(" << libType << target << plugType << " " << links[i] << ")" << endl;
                out << indent << "endif()" << endl;
                out << endl;
            }
        }
    }

    void WriteCMake_ConditionalSources(QTextStream &out, const char *libType, const char *indent)
    {
        QString c(libType);
        c += "Sources:";
        QStringList conditions, srcs;
        if (GetCondition(c, conditions, srcs))
        {
            for (int i = 0; i < conditions.size(); ++i)
            {
                out << indent << "if(" << conditions[i] << ")" << endl;
                out << indent << "    set(LIB" << libType << "_SOURCES ${LIB" << libType << "_SOURCES} " << srcs[i] << ")" << endl;
                out << indent << "endif()" << endl;
                out << endl;
            }
        }
    }

    void
    WriteCMake_AdditionalCode(QTextStream &out)
    {
        if (atts != NULL && atts->codeFile != NULL)
        {
            QStringList targets, names, first, second;
            atts->codeFile->GetAllCodes(targets, names, first, second);
            for (int i = 0; i < targets.size(); ++i)
            {
                if (targets[i] == "xml2cmake")
                {
                    if (!first[i].isEmpty())
                    {
                        out << first[i] << endl;
                    }
                    if (!second[i].isEmpty())
                    {
                        out << second[i] << endl;
                    }
                }
            }
        }
    }

    void WriteCMake_PlotOperator_Includes(QTextStream &out)
    {
        // take any ${} from the CXXFLAGS to mean a variable that contains
        // include directories.
        std::vector<QString> extraIncludes;
        for (size_t i=0; i<cxxflags.size(); i++)
        {
            if(cxxflags[i].startsWith("${"))
                 extraIncludes.push_back(ConvertToProperVisItIncludeDir(cxxflags[i]));
            else if(cxxflags[i].startsWith("$("))
                 extraIncludes.push_back(ConvertToProperVisItIncludeDir(ConvertDollarParenthesis(cxxflags[i])));
            else if(cxxflags[i].startsWith("-I"))
                 extraIncludes.push_back(ConvertToProperVisItIncludeDir(cxxflags[i].right(cxxflags[i].size()-2)));
        }

        out << endl;

        // Includes
        out << "INCLUDE_DIRECTORIES(" << endl;
        out << "${CMAKE_CURRENT_SOURCE_DIR}" << endl;
        if(type == "operator")
            out << "${VISIT_OPERATOR_INCLUDES}" << endl;
        else
            out << "${VISIT_PLOT_INCLUDES}" << endl;
        if(!using_dev)
        {
            out << "${QT_INCLUDE_DIR}" << endl;
            out << "${QT_QTCORE_INCLUDE_DIR}" << endl;
            out << "${QT_QTGUI_INCLUDE_DIR}" << endl;
            out << "${QT_QTWIDGETS_INCLUDE_DIR}" << endl;
        }
        if(extraIncludes.size() > 0)
            out << ToString(extraIncludes, true);
        out << ")" << endl;
        out << endl;
    }

    void CMakeAdd_EngineTargets(QTextStream &out)
    {
        QString ptype = type;
        ptype[0] = type[0].toUpper();
        out << "ADD_LIBRARY(E"<<name<<ptype << "_ser ${LIBE_SOURCES}";
        if (customwefiles)
            out << " ${LIBE_WIN32_SOURCES}";
        out << ")" << endl;
        if(!edefsSer.empty())
        {
            CMakeWrite_TargetDefines(out, "", "E", "_ser", edefsSer);
        }
        if(!ecxxflagsSer.empty())
        {
            CMakeWrite_TargetIncludes(out, "", "E", "_ser", ecxxflagsSer);
        }
        if(!ecxxflagsSer.empty())
        {
            CMakeWrite_TargetLinkDirs(out, "", "E", "_ser", eldflagsSer);
        }
        if (!vtk8_elibsSer.empty())
        {
            out << "\nif(VTK_VERSION VERSION_EQUAL \"8.1.0\")" << endl;
            out << "    set(vtk_elibsSer " << ToString(vtk8_elibsSer) << ")" << endl;
            out << "else()" << endl;
            out << "    set(vtk_elibsSer " << ToString(vtk9_elibsSer) << ")" << endl;
            out << "endif()\n" << endl;
        }
        out << "TARGET_LINK_LIBRARIES(E"<<name<<ptype<<"_ser visitcommon avtpipeline_ser";
        if(type == "plot")
            out << " avtplotter_ser ";
        else if (type == "operator")
            out << " avtexpressions_ser avtfilters_ser ";
        else
            out << " avtdatabase_ser ";
        out << ToString(libs) << ToString(elibsSer);
        if (!vtk8_libs.empty())
            out << "${vtk_libs} ";
        if (!vtk8_elibsSer.empty())
            out << "${vtk_elibsSer} ";
        out << ")" << endl;
        WriteCMake_ConditionalTargetLinks(out, name, "E", (ptype+"_ser"), "");
        if (type != "operator" || hasEngineSpecificCode)
            out << "ADD_TARGET_DEFINITIONS(E"<<name<<ptype<<"_ser ENGINE)" << endl;
        out << "SET(INSTALLTARGETS ${INSTALLTARGETS} E"<<name<<ptype<<"_ser)" << endl;
        out << endl;
        out << "IF(VISIT_PARALLEL)" << endl;
        out << "    ADD_PARALLEL_LIBRARY(E"<<name<<ptype<<"_par ${LIBE_SOURCES})" << endl;
        if(!edefsPar.empty())
        {
            CMakeWrite_TargetDefines(out, "    ", "E", "_par", edefsPar);
        }
        if(!ecxxflagsPar.empty())
        {
            CMakeWrite_TargetIncludes(out, "    ", "E", "_par", ecxxflagsPar);
        }
        if(!eldflagsPar.empty())
        {
            CMakeWrite_TargetLinkDirs(out, "    ", "E", "_par", eldflagsPar);
        }
        if (!vtk8_elibsPar.empty())
        {
            out << "\n    if(VTK_VERSION VERSION_EQUAL \"8.1.0\")" << endl;
            out << "        set(vtk_elibsPar " << ToString(vtk8_elibsPar) << ")" << endl;
            out << "    else()" << endl;
            out << "        set(vtk_elibsPar " << ToString(vtk9_elibsPar) << ")" << endl;
            out << "    endif()\n" << endl;
        }
        out << "    TARGET_LINK_LIBRARIES(E"<<name<<ptype<<"_par visitcommon avtpipeline_par";
        if(type == "plot")
            out << " avtplotter_par ";
        else if (type == "operator")
            out << " avtexpressions_par avtfilters_par ";
        else
            out << " avtdatabase_par ";
        out << ToString(libs) << ToString(elibsPar);
        if (!vtk8_libs.empty())
            out << "${vtk_libs} ";
        if (!vtk8_elibsPar.empty())
            out << "${vtk_elibsPar} ";
        out << ")" << endl;
        WriteCMake_ConditionalTargetLinks(out, name, "E", (ptype+"_par"), "    ");
        if (type != "operator" || hasEngineSpecificCode)
            out << "    ADD_TARGET_DEFINITIONS(E"<<name<<ptype<<"_par ENGINE)" << endl;
        out << "    SET(INSTALLTARGETS ${INSTALLTARGETS} E"<<name<<ptype<<"_par)" << endl;
        out << "ENDIF(VISIT_PARALLEL)" << endl;
        out << endl;
    }

    bool CustomFilesUseFortran(const std::vector<QString> &files) const
    {
        const char *ext[] = {".f", ".f77", ".f90", ".f95", ".for",
                             ".F", ".F77", ".F90", ".F95", ".FOR"};
        for(size_t i = 0; i < files.size(); ++i)
        {
            for(int j = 0; j < 10; ++j)
            {
                if(files[i].endsWith(ext[j]))
                    return true;
            }
        }
        return false;
    }

    void WriteCMake_PlotOperator(QTextStream &out,
                         const QString &guilibname,
                         const QString &viewerlibname)
    {
        bool useFortran = false;

        out << "PROJECT(" << name<< "_" << type << ")" << endl;
        out << endl;
        if (using_dev)
        {
            out << "ADD_" << type.toUpper() << "_CODE_GEN_TARGETS(" << name << ")" << endl;
            out << endl;
        }
        out << "SET(COMMON_SOURCES" << endl;
        out << name << "PluginInfo.C" << endl;
        out << name << "CommonPluginInfo.C" << endl;
        out << atts->name << ".C" << endl;
        out << ")" << endl;
        out << endl;
        out << "SET(LIBI_SOURCES" << endl;
        out << name << "PluginInfo.C" << endl;
        out << ")" << endl;
        out << endl;
        WriteCMake_ConditionalSources(out, "I", "");

        // libG sources
        out << "SET(LIBG_SOURCES" << endl;
        out << name << "GUIPluginInfo.C" << endl;
        out << "Qvis" << name;
        if (type == "plot")
            out << "PlotWindow.C" << endl;
        else
            out << "Window.C" << endl;
        out << "${COMMON_SOURCES}" << endl;
        if (customgfiles)
        {
            useFortran |= CustomFilesUseFortran(gfiles);
            for (size_t i=0; i<gfiles.size(); i++)
                out << gfiles[i] << endl;
        }
        else
            for (size_t i=0; i<defaultgfiles.size(); i++)
                out << defaultgfiles[i] << endl;
        out << ")" << endl;
        out << endl;
        WriteCMake_ConditionalSources(out, "G", "");

        // libV sources
        out << "SET(LIBV_SOURCES" << endl;
        out << name<<"ViewerEnginePluginInfo.C" << endl;
        out << name<<"ViewerPluginInfo.C" << endl;
        if(type == "plot")
            out << "avt"<<name<<"Plot.C" << endl;
        if (customvfiles)
        {
            useFortran |= CustomFilesUseFortran(vfiles);
            for (size_t i=0; i<vfiles.size(); i++)
                out << vfiles[i] << endl;
        }
        else
            for (size_t i=0; i<defaultvfiles.size(); i++)
                out << defaultvfiles[i] << endl;
        out << "${COMMON_SOURCES}" << endl;
        out << ")" << endl;
        if (customvwfiles)
        {
            out << "SET(LIBV_MOC_SOURCES" << endl;
            for (size_t i=0; i<vwfiles.size(); i++)
                out << vwfiles[i] << endl;
            out << ")" << endl;
        }
        out << endl;
        WriteCMake_ConditionalSources(out, "V", "");

        // libE sources
        out << "SET(LIBE_SOURCES" << endl;
        out << name<<"ViewerEnginePluginInfo.C" << endl;
        out << name<<"EnginePluginInfo.C" << endl;
        if (type == "plot")
            out << "avt"<<name<<"Plot.C" << endl;
        if (customefiles)
        {
            useFortran |= CustomFilesUseFortran(efiles);
            for (size_t i=0; i<efiles.size(); i++)
                out << efiles[i] << endl;
        }
        else
            for (size_t i=0; i<defaultefiles.size(); i++)
                out << defaultefiles[i] << endl;
        out << "${COMMON_SOURCES}" << endl;
        out << ")" << endl;
        out << endl;
        WriteCMake_ConditionalSources(out, "E", "");

        if(useFortran)
        {
            out << "ENABLE_LANGUAGE(Fortran)" << endl;
        }

        WriteCMake_PlotOperator_Includes(out);
        WriteCMake_ConditionalIncludes(out);

        // Pass other CXXFLAGS
        bool added_defs = false;
        for (size_t i=0; i<cxxflags.size(); i++)
        {
            if(!cxxflags[i].startsWith("${") &&
               !cxxflags[i].startsWith("$(") &&
               !cxxflags[i].startsWith("-I"))
            {
                 out << "ADD_DEFINITIONS(" << cxxflags[i] << ")" << endl;
                 added_defs = true;
            }
        }

        // Pass DEFINITIONS
        for (size_t i=0; i<defs.size(); i++)
        {
            out << "ADD_DEFINITIONS(" << defs[i] << ")" << endl;
            added_defs = true;
        }
        if (added_defs)
            out << endl;

        WriteCMake_ConditionalDefinitions(out);

        if (!vtk8_libs.empty())
        {
            out << "if(VTK_VERSION VERSION_EQUAL \"8.1.0\")" << endl;
            out << "    set(vtk_libs " << ToString(vtk8_libs) << ")" << endl;
            out << "else()" << endl;
            out << "    set(vtk_libs " << ToString(vtk9_libs) << ")" << endl;
            out << "endif()\n" << endl;
        }

        std::vector<QString> linkDirs;
        linkDirs.push_back("${VISIT_LIBRARY_DIR}");
        if (!using_dev)
        {
            linkDirs.push_back("${VISIT_ARCHIVE_DIR}");
        }
        // Extract extra link directories from LDFLAGS if they have ${},$(),-L
        for (size_t i=0; i<ldflags.size(); i++)
        {
            if(ldflags[i].startsWith("${") || ldflags[i].startsWith("$("))
                 linkDirs.push_back(ldflags[i]);
            else if(ldflags[i].startsWith("-L"))
                 linkDirs.push_back(ldflags[i].right(ldflags[i].size()-2));
        }

        out << "LINK_DIRECTORIES(" << ToString(linkDirs) << ")" << endl;
        out << endl;
        QString ptype;
        if (type == "plot")
            ptype = "Plot";
        else
            ptype = "Operator";
        out << "ADD_LIBRARY(I"<<name<<ptype<<" ${LIBI_SOURCES})" << endl;
        out << "TARGET_LINK_LIBRARIES(I"<<name<<ptype<<" visitcommon)" << endl;
        WriteCMake_ConditionalTargetLinks(out, name, "I", ptype, "");
        out << "SET(INSTALLTARGETS I"<<name<<ptype<<")" << endl;
        out << endl;

        out << "IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)" << endl;
        if (!vtk8_glibs.empty())
        {
            out << "    if(VTK_VERSION VERSION_EQUAL \"8.1.0\")" << endl;
            out << "        set(vtk_glibs " << ToString(vtk8_glibs) << ")" << endl;
			out << "    else()" << endl;
            out << "        set(vtk_glibs " << ToString(vtk9_glibs) << ")" << endl;
            out << "    endif()" << endl;
        }
        out << "    ADD_LIBRARY(G"<<name<<ptype<<" ${LIBG_SOURCES})" << endl;
        out << "    set_target_properties(G"<<name<<ptype<<" PROPERTIES AUTOMOC ON)" << endl;
        out << "    TARGET_LINK_LIBRARIES(G" << name << ptype <<" visitcommon "
            << guilibname << " " << ToString(libs) << ToString(glibs);
        if (!vtk8_libs.empty())
            out << "${vtk_libs} ";
        if (!vtk8_glibs.empty())
            out << "${vtk_glibs} ";
        out << ")" << endl;
        WriteCMake_ConditionalTargetLinks(out, name, "G", ptype, "    ");
        out << endl;

        if (!vtk8_vlibs.empty())
        {
            out << "    if(VTK_VERSION VERSION_EQUAL \"8.1.0\")" << endl;
            out << "        set(vtk_vlibs " << ToString(vtk8_vlibs) << ")" << endl;
            out << "    else()" << endl;
            out << "        set(vtk_vlibs " << ToString(vtk9_vlibs) << ")" << endl;
            out << "    endif()" << endl;
        }
        out << "    ADD_LIBRARY(V"<<name<<ptype<<" ${LIBV_SOURCES})" << endl;
        out << "    ADD_TARGET_DEFINITIONS(V"<<name<<ptype<<" VIEWER)" << endl;
        if (customvwfiles)
        {
            out << "    set_target_properties(V" << name << ptype << " PROPERTIES AUTOMOC ON)" << endl;
        }
        out << "    TARGET_LINK_LIBRARIES(V" << name << ptype << " visitcommon "
            << viewerlibname << " " << ToString(libs) << ToString(vlibs);
        if (!vtk8_libs.empty())
            out << "${vtk_libs} ";
        if (!vtk8_vlibs.empty())
            out << "${vtk_vlibs} ";
        out << ")" << endl;
        WriteCMake_ConditionalTargetLinks(out, name, "V", ptype, "    ");
        out << endl;
        out << "    SET(INSTALLTARGETS ${INSTALLTARGETS} G"<<name<<ptype<<" V"<<name<<ptype<<")" << endl;
        out << endl;
        // libS sources
        out << "    IF(VISIT_PYTHON_SCRIPTING)" << endl;
        out << "        SET(LIBS_SOURCES" << endl;
        out << "            " << name<<"ScriptingPluginInfo.C" << endl;
        out << "            Py"<<atts->name<<".C" << endl;
        if (customsfiles)
            for (size_t i=0; i<sfiles.size(); i++)
                out << "            " << sfiles[i] << endl;
        else
            for (size_t i=0; i<defaultsfiles.size(); i++)
                out << "            " << defaultsfiles[i] << endl;
        out << "            ${COMMON_SOURCES}" << endl;
        out << "        )" << endl;
        WriteCMake_ConditionalSources(out, "S", "        ");
        out << "        ADD_LIBRARY(S"<<name<<ptype<<" ${LIBS_SOURCES})" << endl;
        out << "        IF(WIN32)" << endl;
        out << "            # This prevents python from #defining snprintf as _snprintf" << endl;
        out << "            SET_TARGET_PROPERTIES(S"<<name<<ptype<<" PROPERTIES COMPILE_DEFINITIONS HAVE_SNPRINTF)" << endl;
        out << "        ENDIF()" << endl;
        out << "        TARGET_LINK_LIBRARIES(S" << name << ptype
            << " visitcommon visitpy ${PYTHON_LIBRARY})" << endl;
        WriteCMake_ConditionalTargetLinks(out, name, "S", ptype, "        ");
        out << "        SET(INSTALLTARGETS ${INSTALLTARGETS} S" << name
            << ptype << ")" << endl;
        out << "    ENDIF(VISIT_PYTHON_SCRIPTING)" << endl;
        out << endl;
        // Java sources
        out << "    IF(VISIT_JAVA)" << endl;
        out << "        FILE(COPY " << atts->name<<".java DESTINATION ${JavaClient_BINARY_DIR}/src/" << type << "s)" << endl;
        out << "        ADD_CUSTOM_TARGET(Java"<<name<<" ALL ${Java_JAVAC_EXECUTABLE} ${VISIT_Java_FLAGS} -d ${JavaClient_BINARY_DIR} -classpath ${JavaClient_BINARY_DIR} -sourcepath ${JavaClient_BINARY_DIR} ";
        if(customjfiles)
        {
            for(size_t i = 0; i < jfiles.size(); ++i)
                out << jfiles[i] << " ";
        }
        out << atts->name<<".java" << endl;
        out << "            DEPENDS JavaClient" << endl;
        out << "            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})" << endl;
        out << "    ENDIF(VISIT_JAVA)" << endl;

        out << "ENDIF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)" << endl;
        out << endl;

        CMakeAdd_EngineTargets(out);

        out << "VISIT_INSTALL_" << type.toUpper() << "_PLUGINS(${INSTALLTARGETS})" << endl;
        out << "VISIT_PLUGIN_TARGET_OUTPUT_DIR(" << type << "s ${INSTALLTARGETS})" << endl;
        if (using_dev)
          out << "VISIT_PLUGIN_TARGET_FOLDER(" << type << "s " << name
              << " ${INSTALLTARGETS})" << endl;
        out << endl;
#ifdef _WIN32
        if (!using_dev)
        {
          out << "MESSAGE(STATUS \"Plugin will be installed to: ${VISIT_PLUGIN_DIR}\")" << endl;
        }
#endif
    }

    void WriteCMake_Database(QTextStream &out)
    {
        bool useFortran = false;

        out << "PROJECT("<<name<<"_database)" << endl;
        out << endl;
        if (using_dev)
        {
        out << "ADD_DATABASE_CODE_GEN_TARGETS(" << name ;
        if(skipInfoGen)
            out << " SKIP_INFO";
        out << ")" << endl;
        out << endl;
        }
        out << "SET(COMMON_SOURCES" << endl;
        out << ""<<name<<"PluginInfo.C" << endl;
        out << ""<<name<<"CommonPluginInfo.C" << endl;
        out << ")" << endl;
        out << endl;
        out << "SET(LIBI_SOURCES" << endl;
        out << ""<<name<<"PluginInfo.C" << endl;
        out << ")" << endl;
        out << endl;
        WriteCMake_ConditionalSources(out, "I", "");
        if(!onlyEnginePlugin)
        {
            out << "SET(LIBM_SOURCES" << endl;
            out << ""<<name<<"MDServerPluginInfo.C" << endl;
            out << "${COMMON_SOURCES}" << endl;
            if (custommfiles)
            {
                useFortran |= CustomFilesUseFortran(mfiles);
                for (size_t i=0; i<mfiles.size(); i++)
                    out << mfiles[i] << endl;
            }
            else
                for (size_t i=0; i<defaultmfiles.size(); i++)
                    out << defaultmfiles[i] << endl;
            out << ")" << endl;
            WriteCMake_ConditionalSources(out, "M", "");
            if (customwmfiles)
            {
                useFortran |= CustomFilesUseFortran(wmfiles);
                out << "IF(WIN32)" << endl;
                out << "    SET(LIBM_WIN32_SOURCES" << endl;
                for (size_t i=0; i<wmfiles.size(); i++)
                    out << "    " << wmfiles[i] << endl;
                out << "    )" << endl;
                for (size_t i=0; i<wmfiles.size(); i++)
                {
                    if(wmfiles[i].endsWith(".c"))
                    {
                        out << "    SET_SOURCE_FILES_PROPERTIES("
                            << wmfiles[i] << endl;
                        out << "        PROPERTIES LANGUAGE CXX)" << endl;
                    }
                }
                out << "ENDIF(WIN32)" << endl;
            }
            out << endl;
        }
        if(!noEnginePlugin)
        {
            out << "SET(LIBE_SOURCES" << endl;
            out <<name<<"EnginePluginInfo.C" << endl;
            out << "${COMMON_SOURCES}" << endl;
            if (customefiles)
            {
                useFortran |= CustomFilesUseFortran(efiles);
                for (size_t i=0; i<efiles.size(); i++)
                    out << efiles[i] << endl;
            }
            else
                for (size_t i=0; i<defaultefiles.size(); i++)
                    out << defaultefiles[i] << endl;
            out << ")" << endl;
            WriteCMake_ConditionalSources(out, "E", "");
            if (customwefiles)
            {
                useFortran |= CustomFilesUseFortran(wefiles);
                out << "IF(WIN32)" << endl;
                out << "    SET(LIBE_WIN32_SOURCES" << endl;
                for (size_t i=0; i<wefiles.size(); i++)
                    out << "    " << wefiles[i] << endl;
                out << "    )" << endl;
                for (size_t i=0; i<wefiles.size(); i++)
                {
                    if(wefiles[i].endsWith(".c"))
                    {
                        out << "    SET_SOURCE_FILES_PROPERTIES("
                            << wefiles[i] << endl;
                        out << "        PROPERTIES LANGUAGE CXX)" << endl;
                    }
                }
                out << "ENDIF(WIN32)" << endl;
            }
            out << endl;
        }

        // take any ${} from the CXXFLAGS to mean a variable that contains
        // include directories.
        std::vector<QString> extraIncludes;
        for (size_t i=0; i<cxxflags.size(); i++)
        {
            if(cxxflags[i].startsWith("${"))
                 extraIncludes.push_back(ConvertToProperVisItIncludeDir(cxxflags[i]));
            else if(cxxflags[i].startsWith("$("))
                 extraIncludes.push_back(ConvertToProperVisItIncludeDir(ConvertDollarParenthesis(cxxflags[i])));
            else if(cxxflags[i].startsWith("-I"))
                 extraIncludes.push_back(ConvertToProperVisItIncludeDir(cxxflags[i].right(cxxflags[i].size()-2)));
        }
        out << "INCLUDE_DIRECTORIES(" << endl;
        out << "${CMAKE_CURRENT_SOURCE_DIR}" << endl;
        if(extraIncludes.size() > 0)
            out << ToString(extraIncludes, true) ;
        out << "${VISIT_DATABASE_INCLUDES}" << endl;
        out << ")" << endl;
        out << endl;

        WriteCMake_ConditionalIncludes(out);

        // Pass other CXXFLAGS
        for (size_t i=0; i<cxxflags.size(); i++)
        {
            if(!cxxflags[i].startsWith("${") &&
               !cxxflags[i].startsWith("$(") &&
               !cxxflags[i].startsWith("-I"))
                 out << "ADD_DEFINITIONS(" << cxxflags[i] << ")" << endl;
        }

        // Pass defines
        for (size_t i=0; i<defs.size(); i++)
        {
            out << "ADD_DEFINITIONS(" << defs[i] << ")" << endl;
        }
        if (!defs.empty())
            out << endl;

        // Pass Win32-only defines
        if (!windefs.empty())
        {
            out << "if(WIN32)" << endl;
            for (size_t i=0; i<windefs.size(); i++)
            {
                out << "    add_compile_definitions(" << windefs[i] << ")" << endl;
            }
            out << "endif()"<< endl;
            out << endl;
        }

        WriteCMake_ConditionalDefinitions(out);

        if (!vtk8_libs.empty())
        {
            out << "if(VTK_VERSION VERSION_EQUAL \"8.1.0\")" << endl;
            out << "    set(vtk_libs " << ToString(vtk8_libs) << ")" << endl;
            out << "else()" << endl;
            out << "    set(vtk_libs " << ToString(vtk9_libs) << ")" << endl;
            out << "endif()\n" << endl;
        }

        if(useFortran)
        {
            out << "ENABLE_LANGUAGE(Fortran)" << endl;
            out << endl;
        }

        // Extract extra link directories from LDFLAGS if they have ${},$(),-L
        std::vector<QString> linkDirs;
        linkDirs.push_back("${VISIT_LIBRARY_DIR}");
        for (size_t i=0; i<ldflags.size(); i++)
        {
            if(ldflags[i].startsWith("${") || ldflags[i].startsWith("$("))
                 linkDirs.push_back(ldflags[i]);
            else if(ldflags[i].startsWith("-L"))
                 linkDirs.push_back(ldflags[i].right(ldflags[i].size()-2));
        }
        out << "LINK_DIRECTORIES(" << ToString(linkDirs) << ")" << endl;
        out << endl;
        out << "ADD_LIBRARY(I"<<name<<"Database ${LIBI_SOURCES})" << endl;
        out << "TARGET_LINK_LIBRARIES(I"<<name<<"Database visitcommon)" << endl;
        WriteCMake_ConditionalTargetLinks(out, name, "I", "Database", "");
        out << "SET(INSTALLTARGETS I"<<name<<"Database)" << endl;
        out << endl;
        if(!onlyEnginePlugin)
        {
            out << "IF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)" << endl;

            out << "    ADD_LIBRARY(M"<<name<<"Database ${LIBM_SOURCES}";
            if (customwmfiles)
                out << "     ${LIBM_WIN32_SOURCES}";
            out << ")" << endl;
            if(!mdefs.empty())
            {
                CMakeWrite_TargetDefines(out, "    ", "M", "", mdefs);
            }
            if(!mcxxflags.empty())
            {
                CMakeWrite_TargetIncludes(out, "    ", "M", "", mcxxflags);
            }
            if(!mldflags.empty())
            {
                CMakeWrite_TargetLinkDirs(out, "    ", "M", "", mldflags);
            }
            if (!vtk8_mlibs.empty())
            {
                out << "    if(VTK_VERSION VERSION_EQUAL \"8.1.0\")" << endl;
                out << "        set(vtk_mlibs " << ToString(vtk8_mlibs) << ")" << endl;
                out << "    else()" << endl;
                out << "        set(vtk_mlibs " << ToString(vtk9_mlibs) << ")" << endl;
                out << "    endif()\n" << endl;
            }
            out << "    TARGET_LINK_LIBRARIES(M"<<name<<"Database visitcommon avtdbatts avtdatabase_ser " << ToString(libs) << ToString(mlibs);
            if (!vtk8_libs.empty())
                out << "${vtk_libs} ";
            if (!vtk8_mlibs.empty())
                out << "${vtk_mlibs} ";
            out << ")" << endl;
            WriteCMake_ConditionalTargetLinks(out, name, "M", "Database", "    ");
            out << "    ADD_TARGET_DEFINITIONS(M"<<name<<"Database MDSERVER)" << endl;
            out << "    SET(INSTALLTARGETS ${INSTALLTARGETS} M"<<name<<"Database)" << endl;
            out << "ENDIF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)" << endl;
            out << endl;
        }
        if(!noEnginePlugin)
        {
            CMakeAdd_EngineTargets(out);
        }
        out << "VISIT_INSTALL_DATABASE_PLUGINS(${INSTALLTARGETS})" << endl;
        out << "VISIT_PLUGIN_TARGET_OUTPUT_DIR(databases ${INSTALLTARGETS})" << endl;
        if (using_dev)
          out << "VISIT_PLUGIN_TARGET_FOLDER(databases " << name
              << " ${INSTALLTARGETS})" << endl;
        out << endl;
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
            << "BY xml2cmake" << endl;

        QString qvisithome(visithome);
        QString qvisitplugdirpub(visitplugdirpub);
        QString qvisitplugdirpri(visitplugdirpri);
#ifdef _WIN32
        qvisithome       = ToCMakePath(qvisithome);
        qvisitplugdirpub = ToCMakePath(qvisitplugdirpub);
        qvisitplugdirpri = ToCMakePath(qvisitplugdirpri);
#endif
        FilterVTKLibs(libs,    vtk8_libs,     vtk9_libs);
        FilterVTKLibs(mlibs,   vtk8_mlibs,    vtk9_mlibs);
        FilterVTKLibs(glibs,   vtk8_glibs,    vtk9_glibs);
        FilterVTKLibs(vlibs,   vtk8_vlibs,    vtk9_vlibs);
        FilterVTKLibs(elibsSer,vtk8_elibsSer,vtk9_elibsSer);
        FilterVTKLibs(elibsPar,vtk8_elibsPar,vtk9_elibsPar);

        // If we're not using a development version then we need to always
        // include something in the generated output.
        if(!using_dev)
        {
            out << "CMAKE_MINIMUM_REQUIRED(VERSION 3.8 FATAL_ERROR)" << endl;
            if(installpublic)
            {
                out << "SET(VISIT_PLUGIN_DIR \"" << qvisitplugdirpub
                    << "\")" << endl;
            }
            else // installprivate or default
            {
                out << "SET(VISIT_PLUGIN_DIR \"" << qvisitplugdirpri
                    << "\")" << endl;
            }

            out << "INCLUDE(\"" << qvisithome
                << "/include/PluginVsInstall.cmake\")" << endl;
            out << "INCLUDE(\"" << qvisithome
                << "/include/VisItLibraryDependencies.cmake\")" << endl;
            out << endl;
        }
        else
        {
            // We're using a development version but we're installing public
            // or private.
            if(installpublic)
            {
               out << "SET(VISIT_PLUGIN_DIR " << qvisitplugdirpub << ")" << endl;
            }

            if(installprivate)
            {
               out << "SET(VISIT_PLUGIN_DIR " << qvisitplugdirpri << ")" << endl;
            }
        }

        QString guilibname("gui");
        QString viewerlibname("viewer");
#ifdef WIN32
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
            WriteCMake_Database(out);
        else
            WriteCMake_PlotOperator(out, guilibname, viewerlibname);

        WriteCMake_AdditionalCode(out);
    }

private:
    // couldn't think of a way to support both VTK8 and VTK9 via codegen
    // without also requiring re-generation when switching between 8 and 9.
    // so creating extra storage for them here.
    std::vector<QString> vtk8_libs,     vtk9_libs;
    std::vector<QString> vtk8_glibs,    vtk9_glibs;
    std::vector<QString> vtk8_mlibs,    vtk9_mlibs;
    std::vector<QString> vtk8_vlibs,    vtk9_vlibs;
    std::vector<QString> vtk8_elibsSer, vtk9_elibsSer;
    std::vector<QString> vtk8_elibsPar, vtk9_elibsPar;
};


// ----------------------------------------------------------------------------
//                           Override default types
// ----------------------------------------------------------------------------
#define Plugin       CMakeGeneratorPlugin

#endif
