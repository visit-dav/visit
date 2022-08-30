.. _PlotAtts:

PlotAtts
~~~~~~~~

This example program shows how to set plot attributes.
It changes a Pseudoocolor plot to be semi-transparent.


Running the program
"""""""""""""""""""

From a released client to an installed VisIt_ of the same version:

.. code-block:: shell

    java -cp . PlotAtts -stay -path /path/to/visit/bin -datapath /path/to/silodata


From a development build/java directory:

.. code-block:: shell

    java -cp .:visit.jar PlotAtts -stay -dv -datapath /path/to/silodata


Program source code
"""""""""""""""""""

.. literalinclude:: ../../../java/PlotAtts.java
   :language: Java


