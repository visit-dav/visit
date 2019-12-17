from visit_utils import *
from os.path import join as pjoin
import os
import time
from hdfs_export_utils import *

#
# Main driver program to export any data VisIt can read to hierarchy of
# text files (key-value pairs) suitable for HDFS (Hadoop Filesystem) import.
# Currently groups, material names, material specific variable values,
# units, labels and other miscellaneous metadata are not handled. CSG and
# MFEM meshes get imported according to their default discretization params.
#
# Example command-line 
#     env PYTHONPATH=<path-to-vtk-python-root> LD_LIBRARY_PATH=<path-to-vtk-lib> \
#         visit -cli -nowin -s ./hdfs_export_driver.py <database> <hdfs_export_root>
#
# where
#     path-to-vtk-python-root is path to the same VTK python root that the 
#         version of VisIt you are running is using. For example...
#         ${VISIT_INSTALL}/tpls/2.9.0/vtk/6.1.0/linux-x86_64/lib/python2.7/site-packages
#     path-to-vtk-lib is path to same VTK library root that the version
#         of VisIt you are running is using. For example...
#         ${VISIT_INSTALL}/tpls/2.9.0/vtk/6.1.0/linux-x86_64/lib
#     database is a VisIt database to export (including all timesteps)     
#     hdfs_export_root is a directory into which to pour all the files.
#
# You can re-run hdfs_export_driver.py script for each database you want to export
# and specify the same hdfs_export_root. If you have two databases with identical
# names, however, that will cause a namespace collision during the export. You will
# need to rename databases so that they do not match. Mesh entity keys (nodes, edges,
# faces, volumes) are uniqued across users, databases, timestates, meshes and blocks
# to enable Spark/Hadoop algorithms to operate at arbitrary scope.
#
# Programmer: Mark C. Miller, August, 2015
# Based heavily on original code written by Cyrus Harrison for JSON export
#

def export(outdir,keybase,user,dbfile,tidx,mesh,var_list,compressCmd="gzip -6"):
    result = AddPlot("Mesh", mesh)
    if result != 1:
        result = AddPlot("Curve", mesh)
    if result != 1:
        print("unable to add a relevant plot object")
        return
    DrawPlots()
    tval = query("Time")
    cval = query("Cycle")
    kargs = {"keybase":keybase,"outdir":outdir,"user":user,
        "dbfile":dbfile,"mesh":mesh,"time":tval,"cycle":cval,
        "compressCmd":compressCmd}
    sdir   = os.path.split(os.path.abspath(__visit_source_file__))[0]
    PythonQuery(file=(pjoin(sdir,"hdfs_export.vpq")),vars=var_list,args=[kargs])
    DeleteAllPlots()

def main():
    dbfile = Argv()[0]
    vdbroot = Argv()[1]
    user = os.environ['USER']
    db = os.path.basename(os.path.splitext(dbfile)[0]) 
    outDir = []
    keyBase = []

    descendOutDir(outDir,keyBase,vdbroot)
    descendOutDir(outDir,keyBase,user)
    descendOutDir(outDir,keyBase,db)

    # get all scalars
    SetTryHarderCyclesTimes(1)
    SetTreatAllDBsAsTimeVarying(1)
    result = OpenDatabase(dbfile)
    if not result:
        sys.exit()
    nts = TimeSliderGetNStates()
    var_list_first = []
    mds = []
    for tidx in range(nts):
        mds.append(GetMetaData(dbfile,tidx))
    CloseDatabase(dbfile)
    OpenDatabase(dbfile)
    for tidx in range(nts):
        TimeSliderSetState(tidx)
        md = mds[tidx]
        state_data = [md.GetTimes()[tidx],md.GetCycles()[tidx]]
        descendOutDir(outDir,keyBase,"%06d"%tidx,state_data)
        # handle curves specially (as kind of meshes)
        for c in range(md.GetNumCurves()):
            if md.GetCurves(c).validVariable == 0:
                continue
            if md.GetCurves(c).hideFromGUI == 1:
                continue
            curve = md.GetCurves(c).name
            var_list = [curve, curve.replace("Scalar_Curves/","")]
            descendOutDir(outDir,keyBase,curve.replace('/','~'),[1,1,1])
            print("[Exporting curve %s for time index %d]" %(curve,tidx))
            export(outDir,keyBase,user,dbfile,tidx,curve,var_list)
            ascendOutDir(outDir,keyBase)
        for m in range(md.GetNumMeshes()):
            if md.GetMeshes(m).validVariable == 0:
                continue
            if md.GetMeshes(m).hideFromGUI == 1:
                continue
            mesh = md.GetMeshes(m).name
            mdata = [md.GetMeshes(m).numBlocks,md.GetMeshes(m).spatialDimension,md.GetMeshes(m).topologicalDimension]
            # num blocks, spatial dim, topo dim, extents?
            descendOutDir(outDir,keyBase,mesh.replace('/','~'),mdata)
            var_list = ["__foo__"] # work-around mesh name overwriting first var
            # add vars for global nodes and zones, handle if missing in vpq
            DefineScalarExpression("%s_global_nodeids"%mesh.replace('/','~'), "global_nodeid(<%s>)"%mesh)
            DefineScalarExpression("%s_global_zoneids"%mesh.replace('/','~'), "global_zoneid(<%s>)"%mesh)
            var_list.append("%s_global_nodeids"%mesh.replace('/','~'))
            var_list.append("%s_global_zoneids"%mesh.replace('/','~'))
            for i in range(md.GetNumScalars()):
                if md.GetScalars(i).validVariable == 0:
                    continue
                if md.GetScalars(i).hideFromGUI == 1:
                    continue
                scals = md.GetScalars(i)
                if scals.meshName == mesh:
                    var_list.append(scals.name)
            for i in range(md.GetNumMaterials()):
                mat = md.GetMaterials(i)
                if mat.validVariable == 0:
                    continue
                if mat.hideFromGUI == 1:
                    continue
                if mat.meshName != mesh:
                    continue
                matvf_array_expr = "array_compose("
                for j in range(mat.numMaterials):
                    mat_varname = "%s_%s"%(mat.name,mat.materialNames[j])
                    DefineScalarExpression(mat_varname,"matvf(%s,%d)"%(mat.name,j+1))
                    if j == mat.numMaterials - 1:
                        matvf_array_expr += "<%s>)"%mat_varname
                    else:
                        matvf_array_expr += "<%s>,"%mat_varname
                if mat.numMaterials > 1:
                    DefineArrayExpression("materials", matvf_array_expr)
                    var_list.append("materials")
            print("[Exporting mesh %s for time index %d]" %(mesh,tidx))
            export(outDir,keyBase,user,dbfile,tidx,mesh,var_list)
            ascendOutDir(outDir,keyBase)
        ascendOutDir(outDir,keyBase)

if  __visit_script_file__ == __visit_source_file__:
    main()
    sys.exit()
