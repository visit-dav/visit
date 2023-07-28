.. _Debugging Tips:

Debugging Tips
==============
This section describes various method for debugging VisIt_ when you run into problems.
Whether it is a crash with a new filter you've designed, or badly formed data from a new database reader, one of these options should help.

.. _DebugLogs:

Debug logs
----------

The first method for debugging in VisIt_ is by using VisIt_'s debug logs.
When you run ``visit`` on the command line, you can optionally add the ``-debug 5`` arguments to make VisIt_ write out debugging logs.
The number of debugging logs can be 1, 2, 3, 4, or 5, with debugging log 5 being the most detailed.
When VisIt_'s components are told to run with debugging logs turned on, each component writes a set of debugging logs.
For example, the database server component will write A.mdserver.1.vlog, A.mdserver.2.vlog,...,A.mdserver.5.vlog if you pass ``-debug 5`` on the VisIt_ command line.
Subsequent runs of VisIt_ will copy current logs to files with a prepended *B* (instead of *A*), then *C* and so on.
Most current runs will always begin with *A*.
If you don't want that behavior, you may add ``-clobber_vlogs`` to VisIt_'s command line arguments.
The A.mdserver*.vlog and A.engine*.vlog files are useful when debugging a database reader plug-in.
The A.viewer*.vlog and A.engine*.vlog files are usefule when debugging a plot plugin.

The debugging logs will contain information written to them by the debugging statements in VisIt_'s source code.
If you want to add debugging statements to your AVT code then you can use the *debug1*, *debug2*, *debug3*, *debug4*, or *debug5* streams as shown in the following code listing.
Keep in mind that level 1 debug log files are less populated than level 5, and may be most useful when making temporary code modifications and debugging specific problems and not just providing general information.

.. container:: collapsible

  .. container:: header

    Example for using debug streams

  .. code-block:: c

    // NOTE - This code is incomplete and is for example purposes only.
    // Include this header for debug streams.
    #include <DebugStream.h>

    vtkDataSet *
    avtXXXXFileFormat::GetMesh(const char *meshname)
    {
        // Write messages to different levels of the debug logs.
        debug1 << "Hi from avtXXXXFileFormat::GetMesh" << endl;
        debug4 << "Many database plug-ins prefer debug4" << endl;
        debug5 << "Lots of detail from avtXXXXFileFormat::GetMesh" << endl;
        return 0;
    }

Other forms of the debug argument
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To create debug logs for only specific components, use ``-debug_<compname> <level>``.
For example ``-debug_mdserver 4`` will run the mdserver with level 4 debugging. 
Multiple ``-debug_<compname> <level>`` args are allowed.

For parallel engine logs, ``-debug_engine_rank <r>`` can be used to restrict debug output to the specified rank.
You can even have only every Nth processor output debug logs by using ``-debug-process-stride N``.


.. _DumpingPipelineInfo:

Dumping VTK objects and Pipeline information to disk
----------------------------------------------------

In addition to the ``-debug`` argument, VisIt_ also supports a ``-dump`` argument.
The ``-dump`` argument tells VisIt_'s compute engine to write intermediate results from AVT filters, scalable rendered images, and html pages.
The ``-dump`` option takes an optional argument that specifies the directory for -dump output files.

The intermediate results are VTK files containing the data for every stage of the pipeline execution so you can view the changes to the data made by each AVT filter.
Each VTK filename begins with a number indicating the order of the filter in the pipeline that saved the data, followed by an indication of whether it is an ``input`` or ``output`` for the filter, and finally the filter name.
For example, the input to the project filter could be ``0006.input.avtProjectFilter.vtk``.

The html files contain information about input to and output from each filter, including spatial and data extents, pipeline flags, and number of data files input and output.

While the VTK files dumped by this option are more useful when debugging plots and operators, they can still be useful for debugging database plugins, as data sent from the plugin can be examined.
The files generated at this first stage have ``gdb`` as the beginning of the filename, such as ``gdb.0003.output.GetOutput.dom0000.vtk``

When you run VisIt_ with the ``-dump`` argument, many files will be created since the data is saved at every stage in the execution of VisIt_'s data processing pipeline.
It is a good idea to keep this in mind and to remove those files from time to time.


Dumping only pipeline html files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To get only the html pages, and no VTK objects, use ``-info-dump`` instead.
It also takes an optional argument specifying the directory for output files.


Comparing --dump outputs
~~~~~~~~~~~~~~~~~~~~~~~~

Sometimes the output from ``-dump`` looks correct and doesn't immediately reveal why things are broken.
In those instances it may be helpful to compare a ``-dump`` run from a version of VisIt_ that isn't broken to the ``-dump`` run from the version that is broken.
Doing such a comparison might more quickly reveal which filter in the pipeline has changed it's output VTK object or even pipeline information.


Attaching a debugger 
--------------------

VisIt_ has various options for attaching debuggers on Linux machines, including ``gdb``, ``totalview``, and ``valgrind`` to name a few.
The full list is available in the :ref:`Startup options<StartupOptions>` section under ``Debugging options``.


WaitUntilFile function
----------------------

VisIt_ has a utility function called `WaitUntilFile` that will halt process execution until the file passed into the function has been created.
It takes one argument, a full-path filename referencing a file that does not yet exist.
The function will enter a loop, alternating between short sleeps and checking if the given filename exists.
Once it determines the filename exists, the function will exit and normal program flow will continue.
This allows time for you to attach a debugger to the running process and set breakpoints before creating the filename that signals the function to exit.

While this function can be used anywhere in VisIt_'s pipeline, it is especially useful for debugging problems with a component's startup process, where it may be harder to attach a debugger in time.

`WaitUntilFile` is declared in VisIt_'s `Utility.h` header.

To use `WaitUntilFile` to debug a component's startup process, simply modify the `main` program of the component, adding a call to the `WaitUntilFile` at the very beginning of the method.
Then rebuild and run VisIt_.
Once the desired component is in the `wait` state, attach the debugger, and set a breakpoint.
Then create the file that was passed as the argument to `WaitUntilFile`.

Don't forget the wait file will need to be deleted in between subsequent debugging sessions.

See the table below for components, the files containing their `main` method, and the name of `main` method.

=========  ========================  ================
component  file containing main      main method name
=========  ========================  ================
gui        src/gui/main.C             GUIMain
viewer     src/viewer/main/viewer.C   ViewerMain
engine     src/engine/main/main.C     EngineMain
cli        src/visitpy/cli/cli.C      main
=========  ========================  ================


.. container:: collapsible

  .. container:: header

    An example of modifying GUIMain with WaitUntilFile

  .. code-block:: c

       // Example only, the code block is incomplete.
      #include <Utility.h>

      int
      GUIMain(int argc, char **argv)
      {
          WaitUntilFile("~/guiwait.txt");

          int retval = 0;

          TRY
          {
              // Initialize error logging.
              VisItInit::SetComponentName("gui");
  

Debugging a regression failure outside of the test suite
--------------------------------------------------------

Sometimes the testing harness infrastructure gets in the way of debugging a failing regression test, and you just want to run the testing script or a portion of the script directly with VisIt_'s cli.
Here's a quick way to do just that.

First, you need a script that mimics some of the testing harness functions, so you don't need to modify the actual testing script as much.
Here's an example of what is needed:

.. container:: collapsible

  .. container:: header

    TestingStuff.py

  .. code-block:: python

    # script to aid in debugging regression tests outside of the testing harness
    # it mimics some of the testing methods so that actual test scripts don't
    # need to be modified so much

    # use this script by adding 'Source("TestingStuff.py")' to the top of a
    # regression test.  Use full path if the regression test doesn't live at
    # the same location as this script.



    # mimic testing 'data_path' by specifying a location where the testdata
    # can be found.  It is best if this points to an actual build/testdata dir
    # so that you are using the same data as the regression tests
    def data_path(fname):
        return "/my/path/to/VisIts/testdata/%s"%fname

    def silo_data_path(fname):
        return data_path("silo_hdf5_test_data/%s"%fname)

    def TurnOnAllAnnotations(givenAtts=0):
        """
        Turns on all annotations.

        Either from the default instance of AnnotationAttributes,
        or using 'givenAtts'.
        """
        if (givenAtts == 0):
            a = AnnotationAttributes()
        else:
            a = givenAtts
        a.axes2D.visible = 1
        a.axes3D.visible = 1
        a.axes3D.triadFlag = 1
        a.axes3D.bboxFlag = 1
        a.userInfoFlag = 0
        a.databaseInfoFlag = 1
        a.legendInfoFlag = 1
        SetAnnotationAttributes(a)

    def TurnOffAllAnnotations(givenAtts=0):
        """
        Turns off all annotations.

        Either from the default instance of AnnotationAttributes,
        or using 'givenAtts'.
        """
        if (givenAtts == 0):
            a = AnnotationAttributes()
        else:
            a = givenAtts
        a.axes2D.visible = 0
        a.axes3D.visible = 0
        a.axes3D.triadFlag = 0
        a.axes3D.bboxFlag = 0
        a.userInfoFlag = 0
        a.databaseInfoFlag = 0
        a.legendInfoFlag = 0
        SetAnnotationAttributes(a)

    def Test(fname):
        swa = SaveWindowAttributes()
        swa.family = 0
        swa.fileName = fname
        swa.screenCapture = 0
        SetSaveWindowAttributes(swa)
        SaveWindow()

    def Test(fname, swa = 0, alreadySaved=0):
        if (swa != 0):
            sa = swa
        else:
            sa = SaveWindowAttributes()
        sa.screenCapture = 1
        sa.family = 0
        sa.fileName = fname
        SetSaveWindowAttributes(sa)
        SaveWindow()

    def TestText(name, results):
        print("%s: %s"%(name, results))

    def TestSection(stuff):
        print(stuff)

    def Exit():
        exit()


Now, you can copy a regression test to the same directory as this script, add
``Source("TestingStuff.py")`` to the top of the regression test, and run ``visit -cli -s testname.py``, along with any debugging options you desire.
