.. _BasicExample:

RunViewer
~~~~~~~~~

This program defines the *RunViewer* class, which serves as the base class for some of the other example Java programs.
The *RunViewer* program does much of its initialization of the ViewerProxy, the main class for controlling VisIt_'s viewer, in its *run()* method.
The actual VisIt_-related work, however, is defined in the *work()* method and is overridden in subclasses to perform different VisIt_ functionality.
This program's *work()* method opens the VisIt_ globe.silo database, sets some annotation properties, and makes a Pseudocolor plot and a Mesh plot.
After the plots are drawn, the program changes the plot variables a couple of times and saves out images for each of the plotted variables. 


Running the program
"""""""""""""""""""

From a released client to an installed VisIt_ of the same version:

.. code-block:: shell

    java -cp . RunViewer -stay -path /path/to/visit/bin -datapath /path/to/silodata


From a development build/java directory:

.. code-block:: shell

    java -cp .:visit.jar RunViewer -stay -dv -datapath /path/to/silodata


Program source code
"""""""""""""""""""

.. literalinclude:: ../../../java/RunViewer.java
   :language: Java


