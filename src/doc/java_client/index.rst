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
Note that the tarball untars its contents to ``.`` and not its own directory.
Its best to make a directory (e.g. ``mkdir visit_java``), copy the tarball into that directory and untar it there.

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

``-stay``: 
    tells the program to continue running after the ``work()`` method has completed instead of exiting.

``-path``: 
    path to VisIt_'s bin directory, eg, ``/usr/local/visit/3.3.0/bin``

``-datapath``: 
    path to directory holding VisIt_'s silo example data (used by many of the examples)
    eg: ``/usr/local/visit/3.3.0/data/``. **Note**: the trailing slash is **required**.

``-dv``: 
    A shorthand for ``-path ../bin``.

``-nopty``:
    Is forwarded to VisIt_ launcher to make VisIt_ prompt for passwords in the console/terminal from which Java was run.

All of the examples are available from the top (``.``) directory of the untarred ``jvisit<version>.tar.gz``.

Java Classes Documentation
--------------------------

There is a ``docs`` subdirectory from the untarred ``jvisit<version>.tar.gz`` with a bunch of ``.html`` files.
If you point a web browser at ``index.html`` there, you can find a lot of documentation on the various Java classes available and used in these examples.

Running the Examples
--------------------

Assuming the current working directory is the directory where ``jvisit<version>.tar.gz`` was untarred, to run an example named ``Example.class``, do the following...

.. code-block:: shell

    java -cp .:visit.jar Example -stay -path /path/to/visit/bin -datapath /path/to/silo/data/dir/

.. note:: The Java program to run, here ``Example``, does not include the ``.class`` extension when it is specified on the command line to ``java``.

   The trailing slash for the ``-datapath`` argument is required.

   All arguments *before* the name of the Java program to run are arguments to ``java``.

   All arguments *after* the name of the Java program to run are arguments to the program, or to VisIt_.
   Any arguments not consumed by the Java program are forwarded to VisIt_.

   In client/server scenarios, if the above command line does not work or if the viewer seems to stall when connecting to the remote computer, try adding the ``-nopty`` argument to make VisIt_ prompt for passwords in the console from which Java was run.
   This should rarely be necessary.

Basic example
~~~~~~~~~~~~~

This program defines the ``RunViewer`` class, which serves as the base class for some of the other example Java programs.
The ``RunViewer`` program does much of its initialization of the ViewerProxy, the main class for controlling VisIt_'s viewer, in its ``run()`` method.
The actual VisIt_-related work, however, is defined in the ``work()`` method and is overridden in subclasses to perform different VisIt_ functionality.
This program's ``work()`` method opens the VisIt_ globe.silo database, sets some annotation properties, and makes a Pseudocolor plot and a Mesh plot.
After the plots are drawn, the program changes the plot variables a couple of times and saves out images for each of the plotted variables. 

.. container:: collapsible

    .. container:: header

        **Show/Hide Code for** ``RunViewer``

    .. literalinclude:: ../../java/RunViewer.java
       :language: Java

Controlling lighting
~~~~~~~~~~~~~~~~~~~~

This example program is based on the ``RunViewer`` example program and it shows how to modify lighting in VisIt_.
The basic procedure is to obtain a handle to the state object that you want to modify, in this case, ``LightList`` and then modify the state object and call its Notify() method to send the changed object back to VisIt_'s viewer.
Once the changed object has been sent back to VisIt_'s viewer, you call a method from ViewerMethods that tells VisIt_ to apply the sent object to its internal state. 

.. container:: collapsible

    .. container:: header

        **Show/Hide Code for** ``TryLighting``

    .. literalinclude:: ../../java/TryLighting.java
       :language: Java

Performing queries
~~~~~~~~~~~~~~~~~~

This example program shows how to use some of VisIt_'s query capabilities to perform picks and lineouts.

.. container:: collapsible

    .. container:: header

        **Show/Hide Code for** ``TryQuery``

    .. literalinclude:: ../../java/TryQuery.java
       :language: Java

Getting metadata
~~~~~~~~~~~~~~~~

This program shows how to query metadata for a database and print it to the console.
In real applications, of course, you'd do something more constructive with the metadata object such as populate variable menus in a GUI. 

.. container:: collapsible

    .. container:: header

        **Show/Hide Code for** ``GetMetaData``

    .. literalinclude:: ../../java/GetMetaData.java
       :language: Java

Controlling annotations
~~~~~~~~~~~~~~~~~~~~~~~

This example program shows how to control various annotation objects via the Java API. 

.. container:: collapsible

    .. container:: header

        **Show/Hide Code for** ``TryAnnotations``

    .. literalinclude:: ../../java/TryAnnotations.java
       :language: Java


Making host profiles
~~~~~~~~~~~~~~~~~~~~

This program shows how to create a host profile, add it to the host profile list, and send it to the viewer.
The program then goes on to access data on the remote computer, making use of the host profile that was created.
Additional options such as how to launch the engine in parallel could be added to the host profile.
Also, more profiles could be added to the host profile list before sending it to the viewer. 

.. container:: collapsible

    .. container:: header

        **Show/Hide Code for** ``MakeHostProfile``

    .. literalinclude:: ../../java/MakeHostProfile.java
       :language: Java

Opening the VisIt_ GUI from Java
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This program shows how to start the VisIt_ **GUI** from within your Java application.
By altering the arguments passed to the ``OpenClient()`` method, you could launch other VisIt_ clients too.
A VisIt_ client is a program that uses the ``ViewerProxy`` class to control the viewer.
Examples of VisIt_ clients are: VisIt_'s **GUI**, VisIt_'s Python interface (**CLI**), and any program that uses the VisIt_ Java interface. 

The important part of this code is the call to the ``OpenClient()`` method.
The ``OpenClient`` method takes 3 arguments: ``clientName``, ``clientProgram``, ``clientArgs``.
The ``clientName`` is the internal name that will be used to identify the client inside of VisIt_.
You can pass any name that you want for this.
The ``clientProgram`` argument is a string that identifies the executable for your program.
The ``clientArgs`` argument lets you pass command line arguments to your program when it is started.
When you call ``OpenClient()``, the VisIt_ viewer will attempt to launch the specified VisIt_ client and then the client will be attached to VisIt_ and can control the VisIt_ viewer.
Any number of VisIt_ clients can be connected to the VisIt_ viewer.

.. container:: collapsible

    .. container:: header

        **Show/Hide Code for** ``OpenGUI``

    .. literalinclude:: ../../java/OpenGUI.java
       :language: Java

Determining which variables can be plotted
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This program shows how to open a file and determine which plots can be used with the data from the file.

Each plot in VisIt_ responds to a certain set of variable types (scalar, vector, and so on).
When you open a file, you get a list of variables in the metadata object.
You must match up the variable types supported by a plot and the variables from the metadata in order to determine which plots can accept which variables from the database.
This example program demonstrates a method for doing this comparison.

.. note:: The Java implementation does not offer a ``GetVariableTypes`` method in the plugin interface as it should.
   This is an oversight that may be corrected in a future version of VisIt_.
   In the meantime, this program's ``GetVariableTypes`` method can be used to fulfill the same purpose.

.. container:: collapsible

    .. container:: header

        **Show/Hide Code for** ``PlotTypes``

    .. literalinclude:: ../../java/PlotTypes.java
       :language: Java

Executing Python from Java
~~~~~~~~~~~~~~~~~~~~~~~~~~

This code example shows how to create a Java program that launches VisIt_'s Python **CLI** program and send Python command strings to it for interpretation.
This example program also implements the SimpleObserver interface which lets us observe state objects.
In this case, we observe the plot list and print it whenever we see it.

.. container:: collapsible

    .. container:: header

        **Show/Hide Code for** ``DualClients``

    .. literalinclude:: ../../java/DualClients.java
       :language: Java

Plotting vectors from Java
~~~~~~~~~~~~~~~~~~~~~~~~~~

This example program shows how to create a vector expression and then plot a Vector plot of that expression.
The **Displace** operator is also used to warp the coordinate system. 

.. container:: collapsible

    .. container:: header

        **Show/Hide Code for** ``PlotVector``

    .. literalinclude:: ../../java/PlotVector.java
       :language: Java

Changing plot attributes
~~~~~~~~~~~~~~~~~~~~~~~~

This example program shows how to set plot attributes.
It changes a Pseudoocolor plot to be semi-transparent.

.. container:: collapsible

    .. container:: header

        **Show/Hide Code for** ``PlotAtts``

    .. literalinclude:: ../../java/PlotAtts.java
       :language: Java

Changing points size and shape
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example program shows how to change point size/type for Point meshes.

.. container:: collapsible

    .. container:: header

        **Show/Hide Code for** ``TryPointGlyphing``

    .. literalinclude:: ../../java/TryPointGlyphing.java
       :language: Java

Using Threshold operator
~~~~~~~~~~~~~~~~~~~~~~~~

This example program shows how to use a Threshold operator with a Pseudocolor plot.

ThresholdAttributes needs Vector to set ZonePortions, LowerBounds, and UpperBounds because more than one variable can be used with Threshold.
If more than one variable is requested (not demonstrated in this example), the first entry in the Vector contains information for the first variable, second entry contains information for the second variable and so on.

.. container:: collapsible

    .. container:: header

        **Show/Hide Code for** ``TryThreshold``

    .. literalinclude:: ../../java/TryThreshold.java
       :language: Java

Acknowledgements
----------------

This document is primarily based on visitusers.org wiki pages written by Brad Whitlock.
The Java client itself and most of the examples were also initially created by Brad in 2002.

