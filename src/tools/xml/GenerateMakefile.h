#ifndef GENERATE_MAKEFILE_H
#define GENERATE_MAKEFILE_H

#include "Field.h"
#include <visit-config.h> // for the plugin extension.

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
// ****************************************************************************

class MakefileGeneratorPlugin
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
    MakefileGeneratorPlugin(const QString &n,const QString &l,const QString &t,const QString &vt,const QString &dt,const QString &v, const QString&w, bool hw)
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
            defaultefiles.push_back(filter);
        }
    }
    void Print(ostream &out)
    {
        out << "Plugin: "<<name<<" (\""<<label<<"\", type="<<type<<") -- version "<<version<< endl;
        if (atts)
            atts->Print(cout);
    }
    void WriteMakefile(ostream &out)
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
        out << "  -I"<<vtkdir<<"/MangleMesaInclude \\"<<endl;
        out << "  -I"<<vtkdir<<"/Rendering"<<endl;
        out << "MOC="<<visithome<<"/bin/moc" << endl;
        out << "CXXFLAGS=$(CXXFLAGSORIG) $(QT_CXXFLAGS) $(PY_CXXFLAGS)";
        for (int i=0; i<cxxflags.size(); i++)
            out << " " << cxxflags[i];
        out << endl;
        out << "CPPFLAGS=$(CPPFLAGSORIG) $(VTK_INCLUDE) $(MESA_INCLUDE) -I. -I"<<visithome<<"/include -I"<<visithome<<"/include/visit" << endl;
        out << "LDFLAGS=$(LDFLAGSORIG) $(PY_LDFLAGS) ";
        for (int i=0; i<ldflags.size(); i++)
            out << " " << ldflags[i];
        out << " -L../../plugins/" << type << "s";
        out << endl;
        out << "" << endl;
        out << "##" << endl;
        out << "## Files..." << endl;
        out << "##" << endl;
        if (type=="operator")
        {
            out << "PLUGINDIR=operators" << endl;
            out << "PLUGINNAME=" << name << "Operator"<< endl;
            out << endl;
            out << "WIDGETS=Qvis"<<name<<"Window.h";
            if (customwfiles)
                for (int i=0; i<wfiles.size(); i++)
                    out << " " << wfiles[i];
            out << endl;
            out << "ISRC="<<name<<"PluginInfo.C" << endl;
            out << "COMMONSRC=";
#ifndef __APPLE__
            out << name<<"PluginInfo.C ";
#endif
            out <<name<<"CommonPluginInfo.C "<<atts->name<<".C" << endl;
            out << "GSRC="<<name<<"GUIPluginInfo.C Qvis"<<name<<"Window.C";
            if (customgfiles)
                for (int i=0; i<gfiles.size(); i++)
                    out << " " << gfiles[i];
            else
                for (int i=0; i<defaultgfiles.size(); i++)
                    out << " " << defaultgfiles[i];
            out << endl;
            out << "VSRC="<<name<<"ViewerPluginInfo.C";
            if (customvfiles)
                for (int i=0; i<vfiles.size(); i++)
                    out << " " << vfiles[i];
            else
                for (int i=0; i<defaultvfiles.size(); i++)
                    out << " " << defaultvfiles[i];
            out << endl;
            out << "ESRC="<<name<<"EnginePluginInfo.C";
            if (customefiles)
                for (int i=0; i<efiles.size(); i++)
                    out << " " << efiles[i];
            else
                for (int i=0; i<defaultefiles.size(); i++)
                    out << " " << defaultefiles[i];
            out << endl;
            out << "SSRC="<<name<<"ScriptingPluginInfo.C Py"<<atts->name<<".C";
            if (customsfiles)
                for (int i=0; i<sfiles.size(); i++)
                    out << " " << sfiles[i];
            else
                for (int i=0; i<defaultsfiles.size(); i++)
                    out << " " << defaultsfiles[i];

            out << "" << endl;
            out << "" << endl;
            out << "JAVASRC="<<atts->name<<".java" << endl;
            out << "JAVAOBJ="<<visithome<<"/java/llnl/visit/operators/"<<atts->name<<".class" << endl;
            out << "JAVAPLUGINFLAGS=-d "<<visithome<<"/java -classpath " << visithome << "/java" << endl;  

            out << "" << endl;
            out << "SHLIB_FORCED=";
            for (int i=0; i<libs.size(); i++)
                out << libs[i] << " ";
            out << endl;
            out << "" << endl;
            out << "ILIBS=" << endl;
            out << "GLIBS=-lgui -lmdserverproxy -lviewerproxy -lproxybase "
                   "-lmdserverrpc -lviewerrpc -lwinutil -ldbatts -lavtexceptions "
                   "-lstate -lcomm -lmisc -lplugin -lparser -lutility "
                   "-lI$(PLUGINNAME) $(QT_LDFLAGS) $(QT_LIBS) $(X_LIBS)" << endl;
            out << "SLIBS=-lstate -lmisc -lcomm -lutility $(PY_LIB) -lI$(PLUGINNAME)" << endl;
            out << "VLIBS=-lpipeline_ser -lplotter -lavtfilters_ser "
                   "-lavtmath_ser -lavtview -ldbatts -lavtexceptions -lstate "
                   "-lmisc -lcomm -lparser -lutility -lvisit_vtk "
                   "-llightweight_visit_vtk ";
            //
            // HACK HACK HACK -- This should be a flag in the XML file instead
            //                   of using the name of the operator. The flag
            //                   should be called something like: UsesViewerLib.
            //
            //                   When the operator uses classes out of the viewer
            //                   library, we have to link all of the viewer
            //                   dependencies and most operators don't do need to.
            //
            bool usesViewerLib = (name == "OnionPeel");
            if (usesViewerLib)
            {
               out << "-lviewer -lviewerrpc -lproxybase -lvclproxy -lvclrpc "
                      "-lmdserverproxy -lmdserverrpc -lengineproxy -lenginerpc "
                      "-lplugin -lavtwriter -lviswindow -lqtviswindow -lvtkqt "
                      "-lwinutil $(QT_LIBS) ";
            }

            // Add the rest of the viewer operator libs
            out << "-lI$(PLUGINNAME) $(VTK_LIBS)" << endl;
            out << "ESERLIBS=-lpipeline_ser -lplotter -lavtfilters_ser -lavtmath_ser -lavtview -ldbatts -lavtexceptions -lstate -lmisc -lcomm -lparser -lutility -lvisit_vtk -llightweight_visit_vtk -lI$(PLUGINNAME) $(VTK_LIBS)" << endl;
            out << "EPARLIBS=-lpipeline_par -lplotter -lavtfilters_par -lavtmath_par -lavtview -ldbatts -lavtexceptions -lstate -lmisc -lcomm -lparser -lutility -lvisit_vtk -llightweight_visit_vtk -lI$(PLUGINNAME) $(VTK_LIBS) $(SHLIB_MPI_LIBS)" << endl;
            out << "" << endl;
            out << "IDSO="<<visitplugininstall<<"/operators/libI"<<name<<"Operator" << PLUGIN_EXTENSION << endl;
            out << "GDSO="<<visitplugininstall<<"/operators/libG"<<name<<"Operator" << PLUGIN_EXTENSION << endl;
            out << "SDSO="<<visitplugininstall<<"/operators/libS"<<name<<"Operator" << PLUGIN_EXTENSION << endl;
            out << "VDSO="<<visitplugininstall<<"/operators/libV"<<name<<"Operator" << PLUGIN_EXTENSION << endl;
            out << "ESERDSO="<<visitplugininstall<<"/operators/libE"<<name<<"Operator_ser" << PLUGIN_EXTENSION << endl;
            out << "EPARDSO="<<visitplugininstall<<"/operators/libE"<<name<<"Operator_par" << PLUGIN_EXTENSION << endl;
        }
        else if (type=="plot")
        {
            out << "PLUGINDIR=plots" << endl;
            out << "PLUGINNAME=" << name << "Plot"<< endl;
            out << endl;
            out << "WIDGETS=Qvis"<<name<<"PlotWindow.h";
            if (customwfiles)
                for (int i=0; i<wfiles.size(); i++)
                    out << " " << wfiles[i];
            out << endl;
            out << "ISRC="<<name<<"PluginInfo.C" << endl;
            out << "COMMONSRC=";
#ifndef __APPLE__
            out << name<<"PluginInfo.C ";
#endif
            out << name<<"CommonPluginInfo.C "<<atts->name<<".C" << endl;
            out << "GSRC="<<name<<"GUIPluginInfo.C Qvis"<<name<<"PlotWindow.C";
            if (customgfiles)
                for (int i=0; i<gfiles.size(); i++)
                    out << " " << gfiles[i];
            else
                for (int i=0; i<defaultgfiles.size(); i++)
                    out << " " << defaultgfiles[i];
            out << endl;
            out << "SSRC="<<name<<"ScriptingPluginInfo.C Py"<<atts->name<<".C";
            if (customsfiles)
                for (int i=0; i<sfiles.size(); i++)
                    out << " " << sfiles[i];
            else
                for (int i=0; i<defaultsfiles.size(); i++)
                    out << " " << defaultsfiles[i];
            out << endl;
            out << "VSRC="<<name<<"ViewerPluginInfo.C avt"<<name<<"Plot.C";
            if (customvfiles)
                for (int i=0; i<vfiles.size(); i++)
                    out << " " << vfiles[i];
            else
                for (int i=0; i<defaultvfiles.size(); i++)
                    out << " " << defaultvfiles[i];
            out << endl;
            out << "ESRC="<<name<<"EnginePluginInfo.C avt"<<name<<"Plot.C";
            if (customefiles)
                for (int i=0; i<efiles.size(); i++)
                    out << " " << efiles[i];
            else
                for (int i=0; i<defaultefiles.size(); i++)
                    out << " " << defaultefiles[i];
            out << endl;

            out << "" << endl;
            out << "" << endl;
            out << "JAVASRC="<<atts->name<<".java" << endl;
            out << "JAVAOBJ="<<visithome<<"/java/llnl/visit/plots/"<<atts->name<<".class" << endl;
            out << "JAVAPLUGINFLAGS=-d "<<visithome<<"/java -classpath " << visithome << "/java" << endl;  

            out << "" << endl;
            out << "SHLIB_FORCED=";
            for (int i=0; i<libs.size(); i++)
                out << libs[i] << " ";
            out << endl;
            out << "" << endl;
            out << "ILIBS=" << endl;
            out << "GLIBS=-lgui -lmdserverproxy -lviewerproxy -lproxybase -lmdserverrpc -lviewerrpc -lwinutil -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lplugin -lparser -lutility -lI$(PLUGINNAME) $(QT_LDFLAGS) $(QT_LIBS) $(X_LIBS)" << endl;
            out << "SLIBS=-lstate -lmisc -lcomm -lutility $(PY_LIB) -lI$(PLUGINNAME)" << endl;
            out << "VLIBS=-lpipeline_ser -lplotter -lavtfilters_ser -lavtmath_ser -lavtview -ldbatts -lavtexceptions -lstate -lmisc -lcomm -lparser -lutility -lvisit_vtk -llightweight_visit_vtk -lI$(PLUGINNAME) $(VTK_LIBS)" << endl;
            out << "ESERLIBS=-lpipeline_ser -lplotter -lavtfilters_ser -lavtmath_ser -lavtview -ldbatts -lavtexceptions -lstate -lmisc -lcomm -lparser -lutility -lvisit_vtk -llightweight_visit_vtk -lI$(PLUGINNAME) $(VTK_LIBS)" << endl;
            out << "EPARLIBS=-lpipeline_par -lplotter -lavtfilters_par -lavtmath_par -lavtview -ldbatts -lavtexceptions -lstate -lmisc -lcomm -lparser -lutility -lvisit_vtk -llightweight_visit_vtk -lI$(PLUGINNAME) $(VTK_LIBS) $(SHLIB_MPI_LIBS)" << endl;
            out << "" << endl;
            out << "IDSO="<<visitplugininstall<<"/plots/libI"<<name<<"Plot" << PLUGIN_EXTENSION << endl;
            out << "GDSO="<<visitplugininstall<<"/plots/libG"<<name<<"Plot" << PLUGIN_EXTENSION << endl;
            out << "SDSO="<<visitplugininstall<<"/plots/libS"<<name<<"Plot" << PLUGIN_EXTENSION << endl;
            out << "VDSO="<<visitplugininstall<<"/plots/libV"<<name<<"Plot" << PLUGIN_EXTENSION << endl;
            out << "ESERDSO="<<visitplugininstall<<"/plots/libE"<<name<<"Plot_ser" << PLUGIN_EXTENSION << endl;
            out << "EPARDSO="<<visitplugininstall<<"/plots/libE"<<name<<"Plot_par" << PLUGIN_EXTENSION << endl;
        }
        else if (type=="database")
        {
            out << "PLUGINDIR=databases" << endl;
            out << "PLUGINNAME=" << name << "Database"<< endl;
            out << endl;

            out << "WIDGETS=" << endl;
            out << "ISRC="<<name<<"PluginInfo.C" << endl;
            out << "COMMONSRC=";
#ifndef __APPLE__
            out << name<<"PluginInfo.C ";
#endif
            out << name<<"CommonPluginInfo.C";
            out << endl;
            out << "MSRC="<<name<<"MDServerPluginInfo.C";
            if (haswriter)
            {
                out << " avt" << name << "Writer.C";
            }
            if (custommfiles)
                for (int i=0; i<mfiles.size(); i++)
                    out << " " << mfiles[i];
            else
                for (int i=0; i<defaultmfiles.size(); i++)
                    out << " " << defaultmfiles[i];
            out << endl;
            out << "ESRC="<<name<<"EnginePluginInfo.C";
            if (haswriter)
            {
                out << " avt" << name << "Writer.C";
            }
            if (customefiles)
                for (int i=0; i<efiles.size(); i++)
                    out << " " << efiles[i];
            else
                for (int i=0; i<defaultefiles.size(); i++)
                    out << " " << defaultefiles[i];
            out << endl;

            out << "" << endl;
            out << "SHLIB_FORCED=";
            for (int i=0; i<libs.size(); i++)
                out << libs[i] << " ";
            out << endl;
            out << "" << endl;
            out << "ILIBS=" << endl;
            out << "MLIBS=-lpipeline_ser -ldatabase_ser -lmir -lavtmath_ser -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk  -L../../plugins/databases -lI$(PLUGINNAME) $(VTK_LIBS)" << endl;
            out << "ESERLIBS=-lpipeline_ser -ldatabase_ser -lmir -lavtmath_ser -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk  -L../../plugins/databases -lI$(PLUGINNAME) $(VTK_LIBS)" << endl;
            out << "EPARLIBS=-lpipeline_par -ldatabase_par -lmir -lavtmath_par -ldbatts -lavtexceptions -lstate -lcomm -lmisc -lparser -lplugin -lutility -lvisit_vtk -llightweight_visit_vtk  -L../../plugins/databases -lI$(PLUGINNAME) $(VTK_LIBS) $(SHLIB_MPI_LIBS)" << endl;
            out << "" << endl;
            out << "IDSO="<<visitplugininstall<<"/databases/libI"<<name<<"Database" << PLUGIN_EXTENSION << endl;
            out << "MDSO="<<visitplugininstall<<"/databases/libM"<<name<<"Database" << PLUGIN_EXTENSION << endl;
            out << "ESERDSO="<<visitplugininstall<<"/databases/libE"<<name<<"Database_ser" << PLUGIN_EXTENSION << endl;
            out << "EPARDSO="<<visitplugininstall<<"/databases/libE"<<name<<"Database_par" << PLUGIN_EXTENSION << endl;
        }
        out << endl;
        out << "" << endl;
        out << "DISTRIB=" << endl;
        out << "" << endl;
        out << "##" << endl;
        out << "## Derived objects" << endl;
        out << "##" << endl;
        out << "IOBJ=$(ISRC:.C=.o)" << endl;
        out << "GOBJ=$(COMMONSRC:.C=.o) $(GSRC:.C=.o)" << endl;
        out << "SOBJ=$(COMMONSRC:.C=.o) $(SSRC:.C=.o)" << endl;
        out << "VOBJ=$(COMMONSRC:.C=.o) $(VSRC:.C=.o)" << endl;
        out << "MOBJ=$(COMMONSRC:.C=.o) $(MSRC:.C=.o)" << endl;
        out << "ESEROBJ=$(COMMONSRC:.C=.o) $(ESRC:.C=_ser.o)" << endl;
        out << "EPAROBJ=$(COMMONSRC:.C=.o) $(ESRC:.C=_par.o)" << endl;
        out << "" << endl;
        out << "MOCSRC = $(WIDGETS:.h=_moc.C)" << endl;
        out << "MOCOBJ = $(MOCSRC:.C=.o)" << endl;
        out << "" << endl;
        out << "##" << endl;
        out << "## Standard targets..." << endl;
        out << "##" << endl;
        if (type!="database")
        {
            out << "all: message $(IDSO) $(GUILIB) $(VIEWERLIB) $(ENGINELIBSER) $(ENGINELIBPAR) $(SCRIPTINGLIB) $(JAVACLASS)" << endl;
        }
        else
        {
            if (noengineplugin)
                out << "all: message $(IDSO) $(MDSERVERLIB)" << endl;
            else if (onlyengineplugin)
                out << "all: message $(IDSO) $(ENGINELIBSER) $(ENGINELIBPAR)" << endl;
            else
                out << "all: message $(IDSO) $(MDSERVERLIB) $(ENGINELIBSER) $(ENGINELIBPAR)" << endl;
        }
        out << "" << endl;
        out << "clean:" << endl;
        out << "\t$(RM) $(IOBJ) $(COMMONOBJ)" << endl;
        out << "\t$(RM) $(GOBJ) $(SOBJ) $(VOBJ) $(MOBJ) $(ESEROBJ) $(EPAROBJ)" << endl;
        out << "\t$(RM) $(MOCSRC) $(MOCOBJ)" << endl;
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
            if (noengineplugin)
                out << "\t@echo \"*** Building "<<label<<" Database Plugin (no engine)\"" << endl;
            else if (onlyengineplugin)
                out << "\t@echo \"*** Building "<<label<<" Database Plugin (engine only)\"" << endl;
            else
                out << "\t@echo \"*** Building "<<label<<" Database Plugin\"" << endl;
            out << "\t@echo \"****************************************************************************\"" << endl;
        }
        out << "" << endl;
        out << "##" << endl;
        out << "## moc" << endl;
        out << "##" << endl;
        out << "$(MOCSRC) or_no_widgets: $(WIDGETS)" << endl;
        out << "\t@rm -f $@" << endl;
        out << "\t$(MOC) $(@:_moc.C=.h) > $@" << endl;
        out << "" << endl;
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
