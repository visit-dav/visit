#SET(VISITHOME /Users/childs/visit)
#SET(VISITARCH i386-apple-darwin10_gcc-4.2)

## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER icc TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER icpc TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS "-mmic -I/nics/d/home/hank/visit-trunk-src/third_party_builtin/zlib -I /nics/a/proj/visit/cross-compile-vtk/install/mesa/include" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS "-mmic -I/nics/d/home/hank/visit-trunk-src/third_party_builtin/zlib -I /nics/a/proj/visit/cross-compile-vtk/install/mesa/include" TYPE STRING)
VISIT_OPTION_DEFAULT(ZLIB_LIB "-L/nics/d/home/hank/visit-trunk-src/lib -lz" TYPE STRING)
VISIT_OPTION_DEFAULT(CMAKE_BUILD_TYPE Release)
VISIT_OPTION_DEFAULT(VISIT_USE_X OFF TYPE BOOL)



##
## VisIt Thread Option
##
VISIT_OPTION_DEFAULT(VISIT_THREAD ON TYPE BOOL)

VISIT_OPTION_DEFAULT(VISIT_SERVER_COMPONENTS_ONLY ON TYPE BOOL)
VISIT_OPTION_DEFAULT(VISIT_USE_GLEW OFF TYPE BOOL)
VISIT_OPTION_DEFAULT(VISIT_ZLIB ON TYPE BOOL)

VISIT_OPTION_DEFAULT(VISIT_VTK_DIR /nics/a/proj/visit/cross-compile-vtk/install/vtk)
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR /nics/a/proj/visit/cross-compile-vtk/install/mesa)


