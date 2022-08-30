.. _TryAnnotations:

TryAnnotations
~~~~~~~~~~~~~~

This example program shows how to control various annotation objects via the Java API. 

Running the program
"""""""""""""""""""

From a released client to an installed VisIt_ of the same version:

.. code-block:: shell

    java -cp . TryAnnotations -stay -path /path/to/visit/bin -datapath /path/to/silodata


From a development build/java directory:

.. code-block:: shell

    java -cp .:visit.jar TryAnnotations -stay -dv -datapath /path/to/silodata


Program source code
"""""""""""""""""""

.. literalinclude:: ../../../java/TryAnnotations.java
   :language: Java


