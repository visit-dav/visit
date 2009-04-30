How to set version:  Change version.txt and look  in paraDISPluginInfo.C for "GetVersion()" and "GetID()". 

Important build note:  The makefiles here assume a couple things.  First, they assume that paraDIS_lib is a subdirectory of paraDIS (the plugin directory).  Also, since my code in paraDIS_lib is very very C++ centric and uses lots of STL trickery, it is imperative that all sources, notably including RC_cpp_lib, avtparaDISFileFormat.C, and paradis.C, get compiled with the exact same compiler, so that the exact same C++ headers are included.  I have abstracted as much of that aways as I can with a pure C interface to libparadis, but it still seems to be very picky.  If you don't, you will get segfaults in weird places, like string assignments, etc.  Visit will complain that it cannot read the data and the mdserver or engine_par will quit. 

Debugging tricks for VisIt: 
visit -o datafile -debug 3 -totalview engine_ser -totalview mdserver
Tells visit to run with verbosity of 3, and to run totalview on the mdserver and serial engine.  There is a -valgrind engine_ser ability too and it even works on some platforms.   You can tell totalview to break before it loads new libraries, restricting it from doing so for libs in /usr/gapps/visit, to set breakpoints early in my plugin. 


I now accept a cylinder file.  A cylinder file is in the same format as a Tecplot data file.  It may optionally include an ORIENTATION X Y Z keyword that reorients the cylinder along the given axis, assuming it starts out with orientation 0 0 1. This keyword must come early in the file.  

It is named either "dirname.cylinder" or "basename.cylinder" where "dirname" is the directory of the current paraDIS dumpfile being visualized, and "basename" is the name of the current dumpfile minus the .dat or .prds extension.  Preference is given the latter over the former.  E.g., if there exist the following files: 

test/step0001.dat
test/step0002.dat
test/step0003.dat
test/step0001.cylinder
test/test.cylinder
test/bogus.cylinder

Then "step0001.cylinder" will be used for step0001.dat, test.cylinder will be used for step0002.dat and step0003.dat, and bogus.cylinder will not get used at all.  

For multi-timestep data, if a cylinder file is found for the first timestep, one will be looked for for each timestep.  However, if one is not seen for the first timestep, one will never be looked for for any timestep. ' 
