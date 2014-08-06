-----------------------------
Building VisIt plugin on OSX
-----------------------------

Because the existing cmake system would not work for me, I followed
these instructions to create an in-source build of the IDXPlugin for
VisIt:

http://www.visitusers.org/index.php?title=Building_plugins_using_CMake

Note: these instructions imply that you already have build VisIt. You could do this by following the instructions here: http://uintah.sci.utah.edu/trac/wiki/VisitBuildInstructions

make install the visit-plugin branch of sci-visus/nvisusio.git 
(note: do NOT enable "BUILD_VISIT_PLUGIN" in cmake)

Modify the IDXPlugin.xml to point to the correct directories for your installation of visus (see IDXPlugin.xml.cam for an example)

Run <visit_build>/bin/xml2cmake -clobber IDXPlugin.xml

Modify the resultant CMakeLists.txt:
  o add ${VISIT_INCLUDE_DIR}/avt/View to the INCLUDE_DIRECTORIES.  (maybe in IDXPlugin.xml now)
  o add avtview in LIBS (maybe in IDXPlugin.xml now)
  o add avtfilters_ser to the two serial linker sections, avtfilters_par to the parallel linker section

Create a symlink to the visuscpp/executable/idx_plugin directory in <visit_src>/databases.

Modify the <visit_src>/databases/CMakeLists.txt to include idx_plugin in the "required databases" list.

make -j in your <visit_build> directory.

