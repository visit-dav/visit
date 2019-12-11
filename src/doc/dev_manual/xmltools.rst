XML Tools
============================

Overview
--------------------------------
VisIt developers use several xml-based code generation tools to implement
VisIt's features. The source core for these tools is kept in 
``src/tools/dev/xml/`` and ``src/tools/dev/xmledit/``.


CMake Integration
--------------------

We rely on xml code generation to keep our State object, Attribute, and Plugin
APIs up-to-date. To automate the process we provide CMake targets that call our
xml code generation tools for each object or plugin registred. Individual code
gen targets are all wired into top level targets that allow you to apply the
code gen tools to categories of code gen tasks.  These targets replace older
tools such as regen-ajp and various regenerateatts.py scripts.


Top Level CMake Code Gen Targets
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

=========================== ==================================================
CMake Target                 Target Action
=========================== ==================================================
**gen_cpp_all**              Run xml2atts on all identified objects
**gen_python_all**           Run xml2python on all identified objects
**gen_java_all**             Run xml2java on all identified objects
**gen_info_all**             Run xml2info on all plugins
**gen_cmake_all**            Run xml2cmake on all plugins
**gen_plugin_all**           Run all applicable xml tools for all plugins
=========================== ==================================================

CMake Code Gen Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~

These are the helper functions we use to create targets that call xml tools in
our CMake build system.

XML Tools Helper functions in ``src/CMake/VisItMacros.cmake``:

=========================== ==================================================
CMake Function               Target Action
=========================== ==================================================
**ADD_CPP_GEN_TARGET**       Calls xml2atts
**ADD_PYTHON_GEN_TARGET**    Calls xml2python
**ADD_JAVA_GEN_TARGET**      Calls xml2java
**ADD_INFO_GEN_TARGET**      Calls xml2info
**ADD_CMAKE_GEN_TARGET**     Calls xml2cmake
=========================== ==================================================


The xml2plugin and xml2avt tools are only called when you first create a new
plugin or object, they are not exposed here. 

Plugin Tools Helper functions in ``src/CMake/PluginMacros.cmake``:

* **ADD_PLUGIN_CODE_GEN_TARGETS** - wires up:

 * **ADD_CPP_GEN_TARGET**
 * **ADD_PYTHON_GEN_TARGET**
 * **ADD_JAVA_GEN_TARGET**
 * **ADD_INFO_GEN_TARGET**
 * **ADD_CMAKE_GEN_TARGET**

* **ADD_DATABASE_CODE_GEN_TARGETS** - wires up:

 * **ADD_INFO_GEN_TARGET**
 * **ADD_CMAKE_GEN_TARGET**

* **ADD_OPERATOR_CODE_GEN_TARGETS** - alias for ADD_PLUGIN_CODE_GEN_TARGETS
* **ADD_PLOT_CODE_GEN_TARGETS** - alias for ADD_PLUGIN_CODE_GEN_TARGETS


CMake Options
~~~~~~~~~~~~~~~~~

**VISIT_CMAKE_VERBOSE_GEN_TARGET_MESSAGES** (default: **OFF**) When enabled
display messages about code gen targets during CMake configure time.




