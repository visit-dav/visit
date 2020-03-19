# This script attempts to combine the relevant .a's for the whole build into super
# libsimV2_static_ser.a and libsimV2_static_par.a so we don't need to link with a ton
# of VisIt libraries when we statically link a simulation.
#
# Creation: Brad Whitlock
# Date: Mon Oct 24 17:16:24 PDT 2011
#
# Modifications:
#
#   Jonathan Byrd (Allinea Software) Sun Dec 18, 2011
#   Added static libddtsim to parallel archive
#
#   Kathleen Biagas, Thu Jun 12 10:19:07 PDT 2014
#   Moved GLEW to visit_deps as visitGLEW, add libtess2 to visit_deps.
#   Split avtivp into ser and par versions.
#
################################################################################
import os, sys

visit_deps = (
"libenginerpc.a",
"liblightweight_visit_vtk.a",
"libmdserverproxy.a",
"libmdserverrpc.a",
"libvisit_verdict.a",
"libvisit_vtk.a",
"libvisitcommon.a",
"libsimV2.a",
"libtess2.a",
"libnek5000_interp.a")

avt = (
"libavtdbatts.a",
"libavtmath.a",
"libavtqtviswindow.a",
"libavtshapelets.a",
"libavtview.a")

avt_ser = (
"libavtdatabase_ser.a",
"libavtdbin_ser.a",
"libavtexpressions_ser.a",
"libavtfilters_ser.a",
"libavtivp_ser.a",
"libavtmir_ser.a",
"libavtpipeline_ser.a",
"libavtplotter_ser.a",
"libavtquery_ser.a",
"libavtviswindow_ser.a",
"libavtwriter_ser.a")

engine_ser = (
"libsimV2runtime_ser.a",
"libengine_ser.a")

avt_par = (
"libcognomen.a",
"libavtdatabase_par.a",
"libavtdbin_par.a",
"libavtexpressions_par.a",
"libavtfilters_par.a",
"libavtivp_par.a",
"libavtmir_par.a",
"libavtpipeline_par.a",
"libavtplotter_par.a",
"libavtquery_par.a",
"libavtviswindow_par.a",
"libavtwriter_par.a")

engine_par = (
"libsimV2runtime_par.a",
"libengine_par.a")

ddtsim = (
"libddtsim_static.a",
)

def get_plugin_file_list(filename):
    print(filename)
    f = open(filename, "rt")
    lines = f.readlines()
    plugins = []
    pv = "PLUGIN_VERB"
    lpv = len(pv)
    for line in lines:
        if line[:lpv] == pv:
            pname =  line[lpv+1:-2]
            plugins = plugins + [pname]
    return tuple(plugins)

def get_plot_plugin_archives(plugins, parallel_suffix):
    archives = []
    for p in plugins:
        archives = archives + ["libI%sPlot.a" % p, "libE%sPlot%s.a" % (p, parallel_suffix)]
    return tuple(archives)

def get_operator_plugin_archives(plugins, parallel_suffix):
    archives = []
    for p in plugins:
        archives = archives + ["libI%sOperator.a" % p, "libE%sOperator%s.a" % (p, parallel_suffix)]
    return tuple(archives)

def get_database_plugin_archives(parallel_suffix):
    p = "SimV2"
    archives = ("libI%sDatabase.a" % p, "libE%sDatabase%s.a" % (p, parallel_suffix))
    return archives

def build_ar_command(output_archive, path, source_archives):
    cmd = "ar -rc " + path + "/" + output_archive
    for s in source_archives:
        cmd = cmd + " " + path + "/" + s
    return cmd

def toplevel():
    # Determine the top level directory relative to where we are.
    return os.path.abspath(".")

def libpath():
    return toplevel() + "/lib"

def includepath():
    return toplevel() + "/include"

def os_system(cmd):
    print(cmd)
    os.system(cmd)

def build_archive(archive, tmpdir, path, archlist):
    oldDir = os.path.abspath(os.curdir)
    os.mkdir(tmpdir)
    os.chdir(tmpdir)
    for a in archlist:
        os_system("ar -x %s/%s" % (path, a))

    libname = "%s/%s" % (path, archive)
    os_system("ar cr %s *.o" % libname)
    os_system("ranlib %s" % libname)
    print("Combined archive created at: ", libname)

    os.chdir(oldDir)
    os_system("rm -rf %s" % tmpdir)

#
# Try and combine all VisIt libraries into libsimV2_static_ser.a
#
plot_ser     = get_plot_plugin_archives(get_plugin_file_list(includepath() + "/enabled_plots.h"), "_ser")
operator_ser = get_operator_plugin_archives(get_plugin_file_list(includepath() + "/enabled_operators.h"), "_ser")
database_ser = get_database_plugin_archives("_ser")
ser_libs     = engine_ser + plot_ser + operator_ser + database_ser + avt + avt_ser + visit_deps
build_archive("libsimV2_static_ser.a", "serial_tmp", libpath(), ser_libs)

#
# Try and combine all parallel VisIt libraries into libsimV2_static_par.a
#
plot_par     = get_plot_plugin_archives(get_plugin_file_list(includepath() + "/enabled_plots.h"), "_par")
operator_par = get_operator_plugin_archives(get_plugin_file_list(includepath() + "/enabled_operators.h"), "_par")
database_par = get_database_plugin_archives("_par")
par_libs     = engine_par + plot_par + operator_par + database_par + avt + avt_par + visit_deps + ddtsim
build_archive("libsimV2_static_par.a", "parallel_tmp", libpath(), par_libs)

def get_linkline(libs):
    s = ""
    for lib in libs:
        s = s + "-l" + lib[3:-2] + " "
    return s

#
# Print the link lines that we'd have to use to get the -l's
#
vtk_libs="-lvtkCommon -lvtkGraphics -lvtkHybrid -lvtkRendering -lvtkImaging -lvtkGenericFiltering -lvtkFiltering -lvtkIO -lvtksys -lvtkfreetype -lvtkftgl -lvtkjpeg -lvtktiff -lvtkpng -lvtkexpat -lvtklibxml2 -lvtkzlib "
frameworks = "-lGL "
if sys.platform == "darwin":
    frameworks = "-framework OpenGL -framework AGL -framework SystemConfiguration -framework IOKit -framework Cocoa "

ser_link_line = get_linkline(ser_libs) + "-lz " + vtk_libs + frameworks
print("Serial link line if you link each library: ")
print(ser_link_line)

par_link_line = get_linkline(par_libs) + "-lz " + vtk_libs + frameworks
print("Parallel link line if you link each library: ")
print(par_link_line)
