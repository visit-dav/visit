.. _JavaClient:

Java Client
===========

VisIt_'s Java API allows Java applications to control VisIt_'s viewer.
Java applications can launch VisIt_'s viewer window and control it programmatically, even replacing VisIt_'s GUI with a custom Java GUI.

Disclaimers:

    VisIt_ will not draw its plots inside of your Java application.
    VisIt_'s separate viewer window will be activated and controlled from your application.
    VisIt_ will not be able to plot data from your Java application without going through the file system.

The Java client for a particular version of VisIt is available as separate tarball on our `downloads page <https://visit-dav.github.io/visit-website/releases-as-tables>`_.
Look for ``jvisit<version>.tar.gz``, e.g. ``jvisit3.3.0.tar.gz``.
Simply untar to the directory of your choosing to use it.


Building from source
--------------------

To build the java client from a source build of VisIt_, you must set the ``VISIT_JAVA`` CMake bool variable to ``true`` when configuring VisIt_ with CMake.
Once the CMake configure step is complete, cd to the java subdirectory of the build directory and type ``make``.
This will compile the core parts of the Java interface.
There are two other targets that can be built from the java directory: ``main`` and ``pack``. 
The ``main`` target will build the class files for all the examples.
The ``pack`` target causes a JAR file to be created and then packaged up with the Java source code, docs and example programs into a TAR file that can be shared.
The created ``visit.jar`` file will be present in the build directory.
The ``pack`` target will also build the example class files if not already built.

Examples
--------

What follows are examples of how VisIt_'s Java client can be used.
Source code for each example is available in the ``java`` subdirectory of the source code repository.
There are several arguments that are common to all the examples:

`-stay`: 
    tells the program to continue running after the work() method has completed instead of exiting.

`-path`: 
    path to VisIt_'s bin directory, eg, ``/usr/local/visit/3.3.0/bin``

`-datapath`: 
    path to VisIt_'s silo example data (used by many of the examples)
    eg: ``/usr/local/visit/3.3.0/data``

`-dv`: 
    Tells the example application that it will be connecting to a development version of VisIt_.
    This is important because an installed version of VisIt_ is structured slightly differently than a build.

All of the examples are available from the top (``.``) directory of the untarred `jvisit<version>.tar.gz`.

.. toctree::
   :maxdepth: 1

   Basic example                              <Examples/RunViewer>
   Controlling lighting                       <Examples/TryLighting>
   Performing queries                         <Examples/TryQuery>
   Getting metadata                           <Examples/GetMetaData>
   Controlling annotations                    <Examples/TryAnnotations>
   Making host profiles                       <Examples/MakeHostProfile>
   Opening the visIt GUI from Java            <Examples/OpenGUI>
   Determining which variables can be plotted <Examples/PlotTypes>
   Executing Python from Java                 <Examples/DualClients>
   Plotting vectors from Java                 <Examples/PlotVector>
   Changing plot attributes                   <Examples/PlotAtts>
   Changing points size and shape             <Examples/TryPointGlyphing>
   Using Threshold operator                   <Examples/TryThreshold>


Acknowledgements
----------------

This document is primarily based on visitusers.org wiki pages written by Brad Whitlock.
The Java client itself and most of the examples were also initially created by Brad in 2002.

