.. _RemoteVisualization:

Remote Visualization
====================


Scientific simulations are almost always run on a powerful supercomputer and
accessed using desktop workstations. This means that the databases usually
reside on remote computers. In the past, the practice was to copy the
databases to a visualization server, a powerful computer with very fast
computer graphics hardware. With ever increasing database sizes, it no longer
makes sense to copy databases from the computer on which they were generated.
Instead, it makes more sense to examine the data on the powerful supercomputer
and use local graphics hardware to draw the visualization. VisIt can run in a
distributed mode that allows this exact use case. The GUI and viewer run
locally while the database server and parallel compute engine run on the
remote supercomputer. Running VisIt in distributed mode is almost as easy as
running all components locally. This chapter explains the differences between
running locally and remotely and describes how to run VisIt in distributed
mode.


.. toctree::
    :maxdepth: 2

    Distributed_mode
    Host_Profiles
    Managing_compute_engines
