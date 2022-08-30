.. _DualClients:

DualClients
~~~~~~~~~~~

This code example shows how to create a Java program that launches VisIt_'s Python **CLI** program and send Python command strings to it for interpretation.
This example program also implements the SimpleObserver interface which lets us observe state objects.
In this case, we observe the plot list and print it whenever we see it.


Running the program
"""""""""""""""""""

From a released client to an installed VisIt_ of the same version:

.. code-block:: shell

    java -cp . DualClients -stay -path /path/to/visit/bin -datapath /path/to/silodata


From a development build/java directory:

.. code-block:: shell

    java -cp .:visit.jar DualClients -stay -dv -datapath /path/to/silodata


Program source code
"""""""""""""""""""

.. literalinclude:: ../../../java/DualClients.java
   :language: Java


