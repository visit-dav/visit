XML Tools
============================

Overview
--------------------------------
VisIt developers use several xml-based code generation tools to implement VisIt's features.
The source core for these tools is kept in ``src/tools/dev/xml/`` and ``src/tools/dev/xmledit/``.
 

CMake Code Generation Targets 
--------------------------------

We rely on xml code generation to keep our State object, Attribute, and Plugin APIs up-to-date. To automate the process we provide CMake targets that call our xml code generation tools for each object or plugin registred.  

Here are the top level target names:

* **gen_cpp_all** Run xml2atts on all identified objects
* **gen_python_all** Run xml2python on all identified objects
* **gen_java_all** Run xml2java on all identified objects
* **gen_info_all** Run xml2python on all plugins
* **gen_cmake_all** Run xml2cmake on all plugins
* **gen_plugin_all** Run all applciablet xml tools for all plugins


These targets replace older tools such as regen-ajp and various regenerateatts.py scripts.