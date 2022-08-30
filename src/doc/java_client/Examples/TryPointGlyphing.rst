.. _TryPointGlyphing:

TryPointGlyphing
~~~~~~~~~~~~~~~~

This example program shows how to change point size/type for Point meshes.


Running the program
"""""""""""""""""""

From a released client to an installed VisIt_ of the same version:

.. code-block:: shell

    java -cp . TryPointGlyphing -stay -path /path/to/visit/bin -datapath /path/to/silodata


From a development build/java directory:

.. code-block:: shell

    java -cp .:visit.jar TryPointGlyphing -stay -dv -datapath /path/to/silodata


Program source code
"""""""""""""""""""

.. literalinclude:: ../../../java/TryPointGlyphing.java
   :language: Java


