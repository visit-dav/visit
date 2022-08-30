.. _TryThreshold:

TryThreshold
~~~~~~~~~~~~

This example program shows how to use a Threshold operator with a Pseudocolor plot.


Running the program
"""""""""""""""""""

From a released client to an installed VisIt_ of the same version:

.. code-block:: shell

    java -cp . TryThreshold -stay -path /path/to/visit/bin -datapath /path/to/silodata


From a development build/java directory:

.. code-block:: shell

    java -cp .:visit.jar TryThreshold -stay -dv -datapath /path/to/silodata


Program source code
"""""""""""""""""""
ThresholdAttributes needs Vector to set ZonePortions, LowerBounds, and UpperBounds because more than one variable can be used with Threshold.
If more than one variable is requested (not demonstrated in this example), the first entry in the Vector contains information for the first variable, second entry contains information for the second variable and so on.


.. literalinclude:: ../../../java/TryThreshold.java
   :language: Java


