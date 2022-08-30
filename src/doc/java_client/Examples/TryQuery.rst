.. _TryQuery:

TryQuery
~~~~~~~~

This example program shows how to use some of VisIt's query capabilities to perform picks and lineouts.


Running the program
"""""""""""""""""""

From a released client to an installed VisIt_ of the same version:

.. code-block:: shell

    java -cp . TryQuery -stay -path /path/to/visit/bin -datapath /path/to/silodata


From a development build/java directory:

.. code-block:: shell

    java -cp .:visit.jar TryQuery -stay -dv -datapath /path/to/silodata


Program source code
"""""""""""""""""""

.. literalinclude:: ../../../java/TryQuery.java
   :language: Java


