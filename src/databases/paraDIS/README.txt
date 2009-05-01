2009-05-01 Rich Cook

How to set version:  Change version.txt and look  in paraDISPluginInfo.C for "GetVersion()" and "GetID()". 

BUILDING is now handled by VisIt.  I have removed the CVS directory at the request of the Visit team.

Debugging tricks for VisIt: 
visit -o datafile -debug 3 -totalview engine_ser -totalview mdserver
Tells visit to run with verbosity of 3, and to run totalview on the mdserver and serial engine.  There is a -valgrind engine_ser ability too and it even works on some platforms.   You can tell totalview to break before it loads new libraries, restricting it from doing so for libs in /usr/gapps/visit, to set breakpoints early in my plugin. 


I now accept a cylinder file.  See the paraDIS_tecplot database for details.
