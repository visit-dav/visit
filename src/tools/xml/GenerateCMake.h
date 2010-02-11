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
//    I made all of the database plugins use the ADD_TARGET_DEFINITIONS function.
//
// ****************************************************************************

class CMakeGeneratorPlugin : public Plugin
{
  public:
    CMakeGeneratorPlugin(const QString &n,const QString &l,const QString &t,
        const QString &vt,const QString &dt, const QString &v, const QString &ifile,
        bool hw, bool ho, bool onlyengine, bool noengine) : 
        Plugin(n,l,t,vt,dt,v,ifile,hw,ho,onlyengine,noengine)
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
    GetFilesWith(const QString &name, const vector<QString> &input, std::set<QString> &output)
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
    ToString(const vector<QString> &vec) const
    {
        QString s;
        for(size_t i = 0; i < vec.size(); ++i)
            s += (ConvertDollarParenthesis(vec[i]) + " ");
        return s;
    }

    QString
    VisItIncludeDir() const
    {
        return using_dev ? "${VISIT_INCLUDE_DIR}" : "${VISIT_INCLUDE_DIR}/visit";
    }

    void WriteCMake_PlotOperator_Includes(QTextStream &out, bool isOperator)
    {
        // take any ${} from the CXXFLAGS to mean a variable that contains 
        // include directories.
        vector<QString> extraIncludes;
        for (size_t i=0; i<cxxflags.size(); i++)
        {
            if(cxxflags[i].startsWith("${"))
                 extraIncludes.push_back(cxxflags[i]);
            else if(cxxflags[i].startsWith("$("))
                 extraIncludes.push_back(ConvertDollarParenthesis(cxxflags[i]));
        }

        // Includes
        out << "INCLUDE_DIRECTORIES(" << endl;
        out << "${CMAKE_CURRENT_SOURCE_DIR}" << endl;
        out << "${VISIT_COMMON_INCLUDES}" << endl;
        out << VisItIncludeDir() << "/avt/DBAtts/MetaData" << endl;
        out << VisItIncludeDir() << "/avt/DBAtts/SIL" << endl;
        out << VisItIncludeDir() << "/avt/Database/Database" << endl;
        if(isOperator)
        {
            out << VisItIncludeDir() << "/avt/Expressions/Abstract" << endl;
            out << VisItIncludeDir() << "/avt/Expressions/CMFE" << endl;
            out << VisItIncludeDir() << "/avt/Expressions/Conditional" << endl;
            out << VisItIncludeDir() << "/avt/Expressions/Derivations" << endl;
            out << VisItIncludeDir() << "/avt/Expressions/General" << endl;
            out << VisItIncludeDir() << "/avt/Expressions/ImageProcessing" << endl;
            out << VisItIncludeDir() << "/avt/Expressions/Management" << endl;
            out << VisItIncludeDir() << "/avt/Expressions/Math" << endl;
            out << VisItIncludeDir() << "/avt/Expressions/MeshQuality" << endl;
        }
        out << VisItIncludeDir() << "/avt/FileWriter" << endl;
        out << VisItIncludeDir() << "/avt/Filters" << endl;
        out << VisItIncludeDir() << "/avt/IVP" << endl;
        out << VisItIncludeDir() << "/avt/Math" << endl;
        out << VisItIncludeDir() << "/avt/Pipeline/AbstractFilters" << endl;
        out << VisItIncludeDir() << "/avt/Pipeline/Data" << endl;
        out << VisItIncludeDir() << "/avt/Pipeline/Pipeline" << endl;
        out << VisItIncludeDir() << "/avt/Pipeline/Sinks" << endl;
        out << VisItIncludeDir() << "/avt/Pipeline/Sources" << endl;
        out << VisItIncludeDir() << "/avt/Plotter" << endl;
        out << VisItIncludeDir() << "/avt/QtVisWindow" << endl;
        out << VisItIncludeDir() << "/avt/View" << endl;
        out << VisItIncludeDir() << "/avt/VisWindow/Colleagues" << endl;
        out << VisItIncludeDir() << "/avt/VisWindow/Interactors" << endl;
        out << VisItIncludeDir() << "/avt/VisWindow/Proxies" << endl;
        out << VisItIncludeDir() << "/avt/VisWindow/Tools" << endl;
        out << VisItIncludeDir() << "/avt/VisWindow/VisWindow" << endl;
        out << VisItIncludeDir() << "/gui" << endl;
        if(isOperator)
        {
            out << VisItIncludeDir() << "/mdserver/proxy" << endl;
            out << VisItIncludeDir() << "/mdserver/rpc" << endl;
        }
        out << VisItIncludeDir() << "/viewer/main" << endl;
        out << VisItIncludeDir() << "/viewer/proxy" << endl;
        out << VisItIncludeDir() << "/viewer/rpc" << endl;
        out << VisItIncludeDir() << "/winutil" << endl;
        out << VisItIncludeDir() << "/visit_vtk/full" << endl;
        out << VisItIncludeDir() << "/visit_vtk/lightweight" << endl;
        out << "${QT_INCLUDE_DIR}" << endl;
        out << "${QT_QTCORE_INCLUDE_DIR}" << endl;
        out << "${QT_QTGUI_INCLUDE_DIR}" << endl;
        out << "${VTK_INCLUDE_DIRS} " << endl;
        out << "${PYTHON_INCLUDE_PATH} " << endl;
        out << VisItIncludeDir() << "/visitpy/visitpy ";
        if(extraIncludes.size() > 0)
            out << ToString(extraIncludes);
        out << endl;
        out << ")" << endl;
    }

    void WriteCMake_Plot(QTextStream &out)
    {
        out << "PROJECT(" << name<< ")" << endl;
        out << endl;

        out << "SET(COMMON_SOURCES" << endl;
        out << name << "PluginInfo.C" << endl;
        out << name << "CommonPluginInfo.C" << endl;
        out << atts->name << ".C" << endl;
        out << ")" << endl;
        out << endl;
        out << "SET(LIBI_SOURCES " << endl;
        out << name << "PluginInfo.C" << endl;
        out << ")" << endl;
        out << endl;

        // libG sources
        out << "SET(LIBG_SOURCES" << endl;
        out << name << "GUIPluginInfo.C" << endl;
        out << "Qvis" << name << "PlotWindow.C" << endl;
        out << "${COMMON_SOURCES}" << endl;
        if (customgfiles)
            for (size_t i=0; i<gfiles.size(); i++)
                out << gfiles[i] << endl;
        else
            for (size_t i=0; i<defaultgfiles.size(); i++)
                out << defaultgfiles[i] << endl;
        out << ")" << endl;
        out << "SET(LIBG_MOC_SOURCES" << endl;
        out << "Qvis" << name << "PlotWindow.h" << endl;
        if (customwfiles)
            for (size_t i=0; i<wfiles.size(); i++)
                out << wfiles[i] << endl;
        out << ")" << endl;
        out << "QT_WRAP_CPP(G"<<name<<"Plot LIBG_SOURCES ${LIBG_MOC_SOURCES})" << endl;
        out << endl;

        // libV sources
        out << "SET(LIBV_SOURCES" << endl;
        out << name<<"ViewerPluginInfo.C" << endl;
        out << "avt"<<name<<"Plot.C" << endl;
        if (customvfiles)
            for (size_t i=0; i<vfiles.size(); i++)
                out << vfiles[i] << endl;
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
            out << "QT_WRAP_CPP(V"<<name<<"Plot LIBV_SOURCES ${LIBV_MOC_SOURCES})" << endl;
        }
        out << endl;

        // libE sources
        out << "SET(LIBE_SOURCES" << endl;
        out << name<<"EnginePluginInfo.C" << endl;
        out << "avt"<<name<<"Plot.C" << endl;
        if (customefiles)
            for (size_t i=0; i<efiles.size(); i++)
                out << efiles[i] << endl;
        else
            for (size_t i=0; i<defaultefiles.size(); i++)
                out << defaultefiles[i] << endl;
        out << "${COMMON_SOURCES}" << endl;
        out << ")" << endl;
        out << endl;

        // Special rules for OpenGL and Mesa sources.
        std::set<QString> openglFiles, mesaFiles;
        GetFilesWith("OpenGL", customvfiles ? vfiles : defaultvfiles, openglFiles);
        GetFilesWith("OpenGL", customefiles ? efiles : defaultefiles, openglFiles);
        GetFilesWith("Mesa", customvfiles ? vfiles : defaultvfiles, mesaFiles);
        GetFilesWith("Mesa", customefiles ? efiles : defaultefiles, mesaFiles);
        if(openglFiles.size() > 0)
        {
            out << "IF (NOT WIN32)" << endl;
            out << "    SET_SOURCE_FILES_PROPERTIES(";
            for(std::set<QString>::iterator it = openglFiles.begin();
                it != openglFiles.end(); ++it)
            {
                 out << *it << " ";
            }
            out << "\n        PROPERTIES" << endl;
            out << "        COMPILE_FLAGS \"-I ${OPENGL_INCLUDE_DIR}\"" << endl;
            out << "    )" << endl;
            out << "ENDIF (NOT WIN32)" << endl;
        }
        if(mesaFiles.size() > 0)
        {
            out << "SET_SOURCE_FILES_PROPERTIES(";
            for(std::set<QString>::iterator it = mesaFiles.begin();
                it != mesaFiles.end(); ++it)
            {
                 out << *it << " ";
            }
            out << "\n    PROPERTIES" << endl;
            out << "    COMPILE_FLAGS \"-I ${MESA_INCLUDE_DIR}\"" << endl;
            out << ")" << endl;
        }

        WriteCMake_PlotOperator_Includes(out, false);

        // Pass other CXXFLAGS
        for (size_t i=0; i<cxxflags.size(); i++)
        {
            if(!cxxflags[i].startsWith("${") && !cxxflags[i].startsWith("$("))
                 out << "ADD_DEFINITIONS(\"" << cxxflags[i] << "\")" << endl;
        }
        out << endl;

#if 0
        if (installpublic)
            out << "SET(LIBRARY_OUTPUT_PATH " << visitplugdirpub << ")" << endl;
        else if (installprivate)
            out << "SET(LIBRARY_OUTPUT_PATH " << visitplugdirpri << ")" << endl;
        else
#endif

        out << endl;
        out << "LINK_DIRECTORIES(${VISIT_LIBRARY_DIR} ${QT_LIBRARY_DIR} ${MESA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR} ${VTK_LIBRARY_DIRS})" << endl;
        out << endl;
        out << "ADD_LIBRARY(I"<<name<<"Plot ${LIBI_SOURCES})" << endl;
        out << "TARGET_LINK_LIBRARIES(I"<<name<<"Plot visitcommon)" << endl;
        out << "SET(INSTALLTARGETS I"<<name<<"Plot)" << endl;
        out << endl;

        out << "IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)" << endl;

        out << "    ADD_LIBRARY(G"<<name<<"Plot ${LIBG_SOURCES})" << endl;
        out << "    TARGET_LINK_LIBRARIES(G"<<name<<"Plot visitcommon gui " << ToString(glibs) << ")" << endl;
        out << endl;
        out << "    ADD_LIBRARY(V"<<name<<"Plot ${LIBV_SOURCES})" << endl;
        out << "    TARGET_LINK_LIBRARIES(V"<<name<<"Plot visitcommon viewer "<< ToString(vlibs) << ")" << endl;
        out << "    SET(INSTALLTARGETS ${INSTALLTARGETS} G"<<name<<"Plot V"<<name<<"Plot)" << endl;
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
        out << "        ADD_LIBRARY(S"<<name<<"Plot ${LIBS_SOURCES})" << endl;
        out << "        TARGET_LINK_LIBRARIES(S"<<name<<"Plot visitcommon visitpy ${PYTHON_LIBRARY})" << endl;
        out << "        SET(INSTALLTARGETS ${INSTALLTARGETS} S"<<name<<"Plot)" << endl;
        out << "    ENDIF(VISIT_PYTHON_SCRIPTING)" << endl;
        out << endl;
        // Java sources
        out << "    IF(VISIT_JAVA)" << endl;
        out << "        ADD_CUSTOM_TARGET(Java"<<name<<" ALL ${CMAKE_Java_COMPILER} ${CMAKE_Java_FLAGS} -d ${VISIT_BINARY_DIR}/java -classpath ${VISIT_BINARY_DIR}/java ";
        if(customjfiles)
        {
            for(size_t i = 0; i < jfiles.size(); ++i)
                out << jfiles[i] << " ";
        }
        out << atts->name<<".java)" << endl;
        out << "        ADD_DEPENDENCIES(Java"<<name<<" JavaClient)" << endl;
        out << "    ENDIF(VISIT_JAVA)" << endl;

        out << "ENDIF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)" << endl;
        out << endl;

        out << "ADD_LIBRARY(E"<<name<<"Plot_ser ${LIBE_SOURCES})" << endl;
        out << "TARGET_LINK_LIBRARIES(E"<<name<<"Plot_ser visitcommon avtplotter_ser avtpipeline_ser "<< ToString(elibsSer) << ")" << endl;
        out << "SET(INSTALLTARGETS ${INSTALLTARGETS} E"<<name<<"Plot_ser)" << endl;
        out << "ADD_TARGET_DEFINITIONS(E"<<name<<"Plot_ser ENGINE)" << endl;
        out << endl;
        out << "IF(VISIT_PARALLEL)" << endl;
        out << "    ADD_PARALLEL_LIBRARY(E"<<name<<"Plot_par ${LIBE_SOURCES})" << endl;
        out << "    TARGET_LINK_LIBRARIES(E"<<name<<"Plot_par visitcommon avtplotter_par avtpipeline_par "<< ToString(elibsPar) << ")" << endl;
        out << "    SET(INSTALLTARGETS ${INSTALLTARGETS} E"<<name<<"Plot_par)" << endl;
        out << "    ADD_TARGET_DEFINITIONS(E"<<name<<"Plot_par ENGINE)" << endl;
        out << "ENDIF(VISIT_PARALLEL)" << endl;
        out << endl;
        out << "VISIT_INSTALL_PLOT_PLUGINS(${INSTALLTARGETS})" << endl;
        out << "VISIT_PLUGIN_TARGET_PREFIX(${INSTALLTARGETS})" << endl;
        out << endl;
    }

    void WriteCMake_Operator(QTextStream &out)
    {
        out << "PROJECT(" << name<< ")" << endl;
        out << endl;

        out << "SET(COMMON_SOURCES" << endl;
        out << name << "PluginInfo.C" << endl;
        out << name << "CommonPluginInfo.C" << endl;
        out << atts->name << ".C" << endl;
        out << ")" << endl;
        out << endl;
        out << "SET(LIBI_SOURCES " << endl;
        out << name << "PluginInfo.C" << endl;
        out << ")" << endl;
        out << endl;

        // libG sources
        out << "SET(LIBG_SOURCES" << endl;
        out << name << "GUIPluginInfo.C" << endl;
        out << "Qvis" << name << "Window.C" << endl;
        out << "${COMMON_SOURCES}" << endl;
        if (customgfiles)
            for (size_t i=0; i<gfiles.size(); i++)
                out << gfiles[i] << endl;
        else
            for (size_t i=0; i<defaultgfiles.size(); i++)
                out << defaultgfiles[i] << endl;
        out << ")" << endl;
        out << "SET(LIBG_MOC_SOURCES" << endl;
        out << "Qvis" << name << "Window.h" << endl;
        if (customwfiles)
            for (size_t i=0; i<wfiles.size(); i++)
                out << wfiles[i] << endl;
        out << ")" << endl;
        out << "QT_WRAP_CPP(G"<<name<<"Operator LIBG_SOURCES ${LIBG_MOC_SOURCES})" << endl;
        out << endl;

        // libV sources
        out << "SET(LIBV_SOURCES" << endl;
        out << name<<"ViewerPluginInfo.C" << endl;
        if (customvfiles)
            for (size_t i=0; i<vfiles.size(); i++)
                out << vfiles[i] << endl;
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
            out << "QT_WRAP_CPP(V"<<name<<"Operator LIBV_SOURCES ${LIBV_MOC_SOURCES})" << endl;
        }
        out << endl;

        // libE sources
        out << "SET(LIBE_SOURCES" << endl;
        out << name<<"EnginePluginInfo.C" << endl;
        if (customefiles)
            for (size_t i=0; i<efiles.size(); i++)
                out << efiles[i] << endl;
        else
            for (size_t i=0; i<defaultefiles.size(); i++)
                out << defaultefiles[i] << endl;
        out << "${COMMON_SOURCES}" << endl;
        out << ")" << endl;
        out << endl;

        WriteCMake_PlotOperator_Includes(out, true);

        // Pass other CXXFLAGS
        for (size_t i=0; i<cxxflags.size(); i++)
        {
            if(!cxxflags[i].startsWith("${") && !cxxflags[i].startsWith("$("))
                 out << "ADD_DEFINITIONS(\"" << cxxflags[i] << "\")" << endl;
        }
        out << endl;

#if 0
        if (installpublic)
            out << "SET(LIBRARY_OUTPUT_PATH " << visitplugdirpub << ")" << endl;
        else if (installprivate)
            out << "SET(LIBRARY_OUTPUT_PATH " << visitplugdirpri << ")" << endl;
        else
#endif

        out << endl;
        out << "LINK_DIRECTORIES(${VISIT_LIBRARY_DIR} ${QT_LIBRARY_DIR} ${MESA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR} ${VTK_LIBRARY_DIRS})" << endl;
        out << endl;
        out << "ADD_LIBRARY(I"<<name<<"Operator ${LIBI_SOURCES})" << endl;
        out << "TARGET_LINK_LIBRARIES(I"<<name<<"Operator visitcommon)" << endl;
        out << "SET(INSTALLTARGETS I"<<name<<"Operator)" << endl;
        out << endl;

        out << "IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)" << endl;

        out << "    ADD_LIBRARY(G"<<name<<"Operator ${LIBG_SOURCES})" << endl;
        out << "    TARGET_LINK_LIBRARIES(G"<<name<<"Operator visitcommon gui " << ToString(glibs) << ")" << endl;
        out << endl;
        out << "    ADD_LIBRARY(V"<<name<<"Operator ${LIBV_SOURCES})" << endl;
        out << "    TARGET_LINK_LIBRARIES(V"<<name<<"Operator visitcommon viewer "<< ToString(vlibs) << ")" << endl;
        out << "    SET(INSTALLTARGETS ${INSTALLTARGETS} G"<<name<<"Operator V"<<name<<"Operator)" << endl;
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
        out << "        ADD_LIBRARY(S"<<name<<"Operator ${LIBS_SOURCES})" << endl;
        out << "        TARGET_LINK_LIBRARIES(S"<<name<<"Operator visitcommon visitpy ${PYTHON_LIBRARY})" << endl;
        out << "        SET(INSTALLTARGETS ${INSTALLTARGETS} S"<<name<<"Operator)" << endl;
        out << "    ENDIF(VISIT_PYTHON_SCRIPTING)" << endl;
        out << endl;
        // Java sources
        out << "    IF(VISIT_JAVA)" << endl;
        out << "        ADD_CUSTOM_TARGET(Java"<<name<<" ALL ${CMAKE_Java_COMPILER} ${CMAKE_Java_FLAGS} -d ${VISIT_BINARY_DIR}/java -classpath ${VISIT_BINARY_DIR}/java ";
        if(customjfiles)
        {
            for(size_t i = 0; i < jfiles.size(); ++i)
                out << jfiles[i] << " ";
        }
        out << atts->name<<".java)" << endl;
        out << "        ADD_DEPENDENCIES(Java"<<name<<" JavaClient)" << endl;
        out << "    ENDIF(VISIT_JAVA)" << endl;

        out << "ENDIF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)" << endl;
        out << endl;

        out << "ADD_LIBRARY(E"<<name<<"Operator_ser ${LIBE_SOURCES})" << endl;
        out << "TARGET_LINK_LIBRARIES(E"<<name<<"Operator_ser visitcommon avtexpressions_ser avtfilters_ser avtpipeline_ser "<< ToString(elibsSer) << ")" << endl;
        out << "SET(INSTALLTARGETS ${INSTALLTARGETS} E"<<name<<"Operator_ser)" << endl;
        out << endl;
        out << "IF(VISIT_PARALLEL)" << endl;
        out << "    ADD_PARALLEL_LIBRARY(E"<<name<<"Operator_par ${LIBE_SOURCES})" << endl;
        out << "    TARGET_LINK_LIBRARIES(E"<<name<<"Operator_par visitcommon avtexpressions_par avtfilters_par avtpipeline_par "<< ToString(elibsPar) << ")" << endl;
        out << "    SET(INSTALLTARGETS ${INSTALLTARGETS} E"<<name<<"Operator_par)" << endl;
        out << "ENDIF(VISIT_PARALLEL)" << endl;
        out << endl;
        out << "VISIT_INSTALL_OPERATOR_PLUGINS(${INSTALLTARGETS})" << endl;
        out << "VISIT_PLUGIN_TARGET_PREFIX(${INSTALLTARGETS})" << endl;
        out << endl;
    }

    void WriteCMake_Database(QTextStream &out)
    {
        out << "PROJECT("<<name<<")" << endl;
        out << endl;
        out << "SET(COMMON_SOURCES" << endl;
        out << ""<<name<<"PluginInfo.C" << endl;
        out << ""<<name<<"CommonPluginInfo.C" << endl;
        out << ")" << endl;
        out << endl;
        out << "SET(LIBI_SOURCES " << endl;
        out << ""<<name<<"PluginInfo.C" << endl;
        out << ")" << endl;
        out << endl;
        if(!onlyEnginePlugin)
        {
            out << "SET(LIBM_SOURCES" << endl;
            out << ""<<name<<"MDServerPluginInfo.C" << endl;
            out << "${COMMON_SOURCES}" << endl;
            if (custommfiles)
                for (size_t i=0; i<mfiles.size(); i++)
                    out << mfiles[i] << endl;
            else
                for (size_t i=0; i<defaultmfiles.size(); i++)
                    out << defaultmfiles[i] << endl;
            out << ")" << endl;
            if (customwmfiles)
            {
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
                for (size_t i=0; i<efiles.size(); i++)
                    out << efiles[i] << endl;
            else
                for (size_t i=0; i<defaultefiles.size(); i++)
                    out << defaultefiles[i] << endl;
            out << ")" << endl;
            if (customwefiles)
            {
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
        vector<QString> extraIncludes;
        for (size_t i=0; i<cxxflags.size(); i++)
        {
            if(cxxflags[i].startsWith("${"))
                 extraIncludes.push_back(cxxflags[i]);
            else if(cxxflags[i].startsWith("$("))
                 extraIncludes.push_back(ConvertDollarParenthesis(cxxflags[i]));
        }
        out << "INCLUDE_DIRECTORIES(" << endl;
        out << "${CMAKE_CURRENT_SOURCE_DIR}" << endl;
        out << "${VISIT_COMMON_INCLUDES}" << endl;
        out << VisItIncludeDir() << "/avt/DBAtts/MetaData" << endl;
        out << VisItIncludeDir() << "/avt/DBAtts/SIL" << endl;
        out << VisItIncludeDir() << "/avt/Database/Database" << endl;
        out << VisItIncludeDir() << "/avt/Database/Formats" << endl;
        out << VisItIncludeDir() << "/avt/Database/Ghost" << endl;
        out << VisItIncludeDir() << "/avt/FileWriter" << endl;
        out << VisItIncludeDir() << "/avt/Filters" << endl;
        out << VisItIncludeDir() << "/avt/MIR/Base" << endl;
        out << VisItIncludeDir() << "/avt/MIR/Tet" << endl;
        out << VisItIncludeDir() << "/avt/MIR/Zoo" << endl;
        out << VisItIncludeDir() << "/avt/Math" << endl;
        out << VisItIncludeDir() << "/avt/Pipeline/AbstractFilters" << endl;
        out << VisItIncludeDir() << "/avt/Pipeline/Data" << endl;
        out << VisItIncludeDir() << "/avt/Pipeline/Pipeline" << endl;
        out << VisItIncludeDir() << "/avt/Pipeline/Sinks" << endl;
        out << VisItIncludeDir() << "/avt/Pipeline/Sources" << endl;
        out << VisItIncludeDir() << "/avt/VisWindow/VisWindow" << endl;
        out << VisItIncludeDir() << "/visit_vtk/full" << endl;
        out << VisItIncludeDir() << "/visit_vtk/lightweight" << endl;
        out << "${VTK_INCLUDE_DIRS} " << endl;
        if(extraIncludes.size() > 0)
            out << ToString(extraIncludes) << endl;
        out << ")" << endl;
        out << endl;
        // Pass other CXXFLAGS
        for (size_t i=0; i<cxxflags.size(); i++)
        {
            if(!cxxflags[i].startsWith("${") && !cxxflags[i].startsWith("$("))
                 out << "ADD_DEFINITIONS(\"" << cxxflags[i] << "\")" << endl;
        }
        bool needWindowsDefines = false;
        for (size_t i=0; i<libs.size() && !needWindowsDefines; i++)
        {
            if(libs[i].contains("BOXLIB2D"))
                 needWindowsDefines = true;
            else if(libs[i].contains("BOXLIB3D"))
                 needWindowsDefines = true;
            else if(libs[i].contains("HDF5"))
                 needWindowsDefines = true;
            else if(libs[i].contains("HDF4"))
                 needWindowsDefines = true;
            else if(libs[i].contains("FITS"))
                 needWindowsDefines = true;
        }
        if (needWindowsDefines)
        {
            out << "IF(WIN32)" << endl;
            for (size_t i=0; i<libs.size(); i++)
            {
                if(libs[i].contains("BOXLIB2D"))
                     out << "  ADD_DEFINITIONS(-DBL_FORT_USE_UPPERCASE)" << endl;
                else if(libs[i].contains("BOXLIB3D"))
                     out << "  ADD_DEFINITIONS(-DBL_FORT_USE_UPPERCASE)" << endl;
                else if(libs[i].contains("HDF5"))
                     out << "  ADD_DEFINITIONS(-D_HDF5USEDLL_)" << endl;
                else if(libs[i].contains("HDF4"))
                     out << "  ADD_DEFINITIONS(-DMFHDFLIB_ -D_HDFLIB_)" << endl;
                else if(libs[i].contains("FITS"))
                     out << "  ADD_DEFINITIONS(-D_HDF5USEDLL_)" << endl;
            }
            out << "ENDIF(WIN32)" << endl;
        }

        out << endl;
        // Extract extra link directories from LDFLAGS if they have ${} or $()
        vector<QString> linkDirs;
        for (size_t i=0; i<ldflags.size(); i++)
        {
            if(ldflags[i].startsWith("${") || ldflags[i].startsWith("$("))
                 linkDirs.push_back(ldflags[i]);
        }
        out << "LINK_DIRECTORIES(${VISIT_LIBRARY_DIR} ${VTK_LIBRARY_DIRS} " << ToString(linkDirs) << ")" << endl;
        out << endl;
        out << "ADD_LIBRARY(I"<<name<<"Database ${LIBI_SOURCES})" << endl;
        out << "TARGET_LINK_LIBRARIES(I"<<name<<"Database visitcommon)" << endl;
        out << "SET(INSTALLTARGETS I"<<name<<"Database)" << endl;
        out << endl;
        if(!onlyEnginePlugin)
        {
            out << "IF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)" << endl;

            out << "    ADD_LIBRARY(M"<<name<<"Database ${LIBM_SOURCES}";
            if (customwmfiles)
                out << "     ${LIBM_WIN32_SOURCES}";
            out << "    )" << endl;
            out << "    TARGET_LINK_LIBRARIES(M"<<name<<"Database visitcommon avtdbatts avtdatabase_ser "<< ToString(libs) << ToString(mlibs) << ")" << endl;
            out << "    ADD_TARGET_DEFINITIONS(M"<<name<<"Database MDSERVER)" << endl;
            out << "    SET(INSTALLTARGETS ${INSTALLTARGETS} M"<<name<<"Database)" << endl;
            out << "ENDIF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)" << endl;
            out << endl;
        }
        if(!noEnginePlugin)
        {
            out << "ADD_LIBRARY(E"<<name<<"Database_ser ${LIBE_SOURCES}";
            if (customwefiles)
                out << " ${LIBE_WIN32_SOURCES}";
            out << ")" << endl;
            out << "TARGET_LINK_LIBRARIES(E"<<name<<"Database_ser visitcommon avtdatabase_ser avtpipeline_ser "<< ToString(libs) << ToString(elibsSer) << ")" << endl;
            out << "ADD_TARGET_DEFINITIONS(E"<<name<<"Database_ser ENGINE)" << endl;
            out << "SET(INSTALLTARGETS ${INSTALLTARGETS} E"<<name<<"Database_ser)" << endl;
            out << endl;
            out << "IF(VISIT_PARALLEL)" << endl;
            out << "    ADD_PARALLEL_LIBRARY(E"<<name<<"Database_par ${LIBE_SOURCES})" << endl;
            out << "    TARGET_LINK_LIBRARIES(E"<<name<<"Database_par visitcommon avtdatabase_par avtpipeline_par "<< ToString(libs) << ToString(elibsPar) << ")" << endl;
            out << "    ADD_TARGET_DEFINITIONS(E"<<name<<"Database_par ENGINE)" << endl;
            out << "    SET(INSTALLTARGETS ${INSTALLTARGETS} E"<<name<<"Database_par)" << endl;
            out << "ENDIF(VISIT_PARALLEL)" << endl;
            out << endl;
        }
        out << "VISIT_INSTALL_DATABASE_PLUGINS(${INSTALLTARGETS})" << endl;
        out << "VISIT_PLUGIN_TARGET_PREFIX(${INSTALLTARGETS})" << endl;
    }

    void WriteCMake(QTextStream &out)
    {
        const char *visithome = getenv("VISITARCHHOME");
        if (!visithome && !using_dev)
            throw QString().sprintf("Please set the VISITARCHHOME environment variable.\n"
                                    "You may have it set automatically "
                                    "using 'visit -xml2cmake'.");
#if 0
        const char *visitplugdir = getenv("VISITPLUGININST");
        if (!visitplugdir)
            throw QString().sprintf("Please set the VISITPLUGININST environment variable.\n"
                                    "You may have it set automatically "
                                    "using 'visit -xml2cmake'.");
#endif
        const char *visitplugdirpub = getenv("VISITPLUGININSTPUB");
        if (!visitplugdirpub && installpublic)
            throw QString().sprintf("Please set the VISITPLUGININSTPUB environment variable.\n"
                                    "You may have it set automatically "
                                    "using 'visit -xml2cmake'.");
        const char *visitplugdirpri = getenv("VISITPLUGININSTPRI");
        if (!visitplugdirpri)
        {
           if ((using_dev && installprivate) || !using_dev)
            throw QString().sprintf("Please set the VISITPLUGININSTPRI environment variable.\n"
                                    "You may have it set automatically "
                                    "using 'visit -xml2cmake'.");
        }

        out << "# DO NOT EDIT THIS FILE! THIS FILE IS AUTOMATICALLY GENERATED BY xml2cmake" << endl;

        // If we're not using a development version then we need to always include 
        // something in the generated output.
        if(!using_dev)
        {
            out << "CMAKE_MINIMUM_REQUIRED(VERSION 2.6.4 FATAL_ERROR)" << endl;
            out << "SET(VISIT_INCLUDE_DIR " << visithome << "/include)" << endl;
            out << "SET(VISIT_LIBRARY_DIR " << visithome << "/lib)" << endl;
            out << "SET(VISIT_BINARY_DIR " << visithome << "/bin)" << endl;
            out << "SET(VISIT_ARCHIVE_DIR " << visithome << "/archives)" << endl;
            if(installpublic)
            {
                out << "SET(VISIT_PLUGIN_DIR " << visitplugdirpub << ")" << endl;
            }
            else // installprivate or default
            {
                out << "SET(VISIT_PLUGIN_DIR " << visitplugdirpri << ")" << endl;
            }

            out << "INCLUDE(" << visithome << "/include/PluginVsInstall.cmake)" << endl;
            out << endl;
        }
        else
        {
            // We're using a development version but we're installing public or private.
            if(installpublic)
            {
                out << "SET(VISIT_PLUGIN_DIR " << visitplugdirpub << ")" << endl;
            }

            if(installprivate)
            {
                out << "SET(VISIT_PLUGIN_DIR " << visitplugdirpri << ")" << endl;
            }
        }

        if(type == "plot")
            WriteCMake_Plot(out);
        else if(type == "operator")
            WriteCMake_Operator(out);
        else if(type == "database")
            WriteCMake_Database(out);
    }
};


// ----------------------------------------------------------------------------
//                           Override default types
// ----------------------------------------------------------------------------
#define Plugin       CMakeGeneratorPlugin

#endif
