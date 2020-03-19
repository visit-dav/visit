import os, string, sys

VisItLibs =(\
"libvisitcommon.dylib",
"libavtdbatts.dylib",
"libavtpipeline_par.dylib",
"libavtpipeline_ser.dylib",
"libavtmath.dylib",
"libvisit_vtk.dylib",
"liblightweight_visit_vtk.dylib",
"libXDBLib.dylib"
)

VTKLibs = (\
"libvtkCommonComputationalGeometry-6.1.1.dylib",
"libvtkCommonCore-6.1.1.dylib",
"libvtkCommonDataModel-6.1.1.dylib",
"libvtkCommonExecutionModel-6.1.1.dylib",
"libvtkCommonMath-6.1.1.dylib",
"libvtkCommonMisc-6.1.1.dylib",
"libvtkCommonSystem-6.1.1.dylib",
"libvtkCommonTransforms-6.1.1.dylib",
"libvtkDICOMParser-6.1.1.dylib",
"libvtkFiltersCore-6.1.1.dylib",
"libvtkFiltersExtraction-6.1.1.dylib",
"libvtkFiltersFlowPaths-6.1.1.dylib",
"libvtkFiltersGeneral-6.1.1.dylib",
"libvtkFiltersGeometry-6.1.1.dylib",
"libvtkFiltersSources-6.1.1.dylib",
"libvtkFiltersStatistics-6.1.1.dylib",
"libvtkIOCore-6.1.1.dylib",
"libvtkIOImage-6.1.1.dylib",
"libvtkIOLegacy-6.1.1.dylib",
"libvtkIOXMLParser-6.1.1.dylib",
"libvtkImagingCore-6.1.1.dylib",
"libvtkImagingFourier-6.1.1.dylib",
"libvtkImagingHybrid-6.1.1.dylib",
"libvtkInteractionStyle-6.1.1.dylib",
"libvtkRenderingCore-6.1.1.dylib",
"libvtkRenderingFreeType-6.1.1.dylib",
"libvtkRenderingFreeTypeOpenGL-6.1.1.dylib",
"libvtkRenderingOpenGL-6.1.1.dylib",
"libvtkalglib-6.1.1.dylib",
"libvtkexpat-6.1.1.dylib",
"libvtkfreetype-6.1.1.dylib",
"libvtkftgl-6.1.1.dylib",
"libvtkjpeg-6.1.1.dylib",
"libvtkmetaio-6.1.1.dylib",
"libvtkpng-6.1.1.dylib",
"libvtktiff-6.1.1.dylib",
"libvtksys-6.1.1.dylib",
"libvtkzlib-6.1.1.dylib")

MPILibs = (\
"libpmpich.dylib",
"libmpich.dylib",
"libopa.dylib",
"libmpl.dylib",
"libpmpich.10.dylib",
"libmpich.10.dylib",
"libopa.1.dylib",
"libmpl.1.dylib"
)

def joinlib(a, b):
    return string.replace(os.path.join(a,b), "/./", "/")

def change_lib(libname, visitpath, vtkpath, mpipath, order):
    lib = os.path.split(libname)[-1]
    a,b = order(joinlib("@rpath/lib", lib), joinlib(visitpath, lib))
    cmd = "install_name_tool -id %s %s" % (b, libname)
    print(cmd)
    a = os.system(cmd)
    for lib in VisItLibs:
        a,b = order(joinlib("@rpath/lib", lib), joinlib(visitpath, lib))
        cmd = "install_name_tool -change %s %s %s" % (a, b, libname)
        print(cmd)
        a = os.system(cmd)
    for lib in VTKLibs:
        a,b = order(joinlib("@rpath/lib", lib), joinlib(vtkpath, lib))
        cmd = "install_name_tool -change %s %s %s" % (a, b, libname)
        print(cmd)
        a = os.system(cmd)
    for lib in MPILibs:
        a,b = order(joinlib("@rpath/lib", lib), joinlib(mpipath, lib))
        cmd = "install_name_tool -change %s %s %s" % (a, b, libname)
        print(cmd)
        a = os.system(cmd)
        # Hack for now.
        #cmd = string.replace(cmd, "2.8.0", "2.7.0")
        #print cmd
        #a = os.system(cmd)

#
# Transforms rpath in the library to the VTK and MPICH paths
#
def rpath_to_vtk_mpich(libname, visitpath, vtkpath, mpipath):
    def order(a,b):
        return (a,b)
    print("Turning rpath in %s to proper VTK and MPICH paths." % libname)
    change_lib(libname, visitpath, vtkpath, mpipath, order)

#
# Transforms VTK,MPICH paths to rpath
#
def vtk_mpich_to_rpath(libname, visitpath, vtkpath, mpipath):
    def order(a,b):
        return (b,a)
    print("Turning proper paths to VTK and MPICH in %s to rpath." % libname)
    change_lib(libname, visitpath, vtkpath, mpipath, order)

if sys.argv[1] == "-to-rpath":
    vtk_mpich_to_rpath(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
elif sys.argv[1] == "-from-rpath":
    rpath_to_vtk_mpich(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])


sys.exit(0)
