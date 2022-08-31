.. _Updating Plugins:

Updating Plugins
================

When modifying existing plugins, keep in mind that there are certain files that have an automated re-generation process. 
These include *CMakeLists.txt*, *PluginInfo*, and (for Plots and Operators) *Attributes (cpp, python, java)*.
Being able to regenerate these files aids in future maintenance when the API needs to be changed for all plugins.


CMakeLists.txt
--------------
Regeneration of CMakeLists.txt is handled by the *xml2cmake* :ref:`XML Tool<XML Tools>`. 
Most changes to CMakeLists.txt can be handled by modifying/adding the necessary tags to the plugin's .xml file.
Some tags have an optional *components* attribute that specifies for which component the tag applies.
Multiple components can be specified in a single tag, and must be comma separated.
If no *components* attribute is specified, the tag will be applied to all components.

=================================    ===========================
Component                            Short name used in xml file
=================================    ===========================
Engine(both Serial and Parallel)     **E**
Serial Engine only                   **ESer**
Parallel Engine only                 **EPar**
MDServer                             **M**
GUI                                  **G**
Python scripting                     **S**
Viewer                               **V**
Widgets                              **W**
Widgets for viewer                   **VW**
=================================    ===========================

Here is a list of most often updated tags:

================== ====================================== ===================== 
Tag                Purpose                                Supported components
================== ====================================== ===================== 
 **CXXFLAGS**      Include directories                    M,ESer,EPar
 **DEFINES**       Compile-time/preprocessor definitions  M,ESer,EPar
 **LDFLAGS**       Link flags, link directories           M,ESer,EPar
 **LIBS**          Link libraries                         G,M,ESer,EPar,V
 **Files**         Files to compile                       All
 **WIN32DEFINES**  Windows specific definitions           None, always applies to all 
================== ====================================== ===================== 
 

*xml2cmake* also supports the *Conditional* addition of an include directory, definition, link library or source file.
The condition is a CMake variable that describes something related to the build: e.g. an OS-specification (*WIN32, LINUX, MACOS*), the availability of a third-party library (*HAVE_CONDUIT*) or a build option that can be toggled on/off (*VISIT_OSPRAY*). 
Conditionals must be specified in the *.code* file with *Target* specified as *xml2cmake*, as seen in the code file for the Volume plot:

.. literalinclude:: ../../plots/Volume/VolumeAttributes.code
    :lines: 691-699

These conditionals create these lines in the CMakeLists.txt:

.. literalinclude:: ../../plots/Volume/CMakeLists.txt
    :lines: 65-67

.. literalinclude:: ../../plots/Volume/CMakeLists.txt
    :lines: 61-63

.. literalinclude:: ../../plots/Volume/CMakeLists.txt
    :lines: 83-85

.. literalinclude:: ../../plots/Volume/CMakeLists.txt
    :lines: 115-117

.. literalinclude:: ../../plots/Volume/CMakeLists.txt
    :lines: 125-127


Info files
----------
Regeneration of Info files is handled by the *xml2info* :ref:`XML Tool<XML Tools>`. 

Changes to Info files should be placed in the plugin's *.code* file with the *Target* specified as *xml2info*, and a corresponding *Function* tag placed in the *.xml* file.

For instance, if an operator's Filter becomes useful elsewhere in VisIt, it may be moved out of the operator's directory and into src/avt/Filters.
In which case, a custom method for allcoating the filter must be created, as was done for the *Displace* operator.
Here is the *Displace* operator's code file:

.. literalinclude:: ../../operators/Displace/Displace.code

and the corresponding entry in the .xml file

.. literalinclude:: ../../operators/Displace/Displace.xml
    :lines: 13-14

Attributes
----------

Regeneration of Attributes is handled by three :ref:`XML Tools<XML Tools>`: one for cpp files (*xml2atts*), one for python (*xml2python*) and one for java (*xml2java*).

If fields are removed or renamed, custom *ProcessOldVersions* (cpp) and *getattr/setattr* (python) methods should be created for temporary support of the old fields saved in config/session files and python scripts.
The *VISIT_OBSOLETE_AT_VERSION* macro must be used to specify a version number where support for the old fields will be completely removed from VisIt, at least 2 point releases or later from the version where the removal/renaming occurs. 
A deprecation message should be issued from both the cpp and python code.
Cpp code has a default 'DeprecationMessage' method. 
For a removed field, it takes as arguments: the name of the removed field, the version where it will be completely unsupported. 
The arguments for a renamed field: the old field name, the new field name, the version where the old name will be completely unsupported.

Here is an example from the WellBore plot, which had a field removed in version 3.0.0

Here's code file for the cpp change:

.. literalinclude:: ../../plots/WellBore/WellBoreAttributes.code
    :lines: 265-312

Now for the python getattr and setattr methods:

.. literalinclude:: ../../plots/WellBore/WellBoreAttributes.code
    :lines: 315-380 

**Important**: Changes to fields in the Attributes are not allowed in patch releases, as it may cause incompatibility between client and server running different patches of the same major.minor release.





