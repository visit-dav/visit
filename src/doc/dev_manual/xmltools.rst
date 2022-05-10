.. _XML Tools:

XML Tools
=========

Overview
--------
VisIt_ developers use several xml-based code generation tools to implement VisIt_'s features. 
The source core for these tools is kept in ``src/tools/dev/xml/`` and ``src/tools/dev/xmledit/``.

Types Managed by XML Tools
--------------------------
These XML tools are designed to be aware of a number of basic data types. 
These include...

* ``Bool``
* ``Int``
* ``Float``
* ``Double``
* ``UChar`` (aka Unsigned Char)
* ``String``

In addition to these *basic* types as *scalars*, we also support ``Arrays`` of fixed length and ``Vectors`` of arbitrary length of these basic types.
There is also support for a number of types that require special handling. 
These are...

* types defined in the ``avtTypes.h`` `header file <https://github.com/visit-dav/visit/blob/develop/src/avt/DBAtts/MetaData/avtTypes.h>`_

   .. container:: collapsible

       .. container:: header

           **Show/Hide Code for** ``avtTypes.h``

       .. literalinclude:: ../../../src/avt/DBAtts/MetaData/avtTypes.h
           :language: C++
           :start-after: #ifndef AVT_TYPES_H
           :end-before: DBATTS_API extern const char * AUXILIARY_DATA_DATA_EXTENTS;

* types representing some high-level knowledge such as a ``LineWidth``, a database ``VariableName``, an ``Opacity`` (slider) value, etc.

The implementation of these types in C++, Java, Python and Qt is handled by various of the ``GenerateXXX.h`` `header files <https://github.com/visit-dav/visit/tree/b962e70292903a0444d2c95cd7a6e64e8e946a75/src/tools/dev/xml>`_.

For more details, please see `this issue <https://github.com/visit-dav/visit/issues/5875>`_.

CMake Integration
--------------------

We rely on xml code generation to keep our State object, Attribute, and Plugin APIs up-to-date. 
To automate the process we provide CMake targets that call our xml code generation tools for each object or plugin registered. 
Individual code gen targets are all wired into top level targets that allow you to apply the code gen tools to categories of code gen tasks.  
These targets replace older tools such as regen-ajp and various regenerateatts.py scripts. 
Keep in mind however, that these targets are only created for plugins that are enabled for building. 
Any use of the plugin-reducing CMake vars (*VISIT_BUILD_MINIMAL_PLUGINS* and any of the *VISIT_SELECTED_XXX_PLUGINS*) will limit the created code gen targets to those plugins being built.


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


