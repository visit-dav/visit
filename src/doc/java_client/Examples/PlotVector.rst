.. _PlotVector:

PlotVector
~~~~~~~~~~

This example program shows how to create a vector expression and then plot a Vector plot of that expression.
The **Displace** operator is also used to warp the coordinate system. 

Running the program
"""""""""""""""""""

From a released client to an installed VisIt_ of the same version:

.. code-block:: shell

    java -cp . PlotVector -stay -path /path/to/visit/bin -datapath /path/to/silodata


From a development build/java directory:

.. code-block:: shell

    java -cp .:visit.jar PlotVector -stay -dv -datapath /path/to/silodata


Program source code
"""""""""""""""""""

.. literalinclude:: ../../../java/PlotVector.java
   :language: Java


