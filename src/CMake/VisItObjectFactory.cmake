# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#
# This is a plot-plugin example, creating a vtkOpenGL override for a base 
# Add these lines to the plugin's CMakeLists.txt
#   include(${VISIT_SOURCE_DIR}/VisItObjectFactory.cmake)
#   visit_add_override(MeshPlot vtkMeshPlotMapper vtkOpenGLMeshPlotMapper)
#   visit_object_factory_configure(MeshPlot MeshPlot_overrides)
# 
# add ${CMAKE_CURRENT_BINARY_DIR}/MeshPlotObjectFactory.C to the list of sources
# add ${CMAKE_CURRENT_BINARY_DIR} to the list of includes
#
# In the '.C' file for the class that uses the base class, add these lines
# #include <vtkAutoInit.h>
# VTK_MODULE_INIT(MeshPlot)
#

# visit_add_override, appends the 
# mod_name: the module name. Creates ${mod_name}OjbectFactory class
# base: the name of the base class
# override: the name of the class that will override base
macro(visit_add_override mod_name base override)
  list(APPEND visit_module_overrides ${base})
  set(visit_module_${base}_override ${override})
endmacro()

function(visit_object_factory_configure mod_name override_list)
    foreach(_class ${override_list})
        set(visit-module ${mod_name})
        set(_override ${visit_module_${_class}_override})
        set(_mod_override_includes
            "${_mod_override_includes}\n#include \"${_override}.h\"")
        set(_mod_override_creates "${_mod_override_creates} 
  VTK_CREATE_CREATE_FUNCTION(${_override})")
        set(_mod_override_do "${_mod_override_do}
    this->RegisterOverride(\"${_class}\",
                           \"${_override}\",
                           \"Override for ${visit-module} \", 1,
                           vtkObjectFactoryCreate${_override});")
  endforeach()
  configure_file(${VISIT_SOURCE_DIR}/CMake/VisItObjectFactory.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${mod_name}ObjectFactory.h)
  configure_file(${VISIT_SOURCE_DIR}/CMake/VisItObjectFactory.C.in
    ${CMAKE_CURRENT_BINARY_DIR}/${mod_name}ObjectFactory.C)

endfunction()

