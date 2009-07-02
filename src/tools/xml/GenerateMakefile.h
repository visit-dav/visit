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

#ifndef GENERATE_MAKEFILE_H
#define GENERATE_MAKEFILE_H

#include <QTextStream>
#include "Field.h"
#include <visit-config.h> // for the plugin extension.
#include "Plugin.h"

// ****************************************************************************
//  File:  GenerateMakefile
//
//  Purpose:
//    Contains a set of classes which override the default implementation
//    to create makefile for the plugin.
//
//  Note: This file overrides --
//    Plugin
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 28 14:17:33 PDT 2001
//    Added generation of IDSO.
//
//    Jeremy Meredith, Sat Jan  5 17:34:28 PST 2002
//    Removed avtXXXXFilter.C from the viewer operator plugins.
//    Removed @VTK_LIBS@ from GLIBS.
//    Added @PARALLEL_CPPFLAGS@ to the CPPFLAGS.
//
//    Jeremy Meredith, Wed Jan 16 12:24:37 PST 2002
//    Updated so plugins can build serial and parallel versions concurrently.
//
//    Jeremy Meredith, Wed Feb  6 17:26:31 PST 2002
//    Changed references to plugin->name+"Attributes" to instead refer
//    directly to atts->name.
//
//    Jeremy Meredith, Tue May 14 23:14:04 PDT 2002
//    Removed PARALLEL_CPPFLAGS -- it was being used in serial as well as
//    parallel since both are built concurrently.  Those flags are
//    automatically set now from make-targets.in.
//
//    Jeremy Meredith, Mon Jul 15 01:22:09 PDT 2002
//    Big enhancements so all our plugin makefile can be build automatically.
//
//    Brad Whitlock, Wed Aug 14 15:27:36 PST 2002
//    I added Java targets for the Java plugins.
//
//    Jeremy Meredith, Tue Aug 27 14:32:50 PDT 2002
//    Added support for database plugins.
//    Added mfiles, dbtype, and libs.  Allowed NULL atts.
//
//    Jeremy Meredith, Sat Sep 21 09:06:39 PDT 2002
//    Fixed a bug with building plugins using an installed visit.
//
//    Jeremy Meredith, Thu Oct 17 15:58:29 PDT 2002
//    Added some enhancements for the XML editor.
//
//    Sean Ahern, Fri Nov 15 15:25:23 PST 2002
//    Added "widget files" so we can have custom GUI elements.
//
//    Jeremy Meredith, Thu Feb 27 14:05:40 PST 2003
//    Moved the avtXXXFileFormat.C (or avtXXXDatabase.C) to a MDServer and
//    Engine-specific file.  This allows parallel enhancements in these files.
//
//    Brad Whitlock, Tue Mar 11 07:27:37 PDT 2003
//    I fixed a bug that made the program generate bad Makefiles for operator
//    plugins with widget files.
//
//    Jeremy Meredith, Tue Apr 15 21:10:10 PDT 2003
//    I added a setting for TOPDIR.  This is needed to help make plugins build
//    completely correctly from an installed distribution.
//
//    Jeremy Meredith, Mon May  5 14:39:41 PDT 2003
//    Fixed MOC dependencies.
//
//    Hank Childs, Tue Sep  9 10:04:41 PDT 2003
//    Added support for file writers.
//
//    Brad Whitlock, Mon May 19 14:33:45 PST 2003
//    I made it use the shared library extension from visit-config.h so it
//    writes the plugins to the write filenames on MacOS X. I also changed
//    the list of sources somewhat so plugins load without problems on the Mac.
//
//    Jeremy Meredith, Tue Sep 23 16:57:01 PDT 2003
//    Changed haswriter to a bool.
//
//    Jeremy Meredith, Wed Nov  5 13:28:03 PST 2003
//    Added ability to disable plugins by default.
//    Added avt files for databases.
//
//    Hank Childs, Fri Jan 23 08:54:53 PST 2004
//    Do not link AVT code into Viewer Operators.
//
//    Jeremy Meredith, Tue Mar 30 10:13:46 PST 2004
//    I added support for database plugins with only a mdserver or engine
//    component.  This was critical for simulation support.
//
//    Brad Whitlock, Mon Apr 26 14:08:41 PST 2004
//    I added a couple of symbols for MacOS X.
//
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Jeremy Meredith, Wed Aug 25 11:50:14 PDT 2004
//    Added the concept of an engine-only or everything-but-the-engine plugin.
//    This used to be an argument to main, but now it is part of the XML file.
//
//    Brad Whitlock, Tue Nov 2 14:29:25 PST 2004
//    Added -lparallel_visit_vtk_{ser,par} to the link line for plot and operator
//    plugins so they link again on MacOS X.
//
//    Brad Whitlock, Wed Jan 5 11:15:16 PDT 2005
//    Added -lexpr everywhere -lparser is used so we can link again on MacOS X.
//
//    Jeremy Meredith, Mon Feb  7 13:31:03 PST 2005
//    Added VisIt version to the general plugin info.
//
//    Mark C. Miller, Mon Feb 14 20:27:12 PST 2005
//    Added output of SRC make macro for .depend to work in plugin dirs
//
//    Jeremy Meredith, Tue Feb 22 18:43:25 PST 2005
//    Removed the database writer from the mdserver plugin.
//
//    Brad Whitlock, Thu Mar 3 09:00:24 PDT 2005
//    I removed WriteVersionFile since the version is now included in the
//    common plugin info. I also added the Slice operator to the OnionPeel
//    hack code for MacOS X.
//
//    Brad Whitlock, Wed Mar 30 17:56:47 PST 2005
//    I removed Qt and Python from the CXXFLAGS for database plugins.
//
//    Brad Whitlock, Mon Apr 11 14:10:26 PST 2005
//    Certain variables were set up to include -L../../plugins/databases,
//    which does not work when you build outside the vob. Changed certain
//    libraries that are now ser/par in the LIBS.
//
//    Hank Childs, Tue May 24 09:41:53 PDT 2005
//    Added hasoptions.
//
//    Brad Whitlock, Fri Nov 4 09:42:25 PDT 2005
//    Added code to treat Mesa and OpenGL source files specially for plot
//    plugins.
//
//    Brad Whitlock, Mon Jan 30 11:49:15 PDT 2006
//    Added QUI_LIBS to the link line for GUI plugins.
//
//    Thomas R. Treadway, Tue Feb  6 17:04:03 PST 2007
//    Re ordered $(PY_CXXFLAGS)
//    Added $(GLEW_LIBS) for Mac dependences
//
//    Brad Whitlock, Fri Feb 23 17:07:43 PST 2007
//    Added support for viewer widgets.
// 
//    Cyrus Harrison, Wed Mar  7 08:55:37 PST 2007
//    Allow for engine-specific code in a plugin's source files.
//
//    Brad Whitlock, Tue Mar 13 18:19:46 PST 2007
//    Added -lvisitpy for scripting plugins.
//
//    Thomas R. Treadway, Mon Mar 26 16:50:51 PDT 2007
//    Added more Mac dependences
//
//    Thomas R. Treadway, Tue Apr  3 11:16:32 PDT 2007
//    Need to include Mesa and OpenGL for the Mac dependences
//    Wrapped Mac dependences in #ifdef
//
//    Jeremy Meredith, Thu Jun  7 13:22:18 EDT 2007
//    Remove build rule for moc.C from .h.  This is now in make-targets.in.
//
//    Thomas R. Treadway, Thu Jul 26 15:28:07 PDT 2007
//    Added more Mac dependences for plugins
//
//    Jeremy Meredith, Tue Sep  4 10:23:25 EDT 2007
//    .c extensions didn't work.  Added a bit of a fix, though we should
//    still probably change it so the user specifies the object files,
//    not the source files.
//
//    Gunther H. Weber, Wed Sep 19 16:37:20 PDT 2007
//    Added dependecies for parallel builds on Macs.
//
//    Brad Whitlock, Tue Oct 9 12:27:34 PST 2007
//    Removed linking of libI into libE,libM,libG,libS,libV on the Mac, which
//    was a hold out from when we had to use -flat_namespace. Now, it was just
//    breaking parallel builds.
//
//    Jeremy Meredith, Thu Nov 29 15:03:11 EST 2007
//    Allow the Writer plugin file to be renamed using the custom engine
//    files mechanism.
//
//    Brad Whitlock, Thu Dec 13 16:14:12 PST 2007
//    Fixed plugin install name on MacOS X.
//
//    Hank Childs, Thu Jan 10 14:33:30 PST 2008
//    Added filenames, specifiedFilenames.
//
//    Brad Whitlock, Thu Feb 28 16:54:23 PST 2008
//    Made it use a base class.
//
//    Mark C. Miller, Mon Aug 18 22:00:05 PDT 2008
//    Replaced BZIP2_LIBS with ZLIB_LIB
//
//    Brad Whitlock, Fri Oct  3 15:05:40 PDT 2008
//    Introduced LIBG_QT_LIBS, LIBV_QT_LIBS, which constitute the set of
//    Qt libraries that a plugin must link against rather than all Qt libs.
//
//    Cyrus Harrison, Fri Sep 19 14:20:32 PDT 2008
//    Added custom libs for gui,engine,mdserver, and viewer libs.
//
//    Brad Whitlock, Wed Oct 15 14:27:59 PDT 2008
//    Added support for additional Java source.
//
//    Brad Whitlock, Wed Jun 10 11:44:51 PDT 2009
//    I changed the code so Mesa sources are not added unless configure
//    detected that VTK uses mangled mesa.
//
//    Tom Fogal, Tue Jun 30 22:35:24 MDT 2009
//    I forced GENERAL_PLUGIN_EXPORTS into CXXFLAGS.
//
//    Tom Fogal, Wed Jul  1 13:40:21 MDT 2009
//    Added some export defines.
//
// ****************************************************************************

class MakefileGeneratorPlugin : public Plugin
{
  public:
    MakefileGeneratorPlugin(const QString &n,const QString &l,const QString &t,
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
            {
                defaultefiles.push_back(QString("avt") + name + "Writer.C");
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

    virtual ~MakefileGeneratorPlugin()
    {
    }

    void WriteMakefile(QTextStream &out)
    {
        const char *visithome = getenv("VISITARCHHOME");
        if (!visithome)
            throw QString().sprintf("Please set the VISITARCHHOME environment variable.\n"
                                    "You may have it set automatically "
                                    "using 'visit -xml2makefile'.");
        const char *visitplugdir = getenv("VISITPLUGININST");
        if (!visitplugdir)
            throw QString().sprintf("Please set the VISITPLUGININST environment variable.\n"
                                    "You may have it set automatically "
                                    "using 'visit -xml2makefile'.");
        const char *visitplugdirpub = getenv("VISITPLUGININSTPUB");
        if (!visitplugdirpub)
            throw QString().sprintf("Please set the VISITPLUGININSTPUB environment variable.\n"
                                    "You may have it set automatically "
                                    "using 'visit -xml2makefile'.");
        const char *visitplugdirpri = getenv("VISITPLUGININSTPRI");
        if (!visitplugdirpri)
            throw QString().sprintf("Please set the VISITPLUGININSTPRI environment variable.\n"
                                    "You may have it set automatically "
                                    "using 'visit -xml2makefile'.");

        const char *visitplugininstall = visitplugdir;
        if (installpublic)
            visitplugininstall = visitplugdirpub;
        if (installprivate)
            visitplugininstall = visitplugdirpri;

        QString vtkdir = QString(visithome) + "/include/vtk";
        QString vGraphicsObjects(""), eserGraphicsObjects(""), eparGraphicsObjects("");

        out << "##" << endl;
        out << "## Programs and options..." << endl;
        out << "##" << endl;
        out << "include "<<visithome<<"/include/make-variables" << endl;
        out << "TOPDIR=" << visithome << endl;
        out << "" << endl;
        out << endl;
        out << "##" << endl;
        out << "## Libraries and includes..." << endl;
        out << "##" << endl;
        out << "VTK_INCLUDE= \\" << endl;
        out << "  -I"<<vtkdir<<" \\"<<endl;
        out << "  -I"<<vtkdir<<"/Common \\"<<endl;
        out << "  -I"<<vtkdir<<"/Filtering \\"<<endl;
        out << "  -I"<<vtkdir<<"/Graphics \\"<<endl;
        out << "  -I"<<vtkdir<<"/Hybrid \\"<<endl;
        out << "  -I"<<vtkdir<<"/IO \\"<<endl;
        out << "  -I"<<vtkdir<<"/Imaging \\"<<endl;
        out << "  -I"<<vtkdir<<"/Rendering \\"<<endl;
        out << "  -I"<<vtkdir<<"/Utilities"<<endl;
        if(type == "database")
        {
            out << "CXXFLAGS=$(CXXFLAGSORIG)";
        }
        else
            out << "CXXFLAGS=$(CXXFLAGSORIG) $(QT_CXXFLAGS)";
        out << " -DGENERAL_PLUGIN_EXPORTS";
        // This creates too many exports; we want to use the define
        // conditionally based on which lib? we're compiling.  That is, for
        // libM's we want to define MDSERVER_PLUGIN_EXPORTS, but not
        // ENGINE_PLUGIN_EXPORTS.  The opposite is true for libE's.
        // However we don't compile those separately right now, so we'll
        // just define both regardless.  This adds more symbols than it
        // needs to.
        out << " -DMDSERVER_PLUGIN_EXPORTS";
        out << " -DENGINE_PLUGIN_EXPORTS";
        if(type == "plot" || type == "operator")
        {
            out << " -DGUI_PLUGIN_EXPORTS -DVIEWER_PLUGIN_EXPORTS";
            out << " -DSCRIPTING_PLUGIN_EXPORTS";
        }
        for (size_t i=0; i<cxxflags.size(); i++)
            out << " " << cxxflags[i];
        out << endl;
#ifndef __APPLE__
        out << "CPPFLAGS=$(CPPFLAGSORIG) $(VTK_INCLUDE) -I. -I"<<visithome<<"/include -I"<<visithome<<"/include/visit $(PY_CXXFLAGS)" << endl;
#else
        out << "CPPFLAGS=$(CPPFLAGSORIG) $(VTK_INCLUDE) -I. -I"<<visithome<<"/include -I"<<visithome<<"/include/mesa -I"<<visithome<<"/include/visit $(PY_CXXFLAGS)" << endl;
#endif
        if(type == "database")
            out << "LDFLAGS=$(LDFLAGSORIG) $(PLUGIN_LDFLAGS)";
        else
            out << "LDFLAGS=$(LDFLAGSORIG) $(PLUGIN_LDFLAGS) $(PY_LDFLAGS) ";
        for (size_t i=0; i<ldflags.size(); i++)
            out << " " << ldflags[i];
        out << " -L" << visitplugininstall << "/" << type << "s";
        out << endl << endl;
        out << "LIBG_QT_LIBS=$(QT_CORE_LIB) $(QT_GUI_LIB)" << endl;
        out << "LIBV_QT_LIBS=$(QT_CORE_LIB) $(QT_GUI_LIB) $(QT_OPENGL_LIB)" << endl;
        out << "" << endl;
        out << "##" << endl;
        out << "## Files..." << endl;
        out << "##" << endl;
        if (type=="operator")
        {
            out << "PLUGINDIR=operators" << endl;
            out << "PLUGINNAME="<<name << "Operator" << endl;
            out << endl;
            out << "WIDGETS=Qvis"<<name<<"Window.h";
            if (customwfiles)
                for (size_t i=0; i<wfiles.size(); i++)
                    out << " " << wfiles[i];
            out << endl;
            out << "VIEWERWIDGETS=";
            if (customvwfiles)
            {
                for (size_t i=0; i<vwfiles.size(); i++)
                {
                    out << vwfiles[i];
                    if(i < vwfiles.size() - 1)
                        out << " ";
                }
            }
            out << endl;
            out << "ISRC="<<name<<"PluginInfo.C" << endl;
            out << "COMMONSRC=";
            out << name<<"PluginInfo.C ";
            out <<name<<"CommonPluginInfo.C "<<atts->name<<".C" << endl;
            out << "GSRC="<<name<<"GUIPluginInfo.C Qvis"<<name<<"Window.C";
            if (customgfiles)
                for (size_t i=0; i<gfiles.size(); i++)
                    out << " " << gfiles[i];
            else
                for (size_t i=0; i<defaultgfiles.size(); i++)
                    out << " " << defaultgfiles[i];
            out << endl;
            out << "VSRC="<<name<<"ViewerPluginInfo.C";
            if (customvfiles)
                for (size_t i=0; i<vfiles.size(); i++)
                    out << " " << vfiles[i];
            else
                for (size_t i=0; i<defaultvfiles.size(); i++)
                    out << " " << defaultvfiles[i];
            out << endl;
            out << "ESRC="<<name<<"EnginePluginInfo.C";
            if(hasEngineSpecificCode)
            {
                out << endl;
                out << "ESPECIFICSRC=";
            }
            if (customefiles)
                for (size_t i=0; i<efiles.size(); i++)
                    out << " " << efiles[i];
            else
                for (size_t i=0; i<defaultefiles.size(); i++)
                    out << " " << defaultefiles[i];
            out << endl;
            out << "SSRC="<<name<<"ScriptingPluginInfo.C Py"<<atts->name<<".C";
            if (customsfiles)
                for (size_t i=0; i<sfiles.size(); i++)
                    out << " " << sfiles[i];
            else
                for (size_t i=0; i<defaultsfiles.size(); i++)
                    out << " " << defaultsfiles[i];

            out << "" << endl;
            out << "" << endl;
            out << "SRC=$(ISRC) $(COMMONSRC) $(GSRC) $(VSRC) $(ESRC) $(SSRC)" << endl;

            out << "" << endl;
            out << "" << endl;
            out << "JAVASRC=";
            if(customjfiles)
            {
                for(size_t i = 0; i < jfiles.size(); ++i)
                    out << jfiles[i] << " ";
            }
            out << atts->name<<".java" << endl;
            out << "JAVAOBJ="<<visithome<<"/java/llnl/visit/operators/"<<atts->name<<".class" << endl;
            out << "JAVAPLUGINFLAGS=-d "<<visithome<<"/java -classpath " << visithome << "/java" << endl;  

            out << "" << endl;
            out << "SHLIB_FORCED=";
            for (size_t i=0; i<libs.size(); i++)
                out << libs[i] << " ";
            out << endl;
            out << "" << endl;
            out << "GUI_LIBS_FORCED=";
            for (size_t i=0; i<glibs.size(); i++)
                out << glibs[i] << " ";
            out << endl;
            out << "ENGINE_LIBS_FORCED=";
            for (size_t i=0; i<elibs.size(); i++)
                out << elibs[i] << " ";
            out << endl;
            out << "MDSERVER_LIBS_FORCED=";
            for (size_t i=0; i<mlibs.size(); i++)
                out << mlibs[i] << " ";
            out << endl;
            out << "VIEWER_LIBS_FORCED=";
            for (size_t i=0; i<vlibs.size(); i++)
                out << vlibs[i] << " ";
            out << endl;
#ifndef __APPLE__
            out << "ELIBS_FOR_MACOSX_PREBINDING=" << endl;
            out << "VLIBS_FOR_MACOSX_PREBINDING=" << endl;
            out << "ESERLIBS_FOR_MACOSX_PREBINDING=" << endl;
            out << "EPARLIBS_FOR_MACOSX_PREBINDING=" << endl;
            out << "SLIBS_FOR_MACOSX_PREBINDING=" << endl;
#else
            out << "ELIBS_FOR_MACOSX_PREBINDING=$(ZLIB_LIB) $(GLEW_LIBS) "
                   "$(MESA_LIBS) $(GL_LIBS)" << endl;
            out << "VLIBS_FOR_MACOSX_PREBINDING=-lviewer -lengineproxy "
                   "-lenginerpc -lviewerrpc -lmdserverproxy -lmdserverrpc "
                   "-lvclproxy -lvclrpc -lplugin -lqtviswindow "
                   "-lviswindow_ser -lavtwriter_ser -lvtkqt -lwinutil "
                   "-lproxybase $(QT_LDFLAGS) $(LIBV_QT_LIBS) $(ZLIB_LIB) "
                   "$(GLEW_LIBS) $(MESA_LIBS) $(GL_LIBS)" << endl;
            out << "ESERLIBS_FOR_MACOSX_PREBINDING=-lavtddf_ser "
                   "-ldatabase_ser -lmir_ser -lplugin -lvisit_verdict" << endl;
            out << "EPARLIBS_FOR_MACOSX_PREBINDING=-lavtddf_par "
                   "-ldatabase_par -lmir_par -lplugin -lvisit_verdict" << endl;
            out << "SLIBS_FOR_MACOSX_PREBINDING=-lavtexceptions "
                   "-ldbatts -lplugin -lexpr -lparser" << endl;
#endif
            out << "ILIBS=" << endl;
            out << "GLIBS=-lgui -lmdserverproxy -lviewerproxy -lproxybase "
                   "-lmdserverrpc -lviewerrpc -ldbatts -lwinutil "
                   "-lavtexceptions -lstate -lcomm -lmisc -lplugin "
                   "-lexpr -lparser -lutility "
                   "$(QT_LDFLAGS) $(LIBG_QT_LIBS) $(X_LIBS)" << endl;
            out << "SLIBS=-lstate -lmisc -lutility -lcomm -lvisitpy "
                   "$(SLIBS_FOR_MACOSX_PREBINDING) $(PY_LIB)" << endl;
            out << "VLIBS=-lpipeline_ser -lplotter_ser -lavtfilters_ser "
                   "-lavtmath_ser -lavtview -lavtexceptions -ldbatts -lstate "
                   "-lexpr -lmisc -lcomm -lparser -lutility -lvisit_vtk "
                   "-lrendering_visit_vtk -llightweight_visit_vtk -lparallel_visit_vtk_ser "
                   "$(VLIBS_FOR_MACOSX_PREBINDING) ";
            //
            // HACK HACK HACK -- This should be a flag in the XML file instead
            //                   of using the name of the operator. The flag
            //                   should be called something like: UsesViewerLib.
            //
            //                   When the operator uses classes out of the viewer
            //                   library, we have to link all of the viewer
            //                   dependencies and most operators don't do need to.
            //
            bool usesViewerLib = (name == "OnionPeel" || name == "Slice");
            if (usesViewerLib)
            {
               out << "-lviewer -lviewerrpc -lproxybase -lvclproxy -lvclrpc "
                      "-lmdserverproxy -lmdserverrpc -lengineproxy "
                      "-lenginerpc -lplugin -lavtwriter_ser -lviswindow_ser "
                      "-lqtviswindow -lvtkqt -lwinutil $(LIBV_QT_LIBS) ";
            }

            // Add the rest of the viewer operator libs
            out << "$(VTK_LIBS)" << endl;
            out << "ESERLIBS=-lpipeline_ser -lplotter_ser -lavtfilters_ser "
                   "-lavtmath_ser -lavtview -ldbatts -lavtexceptions "
                   "-lstate -lmisc -lcomm -lexpr -lparser -lutility "
                   "-lvisit_vtk -lrendering_visit_vtk -llightweight_visit_vtk "
                   "-lparallel_visit_vtk_ser -lexpressions_ser "
                   "$(ESERLIBS_FOR_MACOSX_PREBINDING) "
                   "$(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS)" << endl;
            out << "EPARLIBS=-lpipeline_par -lplotter_par -lavtfilters_par "
                   "-lavtmath_par -lavtview -ldbatts -lavtexceptions "
                   "-lstate -lmisc -lcomm -lexpr -lparser -lutility "
                   "-lvisit_vtk -lrendering_visit_vtk -llightweight_visit_vtk "
                   "-lparallel_visit_vtk_par -lexpressions_par "
                   "$(EPARLIBS_FOR_MACOSX_PREBINDING) "
		   "$(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS) $(SHLIB_MPI_LIBS)" << endl;
            out << "" << endl;
            out << "IDSO="<<visitplugininstall<<"/operators/libI"<<name<<"Operator" << VISIT_PLUGIN_EXTENSION << endl;
            out << "GDSO="<<visitplugininstall<<"/operators/libG"<<name<<"Operator" << VISIT_PLUGIN_EXTENSION << endl;
            out << "SDSO="<<visitplugininstall<<"/operators/libS"<<name<<"Operator" << VISIT_PLUGIN_EXTENSION << endl;
            out << "VDSO="<<visitplugininstall<<"/operators/libV"<<name<<"Operator" << VISIT_PLUGIN_EXTENSION << endl;
            out << "ESERDSO="<<visitplugininstall<<"/operators/libE"<<name<<"Operator_ser" << VISIT_PLUGIN_EXTENSION << endl;
            out << "EPARDSO="<<visitplugininstall<<"/operators/libE"<<name<<"Operator_par" << VISIT_PLUGIN_EXTENSION << endl;
        }
        else if (type=="plot")
        {
            out << "PLUGINDIR=plots" << endl;
            out << "PLUGINNAME="<<name << "Plot" << endl;
            out << endl;
            out << "WIDGETS=Qvis"<<name<<"PlotWindow.h";
            if (customwfiles)
                for (size_t i=0; i<wfiles.size(); i++)
                    out << " " << wfiles[i];
            out << endl;
            out << "VIEWERWIDGETS=";
            if (customvwfiles)
            {
                for (size_t i=0; i<vwfiles.size(); i++)
                {
                    out << vwfiles[i];
                    if(i < vwfiles.size() - 1)
                        out << " ";
                }
            }
            out << endl;
            out << "ISRC="<<name<<"PluginInfo.C" << endl;
            out << "COMMONSRC=";
            out << name<<"PluginInfo.C ";
            out << name<<"CommonPluginInfo.C "<<atts->name<<".C" << endl;
            out << "GSRC="<<name<<"GUIPluginInfo.C Qvis"<<name<<"PlotWindow.C";
            if (customgfiles)
                for (size_t i=0; i<gfiles.size(); i++)
                    out << " " << gfiles[i];
            else
                for (size_t i=0; i<defaultgfiles.size(); i++)
                    out << " " << defaultgfiles[i];
            out << endl;
            out << "SSRC="<<name<<"ScriptingPluginInfo.C Py"<<atts->name<<".C";
            if (customsfiles)
                for (size_t i=0; i<sfiles.size(); i++)
                    out << " " << sfiles[i];
            else
                for (size_t i=0; i<defaultsfiles.size(); i++)
                    out << " " << defaultsfiles[i];
            out << endl;
            out << "VSRC="<<name<<"ViewerPluginInfo.C avt"<<name<<"Plot.C";
            if (customvfiles)
            {
                for (size_t i=0; i<vfiles.size(); i++)
                {
                    int suffix = vfiles[i].lastIndexOf(".");
                    if(suffix > 0 && vfiles[i].indexOf("Mesa") != -1)
                    {
#ifdef VISIT_USE_MANGLED_MESA
                        if(vGraphicsObjects.length() == 0) vGraphicsObjects += " ";
                        vGraphicsObjects += (vfiles[i].left(suffix) + "_mesa.o ");
#endif
                    }
                    else if(suffix > 0 && vfiles[i].indexOf("OpenGL") != -1)
                    {
                        if(vGraphicsObjects.length() == 0) vGraphicsObjects += " ";
                        vGraphicsObjects += (vfiles[i].left(suffix) + "_ogl.o ");
                    }
                    else
                        out << " " << vfiles[i];
                }
            }
            else
                for (size_t i=0; i<defaultvfiles.size(); i++)
                    out << " " << defaultvfiles[i];
            out << endl;
            out << "ESRC="<<name<<"EnginePluginInfo.C";
            if(hasEngineSpecificCode)
            {
                out << endl;
                out << "ESPECIFICSRC=";
            }
            out << " avt"<<name<<"Plot.C";

            if (customefiles)
            {
                for (size_t i=0; i<efiles.size(); i++)
                {
                    int suffix = efiles[i].lastIndexOf(".");
                    if(suffix > 0 && efiles[i].indexOf("Mesa") != -1)
                    {
#ifdef VISIT_USE_MANGLED_MESA
                        QString root(efiles[i].left(suffix));

                        if(eserGraphicsObjects.length() == 0) eserGraphicsObjects += " ";
                        if(eparGraphicsObjects.length() == 0) eparGraphicsObjects += " ";
                        eserGraphicsObjects += (root + "_mesa.o ");
                        eparGraphicsObjects += (root + "_par_mesa.o ");
#endif
                    }
                    else if(suffix > 0 && efiles[i].indexOf("OpenGL") != -1)
                    {
                        QString root(efiles[i].left(suffix));

                        if(eserGraphicsObjects.length() == 0) eserGraphicsObjects += " ";
                        if(eparGraphicsObjects.length() == 0) eparGraphicsObjects += " ";
                        eserGraphicsObjects += (root + "_ogl.o ");
                        eparGraphicsObjects += (root + "_par_ogl.o ");
                    }
                    else
                        out << " " << efiles[i];
                }
            }
            else
                for (size_t i=0; i<defaultefiles.size(); i++)
                    out << " " << defaultefiles[i];
            out << endl;

            out << "" << endl;
            out << "" << endl;
            out << "SRC=$(ISRC) $(COMMONSRC) $(GSRC) $(SSRC) $(VSRC) $(ESRC)" << endl;

            out << "" << endl;
            out << "" << endl;
            out << "JAVASRC=";
            if(customjfiles)
            {
                for(size_t i = 0; i < jfiles.size(); ++i)
                    out << jfiles[i] << " ";
            }
            out << atts->name<<".java" << endl;
            out << "JAVAOBJ="<<visithome<<"/java/llnl/visit/plots/"<<atts->name<<".class" << endl;
            out << "JAVAPLUGINFLAGS=-d "<<visithome<<"/java -classpath " << visithome << "/java" << endl;  

            out << "" << endl;
            out << "SHLIB_FORCED=";
            for (size_t i=0; i<libs.size(); i++)
                out << libs[i] << " ";
            out << endl;
            out << "" << endl;
            out << "GUI_LIBS_FORCED=";
            for (size_t i=0; i<glibs.size(); i++)
                out << glibs[i] << " ";
            out << endl;
            out << "ENGINE_LIBS_FORCED=";
            for (size_t i=0; i<elibs.size(); i++)
                out << elibs[i] << " ";
            out << endl;
            out << "MDSERVER_LIBS_FORCED=";
            for (size_t i=0; i<mlibs.size(); i++)
                out << mlibs[i] << " ";
            out << endl;
            out << "VIEWER_LIBS_FORCED=";
            for (size_t i=0; i<vlibs.size(); i++)
                out << vlibs[i] << " ";
            out << endl;
#ifndef __APPLE__
            out << "ELIBS_FOR_MACOSX_PREBINDING=" << endl;
            out << "VLIBS_FOR_MACOSX_PREBINDING=" << endl;
            out << "ESERLIBS_FOR_MACOSX_PREBINDING=" << endl;
            out << "EPARLIBS_FOR_MACOSX_PREBINDING=" << endl;
            out << "SLIBS_FOR_MACOSX_PREBINDING=" << endl;
#else
            out << "ELIBS_FOR_MACOSX_PREBINDING=$(GLEW_LIBS) $(ZLIB_LIB) $(MESA_LIBS) $(GL_LIBS)" << endl;
            out << "VLIBS_FOR_MACOSX_PREBINDING=-lviewer -lengineproxy "
                   "-lenginerpc -lviewerrpc -lmdserverproxy -lmdserverrpc "
                   "-lvclproxy -lvclrpc -lplugin -lqtviswindow "
                   "-lviswindow_ser -lavtwriter_ser -lvtkqt -lwinutil "
                   "-lproxybase $(QT_LDFLAGS) $(LIBV_QT_LIBS) $(ZLIB_LIB) "
                   "$(GLEW_LIBS) $(MESA_LIBS) $(GL_LIBS)" << endl;
            out << "ESERLIBS_FOR_MACOSX_PREBINDING=-lavtddf_ser -ldatabase_ser -lmir_ser -lplugin -lvisit_verdict" << endl;
            out << "EPARLIBS_FOR_MACOSX_PREBINDING=-lavtddf_par -ldatabase_par -lmir_par -lplugin -lvisit_verdict" << endl;
            out << "SLIBS_FOR_MACOSX_PREBINDING=-lavtexceptions -lplugin -ldbatts -lparser -lexpr" << endl;
#endif
            out << "ILIBS=" << endl;
            out << "GLIBS=-lgui -lmdserverproxy -lviewerproxy -lproxybase -lmdserverrpc -lviewerrpc -ldbatts -lwinutil -lavtexceptions -lstate -lcomm -lmisc -lplugin -lexpr -lparser -lutility $(QT_LDFLAGS) $(LIBG_QT_LIBS) $(X_LIBS)" << endl;
            out << "SLIBS=-lstate -lmisc -lcomm -lutility -lvisitpy $(SLIBS_FOR_MACOSX_PREBINDING) $(PY_LIB)" << endl;
            out << "VLIBS=-lpipeline_ser -lplotter_ser -lavtfilters_ser "
                   "-lavtmath_ser -lavtview -ldbatts -lavtexceptions "
                   "-lmisc -lcomm -lstate -lexpr -lparser -lutility "
                   "-lvisit_vtk -lrendering_visit_vtk -llightweight_visit_vtk "
                   "-lparallel_visit_vtk_ser "
                   "$(VLIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS)" << endl;
            out << "ESERLIBS=-lpipeline_ser -lplotter_ser -lavtfilters_ser -lavtmath_ser -lavtview -lavtexceptions -ldbatts -lstate -lmisc -lcomm -lexpr -lparser -lutility -lvisit_vtk -lrendering_visit_vtk -llightweight_visit_vtk -lparallel_visit_vtk_ser -lexpressions_ser $(ESERLIBS_FOR_MACOSX_PREBINDING) $(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS)" << endl;
            out << "EPARLIBS=-lpipeline_par -lplotter_par -lavtfilters_par -lavtmath_par -lavtview -lavtexceptions -ldbatts -lstate -lmisc -lcomm -lexpr -lparser -lutility -lvisit_vtk -lrendering_visit_vtk -llightweight_visit_vtk -lparallel_visit_vtk_par -lexpressions_par $(EPARLIBS_FOR_MACOSX_PREBINDING) $(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS) $(SHLIB_MPI_LIBS)" << endl;
            out << "" << endl;
            out << "IDSO="<<visitplugininstall<<"/plots/libI"<<name<<"Plot" << VISIT_PLUGIN_EXTENSION << endl;
            out << "GDSO="<<visitplugininstall<<"/plots/libG"<<name<<"Plot" << VISIT_PLUGIN_EXTENSION << endl;
            out << "SDSO="<<visitplugininstall<<"/plots/libS"<<name<<"Plot" << VISIT_PLUGIN_EXTENSION << endl;
            out << "VDSO="<<visitplugininstall<<"/plots/libV"<<name<<"Plot" << VISIT_PLUGIN_EXTENSION << endl;
            out << "ESERDSO="<<visitplugininstall<<"/plots/libE"<<name<<"Plot_ser" << VISIT_PLUGIN_EXTENSION << endl;
            out << "EPARDSO="<<visitplugininstall<<"/plots/libE"<<name<<"Plot_par" << VISIT_PLUGIN_EXTENSION << endl;
        }
        else if (type=="database")
        {
            out << "PLUGINDIR=databases" << endl;
            out << "PLUGINNAME="<<name << "Database" << endl;
            out << endl;

            out << "WIDGETS=" << endl;
            out << "VIEWERWIDGETS=" << endl;
            out << "ISRC="<<name<<"PluginInfo.C" << endl;
            out << "COMMONSRC=";
            out << name<<"PluginInfo.C ";
            out << name<<"CommonPluginInfo.C";
            if (hasoptions)
            {
                out << " avt" << name << "Options.C";
            }
            out << endl;
            out << "MSRC="<<name<<"MDServerPluginInfo.C";
            if (has_MDS_specific_code)
            {
                out << endl;
                out << "MSPECIFICSRC=";
            }
            if (custommfiles)
                for (size_t i=0; i<mfiles.size(); i++)
                    out << " " << mfiles[i];
            else
                for (size_t i=0; i<defaultmfiles.size(); i++)
                    out << " " << defaultmfiles[i];
            out << endl;
            out << "ESRC="<<name<<"EnginePluginInfo.C";
            if(hasEngineSpecificCode)
            {
                out << endl;
                out << "ESPECIFICSRC=";
            }

            if (customefiles)
                for (size_t i=0; i<efiles.size(); i++)
                    out << " " << efiles[i];
            else
                for (size_t i=0; i<defaultefiles.size(); i++)
                    out << " " << defaultefiles[i];
            out << endl;

            out << "" << endl;
            out << "" << endl;
            out << "SRC=$(ISRC) $(COMMONSRC) $(MSRC) $(ESRC)" << endl;

            out << "" << endl;
            out << "SHLIB_FORCED=";
            for (size_t i=0; i<libs.size(); i++)
                out << libs[i] << " ";
            out << endl;
            out << "" << endl;
            out << "GUI_LIBS_FORCED=";
            for (size_t i=0; i<glibs.size(); i++)
                out << glibs[i] << " ";
            out << endl;
            out << "ENGINE_LIBS_FORCED=";
            for (size_t i=0; i<elibs.size(); i++)
                out << elibs[i] << " ";
            out << endl;
            out << "MDSERVER_LIBS_FORCED=";
            for (size_t i=0; i<mlibs.size(); i++)
                out << mlibs[i] << " ";
            out << endl;
            out << "VIEWER_LIBS_FORCED=";
            for (size_t i=0; i<vlibs.size(); i++)
                out << vlibs[i] << " ";
            out << endl;
            out << "ELIBS_FOR_MACOSX_PREBINDING=$(ZLIB_LIB) $(GLEW_LIBS) $(MESA_LIBS) $(GL_LIBS)" << endl;
            out << "MLIBS_FOR_MACOSX_PREBINDING=$(GLEW_LIBS) $(ZLIB_LIB) $(MESA_LIBS) $(GL_LIBS)" << endl;
            out << "ILIBS=" << endl;
            out << "MLIBS=-lpipeline_ser -ldatabase_ser -lmir_ser -lavtmath_ser -ldbatts -lavtexceptions -lstate -lmisc -lcomm -lexpr -lparser -lutility -lplugin -lvisit_vtk -llightweight_visit_vtk  $(MLIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS)" << endl;
            out << "ESERLIBS=-lpipeline_ser -ldatabase_ser -lmir_ser -lavtmath_ser -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lexpr -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk $(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS)" << endl;
            out << "EPARLIBS=-lpipeline_par -ldatabase_par -lmir_par -lavtmath_par -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lexpr -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk $(ELIBS_FOR_MACOSX_PREBINDING) $(VTK_LIBS) $(SHLIB_MPI_LIBS)" << endl;

            out << "" << endl;
            out << "IDSO="<<visitplugininstall<<"/databases/libI"<<name<<"Database" << VISIT_PLUGIN_EXTENSION << endl;
            out << "MDSO="<<visitplugininstall<<"/databases/libM"<<name<<"Database" << VISIT_PLUGIN_EXTENSION << endl;
            out << "ESERDSO="<<visitplugininstall<<"/databases/libE"<<name<<"Database_ser" << VISIT_PLUGIN_EXTENSION << endl;
            out << "EPARDSO="<<visitplugininstall<<"/databases/libE"<<name<<"Database_par" << VISIT_PLUGIN_EXTENSION << endl;
        }
        out << endl;
        out << "" << endl;
        out << "DISTRIB=" << endl;
        out << "" << endl;
        out << "##" << endl;
        out << "## Derived objects" << endl;
        out << "##" << endl;
        out << "IOBJ_tmp=$(ISRC:.C=.o)" << endl;
        out << "GOBJ_tmp=$(COMMONSRC:.C=.o) $(GSRC:.C=.o)" << endl;
        out << "SOBJ_tmp=$(COMMONSRC:.C=.o) $(SSRC:.C=.o)" << endl;
        out << "VOBJ_tmp=$(COMMONSRC:.C=.o) $(VSRC:.C=.o)" << vGraphicsObjects << endl;
        out << "MOBJ_tmp=$(COMMONSRC:.C=.o) $(MSRC:.C=.o) $(MSPECIFICSRC:.C=_mds.o)" << endl;
        out << "ESEROBJ_tmp=$(COMMONSRC:.C=.o) $(ESRC:.C=.o)     $(ESPECIFICSRC:.C=_eng.o)" << eserGraphicsObjects << endl;
        out << "EPAROBJ_tmp=$(COMMONSRC:.C=.o) $(ESRC:.C=_par.o) $(ESPECIFICSRC:.C=_par_eng.o)" << eparGraphicsObjects << endl;
        out << "" << endl;
        out << "IOBJ=$(IOBJ_tmp:.c=.o)" << endl;
        out << "GOBJ=$(GOBJ_tmp:.c=.o)" << endl;
        out << "SOBJ=$(SOBJ_tmp:.c=.o)" << endl;
        out << "VOBJ=$(VOBJ_tmp:.c=.o)" << endl;
        out << "MOBJ=$(MOBJ_tmp:.c=.o)" << endl;
        out << "ESEROBJ=$(ESEROBJ_tmp:.c=.o)" << endl;
        out << "EPAROBJ=$(EPAROBJ_tmp:.c=.o)" << endl;
        out << "" << endl;
        out << "MOCSRC = $(WIDGETS:.h=_moc.C)" << endl;
        out << "MOCOBJ = $(MOCSRC:.C=.o)" << endl;
        out << endl;
        out << "VIEWERMOCSRC = $(VIEWERWIDGETS:.h=_moc.C)" << endl;
        out << "VIEWERMOCOBJ = $(VIEWERMOCSRC:.C=.o)" << endl;
        out << endl;
        out << "##" << endl;
        out << "## Standard targets..." << endl;
        out << "##" << endl;
        if (type!="database")
        {
            out << "all: message $(IDSO) $(GUILIB) $(VIEWERLIB) $(ENGINELIBSER) $(ENGINELIBPAR) $(SCRIPTINGLIB) $(JAVACLASS)" << endl;
        }
        else
        {
            if (noEnginePlugin)
                out << "all: message $(IDSO) $(MDSERVERLIB)" << endl;
            else if (onlyEnginePlugin)
                out << "all: message $(IDSO) $(ENGINELIBSER) $(ENGINELIBPAR)" << endl;
            else
                out << "all: message $(IDSO) $(MDSERVERLIB) $(ENGINELIBSER) $(ENGINELIBPAR)" << endl;
        }
        out << "" << endl;
        out << "clean:" << endl;
        out << "\t$(RM) $(IOBJ) $(COMMONOBJ)" << endl;
        out << "\t$(RM) $(GOBJ) $(SOBJ) $(VOBJ) $(MOBJ) $(ESEROBJ) $(EPAROBJ)" << endl;
        out << "\t$(RM) $(MOCSRC) $(MOCOBJ) $(VIEWERMOCSRC) $(VIEWERMOCOBJ)" << endl;
        out << "\t$(RM) $(IDSO) $(GDSO) $(SDSO) $(VDSO) $(MDSO) $(ESERDSO) $(EPARDSO)" << endl;
        out << "\t$(RM) $(JAVAOBJ)" << endl;
        out << "" << endl;
        out << "##" << endl;
        out << "## Other targets..." << endl;
        out << "##" << endl;
        out << "message:" << endl;
        out << "\t@echo" << endl;
        if (type=="operator")
        {
            out << "\t@echo \"****************************************************************************\"" << endl;
            out << "\t@echo \"*** Building "<<label<<" Operator Plugin\"" << endl;
            out << "\t@echo \"****************************************************************************\"" << endl;
        }
        else if (type=="plot")
        {
            out << "\t@echo \"****************************************************************************\"" << endl;
            out << "\t@echo \"*** Building "<<label<<" Plot Plugin\"" << endl;
            out << "\t@echo \"****************************************************************************\"" << endl;
        }
        else if (type=="database")
        {
            out << "\t@echo \"****************************************************************************\"" << endl;
            if (noEnginePlugin)
                out << "\t@echo \"*** Building "<<label<<" Database Plugin (no engine)\"" << endl;
            else if (onlyEnginePlugin)
                out << "\t@echo \"*** Building "<<label<<" Database Plugin (engine only)\"" << endl;
            else
                out << "\t@echo \"*** Building "<<label<<" Database Plugin\"" << endl;
            out << "\t@echo \"****************************************************************************\"" << endl;
        }
        out << "" << endl;
        out << "##" << endl;
        out << "## Automatic dependency stuff" << endl;
        out << "##" << endl;
        out << "include "<<visithome<<"/include/make-targets" << endl;
    }
};


// ----------------------------------------------------------------------------
//                           Override default types
// ----------------------------------------------------------------------------
#define Plugin       MakefileGeneratorPlugin

#endif
