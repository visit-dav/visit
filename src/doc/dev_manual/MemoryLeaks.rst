Finding Memory Leaks
====================

Overview
--------

We support several mechanisms to find memory leaks. The two best
mechanisms are using Valgrind and vtkDebugLeaks. Valgrind is used to detect
memory leaks in a specific component. You run VisIt_ with the appropriate
options and then when VisIt_ is finished running you will get logs with a
report of memory leaks and usage. The log files are quite large. The top
of the log file contains stack traces of where the memory was allocated for
each chunk of memory allocated. It is sorted from the smallest leaks to
the largest leaks. At the bottom of the log is a summary of memory leaks and
usage. vtkDebugLeaks is specifically used to find VTK memory leaks. It
provides a list of all the VTK objects that are still in use when VisIt_
terminates to the terminal. There will be one list for the viewer and one
for the engine (multiple lists if running in parallel).

Building VisIt_ for Valgrind and vtkDebugLeaks
----------------------------------------------

The following steps were from building and running VisIt_ 3.1 on Quartz, a
Linux cluster.

Building the Third party Libraries
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In order to use vtkDebugLeaks you will need to enable it when you build
VTK. Edit the build_visit script and find the line: ::

    vtk_debug_leaks="false"

and change it to: ::

    vtk_debug_leaks="true"

In order for the stack traces from Valgrid to be the most useful, you should
build the third party libraries with debug support. In our case we are
going to do a minimal build with just the Silo and HDF5 I/O libraries. ::

    ./build_visit3_1_0 --required --mesagl --llvm --silo --hdf5 --debug --no-visit \
    --thirdparty-path /usr/workspace/wsa/visit/visit/thirdparty_shared/3.1.0/toss3_debug \
    --makeflags -j16

Building VisIt_
~~~~~~~~~~~~~~~

Just like the third party libraries, VisIt_ needs to be built with debug
support in order for Valgrind to produce useful stack traces. Furthermore,
VisIt_ contains conditional code that does additional cleanup at exit to
eliminate spurious memory leaks that is enabled with VISIT_DEBUG_LEAKS.
The following steps were used to build VisIt_ as described. ::

    cd visit3.1.0
    mkdir build
    cd build
    /usr/workspace/wsa/visit/visit/thirdparty_shared/3.1.0/toss3_debug/cmake/3.9.3/linux-x86_64_gcc-4.9/bin/cmake -DCMAKE_BUILD_TYPE=Debug -DVISIT_CONFIG_SITE=/usr/workspace/wsa/brugger/visit_memory/quartz2498.cmake -DCMAKE_CXX_FLAGS:STRING="-DDEBUG_MEMORY_LEAKS" ../src
    make -j 36

You are now ready to start looking for memory leaks.

Running VisIt_ with Valgrind
----------------------------

Follow these steps to run Valgrind on the viewer to find memory leaks from
a basic use case.

1. Run VisIt_ with Valgrind on the viewer in nowin mode. ::

    cd ..
    mkdir run1
    cd run1
    ../visit3.1.0/build/bin/visit -valgrind viewer -nowin

2. Open ``wave.visit``.
3. Create a Pseudocolor plot of ``pressure``.
4. Save the window.
5. Delete the plot.
6. Close the database.
7. Exit

There are several things to note. We ran in nowin mode to eliminate leaks
from Qt, which are difficult to address. Since we ran in nowin mode we had
to save an image in order to do some rendering. After we saved the image
we deleted the plot and closed the database to clean up as much memory as
possible.

After VisIt_ exits, you will get vtkDebugLeaks output sent to the terminal
as well as log files created by Valgrind. Let's look at the Valgrind output
first.

Looking at the Valgrind Output
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Valgrind creates several log files with the output. You are interested in
the largest one. Here are the files generated from the run described above. ::

    ls -l
    total 48792
    -rw------- 1 brugger brugger 49715025 Dec  4 11:44 vg_viewer_8870.log
    -rw------- 1 brugger brugger      449 Dec  4 11:43 vg_viewer_8915.log
    -rw------- 1 brugger brugger      449 Dec  4 11:44 vg_viewer_9281.log
    -rw------- 1 brugger brugger    32373 Dec  4 11:44 visit0000.png

Here is the output from the end of the largest log file. ::

    tail -11 vg_viewer_8870.log
    ==8870== LEAK SUMMARY:
    ==8870==    definitely lost: 0 bytes in 0 blocks
    ==8870==    indirectly lost: 0 bytes in 0 blocks
    ==8870==      possibly lost: 33,097 bytes in 245 blocks
    ==8870==    still reachable: 30,386,572 bytes in 24,057 blocks
    ==8870==                       of which reachable via heuristic:
    ==8870==                         stdstring          : 25,296 bytes in 649 blocks
    ==8870==         suppressed: 0 bytes in 0 blocks
    ==8870== 
    ==8870== For counts of detected and suppressed errors, rerun with: -v
    ==8870== ERROR SUMMARY: 202 errors from 202 contexts (suppressed: 0 from 0)

This is actually pretty good. There is still more work to be done to address
the possibly lost memory and then there may be issues with the still
reachable. This is probably primarily from a lack of cleanup before exiting.

Looking at the vtkDebugLeaks Output
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You will get vtkDebugLeaks output from both the viewer and engine since 
both have VTK code and both were linked against VTK built with vtkDebugLeaks.
Here is the engine output, which came out first. ::

    vtkDebugLeaks has detected LEAKS!
    Class "9vtkBufferIxE" has 1 instance still around.
    Class "vtkDataSetAttributes" has 2 instances still around.
    Class "vtkGraphInternals" has 1 instance still around.
    Class "vtkOutputWindow" has 1 instance still around.
    Class "vtkInformation" has 1 instance still around.
    Class "vtkInformationIntegerValue" has 4 instances still around.
    Class "vtkGraphEdge" has 1 instance still around.
    Class "vtkIdTypeArray" has 1 instance still around.
    Class "vtkTypeUInt32Array" has 1 instance still around.
    Class "vtkFieldData" has 1 instance still around.
    Class "vtkMergeTree" has 1 instance still around.
    Class "vtkCommand or subclass" has 1 instance still around.
    Class "9vtkBufferIjE" has 1 instance still around.

As you can see, it had relatively few leaks associated with VTK. These may
not even be leaks, they are probably from a lack of cleaning up before exiting.
More work needs to be done here.

Here is the viewer output. ::

    vtkDebugLeaks has detected LEAKS!
    Class "vtkOpenGLRenderTimerLog" has 1 instance still around.
    Class "vtkOpenGLTextActor" has 33 instances still around.
    Class "vtkOpenGLTextMapper" has 1 instance still around.
    Class "vtkTextureObject" has 5 instances still around.
    Class "9vtkBufferIxE" has 146 instances still around.
    Class "vtkTexturedActor2D" has 1 instance still around.
    Class "vtkBackgroundActor" has 1 instance still around.
    Class "vtkCellData" has 195 instances still around.
    Class "vtkVisItTextActor" has 1 instance still around.
    Class "vtkInformationIntegerVectorValue" has 17 instances still around.
    Class "vtkInformationVector" has 1454 instances still around.
    Class "vtkVisItCubeAxesActor" has 1 instance still around.
    Class "vtkPerspectiveTransform" has 6 instances still around.
    Class "vtkPointData" has 195 instances still around.
    Class "vtkProperty2D" has 16 instances still around.
    Class "vtkCompositeDataPipeline" has 290 instances still around.
    Class "vtkMatrix3x3" has 236 instances still around.
    Class "vtkTrivialProducer" has 100 instances still around.
    Class "vtkAxesActor2D" has 1 instance still around.
    Class "vtkOpenGLIndexBufferObject" has 1006 instances still around.
    Class "vtkMatrix4x4" has 1331 instances still around.
    Class "vtkPickingManager" has 1 instance still around.
    Class "QVTKInteractor" has 1 instance still around.
    Class "vtkCoordinate" has 160 instances still around.
    Class "vtkSimpleTransform" has 35 instances still around.
    Class "vtkFollower" has 88 instances still around.
    Class "vtkOutputWindow" has 1 instance still around.
    Class "vtkPoints" has 192 instances still around.
    Class "vtkInformation" has 3503 instances still around.
    Class "vtkActorCollection" has 3 instances still around.
    Class "vtkLine" has 5 instances still around.
    Class "vtkGenericOpenGLRenderWindow" has 1 instance still around.
    Class "vtkVolumeCollection" has 3 instances still around.
    Class "vtkPropCollection" has 5 instances still around.
    Class "vtkInformationIntegerPointerValue" has 36 instances still around.
    Class "vtkTriad2D" has 1 instance still around.
    Class "vtkPolyData" has 159 instances still around.
    Class "vtkLookupTable" has 34 instances still around.
    Class "vtkPixel" has 5 instances still around.
    Class "vtkAppendPolyData" has 1 instance still around.
    Class "vtkOpenGLImageMapper" has 1 instance still around.
    Class "vtkPropPicker" has 1 instance still around.
    Class "vtkActor2D" has 11 instances still around.
    Class "vtkOpenGLCamera" has 3 instances still around.
    Class "vtkOpenGLVertexArrayObject" has 1006 instances still around.
    Class "vtkOpenGLActor" has 115 instances still around.
    Class "vtkOpenGLPolyDataMapper" has 118 instances still around.
    Class "vtkIdList" has 10 instances still around.
    Class "vtkWorldPointPicker" has 1 instance still around.
    Class "vtkDoubleArray" has 10 instances still around.
    Class "vtkMatrixToLinearTransform" has 26 instances still around.
    Class "vtkAlgorithmOutput" has 194 instances still around.
    Class "vtkCullerCollection" has 3 instances still around.
    Class "vtkOpenGLRenderer" has 3 instances still around.
    Class "vtkPolyDataAlgorithm" has 89 instances still around.
    Class "vtkDepthSortPolyData2" has 1 instance still around.
    Class "vtkInformationIntegerValue" has 3756 instances still around.
    Class "vtkOpenGLLight" has 10 instances still around.
    Class "vtkOpenGLPolyDataMapper2D" has 45 instances still around.
    Class "vtkTextProperty" has 93 instances still around.
    Class "vtkCellArray" has 146 instances still around.
    Class "vtkRendererCollection" has 1 instance still around.
    Class "vtkShaderProgram" has 6 instances still around.
    Class "vtkVisItAxisActor2D" has 9 instances still around.
    Class "vtkOpenGLShaderCache" has 1 instance still around.
    Class "vtkTDxInteractorStyleCamera" has 3 instances still around.
    Class "vtkImageData" has 36 instances still around.
    Class "vtkFloatArray" has 222 instances still around.
    Class "vtkInformationStringValue" has 108 instances still around.
    Class "vtkInformationExecutivePortVectorValue" has 194 instances still around.
    Class "vtkOpenGLVertexBufferObject" has 9 instances still around.
    Class "vtkIdTypeArray" has 146 instances still around.
    Class "vtkTransform" has 541 instances still around.
    Class "vtkOutlineSource" has 5 instances still around.
    Class "vtkOpenGLVertexBufferObjectGroup" has 163 instances still around.
    Class "vtkFieldData" has 195 instances still around.
    Class "vtkVisItAxisActor" has 12 instances still around.
    Class "vtkOpenGLProperty" has 40 instances still around.
    Class "vtkOpenGLTexture" has 36 instances still around.
    Class "vtkLineSource" has 1 instance still around.
    Class "vtkInformationDoubleVectorValue" has 90 instances still around.
    Class "vtkLightCollection" has 3 instances still around.
    Class "vtkUnsignedCharArray" has 41 instances still around.
    Class "vtkShader" has 18 instances still around.
    Class "vtkTDxInteractorStyleSettings" has 3 instances still around.
    Class "vtkStreamingDemandDrivenPipeline" has 100 instances still around.
    Class "vtkTextureUnitManager" has 1 instance still around.
    Class "vtkOpenGLVertexBufferObjectCache" has 1 instance still around.
    Class "vtkActor2DCollection" has 3 instances still around.
    Class "vtkTimerLog" has 166 instances still around.
    Class "9vtkBufferIfE" has 222 instances still around.
    Class "9vtkBufferIdE" has 10 instances still around.
    Class "vtkCommand or subclass" has 208 instances still around.
    Class "9vtkBufferIhE" has 41 instances still around.
    Class "vtkInformationExecutivePortValue" has 237 instances still around.
    Class "vtkFXAAOptions" has 3 instances still around.

As you can see, the viewer has considerably more leaks associated with VTK.
Again, these may not be leaks but merely a lack of cleanup before exiting.
More work needs to be done here as well.

That's it. Happy hunting!
